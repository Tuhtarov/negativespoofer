#include "general.h"
#include "utils.h"

UINT32 g_originalSeed = 0;
UINT32 g_currentSeed = 0;
BOOLEAN g_seedSet = FALSE;

// Global vendor list for realistic BIOS vendors
static const CHAR8 *g_vendors[] = {
    (const CHAR8 *)"American Megatrends Inc.",
    (const CHAR8 *)"Phoenix Technologies Ltd.",
    (const CHAR8 *)"Insyde Software",
    (const CHAR8 *)"Coreboot",
    (const CHAR8 *)"Dell Inc.",
    (const CHAR8 *)"HP Inc.",
    (const CHAR8 *)"Lenovo Corporation",
    (const CHAR8 *)"ASUS"
};

// BIOS version patterns
static const CHAR8 *g_biosPrefixes[] = {
    (const CHAR8 *)"FX", (const CHAR8 *)"MX", (const CHAR8 *)"RX", (const CHAR8 *)"EX", (const CHAR8 *)"ZX", (const CHAR8 *)"AX", (const CHAR8 *)"BX"
};

// Manufacturers
static const CHAR8 *g_manufacturers[] = {
    (const CHAR8 *)"DELL", (const CHAR8 *)"HP", (const CHAR8 *)"Lenovo", (const CHAR8 *)"ASUS", (const CHAR8 *)"MSI", (const CHAR8 *)"Gigabyte", (const CHAR8 *)"ASRock", (const CHAR8 *)"Acer"
};

// Product name patterns
__attribute__((unused))
static const CHAR8 *g_productSuffixes[] = {
    (const CHAR8 *)"5560", (const CHAR8 *)"5580", (const CHAR8 *)"7590", (const CHAR8 *)"G3", (const CHAR8 *)"G7", (const CHAR8 *)"Yoga", (const CHAR8 *)"ThinkPad", (const CHAR8 *)"ROG"
};

void SetRandomSeed(UINT32 seed) 
{
    g_originalSeed = seed;
    g_currentSeed = seed;
    g_seedSet = TRUE;
}

UINT32 GetRandomSeed(void) 
{
    return g_originalSeed;
}

int RandomNumber(int l, int h) 
{
    if (!g_seedSet) 
    {
        EFI_TIME time;
        EFI_TIME_CAPABILITIES cap;
        gRT->GetTime(&time, &cap);
        g_currentSeed = time.Day + time.Hour + time.Minute + time.Second + time.Nanosecond;
        g_seedSet = TRUE;
    }

    // Linear congruential generator
    g_currentSeed = (g_currentSeed * 1103515245 + 12345) & 0x7fffffff;
    
    int num = (g_currentSeed >> 16) % (h - l + 1);
    return num + l;
}

void RandomText(CHAR8* s, const int len) 
{
    for (int i = 0; i < len; i++) 
    {
        s[i] = RandomNumber(49, 90);
    }

    s[len] = 0;
}

static void CopyStr(CHAR8 *dest, const CHAR8 *src, int maxLen) 
{
    int i = 0;
    while (src[i] && i < maxLen - 1) 
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
}

static void AppendStr(CHAR8 *dest, const CHAR8 *src, int maxLen) 
{
    int i = 0;
    while (dest[i]) i++;
    
    while (src[0] && i < maxLen - 1) 
    {
        dest[i] = src[0];
        src++;
        i++;
    }
    dest[i] = 0;
}

static void AppendNum(CHAR8 *dest, int num, int maxLen) 
{
    CHAR8 buffer[16];
    int i = 0;
    int temp = num;
    
    if (num == 0) 
    {
        buffer[i++] = '0';
    } 
    else 
    {
        while (temp > 0) 
        {
            buffer[i++] = (temp % 10) + '0';
            temp /= 10;
        }
        // Reverse
        for (int j = 0; j < i / 2; j++) 
        {
            CHAR8 t = buffer[j];
            buffer[j] = buffer[i - 1 - j];
            buffer[i - 1 - j] = t;
        }
    }
    buffer[i] = 0;
    AppendStr(dest, buffer, maxLen);
}

void GenerateVendor(CHAR8 *s, const int len) 
{
    int idx = RandomNumber(0, 7);
    CopyStr(s, g_vendors[idx], len);
}

