#ifndef FINDER_H
#define FINDER_H

#include "smbios.h"

int CheckEntry(SMBIOS3_STRUCTURE_TABLE* entry);
void* FindBySignature();
void* FindByHob();
void* FindByConfig();
SMBIOS3_STRUCTURE_TABLE* FindEntry();

#endif