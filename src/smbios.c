#include "smbios.h"
#include "general.h"
#include <string.h>

#define MAX_OEM_STRING 256

// Глобальная копия SMBIOS таблицы
UINT8 *g_SmbiosCopy = NULL;
UINTN g_SmbiosSize = 0;

UINT16 TableLenght(SMBIOS_STRUCTURE_POINTER table)
{
    char *pointer = (char *)(table.Raw + table.Hdr->Length);
    while ((*pointer != 0) || (*(pointer + 1) != 0))
    {
        pointer++;
    }
    return (UINT16)((UINTN)pointer - (UINTN)table.Raw + 2);
}

UINT16 SmbiosTableLength(SMBIOS_STRUCTURE_POINTER SmbiosTableN)
{
    CHAR8 *AChar;
    UINT16 Length;

    AChar = (CHAR8 *)(SmbiosTableN.Raw + SmbiosTableN.Hdr->Length);
    while ((*AChar != 0) || (*(AChar + 1) != 0))
    {
        AChar++;
    }
    Length = (UINT16)((UINTN)AChar - (UINTN)SmbiosTableN.Raw + 2);
    return Length;
}

// Вычислить общий размер всех SMBIOS таблиц
UINTN GetSmbiosTablesSize(SMBIOS3_STRUCTURE_TABLE *SmbiosPoint)
{
    SMBIOS_STRUCTURE_POINTER Current;
    UINTN TotalSize = 0;
    
    Current.Raw = (UINT8 *)((UINTN)SmbiosPoint->TableAddress);
    
    while (Current.Hdr->Type != SMBIOS_TYPE_END_OF_TABLE)
    {
        UINT16 TableLen = SmbiosTableLength(Current);
        TotalSize += TableLen;
        Current.Raw = Current.Raw + TableLen;
    }
    
    // Добавляем END_OF_TABLE структуру
    TotalSize += SmbiosTableLength(Current);
    
    return TotalSize;
}

// Создать копию SMBIOS таблиц в записываемой памяти
EFI_STATUS CreateWritableSmbiosCopy(SMBIOS3_STRUCTURE_TABLE *SmbiosPoint)
{
    EFI_STATUS Status;
    
    // Вычисляем размер
    g_SmbiosSize = GetSmbiosTablesSize(SmbiosPoint);
    Print(L"[INFO] Total SMBIOS tables size: %d bytes\n", g_SmbiosSize);
    
    // Выделяем память
    Status = gBS->AllocatePool(EfiRuntimeServicesData, g_SmbiosSize, (VOID **)&g_SmbiosCopy);
    if (EFI_ERROR(Status))
    {
        Print(L"[ERROR] Failed to allocate memory for SMBIOS copy: %r\n", Status);
        return Status;
    }
    
    // Копируем данные
    CopyMem(g_SmbiosCopy, (VOID *)((UINTN)SmbiosPoint->TableAddress), g_SmbiosSize);
    
    Print(L"[INFO] Created writable SMBIOS copy at 0x%lx\n", (UINT64)(UINTN)g_SmbiosCopy);
    Print(L"[INFO] Original SMBIOS at 0x%lx\n", (UINT64)(UINTN)SmbiosPoint->TableAddress);
    
    return EFI_SUCCESS;
}

// Обновить указатель SMBIOS Entry Point на новую копию
EFI_STATUS UpdateSmbiosEntryPoint(SMBIOS3_STRUCTURE_TABLE *SmbiosPoint)
{
    Print(L"[INFO] Updating SMBIOS Entry Point...\n");
    Print(L"[INFO] Old table address: 0x%lx\n", (UINT64)(UINTN)SmbiosPoint->TableAddress);
    
    // Обновляем адрес таблицы
    SmbiosPoint->TableAddress = (EFI_PHYSICAL_ADDRESS)(UINTN)g_SmbiosCopy;
    
    // Обновляем максимальный размер
    SmbiosPoint->TableMaximumSize = (UINT32)g_SmbiosSize;
    
    Print(L"[INFO] New table address: 0x%lx\n", (UINT64)(UINTN)SmbiosPoint->TableAddress);
    
    return EFI_SUCCESS;
}

