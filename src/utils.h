#ifndef UTILS_H
#define UTILS_H

// Seed management
void SetRandomSeed(UINT32 seed);
UINT32 GetRandomSeed(void);

// Random number generation
int RandomNumber(int l, int h);
void RandomText(CHAR8 *s, const int len);

// Realistic SMBIOS string generators
void GenerateVendor(CHAR8 *s, const int len);
void GenerateBiosVersion(CHAR8 *s, const int len);
void GenerateReleaseDate(CHAR8 *s, const int len);
void GenerateProduct(CHAR8 *s, const int len);
void GenerateSerialNumber(CHAR8 *s, const int len);
void GenerateAssetTag(CHAR8 *s, const int len);

#endif