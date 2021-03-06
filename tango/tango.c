#ifdef macintosh
#pragma message(__FILE__)
#endif

#include <environnement.h>

#define TANGO_C
#define WND_C

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#ifdef CODE_WARRIOR_VSN
#include <stat.h>  /* pour avoir off_t */
#endif
#ifdef UNIX
/* #include <fcntl.h> */
#include <sys/stat.h>
#endif
#include <math.h>
#include <errno.h>
#ifndef WIN32
#include <strings.h>  /* pour strlen qui rend size_t si intrinseque, et rindex */
#else
#include <string.h>
#include <sys/stat.h>
#include <io.h>
#include <ConsoleWin32.h>
#endif

#ifdef ForTango
	#define APPLI "TANGO"
	#define NOM_APPLI "Tango"
#endif
#ifdef ForGigas
	#define APPLI "GIGAS"
	#define NOM_APPLI "Gigas"
	#define ForOpiumGL
//	#pragma message("Compilation avec OPENGL")
#endif

/* #define CHANGE_ROOTWINDOW */
#include <claps.h>
#include <decode.h>
#include <dico.h>
#include <opium.h>
#include <histos.h>
#include <nanopaw.h>
#include <filtres.h>

#include <tango.h>
#include <rundata.h>
#include <calib.h>
#include <evtread.h>
#include <archive.h>

#ifdef X11
	#ifndef XCODE
		#define ARGSCAN
		#undef MENU_BARRE
	#endif
#endif

// static char *TangoAppli
// #ifdef APPLI_DIR
// 	= chaine(APPLI_DIR)
// #else  /* APPLI_DIR */
// 	= "executables/"chaine(NOM_APPLI)
// #endif /* APPLI_DIR */
// ;

static char *TangoInfoSource
#ifdef COMPIL_DIR
	= chaine(COMPIL_DIR)
#else  /* COMPIL_DIR */
	#ifdef APPLI_DIR
		= chaine(APPLI_DIR)"/Contents/Resources/"
	#else  /* APPLI_DIR */
		= "Info/"chaine(NOM_APPLI)
	#endif /* APPLI_DIR */
#endif /* COMPIL_DIR */
;

static char *TangoVersionCompilo
#ifdef VERSION_PROJET
	#ifdef macintosh
		= chaine(VERSION_PROJET) 
		// "[" chaine(ARCHS) "/" chaine(NATIVE_ARCH) "+" chaine(VALID_ARCHS) "]" pas pris en compte (ARCHS=1, les autres non definies)
	#else  /* macintosh */
		= "non communiquee (OS non Apple)"
	#endif /* macintosh */
#else  /* VERSION_PROJET */
	= "indefinie (symbole VERSION_PROJET non defini)"
#endif /* VERSION_PROJET */
;

#define MAXDIR 2048

typedef struct {
	unsigned char oper;
	unsigned char marque;
	int var;
	float min,max;
	int valides,total;
} TypeCoupure;

#define MAXSELECTIONS 16
#define NOMMAX 32
static TypeCoupure Coupure[MAXSELECTIONS],CoupureLue; static int SelectNb;
static char CoupureNom[NOMMAX];
static char CoupePath[MAXFILENAME];
static char ClesMarquage[256];
static char TangoTitreRun[80];

/* PAS POSSIBLE AVEC PlotVarList DYNAMIQUE:
static ArgDesc CoupureDesc[] = {
	{ 0,     DESC_KEY "evts/et/ou",  &CoupureLue.oper,   0 },
	{ "var", DESC_LISTE PlotVarList, &CoupureLue.var,    0 },
	{ "min", DESC_FLOAT              &CoupureLue.min,    0 },
	{ "max", DESC_FLOAT              &CoupureLue.max,    0 },
	{ "off", DESC_KEY ClesMarquage,  &CoupureLue.marque, 0 },
	DESC_END
}; */
static ArgDesc CoupureDesc[6];
static ArgStruct CoupureAS = { (void *)Coupure, (void *)&CoupureLue, sizeof(TypeCoupure), CoupureDesc };
static ArgDesc JeuCoupureDesc[] = { { "Coupure", DESC_STRUCT_SIZE(SelectNb,MAXSELECTIONS) &CoupureAS, 0 }, DESC_END };

static char *EvtRelitBoutons[3] = { "Abandonner", "Continuer", "\0" };

static char *EvtRunName[MAXFICHIERS + 1];
static char *EvtSousListe[MAXFICHIERS + 1];

static char DemandeFichier,ANouveau;
static int EvtPremierSauve,EvtDernierSauve;
static float StrmStocke,StrmAffiche;
static char EvtUniteNom[MAXEVTNOM];
static PlotEspace EvtEspaceSelecte;
static Panel pEvtRunName,pEvtRunListSave,pEvtRunEvtSave,pEvtRunLoad;
static Panel pEvtNtupleEcrit,pEvtNtupleRelit,pEvtHdrAffichage;
static char FichierRuns[MAXFILENAME],FichierRunBis[MAXFILENAME];
static char SubsetPath[MAXFILENAME],FichierSubset[MAXFILENAME];
static Menu mEvtSelTous;
static Panel pCoupure; static unsigned char CoupOper,EvtMarque; static char AutreCoupure;
static Panel pRunDims;
static TypeFigZone ZoneNtuples,ZoneTraces,ZoneNav,ZoneEvtUnite,ZoneCoupures,ZoneEnvir; // ,ZoneCalib
static TypeFigZone ZoneAffichage,ZoneAffGeneral,ZoneAffEvts;
static Term tSelect,tCoupures,tListe;
static Info iEvtZoneConseil;

static float EnergieMoyenne;
static char EvtUniteModeManuel;
static Panel pEvtUniteCalcule,pEvtUniteZoneFit;

static Panel pCompteZone;
static float AireXmin,AireXmax,AireYmin,AireYmax,AireTaux;
static int AireNb;
static char AireTxtX[40],AireTxtY[40];

//+ extern Menu mProd;

//float ValiditeAjustement;

void TangoDefaults();
char TangoProdInit();
extern MenuItem iProdAffGeneral[];
extern MenuItem iProdAffEvts[];
int ProdAmplitudes();
void ProdVarDump(char *fctn, int n);
char ArchPaqMontre(int paq);
void ProdExit();
void EvtSelectionSauve(char *nom);

#ifdef WIN32
char *rindex(char *s, int c);
void EvtUniteInit();
#endif

