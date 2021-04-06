#ifndef ENVIRONNEMENT_H
#define ENVIRONNEMENT_H

/*
(#define XCODE  : ajoute dans les flags de compilation)
(#define UNIX   : ajoute dans les flags de compilation)
*/
#undef OS9
#undef CODE_WARRIOR_VSN
#define SANS_PCI
#define ATTENTE_AVEC_USLEEP

#ifdef XCODE
	#define darwin
	#define CHAINE_VSN_EXTERNE
#endif /* XCODE */

#ifdef GCC
	// #define INLINE inline
	#define INLINE
#else
	#define INLINE
#endif /* GCC */
//#ifdef GCC
//#pragma message "Avec GCC"
//#else
//#pragma message "Sans GCC"
//#endif

#ifdef darwin
	#define macintosh
	#define MACOSX
	#define LONG_IS_32BITS
	#define ACCES_DB
#endif /* darwin */

#endif /* ENVIRONNEMENT_H */
