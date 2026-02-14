#include "finder.h"
#include "general.h"
#include "patch.h"
#include "smbios.h"
#include "smbios_utils.h"

// Объявление функций из smbios.c
extern UINT8 *g_SmbiosCopy;
extern UINTN g_SmbiosSize;
extern EFI_STATUS CreateWritableSmbiosCopy(SMBIOS3_STRUCTURE_TABLE *SmbiosPoint);
extern EFI_STATUS UpdateSmbiosEntryPoint(SMBIOS3_STRUCTURE_TABLE *SmbiosPoint);

// Диагностический режим - детальная информация о SMBIOS
VOID DiagnosticMode(SMBIOS3_STRUCTURE_TABLE *smbiosEntry)
{
    Print(L"\n");
    Print(L"╔════════════════════════════════════════╗\n");
    Print(L"║   SMBIOS v3 DIAGNOSTIC MODE            ║\n");
    Print(L"╚════════════════════════════════════════╝\n");

    // Информация о Entry Point
    DumpSmbiosEntryInfo(smbiosEntry);

    // Список всех таблиц
    Print(L"[DIAG] Enumerating all SMBIOS tables...\n");
    DumpAllSmbiosTables(smbiosEntry);

    // Детальная проверка Type 0
    Print(L"[DIAG] Detailed check of Type 0 table...\n");
    SMBIOS_STRUCTURE_POINTER type0 = FindTableByType(smbiosEntry, SMBIOS_TYPE_BIOS_INFORMATION, 0);
    if (type0.Type0)
    {
        Print(L"Type 0 found at 0x%lx\n", (UINT64)(UINTN)type0.Raw);
        DumpSmbiosTableStrings(type0);
    }
}

// Режим тестирования записи
VOID TestWriteMode(SMBIOS3_STRUCTURE_TABLE *smbiosEntry)
{
    Print(L"\n");
    Print(L"╔════════════════════════════════════════╗\n");
    Print(L"║   WRITE TEST MODE                      ║\n");
    Print(L"╚════════════════════════════════════════╝\n");

    Print(L"[TEST] Testing memory write capabilities...\n\n");

    // Тест 1: Проверка записи в оригинальные таблицы
    Print(L"Test 1: Original SMBIOS tables\n");
    SMBIOS_STRUCTURE_POINTER type1 = FindTableByType(smbiosEntry, SMBIOS_TYPE_SYSTEM_INFORMATION, 0);

    if (type1.Type1)
    {
        CHAR8 *FirstString = (CHAR8 *)(type1.Raw + type1.Hdr->Length);
        Print(L"  Address: 0x%lx\n", (UINT64)(UINTN)FirstString);
        Print(L"  Current: '%a'\n", FirstString);
        Print(L"  Writable: %s\n\n", TestMemoryWritable(FirstString) ? L"YES" : L"NO");
    }

    // Тест 2: Создание копии и проверка
    Print(L"Test 2: Creating writable copy\n");
    EFI_STATUS Status = CreateWritableSmbiosCopy(smbiosEntry);

    if (!EFI_ERROR(Status))
    {
        Print(L"  Copy created: 0x%lx\n", (UINT64)(UINTN)g_SmbiosCopy);
        Print(L"  Size: %d bytes\n", g_SmbiosSize);
        Print(L"  Writable: %s\n\n", TestMemoryWritable(g_SmbiosCopy) ? L"YES" : L"NO");

        // Тест 3: Сравнение оригинала и копии
        Print(L"Test 3: Comparing original and copy\n");
        BOOLEAN Same = CompareSmbiosTables((UINT8 *)(UINTN)smbiosEntry->TableAddress, g_SmbiosCopy, g_SmbiosSize);
        Print(L"  Tables identical: %s\n\n", Same ? L"YES" : L"NO");

        // Тест 4: Пробная запись в копию
        Print(L"Test 4: Test write to copy\n");
        UpdateSmbiosEntryPoint(smbiosEntry);

        SMBIOS_STRUCTURE_POINTER type1_copy = FindTableByType(smbiosEntry, SMBIOS_TYPE_SYSTEM_INFORMATION, 0);

        if (type1_copy.Type1)
        {
            CHAR8 TestString[] = "TEST_WRITE_123";
            Print(L"  Attempting to write: '%a'\n", TestString);

            Status = UpdateSmbiosString(type1_copy, &type1_copy.Type1->Manufacturer, TestString);

            if (!EFI_ERROR(Status))
            {
                Print(L"  Write status: SUCCESS\n");

                // Проверка
                CHAR8 *CheckString = (CHAR8 *)(type1_copy.Raw + type1_copy.Hdr->Length);
                Print(L"  Verification: '%a'\n", CheckString);

                BOOLEAN WriteSuccess = (SimpleStrnCmp(CheckString, TestString, 10) == 0);
                Print(L"  Result: %s\n", WriteSuccess ? L"SUCCESS" : L"FAILED");
            }
            else
            {
                Print(L"  Write status: FAILED (%r)\n", Status);
            }
        }
    }
    else
    {
        Print(L"  Failed to create copy: %r\n", Status);
    }
}