/* ========================================================================== */
int SvrExited() { return(1); }
/* ========================================================================== */
int HttpTraiteRequete(char *url, char *demandeur, char *requete) {
	return(0);
}
/* ========================================================================== */
void TangoAttends(int ms) {
#ifndef macintosh
	#define DELAIS_UNIX
#endif
#ifdef DELAIS_UNIX
	usleep(ms*1000);
#else
	unsigned long temps,delai,decompte;
	unsigned long TickCount(void);
	temps = TickCount(); /* 1 tick = 1/60 seconde */
	delai = (unsigned long)ms * 60; if(delai < 1000) delai = 1; else delai /= 1000;
	delai += temps; decompte = 1000000000;
	while((TickCount() < delai) && decompte) decompte--;
#endif
}
/* ========================================================================== */
void TangoPrint(char niveau, char *fmt, ...) {
	char texte[256]; va_list argptr; int i;

	if(ModeScript) for(i=0; i<ScriptLoop+1; i++) printf("| ");
	if(fmt) {
		va_start(argptr, fmt);
		vsprintf(texte, fmt, argptr);
		va_end(argptr);
		if(AutoriseOpium) OpiumMessage(niveau,texte);
		else switch(niveau) {
		  case OPIUM_NOTE:  printf("* %s\n",texte); break;
		  case OPIUM_WARN:  printf("> %s\n",texte); break;
		  case OPIUM_ERROR: printf("! %s\n",texte); break;
		  default:          printf("%s\n",texte);
		}
	}
}
/* ========================================================================== */
FILE *TangoFileOpen(char *nom, char *mode) {
	FILE *f;
	
	while(ArchEvtKeepFree()) {
		if((f = fopen(nom,mode))) return(f);
		if(!ArchEvtFilesFull()) return(0);
	}
	return(0);
}
/* ========================================================================== */
int EvtLectMode() {
	if(MaxEvts) 
		OpiumReadKeyB("Mode de chargement","lecture prealable/ouverture seule",&EvtAuPif,20);
	else OpiumError("Evenements de longueur fixe: pas de lecture prealable");
	return(0);
}
/* ========================================================================== */
static int EvtRunInclude(char *nom) {
	int rc;
	char explic[256];

	sprintf(explic,"Incident indetermine depuis %s",__func__);
	if(!(rc = ArchRunOpenRange(nom,EventPartDebut,EventPartFin,1,explic))) {
		TangoPrint(OPIUM_ERROR,explic);
	}
	EvtDernierSauve = EvtNb / 2;
	return(rc);
}
/* ========================================================================== */
int EvtRunAdd() {
	if(EvtRunNb >= MAXFICHIERS) {
		TangoPrint(OPIUM_ERROR,"Deja %d fichiers ouverts! Fichier abandonne",MAXFICHIERS);
		return(0);
	}
	if(DemandeFichier) {
		if(ModeScript) { TangoPrint(OPIUM_ERROR,"Pas de run demande, pas d'analyse!"); return(0); }
//		if(OpiumReadFile("Fichier de sauvegarde",FichierEvents,MAXFILENAME) == PNL_CANCEL) {
		else if(OpiumExec(pEvtRunName->cdr) == PNL_CANCEL) {
			OpiumError("Ce fichier n'a PAS ete ajoute");
			return(0);
		}
	}

	EvtDejaPresents = EvtNb;
	if(!EvtRunInclude(FichierEvents)) {
		TangoPrint(OPIUM_ERROR,"Ce fichier n'a PAS pu etre ajoute");
		return(0);
	}

	return(1);
}
/* ========================================================================== */
int EvtRunAjoute(Menu menu, MenuItem *item) {
	DemandeFichier = 1;
	if(EvtRunAdd()) { if(Trigger.enservice) TangoProdInit(); }
	return(0);
}
/* ========================================================================== */
int EvtRunDel() {
	int run,evt; int evts_decales;
	off_t octets_decales;

	if(EvtRunNb < 1) {
		OpiumError("Pas de fichier ouvert!");
		return(0);
	}
	if(OpiumReadList("Nom du fichier a retirer",EvtRunName,&EvtRunNum,MAXFILENAME) == PNL_CANCEL) return(0);

	printf("* Retrait du run '%s'\n",EvtRunName[EvtRunNum]);
	if(EvtRunInfo[EvtRunNum].path >= 0) close(EvtRunInfo[EvtRunNum].path);
	if(EvtNb) PlotNtupleSupprime();
	if(EvtRunNum < (EvtRunNb - 1)) {
		octets_decales = EvtRunInfo[EvtRunNum + 1].debut - EvtRunInfo[EvtRunNum].debut;
		evts_decales = EvtRunInfo[EvtRunNum + 1].premier - EvtRunInfo[EvtRunNum].premier;
	} else {
		octets_decales = OctetsDejaLus - EvtRunInfo[EvtRunNum].debut;
		evts_decales = EvtNb - EvtRunInfo[EvtRunNum].premier;
	}
	OctetsDejaLus -= octets_decales;
	EvtNb -= evts_decales;
	if(MaxEvts) for(evt=EvtRunInfo[EvtRunNum].premier; evt<EvtNb; evt++) 
		EvtPos[evt] = EvtPos[evt + evts_decales] - octets_decales;
	printf(". Il reste %d evenements\n",EvtNb);
	EvtRunNb--;
	for(run=EvtRunNum; run<EvtRunNb; run++) {
		memcpy(EvtRunInfo+run,EvtRunInfo+run+1,sizeof(TypeEvtRun));
		EvtRunInfo[run].debut -= octets_decales;
		EvtRunInfo[run].premier -= evts_decales;
	}
	EvtRunInfo[EvtRunNb].nom[0] = '\0';
	printf(". Il reste %d fichiers\n",EvtRunNb);
	PlotNtupleCree(EvtNb,0.0);
	printf(". Le Ntuple (de %d evenements) a ete remis a %g\n",EvtNb,0.0);
	return(0);
}
/* ========================================================================== */
int EvtRunListDisplay() {
	off_t taille;
	int i,dernier,nb;

	if(!tListe) return(0);
	TermHold(tListe);
	TermEmpty(tListe);
	if(EvtRunNb == 0) TermPrint(tListe,"Aucun run en memoire\n");
	else {
		TermPrint(tListe,"nom [taille] : nb_evts/total @t0\n-------------------------\n\n");
		for(i=0; i<EvtRunNb; i++) {
			if(i < (EvtRunNb - 1)) {
				taille = EvtRunInfo[i + 1].debut - EvtRunInfo[i].debut;
				dernier = EvtRunInfo[i + 1].premier;
			} else {
				taille = OctetsDejaLus - EvtRunInfo[i].debut;
				dernier = EvtNb;
			}
			nb = dernier - EvtRunInfo[i].premier;
			TermPrint(tListe,"%-40s [%12.3f Mo] : %6d/%6d evts @%d,%06d\n",
				EvtRunInfo[i].nom,(float)taille/(1024.0*1024.0),nb,dernier,
				EvtRunInfo[i].t0sec,EvtRunInfo[i].t0msec);
			/*
			if(MaxEvts) {
				j = 0;
				for(evt = EvtRunInfo[i].premier; evt <dernier; evt++) {
					TermPrint(tListe," %ld",EvtPos[evt] - EvtRunInfo[i].debut);
					if(!(++j % 10)) TermPrint(tListe,"\n");
				}
				if(j % 10) TermPrint(tListe,"\n");
			} */
		}
		TermPrint(tListe,"=== (%d run(s)) ===\n",EvtRunNb);
	}
	TermRelease(tListe);
	OpiumDisplay(tListe->cdr);

	return(0);
}
/* ========================================================================== */
int EvtRunListSave() {
	off_t taille,dim;
	FILE *f; int i,j,l,lmax,evt,dernier,nb;

	strcat(strcpy(FichierRuns,EvtsPath),"liste");
	if(OpiumExec(pEvtRunListSave->cdr) == PNL_CANCEL) return(0);
	if(AjouteNtuple) f = TangoFileOpen(FichierRuns,"a"); else f = TangoFileOpen(FichierRuns,"w");
	if(f) {
		fprintf(f,"# (nom taille nb_evts t0)\n");
		for(i=0; i<EvtRunNb; i++) {
			if(i < (EvtRunNb - 1)) {
				taille = EvtRunInfo[i + 1].debut - EvtRunInfo[i].debut;
				dernier = EvtRunInfo[i + 1].premier;
			} else {
				taille = OctetsDejaLus - EvtRunInfo[i].debut;
				dernier = EvtNb;
			}
			nb = dernier - EvtRunInfo[i].premier;
#ifdef LONG_IS_32BITS
			lmax = fprintf(f,"=\"%s\" %lld %d %d,%06d\n",EvtRunInfo[i].nom,taille,nb,EvtRunInfo[i].t0sec,EvtRunInfo[i].t0msec);
#else
			lmax = fprintf(f,"=\"%s\" %ld %d %d,%06d\n",EvtRunInfo[i].nom,taille,nb,EvtRunInfo[i].t0sec,EvtRunInfo[i].t0msec);
#endif
			if(MaxEvts) {
				j = 0; l = 0;
				for(evt = EvtRunInfo[i].premier; evt <dernier; evt++) {
					if(l > (lmax - 10)) { fprintf(f,"\n"); l = 0; }
					if(EvtPos[evt] == -1) dim = -1;
					else dim = EvtPos[evt] - EvtRunInfo[i].debut;
#ifdef LONG_IS_32BITS
					l += fprintf(f," %lld",dim);
#else
					l += fprintf(f," %ld",dim);
#endif
//					if(!(++j % 10)) fprintf(f,"\n");
				}
//				if(j % 10) fprintf(f,"\n");
				fprintf(f,"\n");
			}
		}
		fclose(f);
		printf("* Catalogue %s dans '%s'\n",AjouteNtuple? "ajoute": "sauve",FichierRuns);
	} else {
		printf("- Le catalogue '%s' n'a pas pu etre cree\n. %s\n",FichierRuns,strerror(errno));
	}

	return(0);
}
/* ========================================================================== */
int EvtRunPurge() {
	/* voir ArchRunClose */
	int j;

	if(EvtNb) PlotNtupleSupprime();
	for(j=0; j<EvtRunNb; j++) if(EvtRunInfo[j].path >= 0) close(EvtRunInfo[j].path);
	EvtRunNb = 0;
	EvtNb = 0;
	OctetsDejaLus = 0;
	EvtRunInfo[EvtRunNb].nom[0] = '\0';
	printf("* Tous les runs ont ete decharges\n");
	return(0);
}
/* ========================================================================== */
int EvtRunLoad() {
	char pas_utile;
	char *c,*item; char ligne[256],*pos;
	FILE *f; int i,nb,evt; off_t taille;

	strcpy(FichierRuns,FichierEvents);
	c = rindex(FichierRuns,'.');
	if(c) *c = '\0';
	strcat(FichierRuns,".ctlg");
	if(OpiumExec(pEvtRunLoad->cdr) == PNL_CANCEL) return(0);
	if(!(f = TangoFileOpen(FichierRuns,"r"))) {
		printf(". Le catalogue '%s' n'a pas pu etre relu\n. %s\n",FichierRuns,strerror(errno));
		OpiumError("Catalogue illisible (%s)",strerror(errno));
		return(0);
	}
	if(AjouteNtuple) { if(EvtNb) PlotNtupleSupprime(); } else EvtRunPurge();
	printf("* Lecture du catalogue '%s'\n",FichierRuns);
	evt = 0; i = EvtRunNb;
	while(LigneSuivante(ligne,256,f)) {
		if(ligne[0] == '#') continue;
		if(EvtRunNb >= MAXFICHIERS) {
			OpiumError("Deja %d fichiers ouverts! Fichier abandonne",MAXFICHIERS);
			return(0);
		}
		if(ligne[0] == '=') {
			taille = 0;
			c = ligne + 1;
			item = ItemSuivant(&c,' ');
			i = EvtRunNb;
			strncpy(EvtRunInfo[i].nom,item,MAXFILENAME);
			if(i) { item = ItemSuivant(&c,' '); pas_utile = (*item !='\0'); }
			else pas_utile = 0;
			if(pas_utile) {
				EvtRunInfo[i].path = -1;
				EvtRunInfo[i].debut = OctetsDejaLus;
				EvtRunInfo[i].premier = EvtNb;
#ifdef LONG_IS_32BITS
				sscanf(item,"%lld",&taille);
#else
				sscanf(item,"%ld",&taille);
#endif
				item = ItemSuivant(&c,' '); sscanf(item,"%d",&nb);
				item = ItemSuivant(&c,','); sscanf(item,"%d",&(EvtRunInfo[i].t0sec));
				item = ItemSuivant(&c,' '); sscanf(item,"%d",&(EvtRunInfo[i].t0msec));
				OctetsDejaLus += taille;
				EvtNb += nb;
				EvtRunNb++;
				if(MaxEvts) {
					for(evt = EvtRunInfo[i].premier; evt<EvtNb; evt++) EvtPos[evt] = -1; /* EvtRunInfo[i].debut; */
				}
				printf("* Prise en compte du run #%d, '%s'\n",EvtRunNb,EvtRunInfo[i].nom);
#ifdef LONG_IS_32BITS
				printf(". Fichier de %lld octets (0x%08llX)\n",taille,taille);
#else
				printf(". Fichier de %ld octets (0x%08lX)\n",taille,taille);
#endif
				if(EvtRunNb > 1) printf(". Ajout de  %d evenements\n",nb);
				printf(". Au total, %d evenements\n",EvtNb);
			} else {
				if(!EvtRunInclude(EvtRunInfo[i].nom)) {
					printf("= Ce fichier NE SERA PAS pris en compte\n");
					continue;
				}
				/* Initialise le reste de la prod qui depend du contenu de l'archive */
				if(i == 0) {
					if(!ProdInit(ANouveau)) {
						OpiumError("Impossible d'initialiser la production");
						return(0);
					}
				}
				ANouveau = 1;
			}
			evt = EvtRunInfo[i].premier;
		} else {
			c = ligne + 1;
			do {
				pos = ItemSuivant(&c,' ');
				if(*pos == '\0') break;
				if(evt >= MaxEvts) {
					OpiumError("Catalogue deteriore (#evt > %d)",MaxEvts);
					break;
				}
				sscanf(pos,"%lld",&(EvtPos[evt]));
				EvtPos[evt] += EvtRunInfo[i].debut;
				evt++;
			} while(*c);
		}
	}
	fclose(f);
	EvtLitUnSeul(EvtNb-1,0);
	RunTempsTotal = ArchEvt.sec - RunTempsInitial;
	EvtRunInfo[EvtRunNb].nom[0] = '\0';
	PlotNtupleCree(EvtNb,0.0);
	printf(". Le Ntuple (de %d evenements) a ete remis a %g\n",EvtNb,0.0);
	if(!ProdInit(ANouveau)) {
		OpiumError("Impossible d'initialiser la production");
		return(0);
	}
	ANouveau = 1;

	return(0);
}		
/* ========================================================================== */
int EvtSuppEcrit(char *nom, char sel_evt, char ajoute) {
	int num,dim; FILE *f;
	
	if(ajoute) f = fopen(nom,"a"); else f = fopen(nom,"w");
	if(!f) return(0);
	
	fprintf(f,"num T(s) T(us)\n");
	dim = *(EvtEspace->dim);
	for(num=0; num<dim; num++) {
		if(sel_evt) { if(PlotNtupleExclu(EvtEspace,num)) continue; }
		fprintf(f,"%d %12d %06d\n",num,EvtDateExacte[num].t.s,EvtDateExacte[num].t.us);
	}
	fclose(f);
	return(1);
}
/* ========================================================================== */
int EvtSuppRelit(char *nom, PlotEspace espace) {
	FILE *f;
#define LIGNE_MAX 8192
	char ligne[LIGNE_MAX]; char *c,*item;
	int i,n,num,dim;
	
	dim = *(espace->dim);
	f = fopen(nom,"r");
	if(!f) {
		OpiumFail("Fichier %s illisible (%s)",nom,strerror(errno));
		return(0);
	}
	printf("* Relecture du fichier des infos TANGO: %s\n",nom);
	n = -1; // pour ne pas compter l'entete
	while(LigneSuivante(ligne,LIGNE_MAX,f)) {
		if(ligne[0] == '#') continue;
		c = ligne; i = 0;
		if(n >= 0) do {
			item = ItemJusquA(' ',&c);
			if(*item == '\0') break;
			switch(i) {
			  case 0: sscanf(item,"%d",&num); break;
			  case 1: if((num >= 0) && (num < dim)) sscanf(item,"%d",&(EvtDateExacte[num].t.s)); break;
			  case 2: if((num >= 0) && (num < dim)) sscanf(item,"%d",&(EvtDateExacte[num].t.us)); break;
			}
			i++;
		} while(1);
		n++;
	}
	fclose(f);
	printf("  > Relu %d info%s (total: %d info%s)\n",Accord1s(n),Accord1s(dim));
	return(n+1);
}
/* ========================================================================== */
int EvtNtupleAffiche() {
	int evt;

	evt = EvtDemande;
	if(OpiumReadInt("Numero",&evt) == PNL_CANCEL) return(0);
	if(evt < 1) return(0);
	PlotNtupleAffiche(EvtEspace,evt-1,-1,-1,-1,-1);
	EvtDemande = evt;
	AfficheNtuple = 1;
	return(0);
}
/* ========================================================================== */
int EvtNtupleEcrit() {
	FILE *f; char avec_version,ajoute,a_faire;

	//-	if(!ModeScript) { if(OpiumExec(pEvtNtupleEcrit->cdr) == PNL_CANCEL) return(0); }
	avec_version = 0;
reouvre_ntp:
	if(!ArchEvtKeepFree()) return(0);
	ajoute = AjouteNtuple; a_faire = 1;
	if(avec_version) {
		if(AjouteNtuple) f = fopen(FichierNtuple,"a"); else f = fopen(FichierNtuple,"w");
		if(f) {
			fprintf(f,"# N-tuple cree par Tango version %s, date: 20%s\n",TangoVersion,DateJour());
			fclose(f);
			ajoute = 1;
		} else a_faire = 0;
	}
	if(a_faire) {
		PlotNtupleSeparateur(Separateur);
		if(PlotNtupleEcrit(FichierNtuple,SelecteVar,SelecteEvt,ajoute)) {
			printf("* Ntuple %s dans '%s'\n",AjouteNtuple? "ajoute": "sauve",FichierNtuple);
			a_faire = 0;
			if(!ModeScript && (EvtRunNb > 1)) {
				if(OpiumCheck("Veux-tu sauver la liste des runs?")) EvtRunListSave();
			}
		}
	}
	if(a_faire) {
		if(ArchEvtFilesFull()) goto reouvre_ntp;
		TangoPrint(OPIUM_ERROR,"Ecriture des ntuples impossible (%s)",strerror(errno));
	}

	if(EvtDateExacte) {
	reouvre_tgo:
		if(!ArchEvtKeepFree()) return(0);
		if(AjouteNtuple) f = fopen(FichierSupp,"a"); else f = fopen(FichierSupp,"w");
		if(f) {
			fprintf(f,"# Infos crees par Tango version %s, date: 20%s\n",TangoVersion,DateJour());
			fclose(f);
			if(EvtSuppEcrit(FichierSupp,SelecteEvt,1)) {
				printf("* Infos TANGO %s dans '%s'\n",AjouteNtuple? "ajoutees": "sauvees",FichierSupp);
			} else f = 0;
		}
		if(!f) {
			if(ArchEvtFilesFull()) goto reouvre_tgo;
			TangoPrint(OPIUM_WARN,"Ecriture des infos TANGO impossible (%s)",strerror(errno));
		}
	}

	return(0);
}
/* ========================================================================== */
int EvtNtupleProfond() {
	PlotEspace espace; int max;
	char texte[256];

/* En fait, tous les espaces vont avoir la meme profondeur */
	if(PlotEspaceNb > 1) {
		if(OpiumReadList("Espace",PlotEspacesList,&EspaceNum,MAXNOMESPACE) == PNL_CANCEL) return(0);
	} else EspaceNum = 0;
	espace = PlotEspacePtr[EspaceNum];
	max = espace->max;
	sprintf(texte,"#%s maxi attendu",espace->nom);
	if(OpiumReadInt(texte,&max) == PNL_CANCEL) return(0);
	if(max != espace->max) {
		if(espace->max) PlotNtupleSupprime();
		PlotNtupleCree(max,0.0);
		printf("* Le Ntuple (de %d %s) a ete remis a %g\n",espace->max,espace->nom,0.0);
	}
	return(0);
}
/* ========================================================================== */
int EvtNtupleRelit() {
	/* Initialise le reste de la prod qui depend du contenu de l'archive */

//	if(OpiumExec(pEvtNtupleRelit->cdr) == PNL_CANCEL) return(0);
	if(!ProdInit(ANouveau)) {
		TangoPrint(OPIUM_ERROR,"Impossible d'initialiser la production");
		return(0);
	}
	ANouveau = 1;

	if(PlotNtupleRelit(FichierNtuple,AjouteNtuple)) {
		EvtsAnalyses = *(EvtEspace->dim);
		PaqNb = *(PaqEspace->dim);
		if(AutoriseOpium) { if(OpiumDisplayed(pEvtAnalyses->cdr)) PanelRefreshVars(pEvtAnalyses); }
	} else TangoPrint(OPIUM_WARN,"Pas de Ntuple disponible");

	if(EvtDateExacte) {
		if(!EvtSuppRelit(FichierSupp,EvtEspace)) TangoPrint(OPIUM_WARN,"Pas d'info TANGO disponible");
	}

	return(0);
}
#ifdef ETALONNE
/* ========================================================================== */
int Etalonne() {
	int p;
	float reference,coef,*adrs; int i,dim;

	if((p = PlotChoixFig1D()) < 0) return(0);
	reference = ResultatFit.gau.x0;
	if(OpiumReadFloat("Valeur visee",&reference) == PNL_CANCEL) return(0);
	coef = reference / ResultatFit.gau.x0;
	adrs = *PlotVar[Plot[p].ab6].adrs;
	dim = *PlotVar[Plot[p].ab6].dim;
	for(i=0; i<dim; i++,adrs++) *adrs *= coef;
	return(0);
}
#endif
/* ========================================================================== */
char EvtLitUnSeul(int evt, char affiche) {
	char rc; char explic[256];

	rc = ArchEvtGet(evt,explic);
	if(affiche) {
		if(rc == 0) TangoPrint(OPIUM_WARN,"Fin de fichier rencontree");
		else if(rc < 0) {
			TangoPrint(OPIUM_ERROR,"Lecture de l'evenement %d en erreur",evt);
		}
	}
	if(rc <= 0) TangoPrint(OPIUM_WARN,explic);
	return(rc);
}
/* ========================================================================== */
int EvtLitASuivre(int evt, char affiche) {
	int rc; char explic[256];

	rc = ArchEvtGet(evt,explic);
	if((rc <= 0) && affiche) printf("! %s\n",explic);
	if(rc == 0) { if(evt < EvtNb) EvtNb = evt; }
	else if((rc < 0) && affiche) {
		if(OpiumChoix(2,EvtRelitBoutons,"Lecture de l'evenement %d en erreur (voir journal)",evt)) rc = -1;
	}
	return(rc);
}
/* ========================================================================== */
static void EvtMontre(int evt) { ArchEvtMontre(evt); }
/* ========================================================================== */
static char PaqMontre(int paq) { return(ArchPaqMontre(paq)); }
/* ========================================================================== */
int EvtChoix() {
	int evt; int i,j,n,num; int numero[MAXFICHIERS];

	evt = EvtDemande;
	if(EvtRunNb == 1) {
		i = 0;
		num = (evt - EvtRunInfo[i].premier) + EvtRunInfo[i].num0;
		if(OpiumReadInt("Evenement no",&num) == PNL_CANCEL) return(0);
		evt = (num - EvtRunInfo[i].num0) + EvtRunInfo[i].premier;
	} else {
		for(i=0; i<EvtRunNb; i++) if(EvtRunInfo[i].premier > evt) break;
		if(i == 0) EvtDemande = evt = EvtRunInfo[i].premier;
		else --i;
		num = (evt - EvtRunInfo[i].premier) + EvtRunInfo[i].num0;
		if(OpiumReadInt("Evenement no",&num) == PNL_CANCEL) return(0);
//1		printf("* Recherche de l'evenement SAMBA #%d\n",num);
		n = 0;
		for(i=0; i<EvtRunNb; i++) {
			if((EvtRunInfo[i].num0 <= num) && (EvtRunInfo[i].numN > num)) {
				EvtSousListe[n] = EvtRunInfo[i].fichier;
//1				printf("- trouve[%d] dans %s qui contient [%d .. %d[\n",
//1					n,EvtSousListe[n],EvtRunInfo[i].num0,EvtRunInfo[i].numN);
				numero[n] = i;
				n++;
			}
		}
		if(n == 0) {
			for(i=0; i<EvtRunNb; i++) if(EvtRunInfo[i].num0 > num) break;
			if(i == 0) {
				OpiumError("Grosse erreur systeme!!! num0[0]=%d > num=%d",
						   EvtRunInfo[i].num0,num);
				return(0);
			}
			--i;
		} else if(n == 1) i = numero[n-1];
		else {
			EvtSousListe[n] = "\0"; j = 0;
			if(OpiumReadList("Celui du run",EvtSousListe,&j,16) == PNL_CANCEL) return(0);
			i = numero[j];
		}
		evt = (num - EvtRunInfo[i].num0) + EvtRunInfo[i].premier;
//1		printf("- fichier prevu: %d/%d (%d->#%d soit %d->#%d)\n",i,n,
//1			   EvtRunInfo[i].premier,EvtRunInfo[i].num0,evt,num);
	}
	if(EvtLitUnSeul(evt,1)) EvtMontre(evt);
	else OpiumFail("Evenement #%d inutilisable (numero maxi: %d)",evt,EvtNb);
	return(0);
}
/* ========================================================================== */
int EvtPrecedent() {
	int evt;

	if(EvtDemande > 0) {
		evt = EvtDemande - 1;
		if(EvtLitUnSeul(evt,1)) EvtMontre(evt);
		else OpiumFail("Impossible de recuperer l'evenement #%d",evt);
	} else OpiumWarn("C'etait le premier evenement");
	return(0);
}
/* ========================================================================== */
int EvtSuivant() {
	int evt;

	if(EvtDemande < EvtNb) {
		evt = EvtDemande + 1;
		if(EvtLitUnSeul(evt,1)) EvtMontre(evt);
		else OpiumFail("Impossible de recuperer l'evenement #%d",evt);
	} else OpiumWarn("C'etait le dernier evenement (numero maxi: %d)",EvtNb);
	return(0);
}
/* ========================================================================== */
int EvtDefile() {
	int num;
	char prompt[40];
	char encore;

/* Boucle sur tous les evenements enregistres */
	num = EvtDemande;
	strcpy(prompt,"Evenement suivant?");
	while((encore = EvtLitASuivre(++num,1))) {
		EvtMontre(num);
		if(!OpiumCheck(prompt)) break;
	}
	if(!encore) OpiumNote("Fichier termine");
	return(0);
}
/* ========================================================================== */
int EvtAuto() {
	int evt,delai;
	char encore;

	delai = 1000;
	if(OpiumReadInt("Attente entre evenements (ms)",&delai) == PNL_CANCEL) return(0);
	OpiumProgresTitle("Evenements");
	OpiumProgresInit(EvtNb);
	evt = EvtDemande;
	while((encore = EvtLitASuivre(evt,1))) {
		if(!OpiumProgresRefresh(evt)) break;
		EvtMontre(evt);
		TangoAttends(delai);
		evt++;
	}
	OpiumProgresClose();
	if(!encore) OpiumNote("Fichier termine");
	return(0);
}
/* ========================================================================== */
int NtuplePointe() {
	int ntp,evt; char rc; PlotEspace espace = 0;

	ntp = PlotNtuplePointe(&espace);
	if(ntp < 0) { OpiumError("%s: aucun trouve",espace->nom); return(0); }
	rc = 0;
	if(espace->num == EvtEspace->num) {
		evt = ntp;
		if((rc = EvtLitUnSeul(evt,1))) EvtMontre(evt);
	} else if(espace->num == PaqEspace->num) rc = PaqMontre(ntp);
	if(!rc) OpiumError("%s: #%d inutilisable (numero maxi: %d)",espace->nom,ntp,*(espace->dim));
	return(0);
}
/* ========================================================================== */
int EvtCompteZone() {
	int p; int posx,posy,larg,haut,px,py,dy;
	int ix0,iy0,ix1,iy1;
	int evt; float *x,*y;
	PlotEspace espace; int dim,nb;
	char texte[255];
	
	if((p = PlotChoixFig2D()) < 0) return(0);
	OpiumGetPosition((Plot[p].g)->cdr,&posx,&posy); OpiumGetSize((Plot[p].g)->cdr,&larg,&haut);
	px = posx + larg + WndColToPix(3);
	dy = WndLigToPix(6);
	if(posy > dy) py = posy - dy; else py = posy + haut + WndLigToPix(3);
	InfoWrite(iEvtZoneConseil,1,"Pointes d'abord le coin haut-gauche,");
	InfoWrite(iEvtZoneConseil,2,"et termines par le coin bas-droit");
	OpiumPosition(iEvtZoneConseil->cdr,posx,py);
	OpiumDisplay(iEvtZoneConseil->cdr);
	GraphGetRect(Plot[p].g,&ix0,&AireXmin,&iy0,&AireYmin,&ix1,&AireXmax,&iy1,&AireYmax);
	OpiumClear(iEvtZoneConseil->cdr);
	
	espace = PlotVar[Plot[p].ab6].espace;
	dim = *(espace->dim);
	x = *PlotVar[Plot[p].ab6].adrs;
	y = *PlotVar[Plot[p].ord].adrs;
	nb = 0;
	for(evt=0; evt<dim; evt++,x++,y++) {
		if((*x >= AireXmin) && (*x <= AireXmax)
		&& (*y >= AireYmin) && (*y <= AireYmax)) nb++;
	}
	sprintf(texte,"%d evenement%s dans [%g < %s < %g], [%g < %s < %g] (%.3g Hz)",
		Accord1s(nb),AireXmin,PlotVarList[Plot[p].ab6],AireXmax,
		AireYmin,PlotVarList[Plot[p].ord],AireYmax,
		RunTempsTotal? (float)nb/(float)RunTempsTotal: 0);
	printf("* %s\n",texte);
	sprintf(AireTxtX,"%s min",PlotVarList[Plot[p].ab6]);
	sprintf(AireTxtY,"%s min",PlotVarList[Plot[p].ord]);
	AireNb = nb; AireTaux = RunTempsTotal? (float)nb / (float)RunTempsTotal: 0.0;
	OpiumPosition(pCompteZone->cdr,px,posy);
	if(OpiumAlEcran(pCompteZone)) OpiumRefresh(pCompteZone->cdr);
	else OpiumDisplay(pCompteZone->cdr);
	
	return(0);
}
/* ========================================================================== */
int EvtDemandeMontre() {
	int evt;
	int p; int x,y; float *adrs;
	Graph graph;
	
	// for(p=0; p<PlotsNb; p++) {
	//	printf("(%s) %s: %s %s, %s\n",__func__,Plot[p].titre,PlotTypeTexte[Plot[p].type],(Plot[p].vide)?"vide":"utilisable",OpiumDisplayed((Plot[p].g)->cdr)?"affiche":"masque");
	// }
	evt = EvtDemande;
	for(p=0; p<PlotsNb; p++) if(!Plot[p].vide && ((Plot[p].type == PLOT_H2D) || (Plot[p].type == PLOT_BIPLOT)) && OpiumDisplayed((Plot[p].g)->cdr)) {
		// printf("(%s) %s concerne\n",__func__,Plot[p].titre);
		graph = Plot[p].g;
		adrs = *PlotVar[Plot[p].ab6].adrs;
		x = GraphScreenXFloat(graph,adrs[evt]);
		adrs = *PlotVar[Plot[p].ord].adrs;
		y = GraphScreenYFloat(graph,adrs[evt]);
		if((x >= 0) && (x < (graph->cdr)->dh)
		   && (y >= 0) && (y < (graph->cdr)->dv)) {
			WndFrame f; WndContextPtr gc; char legende[32];
			if(OpiumCoordFenetre(graph->cdr,&f)) {
				// printf("(%s) %s trace\n",__func__,Plot[p].titre);
				gc = GRF_AJOUT; WndContextFgndRGB(f,gc,GRF_RGB_YELLOW);
				WndLine(f,gc,x-10,y,20,0);
				WndLine(f,gc,x,y-10,0,20);
				sprintf(legende,"%d",ArchEvt.num);
				WndString(f,gc,x+2,y,legende);
			} // else printf("(%s) %s introuvable\n",__func__,Plot[p].titre);
		}
	}
	return(0);
}
/* ========================================================================== */
char EvtAdmis(int evt) {
	if(EvtEspaceSelecte) return(PlotNtupleInclu(EvtEspaceSelecte,evt));
	else return(0);
}
/* ========================================================================== */
int EvtCoupeResultat(PlotEspace espace) {
	int evt,dim;
	// ajustement des delais: fait par ProdCalculeDelai()

	dim = *(espace->dim);
	EvtValides = 0;
	for(evt=0; evt<dim; evt++) if(!PlotNtupleExclu(espace,evt)) EvtValides++;
	if(EvtNb > 0) EvtEfficacite = 100.0 * EvtValides / EvtNb;
	else EvtEfficacite = 0.0;
	if(RunTempsTotal > 0.0) EvtTaux = EvtValides / (float)RunTempsTotal;
	else EvtTaux = 0.0;
	if(AutoriseOpium) { if(OpiumDisplayed(pEvtValides->cdr)) PanelRefreshVars(pEvtValides); }
	
	return(EvtValides);
}
/* ========================================================================== */
void EvtVarCalcule(int evt, int var1, unsigned char oper, int var2, float *resultat) {
    float *adrs1,*adrs2,*adrs3;
//    PlotEspace espace; int dim;

//    espace = PlotVar[var1].espace;
//    if(espace != PlotVar[var2].espace) return;
//    dim = *(espace->dim);

	if(!PlotVar) { printf("(%s) Variables @%08X: indefinies\n",__func__,(hexa)PlotVar); return; }
	if(!(PlotVar[var1].adrs) || !(PlotVar[var2].adrs)) {
		printf("(%s) Calcul impossible: @%08X = @%08X %c @%08X\n",__func__,(hexa)resultat,(hexa)PlotVar[var1].adrs,Opers[oper],(hexa)PlotVar[var2].adrs);
		return;
	}
    adrs1 = *PlotVar[var1].adrs + evt;
    adrs2 = *PlotVar[var2].adrs + evt;
    adrs3 = resultat + evt;
//-    if(evt < 10) printf("(%s) calcule @%08X[%d] (@%08X) = %g %c %g",__func__,(hexa)resultat,evt,(hexa)adrs3,*adr1,Opers[2*oper],*adrs2);
    switch(oper) {
      case 0: *adrs3 = *adrs1 + *adrs2; break;
      case 1: *adrs3 = *adrs1 - *adrs2; break;
      case 2: *adrs3 = *adrs1 * *adrs2; break;
      case 3: if(*adrs2 != 0.0) *adrs3 = *adrs1 / *adrs2; else *adrs3 = 0.0; break;
    }
//-    printf(" = %g\n",*adrs3);
}
/* ========================================================================== */
void EvtCoupeOpere(unsigned char oper, int numvar, float mini, float maxi, unsigned char marque) {
	int evt; float *adrs;
	PlotEspace espace; int dim;

//	printf(". Coupure a effectuer: %g < %s < %g\n",VarMini,PlotVarList[NumVar],VarMaxi);
	adrs = *PlotVar[numvar].adrs;
	espace = PlotVar[numvar].espace;
	dim = *(espace->dim);
//	printf(". Variable @%08X[%d]\n",(hexa)adrs,dim);
	switch(oper) {
	  case OPER_NOUVEAU: /* nouvelle coupure */
		for(evt=0; evt<dim; evt++,adrs++) {
			if((*adrs >= mini) && (*adrs  <=  maxi)) PlotNtupleMarque(espace,evt,MARQUE_NORMAL);
			else PlotNtupleMarque(espace,evt,marque);
		}
		EvtEspaceSelecte = espace;
		break;
	  case OPER_ET: /* coupure precedente ET celle qui vient */
		for(evt=0; evt<dim; evt++,adrs++) if(!PlotNtupleExclu(espace,evt)) {
			if((*adrs < mini) || (*adrs  >  maxi)) PlotNtupleMarque(espace,evt,marque);
		}
		break;
	  case OPER_OU: /* coupure precedente OU celle qui vient */
		for(evt=0; evt<dim; evt++,adrs++) if(PlotNtupleExclu(espace,evt)) {
			if((*adrs >= mini) && (*adrs  <=  maxi)) PlotNtupleMarque(espace,evt,MARQUE_NORMAL);
		}
		break;
	  case OPER_TOUT: /* tout recuperer */
			for(evt=0; evt<dim; evt++) {
				PlotNtupleMarque(espace,evt,MARQUE_NORMAL);
			}
		EvtEspaceSelecte = espace;
		break;
	}
	EvtCoupeResultat(espace);
}
/* ========================================================================== */
void EvtCoupure(unsigned char oper, int numvar, float mini, float maxi, unsigned char marque) {

	EvtCoupeOpere(oper,numvar,mini,maxi,marque);
	switch(oper) {
	  case OPER_NOUVEAU: /* nouvelle coupure */
		SelectNb = 0;
		CoupOper = 1;
		break;
	  case OPER_TOUT: /* tout recuperer */
		SelectNb = -1;
		CoupOper = 0;
		AutreCoupure = 0;
		break;
	}
	if(SelectNb < MAXSELECTIONS) {
		if(SelectNb >= 0) {
			Coupure[SelectNb].oper = oper; Coupure[SelectNb].var = numvar;
			Coupure[SelectNb].min = mini; Coupure[SelectNb].max = maxi;
			Coupure[SelectNb].marque = marque;
			Coupure[SelectNb].valides = EvtValides;
			Coupure[SelectNb].total = *(PlotVar[numvar].espace->dim);
		}
		SelectNb++;
	} else TangoPrint(OPIUM_ERROR,"Coupure non enregistree");
}
/* ========================================================================== */
char EvtCoupeSur(unsigned char oper, char *nom, float mini, float maxi, char imprime) {
	int num;

	if(oper == OPER_TOUT) {
		num = 0;
	} else if((num = PlotIndexVar(nom)) < 0) {
		TangoPrint(OPIUM_ERROR,"Variable non definie: '%s'",nom);
		return(0);
	}
	EvtCoupure(oper,num,mini,maxi,MARQUE_ABSENT);
	if(imprime) printf(". %d evenement%s valide%s par %g < %s < %g\n",Accord2s(EvtValides),mini,nom,maxi);
	return(1);
}
/* ========================================================================== */
static void EvtCoupeReporte(Term term, char conclusion) {
	int i;

	if(AutoriseOpium) {
		TermEmpty(term);
		TermHold(term);
		TermPrint(term,"\n");
		if(SelectNb == 0) { if(conclusion) TermPrint(term,"=== Aucune coupure ===\n"); }
		else {
			TermPrint(term," %s     %16s dans [%g .. %g]: %d/%d",
					  (SelectNb > 1)? "1/":"  ",PlotVarList[Coupure[0].var],Coupure[0].min,Coupure[0].max,Coupure[0].valides,Coupure[0].total);
			if(Coupure[0].marque != MARQUE_ABSENT) TermPrint(term," (exclus: marques %s)",TexteMarquage[Coupure[0].marque]);
			TermPrint(term,"\n");
			for(i=1; i<SelectNb; i++) {
				TermPrint(term,"%2d/  %s %16s dans [%g .. %g]: %d/%d",
						  i+1,(Coupure[i].oper == 1)? "et": "ou",PlotVarList[Coupure[i].var],Coupure[i].min,Coupure[i].max,Coupure[i].valides,Coupure[i].total);
				if(Coupure[i].marque != MARQUE_ABSENT) TermPrint(term," (exclus: marques %s)",TexteMarquage[Coupure[i].marque]);
				TermPrint(term,"\n");
			}
			if(!conclusion) TermPrint(term,"\n");
		}
		if(conclusion) TermPrint(term,"=== (%d evenement%s valide%s) ===\n",Accord2s(EvtValides));
		else TermPrint(term,"(%d coupure%s)\n",Accord1s(SelectNb));
		TermRelease(term);
		if(OpiumDisplayed(term->cdr)) OpiumRefresh(term->cdr); else OpiumDisplay(term->cdr);
	}
}
/* ========================================================================== */
void EvtCoupeRefresh() {
	int i;

	if(AutoriseOpium) {
		PlotRefreshSiUniqueBiplot();
		if(tCoupures) EvtCoupeReporte(tCoupures,0);
		else if(tSelect) EvtCoupeReporte(tSelect,1);
	}
	 
	if(SelectNb == 0) printf("  > Coupure courante: aucune\n");
	else {
		printf("  > Coupure courante:\n");
		printf("    |   1/     %16s dans [%g .. %g]: %d/%d (marque exclus: %s)\n",PlotVarList[Coupure[0].var],
			Coupure[0].min,Coupure[0].max,Coupure[0].valides,Coupure[0].total,TexteMarquage[Coupure[0].marque]);
		for(i=1; i<SelectNb; i++) {
			printf("    |  %2d/  %s %16s dans [%g .. %g]: %d/%d (marque exclus: %s)\n",i+1,
				(Coupure[i].oper == 1)? "et": "ou",
				PlotVarList[Coupure[i].var],Coupure[i].min,Coupure[i].max,Coupure[i].valides,Coupure[i].total,
				TexteMarquage[Coupure[i].marque]);
		}
	}
	printf("  ==  %d evenement%s valide%s",Accord2s(EvtValides));
}
/* ========================================================================== */
int EvtCoupeAdd() {
	do { // int i;
//		printf(". Coupure selon les %d variables suivantes:\n",PlotVarNb);
//		i = 0; while(PlotVarList[i][0]) { printf("  | %2d: '%s'\n",i,PlotVarList[i]); if(++i > 99) break; }
//		if(OpiumReadInt("No variable",&NumVar) == PNL_CANCEL) return(0);
		if(OpiumExec(pCoupure->cdr) == PNL_CANCEL) return(0);
		printf(". Coupure demandee: %g < %s < %g\n",VarMini,PlotVarList[NumVar],VarMaxi);
//		if(OpiumReadInt("No variable",&NumVar) == PNL_CANCEL) return(0);
		EvtCoupure(CoupOper,NumVar,VarMini,VarMaxi,EvtMarque);
		EvtCoupeRefresh();
	} while(AutreCoupure);
	return(0);
}
/* ========================================================================== */
int EvtCoupeZone() {
	int p; int posx,posy,larg,haut,px,py,dy;
	int ix0,iy0,ix1,iy1;
	
	if((p = PlotChoixFig2D()) < 0) return(0);
	OpiumGetPosition((Plot[p].g)->cdr,&posx,&posy); OpiumGetSize((Plot[p].g)->cdr,&larg,&haut);
	px = posx + larg + WndColToPix(3);
	dy = WndLigToPix(6);
	if(posy > dy) py = posy - dy; else py = posy + haut + WndLigToPix(3);
	InfoWrite(iEvtZoneConseil,1,"Pointes d'abord le coin haut-gauche,");
	InfoWrite(iEvtZoneConseil,2,"et termines par le coin bas-droit");
	OpiumPosition(iEvtZoneConseil->cdr,posx,py);
	OpiumDisplay(iEvtZoneConseil->cdr);
	GraphGetRect(Plot[p].g,&ix0,&AireXmin,&iy0,&AireYmin,&ix1,&AireXmax,&iy1,&AireYmax);
	OpiumClear(iEvtZoneConseil->cdr);

	EvtMarque = MARQUE_ABSENT;
	CoupOper = SelectNb? OPER_ET: OPER_NOUVEAU; 
	NumVar = Plot[p].ab6; VarMini = AireXmin; VarMaxi = AireXmax; 
	printf(". Coupure demandee: %g < %s < %g\n",VarMini,PlotVarList[NumVar],VarMaxi);
	EvtCoupure(CoupOper,NumVar,VarMini,VarMaxi,EvtMarque);
	CoupOper = OPER_ET; 
	NumVar = Plot[p].ord; VarMini = AireYmin; VarMaxi = AireYmax; 
	printf(". Coupure demandee: %g < %s < %g\n",VarMini,PlotVarList[NumVar],VarMaxi);
	EvtCoupure(CoupOper,NumVar,VarMini,VarMaxi,EvtMarque);
	EvtCoupeRefresh();

	return(0);
}
/* ========================================================================== */
int EvtCoupeRemove() {
	int num,i;

	num = SelectNb;
	if(OpiumReadInt("Laquelle",&num) == PNL_CANCEL) return(0);
	if((num <= 0) || (num > SelectNb)) {
		OpiumError("Pas de coupure #%d! (maxi:%d)",num,SelectNb);
		return(0);
	}
	num--;
	SelectNb--;
	for(i=num; i<SelectNb; i++)
		memcpy(&(Coupure[i]),&(Coupure[i+1]),sizeof(TypeCoupure));
	Coupure[0].oper = OPER_NOUVEAU;
	for(i=0; i<SelectNb; i++) {
		EvtCoupeOpere(Coupure[i].oper,Coupure[i].var,
			Coupure[i].min,Coupure[i].max,Coupure[i].marque);
	}
	EvtCoupeRefresh();
	return(0);
}
/* ========================================================================== */
int EvtCoupeRAZ() {
	if(SelectNb > 0) {
		EvtCoupure(OPER_TOUT,Coupure[SelectNb - 1].var,0,0,OPER_TOUT);
		EvtCoupeRefresh();
	}
	return(0);
}
/* ========================================================================== */
int EvtCoupeSauve() {
	char *liste[MAXSELECTIONS+1],nom[NOMMAX]; int nb,i,j;
	char nom_complet[MAXFILENAME];
	
	if(!SelectNb) { OpiumError("Pas de coupure a sauver"); return(0); }
	RepertoireListeCree(0,CoupePath,liste,MAXSELECTIONS,&nb);
	if(nb) {
		liste[nb] = "(nouveau)";
		liste[nb+1] = "\0";
		i = nb - 1; // nb;
		if(OpiumReadList("Jeu de coupures",liste,&i,NOMMAX) == PNL_CANCEL) return(0);
	} else i = 0;
	if(i == nb) {
		sprintf(nom,"Jeu_%02d",i+1);
		if(OpiumReadText("Nom du jeu de coupures",nom,NOMMAX) == PNL_CANCEL) return(0);
		for(j=0; j<nb; j++) if(!strcmp(liste[j],nom)) {
			OpiumError("Ce nom existe deja!");
			RepertoireListeLibere(liste,nb-1);
			return(0);
		}
		liste[nb] = nom;
	}
	strcat(strcpy(nom_complet,CoupePath),liste[i]);
	RepertoireCreeRacine(nom_complet);
	if(ArgPrint(nom_complet,JeuCoupureDesc) <= 0) printf("! Sauvegarde impossible (%s)\n",strerror(errno));
	else printf("* %d coupure%s sauvee%s dans '%s'\n",Accord2s(SelectNb),nom_complet);

	RepertoireListeLibere(liste,nb-1); // erreur si free("\0")
	return(0);
}
/* ========================================================================== */
int EvtCoupeJeu(char *nom) {
	int i; char nom_complet[MAXFILENAME];
	
	SelectNb = 0;
	strcat(strcpy(nom_complet,CoupePath),nom);
	errno = 0;
	if(ArgScan(nom_complet,JeuCoupureDesc) <= 0) TangoPrint(OPIUM_ERROR,"Relecture de %s en erreur (%s)",nom_complet,strerror(errno));
	else TangoPrint(OPIUM_NOTE,"%d coupure%s relue%s dans '%s'",Accord2s(SelectNb),nom_complet);
	Coupure[0].oper = OPER_NOUVEAU;
	for(i=0; i<SelectNb; i++) {
		EvtCoupeOpere(Coupure[i].oper,Coupure[i].var,Coupure[i].min,Coupure[i].max,Coupure[i].marque);
		Coupure[i].valides = EvtValides; Coupure[i].total = *(PlotVar[Coupure[i].var].espace->dim);
	}
	EvtCoupeRefresh();
	
	return(0);
}
/* ========================================================================== */
int EvtCoupeRelit() {
	char *liste[MAXSELECTIONS+1]; int nb,i;

	RepertoireListeCree(0,CoupePath,liste,MAXSELECTIONS,&nb);
	if(!nb) { OpiumError("Pas de jeu de coupures sauve"); return(0); }
	i = 0;
	if(OpiumReadList("Jeu de coupures",liste,&i,NOMMAX) == PNL_OK) EvtCoupeJeu(liste[i]);
	RepertoireListeLibere(liste,nb);

	return(0);
}
/* ========================================================================== */
int EvtCoupeSubset(Menu menu, MenuItem *item) {
	strcpy(FichierSubset,EventName);
	if(OpiumReadText("Subset name",FichierSubset,80) == PNL_OK) EvtSelectionSauve(FichierSubset);
	return(0);
}
/* ========================================================================== */
void EvtSelectionSauve(char *nom) {
	int evt,vt,voie,vm; int i;
	char explaination[256];  /* reason of a failure in the evtread library  */
	char nom_complet[MAXFILENAME]; FILE *f;

	strcat(strcpy(nom_complet,SubsetPath),nom);
	RepertoireCreeRacine(nom_complet);
	if(!(f = fopen(nom_complet,"w"))) {
		TangoPrint(OPIUM_ERROR,"Unable to write to %s (%s)",nom,strerror(errno));
		TangoPrint(OPIUM_WARN,"Destination was %s",SubsetPath);
		return;
	}

	for(evt=0; evt<EvtNb; evt++) if(EvtAdmis(evt)) {
		if(!ArchEvtGet(evt,explaination)) {
			fprintf(f,"! Event #%d unreadable: %s\n",evt,explaination);
			continue;
		}
		fprintf(f,"Event #%d occured at %d.%06d s (trigged on detector %5s, channel '%s')\n",
				ArchEvt.num,ArchEvt.sec,ArchEvt.msec,Bolo[ArchEvt.bolo_hdr].nom,
				VoieManip[ArchEvt.voie_hdr].nom);
		fprintf(f,"  %d channels recorded\n",ArchEvt.nbvoies);
		for(vt=0; vt<ArchEvt.nbvoies; vt++) {
			voie = ArchEvt.voie[vt].num;
			vm = VoieManip[voie].type;
			fprintf(f,"  channel '%s' (type '%s', %d values were saved):",VoieManip[voie].nom,ModeleVoie[vm].nom,ArchEvt.voie[vt].dim);
			for(i=0; i<ArchEvt.voie[vt].dim; i++) {
				if(!(i % 10)) fprintf(f,"\n    %5d:",i);
				fprintf(f," %10d",VoieEvent[voie].brutes.t.sval[i]);
			}
			fprintf(f,"\n");
		}
	}
	fclose(f);
	TangoPrint(OPIUM_NOTE,"Subset %s written in %s",nom,SubsetPath);
	return;
}
/* ========================================================================== */
int EvtMarquage() {
	PlotEspace espace; int dim;
	int evt;
	char trace; unsigned char marque;
	char texte[80]; size_t l;

	if(OpiumReadList("Dans l'espace",PlotEspacesList,&EspaceNum,MAXNOMESPACE) == PNL_CANCEL) return(0);
	espace = PlotEspacePtr[EspaceNum];
	strcpy(texte,PlotEspacesList[EspaceNum]); l = strlen(texte);
	if(l > 1) {
		if(texte[l-1] == 's') texte[--l] = '\0';
	}
	strcat(texte," No");
	dim = *(espace->dim);
	evt = EvtDemande + 1;  /* +1 a cause de la numerotation SAMBA */
	if(OpiumReadInt(texte,&evt) == PNL_CANCEL) return(0);
	evt -= 1;
	if((evt < 0) || (evt >= dim)) {
		texte[l] = '\0';
		OpiumError("Numero de %s illegal: %d",texte,evt+1);
		return(0);
	}
	trace = espace->sel[evt].trace;
	marque = 0;
	if(trace == CodeMarquage[MARQUE_ABSENT]) marque = MARQUE_NORMAL;
	else if(trace == CodeMarquage[MARQUE_NORMAL]) marque = MARQUE_TAGGE;
	else if(trace == CodeMarquage[MARQUE_TAGGE]) marque = MARQUE_NORMAL;
	if(OpiumReadListB("Marquage",TexteMarquage,(char*)&marque,8) == PNL_CANCEL)
		return(0);
	EvtDemande = evt;
	PlotNtupleMarque(espace,evt,marque);
	EvtCoupeResultat(espace);
	PlotRefreshSiUniqueBiplot();
	return(0);
}
/* ========================================================================== */
int EvtSelGlobale() {
	int evt; int action;
	
	if(!EvtSel) {
		OpiumError("Action non implementee quand l'option -m vaut 0");
		return(0);
	}
	action = OpiumExec(mEvtSelTous->cdr) - 1;
	if(action < 2) {
		for(evt=0; evt<EvtNb; evt++) EvtSel[evt] = action;
		printf("* Tous les evenements sont %s la selection",action? "dans": "sortis de");
	}
	return(0);
}
/* ========================================================================== */
int EvtSelMarque() {
	int evt; int i; int num;

	if(!EvtSel) {
		OpiumError("Action non implementee quand l'option -m vaut 0");
		return(0);
	}
	evt = EvtDemande;
	EvtSel[evt] = 1 - EvtSel[evt];
	if(EvtRunNb == 1) i = 0;
	else {
		for(i=0; i<EvtRunNb; i++) if(EvtRunInfo[i].premier > evt) break;
		if(i == 0) {
			OpiumError("ERREUR SYSTEME, evenement [%d] introuvable",evt);
			return(0);
		}
		--i;
	}
	num = (evt - EvtRunInfo[i].premier) + EvtRunInfo[i].num0;
	printf("* Evenement %s:%d %s la selection\n",EvtRunInfo[i].fichier,num,
		EvtSel[evt]? "dans": "sorti de");
	return(0);
}
/* ========================================================================== */
int EvtSelZone() {
	int p; PlotEspace espace; int dim; TypeGraphIndiv *sel;
	int ix0,iy0,ix1,iy1;
	int num; float *xadrs,*yadrs;
	int nb;
	
	if((p = PlotChoixFig2D()) < 0) return(0);
	espace = PlotVar[Plot[p].ab6].espace;
	dim = *(espace->dim);
	GraphGetRect(Plot[p].g,&ix0,&AireXmin,&iy0,&AireYmin,&ix1,&AireXmax,&iy1,&AireYmax);
	sel = espace->sel;
	xadrs = *PlotVar[Plot[p].ab6].adrs;
	yadrs = *PlotVar[Plot[p].ord].adrs;
	nb = 0;
	for(num=0; num<dim; num++) {
		if(sel[num].trace != CodeMarquage[MARQUE_ABSENT]) {
			if((*xadrs >= AireXmin) && (*xadrs <= AireXmax) && (*yadrs >= AireYmin) && (*yadrs <= AireYmax)) {
				EvtSel[num] = 1; nb++;
			} else EvtSel[num] = 0;
		}
		xadrs++; yadrs++;
	}
	printf("* %d evenements selectionnes\n",nb);
	return(0);
}
/* ========================================================================== */
int EvtSelGet() {
	char liste[MAXFILENAME]; char ligne[1024],*c;
	FILE *f; size_t lngr;
	char nom_lu[MAXFILENAME],nom_complet[MAXFILENAME]; int num,i;
	int evts_avant,evt,nb,max;

	if(!EvtSel) {
		OpiumError("Action non implementee quand l'option -m vaut 0");
		return(0);
	}
	strcat(strcpy(liste,AnaPath),"Selection");
	if(OpiumReadFile("a partir du fichier",liste,MAXFILENAME) == PNL_CANCEL) return(0);
	if((f = TangoFileOpen(liste,"r")) <= 0) {
		OpiumError("Lecture impossible (%s)",strerror(errno));
		return(0);
	}
	evts_avant = EvtNb;
	do {
		if(!LigneSuivante(ligne,1024,f)) break;
		c = ligne;
		if((*c == '\n') || (*c == '\0') || (*c == '#')) continue;
		sscanf(ligne,"%s %d",nom_lu,&num);
		if(nom_lu[0] == SLASH) strcpy(nom_complet,nom_lu); 
		else strcat(strcpy(nom_complet,EvtsPath),nom_lu);
		lngr = strlen(nom_complet);
//		printf("(EvtSelGet) A inclure: %s\n",nom_complet);
		for(i=0; i<EvtRunNb; i++) {
//			printf("(EvtSelGet) Deja inclus: %s\n",EvtRunInfo[i].nom);
			if(!strncmp(nom_complet,EvtRunInfo[i].nom,lngr)) break;
		}
		if(i >= EvtRunNb) {
			if(EvtRunNb >= MAXFICHIERS) {
				printf("= Deja %d fichiers ouverts! Fichier %s abandonne",MAXFICHIERS,nom_lu);
				return(0);
			}
			if(!EvtRunInclude(nom_complet)) {
				printf("= Le fichier %s NE SERA PAS pris en compte\n",EvtRunInfo[i].nom);
				continue;
			}
			nb = EvtRunInfo[i].numN - EvtRunInfo[i].num0;
			max = EvtRunInfo[i].premier + nb;
			for(evt=EvtRunInfo[i].premier; evt<max; evt++) EvtSel[evt] = 0;
		}
		evt = (num - EvtRunInfo[i].num0) + EvtRunInfo[i].premier;
		EvtSel[evt] = 1;
	} while(1);
	fclose(f);

	if(!ProdInit(ANouveau)) {
		OpiumError("Impossible d'initialiser la production");
		return(0);
	}
	ANouveau = 1;
	if(evts_avant) PlotNtupleSupprime();
	PlotNtupleCree(EvtNb,0.0);
	printf(". Le Ntuple (de %d evenements) a ete remis a %g\n",EvtNb,0.0);
	EvtLitUnSeul(evts_avant,1);

	return(0);
}
/* ========================================================================== */
int EvtSelSave() {
	char entete;
	FILE *fdest; int i,evt,premier; long nb; char binaire;
	char explic[256];
	int fsrce; off_t pos,dim,hdrlngr,buflngr;
	int *buffer; size_t lngr;
	
	strcpy(FichierRunBis,FichierEvents);
	strcat(FichierRunBis,"_a");
	binaire = 1; /* binaire par defaut, sinon ascii (demander maintenant) */
#ifdef SAUVE_INTERVALLE
	AjouteNtuple = 0;
	if(OpiumExec(pEvtRunEvtSave->cdr) == PNL_CANCEL) return(0);
	if(AjouteNtuple) {
		fdest = TangoFileOpen(FichierRunBis,"a");
		entete = 0;
	} else {
		fdest = TangoFileOpen(FichierRunBis,"w");
		entete = 1;
	}
#else
	fdest = TangoFileOpen(FichierRunBis,"w");
	entete = 1;
	EvtPremierSauve = 0;
	EvtDernierSauve = EvtNb;
#endif
	buflngr = 0; buffer = 0;
	if(fdest) {
		if(EvtPremierSauve < 0) EvtPremierSauve = 0;
		if(EvtDernierSauve > EvtNb) EvtDernierSauve = EvtNb;
		for(evt=EvtPremierSauve; evt<EvtDernierSauve; evt++) {
			if(MaxEvts) {
				if(!EvtSel[evt]) continue;
				pos = EvtPos[evt];
				dim = EvtPos[evt + 1] - pos;
			} else {
				pos = EvtPos[0] + (evt * EvtDim);
				dim = EvtDim;
			}
			if(pos > OctetsDejaLus) {
				OpiumError("Position evenement #%d inaccessible (%ld/%ld)",evt,pos,OctetsDejaLus);
				return(0);
			}
			if(EvtRunNb == 1) i = 0; else {
				for(i=0; i<EvtRunNb; i++) if(EvtRunInfo[i].debut > pos) break;
				if(i == 0) {
					OpiumError("Grosse erreur systeme!!! debut[0]=%ld > pos=%ld",
							   EvtRunInfo[i].debut,pos);
					return(0);
				}
				--i;
			}
			if((fsrce = EvtRunInfo[i].path) < 0) {
				/* FAUX!! ArchRunOpen rend 0 ou 1 ... voir ArchEvtGet. */
				fsrce = EvtRunInfo[i].path = ArchRunOpen(EvtRunInfo[i].nom,0,explic);
				if(fsrce == 0) {
					OpiumError("Fichier %s inutilisable (%s)",EvtRunInfo[i].nom,explic);
					return(0);
				}
			}
			if(entete) {
				for(premier=evt-1; premier >= 0; ) {
					if(EvtPos[premier] < EvtRunInfo[i].debut) break;
					premier--;
				}
				hdrlngr = EvtPos[++premier] - EvtRunInfo[i].debut;
				lngr = (size_t)hdrlngr;
				if(buflngr < hdrlngr) {
					if(buffer) free(buffer);
					buffer = (int *)malloc(lngr);
					if(!buffer) {
						OpiumError("Manque %lld octets pour demarrer. Creation abandonnee",hdrlngr);
						break;
					}
					buflngr = hdrlngr;
				}
				lseek(fsrce,0,SEEK_SET);
				nb = read(fsrce,buffer,lngr);
				if(nb != lngr) {
					printf(". lecture de l'evenement %d en erreur: %s\n",evt--,strerror(errno));
					OpiumError("%s:%d Erreur pendant la lecture du fichier", __FILE__, __LINE__);
					break;
				}
				fwrite(buffer,lngr,1,fdest);
				entete = 0;
			}
			lseek(fsrce,pos - EvtRunInfo[i].debut,SEEK_SET);
			if(binaire) {
				lngr = (size_t)dim;
				if(buflngr < dim) {
					if(buflngr) free(buffer);
					buffer = (int *)malloc(lngr);
					if(!buffer) {
						OpiumError("Manque %d octets pour continuer. Dernier evt sauve: %d",dim * sizeof(int),--evt);
						break;
					}
					buflngr = dim;
				}
				nb = read(fsrce,buffer,lngr);
				if(nb != dim) {
					printf(". lecture de l'evenement %d en erreur: %s\n",evt--,strerror(errno));
					OpiumError("%s:%d Erreur pendant la lecture du fichier", __FILE__, __LINE__);
					break;
				}
				fwrite(buffer,lngr,1,fdest);
			} else /* ASCII */ {
			}
		}
		fclose(fdest);
		EvtPremierSauve = evt + 1;
		EvtDernierSauve = EvtNb;
	} else OpiumError("Operation impossible (%s)",strerror(errno));
	return(0);
}
/* ========================================================================== */
int EvtConserve() { ArchEvtSave(1); return(0); }
/* ========================================================================== */
int EvtErase() { ArchEvtErase(); return(0); }
/* ========================================================================== */
int HdrRunAffiche() {
	if(OpiumDisplayed(pHdrRun->cdr)) OpiumClear(pHdrRun->cdr);
	else OpiumDisplay(pHdrRun->cdr);
	return(0);
}
/* ===================================================================== 
int HdrNtpAffiche() {
	AfficheNtuple = 1 - AfficheNtuple;
	OpiumNote("Les n-tuples seront desormais %s",AfficheNtuple? "affiches": "masques");
	return(0);
}
   ===================================================================== 
int HdrEvtAffiche() {
	AfficheHdrEvt = 1 - AfficheHdrEvt;
	OpiumNote("Les entetes d'evenements seront desormais %s",AfficheHdrEvt? "affichees": "masquees");
	return(0);
}
   ===================================================================== */
