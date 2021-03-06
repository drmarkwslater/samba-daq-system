#ifndef DEFINEVAR_H
#define DEFINEVAR_H

#include <environnement.h>
#include <execinfo.h>
/*
#pragma message("Utilisation de "__FILE__)
#ifdef macintosh
#pragma message("pour macintosh")
#else
#pragma message("sauf macintosh")
#endif */

#define MAXFILENAME 256
#define _ORIGINE_ __func__,__FILE__,__LINE__
#define chaine(s) chaine_preprocesseur(s)
#define chaine_preprocesseur(s) #s
#define PrevenirSi(expression) do if(expression) fprintf(stderr,"*** Attention: " #expression "\n"); while(0)

#define unless(condition) if(!(condition))
#define until(condition) while(!(condition))
#define forever while(1)
#define repeter do
#define indefiniment while(1)
#define tant_que while
#define pour_tout(i,m) for(i=0; i<m; i++)
// #define si if ... mais 'si' est utilise dans CoreServices > CarbonCore/MachineExceptions.h
#define cas_ou switch
#define selon_que switch
#define vaut case
#define au_pire default
#define sors break

#define Malloc(nb,type) (type*)malloc((nb) * sizeof(type))
#define MallocOrExit(ptr,nb,type) if(!(ptr = (type*)malloc(nb * sizeof(type)))) exit(ENOMEM)
#define FilePtr(p) (&_iob[p])
#define Abs(x) ((x > 0)? x : -x)
#define LePlusPetit(val1,val2) (val1 < val2)? val1: val2
#define LePlusGrand(val1,val2) (val1 > val2)? val1: val2
#define BCD(valeur) ((valeur/10)*16 + valeur-((valeur/10)*10))

#define ImprimePileAppels {\
	void* callstack[128];\
	int frame_num, frames_nb = backtrace(callstack, 128);\
	char** frame_texte = backtrace_symbols(callstack, frames_nb);\
	printf("| Suite des %d appel%s jusqu'a %s\n",frames_nb,(frames_nb > 1)?"s":"",__func__);\
	for (frame_num = 0; frame_num < frames_nb; ++frame_num) {\
		char *r,*s,c;\
		s = r = frame_texte[frame_num];\
		while((*s != '\0') && (*s != ' ')) s++; c = *s;\
		*s = '\0'; printf("| %s) ",r); if(c == '\0') continue;\
		r = s + 1; while((*r != '\0') && (*r == ' ')) r++; if(*r == '\0') continue;\
		s = r; while((*s != '\0') && (*s != ' ')) s++; c = *s; if(c == '\0') continue;\
		r = s + 1; while((*r != '\0') && (*r == ' ')) r++; if(*r == '\0') continue;\
		s = r; while((*s != '\0') && (*s != ' ')) s++; c = *s;\
		*s = '\0'; printf("@%s: ",r); if(c == '\0') continue;\
		r = s + 1; printf("%s\n",r);\
	}\
	free(frame_texte);\
}

//#ifdef PI
#undef PI
//#endif
#define PI 3.1415926535897932384626
#define RADTODEG (180.0 / PI)
#define DegtoRad (double)(0.0174532925199)
#define RadtoDeg (double)(57.295779513082)

#define E_COULOMB 1.602176e-19
#define E_PICOCOULOMB (E_COULOMB * 1.0e12)

#ifdef ANCIEN_HASARD
	#define HasardTirage(sigma,longueur,tirage) \
	{ int i; \
		tirage = 0.0; \
		for(i=0; i<longueur; i++) tirage += (float)(random() % 10000) / 10000.0; \
		tirage = sigma * ((tirage / (float)longueur) - 0.5); \
	}
	#define TireAuHasard(val)  val = ((val*39709+13) & 65535)
	#define Normalise(rnd,val) (((rnd+65536) * val) >> 17)
#endif

#define HasardInit srandom(time(NULL))
#define HasardTirage(sigma,longueur,tirage) \
	{ int i; long k; \
		k = 0; i = longueur; \
		while(i--) k += (random() & 0x3FFFF); \
		tirage = sigma * ((float)(k / longueur) / 262144.0); \
	}

#ifdef WIN32
	typedef long int64;
	typedef unsigned long hex64;
	#define FILESEP "\\"
#endif

#ifdef OS9
	#define FILESEP "/"
	typedef long int64;
	typedef unsigned long hex64;
#endif

#ifdef macintosh
	#ifdef MACOSX
		#define FILESEP "/"
	#else
		#define FILESEP ":"
	#endif
	typedef long long int64;
	typedef unsigned long long hex64;
#endif

#ifdef UNIX
	#ifndef macintosh
		#define FILESEP "/"
		#ifdef linux
			typedef long long int64;
			typedef unsigned long long hex64;
		#else
			typedef long int64;
			typedef unsigned long hex64;
		#endif
	#endif
#endif

