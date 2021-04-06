/*
 *  monit.h
 *  Samba
 *
 *  Created by Michel GROS on 18.07.12.
 *  Copyright 2012 CEA/DSM/DAPNIA. All rights reserved.
 *
 */

#ifndef MONIT_H
#define MONIT_H

#ifdef MONIT_C
#define MONIT_VAR
#else
#define MONIT_VAR extern
#endif

#include <rundata.h>

typedef enum {
	MONIT_SIGNAL = 0,
	MONIT_HISTO,
	MONIT_FFT,
	MONIT_FONCTION,
	MONIT_BIPLOT,
	MONIT_2DHISTO,
	MONIT_EVENT,
	MONIT_MOYEN,
	MONIT_PATTERN,
	MONIT_NBTYPES
} MONIT_TYPE;

typedef enum {
	MONIT_AMPL = 0,
	MONIT_BRUTE,
	MONIT_ABS,
	MONIT_MAX,
	MONIT_MONTEE,
	MONIT_DESCENTE,
	MONIT_DUREE,
	MONIT_BASE,
	MONIT_BRUIT,
	MONIT_ENERGIE,
	MONIT_SEUIL,
	MONIT_DECAL,
	MONIT_NBVARS
} MONIT_VARIABLE;

typedef enum {
	MONIT_IMPR_BOLOS_BB1 = 0,
	MONIT_IMPR_BOLOS_MIXTE,
	MONIT_NBIMPR_BOLOS
} MONIT_IMPR_BOLOS;

typedef enum {
	MONIT_ADU = 0,
	MONIT_ENTREE_ADC,
	MONIT_ENTREE_BB,
	MONIT_SORTIE_BOLO,
	MONIT_KEV,
	MONIT_NBUNITES
} MONIT_UNITE;
#define MONIT_UNITE_CLES "ADU/mV ADC/mV BB/nV detec/keV"

#ifdef INUTILISE
typedef enum {
	MONIT_EVTCOURANT = 0,
	MONIT_EVTJUSTE,
	MONIT_NBEVTS
} MONIT_EVT;
MONIT_VAR char *MonitFenEvtName[MONIT_NBEVTS+1]
#ifdef MONIT_C
= { "courant", "ajuste", "\0" }
#endif
;
#endif /* INUTILISE */

typedef struct {
	float r,g,b;
} TypeMonitRgb;

typedef struct {
//	short bolo,cap,voie;
	int bolo;
	int cap;
	int voie;
	int var;        /* 0: amplitude / 1: montee / etc...  ou 0: brutes / 1: traitees */
	short numx;     /* numero de donnee effectif dans le graphe                      */
	char ajuste;    /* vrai si ajustement automatique du graphique demande           */
	char voie_lue[MODL_NOM];
	char *liste_voies[DETEC_CAPT_MAX+1];
	TypeMonitRgb rgb;
	OpiumColor couleur;
} TypeMonitTrace;

typedef struct {
	int pts;
	union {
		struct {
			int min;             /* ADU                                 */
			int max;             /* ADU                                 */
		} i;
		struct {
			float min;           /* FFT: nV/rac(Hz)                     */
			float max;           /* FFT: nV/rac(Hz)                     */
		} r;
	};
} TypeMonitAxe;

#define MONIT_FENTITRE 80
typedef struct {
	char titre[MONIT_FENTITRE];
	 char type;      /* DonneesBrutes, Evenement, Histo ou FFT  */ // unsigned?
	char affiche;            /* affichage demande                       */
	char grille;             /* grille demandee                         */
	char change;             /* changement de detecteur demande         */
	int posx,posy,larg,haut; /* position et dimensions                  */
	TypeMonitAxe axeX,axeY;
	int nb;                  /* nombre de traces                        */
	TypeMonitTrace trace[MAXTRACES];
	Graph g;
	int pts;                 /* #bins (histo) ou echantillons (fft)     */
	union {
//-		struct {
//-			int cadre;           /* largeur du cadre (= f->larg * zoom) */
//-		} b;                     /* (propre au type 'brutes')           */
		struct {
			Histo histo;
			HistoData hd[MAXTRACES];
		} h;                     /* (propre au type 'histo' et 'fonction') */
		struct {
			H2D histo;
			char xlog;
		} s;                     /* (propre au type 'histo 2D')         */
		struct {
			char excl;           /* 0:indifferent, 1:exclusion evts     */
			char xlog;
			int intervalles;
//-			float min;
			float freq[MAXTRACES][2];
			float *spectre[MAXTRACES];
		} f;                     /* (propre au type 'fft')              */
	} p;
} TypeMonitFenetre;

MONIT_VAR int MonitFenNb,MonitFenNum;
MONIT_VAR TypeMonitFenetre MonitFen[MAXMONIT+2],MonitFenLue,*MonitFenEnCours;
MONIT_VAR char *MonitFenTitre[MAXMONIT+2];
MONIT_VAR char MonitAffModes,MonitAffTrig,MonitAffEtat,MonitAffNet;
MONIT_VAR char MonitAffBases,MonitAffpTaux,MonitAffgTaux,MonitAffgSeuils;
MONIT_VAR Panel pMonitBoloVoie;
MONIT_VAR float MonitEvtAmpl;
MONIT_VAR float MonitADUenmV;    /* Valeur d'1 ADU en mV a l'entree de l'ADC         */
MONIT_VAR float MonitADUenkeV;   /* Valeur d'1 ADU en keV                            */
MONIT_VAR float MonitIntervSecs;
MONIT_VAR int   MonitIntervAff;
MONIT_VAR char  MonitAffEvt;
MONIT_VAR char  MonitSynchro;    /* Synchronisation de l'affichage sur la modulation */
MONIT_VAR float MonitT0;         /* Origine des temps pour l'affichage               */
MONIT_VAR int64 MonitP0;         /* Timestamp correspondant                          */
MONIT_VAR char  MonitChgtUnite;  /* Changement d'unite: ADU -> Volts ou keV          */
MONIT_VAR int   MonitHaut,MonitLarg;  /* utilises dans settings.c et calculs.c */
MONIT_VAR char  MonitHampl,MonitHmontee,MonitH2D;
MONIT_VAR float MonitEvtAb6[VOIES_MAX][2],MonitTrmtAb6[VOIES_MAX][2];
MONIT_VAR int   MonitEvtAff;
MONIT_VAR float CoupureAmpl[2];   MONIT_VAR int   MonitHamplY[2];
MONIT_VAR float CoupureMontee[2]; MONIT_VAR int   MonitHmonteeY[2];
MONIT_VAR float Coup2DAmplX[2];   MONIT_VAR float Coup2DAmplY[2];
MONIT_VAR float Coup2DMonteeX[2]; MONIT_VAR float Coup2DMonteeY[2];
MONIT_VAR int   MonitH2DY[2];
MONIT_VAR Panel pMonitDesc; 

int MonitChoisiBoloVoie(int *voie_choisie);
int MonitFenMemo(TypeMonitFenetre *f);
Graph MonitFenCreeGraphe(TypeMonitFenetre *f, char nouveau);
int MonitFenAffiche(TypeMonitFenetre *f, char reellement);
int MonitFenBuild(TypeMonitFenetre *f);
void MonitFenClear(TypeMonitFenetre *f);
void MonitFenFree(TypeMonitFenetre *f);
float MonitUnitesADU(Graph g, int sens, float val);
int MonitEvtAffiche(int lequel, void *qui, int affiche);
int MonitEvtPrecedent(Menu menu, MenuItem *item);
int MonitEvtSuivant(Menu menu, MenuItem *item);
int MonitSauve();

#endif
