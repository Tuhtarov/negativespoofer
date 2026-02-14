#include "general.h"
#include "finder.h"
#include "smbios.h"
#include <string.h> 

EFI_GUID Smbios3TableGuid = { 0xF2FD1544, 0x9794, 0x4A2C, { 0x99, 0x2E, 0xE5, 0xBB, 0xCF, 0x20, 0xE3, 0x94 } };

void* FindByConfig() 
{
    EFI_PHYSICAL_ADDRESS* table;

    EFI_STATUS status = LibGetSystemConfigurationTable(&Smbios3TableGuid, (void**)&table);
    if (status == EFI_SUCCESS)
        return table;

    return 0;
}

SMBIOS3_STRUCTURE_TABLE* FindEntry() 
{
    SMBIOS3_STRUCTURE_TABLE* address = FindByConfig();
    if (address) 
        return address;
         
    return 0;
}