int EvtCoefApplique() { PasImplementee(); return(0); }
/* ========================================================================== */
int EvtCoefModifie() { PasImplementee(); return(0); }
/* ========================================================================== */
int EvtUniteTemps() {
	char ok;
	
	ok = 1;
	if(OpiumReadBool("Sauver ces valeurs",&ok) == PNL_CANCEL) return(0);
	if(!ArgPrint(UniteTempsFile,EvtUniteTempsDesc))
		OpiumFail("Fichier %s inutilisable (%s)",UniteTempsFile,strerror(errno));
	else printf("* Parametres pour l'evenement unite sauves dans le fichier %s\n",UniteTempsFile);
	return(0);
}
/* ========================================================================== */
int EvtUniteDemandeCalcul() {
	if(OpiumExec(pEvtUniteCalcule->cdr) == PNL_CANCEL) return(0);
	EvtUniteCalcule(EvtUniteNom,EnergieMoyenne,EvtUniteModeManuel,1);
	return(0);
}
/* ========================================================================== */
int EvtUniteDemandeEcriture() {
	if(OpiumReadFile("Sur le fichier",UniteEventFile,MAXFILENAME) == PNL_OK) {
		RepertoireCreeRacine(UniteEventFile);
		EvtUniteSauve(UniteEventFile);
	}
	return(0);
}
/* ========================================================================== */
int EvtUniteDemandeLecture() {
	if(OpiumReadFile("Depuis le fichier",UniteEventFile,MAXFILENAME) == PNL_OK)
		EvtUniteRelit(UniteEventFile);
	return(0);
}
/* ========================================================================== */
int EvtUniteDemandeFit() {
//	if(OpiumReadFloat("Validite ajustement (pr amplitude)",&ValiditeAjustement) == PNL_CANCEL) return(0);
// Le nom est impose par ProdChannelUnite puisque qu'il fourni les adresses....
//	if(OpiumReadText("Nom de l'evenement",EvtUniteNom,MAXEVTNOM) == PNL_CANCEL) return(0);
	EvtUniteFitteEnergies(EvtUniteNom,1);
	if(OpiumAlEcran(mNtpSauve)) MenuItemAllume(mNtpSauve,1,0,GRF_RGB_YELLOW);
	return(0);
}
/* ========================================================================== */
int EvtCalibAffiche() { PasImplementee(); return(0); }
/* ========================================================================== */
int TangoAnalyse() { OpiumFork(cTangoAnalyse); return(0); }
/* ========================================================================== */
int SambaEditArgs(Menu menu, MenuItem *item) {
	Panel p;

	p = PanelDesc(TangoDesc,1);
	if(OpiumExec(p->cdr) == PNL_OK) ArgPrint("TangoArgs",TangoDesc);
	PanelDelete(p);
	return(0);
}
/* ========================================================================== */
static Menu mPpal,mRun,mEvtsSelection /* ,mCalib,mCalibCoef,mCalibUnite */;
static Cadre mEvtsCdr;
int ProductionTotale(),ProdComplete();
extern Menu mFiltres;
MenuItem iPpalEvts[] = {
	{ "Run",                MNU_MENU     &mRun },
//0	{ "Production",         MNU_MENU     &mProd },
//0	{ "Evenements",         MNU_MENU     &mEvts },
//1	{ "Evenements",         MNU_FORK     &mEvtsCdr },
//2	{ "Evenements",         MNU_FORK     &cTangoAnalyse },
	{ "Analyse",            MNU_FONCTION   TangoAnalyse },
	{ "Filtres",	        MNU_MENU	 &mFiltres },
	{ "Supplements",        MNU_MENU     &mProdDialogue },
//1	{ "Traces",             MNU_MENU     &mPlot },
//1	{ "Calib 1 voie",       MNU_MENU     &mCalib },
	{ "Fin analyse",        MNU_RETOUR },
	MNU_END
};
MenuItem iPpalGL[] = {
	{ "Run",                MNU_MENU     &mRun },
	{ "Filtres",	        MNU_MENU	 &mFiltres },
	{ "Supplements",        MNU_MENU     &mProdDialogue },
	MNU_END
};
MenuItem iPpalStrm[] = {
	{ "Run",                MNU_MENU     &mRun },
	{ "Fin lecture",        MNU_RETOUR },
	MNU_END
};
MenuItem iRun[] = {
	{ "Mode de lecture",    MNU_FONCTION   EvtLectMode },
	{ "Ajouter un run",     MNU_FONCTION   EvtRunAjoute },
	{ "Dimensions totales", MNU_PANEL    &pRunDims },
	{ "Entete courante",    MNU_FONCTION   HdrRunAffiche },
	{ "Retirer un run",     MNU_FONCTION   EvtRunDel },
	{ "Liste des runs",     MNU_FONCTION   EvtRunListDisplay },
	{ "Sauver la liste",    MNU_FONCTION   EvtRunListSave },
	{ "Tout retirer",       MNU_FONCTION   EvtRunPurge },
	{ "Relire une liste",   MNU_FONCTION   EvtRunLoad },
	{ "Fermer un fichier",  MNU_FONCTION   ArchEvtKeepFree },
	{ "Editer TangoArgs",   MNU_FONCTION   SambaEditArgs },
//	{ "Langue",             MNU_FONCTION   SambaLangueDemande },
	{ "Quitter",            MNU_RETOUR },
	MNU_END
};
MenuItem iEvts[] = {
	{ "Evenement No...",    MNU_FONCTION   EvtChoix },
	{ "Conserver",          MNU_FONCTION   EvtConserve },
	{ "Precedent",          MNU_FONCTION   EvtPrecedent },
	{ "Suivant",            MNU_FONCTION   EvtSuivant },
	{ "Defiler",            MNU_FONCTION   EvtDefile },
//	{ "Automatique",        MNU_FONCTION   EvtAuto },
	{ "D'apres biplot",     MNU_FONCTION   NtuplePointe },
	{ "Population zone",    MNU_FONCTION   EvtCompteZone },
	{ "Actuel sur biplots", MNU_FONCTION   EvtDemandeMontre },
	{ "Operations",         MNU_SEPARATION },
	{ "Coupures",           MNU_MENU	 &mCoupures },
	{ "Selection",          MNU_MENU	 &mEvtsSelection },
	{ "\0",                 MNU_SEPARATION },
	{ "Entetes",            MNU_PANEL    &pEvtHdrAffichage },
	{ "Effacer",            MNU_FONCTION   EvtErase },
	{ "Quitter",            MNU_RETOUR },
	MNU_END
};
MenuItem iEvtsNav[] = {
	{ "Evenement No...",    MNU_FONCTION   EvtChoix },
	{ "Conserver",          MNU_FONCTION   EvtConserve },
	{ "Precedent",          MNU_FONCTION   EvtPrecedent },
	{ "Suivant",            MNU_FONCTION   EvtSuivant },
	{ "Defiler",            MNU_FONCTION   EvtDefile },
	//	{ "Automatique",        MNU_FONCTION   EvtAuto },
	{ "D'apres biplot",     MNU_FONCTION   NtuplePointe },
	{ "Population zone",    MNU_FONCTION   EvtCompteZone },
	{ "Actuel sur biplots", MNU_FONCTION   EvtDemandeMontre },
	{ "Marquer",            MNU_FONCTION   EvtMarquage },
	{ "Entetes",            MNU_PANEL    &pEvtHdrAffichage },
	{ "Effacer",            MNU_FONCTION   EvtErase },
	MNU_END
};
MenuItem iEvtsSelection[] = {
	{ "Selection de tous",  MNU_FONCTION   EvtSelGlobale },
	{ "Ajouter/retirer 1",  MNU_FONCTION   EvtSelMarque },
	{ "Zone de biplot",     MNU_FONCTION   EvtSelZone },
	{ "Lire selectes",      MNU_FONCTION   EvtSelGet },
	{ "Sauver selectes",    MNU_FONCTION   EvtSelSave },
	{ "Quitter",            MNU_RETOUR },
	MNU_END
};
extern MenuItem iNtpCalcul[];
MenuItem iCoupures[] = {
	{ "Couper variable",    MNU_FONCTION   EvtCoupeAdd },
	{ "Couper zone",        MNU_FONCTION   EvtCoupeZone },
	{ "Retirer coupure",    MNU_FONCTION   EvtCoupeRemove },
	{ "Supprimer toutes",   MNU_FONCTION   EvtCoupeRAZ },
	{ "\0",                 MNU_SEPARATION },
	{ "Sauver coupures",    MNU_FONCTION   EvtCoupeSauve },
	{ "Refaire coupures",   MNU_FONCTION   EvtCoupeRelit },
	{ "Marquer",            MNU_FONCTION   EvtMarquage },
	{ "Quitter",            MNU_RETOUR },
	MNU_END
};
MenuItem iCoupuresActions[] = {
	{ "Couper variable",    MNU_FONCTION   EvtCoupeAdd },
	{ "Couper zone",        MNU_FONCTION   EvtCoupeZone },
	{ "Retirer coupure",    MNU_FONCTION   EvtCoupeRemove },
	{ "Supprimer toutes",   MNU_FONCTION   EvtCoupeRAZ },
	MNU_END
};
MenuItem iCoupuresGestion[] = {
	{ "Sauver",            MNU_FONCTION   EvtCoupeSauve },
	{ "Refaire",           MNU_FONCTION   EvtCoupeRelit },
	MNU_END
};
/*
MenuItem iCalib[] = {
	"Coefficient",     MNU_MENU     &mCalibCoef,
	"Evenement unite", MNU_MENU     &mCalibUnite,
	"Afficher",        MNU_FONCTION   CalibAffiche,
	"Quitter",         MNU_RETOUR,
	MNU_END
};
MenuItem iCalibCoef[] = {
	"Appliquer",       MNU_FONCTION   CalibCoefApplique,
	"Modifier",        MNU_FONCTION   CalibCoefModifie,
	"Quitter",         MNU_RETOUR,
	MNU_END
};
MenuItem iCalibUnite[] = {
	"Calculer",        MNU_FONCTION   EvtUniteCalcule,
	"Normaliser",      MNU_FONCTION   EvtUniteFitteEnergies,
	"Quitter",         MNU_RETOUR,
	MNU_END
};
*/
/* MenuItem iCalib[] = {
//	{ "Appliquer coef.",     MNU_FONCTION   EvtCoefApplique }, variante manip-independante de ProdCalibApplique?
//	{ "Modifier coef.",      MNU_FONCTION   EvtCoefModifie }, variante manip-independante de ProdCalibCorrige?
	{ "Calculer evt unite",  MNU_FONCTION   EvtUniteDemandeCalcul },
	{ "Sauver evt unite",    MNU_FONCTION   EvtUniteDemandeEcriture },
	{ "Relire evt unite",    MNU_FONCTION   EvtUniteDemandeLecture },
	{ "Normaliser evts",     MNU_FONCTION   EvtUniteDemandeFit },
//	{ "Afficher",            MNU_FONCTION   EvtCalibAffiche }, objet??
	{ "Quitter",             MNU_RETOUR },
	MNU_END
};
*/
MenuItem iEvtUnite[] = {
	{ "Zone de fit",         MNU_PANEL     &pEvtUniteZoneFit },
	{ "Calculer evt unite",  MNU_FONCTION   EvtUniteDemandeCalcul },
	{ "Sauver evt unite",    MNU_FONCTION   EvtUniteDemandeEcriture },
	{ "Relire evt unite",    MNU_FONCTION   EvtUniteDemandeLecture },
	{ "Normaliser evts",     MNU_FONCTION   EvtUniteDemandeFit },
	MNU_END
};
MenuItem iEvtSelTous[] = {
	{ "Retirer tous les evts",  MNU_CONSTANTE   1 },
	{ "Ajouter tous les evts",  MNU_CONSTANTE   2 },
	{ "Ne rien changer",        MNU_CONSTANTE   3 },
	MNU_END
};
int ProdCalibChoixMesure(),ProdCalibApplique(),ProdCalibCalcule(), ProdCalibCorrige(),ProdCalibMontre(),ProdCalibSauve();
/* MenuItem iCalibration[] = {
	"Choix mesure",         MNU_FONCTION   ProdCalibChoixMesure,
	"Appliquer fichier",    MNU_FONCTION   ProdCalibApplique,
	"Calculer (auto)",      MNU_FONCTION   ProdCalibCalcule,
	"Modifier (manu)",      MNU_FONCTION   ProdCalibCorrige,
	"Afficher coefs",       MNU_FONCTION   ProdCalibMontre,
	"Sauver coefs",         MNU_FONCTION   ProdCalibSauve,
	MNU_END
}; */
int ProdSauveNtuples();
MenuItem iNtpSauve[] = { "Sauver ntuples", MNU_FONCTION ProdSauveNtuples, MNU_END };

