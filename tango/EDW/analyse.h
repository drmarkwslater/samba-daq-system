#ifndef ANALYSE_H
#define ANALYSE_H

#include <defineVAR.h>
#include <opium.h>
#include <rundata.h>
#include <dateheure.h>

typedef short TypeSignee;

VAR int  ArchGraphPosX,ArchGraphPosY;
VAR int  ArchGraphHaut,ArchGraphLarg;
VAR char ProdCalibPath[MAXFILENAME];
VAR int  BoloTrigge,ModlTrigge;
VAR float *VoieFittee[VOIES_MAX];
VAR float *VoieDeconv[VOIES_MAX];
VAR int VoieDeconvDim[VOIES_MAX];
VAR float VoieRC[VOIES_MAX];
VAR char MesureFaite;

//#define MAX_VARS_NTUPLE (7*VOIES_MAX)+DETEC_MAX+16
#define MAX_VARS_NTUPLE 2048
#define MAX_NTP_VOIE 64
VAR TypePlotVar Ntuple[MAX_VARS_NTUPLE + 1];
VAR int ProdNtpNum[VOIES_MAX][MAX_NTP_VOIE];
VAR int ProdNtpNb[VOIES_MAX];
VAR int NtStdNb,NtTotNb;

VAR float *EvtPaquetNum;
VAR float *PaqAmpPremier;
VAR float *PaqAmpTotale;
VAR float *PaqMultiplicite;
VAR float *PaqEtendue;

/* Inutilise a cette heure (et nous sommes le 24.01.07 a 18h05)
VAR double *SignalFiltre[VOIES_MAX];
VAR int MaxFiltre[VOIES_MAX];
VAR int NbFiltre[VOIES_MAX]; */

VAR Panel pAfficheBolo,pAfficheVoie,pAfficheAmplMin,pAfficheDim,pArchBolos;
VAR char AfficheBolo[DETEC_MAX],AfficheVoie[DETEC_CAPT_MAX];
VAR char AfficheBoloDelaListe;

VAR char  AfficheEvtCalib,AfficheEvtDeconv;
VAR char  ProdPatternActif,ProdCrosstalkActif,ProdFiltreActif;
VAR float ProdMontee10,ProdMontee90;
VAR int   ProdPreLissage,ProdPostLissage;
VAR int   ProdPaquetAmpl;
VAR float ProdPaquetDuree;

VAR char ProdVarsName[MAXFILENAME],ProdCalibName[MAXFILENAME],ProdTempsName[MAXFILENAME];
VAR char ProdCtesName[MAXFILENAME],ProdChaleurName[MAXFILENAME];
VAR char ProdVarsFile[MAXFILENAME],ProdCalibFile[MAXFILENAME],ProdTempsFile[MAXFILENAME];
VAR char ProdCtesFile[MAXFILENAME],ProdChaleurFile[MAXFILENAME];
VAR float PolarIonisation;
VAR float EnergieRef,RefChal,RefIonis;
VAR float NeutronsFact,NeutronsExp;
VAR float SeuilChal,ResolChalBase,ResolChalRef;
VAR float SeuilIonis,ResolIonisBase,ResolIonisRef;
VAR float TauxCentrePur,ProdQuotaMontee;

VAR char MesurePourCalib;

/* voir production.c */
typedef struct {
	int num;           /* index dans le tableau des noms des voies            */
	int mesure;        /* index inverse (index de la mesure si <voie> connue) */
	float avant;       /* temps enregistre avant le maximum                   */ 
	float apres;       /* temps a utiliser apres le maximum pour l'evt unite  */
	float start,stop;  /* intervalle de calcul de la ligne de base            */
	float RC;          /* temps de descente de l'ampli a utiliser             */
} TypeVoieTemps;
VAR TypeVoieTemps VoieTemps[VOIES_MAX],VoieTempsLue;
VAR int VoieTempsNb;

#ifndef FICHIER_TEMPS_MANU
	VAR ArgDesc ProdTempsVoieDesc[6],ProdTempsDesc[9];
#endif
VAR ArgStruct ProdTempsDescAS
#ifdef MAINSAMBA
 = { (void *)VoieTemps, (void *)&VoieTempsLue, sizeof(TypeVoieTemps), ProdTempsVoieDesc }
#endif
;

typedef enum {
	EDW_SAMBA = 0,
	EDW_AMPL,
#ifdef CALCULE_SURFACE
	EDW_SURF,
#endif
	EDW_EVT,
	EDW_NBMESURES
} EDW_MESURE;

VAR char *NomMesure[]
#ifdef MAINSAMBA
 = { "samba", "amplitudes", 
	#ifdef CALCULE_SURFACE
	 "surfaces", 
	#endif
	 "evts unite", "\0" }
#endif
;

VAR char *ClesMesure
#ifdef MAINSAMBA
#ifdef CALCULE_SURFACE
 = "samba/amplitudes/surfaces/evts-unite"
#else
 = "samba/amplitudes/evts-unite"
#endif
#endif
;

VAR float *crosstalk[10];
VAR short *adrs_debut_centre;

int ArchEvtPattern(char actif);
int ArchEvtCrosstalk(char actif);
int ArchEvtFilter(char actif);
void ArchSetFilter(int voie, TypeFiltre *filtre);

int ellfcalc(char modele, char type, short degre, float dbr, int maxi,
			 float omega1, float omega2, float omega3, int *ncoef, double *dir, double *inv);
VAR char *FiltreAutorises[MAXFILTRES+2];
VAR int FiltreVoies[VOIES_MAX]; // numero de filtre pour chaque voie

#endif

