#ifndef EVENEMENT_H
#define EVENEMENT_H

/* ................................ evenement ............................... */

#define REPART_EVENT (unsigned short)0x8001
#define REPART_EVENT_DEF    0x0001
#define REPART_EVENT_DATA   0x0002
#define REPART_EVENT_CLOSED 0x0003
static char *Etiquette[4] = { "INDEFINIE", "DEFINITION", "DONNEES", "EVT TERMINE" };

typedef struct {
	int64 stamp;      /* date du debut de trace                            */
	short dim;        /* nombre total de donnees transmises                */
	short entree;     /* numero de numeriseur (index dans repart->entree)  */
	short adc;        /* numero d'adc: voie = numeriseur->voie[adc]        */
	unsigned short nul;
} TypeEventChannel;

typedef struct {
	int64 stamp;             /* date absolue en nb de coups d'horloge                    */
	int num;                 /* numero d'evenement                                       */
	unsigned char  bb;       /* numeriseur de la voie ayant declenche                    */
	unsigned char  trig;     /* voie (en fait, ADC) ayant declenche                      */
	unsigned short niveau;   /* maximum de  l'evenement                                  */
	unsigned short dim;      /* nombre total de voies transmises                         */
	unsigned char version;   /* version de la structure                                  */
	unsigned char vide;      /* complement inutilise en version 0                        */
	unsigned int temps_mort; /* temps depuis le dernier evenement                        */
} TypeEventDefinition;

typedef struct {
	TypeEventChannel canal[REPART_VOIES_MAX];
} TypeEventCanaux;

typedef struct {
	TypeEventDefinition def;
	TypeEventCanaux trace;
} TypeEventHeader;

typedef struct {
	unsigned short etiquette; /* determine le contenu de la trame (REPART_EVENT_xxx) */
	unsigned short nul[3];
} TypeEventTypeTrame;

typedef struct {
	int evt;                         /* numero d'evenement (comparer avec EventHeader)    */
	short tranche;                   /* numero de tranche                                 */
	short nb;                        /* nombre de valeurs dans cette tranche              */
} TypeEventDataRef;

#define REPART_EVENT_SIZE (CALI_TAILLE_DONNEES-sizeof(TypeEventTypeTrame)-sizeof(TypeEventDataRef))
#define REPART_EVENT_NBVAL (REPART_EVENT_SIZE/sizeof(TypeADU))
typedef struct {
	TypeEventDataRef ref;
	TypeDonnee val[REPART_EVENT_NBVAL]; /* tableau des valeurs transmises                    */
} TypeEventData;

typedef struct {
	TypeEventTypeTrame type;
	union {
		TypeEventHeader hdr;
		TypeEventData donnees;
	} is;
} TypeEventFrame;

#endif /* EVENEMENT_H */