SMBIOS_STRUCTURE_POINTER FindTableByType(SMBIOS3_STRUCTURE_TABLE *SmbiosPoint, UINT8 SmbiosType, UINTN IndexTable)
{
    SMBIOS_STRUCTURE_POINTER SmbiosTableN;
    UINTN SmbiosTypeIndex;

    SmbiosTypeIndex = 0;
    SmbiosTableN.Raw = (UINT8 *)((UINTN)SmbiosPoint->TableAddress);
    if (SmbiosTableN.Raw == NULL)
    {
        return SmbiosTableN;
    }
    while ((SmbiosTypeIndex != IndexTable) || (SmbiosTableN.Hdr->Type != SmbiosType))
    {
        if (SmbiosTableN.Hdr->Type == SMBIOS_TYPE_END_OF_TABLE)
        {
            SmbiosTableN.Raw = NULL;
            return SmbiosTableN;
        }
        if (SmbiosTableN.Hdr->Type == SmbiosType)
        {
            SmbiosTypeIndex++;
        }
        SmbiosTableN.Raw = (UINT8 *)(SmbiosTableN.Raw + SmbiosTableLength(SmbiosTableN));
    }
    return SmbiosTableN;
}

UINTN iStrLen(CONST CHAR8 *String, UINTN MaxLen)
{
    UINTN Len = 0;
    CONST CHAR8 *BA;
    if (MaxLen > 0)
    {
        for (Len = 0; Len < MaxLen; Len++)
        {
            if (String[Len] == 0)
            {
                break;
            }
        }
        BA = &String[Len - 1];
        while ((Len != 0) && ((*BA == ' ') || (*BA == 0)))
        {
            BA--;
            Len--;
        }
    }
    else
    {
        BA = String;
        while (*BA)
        {
            BA++;
            Len++;
        }
    }
    return Len;
}

void EditString(SMBIOS_STRUCTURE_POINTER table, SMBIOS_STRING *field, const CHAR8 *buffer)
{
    UpdateSmbiosString(table, field, buffer);
}

EFI_STATUS UpdateSmbiosString(OUT SMBIOS_STRUCTURE_POINTER SmbiosTableN, SMBIOS_STRING *Field, IN CONST CHAR8 *Buffer)
{
    CHAR8 *AString;
    UINTN ALength, BLength;
    UINT8 IndexStr = 1;

    if ((SmbiosTableN.Raw == NULL) || !Buffer || !Field)
    {
        return EFI_NOT_FOUND;
    }

    // находим строку по индексу Field
    AString = (CHAR8 *)(SmbiosTableN.Raw + SmbiosTableN.Hdr->Length);
    while (IndexStr != *Field)
    {
        if (*AString) IndexStr++;
        while (*AString != 0) AString++;
        AString++;

        if (*AString == 0)
        {
            *Field = IndexStr;
            break;
        }
    }

    ALength = iStrLen(AString, 0);
    BLength = iStrLen(Buffer, MAX_OEM_STRING);

    Print(L"[DEBUG] Table type %d field %d\n", SmbiosTableN.Hdr->Type, *Field);
    Print(L"[DEBUG] String at: 0x%lx\n", (UINT64)(UINTN)AString);
    Print(L"[DEBUG] Old: '%a' (%d bytes)\n", AString, ALength);

    // Проверка: находимся ли мы в копии памяти?
    if (g_SmbiosCopy != NULL)
    {
        UINTN Offset = (UINTN)AString - (UINTN)g_SmbiosCopy;
        if (Offset < g_SmbiosSize)
        {
            Print(L"[INFO] Writing to SMBIOS copy (offset: 0x%x)\n", Offset);
        }
        else
        {
            Print(L"[WARN] String not in SMBIOS copy! This may fail.\n");
        }
    }

    // Тест записи
    CHAR8 TestByte = AString[0];
    AString[0] = 'T';
    if (AString[0] != 'T')
    {
        Print(L"[ERROR] Memory is READ-ONLY!\n");
        return EFI_WRITE_PROTECTED;
    }
    AString[0] = TestByte;

    // Копируем данные
    UINTN CopyLen = (BLength < ALength) ? BLength : ALength;
    CopyMem(AString, Buffer, CopyLen);

    if (BLength < ALength)
    {
        for (UINTN i = CopyLen; i < ALength; i++)
        {
            AString[i] = ' ';
        }
    }

    AString[ALength] = 0;

    Print(L"[DEBUG] Final: '%a'\n", AString);

    // Верификация
    if (SimpleStrnCmp(AString, Buffer, CopyLen) != 0)
    {
        Print(L"[ERROR] Verification failed!\n");
        return EFI_DEVICE_ERROR;
    }

    Print(L"[SUCCESS] String updated\n");
    return EFI_SUCCESS;
}