/* ========================================================================== */
void extensions_menu(Cadre cdr) {
	OpiumKeyEnable(cdr,26,HelpMenu);   /* Ctrl-Z */
}
/* ========================================================================== */
void extensions_ps(Cadre cdr) {
#ifdef macintosh
	OpiumKeyEnable(cdr,OPIUM_ALT|'p',OpiumPSprint);  /* Pomme-P */
#else
	OpiumKeyEnable(cdr,16,OpiumPSprint);  /* Ctrl-P */
#endif
}
/* ========================================================================== */
void TangoCreePath(char *base, char rep, char *dir, char *path) {
	strcpy(path,dir);
	if(rep) RepertoireTermine(path);
	if(path[0] != SLASH) {
		if(!strcmp(path,".") || !strcmp(path,"./")) strcpy(path,base);
		else { strcat(strcpy(path,base),dir); if(rep) RepertoireTermine(path); }
	}
}
/* ========================================================================== */
char TangoProdInit() {
	EvtLitUnSeul(EvtNb-1,0);
	printf("* Temps final: %d,%06d\n",ArchEvt.sec,ArchEvt.msec);
	RunTempsTotal = (ArchEvt.sec - RunTempsInitial);

	/* Initialise le reste de la prod qui depend du contenu de l'archive */
	if(!ProdInit(ANouveau)) {
		TangoPrint(OPIUM_ERROR,"Impossible d'initialiser la production");
		return(0);
	}
	ANouveau = 1;
	if(EvtDejaPresents) PlotNtupleSupprime();
	PlotNtupleCree(EvtNb,0.0);
	EvtDejaPresents = EvtNb;
	printf(". Le Ntuple (de %d evenements) a ete remis a %g\n",EvtNb,0.0);
	// printf(". Mise a jour demandee pour @%08X\n",(hexa)pEvtAnalyses);
	return(1);
}
/* ========================================================================== */
int TangoQualiteChange() { GraphQualiteDefaut(FondPlots); return(1); }
/* ========================================================================== */
int TangoQualiteGraph() {
	if(OpiumExec(pQualiteGraph->cdr) == PNL_CANCEL) return(0);
	TangoQualiteChange();
	OpiumNote("Affichage des prochains graphiques: %s",WndSupportNom[FondPlots]);
	return(0);
}