// Основная функция с выбором режима
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;

    InitializeLib(ImageHandle, SystemTable);

    Print(L"\n");
    Print(L"╔════════════════════════════════════════╗\n");
    Print(L"║   SMBIOS v3 Patcher with Diagnostics   ║\n");
    Print(L"╚════════════════════════════════════════╝\n");
    Print(L"\n");

    Print(L"[WORK] Searching for SMBIOS table entry...\n");
    SMBIOS3_STRUCTURE_TABLE *smbiosEntry = FindEntry();
    if (!smbiosEntry)
    {
        Print(L"[FAIL] Failed to locate SMBIOS table entry\n");
        return EFI_NOT_FOUND;
    }
    Print(L"[INFO] SMBIOS Entry Point: 0x%lx\n", (UINT64)(UINTN)smbiosEntry);
    Print(L"[INFO] SMBIOS Tables: 0x%lx\n", (UINT64)(UINTN)smbiosEntry->TableAddress);
    Print(L"[INFO] Version: %d.%d.%d\n", smbiosEntry->MajorVersion, smbiosEntry->MinorVersion, smbiosEntry->DocRev);

    Print(L"\n");
    Print(L"Select mode:\n");
    Print(L"  [D] Diagnostic - Show detailed SMBIOS info\n");
    Print(L"  [T] Test Write - Test memory write capabilities\n");
    Print(L"  [P] Patch - Apply SMBIOS patches\n");
    Print(L"  [Q] Quit\n");
    Print(L"\nYour choice: ");

    // Ждём нажатия клавиши
    while (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key) == EFI_NOT_READY)
        ;

    Print(L"%c\n\n", Key.UnicodeChar);

    switch (Key.UnicodeChar)
    {
    case L'D':
    case L'd':
        DiagnosticMode(smbiosEntry);
        break;

    case L'T':
    case L't':
        TestWriteMode(smbiosEntry);
        break;

    case L'P':
    case L'p':
        Print(L"[WORK] Creating writable SMBIOS copy...\n");
        Status = CreateWritableSmbiosCopy(smbiosEntry);
        if (EFI_ERROR(Status))
        {
            Print(L"[FAIL] Failed to create copy: %r\n", Status);
            return Status;
        }

        Print(L"[WORK] Updating Entry Point...\n");
        Status = UpdateSmbiosEntryPoint(smbiosEntry);
        if (EFI_ERROR(Status))
        {
            Print(L"[FAIL] Failed to update entry point: %r\n", Status);
            return Status;
        }

        // Обновляем контрольную сумму
        UpdateSmbios3Checksum(smbiosEntry);

        Print(L"\n[WORK] Starting SMBIOS patching...\n");
        PatchAll(smbiosEntry);

        Print(L"\n[SUCCESS] All patches applied!\n");
        Print(L"[INFO] Modified tables at: 0x%lx\n", (UINT64)(UINTN)smbiosEntry->TableAddress);
        break;

    case L'Q':
    case L'q':
        Print(L"Exiting...\n");
        return EFI_SUCCESS;

    default:
        Print(L"Invalid choice\n");
        return EFI_INVALID_PARAMETER;
    }

    Print(L"\n");
    Print(L"Press any key to exit...\n");
    while (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key) == EFI_NOT_READY)
        ;

    return EFI_SUCCESS;
}