#ifdef __INTEL__
	#define FILESEP "\\"
	typedef long long int64;
	typedef unsigned long long hex64;
#endif

#define _NI_int64_DEFINED_

#ifndef MAXNAMLEN
	#define MAXNAMLEN 40
#endif

#ifdef XCODE
	typedef unsigned short UInt16;
	typedef unsigned long Ulong;
#endif
#ifndef XCODE /* __MACTYPES__ inclus plus tard... */
	typedef unsigned short UInt16;
	typedef unsigned long Ulong;
	typedef unsigned long long UInt64;
	typedef unsigned long long UnsignedWide;
#endif
typedef unsigned long long IntAdrs;
typedef void *TypeAdrs;

typedef unsigned long  lhex;
typedef unsigned int   hexa;
typedef unsigned short shex;
typedef unsigned char  byte;

typedef int (*TypeFctn)();
typedef float (*TypeFloatFctn)();

/* ========================================================================== */

typedef union {
	struct {
		unsigned char msb,lsb;
	} octet;
	unsigned short mot;
} ShortSwappable;

typedef union {
	struct {
		unsigned char b0,b1,b2,b3;
	} octet;
	unsigned int mot;
} IntSwappable;

typedef union {
	struct {
		unsigned char b0,b1,b2,b3;
	} octet;
	float mot;
} FloatSwappable;

typedef union {
	struct {
		unsigned char b0,b1,b2,b3,b4,b5,b6,b7;
	} octet;
	int64 mot64;
} LongSwappable;

typedef union {
	struct {
		unsigned char b0,b1,b2,b3,b4,b5,b6,b7;
	} octet;
	double mot64;
} DoubleSwappable;

typedef union {
	struct {
		unsigned short s0,s1;
	} court;
	unsigned int mot;
} TwoShortsSwappable;

typedef union {
	struct {
		unsigned short s0,s1,s2,s3;
	} court;
	int64 mot64;
} FourShortsSwappable;

