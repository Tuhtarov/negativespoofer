#include "patch.h"
#include "general.h"
#include "smbios.h"
#include "utils.h"

void EditRandom(SMBIOS_STRUCTURE_POINTER table, SMBIOS_STRING *field)
{
    CHAR8 buffer[258];
    RandomText(buffer, 257);

    if (field)
    {
        EditString(table, field, buffer);
    }
}

void PatchType0(SMBIOS3_STRUCTURE_TABLE *entry)
{
    SMBIOS_STRUCTURE_POINTER table = FindTableByType(entry, SMBIOS_TYPE_BIOS_INFORMATION, 0);
    Print(L"[WORK] Patching type0 table at 0x%08x...\n", table);

    if (!table.Type0)
    {
        Print(L"[FAIL] Table is non existent\n");
        return;
    }

    CHAR8 vendor[128];
    CHAR8 version[128];
    CHAR8 date[128];

    GenerateVendor(vendor, 128);
    GenerateBiosVersion(version, 128);
    GenerateReleaseDate(date, 128);

    EditString(table, &table.Type0->Vendor, vendor);
    EditString(table, &table.Type0->BiosVersion, version);
    EditString(table, &table.Type0->BiosReleaseDate, date);

    Print(L"[INFO] Patched type0 table\n");
    Print(L"\n\n");
}

void PatchType1(SMBIOS3_STRUCTURE_TABLE *entry)
{
    SMBIOS_STRUCTURE_POINTER table = FindTableByType(entry, SMBIOS_TYPE_SYSTEM_INFORMATION, 0);
    Print(L"[WORK] Patching type1 table at 0x%08x...\n", table);

    if (!table.Type1)
    {
        Print(L"[FAIL] Table is non existent\n");
        return;
    }

    CHAR8 mfg[128];
    CHAR8 product[128];
    CHAR8 version[128];
    CHAR8 serial[128];

    GenerateVendor(mfg, 128);
    GenerateProduct(product, 128);
    GenerateBiosVersion(version, 128);
    GenerateSerialNumber(serial, 128);

    EditString(table, &table.Type1->Manufacturer, mfg);
    EditString(table, &table.Type1->ProductName, product);
    EditString(table, &table.Type1->Version, version);
    EditString(table, &table.Type1->SerialNumber, serial);

    Print(L"[INFO] Patched type1 table\n");
    Print(L"\n\n");
}

void PatchType2(SMBIOS3_STRUCTURE_TABLE *entry)
{
    SMBIOS_STRUCTURE_POINTER table = FindTableByType(entry, SMBIOS_TYPE_BASEBOARD_INFORMATION, 0);
    Print(L"[WORK] Patching type2 table at 0x%08x...\n", table);

    if (!table.Type2)
    {
        Print(L"[FAIL] Table is non existent\n");
        return;
    }

    CHAR8 mfg[128];
    CHAR8 product[128];
    CHAR8 version[128];
    CHAR8 serial[128];

    GenerateVendor(mfg, 128);
    GenerateProduct(product, 128);
    GenerateBiosVersion(version, 128);
    GenerateSerialNumber(serial, 128);

    EditString(table, &table.Type2->Manufacturer, mfg);
    EditString(table, &table.Type2->ProductName, product);
    EditString(table, &table.Type2->Version, version);
    EditString(table, &table.Type2->SerialNumber, serial);

    Print(L"[INFO] Patched type2 table\n");
    Print(L"\n\n");
}

void PatchType3(SMBIOS3_STRUCTURE_TABLE *entry)
{
    SMBIOS_STRUCTURE_POINTER table = FindTableByType(entry, SMBIOS_TYPE_SYSTEM_ENCLOSURE, 0);
    Print(L"[WORK] Patching type3 table at 0x%08x...\n", table);

    if (!table.Type3)
    {
        Print(L"[FAIL] Table is non existent\n");
        return;
    }

    CHAR8 mfg[128];
    CHAR8 version[128];
    CHAR8 serial[128];
    CHAR8 tag[128];

    GenerateVendor(mfg, 128);
    GenerateBiosVersion(version, 128);
    GenerateSerialNumber(serial, 128);
    GenerateAssetTag(tag, 128);

    EditString(table, &table.Type3->Manufacturer, mfg);
    EditString(table, &table.Type3->Version, version);
    EditString(table, &table.Type3->SerialNumber, serial);
    EditString(table, &table.Type3->AssetTag, tag);

    Print(L"[INFO] Patched type3 table\n");
    Print(L"\n\n");
}

void PatchAll(SMBIOS3_STRUCTURE_TABLE *entry)
{
    PatchType0(entry);
    PatchType1(entry);
    PatchType2(entry);
    PatchType3(entry);
}