#ifdef NTP_PROGRES_INTEGRE
#define NTP_AFFICHEUR 200
//- static MenuItem iNtpStoppe[] = { { "Stopper", MNU_FONCTION TangoProdStoppe }, MNU_END };
static TypeInstrumGlissiere iNtpAfficheur = { NTP_AFFICHEUR, 8, 1, 0, 3, 3 };
static Instrum iNtpNiveau;
static Panel pNtpProgres;
static int NtpTaux,NtpMaj;
static int64 NtpDebut;
static float NtpReste,NtpTotal,NtpFini;
static char NtpContinueProd;

/* ========================================================================== */
void TangoProdReset(int total) { 
	NtpFini = total;
	NtpMaj = NtpFini / 100;
	if(NtpMaj == 0) NtpMaj = 1;
	NtpContinueProd = 1;
	NtpTaux = 0;
	NtpReste = NtpTotal = 0.0;
	NtpDebut = DateMicroSecs();
	if(OpiumDisplayed(iNtpNiveau->cdr)) OpiumRefresh(iNtpNiveau->cdr);
	if(OpiumDisplayed(pNtpProgres->cdr)) OpiumRefresh(pNtpProgres->cdr);
	OpiumUserAction();
}
/* ========================================================================== */
char TangoProdContinue(int actuel) { 
	int64 deja_passe;
	
	if(!(actuel % NtpMaj)) {
		if(NtpFini) NtpTaux = (actuel * 100) / NtpFini;
		else NtpTaux = 100;
		if(actuel) {
			deja_passe = DateMicroSecs() - NtpDebut;
			NtpTotal = (float)deja_passe * (float)NtpFini / (float)actuel / 1000000.0;
			NtpReste = NtpTotal - ((float)deja_passe / 1000000.0);
		}
		if(OpiumDisplayed(iNtpNiveau->cdr)) OpiumRefresh(iNtpNiveau->cdr);
		if(OpiumDisplayed(pNtpProgres->cdr)) OpiumRefresh(pNtpProgres->cdr);
		OpiumUserAction();
	}
	return(NtpContinueProd);
}
/* ========================================================================== */
static int TangoProdStoppe() { NtpContinueProd = 0; return(0); }
#endif /* NTP_PROGRES_INTEGRE */