#ifdef INLINE_DANS_DEFINEVAR
/* ========================================================================== */
INLINE void ByteSwappeInt(unsigned int *tab) {
	IntSwappable tempo;

	tempo.octet.b0 = ((IntSwappable *)tab)->octet.b3;
	tempo.octet.b1 = ((IntSwappable *)tab)->octet.b2;
	tempo.octet.b2 = ((IntSwappable *)tab)->octet.b1;
	tempo.octet.b3 = ((IntSwappable *)tab)->octet.b0;
	*tab = tempo.mot;
}
/* ========================================================================== */
INLINE void ByteSwappeLong(int64 *tab) {
	LongSwappable tempo;

	tempo.octet.b0 = ((LongSwappable *)tab)->octet.b7;
	tempo.octet.b1 = ((LongSwappable *)tab)->octet.b6;
	tempo.octet.b2 = ((LongSwappable *)tab)->octet.b5;
	tempo.octet.b3 = ((LongSwappable *)tab)->octet.b4;
	tempo.octet.b4 = ((LongSwappable *)tab)->octet.b3;
	tempo.octet.b5 = ((LongSwappable *)tab)->octet.b2;
	tempo.octet.b6 = ((LongSwappable *)tab)->octet.b1;
	tempo.octet.b7 = ((LongSwappable *)tab)->octet.b0;
	*tab = tempo.mot64;
}
/* ========================================================================== */
INLINE void ByteSwappeShort(unsigned short *tab) {
	ShortSwappable tempo;

	tempo.octet.lsb = ((ShortSwappable *)tab)->octet.msb;
	tempo.octet.msb = ((ShortSwappable *)tab)->octet.lsb;
	*tab = tempo.mot;
}
/* ========================================================================== */
INLINE void ByteSwappeIntArray(unsigned int *tab, int nb) {
	int i; unsigned int *adrs;
	IntSwappable tempo;

	for(i=0, adrs = tab; i<nb; i++, adrs++) {
		tempo.octet.b0 = ((IntSwappable *)adrs)->octet.b3;
		tempo.octet.b1 = ((IntSwappable *)adrs)->octet.b2;
		tempo.octet.b2 = ((IntSwappable *)adrs)->octet.b1;
		tempo.octet.b3 = ((IntSwappable *)adrs)->octet.b0;
		*adrs = tempo.mot;
	}
}
/* ========================================================================== */
INLINE void ByteSwappeFloatArray(float *tab, int nb) {
	int i; float *adrs;
	FloatSwappable tempo;

	for(i=0, adrs = tab; i<nb; i++, adrs++) {
		tempo.octet.b0 = ((IntSwappable *)adrs)->octet.b3;
		tempo.octet.b1 = ((IntSwappable *)adrs)->octet.b2;
		tempo.octet.b2 = ((IntSwappable *)adrs)->octet.b1;
		tempo.octet.b3 = ((IntSwappable *)adrs)->octet.b0;
		*adrs = tempo.mot;
	}
}
/* ========================================================================== */
INLINE void ByteSwappeLongArray(int64 *tab, int nb) {
	int i; int64 *adrs;
	LongSwappable tempo;

	for(i=0, adrs = tab; i<nb; i++, adrs++) {
		tempo.octet.b0 = ((LongSwappable *)adrs)->octet.b7;
		tempo.octet.b1 = ((LongSwappable *)adrs)->octet.b6;
		tempo.octet.b2 = ((LongSwappable *)adrs)->octet.b5;
		tempo.octet.b3 = ((LongSwappable *)adrs)->octet.b4;
		tempo.octet.b4 = ((LongSwappable *)adrs)->octet.b3;
		tempo.octet.b5 = ((LongSwappable *)adrs)->octet.b2;
		tempo.octet.b6 = ((LongSwappable *)adrs)->octet.b1;
		tempo.octet.b7 = ((LongSwappable *)adrs)->octet.b0;
		*adrs = tempo.mot64;
	}
}
/* ========================================================================== */
INLINE void ByteSwappeDoubleArray(double *tab, int nb) {
	int i; double *adrs;
	LongSwappable tempo;

	for(i=0, adrs = tab; i<nb; i++, adrs++) {
		tempo.octet.b0 = ((LongSwappable *)adrs)->octet.b7;
		tempo.octet.b1 = ((LongSwappable *)adrs)->octet.b6;
		tempo.octet.b2 = ((LongSwappable *)adrs)->octet.b5;
		tempo.octet.b3 = ((LongSwappable *)adrs)->octet.b4;
		tempo.octet.b4 = ((LongSwappable *)adrs)->octet.b3;
		tempo.octet.b5 = ((LongSwappable *)adrs)->octet.b2;
		tempo.octet.b6 = ((LongSwappable *)adrs)->octet.b1;
		tempo.octet.b7 = ((LongSwappable *)adrs)->octet.b0;
		*adrs = tempo.mot64;
	}
}
/* ========================================================================== */
INLINE void ByteSwappeShortArray(unsigned short *tab, int nb) {
	int i; unsigned short *adrs;
	ShortSwappable tempo;

	for(i=0, adrs = tab; i<nb; i++, adrs++) {
		tempo.octet.lsb = ((ShortSwappable *)adrs)->octet.msb;
		tempo.octet.msb = ((ShortSwappable *)adrs)->octet.lsb;
		*adrs = tempo.mot;
	}
}
/* ========================================================================== */
INLINE void TwoShortsSwappeInt(unsigned int *entier) {
	TwoShortsSwappable tempo;

	tempo.court.s1 = ((TwoShortsSwappable *)entier)->court.s0;
	tempo.court.s0 = ((TwoShortsSwappable *)entier)->court.s1;
	*entier = tempo.mot;
}
/* ========================================================================== */
INLINE void FourShortsSwappeLong(int64 *mot64) {
	FourShortsSwappable tempo;

	tempo.court.s3 = ((FourShortsSwappable *)mot64)->court.s0;
	tempo.court.s2 = ((FourShortsSwappable *)mot64)->court.s1;
	tempo.court.s1 = ((FourShortsSwappable *)mot64)->court.s2;
	tempo.court.s0 = ((FourShortsSwappable *)mot64)->court.s3;
	*mot64 = tempo.mot64;
}
/* ========================================================================== */
INLINE void TwoShortsSwappeIntArray(unsigned int *tab, int nb) {
	int i; unsigned int *adrs;
	TwoShortsSwappable tempo;

	for(i=0, adrs = tab; i<nb; i++, adrs++) {
		tempo.court.s1 = ((TwoShortsSwappable *)adrs)->court.s0;
		tempo.court.s0 = ((TwoShortsSwappable *)adrs)->court.s1;
		*adrs = tempo.mot;
	}
}
/* ========================================================================== */
INLINE void FourShortsSwappeLongArray(int64 *tab, int nb) {
	int i; int64 *adrs;
	FourShortsSwappable tempo;

	for(i=0, adrs = tab; i<nb; i++, adrs++) {
		tempo.court.s3 = ((FourShortsSwappable *)adrs)->court.s0;
		tempo.court.s2 = ((FourShortsSwappable *)adrs)->court.s1;
		tempo.court.s1 = ((FourShortsSwappable *)adrs)->court.s2;
		tempo.court.s0 = ((FourShortsSwappable *)adrs)->court.s3;
		*adrs = tempo.mot64;
	}
}
/* ========================================================================== */
INLINE int Modulo(int64 v1, int v2) {
	int64 max,tours,point1;
	int k;

	max = (int64)v2;
	tours = v1 / max;
	point1 = v1 - (tours * max);
	k = (int)point1;
	return(k);
}
/* ========================================================================== */
INLINE int inintf(float x) { return((x >= 0.0)? (int)(x+ 0.5): (int)(x - 0.5)); }
/* ========================================================================== */
#endif /* INLINE_DANS_DEFINEVAR */

#endif



