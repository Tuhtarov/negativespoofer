#ifndef SMBIOS_UTILS_H
#define SMBIOS_UTILS_H

#include "smbios.h"
#include "general.h"

// Вычислить контрольную сумму для SMBIOS Entry Point
UINT8 CalculateSmbiosChecksum(UINT8 *Buffer, UINTN Size)
{
    UINT8 Sum = 0;
    for (UINTN i = 0; i < Size; i++)
    {
        Sum += Buffer[i];
    }
    return (UINT8)(256 - Sum);
}

// Обновить контрольную сумму SMBIOS 3.x Entry Point
EFI_STATUS UpdateSmbios3Checksum(SMBIOS3_STRUCTURE_TABLE *Entry)
{
    if (!Entry)
        return EFI_INVALID_PARAMETER;
    
    // Проверяем сигнатуру (вручную сравниваем байты)
    if (Entry->AnchorString[0] != '_' || 
        Entry->AnchorString[1] != 'S' ||
        Entry->AnchorString[2] != 'M' ||
        Entry->AnchorString[3] != '3' ||
        Entry->AnchorString[4] != '_')
    {
        Print(L"[ERROR] Invalid SMBIOS 3.x signature\n");
        return EFI_INVALID_PARAMETER;
    }
    
    // Сбрасываем текущую контрольную сумму
    Entry->EntryPointStructureChecksum = 0;
    
    // Вычисляем новую
    Entry->EntryPointStructureChecksum = CalculateSmbiosChecksum(
        (UINT8 *)Entry, 
        Entry->EntryPointLength
    );
    
    Print(L"[INFO] Updated SMBIOS checksum: 0x%lx\n", (UINT64)(UINTN)Entry->EntryPointStructureChecksum);
    
    return EFI_SUCCESS;
}

// Проверить контрольную сумму
BOOLEAN VerifySmbios3Checksum(SMBIOS3_STRUCTURE_TABLE *Entry)
{
    UINT8 Sum = 0;
    UINT8 *Buffer = (UINT8 *)Entry;
    
    for (UINTN i = 0; i < Entry->EntryPointLength; i++)
    {
        Sum += Buffer[i];
    }
    
    return (Sum == 0);
}

// Вывести информацию о SMBIOS Entry Point
VOID DumpSmbiosEntryInfo(SMBIOS3_STRUCTURE_TABLE *Entry)
{
    Print(L"\n=== SMBIOS Entry Point Info ===\n");
    Print(L"Anchor String: %c%c%c%c%c\n", 
        Entry->AnchorString[0], Entry->AnchorString[1], 
        Entry->AnchorString[2], Entry->AnchorString[3],
        Entry->AnchorString[4]);
    Print(L"Entry Point Length: %d bytes\n", Entry->EntryPointLength);
    Print(L"Major Version: %d\n", Entry->MajorVersion);
    Print(L"Minor Version: %d\n", Entry->MinorVersion);
    Print(L"Docrev: %d\n", Entry->DocRev);
    Print(L"Entry Point Revision: 0x%lx\n", (UINT64)(UINTN)Entry->EntryPointRevision);
    Print(L"Table Max Size: %d bytes\n", Entry->TableMaximumSize);
    Print(L"Table Address: 0x%lx\n", (UINT64)(UINTN)Entry->TableAddress);
    Print(L"Checksum: 0x%lx ", (UINT64)(UINTN)Entry->EntryPointStructureChecksum);
    
    if (VerifySmbios3Checksum(Entry))
    {
        Print(L"[VALID]\n");
    }
    else
    {
        Print(L"[INVALID]\n");
    }
    Print(L"==============================\n\n");
}

// Вывести все строки SMBIOS таблицы
VOID DumpSmbiosTableStrings(SMBIOS_STRUCTURE_POINTER Table)
{
    CHAR8 *String;
    UINT8 Index = 1;
    
    Print(L"\n--- Strings for Table Type %d ---\n", Table.Hdr->Type);
    
    String = (CHAR8 *)(Table.Raw + Table.Hdr->Length);
    
    while (*String != 0)
    {
        Print(L"String %d: '%a'\n", Index, String);
        Index++;
        
        // Переход к следующей строке
        while (*String != 0) String++;
        String++;
    }
    
    if (Index == 1)
    {
        Print(L"(No strings)\n");
    }
    
    Print(L"-------------------------------\n\n");
}

// Вывести все SMBIOS таблицы
VOID DumpAllSmbiosTables(SMBIOS3_STRUCTURE_TABLE *SmbiosPoint)
{
    SMBIOS_STRUCTURE_POINTER Current;
    UINTN TableCount = 0;
    
    Print(L"\n====== All SMBIOS Tables ======\n");
    
    Current.Raw = (UINT8 *)((UINTN)SmbiosPoint->TableAddress);
    
    while (Current.Hdr->Type != SMBIOS_TYPE_END_OF_TABLE)
    {
        UINT16 TableLen = SmbiosTableLength(Current);
        
        Print(L"\nTable #%d:\n", TableCount);
        Print(L"  Type: %d\n", Current.Hdr->Type);
        Print(L"  Length: %d bytes\n", Current.Hdr->Length);
        Print(L"  Handle: 0x%lx\n", (UINT64)(UINTN)Current.Hdr->Handle);
        Print(L"  Total Size: %d bytes\n", TableLen);
        Print(L"  Address: 0x%lx\n", (UINT64)(UINTN)Current.Raw);
        
        DumpSmbiosTableStrings(Current);
        
        Current.Raw = Current.Raw + TableLen;
        TableCount++;
    }
    
    Print(L"\nTotal tables found: %d\n", TableCount);
    Print(L"===============================\n\n");
}

// Сравнить две области памяти
BOOLEAN CompareMemory(UINT8 *Mem1, UINT8 *Mem2, UINTN Size)
{
    for (UINTN i = 0; i < Size; i++)
    {
        if (Mem1[i] != Mem2[i])
        {
            return FALSE;
        }
    }
    return TRUE;
}

// Сравнить две SMBIOS таблицы
BOOLEAN CompareSmbiosTables(UINT8 *Table1, UINT8 *Table2, UINTN Size)
{
    for (UINTN i = 0; i < Size; i++)
    {
        if (Table1[i] != Table2[i])
        {
            Print(L"[DIFF] Offset 0x%x: 0x%02x != 0x%02x\n", i, Table1[i], Table2[i]);
            return FALSE;
        }
    }
    return TRUE;
}

// Проверить доступность памяти для записи
BOOLEAN TestMemoryWritable(VOID *Address)
{
    volatile UINT8 *TestAddr = (volatile UINT8 *)Address;
    UINT8 Original = *TestAddr;
    
    // Попытка записи
    *TestAddr = ~Original;
    
    // Проверка
    BOOLEAN Writable = (*TestAddr == (UINT8)~Original);
    
    // Восстановление
    *TestAddr = Original;
    
    return Writable;
}

#endif // SMBIOS_UTILS_H
