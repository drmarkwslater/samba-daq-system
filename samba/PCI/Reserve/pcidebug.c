// #define VME// #define VXWORKS/* Fichiers VxWorks dans /export/home/sigx9/TORNADO/target/h */#include <stdlib.h>#include <stdio.h>#include <string.h>    /* pour strxxx */#ifdef VXWORKS#include <globals.h>#include <ioLib.h>     /* pour ioctl */#include <sysLib.h>#include <dllLib.h>#include <vxLib.h>     /* pour vxMemProbe *//* #include <drv/pci/pciConfigLib.h> *//* #include <pciMapLib.h> double-emploi avec le precedent */#include <vxWorks.h>   /* pour PciInit, ainsi que les suivants */#include <dllLib.h>char *sysModel(),*sysBspRev();#include "pciMapLib.h"#include "config.h" /* voir commentaire suivant, extrait de pciIomapLib.c *//* * The following defines specify, by default, the maximum number of busses, * devices and functions allowed by the PCI 2.1 Specification. * * Any or all may be overriden by defining them in config.h. */typedef struct {	unsigned char b;  /* Bus      */	unsigned char d;  /* Device   */	unsigned char f;  /* Function */} TypePCIboard,*PCIboard;#define main pcidebug#endif#ifdef macintosh#include <NameRegistry.h>#include <PCI.h>#include <PciOffsets.h>#include <Memory.h>#define ERROR -1typedef RegEntryID TypePCIboard,*PCIboard;typedef UInt32 UINT32;char *PCIname[] = {	"Devices:device-tree:pci",	"Devices:device-tree:pci:pci-bridge",	"Devices:device-tree:bandit",/*	"Root:PowerMac3,1:Core99PE:pci:AppleMacRiscPCI:pci-bridge:IOPCI2PCIBridge", */	"\0"};#endif// PciConfigReadByte(PCIboard b, void *adrs, unsigned char *val)#ifdef VXWORKS#define PciConfigReadByte(b,adrs,val) pciConfigInByte(b->b,b->d,b->f,adrs,val)#define PciConfigReadWord(b,adrs,val) pciConfigInWord(b->b,b->d,b->f,adrs,val)#define PciConfigReadLong(b,adrs,val) pciConfigInLong(b->b,b->d,b->f,adrs,val)#define PciConfigWriteByte(b,adrs,val) pciConfigOutByte(b->b,b->d,b->f,adrs,val)#define PciConfigWriteWord(b,adrs,val) pciConfigOutWord(b->b,b->d,b->f,adrs,val)#define PciConfigWriteLong(b,adrs,val) pciConfigOutLong(b->b,b->d,b->f,adrs,val)#endif#ifdef macintosh#define PciConfigReadByte(b,adrs,val) ExpMgrConfigReadByte(b,(void *)(adrs),(UInt8 *)val)#define PciConfigReadWord(b,adrs,val) ExpMgrConfigReadWord(b,(void *)(adrs),(UInt16 *)val)#define PciConfigReadLong(b,adrs,val) ExpMgrConfigReadLong(b,(void *)(adrs),(UInt32 *)val)#define PciConfigWriteByte(b,adrs,val) ExpMgrConfigWriteByte(b,(void *)(adrs),(UInt8)val)#define PciConfigWriteWord(b,adrs,val) ExpMgrConfigWriteWord(b,(void *)(adrs),(UInt16)val)#define PciConfigWriteLong(b,adrs,val) ExpMgrConfigWriteLong(b,(void *)(adrs),(UInt32)val)#endif#ifndef PCI_MAX_BUS#  define PCI_MAX_BUS	256#endif#ifndef PCI_MAX_DEV#  define PCI_MAX_DEV	21#endif#ifndef PCI_MAX_FUNC#  define PCI_MAX_FUNC	8#endif#ifdef mv2604#define BOARD_TYPE "mv2604"#endif#ifdef mv2700#define BOARD_TYPE "mv2700"#endif#define EXTENDER_VID 0x1011/* offset pour la definition des bus dans le CSH du bridge */#define EXTENDER_BUS_DEFINITION 0x18enum {  PCI_CLASS_OLD = 0,  PCI_CLASS_DISK,  PCI_CLASS_NET,  PCI_CLASS_DPLY,  PCI_CLASS_MMED,  PCI_CLASS_MEM,  PCI_CLASS_BRDG,  PCI_CLASS_COMM,  PCI_CLASS_SYST,  PCI_CLASS_INPT,  PCI_CLASS_DOCK,  PCI_CLASS_PROC,  PCI_CLASS_SRAL,  PCI_CLASS_OTHR,  PCI_NB_CLASSES};char *Classe[PCI_NB_CLASSES] = {  "old",  "disk",  "net",  "dply",  "mmed",  "mem",  "brdg",  "comm",  "syst",  "inpt",  "dock",  "proc",  "sral",  "othr"};char *TypeDevice[PCI_NB_CLASSES] = {  "Ancien device",  "Disque",  "Lien reseau",  "Display",  "Multimedia",  "Memoire",  "Bridge",  "Communication",  "Systeme",  "Input",  "Docking",  "Processeur",  "E/S serie",  "Divers"};#define PCI_NB_DISKS 6char *Disque[PCI_NB_DISKS] = {  "SCSI",  "IDE",  "floppy",  "IPI",  "RAID",  "divers"};#define PCI_NB_NETS 5char *Net[PCI_NB_NETS] = {  "Ethernet",  "Token Ring",  "FDDI",  "ATM",  "divers"};#define PCI_NB_BRIDGES 9char *Bridge[PCI_NB_BRIDGES] = {  "Host/PCI",  "PCI/ISA",  "PCI/EISA",  "PCI/MicroChannel",  "PCI/PCI",  "PCI/PCMCIA",  "PCI/NuBus",  "PCI/CardBus",  "divers"};#define PCI_NB_INPUTS 4char *InputDev[PCI_NB_INPUTS] = {  "keyboard",  "digitizer (pen)",  "mouse",  "divers"};#define PCI_NB_SERIALS 5char *Serial[PCI_NB_SERIALS] = {  "FireWire",  "ACCESSbus",  "SSA",  "USB",  "FibreChannel"};#define KB 1024#define MB (KB * KB)char PciOpened = 0,Log;int *Allouee; int NbAlloues;int Ram_vid = 0x11B0, Ram_did = 0x0200, Ram_index = 0; /* ========================================================================== */void debughelp() {  printf("pciread   board,adrs\n");  printf("pciwrite  board,adrs,val\n");  printf("pcidump   board,check_mem\n");#ifdef VME  printf("pciscan   entendu,check_memoire\n");#endif  printf("raminit   baseadrs,log\n");  printf("ramread   adrs\n");  printf("ramwrite  adrs,valeur\n");#ifdef VXWORKS  printf("memread   adrs\n");  printf("memwrite  adrs,valeur\n");  printf("memcheck  baseadrs,maxi,relit,log\n");#endif}/* ========================================================================== */int PciDeviceFind(unsigned short vid, unsigned short did, int index, PCIboard board) {#ifdef VXWORKS	int rc;	int b,d,f;	rc = pciFindDevice(vid,did,index,&b,&d,&f);	board->b = b; board->d = d; board->f = f;	return(rc);#endif#ifdef macintosh	char nom[256];	int type;	type = 0;	while(PCIname[type][0]) {		sprintf(nom,"%s:pci%04x,%04x",PCIname[type],vid,did);		if(RegistryCStrEntryLookup(NULL,nom,board) == noErr) return(0);		type++;	}	return(-1);#endif}/* ========================================================================== */int PciInit(char bavard) {#ifdef VME  int k; char avec_extenseur;  TypePCIboard b;#endif#ifdef VXWORKS  UINT32 vendor;#endif  if(PciOpened) return(1);  if(bavard) {    printf("===============================================================================\n");    printf("Initialisation du bus PCI\n\n");#ifdef VXWORKS    printf("%s - BSP %s ",sysModel(),sysBspRev());`#endif  }  #ifdef VME  vendor = EXTENDER_VID; avec_extenseur = 1;  /* voit-on le bridge 'PCI-PCI vers extenseur a 2 PMC' ?? */  if(PciDeviceFind(vendor,0x22,0,&b) == ERROR) {    /* voit-on le bridge 'PCI-PCI vers extenseur a 3 PMC' ?? */    if(PciDeviceFind(vendor,0x25,0,&b) == ERROR) {      avec_extenseur = 0; /* finalement, pas d'extenseur visible */      if(bavard) printf("sans extenseur\n");    } else if(bavard) printf("avec extenseur a 3 PMC\n");  } else if(bavard) printf("avec extenseur a 2 PMC\n");  if(avec_extenseur) {    PciConfigWriteLong(b,PCI_CFG_PRIMARY_BUS,0x00010100);    PciConfigWriteLong(b,PCI_CFG_MEM_BASE,0x7FF01000);/*     PciConfigWriteLong(b,PCI_CFG_MEM_BASE,0xFDFFFD00); *//*     PciConfigWriteLong(b,PCI_CFG_PRE_MEM_BASE,0x7FF01000); */    PciConfigWriteLong(b,PCI_CFG_PRE_MEM_BASE,0xFFFFFFFF);/*     PciConfigWriteLong(b,PCI_CFG_PRE_MEM_BASE_U,0); *//*     PciConfigWriteLong(b,PCI_CFG_PRE_MEM_LIMIT_U,0); */         /* nous faisons            FAST=0 disable Fast BackToBack (bizarrement, ce bit est read only)            SERR=1 enable system error            PERR=1 enable parity error            MSTR=1 enable bus mastering            MEMSP=1 enable memory access            ce qui donne la commande 0x146 */    PciConfigWriteLong(b,PCI_CFG_COMMAND,0x146);    if(bavard) {      printf("Bus #%d, Device #%d, Fonction #%d\n",b,d,f);      PciConfigReadLong(b,PCI_CFG_COMMAND,&k);      printf("Command: %04X, Status: %04X\n",	     k & 0xFFFF,(k >> 16) & 0xFFFF);      PciConfigReadLong(b,PCI_CFG_MEM_BASE,&k);      printf("Memory: %08X (Start: %04X0000, End: %04XFFFF)\n",	     k,(k & 0xFFF0),((k >> 16) & 0xFFF0) | 0xF);      PciConfigReadLong(b,PCI_CFG_PRE_MEM_BASE,&k);      printf("Prefetechable Memory: %08X (Start: %04X0000, End: %04XFFFF)\n",	     k,(k & 0xFFF0),((k >> 16) & 0xFFF0) | 0xF);    }  }#endif  PciOpened = 1;  return(1);}/* ========================================================================== */unsigned int pciread(PCIboard b, void *adrs) {  unsigned int val;  PciConfigReadLong(b,adrs,&val);#ifdef VXWORKS  printf("%d.%02d.%d @%02X -> %08X\n",b->b,b->d,b->f,adrs,val);#endif#ifdef macintosh  printf(" -> %08X\n",adrs,val);#endif  return(val);}/* ========================================================================== */unsigned int pciwrite(PCIboard b, void *adrs, unsigned int val) {  PciConfigWriteLong(b,adrs,val);#ifdef VXWORKS  printf("%d.%02d.%d @%02X <- %08X\n",b->b,b->d,b->f,adrs,val);#endif#ifdef macintosh  printf(" <- %08X\n",adrs,val);#endif  return(val);}/* ========================================================================== */unsigned int pcidump(PCIboard b, char memoire) {  unsigned int val,did,mem,cmd,msk;  char vendeur[32];  int id,classe;  int i,k;  PciInit(0);#ifdef VXWORKS  printf(".................................. (Bus #%d) ...................................\n",b->b);  printf("%02d (%d): ",b->d,b->f);#endif  PciConfigReadLong(b,PCI_CFG_VENDOR_ID,&did);  if(did == 0xFFFFFFFF) {    printf("pas de device\n");    did = 0;    return(did);  }  id = did & 0xFFFF;  if(id == 0x1057) strcpy(vendeur,"Motorola");  else if(id == 0x1000) strcpy(vendeur,"Symbios");  else if(id == 0x1011) strcpy(vendeur,"DEC");  else if(id == 0x10AD) strcpy(vendeur,"Windbond");  else if(id == 0x10B5) strcpy(vendeur,"PLX");  else if(id == 0x10DC) strcpy(vendeur,"CERN");  else if(id == 0x11B0) strcpy(vendeur,"RAMiX");  else sprintf(vendeur,"%04X",id);  PciConfigReadLong(b,PCI_CFG_REVISION,&val);  classe = (val >> 8) & 0xFFFFFF;  i = (classe >> 16) & 0xFF;  k = (classe >> 8) & 0xFF;  if((i < 0) || (i >= PCI_NB_CLASSES)) i = PCI_NB_CLASSES - 1;  printf("%s",TypeDevice[i]);  if(i == PCI_CLASS_OLD) {    if(k == 0) printf(" non-VGA"); else if(k == 1) printf(" VGA");  } else if(i == PCI_CLASS_DISK) {    if(k >= PCI_NB_DISKS) k = PCI_NB_DISKS - 1;    printf(" %s",Disque[k]);  } else if(i == PCI_CLASS_NET) {    if(k >= PCI_NB_NETS) k = PCI_NB_NETS - 1;    printf(" %s",Net[k]);  } else if(i == PCI_CLASS_DPLY) {    if(k == 0) {      i = classe & 0xFF;      if(i == 0) printf(" compatible VGA"); else if(i == 1) printf(" compatible 8514");    } else if(k == 1) printf(" XGA");  } else if(i == PCI_CLASS_MMED) {    if(k == 0) printf(" video"); else if(k == 1) printf(" audio");  } else if(i == PCI_CLASS_MEM) {    if(k == 0) printf(" RAM"); else if(k == 1) printf(" Flash");  } else if(i == PCI_CLASS_BRDG) {    if(k >= PCI_NB_BRIDGES) k = PCI_NB_BRIDGES - 1;    printf(" %s",Bridge[k]);  } else if(i == PCI_CLASS_INPT) {    if(k >= PCI_NB_INPUTS) k = PCI_NB_INPUTS - 1;    printf(" %s",InputDev[k]);  } else if(i == PCI_CLASS_PROC) {    if(k == 0x00) printf(" 386");     else if(k == 0x01) printf(" 486");     else if(k == 0x02) printf(" Pentium");     else if(k == 0x10) printf(" Alpha");     else if(k == 0x20) printf(" PowerPC");     else if(k == 0x40) printf(" (co-pro)");   } else if(i == PCI_CLASS_SRAL) {    if(k >= PCI_NB_SERIALS) k = PCI_NB_SERIALS - 1;    printf(" %s",Serial[k]);  }  printf(" %s-%04X",vendeur,(did >> 16) & 0xFFFF);  printf(" revision %d\n\n",val & 0xFF);  printf("Device: %04X                                Vendor: %04X\n",	 (did >> 16) & 0xFFFF,did & 0xFFFF);  PciConfigReadLong(b,PCI_CFG_COMMAND,&cmd);  printf("Status: %04X                               Command: %04X ",	 (cmd >> 16) & 0xFFFF,cmd & 0xFFFF);  if(cmd & 7) {    if(cmd & PCI_CMD_MASTER_ENABLE) printf("-Master");    if(cmd & PCI_CMD_MEM_ENABLE) printf("-Mem");    if(cmd & PCI_CMD_IO_ENABLE) printf("-I/O");    printf(" enabled\n");  } else printf("-no access\n");  printf("Classe: %06X                            Revision:   %02X\n",	 classe,val & 0xFF);  PciConfigReadLong(b,PCI_CFG_CACHE_LINE_SIZE,&val);  printf("BIST: %02X     HdrType:  %02X     Latcy:  %02X     Cache:   %02X\n",	 (val >> 24) & 0xFF,(val >> 16) & 0xFF,(val >> 8) & 0xFF,val & 0xFF);  for(i=0; i<6; i++) {    PciConfigReadLong(b,PCI_CFG_BASE_ADDRESS_0 + (i * 4),&mem);    if(memoire) {      val = 0xFFFFFFFF;      PciConfigWriteLong(b,PCI_CFG_BASE_ADDRESS_0 + (i * 4),val);      PciConfigReadLong(b,PCI_CFG_BASE_ADDRESS_0 + (i * 4),&val);      if(val == 0) break;      PciConfigWriteLong(b,PCI_CFG_BASE_ADDRESS_0 + (i * 4),mem);    } else val = mem;    printf("Base address  #%d: %08X  ",i,mem);    if(val & PCI_BASE_IO) printf("(I/O space"); else printf("(Mem space");    if((val & 0x6) == PCI_BASE_BELOW_1M) printf(", base<1Mb [0x00100000]");    if((val & 0x6) == PCI_BASE_IN_64BITS) printf(", 64 bits");    if(memoire) {      printf(", =%08X: ",val);      msk = (val & PCI_BASE_IO)?  PCI_IOBASE_MASK: PCI_MEMBASE_MASK;      k = 0xFFFFFFFF - (val & msk); k++;      if(k < KB) printf("%d bytes",k);      else if(k < MB) printf("0x%04X bytes [%d Kb]",k,k/KB);      else printf("0x%08X bytes [%d Mb]",k,k/MB);    }    printf(")\n");  }  PciConfigReadLong(b,PCI_CFG_CIS,&val);  printf("CardBus CIS pntr: %08X\n",val);  PciConfigReadLong(b,PCI_CFG_SUB_VENDER_ID,&val);  id = val & 0xFFFF;  if(id == 0x1057) strcpy(vendeur,"Motorola");  else if(id == 0x1000) strcpy(vendeur,"Symbios");  else if(id == 0x1011) strcpy(vendeur,"DEC");  else if(id == 0x10AD) strcpy(vendeur,"Windbond");  else if(id == 0x10B5) strcpy(vendeur,"PLX");  else if(id == 0x10DC) strcpy(vendeur,"CERN");  else if(id == 0x11B0) strcpy(vendeur,"RAMiX");  else sprintf(vendeur,"%04X",id);  printf("    Sous-systeme: %s-%04X\n",vendeur,(val >> 16) & 0xFFFF);  PciConfigReadLong(b,PCI_CFG_EXPANSION_ROM,&mem);  if(memoire) {    val = 0xFFFFFFFE;    PciConfigWriteLong(b,PCI_CFG_EXPANSION_ROM,val);    PciConfigReadLong(b,PCI_CFG_EXPANSION_ROM,&val);    if(val != 0) PciConfigWriteLong(b,PCI_CFG_EXPANSION_ROM,mem);  } else val = mem;  printf("ROM base address: %08X, %s",mem,(mem & 1)?"active":"inactive");  if(memoire) {    printf(" (=%08X: ",val);    k = 0xFFFFFFFF - (val & 0xFFFFF800); k++;    if(k < KB) printf("%d bytes",k);    else if(k < MB) printf("0x%04X bytes [%d Kb]",k,k/KB);    else printf("0x%08X bytes [%d Mb]",k,k/MB);    printf(")");  }  printf("\n");  PciConfigReadLong(b,PCI_CFG_DEV_INT_LINE,&val);  printf("MaxLat: %02X    MinGnt:  %02X     ItPin:  %02X    ItLine:   %02X\n",	 (val >> 24) & 0xFF,(val >> 16) & 0xFF,(val >> 8) & 0xFF,val & 0xFF);  printf("\n");    return(did);}#ifdef VME/* ========================================================================== */int pciscan(char etendu, char memoire) {  TypePCIboard b;  int bus,d,f,adrs,nb;  int classe;  UINT32 device;  UINT32 vendor;  char avec_extenseur;  struct {    int vid,did,adrs;    int classe;  } carte_pci[PCI_MAX_DEV];  int sysGetBusSpd();  printf("===============================================================================\n");  printf("pciscan: Examen du bus PCI\n\n");  printf("Programme compile pour une carte %s\n",BOARD_TYPE);#ifdef VXWORKS  printf("Carte examinee: %s @%d MHz ",sysModel(),sysGetBusSpd());#endif  nb = 0;  vendor = EXTENDER_VID; avec_extenseur = 1;  /* voit-on le bridge 'PCI-PCI vers extenseur a 2 PMC' ?? */  if(PciDeviceFind(vendor,0x22,0,&b) == ERROR) {    /* voit-on le bridge 'PCI-PCI vers extenseur a 3 PMC' ?? */    if(PciDeviceFind(vendor,0x25,0,&b) == ERROR) {      avec_extenseur = 0; /* finalement, pas d'extenseur visible */      printf("sans extenseur\n");    } else printf("avec extenseur a 3 PMC\n");  } else printf("avec extenseur a 2 PMC\n");  printf("BSP %s\n",sysBspRev());  if(avec_extenseur) PciConfigWriteLong(b,EXTENDER_BUS_DEFINITION,0x00010100);  for(bus = 0; bus < PCI_MAX_BUS; bus++) {    printf("................................... Bus #%d ....................................\n",b);    for(d = 0; d < ((bus == 0) ? PCI_MAX_DEV : 16); ++d) {      carte_pci[d].vid = -1;      for (f = 0; f < PCI_MAX_FUNC; f++) {#if (PCI_MAX_DEV > 31) && (PCI_MAX_FUNC > 7)	/* avoid a special bus cycle */	if ((d == 0x1f) && (f == 0x07)) continue;#endif/* Probleme de trouver b quand on a bus,d,f */	PciConfigReadLong(b,PCI_CFG_VENDOR_ID,&vendor);	if(vendor == 0xFFFFFFFF) break;	nb++;	if(etendu) pcidump(b,memoire);	else {	  device  = (vendor >> 16) & 0xFFFF;	  vendor &= 0xFFFF;	  carte_pci[d].vid = vendor;	  carte_pci[d].did = device;	  if(PciConfigReadLong(b,PCI_CFG_BASE_ADDRESS_0,&adrs) == ERROR)	    carte_pci[d].adrs = 0xFFFF;	  else carte_pci[d].adrs = (adrs >> 16) & 0xFFFF;	  if(PciConfigReadLong(b,PCI_CFG_REVISION,&classe) == ERROR)	    carte_pci[d].classe = PCI_NB_CLASSES - 1;	  else {	    classe = (classe >> 24) & 0xFF;	    if((classe < 0) || (classe >= PCI_NB_CLASSES)) classe = PCI_NB_CLASSES - 1;	    carte_pci[d].classe = classe;	  }	}	break;      }    }        if(!etendu) {      for(d = 0; d < 16; ++d) printf(" %02d  ",d);      printf("\n");      for(d = 0; d < 16; ++d) {	if(carte_pci[d].vid == -1) printf(" --  ");	else printf("%04X ",carte_pci[d].vid);      }      printf("\n");      for(d = 0; d < 16; ++d) {	if(carte_pci[d].vid == -1) printf("     ");	else printf("%04X ",carte_pci[d].did);      }      printf("\n");      for(d = 0; d < 16; ++d) {	if(carte_pci[d].vid == -1) printf("     ");	else printf("%-4s ",Classe[carte_pci[d].classe]);      }      printf("\n");      for(d = 0; d < 16; ++d) {	if(carte_pci[d].vid == -1) printf("     ");	else printf("%04X ",carte_pci[d].adrs);      }      printf("\n");      if(bus == 0) {	for(d = 16; d < PCI_MAX_DEV; ++d) {	  printf(" %02d  ",d);	}	printf("\n");	for(d = 16; d < PCI_MAX_DEV; ++d) {	  if(carte_pci[d].vid == -1) printf(" --  ");	  else printf("%04X ",carte_pci[d].vid);	}	printf("\n");	for(d = 16; d < PCI_MAX_DEV; ++d) {	  if(carte_pci[d].vid == -1) printf("     ");	  else printf("%04X ",carte_pci[d].did);	}	printf("\n");	for(d = 16; d < PCI_MAX_DEV; ++d) {	  if(carte_pci[d].vid == -1) printf("     ");	  else printf("%-4s ",Classe[carte_pci[d].classe]);	}	printf("\n");	for(d = 16; d < PCI_MAX_DEV; ++d) {	  if(carte_pci[d].vid == -1) printf("     ");	  else printf("%04X ",carte_pci[d].adrs);	}	printf("\n");      }      printf("\n");    }  }  printf("================================ C'est tout! ==================================\n");  return(nb);}#endif#ifdef SPECIAL/* ========================================================================== */void pcitest() {  pciConfigOutLong(0,16,0,4,0x040);  pciConfigOutLong(0,16,0,4,0x100);}#endif/* ========================================================================== */int raminit(int base, char bavard) {  TypePCIboard b;  short cmd;  int i,k;#define PMC551_SDRAM_MA  0x60#define PMC551_SDRAM_CMD 0x62#define PMC551_DRAM_CFG  0x64#define PMC551_COMMAND_DONE 0x01  PciInit(0);  if(PciDeviceFind(Ram_vid,Ram_did,Ram_index,&b) == ERROR) {    if(bavard) printf("Carte %04X-%04X #%d absente\n",Ram_vid,Ram_did,Ram_index);    return(0);  }#ifdef VXWORKS  if(bavard)    printf("Carte %04X-%04X #%d trouvee: bus #%d, device #%d, fonction #%d\n",Ram_vid,Ram_did,Ram_index,b,d,f);#endif#ifdef macintosh  if(bavard)    printf("Carte %04X-%04X #%d trouvee\n",Ram_vid,Ram_did,Ram_index);#endif  k = 0xFFFFFFFF;  PciConfigWriteLong(&b,PCI_CFG_BASE_ADDRESS_0,k);  PciConfigReadLong(&b,PCI_CFG_BASE_ADDRESS_0,&k);  if(bavard) printf("Registre d'adresse de base #0 en test: %08X\n",k);  /* if(k == 0) registre non implemente */  /* if(k & PCI_BASE_IO) registre dans l'espace I/O */  /* if((k & 0x6) == PCI_BASE_BELOW_1M)  base en-dessous de 1Mb (0x00100000) */  /* if((k & 0x6) == PCI_BASE_IN_64BITS) base sur 64 bits, utiliser aussi le registre suivant */  /* i = 0x10; while((i <= 0x40000000) && !(k & i)) i = i << 1; */  i = 0xFFFFFFFF - (k & 0xFFFFFFF0); i++;  if(bavard) printf("Taille declaree: %08X (%d Mb)\n",i,i / (1024 * 1024));  k = ((base - 1) / i) + 1;  base = k * i;  if(bavard) printf("Adresse de base memoire: %08X\n",base);  if(bavard) {    PciConfigReadLong(&b,PCI_CFG_BASE_ADDRESS_0,&k);    printf("Registre d'adresse de base #0 final: %08X\n",k);    PciConfigReadWord(&b,PCI_CFG_COMMAND,&cmd);    printf("Registre de commande: %04X\n",cmd);    /*    for(i=1; i<6; i++) {      k = 0xFFFFFFFF;      PciConfigWriteLong(&b,PCI_CFG_BASE_ADDRESS_0 + (i * 4),k);      PciConfigReadLong(&b,PCI_CFG_BASE_ADDRESS_0 + (i * 4),&k);      printf("Registre d'adresse de base #%d en test: %08X\n",i,k);    }    */  }  if(bavard) printf("\nCommandes avec MA=0x400\n");  PciConfigWriteWord(&b,PMC551_SDRAM_MA,0x400);  if(bavard) printf("  Ecriture de la commande 0xBF\n");  PciConfigWriteWord(&b,PMC551_SDRAM_CMD,0xBF);  do {    PciConfigReadWord(&b,PMC551_SDRAM_CMD,&cmd);    if(bavard) printf("    Attente de fin d'execution\n");  } while(cmd & PMC551_COMMAND_DONE);  if(bavard) printf("    Execution terminee\n");  for(i=0; i<8; i++) {    if(bavard) printf("  Ecriture de la commande 0xDF\n");    PciConfigWriteWord(&b,PMC551_SDRAM_CMD,0xDF);    do {      PciConfigReadWord(&b,PMC551_SDRAM_CMD,&cmd);      if(bavard) printf("    Attente de fin d'execution\n");    } while(cmd & PMC551_COMMAND_DONE);    if(bavard) printf("    Execution terminee\n");  }  if(bavard) printf("Commandes avec MA=0x020\n");  PciConfigWriteWord(&b,PMC551_SDRAM_MA,0x020);  if(bavard) printf("  Ecriture de la commande 0xFF\n");  PciConfigWriteWord(&b,PMC551_SDRAM_CMD,0xFF);  do {    PciConfigReadWord(&b,PMC551_SDRAM_CMD,&cmd);    if(bavard) printf("    Attente de fin d'execution\n");  } while(cmd & PMC551_COMMAND_DONE);  if(bavard) printf("    Execution terminee\n");  if(bavard) printf("Configuration finale\n");  PciConfigReadLong(&b,PMC551_DRAM_CFG,&k);  k |= 0x02000000;  PciConfigWriteLong(&b,PMC551_DRAM_CFG,k);  if(bavard) printf("    Configuration terminee\n");#ifdef ADRESSAGE_EXPRESS  pciDevConfig(b,d,f,0,base,    PCI_CMD_MEM_ENABLE	       /*  | PCI_CMD_WI_ENABLE */  | PCI_CMD_PERR_ENABLE  | PCI_CMD_SERR_ENABLE); /* 0x142 */#else  PciConfigWriteLong(&b,PCI_CFG_BASE_ADDRESS_0,base);  /*  PciConfigWriteWord(&b,PCI_CFG_COMMAND,    PCI_CMD_MEM_ENABLE  | PCI_CMD_PERR_ENABLE  | PCI_CMD_SERR_ENABLE);  */#endif  return(base);}/* ========================================================================== */unsigned int ramread(unsigned int adrs) {  return(*(unsigned int *)adrs);}/* ========================================================================== */unsigned int ramwrite(unsigned int adrs, unsigned int val) {  *(unsigned int *)adrs = val;  return(val);}/* ========================================================================== */unsigned int memread(unsigned int adrs) {  unsigned int lu;  lu = 0xFFFFFFFF;#ifdef VXWORKS  if(vxMemProbe((char *)adrs,VX_READ,4,(char *)&lu) ==  OK) {    printf("@%08X -> %08X\n",adrs,lu);  } else printf("Lecture @%08X en erreur (impossible)\n",adrs);#endif#ifdef macintosh	lu = *(unsigned int *)adrs;	printf("@%08X -> %08X\n",adrs,lu);#endif  return(lu);}/* ========================================================================== */unsigned int memwrite(unsigned int adrs, unsigned int val) {#ifdef VXWORKS  if(vxMemProbe((char *)adrs,VX_WRITE,4,(char *)&val) ==  OK) {    printf("@%08X <- %08X\n",adrs,val);  } else printf("Ecriture @%08X en erreur (impossible)\n",adrs);#endif#ifdef macintosh	*(unsigned int *)adrs = val;    printf("@%08X <- %08X\n",adrs,val);#endif  return(val);}/* ========================================================================== */unsigned int shortread(unsigned int adrs) {  unsigned int lu;  lu = 0xFFFFFFFF;#ifdef VXWORKS  if(vxMemProbe((char *)adrs,VX_READ,4,(char *)&lu) ==  OK) {    printf("@%08X -> %08X\n",adrs,lu);  } else printf("Lecture @%08X en erreur (impossible)\n",adrs);#endif#ifdef macintosh	lu = (unsigned int)*(unsigned short *)adrs;	printf("@%08X -> %04X\n",adrs,(unsigned short)lu);#endif  return(lu);}/* ========================================================================== */unsigned int shortwrite(unsigned int adrs, unsigned int val) {#ifdef VXWORKS  if(vxMemProbe((char *)adrs,VX_WRITE,4,(char *)&val) ==  OK) {    printf("@%08X <- %08X\n",adrs,val);  } else printf("Ecriture @%08X en erreur (impossible)\n",adrs);#endif#ifdef macintosh	*(unsigned short *)adrs = (unsigned short)val;    printf("@%08X <- %04X\n",adrs,(unsigned short)val);#endif  return(val);}/* ========================================================================== */unsigned int blockread(unsigned int adrs, int nb) {  unsigned int lu;  int i,j;  lu = 0xFFFFFFFF;  i = nb / 4; j = 0;  while(i--) {#ifdef VXWORKS	if(vxMemProbe((char *)adrs,VX_READ,4,(char *)&lu) != OK) {		printf("Lecture @%08X en erreur (impossible)\n",adrs);		break;	}#endif#ifdef macintosh	lu = *(unsigned int *)adrs;#endif	if(j == 0) printf("@%08X:",adrs);	printf("  %08X",lu);	adrs += 4; if(j < 7) j++; else { j = 0; printf("\n"); }  }  if(j > 0) printf("\n");  return(lu);}/* ========================================================================== */unsigned int blockwrite(unsigned int adrs, int nb, unsigned int val) {  int i; unsigned int cour;  cour = adrs;  i = nb / 4;  while(i--) {#ifdef VXWORKS	if(vxMemProbe((char *)cour,VX_WRITE,4,(char *)&val) !=  OK) {		printf("Ecriture @%08X en erreur (impossible)\n",cour);		break;	}#endif#ifdef macintosh	*(unsigned int *)cour = val;#endif	cour += 4;  }  if(i == -1) printf("@%08X+%X <- %08X *%x\n",adrs,nb,val,nb);  else printf("Erreur d'ecriture a l'adresse %08X\n",cour);  return(val);}/* ========================================================================== */unsigned int stackread(unsigned int adrs, int nb) {  unsigned int lu;  int i,j;  lu = 0xFFFFFFFF;  i = nb; j = 0;  while(i--) {#ifdef VXWORKS	if(vxMemProbe((char *)adrs,VX_READ,4,(char *)&lu) != OK) {		printf("Lecture @%08X en erreur (impossible)\n",adrs);		break;	}#endif#ifdef macintosh	lu = *(unsigned int *)adrs;#endif	if(Log) {		if(j == 0) printf("@%08X:",adrs);		printf("  %08X",lu);		if(j < 7) j++; else { j = 0; printf("\n"); }	}  }  if(Log) { if(j > 0) printf("\n"); }  return(lu);}/* ========================================================================== */unsigned int stackwrite(unsigned int adrs, int nb, unsigned int val) {  int i;  i = nb;  while(i--) {#ifdef VXWORKS	if(vxMemProbe((char *)adrs,VX_WRITE,4,(char *)&val) !=  OK) {		printf("Ecriture @%08X en erreur (impossible)\n",adrs);		break;	}#endif#ifdef macintosh	*(unsigned int *)adrs = val;#endif  }  if(i == -1) printf("@%08X <- %08X *%x\n",adrs,val,nb);  else printf("Erreur d'ecriture a l'iteration 0x%X\n",nb-i);  return(val);}#ifdef VXWORKS/* ========================================================================== */int memcheck(unsigned int base, unsigned int maxi, char verif, char bavard) {  unsigned int adrs;  unsigned int taille,prec,lu;  char erreur;  taille = 0; prec = 0; /* pour faire taire le compilo */  erreur = 0;  do {    adrs = base + taille;    if(bavard) printf("Ecriture en 0x%08X\n",adrs); fflush(stdout);    /*    *(int *)adrs = taille; */    if(vxMemProbe((char *)adrs,VX_WRITE,4,(char *)&taille) ==  OK) {      if(bavard) printf("  Ecriture correcte a %d Mb\n",taille / (1024 * 1024));      if(verif) {	if(taille) *(int *)(base + prec) = prec;	if(vxMemProbe((char *)adrs,VX_READ,4,(char *)&lu) ==  OK) {	  if(lu == taille) {	    if(bavard) printf("  Lecture correcte: taille minimum %d Mb\n",taille / (1024 * 1024));	  } else {	    if(bavard) printf("  Lecture en erreur, 0x%08X au lieu de 0x%08X\n",lu,taille);	    erreur = 1;	  }	} else {	  if(bavard) printf("  Lecture en erreur (impossible)\n");	  erreur = 1;	}      }    } else {      if(bavard) printf("  Ecriture en erreur (impossible)\n");      erreur = 1;    }    if(erreur) {      if(taille) {	taille -= 4;	adrs = base + taille;	if(bavard) printf("Ecriture en 0x%08X\n",adrs); fflush(stdout);	if(vxMemProbe((char *)adrs,VX_WRITE,4,(char *)&taille) ==  OK) {	  taille += 4;	  if(bavard) printf("  Ecriture correcte, taille effective %d Mb (0x%08x)\n",			     taille / (1024 * 1024),taille);	} else {	  if(bavard) printf("  Ecriture encore en erreur, pas d'autre adresse testee\n");	  taille = prec + 4;	}      }      break;    }    prec = taille;    if(!taille) taille = 0x01000000; else taille *= 2;  } while(taille <= maxi);  return(taille);}#endif/* ========================================================================== */main() {  char commande[80],mode,precedente[80];  unsigned short vid,did;  TypePCIboard board;  int bus,dev,adrs,val,nb,verif,log;  int phys,dispo;  int j,rc;#ifdef VXWORKS  int fin,lu,i;  ioctl(0,FIOSETOPTIONS,OPT_RAW);  ioctl(0,FIOSETOPTIONS,OPT_CRMOD);#endif  Log = 1;  Allouee = 0;  bus = dev = adrs = 0;  val = 0;  verif = log = 0;  strcpy(precedente,"?");  do {    printf("PciDbg> "); fflush(stdout);    fgets(commande,80,stdin);    if(commande[0] == '!') strcpy(commande,precedente);    else strcpy(precedente,commande);    mode = commande[0];    switch(mode) {    case '?': case 'h':       printf("cs vendor,device     : configuration select\n");      printf("cw adrs,val          : configuration write\n");      printf("cr adrs              : configuration read\n");      printf("cd 0/1               : configuration dump <verif-memoire>\n");#ifdef VME      printf("cs 0/1,0/1           : configuration scan <etendu> <verif-memoire>\n");#endif#ifdef SPECIAL      printf("ct                   : routine pcitest\n");#endif      printf("\n");      printf("ri baseadrs,0/1      : init PMC551 <log>\n");      printf("rr adrs              : ram read (non protege)\n");      printf("rw adrs,val          : ram write (non protege)\n");      printf("\n");      printf("mr adrs              : memory read\n");      printf("mw adrs,val          : memory write\n");      printf("sr adrs              : short memory read\n");      printf("sw adrs,val          : short memory write\n");      printf("br adrs,nb           : block read\n");      printf("bw adrs,nb,val       : block write\n");      printf("fr adrs,nb           : FIFO read\n");      printf("fw adrs,nb,val       : FIFO write\n");      printf("\n");#ifdef VXWORKS      printf("mc base,maxi,0/1,0/1 : memory check <relit> <log>\n");      printf("md adrs,lngr         : memory dump\n");      printf("\n");#endif      printf("am nb                : allocation memoire (octets)\n");      printf("ar val               : remplissage memoire allouee\n");      printf("ai                   : impression memoire allouee\n");      printf("d[+|-]               : display de 'fr' [oui|non]\n");      printf("?                    : help\n");      printf("!                    : commande precedente\n");      printf("q/x                  : quitter\n");#ifdef VXWORKS      printf("Voir aussi debughelp sous WindSh\n");#endif      break;	/* Configuration */    case 'c':      switch(commande[1]) {      case 's':		if(mode != '!') sscanf(commande+2,"%hx,%hx",&vid,&did);		if(PciDeviceFind(vid,did,0,&board) == ERROR) 			printf("%04X-%04X absente\n",vid,did);		else printf("%04X-%04X trouvee\n",vid,did);		break;      case 'w':		if(mode != '!') sscanf(commande+2,"%x,%x",&adrs,&val);		PciConfigWriteLong(&board,adrs,val);		printf("%04X-%04X: @%02X <- %08X\n",vid,did,adrs,val);		break;      case 'r':		if(mode != '!') sscanf(commande+2,"%d",&adrs);		PciConfigReadLong(&board,adrs,&val);		printf("%04X-%04X: @%02X -> %08X\n",vid,did,adrs,val);		break;      case 'd':		if(mode != '!') sscanf(commande+2,"%d",&verif);		pcidump(&board,verif);		break;#ifdef VME      case 's':		if(mode != '!') sscanf(commande+2,"%d,%d",&val,&verif);		pciscan(val,verif);		break;#endif#ifdef SPECIAL      case 't':		pcitest();		break;#endif	  }      break;	/* RAM */    case 'r':      switch(commande[1]) {      case 'i':		if(mode != '!') sscanf(commande+2,"%x,%d",&adrs,&log);		raminit(adrs,log);		break;      case 'r':		if(mode != '!') sscanf(commande+2,"%x",&adrs);		val = 0xffffffff;		val = ramread(adrs);		printf("@%08X -> %08X\n",adrs,val);		break;      case 'w':		if(mode != '!') sscanf(commande+2,"%x,%x",&adrs,&val);		ramwrite(adrs,val);		printf("@%08X <- %08X\n",adrs,val);		break;      }      break;	/* Memoire (int) */    case 'm':      switch(commande[1]) {      case 'r':		if(mode != '!') sscanf(commande+2,"%x",&adrs);		memread(adrs);		break;      case 'w':		if(mode != '!') sscanf(commande+2,"%x,%x",&adrs,&val);		memwrite(adrs,val);		break;#ifdef VXWORKS      case 'c':		if(mode != '!') sscanf(commande+2,"%x,%x,%d,%d",&adrs,&val,&verif,&log);		memcheck(adrs,val,verif,log);		break;      case 'd':		if(mode != '!') sscanf(commande+2,"%x,%x",&adrs,&val);		fin = adrs + val;		while(adrs < fin) {		  printf("%08X: ",adrs);		  i = 4;		  while(i-- && (adrs < fin)) {		    if(vxMemProbe((char *)adrs,VX_READ,4,(char *)&lu) ==  OK) {		      printf(" %08X",lu);		      adrs += sizeof(unsigned int);		    } else {		      printf("\nLecture @%08X en erreur (impossible)",adrs);		      fin = adrs;		      break;		    }		  }		  printf("\n");		}		adrs = fin;		break;#endif      }      break;	/* Memoire (short) */    case 's':      switch(commande[1]) {      case 'r':		if(mode != '!') sscanf(commande+2,"%x",&adrs);		shortread(adrs);		break;      case 'w':		if(mode != '!') sscanf(commande+2,"%x,%x",&adrs,&val);		shortwrite(adrs,val);		break;	  }	  break;	/* Memoire (block) */    case 'b':      switch(commande[1]) {      case 'r':		if(mode != '!') sscanf(commande+2,"%x,%x",&adrs,&nb);		blockread(adrs,nb);		break;      case 'w':		if(mode != '!') sscanf(commande+2,"%x,%x,%x",&adrs,&nb,&val);		blockwrite(adrs,nb,val);		break;	  }	  break;	/* Memoire (FIFO) */    case 'f':      switch(commande[1]) {      case 'r':		if(mode != '!') sscanf(commande+2,"%x,%x",&adrs,&nb);		stackread(adrs,nb);		break;      case 'w':		if(mode != '!') sscanf(commande+2,"%x,%x,%x",&adrs,&nb,&val);		stackwrite(adrs,nb,val);		break;	  }	  break;	/* Allocation locale */    case 'a':      switch(commande[1]) {      case 'm':		if(mode != '!') sscanf(commande+2,"%x",&nb);		if(Allouee) free(Allouee);		NbAlloues = nb;		Allouee = (int *)malloc(NbAlloues);#ifdef macintosh		rc = LockMemoryContiguous(Allouee,NbAlloues);		if(rc) printf("Le bloc[%X] ne p[eut etre place en RAM:\n%s\nDMA impossible\n",			NbAlloues,strerror(rc));		else {			LogicalToPhysicalTable table;			table.logical.address = Allouee;			table.logical.count = NbAlloues;			nb = 8;			rc = GetPhysical(&table,&nb);			if(rc) printf("L'adresse physique du bloc ne peut etre connue:\n%s\nDMA impossible\n",				strerror(rc));			else {				phys = (int)table.physical[0].address;				dispo = table.physical[0].count;				printf("Adresse physique (1/%d): %08X[%X]\n",nb,phys,dispo);			}		}#endif      case '?':		printf("Adresse logique       : %08X[%X]\n",Allouee,NbAlloues);		break;      case 'r':		if(mode != '!') sscanf(commande+2,"%x",&val);		for(j=0; j<NbAlloues; j++) Allouee[j] = val;		break;      case 'i':#ifdef macintosh		printf("Adresse physique: %08X[%X]\n",phys,dispo);#endif		printf("Adresse logique : %08X[%X]:",Allouee,NbAlloues);		for(j=0; j<NbAlloues; j++) {			if(!(j%8)) printf("\n%4x: ",j);			printf(" %08X",Allouee[j]);		}		printf("\n");		break;	  }	  break;	/* Divers */    case 'd':  	  Log = (commande[1] != '-');	  break;    }  } while((mode != 'x') && (mode != 'q'));  printf("pcidebug exited\n");  return(0);}