void GenerateBiosVersion(CHAR8 *s, const int len) 
{
    // Format: XXnnn.nn (e.g., FX705DT.316)
    CopyStr(s, g_biosPrefixes[RandomNumber(0, 6)], len);
    AppendNum(s, RandomNumber(100, 999), len);
    AppendNum(s, RandomNumber(100, 999), len);
}

void GenerateReleaseDate(CHAR8 *s, const int len) 
{
    // Format: MM/DD/YYYY (e.g., 01/28/2021)
    int month = RandomNumber(1, 12);
    int day = RandomNumber(1, 28);
    int year = RandomNumber(2015, 2023);
    
    if (month < 10) AppendStr(s, (const CHAR8 *)"0", len);
    AppendNum(s, month, len);
    AppendStr(s, (const CHAR8 *)"/", len);
    
    if (day < 10) AppendStr(s, (const CHAR8 *)"0", len);
    AppendNum(s, day, len);
    AppendStr(s, (const CHAR8 *)"/", len);
    AppendNum(s, year, len);
}

void GenerateProduct(CHAR8 *s, const int len) 
{
    // Format: MANUFACTURER DIGIT-SUFFIX (e.g., DELL XPS 15)
    CopyStr(s, g_manufacturers[RandomNumber(0, 7)], len);
    AppendStr(s, (const CHAR8 *)" ", len);
    
    switch(RandomNumber(0, 2)) 
    {
        case 0:
            AppendStr(s, (const CHAR8 *)"XPS ", len);
            break;
        case 1:
            AppendStr(s, (const CHAR8 *)"Latitude ", len);
            break;
        default:
            AppendStr(s, (const CHAR8 *)"Precision ", len);
            break;
    }
    AppendNum(s, RandomNumber(13, 17), len);
}

void GenerateSerialNumber(CHAR8 *s, const int len) 
{
    // Format: Realistic serial number like L7235NRCV992NH2MD (17+ chars)
    const CHAR8 *letters = (const CHAR8 *)"ABCDEFGHJKLMNPQRSTVWXYZ";
    const CHAR8 *numbers = (const CHAR8 *)"0123456789";
    int letterCount = 23;
    int numberCount = 10;
    
    int pos = 0;
    int maxChars = (len > 17) ? len - 1 : 16;
    
    // Start with 1-2 letters
    s[pos++] = letters[RandomNumber(0, letterCount - 1)];
    if (RandomNumber(0, 1) == 0 && pos < maxChars) {
        s[pos++] = letters[RandomNumber(0, letterCount - 1)];
    }
    
    // Add 4-5 numbers
    for (int i = 0; i < 5 && pos < maxChars; i++) {
        s[pos++] = numbers[RandomNumber(0, numberCount - 1)];
    }
    
    // Add 2-3 letters
    for (int i = 0; i < 3 && pos < maxChars; i++) {
        s[pos++] = letters[RandomNumber(0, letterCount - 1)];
    }
    
    // Add 3-4 numbers
    for (int i = 0; i < 4 && pos < maxChars; i++) {
        s[pos++] = numbers[RandomNumber(0, numberCount - 1)];
    }
    
    // Add 2-3 letters
    for (int i = 0; i < 3 && pos < maxChars; i++) {
        s[pos++] = letters[RandomNumber(0, letterCount - 1)];
    }
    
    // Add 1-2 more numbers
    for (int i = 0; i < 2 && pos < maxChars; i++) {
        s[pos++] = numbers[RandomNumber(0, numberCount - 1)];
    }
    
    s[pos] = 0;
}

void GenerateAssetTag(CHAR8 *s, const int len) 
{
    // Format: TAG-XXXXXXXX
    CopyStr(s, (const CHAR8 *)"TAG-", len);
    const CHAR8 *chars = (const CHAR8 *)"ABCDEFGHJKLMNPQRSTVWXYZ0123456789";
    int charCount = 34;
    
    int baseLen = 4;
    for (int i = baseLen; i < ((len > 12) ? 12 : len - 1); i++) 
    {
        s[i] = chars[RandomNumber(0, charCount - 1)];
    }
    s[((len > 12) ? 12 : len - 1)] = 0;
}