/* static TypeS_us T0,T1; char Toper;
   ===================================================================== 
static int TangoTest() {
	TypeS_us t2;


	printf("   %6d,%06d\n",T0.t.s,T0.t.us);
	printf(" %c %6d,%06d\n",Toper? '-': '+',T1.t.s,T1.t.us);
	printf(" ---------------\n");
	t2.s_us = S_usOper(&T0,Toper,&T1);
	printf(" = %6d,%06d\n\n",t2.t.s,t2.t.us);
	return(0);
}
   ===================================================================== */
static void TangoPlancheEvtsConstruit() {
	Menu m,s; Panel p; int x,y,xt,yt,xm,ym;
	FILE *f;

    // printf("* Construction de la planche principale\n");
	ym = 0;
	cTangoAnalyse = BoardCreate();
	snprintf(TangoTitreRun,80,"%s: Run %s",APPLI,EventName);
	OpiumTitle(cTangoAnalyse,TangoTitreRun);
	x = 2 * Dx; y = Dy;

	p = PanelCreate(2); PanelMode(p,PNL_DIRECT|PNL_RDONLY); PanelSupport(p,WND_CREUX);
	PanelInt(p,L_("Nombre total d'evenements"),&EvtNb);
	PanelInt(p,L_("Temps total d'acquisition"),&RunTempsTotal);
	BoardAddPanel(cTangoAnalyse,p,Dx,Dy,0);
	
#ifndef MENU_BARRE
	MenuTitle(mPpal,0);
	OpiumSupport(mPpal->cdr,WND_PLAQUETTE);
	MenuColumns(mPpal,MenuItemNb(mPpal));
	BoardAddMenu(cTangoAnalyse,mPpal,50 * Dx,Dy,0);
#endif

	x = 2 * Dx; y += (4 * Dy);
	BoardAreaOpen("Ntuples",WND_PLAQUETTE); // donne des rainures
#ifdef NTP_PROGRES_INTEGRE
	BoardAddBouton(cTangoAnalyse,L_("Calculer"),ProductionTotale,x+(4*Dx),y,0);
	if((f = fopen(FichierNtuple,"r"))) {
		BoardAddBouton(cTangoAnalyse,L_("Relire"),EvtNtupleRelit,x+(20*Dx),y,0);
		fclose(f);
	}
	y += (2 * Dy);
	iNtpNiveau = InstrumInt(INS_COLONNE,&iNtpAfficheur,&NtpTaux,0,100); InstrumSupport(iNtpNiveau,WND_CREUX);
	BoardAddInstrum(cTangoAnalyse,iNtpNiveau,x+Dx,y,0); y += (4 * Dy);
	pNtpProgres = p = PanelCreate(2); 
	PanelColumns(p,2); PanelMode(p,PNL_DIRECT); PanelSupport(p,WND_CREUX);
	PanelItemSelect(p,PanelFloat(p,L_("Reste"),&NtpReste),0); PanelFormat(p,1,"%.1f"); PanelLngr(p,1,6);
	PanelItemSelect(p,PanelFloat(p,L_("secs sur"),&NtpTotal),0); PanelFormat(p,2,"%.1f"); PanelLngr(p,2,6);
	NtpTaux = 0; NtpReste = NtpTotal = 0.0; NtpContinueProd = 1;
	BoardAddPanel(cTangoAnalyse,p,x+Dx,y,0); y += (2 * Dy);
	BoardAddBouton(cTangoAnalyse,"Stop",TangoProdStoppe,(NTP_AFFICHEUR)/2+Dx,y,0); y += (2 * Dy);
#else  /* NTP_PROGRES_INTEGRE */
	m = MenuLocalise(iNtpCalcul); OpiumSupport(m->cdr,WND_PLAQUETTE);
	BoardAddMenu(cTangoAnalyse,m,x+(3*Dx),y,0); y += ((MenuItemNb(m) + 1) * Dy);
	BoardAddBouton(cTangoAnalyse,L_("Calculer"),ProductionTotale,x+(3*Dx),y,0);
	if((f = fopen(FichierNtuple,"r"))) {
		BoardAddBouton(cTangoAnalyse,L_("Relire"),EvtNtupleRelit,x+(18*Dx),y,0);
		fclose(f);
	}
	y += (2 * Dy);
#endif /* NTP_PROGRES_INTEGRE */
	p = pEvtAnalyses = PanelCreate(1); PanelMode(p,PNL_DIRECT|PNL_RDONLY); PanelSupport(p,WND_CREUX);
	PanelInt(p,L_("Utilisables"),&EvtsAnalyses);
	PanelItemIScale(p,1,0,4*EvtNb/3); PanelItemColors(p,1,TangoRougeOrangeJauneVert,4);
	BoardAddPanel(cTangoAnalyse,p,x,y,0);
//?	BoardAreaMargesVert(Dy/4,0);
	BoardAreaClose(cTangoAnalyse,&ZoneNtuples,&x,&y);
	if(y > ym) ym = y;
	
	x += (3 * Dx); y = 5 * Dy;
	BoardAreaOpen(L_("Traces"),WND_PLAQUETTE); // donne des rainures
	m = MenuLocalise(mPlotTypes->items); OpiumSupport(m->cdr,WND_PLAQUETTE);
	BoardAddMenu(cTangoAnalyse,m,x,y,0);
	// BoardAddBouton(cTangoAnalyse,L_("Qualite"),ProdQualiteGraph,OPIUM_EN_DESSOUS_DE OPIUM_PRECEDENT);
	m = MenuLocalise(iPlotExamine); OpiumSupport(m->cdr,WND_PLAQUETTE);
	BoardAddMenu(cTangoAnalyse,m,x+(12*Dx),y,0);
	p = PanelCreate(1); PanelSupport(p,WND_CREUX);
	PanelListB(p,"Qualite",WndSupportNom,&FondPlots,8); PanelItemColors(p,1,TangoBlancNoir,2);
	PanelOnApply(p,TangoQualiteChange,0);
	// PanelMode(p,PNL_DIRECT); 
	// PanelItemModif(p,1,TangoQualiteChange,0); ne marche pas comme on voudrait avec juste 1 item et PNL_DIRECT
	BoardAddPanel(cTangoAnalyse,p,x+(4*Dx),y+((MenuItemNb(m)+1)*Dy),0);
	BoardAreaClose(cTangoAnalyse,&ZoneTraces,&x,&y);
	if(y > ym) ym = y;
	
	x += (3 * Dx); y = 5 * Dy;
	BoardAreaOpen(L_("Navigation"),WND_PLAQUETTE); // donne des rainures
	m = MenuLocalise(iEvtsNav); OpiumSupport(m->cdr,WND_PLAQUETTE);
	BoardAddMenu(cTangoAnalyse,m,x,y,0);
	BoardAreaClose(cTangoAnalyse,&ZoneNav,&x,&y);
	xt = x;
	if(y > ym) ym = y;
	
	x += (3 * Dx);
	xm = x;
	y = 6 * Dy; // au ras des precedentes zones
	BoardAreaOpen(L_("Affichage"),WND_PLAQUETTE); // donne des rainures
	if(LectEnvirDesc) {
		p = PanelDesc(LectEnvirDesc,0); PanelMode(p,PNL_DIRECT|PNL_RDONLY); PanelSupport(p,WND_CREUX);
		yt = (PanelItemNb(p) + 3) * Dy;
		if((Dy + yt) < (ym + (2 * Dy))) {
			BoardAreaOpen(L_("Conditions"),WND_PLAQUETTE); // donne des rainures
			BoardAddPanel(cTangoAnalyse,p,x,y,0);
			BoardAreaClose(cTangoAnalyse,&ZoneEnvir,&x,&y);
			x = xm; y += (2 * Dy);
		} else BoardAddPanel(cTangoAnalyse,p,x,Dy,0);
//		if(yt < (ym + (2 * Dy))) y = ym + (2 * Dy);
	} else yt = 0;
	BoardAreaOpen(L_("General"),WND_PLAQUETTE); // donne des rainures
	mProdAffGeneral = MenuLocalise(iProdAffGeneral); OpiumSupport(mProdAffGeneral->cdr,WND_PLAQUETTE);
	BoardAddMenu(cTangoAnalyse,mProdAffGeneral,x,y,0); y += (2 * Dy);
	BoardAreaClose(cTangoAnalyse,&ZoneAffGeneral,&x,&y);
	if(LectEnvirDesc) { x += (2 * Dx); y = (6 * Dy) + yt; }
	else { x = xm; y += (2 * Dy); }
	BoardAreaOpen(L_("Evenements"),WND_PLAQUETTE); // donne des rainures
	mProdAffEvts = MenuLocalise(iProdAffEvts); OpiumSupport(mProdAffEvts->cdr,WND_PLAQUETTE);
	BoardAddMenu(cTangoAnalyse,mProdAffEvts,x,y,0); // y += (2 * Dy);
	BoardAreaClose(cTangoAnalyse,&ZoneAffEvts,&x,&y);
	BoardAreaClose(cTangoAnalyse,&ZoneAffichage,&x,&y);
	x = xm; y += (2 * Dy);

	BoardAreaOpen(L_("Evenement unite"),WND_PLAQUETTE); // donne des rainures
	m = MenuLocalise(iEvtUnite); OpiumSupport(m->cdr,WND_PLAQUETTE);
	BoardAddMenu(cTangoAnalyse,m,x,y,0);
	mNtpSauve = MenuLocalise(iNtpSauve); OpiumSupport(mNtpSauve->cdr,WND_PLAQUETTE);
	y += (MenuItemNb(m) + 1) * Dy;
	BoardAddMenu(cTangoAnalyse,mNtpSauve,x,y,0);
	/* {
		p = PanelCreate(6); PanelColumns(p,3); PanelSupport(p,WND_CREUX); PanelMode(p,PNL_BYLINES);
		PanelBlank(p); PanelFormat(p,PanelInt(p,0,&T0.t.s),"%-6d"); PanelFormat(p,PanelInt(p,",",&T0.t.us),"%06d");
		PanelKeyB(p,0,"+/-",&Toper,2); PanelFormat(p,PanelInt(p,0,&T1.t.s),"%-6d"); PanelFormat(p,PanelInt(p,",",&T1.t.us),"%06d");
		PanelBoutonText(p,PNL_APPLY,"Calculer");
		PanelOnApply(p,TangoTest,0);
		BoardAddPanel(cTangoAnalyse,p,x,y + ((MenuItemNb(m) + 1) * Dy),0);
	} */
	BoardAreaClose(cTangoAnalyse,&ZoneEvtUnite,&x,&y);
	// if(y > ym) ym = y;

	/* x = xm; y += (2 * Dy);
	BoardAreaOpen("Calibration",WND_PLAQUETTE);
	m = MenuLocalise(iCalibration); OpiumSupport(m->cdr,WND_PLAQUETTE);
	BoardAddMenu(cTangoAnalyse,m,x,y,0);
	BoardAreaClose(cTangoAnalyse,&ZoneCalib,&x,&y); */
	

	y = ym;
	x = 2 * Dx; y += (2 * Dy);
	BoardAreaOpen(L_("Coupures"),WND_PLAQUETTE); // donne des rainures
	m = MenuLocalise(iCoupuresActions); OpiumSupport(m->cdr,WND_PLAQUETTE);
	BoardAddMenu(cTangoAnalyse,m,x,y,0);
	p = pEvtValides = PanelCreate(3); PanelMode(p,PNL_DIRECT|PNL_RDONLY); PanelSupport(p,WND_CREUX);
	PanelInt(p,L_("Evenements restant"),&EvtValides);
	PanelFloat(p,L_("Efficacite"),&EvtEfficacite);
	PanelFloat(p,L_("Taux (Hz)"),&EvtTaux);
	BoardAddPanel(cTangoAnalyse,p,x+(19*Dx),y,0);
	s = MenuLocalise(iCoupuresGestion); OpiumSupport(s->cdr,WND_PLAQUETTE);
	BoardAddMenu(cTangoAnalyse,s,xt-(15*Dx),y,0); // -(8*Dx) minimum
	BoardAddBouton(cTangoAnalyse,"Sauver evts",EvtCoupeSubset,xt-(15*Dx),y+(3 * Dy),0);
	tCoupures = TermCreate(12,WndPixToCol(xt-x)-1,1024); OpiumSupport(tCoupures->cdr,WND_CREUX); // TermCreate(12,96,1024) colle avec la fin du menu "Evenement unite"
	TermTitle(tCoupures,L_("Coupure courante"));
	BoardAddTerm(cTangoAnalyse,tCoupures,x,y+((MenuItemNb(m)+1)*Dy),0);
	BoardAreaClose(cTangoAnalyse,&ZoneCoupures,&x,&y);

	OpiumPosition(cTangoAnalyse,OpiumServerWidth(0)/2,40);
}
/* ========================================================================== */
void TangoCreeCoupureDesc() {
	ArgDesc *desc;

	// printf("* Construction des panels de choix de variables\n");
	desc = &(CoupureDesc[0]); /* PlotVarList cree dans ProdInit() par PlotVarsDeclare() */
	ArgAdd(desc++,0,     DESC_KEY "evts/et/ou",  &CoupureLue.oper,   0);
	ArgAdd(desc++,"var", DESC_LISTE PlotVarList, &CoupureLue.var,    0);
	ArgAdd(desc++,"min", DESC_FLOAT              &CoupureLue.min,    0);
	ArgAdd(desc++,"max", DESC_FLOAT              &CoupureLue.max,    0);
	ArgAdd(desc++,"off", DESC_KEY ClesMarquage,  &CoupureLue.marque, 0);
}
/* ========================================================================== */
void TangoCreePanelsVars() {

	/* PlotVarList doit avoir ete cree, et il l'est dans ProdInit() */
	pCoupure = PanelCreate(6);
	PanelKeyB (pCoupure,"Coupure","nouvelle/precedente ET/precedente OU/tout",(char*)&CoupOper,16);
	PanelList (pCoupure,"Sur variable",PlotVarList,&NumVar,16);
	PanelFloat(pCoupure,"Au minimum",&VarMini);
	PanelFloat(pCoupure,"et maximum",&VarMaxi);
	PanelListB(pCoupure,"Marquage exclus",TexteMarquage,(char*)&EvtMarque,8);
	PanelBool (pCoupure,"Coupure a suivre",&AutreCoupure);
	
	/* Ajout de variables */
	pProdVarDef = PanelCreate(5);
	PanelText(pProdVarDef,"Nom",ProdVarAjoutee.nom,PRODVAR_NOM);
	PanelKeyB(pProdVarDef,"Niveau",Niveaux,&(ProdVarAjoutee.niveau),12);
	PanelList(pProdVarDef,"Operande 1",PlotVarList,&(ProdVarAjoutee.var1),32);
	PanelKeyB(pProdVarDef,"Operateur",Opers,&(ProdVarAjoutee.oper),2);
	PanelList(pProdVarDef,"Operande 2",PlotVarList,&(ProdVarAjoutee.var2),32);
	pProdVarQui = PanelCreate(1);
	PanelList(pProdVarQui,"Laquelle",ProdVarNom,&ProdVarNum,PRODVAR_NOM);
	pProdVarMod = PanelCreate(5);
	PanelText(pProdVarMod,"Nom",ProdVarAjoutee.nom,PRODVAR_NOM); PanelItemSelect(pProdVarMod,1,0);
	PanelKeyB(pProdVarMod,"Niveau",Niveaux,&(ProdVarAjoutee.niveau),12);
	PanelList(pProdVarMod,"Operande 1",PlotVarList,&(ProdVarAjoutee.var1),32);
	PanelKeyB(pProdVarMod,"Operateur",Opers,&(ProdVarAjoutee.oper),2);
	PanelList(pProdVarMod,"Operande 2",PlotVarList,&(ProdVarAjoutee.var2),32);
}
/* ========================================================================== */
int TangoAfficheStream() {
	int voie;

	ArchStrmRazDisplay();

	for(voie=0; voie<VoiesNb; voie++) if(VoieManip[voie].affiche) {
	#ifdef A_TERMINER
		if(!VoieTampon[voie].brutes->t.sval) continue;
		VoieTampon[voie].brutes->dim = DureeTampons / VoieEvent[voie].horloge;
		nbpts = VoieTampon[voie].brutes->dim;
		if(nbpts != VoieTampon[voie].brutes->max) {
			if(VoieTampon[voie].brutes->t.sval) {
				free(VoieTampon[voie].brutes->t.sval);
				VoieTampon[voie].brutes->t.sval = 0;
			}
			VoieTampon[voie].brutes->max = 0;
		}
		if(!VoieTampon[voie].brutes->t.sval) {
			VoieTampon[voie].brutes->t.sval = (TypeDonnee *)malloc(nbpts * sizeof(TypeDonnee));
			if(!VoieTampon[voie].brutes->t.sval) {
				printf("Donnees voie #%d: %d octets introuvables\n",voie,nbpts * sizeof(TypeDonnee));
				VoieTampon[voie].brutes->max = 0;
				continue;
			}
			memset((void *)VoieTampon[voie].brutes->t.sval,0xB,nbpts * sizeof(TypeDonnee));
		}
		VoieTampon[voie].brutes->max = nbpts;
		VoieTampon[voie].prochain = VoieTampon[voie].brutes->t.sval;

		ArchStrmFenetre(voie);
	#endif
	}
	return(0);
}
/* ========================================================================== */
static void TangoPlancheStrmConstruit() {
	int voie;
	Panel p;
	int y;

	StrmStocke = 10.; /* s */; StrmAffiche = 1.0; /* ms */
	y = Dy;
	cTangoAnalyse = BoardCreate();
	snprintf(TangoTitreRun,80,"Run %s",EventName);
	OpiumTitle(cTangoAnalyse,TangoTitreRun);

	p = PanelCreate(VoiesNb); PanelSupport(p,WND_CREUX);
	for(voie=0; voie<VoiesNb; voie++) {
		VoieManip[voie].affiche = 1;
		PanelBool(p,VoieManip[voie].nom,&(VoieManip[voie].affiche));
	}
	BoardAddPanel(cTangoAnalyse,p,Dx,Dy,0);

	p = PanelCreate(3); PanelSupport(p,WND_CREUX);
	PanelFloat(p,"Duree stockee (s)",&StrmStocke);
	PanelFloat(p,"Duree affichee (ms)",&StrmAffiche);
	BoardAddBouton(cTangoAnalyse,L_("Afficher"),TangoAfficheStream,Dx,y,0);
}
#define GADGET
/* ========================================================================== */
static void TangoStartOpium(char *titre) {
	/* Initialisation de l'interface Utilisateur [Opium] */
	int i; char nom_ppal[32];
#ifdef GADGET
	Info presentation;
#endif

	//	WndLog(500,45,60,100,"Journal de TANGO");
	WndSetFontName(SambaFontName);
	WndSetFontSize(SambaFontSize);
	if(FondNoir) WndSetBgndBlack(1);
#ifdef X11
	if(!strcmp(ServeurX11,".")) strcpy(ServeurX11,getenv("DISPLAY"));
	printf("  Affichage X11 demande sur %s\n",ServeurX11);
#endif
	if(!OpiumInit(ServeurX11)) { printf("  ! Affichage impossible\n"); exit(0); }
	printf("  Affichage initialise\n"); fflush(stdout);
	AutoriseOpium = 1;
#ifdef CHANGE_ROOTWINDOW
	WndNewRoot(APPLI,OpiumServerWidth(),OpiumServerHeigth());
#endif
#ifdef WIN32_BACKGROUND
	SetWindowText(WndRoot, titre);
#endif

	/* Prise en compte de la langue */
	SambaLangueDefini(TangoInfoSource,TangoInfoDir);
	// SambaLangueUtilise(1);

	//	LogOnTerm("Journal de Tango",24,80,32768);

	/* Construction de l'interface Utilisateur */
	Dx = WndColToPix(1); Dy = WndLigToPix(1);
	GraphQualiteDefaut(FondPlots? WND_Q_ECRAN: WND_Q_PAPIER);
	GraphLargeurTraits(WND_Q_PAPIER,SambaTraits); // au cas ou on demanderait != 3
	pQualiteGraph = PanelCreate(1);
	PanelListB(pQualiteGraph,"Qualite des prochains graphiques",WndSupportNom,&FondPlots,8);
	PanelItemColors(pQualiteGraph,1,TangoBlancNoir,2);
#ifdef GADGET
	presentation = InfoCreate(1,80);
	OpiumPosition(presentation->cdr,5,48);
	InfoTitle(presentation,titre);
	InfoWrite(presentation,1,"Initialisation v%s en cours",TangoVersion);
	OpiumDisplay(presentation->cdr);
#endif
	HelpMode(1);
	OpiumOptions(OPIUM_MENU,(TypeOpiumFctn)extensions_menu);
	OpiumOptions(OPIUM_PANEL,(TypeOpiumFctn)extensions_ps);
	OpiumOptions(OPIUM_TERM,(TypeOpiumFctn)extensions_ps);
	OpiumOptions(OPIUM_GRAPH,(TypeOpiumFctn)extensions_ps);
	LogInit();
	if(EvtPhotoDir[0]) {
		OpiumPhotoDir(EvtPhotoDir);
		printf("* Photos sauvees dans %s\n",EvtPhotoDir);
	}
	strcat(strcpy(nom_ppal,"Topmenu "),EventName);

	if(Trigger.enservice) {
#ifdef MENU_BARRE
		mPpal = MenuIntitule(iPpalEvts,0);
#else  /* !MENU_BARRE */
		mPpal = MenuIntitule(iPpalGL,nom_ppal);
#endif /* !MENU_BARRE */
	} else mPpal = MenuIntitule(iPpalStrm,nom_ppal);
	mRun  = MenuLocalise(iRun);
	if(!Trigger.enservice) MenuItemHide(mRun,(IntAdrs)"Dimensions totales");
	mPlot = 0;
	mNtpSauve = 0;
	/*
	 mCalib = MenuLocalise(iCalib);
	 mCalibCoef = MenuLocalise(iCalibCoef);
	 mCalibUnite = MenuLocalise(iCalibUnite);
	 */

	pEvtRunName = PanelCreate(4);
	PanelFile(pEvtRunName,"Fichier",FichierEvents,MAXFILENAME);
	if(Trigger.enservice) PanelInt (pEvtRunName,"Nb.evts (0:tous)",&EvtsParFichier);
	PanelInt (pEvtRunName,"Premiere partition",&EventPartDebut);
	PanelInt (pEvtRunName,"Derniere partition",&EventPartFin);

	pEvtRunListSave = PanelCreate(2);
	PanelFile(pEvtRunListSave,"Fichier",FichierRuns,MAXFILENAME);
	PanelKeyB(pEvtRunListSave,"Ecriture","creation/ajout",&AjouteNtuple,10);

	pEvtRunLoad = PanelCreate(2);
	PanelFile(pEvtRunLoad,"Fichier",FichierRuns,MAXFILENAME);
	PanelKeyB(pEvtRunLoad,"Catalogue","remplacement/extension",&AjouteNtuple,15);

	if(Trigger.enservice) {
		mEvts = MenuLocalise(iEvts); mEvtsCdr = mEvts->cdr;
		mEvtsSelection = MenuLocalise(iEvtsSelection);
		mEvtSelTous = MenuLocalise(iEvtSelTous);
		mCoupures = MenuLocalise(iCoupures);

		pEvtRunEvtSave = PanelCreate(4);
		PanelInt(pEvtRunEvtSave,"Premier evenenemt a sauver",&EvtPremierSauve);
		PanelInt(pEvtRunEvtSave,"Dernier evenenemt a sauver",&EvtDernierSauve);
		PanelFile(pEvtRunEvtSave,"Fichier",FichierRunBis,MAXFILENAME);
		PanelKeyB(pEvtRunEvtSave,"Ecriture","creation/ajout",&AjouteNtuple,10);

		pEvtNtupleEcrit = PanelCreate(5);
		PanelFile(pEvtNtupleEcrit,"Fichier",FichierNtuple,MAXFILENAME);
		PanelKeyB(pEvtNtupleEcrit,"Variables a sauver","toutes/selection actuelle",&SelecteVar,20);
		// PanelList(pEvtNtupleEcrit,"Espace",PlotEspacesList,&EspaceNum,MAXNOMESPACE);
		PanelKeyB(pEvtNtupleEcrit,"Coupure sur l'espace","aucune/actuelle",&SelecteEvt,10);
		PanelKeyB(pEvtNtupleEcrit,"Ecriture","creation/ajout",&AjouteNtuple,10);

		pEvtNtupleRelit = PanelCreate(3);
		PanelFile(pEvtNtupleRelit,"Fichier",FichierNtuple,MAXFILENAME);
		// PanelList(pEvtNtupleRelit,"Espace",PlotEspacesList,&EspaceNum,MAXNOMESPACE);
		PanelKeyB(pEvtNtupleRelit,"Ntuple","remplacement/extension",&AjouteNtuple,15);

		pEvtHdrAffichage = PanelCreate(3);
		PanelBool(pEvtHdrAffichage,"Ntuple",&AfficheNtuple);
		PanelBool(pEvtHdrAffichage,"Evenement",&AfficheHdrEvt);
		PanelBool(pEvtHdrAffichage,"Voies",&AfficheVoies);

		pEvtUniteCalcule = PanelCreate(3);
		// PanelText(pEvtUniteCalcule,"Nom de l'evenement",EvtUniteNom,MAXEVTNOM);
		PanelFloat(pEvtUniteCalcule,"Energie moyenne",&EnergieMoyenne);
		PanelBool(pEvtUniteCalcule,"Selection manuelle",&EvtUniteModeManuel);

		pEvtUniteZoneFit = PanelCreate(2);
		PanelFloat(pEvtUniteZoneFit,"Duree de montee (ms)",&EvtUniteMont);
		PanelFloat(pEvtUniteZoneFit,"Duree de descente (ms)",&EvtUniteDesc);
		PanelOnApply(pEvtUniteZoneFit,EvtUniteTemps,0);
		PanelOnOK(pEvtUniteZoneFit,EvtUniteTemps,0);

		tSelect = TermCreate(10,80,1024);
		TermTitle(tSelect,"Coupure courante");
		tListe = TermCreate(24,100,4096);
		TermTitle(tListe,"Runs pris en compte");

		tCoupures = 0;

		iEvtZoneConseil = InfoCreate(2,50);
		InfoTitle(iEvtZoneConseil,"Designation des limites de zone de coupure");
		OpiumPosition(iEvtZoneConseil->cdr,WndColToPix(5),WndLigToPix(9));

		pCompteZone = PanelCreate(6); PanelColumns(pCompteZone,2); PanelMode(pCompteZone,PNL_BYLINES|PNL_RDONLY);
		i = PanelInt  (pCompteZone,"Evenements",&AireNb);
		PanelItemColors(pCompteZone,i,TangoRougeVertJaune,2);
		PanelItemSouligne(pCompteZone,i,1);
		i = PanelFloat(pCompteZone,"taux",&AireTaux);
		PanelItemRScale(pCompteZone,i,-1.0,1.0);
		PanelItemColors(pCompteZone,i,TangoRougeVertJaune,2);
		PanelItemSouligne(pCompteZone,i,1);
		PanelFloat(pCompteZone,AireTxtX,&AireXmin); PanelFloat(pCompteZone,"max",&AireXmax);
		i = PanelFloat(pCompteZone,AireTxtY,&AireYmin);
		// PanelItemSouligne(pCompteZone,i,1);
		i = PanelFloat(pCompteZone,"max",&AireYmax);
		// PanelItemSouligne(pCompteZone,i,1);

		FiltresDialogueCreate();

		pRunDims = PanelCreate(2); PanelMode(pRunDims,PNL_DIRECT|PNL_RDONLY);
		PanelInt(pRunDims,"Nombre total d'evenements",&EvtNb);
		PanelInt(pRunDims,"Temps total d'acquisition",&RunTempsTotal);
		OpiumTitle(pRunDims->cdr,EventName);

		ArchCreeUI();
		OpiumPosition(pHdrRun->cdr,100,100);
		OpiumPosition(pHdrEvt->cdr,800,100);

		PlotBuildUI();
		PlotVarsInterface();
		TangoCreePanelsVars();
		ProdCreeUI();
	//	bEvtPlanche = BoardCreate();
		TangoPlancheEvtsConstruit();

	#ifdef GADGET
		InfoWrite(presentation,1,"%s version %s",APPLI,TangoVersion);
		OpiumRefresh(presentation->cdr);
		// OpiumClear(presentation->cdr);
	#endif
		// OpiumDisplay(pRunDims->cdr);
		OpiumFork(cTangoAnalyse);
	#ifdef MENU_BARRE
		printf("= Creation de la barre de menus\n");
		// WndMenuDebug(1);
		MenuBarreCreate(mPpal); /* mPlot doit etre cree pour pouvoir apparaitre dans le MenuBar */
		#ifdef GADGET
		MenuBarreFenetreOuverte(presentation->cdr);
		#endif
		MenuBarreFenetreOuverte(cTangoAnalyse);
		MenuBarreExec();
	#else /* !MENU_BARRE */
		//+ printf("= Le menu principal est une fenetre flottante\n");
		//+ OpiumFork(cTangoAnalyse);
		//+ OpiumExec(mPpal->cdr);
		printf("= Le menu principal est integre a la planche\n");
		OpiumExec(cTangoAnalyse);
	#endif /* MENU_BARRE */

	} else /* cas !Trigger.enservice */ {
		TangoPlancheStrmConstruit();
		OpiumFork(cTangoAnalyse);
	#ifdef MENU_BARRE
		MenuBarreCreate(mPpal); /* mPlot doit etre cree pour pouvoir apparaitre dans le MenuBar */
		#ifdef GADGET
		MenuBarreFenetreOuverte(presentation->cdr);
		#endif
		MenuBarreFenetreOuverte(cTangoAnalyse);
		MenuBarreExec();
	#else /* !MENU_BARRE */
		printf("= Le menu principal est une fenetre flottante\n");
		OpiumFork(cTangoAnalyse);
		OpiumExec(mPpal->cdr);
	#endif /* MENU_BARRE */
	}
}
/* ========================================================================== */
// #ifdef WIN32: int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) { }
int main(int argc, char *argv[]) {
	int i,j,k,var,qual; size_t l;
	char *r,c;
	char creation_args,debug_appel;
	char titre[80]; char start_dir[MAXFILENAME];

//	T0.t.s = T0.t.us = T1.t.s = T1.t.us = 0; Toper = 0;
	debug_appel = 1;
	/* Initialisation generale du logiciel */
	AutoriseOpium = 0;
	ScriptLoop = 0;
	DemandeFichier = 0; ANouveau = 0;
	RunsName[0] = '\0';
	EvtPhotoDir[0] = '\0';
	EvtPos = 0; EvtSel = 0; EvtNb = EvtLus = 0;
	EvtRunNb = 0; OctetsDejaLus = 0; RunTempsTotal = 0;
	EvtPremierSauve = 0; EvtDernierSauve = 0;
	EventPartDebut = 0; EventPartFin = 999999; TrancheRelue = -1;
	for(i=0; i<=MAXFICHIERS; i++) EvtRunName[i] = EvtRunInfo[i].nom;
	EspaceNum = 0;
	EvtUniteMont = 1.0; EvtUniteDesc = 1.0;
	EvtsParFichier = 0;
	EvtRunNum = 0;
	EvtDemande = 0;
	CoupOper = 0;
	sprintf(CoupureNom,"coupure_1");
	EvtMarque = MARQUE_ABSENT;
	AutreCoupure = 0;
	EvtValides = 0;
	EvtsAnalyses = 0;
	EvtEspaceSelecte = 0;
#ifdef NTP_PROGRES_INTEGRE
	NtpContinueProd = 1;
#endif
	SelectNb = 0;
	SelecteVar = SelecteEvt = 0;
    strcpy(ProdVarAjoutee.nom,"extra"); ProdVarAjoutee.niveau = 0;
    ProdVarAjoutee.var1 = 0; ProdVarAjoutee.var2 = 1; ProdVarAjoutee.oper = 0;
    ProdVarAjoutee.a_calculer = 0;
    ProdVarNum = ProdVarNb = 0;
    for(var=0; var<PRODVAR_MAX; var++) ProdVarNom[var] = ProdVar[var].nom;
    ProdVarNom[var] = "\0";
    ProdVarDetecNb = ProdVarVoieNb = 0;
	EnergieMoyenne = 1.0;
	EvtUniteModeManuel = 0;
	EvtEfficacite = 100.0;
	EvtTaux = 0.0;
	TangoRCdefaut = 0.140;
	Separateur = PLOT_SEP_PTVIRG;
	//	ValiditeAjustement = 0.99;
	AfficheHdrEvt = 0; AfficheVoies = 0; AfficheNtuple = 1;
	pHdrRun = pHdrEvt = 0;
	sprintf(titre,"%s version %s",NOM_APPLI,TangoVersion);
#ifdef macintosh
#ifdef CODE_WARRIOR_VSN
	printf("= %s, compilation %s sous CW%x\n",titre,TangoCompile,__MWERKS__);
#else
	printf("= %s, compilation %s sous Xcode\n",titre,TangoCompile);
#endif
	//	printf("AltiVec %s utilise\n\n",__ALTIVEC__?"\0":"non");
#endif
#ifdef UNIX
	printf("= %s, compilation %s sous Unix\n",titre,TangoCompile);
#endif
#ifdef WIN32
	printf("= %s, compilation %s pour Windows\n",titre,TangoCompile);
#endif
	if(debug_appel) printf("  Version d'apres le compilateur: %s\n\n",TangoVersionCompilo);

	/* Parametres susceptibles d'etre modifies par l'utilisateur */
	RepertoireInit(HomeDir);
	SambaTopDirAt(TopDir,"TangoArgs"); strcpy(start_dir,TopDir);
	AnnexesDefaults();
	TangoDefaults();
	strcpy(InfoPrefs,TangoInfoSource);
    strcpy(XcodeTangoDebugTxt,"NO"); XcodeTangoDebug = 0;

	strcpy(&(TangoColrFig[WND_Q_ECRAN][0]),"0000FFFF0000");
	strcpy(&(TangoColrFit[WND_Q_ECRAN][0]),"FFFFFFFF0000");
	strcpy(&(TangoColrFig[WND_Q_PAPIER][0]),"00000000BFFF");
	strcpy(&(TangoColrFit[WND_Q_PAPIER][0]),"BFFF00007FFF");
	Bavard = 0;

	/* Modifications utilisateur au moment de l'appel */
	printf("  Les parametres par defaut d'appel de %s sont dans '%s%s'\n",NOM_APPLI,TopDir,"TangoArgs");
	creation_args = (ArgScan("TangoArgs",TangoDesc) == 0);
	ArgListChange(argc,argv,TangoDesc);
	//	ArgPrint("*",ListeArgs);
	if(!strcmp(CommandeDirecte,"non")) CommandeDirecte[0] = '\0'; // pour compatibilite avec -batch=<cmde>
	ModeScript = (CommandeDirecte[0] != '\0');
	if(debug_appel) printf("= Execution en mode %s\n",ModeScript? "batch": "interactif");
	if(creation_args) {
		ArgPrint("TangoArgs",TangoDesc);
		if(!SousUnix) {
			printf("Le fichier TangoArgs vient d'etre cree: corriges-le dans:\n   '%s'\n",start_dir);
			exit(0);
		}
	}
    XcodeTangoDebug = !strcmp(XcodeTangoDebugTxt,"YES");
    if(debug_appel) printf("  Execution en mode %s (NSDocumentRevisionsDebugMode: %s)\n",XcodeTangoDebug? "debug sous Xcode": "production",XcodeTangoDebugTxt);

	/* Creation des noms des differents fichiers */
	RepertoireNomComplet(start_dir,TopDir,1,FullTopDir);
	SambaAjouteTopDir(PrefsTango,TangoDir);
	SambaAjouteTopDir(SauvPrefs,SauvVol);
	SambaAjouteTopDir(ResuPrefs,ResuPath);
	SambaAjouteTopDir(FigsPrefs,FigsPath);
	SambaAjouteTopDir(DbazPrefs,DbazPath);
	SambaAjouteTopDir(InfoPrefs,TangoInfoDir);
	TangoCreePath(TangoDir,0,NomFiltres,FichierFiltres);
	TangoCreePath(TangoDir,1,AnaDir,AnaPath);
	TangoCreePath(AnaPath,1,GrafDir,GrafPath);
	TangoCreePath(AnaPath,1,CalibDir,CalibPath);
	TangoCreePath(AnaPath,1,CoupeDir,CoupePath);
	TangoCreePath(AnaPath,1,SubsetDir,SubsetPath);
	TangoCreePath(CalibPath,0,UniteEventName,UniteEventFile);
	TangoCreePath(CalibPath,0,UniteTempsName,UniteTempsFile);

	TangoCreePath(SauvVol,1,EvtsDir,EvtsPath);

	EvtAuPif = Rapide; Discontinu = Tolerant;
	
	printf("  Les runs seront relus depuis:\n   '%s'\n",EvtsPath);
	if(RunsName[0]) {
		char *liste[MAXDIR]; int nb;
		RepertoireListeCree(1,EvtsPath,liste,MAXDIR,&nb);
		l = strlen(RunsName);
		j = 1; k = 0;
		for(i=0; i<nb; i++) if((RunsName[0] == '.') || !strncmp(liste[i],RunsName,l)) {
			if(!k++) printf("\n%3d:",j); if(k >= 5) k = 0;
			printf("  %s",liste[i]);
		}
		printf("\n");
		RepertoireListeLibere(liste,nb);
		exit(0);
	}
	printf("  Les fichiers de configuration sont dans:\n   '%s'\n",PrefsTango);
	printf("\n");
	printf("  Les fichiers POSTSCRIPT sont par defaut dans:\n   '%s%s'\n",HomeDir,"Desktop");
	printf("  Les resultats seront stockes dans:\n   '%s'\n",ResuPath);
	printf("  La base de donnees des detecteurs est dans:\n   '%s'\n",DbazPath);
	printf("  Les figures seront sauvees dans:\n   '%s'\n",FigsPath);
	printf("\n");
	printf("  Les coefficients de calibration sont dans:\n   '%s'\n",CalibPath);
	printf("  Les selections d'evenements sont dans:\n   '%s'\n",SubsetPath);
	printf("  Les definitions de traces sont dans:\n   '%s'\n",GrafPath);
	printf("\n");
	
	FiltresInit(MAXFILTRES); 
	FiltresLit(FichierFiltres,1);
	printf(". %d filtres lus sur %s\n",FiltreNb,FichierFiltres);
	EvtUniteInit();
	ArgScan(UniteTempsName,EvtUniteTempsDesc);
	ArgKeyFromList(ClesMarquage,TexteMarquage);

	/* Initialise le processus de relecture des donnees */
	ArchReadInit(MaxEvts);         /* general */
	ArchInit(OptnSpec,EventName);  /* specifique manip (EDW ou autre) */
	r = EventName;
	ItemTrouve(&r,"[",&c);
	if(c == '[') {
		sscanf(ItemTrouve(&r,"-]",&c),"%d",&EventPartDebut);
		if(c == '-') {
			sscanf(ItemSuivant(&r,']'),"%d",&EventPartFin);
			if(EventPartFin == 0) EventPartFin = 999999;
		} else EventPartFin = EventPartDebut;
	}
	/* Noms complets du fichier des donnees */
	if(EventName[0]) strcat(strcpy(FichierEvents,EvtsPath),EventName);
	else { strcpy(FichierEvents,EvtsPath); DemandeFichier = 1; }

	/* Lecture des donnees */
	printf("***** Lecture des donnees demarree\n");
	Trigger.enservice = 1; // par defaut...
	if(!EvtRunAdd()) return(0);
	printf("***** Lecture des donnees terminee\n");
	if(Trigger.enservice) {
		PlotInit();
		for(qual=0; qual<WND_NBQUAL; qual++) PlotColors(qual,&(TangoColrFig[qual][0]),&(TangoColrFit[qual][0]));
		PlotRepertoire(GrafPath);
		EvtEspace = PlotEspaceCree("Evt",&EvtNb);
		PaqEspace = PlotEspaceCree("Paquet",&PaqNb);
		// ProdVarDump(__func__,8);
		if(!TangoProdInit()) return(0); /* contient ProdInit et PlotNtupleCree d'apres le fichier lu; rend FichierNtuple et FichierSupp */
		// ProdVarDump(__func__,9);

		/* Reperes pour la lecture des evenements */
		EvtLitUnSeul(0,0);       // printf("* Temps initial: %d,%06d\n",ArchEvt.sec,ArchEvt.msec);
		RunTempsInitial = ArchEvt.sec;
		EvtLitUnSeul(EvtNb-1,0); //	printf("* Temps final: %d,%06d\n",ArchEvt.sec,ArchEvt.msec);
		PaqEvtPremier = Malloc(EvtNb,int); // RE-ALLOUER EN CAS DE RELECTURE
		RunTempsTotal = (ArchEvt.sec - RunTempsInitial);
		if(RunTempsTotal) EvtTaux = EvtNb / (float)RunTempsTotal;
		EvtValides = EvtNb;
		// ProdVarDump(__func__,6);
	}
	printf("***** Initialisation generale terminee\n");

	/* Fin de l'initialisation generale et lancement du dialogue */
	if(ModeScript) {
	#define MAXLOOP 5
		FILE *f[MAXLOOP]; char cmde_lue[MAXLOOP][MAXFILENAME],*r[MAXLOOP];
		char nom_complet[MAXFILENAME]; char sep,*commande,*parm; char ack,erreur;

		TangoCreeCoupureDesc();
		printf(" _____ Execution immediate ___________________\n");
		f[ScriptLoop] = 0; r[ScriptLoop] = CommandeDirecte; ack = 0;
		if(ack) printf("| Lu: '%s'\n",r[ScriptLoop]);
		do {
			if(f[ScriptLoop]) {
				RepertoireSimplifie(TopDir);
				if(f[ScriptLoop] == stdin) printf("@%s: ",TopDir);
				if(fgets(&(cmde_lue[ScriptLoop][0]),MAXFILENAME,f[ScriptLoop])) {
					r[ScriptLoop] = &(cmde_lue[ScriptLoop][0]);
					l = strlen(r[ScriptLoop]); if(l < 1) continue; else l--;
					if(*(r[ScriptLoop]+l) == '\n') *(r[ScriptLoop]+l) = '\0';
					if(ack) TangoPrint(0,"Lu: '%s'",r[ScriptLoop]);
				} else {
					if(ferror(f[ScriptLoop])) {
						TangoPrint(OPIUM_ERROR,"Lecture impossible: %s",strerror(errno));
						while(ScriptLoop) { if(f[ScriptLoop] != stdin) fclose(f[ScriptLoop]); ScriptLoop--; }
						break;
					}
					fclose(f[ScriptLoop]); --ScriptLoop;
				}
			}
			erreur = 0;
			while(*r[ScriptLoop] && (*r[ScriptLoop] != '\n') && (*r[ScriptLoop] != '#') && !erreur) {
				commande = ItemJusquA(',',&r[ScriptLoop]);
				parm = commande; ItemAvant(": ",&sep,&parm);
				while((*parm != '\0') && (*parm == ' ')) parm++;
				l = strlen(parm); while(l && (parm[l-1] == ' ')) l--;
				if(ack) TangoPrint(OPIUM_WARN,"Commande %s : %s",commande,parm);
				if(!strcmp(commande,"help")) {
					TangoPrint(0,"Commandes:");
					TangoPrint(0,"  file:<fichier_script>");
					TangoPrint(0,"  file:.                 (commandes a suivre: au clavier)");
					TangoPrint(0,"  ntuple:calcule");
					TangoPrint(0,"  ntuple:relit");
					TangoPrint(0,"  coupures:<fichier_de_coupures>");
					TangoPrint(0,"  subset:<fichier_evenements>");
					TangoPrint(0,"  cd:<directory>");
					TangoPrint(0,"  ls:<directory>");
					TangoPrint(0,"  ack                    (rend compte des commandes lues)");
					TangoPrint(0,"  menus");
					TangoPrint(0,"  exit");
					TangoPrint(0,"Les commandes peuvent etre separees par des ','");
				} else if(!strcmp(commande,"file")) {
					if(ScriptLoop >= MAXLOOP) {
						TangoPrint(OPIUM_ERROR,"Deja %d imbrications",MAXLOOP); erreur = 1;
					}
					if(!strcmp(parm,".")) f[ScriptLoop+1] = stdin;
					else {
						strcat(strcpy(nom_complet,TopDir),parm);
						if(!(f[ScriptLoop+1] = fopen(nom_complet,"r"))) {
							TangoPrint(OPIUM_ERROR,"Fichier %s illisible: %s",nom_complet,strerror(errno));
							erreur = 1;
						}
					}
					ScriptLoop++;
					break;
				} else if(!strcmp(commande,"ntuple")) {
					if(!strcmp(parm,"calcule")) { ProdAmplitudes(); EvtNtupleEcrit(); }
					else if(!strcmp(parm,"relit")) EvtNtupleRelit();
				}else if(!strncmp(commande,"coupures",8)) {
					EvtCoupeJeu(parm);
				} else if(!strcmp(commande,"subset")) {
					EvtSelectionSauve(parm);
				} else if(!strcmp(commande,"cd")) {
					if(*parm == '/') strcpy(nom_complet,parm);
					else strcat(strcpy(nom_complet,TopDir),parm);
					RepertoireSimplifie(nom_complet);
					if(chdir(nom_complet) == 0) {
						strcpy(TopDir,nom_complet);
						RepertoireTermine(TopDir);
						TangoPrint(OPIUM_NOTE,"Repertoire de travail utilise: '%s'",TopDir);
					} else {
						TangoPrint(OPIUM_ERROR,"Repertoire de travail inutilisable: '%s' (%s)",nom_complet,strerror(errno));
						erreur = 1;
					}
				} else if(!strcmp(commande,"ls")) {
					char *liste[MAXDIR]; int i,k,nb;
					if(*parm == '/') strcpy(nom_complet,parm);
					else strcat(strcpy(nom_complet,TopDir),parm);
					RepertoireSimplifie(nom_complet);
					RepertoireListeCree(0,nom_complet,liste,MAXDIR,&nb);
					if(nb) {
						TangoPrint(OPIUM_NOTE,"Files of %s:",nom_complet);
						k = 0;
						for(i=0; i<nb; i++) {
							if(!k++) { if(i) printf("\n"); TangoPrint(0,0); printf("%3d:",i+1); }
							if(k >= 5) k = 0;
							if(strncmp(liste[i],"Icon",4)) printf("  %s",liste[i]); else printf("  Icon+");
						}
						printf("\n");
					} else TangoPrint(OPIUM_WARN,"No files in %s:",nom_complet);
					RepertoireListeLibere(liste,nb);
					RepertoireListeCree(1,nom_complet,liste,MAXDIR,&nb);
					if(nb) {
						TangoPrint(OPIUM_NOTE,"Subdirectories of %s:",nom_complet);
						k = 0;
						for(i=0; i<nb; i++) {
							if(!k++) { if(i) printf("\n"); TangoPrint(0,0); printf("%3d:",i+1); }
							if(k >= 5) k = 0;
							printf("  %s",liste[i]);
						}
						printf("\n");
					} else TangoPrint(OPIUM_WARN,"No subdirectories in %s:",nom_complet);
					RepertoireListeLibere(liste,nb);
				} else if(!strcmp(commande,"ack")) {
					ack = 1 - ack;
					TangoPrint(OPIUM_NOTE,"Les commandes %s confirmees",ack? "seront": "ne seront PLUS");
				} else if(!strcmp(commande,"menus")) {
					ModeScript = 0;
					TangoStartOpium(titre);
					ModeScript = 1;
				} else if(!strcmp(commande,"exit")) {
					while(ScriptLoop) { if(f[ScriptLoop] != stdin) fclose(f[ScriptLoop]); ScriptLoop--; }
				} else {
					TangoPrint(OPIUM_ERROR,"Commande %s inconnue",commande);
					erreur = 1;
				}
			}
			if((f[ScriptLoop] != stdin) && erreur) ScriptLoop = 0;
		} while(ScriptLoop > 0);
		printf("|_____ Fin d'execution du batch __________________\n\n");
	} else TangoStartOpium(titre);

	/* Fin de l'execution */
	ArchRunClose();
	ProdExit();
	PlotExit();
	ArchExit();  /* specifique manip (EDW ou autre) */
	OpiumExit();
	exit(0);
	return(0);
}
