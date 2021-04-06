#ifndef PCILIB_H
#define PCILIB_H

#ifdef CODE_WARRIOR_VSN
#include <PciDefine.h>
#else
#include <PCI/PciDefine.h>
#endif

unsigned int PciDeviceID(PciBoard board);
char PciDeviceFind(unsigned short vid, unsigned short did, int index, PciBoard board);
Ulong *PciDeviceAdrs(PciBoard board, int reg);

#endif /* PCILIB_H */
