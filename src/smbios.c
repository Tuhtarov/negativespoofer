#include "smbios.h"
#include "general.h"
#include <string.h>

#define MAX_OEM_STRING 256

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
    Length = (UINT16)((UINTN)AChar - (UINTN)SmbiosTableN.Raw + 2); // length includes 00
    return Length;
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

UINTN SpaceLength(const char *text, UINTN maxLength)
{
    UINTN lenght = 0;
    const char *ba;

    if (maxLength > 0)
    {
        for (lenght = 0; lenght < maxLength; lenght++)
        {
            if (text[lenght] == 0)
            {
                break;
            }
        }

        ba = &text[lenght - 1];

        while ((lenght != 0) && ((*ba == ' ') || (*ba == 0)))
        {
            ba--;
            lenght--;
        }
    }
    else
    {
        ba = text;
        while (*ba)
        {
            ba++;
            lenght++;
        }
    }

    return lenght;
}

void EditString(SMBIOS_STRUCTURE_POINTER table, SMBIOS_STRING *field, const CHAR8 *buffer)
{
    UpdateSmbiosString(table, field, buffer);
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

EFI_STATUS UpdateSmbiosString(OUT SMBIOS_STRUCTURE_POINTER SmbiosTableN, SMBIOS_STRING *Field, IN CONST CHAR8 *Buffer)
{
    CHAR8 *AString;
    CHAR8 *C1; // pointers for copy
    CHAR8 *C2;
    UINTN Length = SmbiosTableLength(SmbiosTableN);
    UINTN ALength, BLength;
    UINT8 IndexStr = 1;

    if ((SmbiosTableN.Raw == NULL) || !Buffer || !Field)
    {
        return EFI_NOT_FOUND;
    }
    AString = (CHAR8 *)(SmbiosTableN.Raw + SmbiosTableN.Hdr->Length); // first string
    while (IndexStr != *Field)
    {
        if (*AString)
        {
            IndexStr++;
        }
        while (*AString != 0)
            AString++; // skip string at index
        AString++;     // next string
        if (*AString == 0)
        {
            // this is end of the table
            if (*Field == 0)
            {
                AString[1] = 0; // one more zero
            }
            *Field = IndexStr; // index of the next string that  is empty
            if (IndexStr == 1)
            {
                AString--; // first string has no leading zero
            }
            break;
        }
    }
    // AString is at place to copy
    ALength = iStrLen(AString, 0);
    BLength = iStrLen(Buffer, MAX_OEM_STRING);

    Print(L"Table type %d field %d\n", SmbiosTableN.Hdr->Type, *Field);
    Print(L"Old string: %a\n", AString);
    Print(L"New string: %a\n", Buffer);

    if (BLength > ALength)
    {
        // Shift right
        C1 = (CHAR8 *)SmbiosTableN.Raw + Length; // old end
        C2 = C1 + BLength - ALength;             // new end
        *C2 = 0;
        while (C1 != AString)
            *(--C2) = *(--C1);
    }
    else if (BLength < ALength)
    {
        // Shift left
        C1 = AString + ALength; // old start
        C2 = AString + BLength; // new start
        while (C1 != ((CHAR8 *)SmbiosTableN.Raw + Length))
        {
            *C2++ = *C1++;
        }
        *C2 = 0;
        *(--C2) = 0; // end of table
    }
    CopyMem(AString, Buffer, BLength);
    *(AString + BLength) = 0; // not sure there is 0

    return EFI_SUCCESS;
}
