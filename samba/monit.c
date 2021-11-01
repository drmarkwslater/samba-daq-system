#define BRANCHE_MONIT
#ifdef macintosh
#pragma message(__FILE__)
#endif

#define MONIT_C

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <environnement.h>
#include <dateheure.h>
#include <claps.h>
#include <decode.h>
#include <opium.h>
// int GraphDataRGB(Graph graph, int num, WndColorLevel r, WndColorLevel g, WndColorLevel b);
#include <samba.h>
#include <repartiteurs.h>
#include <objets_samba.h>
#include <monit.h>

#define MONIT_FEN_TYPES "signal/histo/FFT/fonction/biplot/2D-histo/evenement/evtmoyen/pattern"
static char *MonitFenTypeName[MONIT_NBTYPES+1] = {
	"signal", "histo", "fft", "fonction", "biplot", "2d-histo", "evenement", "evt moyen", "pattern", "\0"
};
static int MonitFenTypeAvecTrmt[] = { 1, 0, 1, 0, 0, 0, 1, 0, 0 };
static int MonitFenTypeAvecVar[]  = { 0, 1, 0, 1, 1, 1, 0, 0, 0 };

static char *MonitFenVarName[MONIT_NBVARS+1] = {
	"amplitude", "amplibrute", "abs(ampl)", "maximum",
	"tps.montee", "descente", "duree",
	"base", "bruit", "energie", "seuil", "decal", // "delai", deja pris en compte avec MONIT_DECAL
	"\0"
};

typedef enum {
	MONIT_EFFACE = 0,
	MONIT_INSERE,
	MONIT_GARDE,
	MONIT_NBACTIONS
} MONIT_ACTION;
#define MONIT_ACTION_CLES "retirer/inserer/garder"

static char   MonitASauver;
static Panel pMonitGeneral; // pFichierMonitLect,pFichierMonitEcri,
static Panel pMonitLimites1Voie;
static Menu  mMonitEvts;
static Term  tMonit;
static char   MonitTitre[80];
static WndColor *MonitLUTred,*MonitLUTall;
static char   UserFenetres[MAXFILENAME];
static char   MonitNomTypes[256];
static short  MonitEvtIndex[2];
static char   MonitColrFig[WND_NBQUAL][16],MonitColrFit[WND_NBQUAL][16];

/* #ifdef X11
#define Jaune "yellow"
#define Vert  "green"
#define Bleu  "blue"
#endif
#ifdef QUICKDRAW
#define Jaune "FFFFFFFF0000"
#define Vert  "0000FFFF0000"
#define Bleu  "00000000FFFF"
#endif */

static float MonitDuree;   /* duree de donnees brutes affichees (ms) (voie par voie) */
static int   MonitEvts;
static ArgDesc MonitDesc[] = {
	{ "Graphiques.hauteur",   DESC_INT   &MonitHaut,          "Hauteur des graphiques (pixels)" },
	{ "Graphiques.largeur",   DESC_INT   &MonitLarg,          "Largeur des graphiques (pixels)" },
	{ "Graphique.synchro",    DESC_BOOL  &MonitSynchro,       "Synchronisation avec la synchronisation" },
	{ "Donnees.duree",        DESC_FLOAT &MonitDuree,         "Duree de donnees affichee (ms)" },
	{ "Donnees.unite.chgt",   DESC_KEY MONIT_UNITE_CLES, &MonitChgtUnite, "Changement d'unite des donnees ("MONIT_UNITE_CLES")" },
	{ "Donnees.unite.mV",     DESC_FLOAT &MonitADUenmV,       "Valeur d'un ADU en mV" },
	{ "Donnees.unite.keV",    DESC_FLOAT &MonitADUenkeV,      "Valeur d'un ADU en keV" },
	{ "Donnees.evts",         DESC_INT   &MonitEvts,          "Nombre d'evenements maxi affiches en histo" },

	{ "Affichage.modes",      DESC_BOOL  &MonitAffModes,      "Affichage des modes de fonctionnement" },
	{ "Affichage.trigger",    DESC_BOOL  &MonitAffTrig,       "Affichage du trigger demande" },
	{ "Affichage.ldb",        DESC_BOOL  &MonitAffBases,      "Affichage des lignes de base" },
	{ "Affichage.taux.panel", DESC_BOOL  &MonitAffpTaux,      "Affichage du panel des taux d'evenements" },
	{ "Affichage.taux.graph", DESC_BOOL  &MonitAffgTaux,      "Affichage du graphique des taux d'evenements" },
	{ "Affichage.seuils",     DESC_BOOL  &MonitAffgSeuils,    "Affichage du graphique des seuils" },
	{ "Affichage.etat",       DESC_BOOL  &MonitAffEtat,       "Affichage de l'etat de l'acquisition" },
	{ "Affichage.reseau",     DESC_BOOL  &MonitAffNet,        "Affichage de l'etat du reseau" },
	{ "Affichage.donnees",    DESC_TEXT   UserFenetres,       "Fichier de definition de l'affichage des donnees brutes" },
	{ "Affichage.interv",     DESC_INT   &MonitIntervAff,     "Intervalle entre affichage des donnees (ms)" },
	{ "Affichage.evt",        DESC_BOOL  &MonitAffEvt,        "Affichage du dernier evenement" },

	{ "Histo.ampl",           DESC_BOOL  &MonitHampl,         "Affichage de l'histogramme des amplitudes" },
	{ "Histo.ampl.min",       DESC_INT   &(MonitHamplY[1]),   "Minimum pour l'histogramme des amplitudes" },
	{ "Histo.ampl.max",       DESC_INT   &(MonitHamplY[1]),   "Maximum pour l'histogramme des amplitudes" },
	{ "Histo.montee",         DESC_BOOL  &MonitHmontee,       "Affichage de l'histogramme des temps de montee" },
	{ "Histo.montee.min",     DESC_INT   &(MonitHmonteeY[0]), "Minimum pour l'histogramme des temps de montee" },
	{ "Histo.montee.max",     DESC_INT   &(MonitHmonteeY[1]), "Maximum pour l'histogramme des temps de montee" },
	{ "Histo.2D",             DESC_BOOL  &MonitH2D,           "Affichage de l'histo amplitude x tps montee" },
	{ "Histo.2D.min",         DESC_INT   &(MonitH2DY[0]),     "Minimum pour l'histo amplitude x tps montee" },
	{ "Histo.2D.max",         DESC_INT   &(MonitH2DY[1]),     "Maximum pour l'histo amplitude x tps montee" },

	DESC_END
};

/* ========================================================================== */
#define MONIT_NL
#ifdef MONIT_NL

/* ........................... Parametres communs a toutes les fenetres ........................... */

static ArgDesc MonitFenPosDesc[] = {
	{ "x",        DESC_INT   &(MonitFenLue.posx), 0 },
	{ "y",        DESC_INT   &(MonitFenLue.posy), 0 },
	DESC_END
};
static ArgDesc MonitFenDimDesc[] = {
	{ "l",        DESC_INT   &(MonitFenLue.larg), 0 },
	{ "h",        DESC_INT   &(MonitFenLue.haut), 0 },
	DESC_END
};

/* .......... Complements a la definition generale de la fenetre, en fonction de son type .......... */

static ArgDesc MonitFenComplSignalDesc[] = {
	{ "min",      DESC_INT   &(MonitFenLue.axeY.i.min), 0 },
	{ "max",      DESC_INT   &(MonitFenLue.axeY.i.max), 0 },
	{ "duree_ms", DESC_FLOAT &(MonitFenLue.axeX.r.max), 0 },
	DESC_END
};
static ArgDesc MonitFenComplHistoDesc[] = {
	{ "bin_min",  DESC_FLOAT &(MonitFenLue.axeX.r.min), 0 },
	{ "bin_max",  DESC_FLOAT &(MonitFenLue.axeX.r.max), 0 },
	{ "nb.bins",  DESC_INT   &(MonitFenLue.axeX.pts), 0 },
	{ "max.evts", DESC_INT   &(MonitFenLue.axeY.i.max), 0 },
	DESC_END
};
static ArgDesc MonitFenComplFftDesc[] = {
	{ "nb.points",   DESC_INT   &(MonitFenLue.axeX.pts), 0 },
	{ "freq_min",    DESC_FLOAT &(MonitFenLue.axeX.r.min), 0 },
	{ "freq_max",    DESC_FLOAT &(MonitFenLue.axeX.r.max), 0 },
	{ "bruit_min",   DESC_FLOAT &(MonitFenLue.axeY.r.min), 0 },
	{ "bruit_max",   DESC_FLOAT &(MonitFenLue.axeY.r.max), 0 },
	{ "intervalles", DESC_INT   &(MonitFenLue.p.f.intervalles), 0 },
	{ "axe_X",       DESC_KEY "lineaire/log", &(MonitFenLue.p.f.xlog), 0 },
	{ "hors_evts",   DESC_KEY "non/active",   &(MonitFenLue.p.f.excl), 0 },
	DESC_END
};
static ArgDesc MonitFenComplFonctionDesc[] = {
	{ "min",      DESC_FLOAT &(MonitFenLue.axeY.r.min), 0 },
	{ "max",      DESC_FLOAT &(MonitFenLue.axeY.r.max), 0 },
	{ "nb.evts",  DESC_INT   &(MonitFenLue.axeX.i.max), 0 },
	DESC_END
};
static ArgDesc MonitFenComplBiplotDesc[] = {
	{ "X.min",  DESC_FLOAT &(MonitFenLue.axeX.r.min), 0 },
	{ "X.max",  DESC_FLOAT &(MonitFenLue.axeX.r.max), 0 },
	{ "Y.min",  DESC_FLOAT &(MonitFenLue.axeY.r.min), 0 },
	{ "Y.max",  DESC_FLOAT &(MonitFenLue.axeY.r.max), 0 },
	DESC_END
};
static ArgDesc MonitFenComplH2dDesc[] = {
	{ "axe_X",  DESC_KEY "lineaire/log", &(MonitFenLue.p.s.xlog), 0 },
	DESC_END
};
static ArgDesc MonitFenComplEvtDesc[] = {
	{ "min",      DESC_INT   &(MonitFenLue.axeY.i.min), 0 },
	{ "max",      DESC_INT   &(MonitFenLue.axeY.i.max), 0 },
	{ "duree_ms", DESC_FLOAT &(MonitFenLue.axeX.r.max), 0 },
	DESC_END
};
static ArgDesc MonitFenComplMoyenDesc[] = {
	{ "min", DESC_INT &(MonitFenLue.axeY.i.min), 0 },
	{ "max", DESC_INT &(MonitFenLue.axeY.i.max), 0 },
	DESC_END
};
static ArgDesc MonitFenComplPatternDesc[] = {
	{ "min", DESC_INT &(MonitFenLue.axeY.i.min), 0 },
	{ "max", DESC_INT &(MonitFenLue.axeY.i.max), 0 },
	DESC_END
};
static ArgDesc *MonitFenComplDesc[] = {
	MonitFenComplSignalDesc,     MonitFenComplHistoDesc,    MonitFenComplFftDesc,
	MonitFenComplFonctionDesc,   MonitFenComplBiplotDesc,   MonitFenComplH2dDesc,
	MonitFenComplEvtDesc,        MonitFenComplMoyenDesc,    MonitFenComplPatternDesc,
	0
};

/* ......................... Definition des traces en fonction du type de la fenetre ......................... */

static TypeMonitTrace MonitTraceLue;

/* ... Nom de la voie (detecteur + capteur) ... */
static ArgDesc MonitTraceBoloCaptDesc[] = {
//!	{ "detecteur", DESC_LISTE BoloNom,           &MonitTraceLue.bolo, 0 },
//!	{ DESC_EXEC MonitTraceListeCapt, pointeur_sur_donnees }, // void MonitTraceListeCapt(void *pointeur_sur_donnees)
//!	{ "capteur",   DESC_STR_DIM(MODL_NOM,DETEC_CAPT_MAX) MonitTraceLue.liste_voies, 0 },
	{ "capteur",   DESC_STR(MODL_NOM)   MonitTraceLue.voie_lue, 0 },
	{ "detecteur", DESC_LISTE BoloNom, &MonitTraceLue.bolo, 0 },
	DESC_END
}; static ArgDesc *MonitTraceBoloCaptDescPtr = MonitTraceBoloCaptDesc;

/* ... Couleur de la trace ... */
static ArgDesc MonitTracePcentDesc[] = {
	{ "rouge",   DESC_FLOAT &(MonitTraceLue.rgb.r), 0 },
	{ "vert",    DESC_FLOAT &(MonitTraceLue.rgb.g), 0 },
	{ "bleu",    DESC_FLOAT &(MonitTraceLue.rgb.b), 0 },
	DESC_END
};
static ArgDesc MonitTraceColorDesc[] = {
	{ "couleur", DESC_STRUCT &MonitTracePcentDesc, 0 },
	DESC_END
}; static ArgDesc *MonitTraceColorDescPtr = MonitTraceColorDesc;

/* ......................... Liste des traces en fonction du type de la fenetre ......................... */

/* ... Donnee des tampons, brute ou traitee: "Signal" ... */
static ArgDesc MonitTraceSignalDesc[] = {
	DESC_INCLUDE(MonitTraceBoloCaptDescPtr),
	{ "etat",    DESC_LISTE TrmtClassesListe, &(MonitTraceLue.var), 0 }, // nom du tampon
	DESC_INCLUDE(MonitTraceColorDescPtr),
	DESC_END
};
static ArgStruct MonitFenTracesSignalAS  = { (void *)MonitFenLue.trace, (void *)&MonitTraceLue, sizeof(TypeMonitTrace), MonitTraceSignalDesc };
static ArgDesc MonitFenTracesSignalDesc[] = {
	{ "donnees", DESC_STRUCT_SIZE(MonitFenLue.nb,MAXTRACES) &MonitFenTracesSignalAS, 0 },
	DESC_END
};

/* ... Histogramme d'un ntuple: "Histo" ... */
static ArgDesc MonitTraceVarsDesc[] = {
	{ "var",     DESC_LISTE MonitFenVarName, &MonitTraceLue.var, 0 }, // nom du ntuple
	DESC_INCLUDE(MonitTraceBoloCaptDescPtr),
	DESC_INCLUDE(MonitTraceColorDescPtr),
	DESC_END
};
static ArgStruct MonitFenTracesHistoAS  = { (void *)MonitFenLue.trace, (void *)&MonitTraceLue, sizeof(TypeMonitTrace), MonitTraceVarsDesc };
static ArgDesc MonitFenTracesHistoDesc[] = {
	{ "donnees", DESC_STRUCT_SIZE(MonitFenLue.nb,MAXTRACES) &MonitFenTracesHistoAS, 0 },
	DESC_END
};

/* ... Resultat d'un traitement des donnees (pattern ou evt moyen): "Resul" ... */
static ArgDesc MonitTraceResulDesc[] = {
	DESC_INCLUDE(MonitTraceBoloCaptDescPtr),
	DESC_INCLUDE(MonitTraceColorDescPtr),
	DESC_END
};
static ArgStruct MonitFenTracesResulAS  = { (void *)MonitFenLue.trace, (void *)&MonitTraceLue, sizeof(TypeMonitTrace), MonitTraceResulDesc };
static ArgDesc MonitFenTracesResulDesc[] = {
	{ "donnees", DESC_STRUCT_SIZE(MonitFenLue.nb,MAXTRACES) &MonitFenTracesResulAS, 0 },
	DESC_END
};

/* ... Graphique a deux ntuples: on doit avoir 2 et seulement 2 traces de type ntuple ... */
static ArgDesc MonitTraceXvarDesc[] = {
	{ "var",       DESC_LISTE MonitFenVarName, &MonitFenLue.trace[0].var, 0 }, // nom du ntuple
	{ "capteur",   DESC_STR(MODL_NOM)           MonitFenLue.trace[0].voie_lue, 0 },
	{ "detecteur", DESC_LISTE BoloNom,         &MonitFenLue.trace[0].bolo, 0 },
	{ "bin_min",   DESC_FLOAT                  &(MonitFenLue.axeX.r.min), 0 },
	{ "bin_max",   DESC_FLOAT                  &(MonitFenLue.axeX.r.max), 0 },
	{ "nb.bins",   DESC_INT                    &(MonitFenLue.axeX.pts), 0 },
	DESC_END
};
static ArgDesc MonitTraceYvarDesc[] = {
	{ "var",       DESC_LISTE MonitFenVarName, &MonitFenLue.trace[1].var, 0 }, // nom du ntuple
	{ "capteur",   DESC_STR(MODL_NOM)           MonitFenLue.trace[1].voie_lue, 0 },
	{ "detecteur", DESC_LISTE BoloNom,         &MonitFenLue.trace[1].bolo, 0 },
	{ "bin_min",   DESC_FLOAT                  &(MonitFenLue.axeY.r.min), 0 },
	{ "bin_max",   DESC_FLOAT                  &(MonitFenLue.axeY.r.max), 0 },
	{ "nb.bins",   DESC_INT                    &(MonitFenLue.axeY.pts), 0 },
	DESC_END
};
static ArgDesc MonitFenTracesH2dDesc[] = {
	{ "X",       DESC_STRUCT &MonitTraceXvarDesc, 0 },
	{ "Y",       DESC_STRUCT &MonitTraceYvarDesc, 0 },
	DESC_END
};

static ArgDesc *MonitFenTracesDesc[] = {
	MonitFenTracesSignalDesc, MonitFenTracesHistoDesc,  MonitFenTracesSignalDesc,
	MonitFenTracesHistoDesc,  MonitFenTracesHistoDesc,  MonitFenTracesH2dDesc,
	MonitFenTracesSignalDesc, MonitFenTracesResulDesc,  MonitFenTracesResulDesc,
	
	0
};

/* ................................... Structure du fichier ............................................. */

// #define MONIT_FEN_TYPES "signal/histo/FFT/fonction/biplot/histo2d/evenement/evtmoyen/pattern"
//                            0      1    2     3       4       5       6         7        8
static ArgDesc MonitFenLueDesc[] = {
	{ 0, DESC_STR(MONIT_FENTITRE)             MonitFenLue.titre,    0 },
	{ 0, DESC_KEY MONIT_FEN_TYPES,          &(MonitFenLue.type),    0 },
	{ 0, DESC_VARIANTE(MonitFenLue.type)      MonitFenComplDesc,    0 },
	{ 0, DESC_VARIANTE(MonitFenLue.type)      MonitFenTracesDesc,   0 },
	{ "dimension", DESC_STRUCT               &MonitFenDimDesc,      0 },
	{ "position",  DESC_STRUCT               &MonitFenPosDesc,      0 },
	{ "affichage", DESC_KEY "non/oui/tjrs", &(MonitFenLue.affiche), 0 },
	{ "grille",    DESC_BOOL                &(MonitFenLue.grille),  0 },
	DESC_END
};
static ArgStruct MonitFenAS  = { (void *)MonitFen, (void *)&MonitFenLue, sizeof(TypeMonitFenetre), MonitFenLueDesc };
static ArgDesc MonitFenDesc[] = {
	{ "Fenetres", DESC_STRUCT_SIZE(MonitFenNb,MAXMONIT) &MonitFenAS, 0 },
	DESC_END
};

#endif /* MONIT_NL */

static char  MonitNouvelle[80];
static int   MonitVoieVal0,MonitVoieValN;
static Panel pMonitVoiePrint;

static Panel pMonitFen,pMonitFenZoomData,pMonitFenZoomEvts,pMonitFenZoomHisto,pMonitFenZoomFft;
static Panel pMonitFenZoomFctn,pMonitFenZoomBiplot,pMonitFenZoomH2D,pMonitFenZoomMoyen;
static Panel pMonitTraces,pMonitFenAffiche,pMonitFenDetecteur,pMonitFenChange;
static int   MonitBoloAvant;
// static TypeMonitAxe MonitX,MonitY;
// static int   MonitMin,MonitMax;
// static float MonitRMin,MonitRMax;
// static float MonitFMin;
static char  MonitEditTraces,MonitFenModifiees;
static Panel pMonitFenLect,pMonitFenEcri;
static Panel pMonitEcran;

static Graph gFiltree[VOIES_MAX];
static float FiltreeAb6[2];

static int MonitFenDecode();
static int MonitFenEncode();

/* ========================================================================== */
int MonitGeneral() {
	if(OpiumExec(pMonitGeneral->cdr) == PNL_CANCEL) return(0);
	MonitFenLue.larg = MonitLarg;
	MonitFenLue.haut = MonitHaut;
	MonitASauver = 1;

	switch(MonitChgtUnite) {
	  case MONIT_ADU:
		ValeurADU = 1.0;
		break;
	  case MONIT_ENTREE_ADC:
		ValeurADU = MonitADUenmV;
		break;
	  case MONIT_KEV:
		ValeurADU = MonitADUenkeV;
		break;
	  case MONIT_ENTREE_BB:
//		ValeurADU = MonitADUenmV / GainBB[voie][bolo];
//		break;
	  case MONIT_SORTIE_BOLO:
//		ValeurADU = MonitADUenmV * 1000000.0 / (GainBB[voie][bolo] * Bolo[bolo].gainFET[voie]);
//		break;
	  default:
		OpiumError("Unite non programmee. On revient aux ADUs");
		MonitChgtUnite = MONIT_ADU;
		ValeurADU = 1.0;
		break;
	}

	return(0);
}
/* ========================================================================== */
void MonitFenDump(TypeMonitFenetre *f) {
	TypeMonitTrace *t; TypeMonitRgb *nuance; int j;

	printf("Fenetre %-32s ",f->titre);
	if(f->type == MONIT_HISTO)
		printf("[%-5g .. %5g] ",f->axeX.r.min,f->axeX.r.max);
	else if((f->type == MONIT_FFT) || (f->type == MONIT_FONCTION))
		printf("[%-5g .. %5g] ",f->axeY.r.min,f->axeY.r.max);
	else printf("[%-5d .. %5d] ",f->axeY.i.min,f->axeY.i.max);
	switch(f->type) {
		case MONIT_SIGNAL:
			printf(L_("(%g ms de signal)"),f->axeX.r.max);
			break;
		case MONIT_EVENT:
			printf("(%s)",MonitFenTypeName[f->type]);
			break;
		case MONIT_HISTO:
			printf(L_("(%s sur %d bins)"),MonitFenTypeName[f->type],f->axeX.pts);
			break;
		case MONIT_FFT:
			printf(L_("(%s sur %d x %d echantillons %s evts)"),MonitFenTypeName[f->type],
				   f->p.f.intervalles,f->axeX.pts,f->p.f.excl? "avec": "hors");
			break;
		case MONIT_PATTERN:
		case MONIT_MOYEN:
			printf("(%s%s)",MonitFenTypeName[f->type],f->nb?"s":"");
			break;
		case MONIT_FONCTION:
		case MONIT_BIPLOT:
		case MONIT_2DHISTO:
			break;
	}
	printf(": %d voie%c\n",f->nb,(f->nb > 1)? 's': ' ');
	t = f->trace;
	for(j=0; j<f->nb; j++,t++) {
		switch(f->type) {
			case MONIT_PATTERN:
			case MONIT_MOYEN:
				printf("    %-16s",VoieManip[(int)t->voie].nom);
				break;
			case MONIT_SIGNAL:
			case MONIT_EVENT:
			case MONIT_FFT:
				printf("    %-16s %-8s",VoieManip[t->voie].nom,TrmtClassesListe[t->var]);
				break;
			case MONIT_HISTO:
				printf("    %-12s %-16s",MonitFenVarName[t->var],VoieManip[t->voie].nom);
				break;
			case MONIT_FONCTION:
			case MONIT_BIPLOT:
			case MONIT_2DHISTO:
				break;
		}
		nuance = &(t->rgb);
		printf(" (%6.2f%% rouge, %6.2f%% verte, %6.2f%% bleue)\n",nuance->r,nuance->g,nuance->b);
	}
	printf("            fenetre %d x %d en (%d, %d)\n",f->larg,f->haut,f->posx,f->posy);
}
/* ========================================================================== */
static int MonitFenListeCapteurs(Panel panel, int num, void *arg) {
	int bolo,cap; int i;
	
	i = (int)arg;
	bolo = MonitFenEnCours->trace[i].bolo;
	for(cap=0; cap<Bolo[bolo].nbcapt; cap++) MonitFenEnCours->trace[i].liste_voies[cap] = Bolo[bolo].captr[cap].nom;
	MonitFenEnCours->trace[i].liste_voies[cap] = "\0";
	if(MonitFenEnCours->trace[i].cap >= Bolo[bolo].nbcapt) MonitFenEnCours->trace[i].cap = 0;
//	printf("(%s) Liste des voies du detecteur '%s':\n",__func__,Bolo[bolo].nom);
//	for(cap=0; cap<Bolo[bolo].nbcapt; cap++) printf("'%s'\n",MonitFenEnCours->trace[i].liste_voies[cap]);
	
	return(0);
}
/* ========================================================================== */
static void MonitCouleurVersPcent(OpiumColor *c, TypeMonitRgb *rgb) {
	rgb->r = (float)(c->r) * WND_COLORFLOAT;
	rgb->g = (float)(c->g) * WND_COLORFLOAT;
	rgb->b = (float)(c->b) * WND_COLORFLOAT;
}
/* ========================================================================== */
static void MonitPcentVersCouleur(TypeMonitRgb *rgb, OpiumColor *c) {
	c->r = rgb->r / WND_COLORFLOAT;
	c->g = rgb->g / WND_COLORFLOAT;
	c->b = rgb->b / WND_COLORFLOAT;
}
/* ========================================================================== */
static void MonitCouleurDefaut(OpiumColor *c) {
	c->r = WndLevelBgnd[WND_Q_ECRAN];   // 0x0000
	c->g = WndLevelText[WND_Q_ECRAN];   // 0xFFFF
	c->b = WndLevelBgnd[WND_Q_ECRAN];   // 0x0000
}
/* ========================================================================== */
static void MonitFenDefault(TypeMonitFenetre *f, char *nom) {
	TypeMonitTrace *t;

	strncpy(f->titre,nom,80);
	f->type = MONIT_SIGNAL;
	f->affiche = 1; f->grille = 0;
	f->change = 0;
	f->posx = f->posy = OPIUM_AUTOPOS;
	f->larg = MonitLarg; f->haut = MonitHaut;
//	f->axeX.r.min = 0; f->axeX.r.max = MonitDuree; f->axeX.pts = 100;
//	f->axeY.i.min = -8192; f->axeY.i.max = 8191; f->axeY.i.max = MonitEvts;
	f->axeX.i.min = 0; f->axeX.i.max = 0; f->axeX.pts = 100;
	f->axeY.i.min = 0; f->axeY.i.max = 0; f->axeY.pts = 100;
	f->g = 0;
	f->p.h.histo = 0;
	f->nb = 0;
	t = &(f->trace[0]);
	t->bolo = 0; t->cap = 0; t->voie = 0; /* t->vm = 0; */ t->var = TRMT_AU_VOL;
	MonitFenEnCours = f; MonitFenListeCapteurs(0,0,(void *)0);
	MonitCouleurDefaut(&(t->couleur));
	MonitCouleurVersPcent(&(t->couleur),&(t->rgb));
}
/* ========================================================================== */
static inline void MonitFenDimMin(TypeMonitFenetre *f) {
	if(f->larg < 40) f->larg = 40; if(f->haut < 40) f->haut = 40;
}
/* ========================================================================== */
int MonitFenLit() {
	if(OpiumExec(pMonitFenEcri->cdr) == PNL_CANCEL) return(0);
	return(MonitFenDecode());
}
/* ========================================================================== */
static int MonitFenDecode() {
	FILE *fichier;
	int i,j,l; int nb_averts,nb_erreurs;
	int bolo,voie,cap; int var;
	char nombolo[DETEC_NOM],nomvoie[MODL_NOM],nomvar[16];
	char *ptrbolo,*ptrvoie,*ptrvar,*ptr;
	char *r,*nom,*parametre,ligne[256],titre[80];
	char entree_traces;
	TypeMonitFenetre *f; TypeMonitTrace *t; OpiumColor *c;

	fichier = fopen(MonitFenFichier,"r");
	if(!fichier && InstalleSamba) {
		TypeMonitFenetre *f; TypeMonitTrace *t; int voie;
		printf("> Installe des graphiques  dans '%s'\n",MonitFenFichier);
		f = MonitFen;
		voie = 0;
		sprintf(f->titre,"Trace %s",VoieManip[voie].nom);
		f->type = MONIT_SIGNAL;
		f->axeX.r.max = 1000.0 / Echantillonnage;
		f->axeY.i.min = -8192; f->axeY.i.max = 8191;
		f->larg = 400; f->haut = 250; f->posx = 20; f->posy = 200; f->affiche = 1; f->grille = 0;
		f->nb = 1;
		t = f->trace;
		t->voie = voie;
		MonitCouleurDefaut(&(t->couleur));
		MonitCouleurVersPcent(&(t->couleur),&(t->rgb));
		MonitFenNb++;
		MonitFenEncode();
		fichier = fopen(MonitFenFichier,"r");
	}
	if(!fichier) {
		printf("! Fichier des graphiques illisible: '%s' (%s)\n",MonitFenFichier,strerror(errno));
		return(0);
	}
	printf("= Lecture des graphiques                dans '%s'\n",MonitFenFichier);

	MonitEditTraces = 1;
	MonitFenNb = 0; MonitFenNum = 0;
	strcpy(MonitFen[MonitFenNb].titre,MonitNouvelle);
	MonitFen[MonitFenNb + 1].titre[0] = '\0';
	nb_erreurs = nb_averts = 0;

	i = 0; f = MonitFen; l = 0;
	entree_traces = 0;

	do {
		r = LigneSuivante(ligne,256,fichier);
		if(!r) break;
		ligne[255] = '\0';
		l++;
        if((l == 1) && !strncmp(r+2,DESC_SIGNE,strlen(DESC_SIGNE))) {
		#ifdef MONIT_NL
			// ArgDebug(1);
			ArgFromFile(fichier,MonitFenDesc,0);
			// ArgDebug(0);
			for(i=0, f=MonitFen; i<MonitFenNb; i++,f++) {
				if(f->type == MONIT_HISTO) f->p.h.histo = 0;
				else if(f->type == MONIT_2DHISTO) { f->p.s.histo = 0; f->nb = 2; }
				for(j=0; j<f->nb; j++) {
					if(f->type != MONIT_2DHISTO) MonitPcentVersCouleur(&(f->trace[j].rgb),&(f->trace[j].couleur));
					bolo = f->trace[j].bolo;
					for(cap=0; cap<Bolo[bolo].nbcapt; cap++) if(!strcmp(f->trace[j].voie_lue,Bolo[bolo].captr[cap].nom)) break;
					if(cap >= Bolo[bolo].nbcapt) {
						cap = 0;
						printf("! Le detecteur %s n'a pas de capteur %s\n",Bolo[bolo].nom,f->trace[j].voie_lue);
						nb_erreurs++;
					}
					f->trace[j].cap = cap;
					f->trace[j].voie = Bolo[bolo].captr[cap].voie;
					if(f->type == MONIT_FFT) f->p.f.spectre[j] = 0;
					MonitFenEnCours = f; MonitFenListeCapteurs(0,0,(void *)(long)j);
				}
				// MonitFenDump(f);
			}
		#endif
			break;
        }
		if((*r == '#') || (*r == '\n')) continue;
		if(*r == '+') {
			i = MonitFenNb;
			if(MonitFenNb < MAXMONIT) {
				if(i) {
					if(f->affiche) MonitFenAffiche(&(MonitFen[i - 1]),0);
					f++;
				}
				MonitFenNb++;
				strcpy(MonitFen[MonitFenNb].titre,MonitNouvelle);
				MonitFen[MonitFenNb + 1].titre[0] = '\0';
			} else 	{
				printf("  ! ligne %d: Pas plus de %d fenetres. Dernieres fenetres non relues\n",l,MAXMONIT);
				nb_erreurs++;
				break;
			}
			r++;
			nom = ItemSuivant(&r,'/');
			if(*nom != '\0') {
				if(!strcmp(nom,MonitNouvelle)) {
					printf("  ! ligne %d: Titre interdit\n",l);
					nb_averts++;
					*nom = '\0';
				} else {
					for(j=0; j<i; j++) if(!strcmp(nom,MonitFen[j].titre)) break;
					if(j < i) {
						printf("  ! ligne %d: Titre '%s' deja utilise par la fenetre #%d\n",l,nom,j+1);
						nb_averts++;
						*nom = '\0';
					}
				}
			}
			if(*nom == '\0') {
				sprintf(titre,"Fenetre #%d",i+1);
				MonitFenDefault(f,titre);
			} else MonitFenDefault(f,nom);
			f->nb = 0;
			entree_traces = (*r == '\0');
		}
		if(!entree_traces) {
			while(*r != '\0') {
				parametre = ItemSuivant(&r,'/');
				if(*parametre == '\0') continue;
				nom = ItemSuivant(&parametre,'=');
				if(!strcmp(nom,"donnees")) {
					for(f->type=0; f->type<MONIT_NBTYPES; f->type += 1) {
						if(!strcmp(parametre,MonitFenTypeName[f->type])) break;
					}
					if(f->type == MONIT_NBTYPES) f->type = MONIT_SIGNAL;
					if(f->type == MONIT_HISTO) {
						f->p.h.histo = 0;
						f->axeX.r.min = 0.0; f->axeX.r.max = 0.0;
					} else if(f->type == MONIT_FFT) {
						f->axeX.r.min = 0.0;  f->axeX.r.max = 100.0;
						f->axeY.r.min = 0.01; f->axeY.r.max = 10000.0;
						f->p.f.xlog = 0;
						f->p.f.excl = 0;
						f->p.f.intervalles = 1;
					}
				}
				else if(!strcmp(nom,"duree"))
					sscanf(parametre,"%g",&(f->axeX.r.max));
				else if(!strcmp(nom,"freq-min"))
					sscanf(parametre,"%g",&(f->axeX.r.min));
				else if(!strcmp(nom,"freq-max"))
					sscanf(parametre,"%g",&(f->axeX.r.max));
				else if(!strcmp(nom,"bins"))
					sscanf(parametre,"%d",&(f->axeX.pts));
				else if(!strcmp(nom,"echantillons"))
					sscanf(parametre,"%d",&(f->axeX.pts));
				else if(!strcmp(nom,"evts")) {
					if(f->type == MONIT_HISTO) sscanf(parametre,"%d",&(f->axeY.i.max));
					else if(f->type == MONIT_FFT) f->p.f.excl = strcmp(parametre,"avec")? 0: 1;
				} else if(!strcmp(nom,"axeX")) {
					if(!strcmp(parametre,"log")) f->p.f.xlog = 1;
					else f->p.f.xlog = 0;
				} else if(!strcmp(nom,"interv")) {
					sscanf(parametre,"%d",&(f->p.f.intervalles));
					if(f->p.f.intervalles <= 0) f->p.f.intervalles = 1;
				} else if(!strcmp(nom,"limites")) {
					if(f->type == MONIT_HISTO)
						sscanf(parametre,"%g %g",&(f->axeX.r.min),&(f->axeX.r.max));
						// printf("- Limites[%s] = [%g, %g]\n",f->titre,f->axeX.r.min,f->axeX.r.max);
					else if(f->type == MONIT_FFT)
						sscanf(parametre,"%g %g",&(f->axeY.r.min),&(f->axeY.r.max));
						// printf("- Limites[%s] = [%g, %g]\n",f->titre,f->axeY.r.min,f->axeY.r.max);
					else sscanf(parametre,"%d %d",&(f->axeY.i.min),&(f->axeY.i.max));
				} else if(!strcmp(nom,"dimensions")) {
					sscanf(parametre,"%d %d",&(f->larg),&(f->haut));
					MonitFenDimMin(f);
					// printf("- Dimensions[%s] = (%d, %d)\n",f->titre,f->larg,f->haut);
				} else if(!strcmp(nom,"position"))
					sscanf(parametre,"%d %d",&(f->posx),&(f->posy));
				else if(!strcmp(nom,"affichage")) {
					if(*parametre == '\0') f->affiche = 1;
					else f->affiche = ((*parametre == 'n') || (*parametre == '0'))? 0: ((*parametre == 't')? 2: 1);
				} else if(!strcmp(nom,"grille")) {
					if(*parametre == '\0') f->grille = 1;
					else f->grille = ((*parametre == 'n') || (*parametre == '0'))? 0: 1;
				} else if(!strncmp(nom,"trace",5)) {
					entree_traces = 1;
					r = parametre;
					break;
				}
			}
		}
		if(entree_traces && (*r != '\0')) {
			if(f->nb >= MAXTRACES) {
				printf("  ! ligne %d: Pas plus de %d traces par fenetre\n",l,MAXTRACES);
				nb_erreurs++;
				continue;
			}
			t = f->trace + f->nb;
			t->ajuste = 0;
			c = &(t->couleur);
			nom = ItemSuivant(&r,'(');
			if(*r) {
				/* nouveau format: couleurs entre (), donc noms reperables */
				parametre = ItemSuivant(&r,')');
				sscanf(parametre,"%hX %hX %hX",&(c->r),&(c->g),&(c->b));
				MonitCouleurVersPcent(c,&(t->rgb));
				r = nom;
				if(f->type == MONIT_HISTO) {
					ptrvar = ItemSuivant(&r,' ');
					ptrvoie = ItemSuivant(&r,' ');
					ptrbolo = ItemSuivant(&r,' ');
					if(ptrbolo[0] == '\0') { if(BoloNb > 1) { ptr = ptrbolo; ptrbolo = ptrvoie; ptrvoie = ptr; } } /* cas de la voie ou du bolo unique */
					for(var=0; var<MONIT_NBVARS; var++) if(!strcmp(ptrvar,MonitFenVarName[var])) break;
					if(var == MONIT_NBVARS) {
						printf("  ! ligne %d: Variable '%s' inconnue\n",l,ptrvar);
						nb_erreurs++;
						continue;
					}
					t->var = var;
				} else {
					ptrvoie = ItemSuivant(&r,' ');
					ptrbolo = ItemSuivant(&r,' ');
					ptrvar = ItemSuivant(&r,' ');
					if(ptrvar[0] == '\0') { ptr = ptrvar; ptrvar = ptrbolo; if(BoloNb > 1) { ptrbolo = ptrvoie; ptrvoie = ptr; } } /* cas de la voie ou du bolo unique */
					if(ptrvar[0] == '\0') t->var = TRMT_AU_VOL;
					else {
						for(var=0; var<TRMT_NBCLASSES; var++) if(!strcmp(ptrvar,TrmtClassesListe[var])) break;
						if(var == TRMT_NBCLASSES) {
							printf("  ! ligne %d: Type de signal '%s' inconnu\n",l,ptrvar);
							nb_erreurs++;
							continue;
						}
						t->var = var;
					}
				}
				if(BoloNb == 1) bolo = 0; /* cas du detecteur unique */
				else {
					if(ptrbolo[0] == '\0') { ptr = ptrbolo; ptrbolo = ptrvoie; ptrvoie = ptr; } /* cas de la voie unique? */
					for(bolo=0; bolo<BoloNb; bolo++) if(!strcmp(ptrbolo,Bolo[bolo].nom)) break;
					if(bolo == BoloNb) {
						printf("  ! ligne %d: Detecteur '%s' inconnu\n",l,ptrbolo);
						nb_erreurs++;
						continue;
						/* Finalement, c'est pas graaave
						 } else if(Bolo[bolo].lu == DETEC_HS) {
						 MonitBoloAvant = bolo; f->change = 1;
						 if(!nb_erreurs) printf("* Fichier %s en erreur:\n",MonitFenFichier);
						 printf("  . ligne %d: Bolo '%s' hors service\n",l,ptrbolo);
						 nb_erreurs++; */
						/* continue; si on accepte, c'est possible de garder la fenetre */
					}
				}
				if((ptrvoie[0] == '\0') || (Bolo[bolo].nbcapt == 1)) cap = 0;
				else for(cap=0; cap<Bolo[bolo].nbcapt; cap++) {
					if(!strcmp(ptrvoie,Bolo[bolo].captr[cap].nom)) break;
				}
				if(cap == Bolo[bolo].nbcapt) {
					printf("  ! ligne %d: Pas de voie '%s' pour le detecteur '%s'\n",l,ptrvoie,Bolo[bolo].nom);
					nb_erreurs++;
					voie = Bolo[bolo].captr[0].voie;
				} else {
					voie = Bolo[bolo].captr[cap].voie; 
				}
			} else {
				/* ancien format: couleurs a la volee, donc noms pas reperables */
				if(f->type == MONIT_HISTO) {
					if(BoloNb > 1) sscanf(nom,"%s %s %s %hX %hX %hX\n",nomvar,nomvoie,nombolo,&(c->r),&(c->g),&(c->b));
					else sscanf(nom,"%s %s %hX %hX %hX\n",nomvar,nomvoie,&(c->r),&(c->g),&(c->b));
					for(var=0; var<MONIT_NBVARS; var++) if(!strcmp(nomvar,MonitFenVarName[var])) break;
					if(var == MONIT_NBVARS) {
						printf("  ! ligne %d: Variable '%s' inconnue\n",l,nomvar);
						nb_erreurs++;
						continue;
					}
					t->var = var;
				} else if(f->type == MONIT_SIGNAL) {
					if(BoloNb > 1) sscanf(nom,"%s %s %s %hX %hX %hX\n",nomvoie,nombolo,nomvar,&(c->r),&(c->g),&(c->b));
					else sscanf(nom,"%s %s %hX %hX %hX\n",nomvoie,nomvar,&(c->r),&(c->g),&(c->b));
					for(var=0; var<TRMT_NBCLASSES; var++) if(!strcmp(nomvar,TrmtClassesListe[var])) break;
					if(var == TRMT_NBCLASSES) {
						printf("  ! ligne %d: Type de signal '%s' inconnu\n",l,nomvar);
						nb_erreurs++;
						continue;
					}
					t->var = var;
				} else {
					if(BoloNb > 1) sscanf(nom,"%s %s %hX %hX %hX\n",nomvoie,nombolo,&(c->r),&(c->g),&(c->b));
					else sscanf(nom,"%s %hX %hX %hX\n",nomvoie,&(c->r),&(c->g),&(c->b));
					t->var = TRMT_AU_VOL;
				}
				MonitCouleurVersPcent(c,&(t->rgb));
				if(BoloNb > 1) {
					for(bolo=0; bolo<BoloNb; bolo++) if(!strcmp(nombolo,Bolo[bolo].nom)) break;
					if(bolo == BoloNb) {
						printf("  ! ligne %d: Detecteur '%s' inconnu\n",l,nombolo);
						nb_erreurs++;
						continue;
					/* Finalement, c'est pas graaave
					} else if(Bolo[bolo].lu == DETEC_HS) {
						MonitBoloAvant = bolo; f->change = 1;
						if(!nb_erreurs) printf("* Fichier %s en erreur:\n",MonitFenFichier);
						printf("  . ligne %d: Bolo '%s' hors service\n",l,nombolo);
						nb_erreurs++; */
						/* continue; si on accepte, c'est possible de garder la fenetre */
					}
				} else bolo = 0;
				for(cap=0; cap<Bolo[bolo].nbcapt; cap++) {
					if(!strcmp(nomvoie,Bolo[bolo].captr[cap].nom)) break;
				}
				if(cap == Bolo[bolo].nbcapt) {
					printf("  ! ligne %d: Pas de voie '%s' pour le detecteur '%s'\n",l,nomvoie,nombolo);
					nb_erreurs++;
					continue;
				} else {
					voie = Bolo[bolo].captr[cap].voie; 
				}
			}
			t->bolo = bolo; t->cap = cap; t->voie = voie;
			for(cap=0; cap<Bolo[bolo].nbcapt; cap++) t->liste_voies[cap] = Bolo[bolo].captr[cap].nom;
			t->liste_voies[cap] = "\0";
			if(f->type == MONIT_FFT) f->p.f.spectre[f->nb] = 0;
			f->nb += 1;
		}
	} forever;

	if(i && f->affiche) MonitFenAffiche(&(MonitFen[i - 1]),0);
	if(nb_averts) fprintf(stderr,"  = %d avertissement%s\n",Accord1s(nb_averts));
	if(nb_erreurs) fprintf(stderr,"  ! %d erreur%s, fichier a revoir\n",Accord1s(nb_erreurs));
	if(fichier) fclose(fichier);
	if(nb_averts || nb_erreurs) OpiumFail(L_("Graphiques utilisateur a revoir (cf journal)"));
	return(0);
}
/* ========================================================================== */
int MonitFenEcrit() {
	if(SettingsSauveChoix) {
		if(OpiumExec(pMonitFenEcri->cdr) == PNL_CANCEL) return(0);
	}
	return(MonitFenEncode());
}
/* ========================================================================== */
static int MonitFenEncode() {
	int i,j;
	TypeMonitFenetre *f; TypeMonitTrace *t;

	RepertoireCreeRacine(MonitFenFichier);
#define MONIT_ECRIT_NL
#ifdef MONIT_ECRIT_NL
	//- char nom_fenetres[MAXFILENAME];
	for(i=0, f=MonitFen; i<MonitFenNb; i++,f++) {
		int bolo,cap;
		if(f->type == MONIT_2DHISTO) f->nb = 2;
		t = f->trace;
		for(j=0; j<f->nb; j++,t++) {
			bolo = t->bolo;
			if((cap = t->cap) >= Bolo[bolo].nbcapt) cap = 0;
			strcpy(t->voie_lue,Bolo[bolo].captr[cap].nom);
		}
	}
	//- SambaAjoutePrefPath(nom_fenetres,"Fenetres");
	//- ArgPrint(nom_fenetres,MonitFenDesc);
	ArgPrint(MonitFenFichier,MonitFenDesc);
#else /* MONIT_ECRIT_NL */
	FILE *fichier; int l; OpiumColor *c; Cadre cdr;
	fichier = fopen(MonitFenFichier,"w");
	if(!fichier) {
		OpiumError(L_("Fichier '%s' inutilisable: %s"),MonitFenFichier,strerror(errno));
		return(0);
	}

	for(i=0, f=MonitFen; i<MonitFenNb; i++,f++) {
		MonitFenMemo(f);
		fprintf(fichier,"+ %s /donnees=%s ",f->titre,MonitFenTypeName[f->type]);
		switch(f->type) {
		  case MONIT_SIGNAL:
		  case MONIT_EVENT:
			fprintf(fichier,"/duree=%g",f->axeX.r.max);
			break;
		  case MONIT_HISTO:
			fprintf(fichier,"/bins=%d/evts=%d",f->axeX.pts,f->axeY.i.max);
			break;
		  case MONIT_FFT:
			fprintf(fichier,"/freq-min=%g/freq-max=%g",f->axeX.r.min,f->axeX.r.max);
			fprintf(fichier,"/echantillons=%d/axeX=%s/interv=%d/evts=%s",
				f->axeX.pts,f->p.f.xlog?"log":"lin",f->p.f.intervalles,f->p.f.excl?"avec":"sans");
			break;
		}
		if(f->type == MONIT_HISTO)
			fprintf(fichier,"/limites=%g %g",f->axeX.r.min,f->axeX.r.max);
		else if(f->type == MONIT_FFT)
			fprintf(fichier,"/limites=%g %g",f->axeY.r.min,f->axeY.r.max);
		else fprintf(fichier,"/limites=%d %d",f->axeY.i.min,f->axeY.i.max);
		fprintf(fichier,"\n");
		if(f->g) {
			cdr = (f->g)->cdr;
			if(OpiumDisplayed(cdr)) {
				OpiumGetPosition(cdr,&(f->posx),&(f->posy));
				OpiumGetSize    (cdr,&(f->larg),&(f->haut));
			}
		}
		MonitFenDimMin(f);
		l = strlen(f->titre) + 3;
		while(l--) fprintf(fichier," ");
		fprintf(fichier,"/dimensions=%d %d/position=%d %d/affichage=%s/grille=%s /traces=\n",
			f->larg,f->haut,f->posx,f->posy,f->affiche? ((f->affiche == 2)? "tjrs": "oui"):"non",f->grille? "oui":"non");
		t = f->trace;
		for(j=0; j<f->nb; j++,t++) {
			c = &(t->couleur);
			switch(f->type) {
			  case MONIT_PATTERN:
			  case MONIT_MOYEN:
				fprintf(fichier,"       %-16s\t (%04X %04X %04X)\n",
					VoieManip[t->voie].nom,c->r,c->g,c->b);
				break;
			  case MONIT_SIGNAL:
			  case MONIT_EVENT:
			  case MONIT_FFT:
				fprintf(fichier,"       %-16s %-8s\t (%04X %04X %04X)\n",
					VoieManip[t->voie].nom,TrmtClassesListe[t->var],c->r,c->g,c->b);
				break;
			  case MONIT_HISTO:
				fprintf(fichier,"       %-12s %-16s\t (%04X %04X %04X)\n",
					MonitFenVarName[t->var],VoieManip[t->voie].nom,c->r,c->g,c->b);
				break;
			}
		}
	}
	fclose(fichier);
#endif /* MONIT_ECRIT_NL */

	MonitFenModifiees = 0;
	return(0);
}
/* ========================================================================== */
int MonitFenPanel(Panel panel, int num, void *arg) {
	Panel p = (Panel)panel;
//	printf("* %s(%08X, %d, %08X)\n",__func__,(hexa)p,num,(hexa)arg);
	if(p) { PanelApply(p,1); if(OpiumDisplayed(p->cdr)) OpiumClear(p->cdr); }
//	printf("  %s: fenetre de type %d (%s)\n",__func__,MonitFenLue.type,MonitFenTypeName[MonitFenLue.type]);
	PanelErase(pMonitFen);
	switch(MonitFenLue.type) {
	  case MONIT_SIGNAL   : PanelTitle(pMonitFen,L_("Fenetre de signal")); break;
	  case MONIT_HISTO    : PanelTitle(pMonitFen,L_("Fenetre d'histogrammes")); break;
	  case MONIT_FFT      : PanelTitle(pMonitFen,L_("Fenetre en frequence")); break;
	  case MONIT_FONCTION : PanelTitle(pMonitFen,L_("Fonction Ntuple(evt)")); break;
	  case MONIT_BIPLOT   : PanelTitle(pMonitFen,L_("Bi-plot Ntuple")); break;
	  case MONIT_2DHISTO  : PanelTitle(pMonitFen,L_("Histo 2D")); break;
	  case MONIT_EVENT    : PanelTitle(pMonitFen,L_("Fenetre d'evenement")); break;
	  case MONIT_MOYEN    : PanelTitle(pMonitFen,L_("Evenement moyen")); break;
	  case MONIT_PATTERN  : PanelTitle(pMonitFen,L_("Motif soustrait")); break;
	}
	PanelText (pMonitFen,"Titre",MonitFenLue.titre,32);
	PanelItemExit(pMonitFen,PanelKeyB(pMonitFen,L_("Donnees presentees"),MonitNomTypes,(char *)&(MonitFenLue.type),11),MonitFenPanel,0);
	switch(MonitFenLue.type) {
	  case MONIT_SIGNAL:
	  case MONIT_EVENT:
		PanelFloat(pMonitFen,L_("Duree affichee (ms)"),&(MonitFenLue.axeX.r.max));
	  case MONIT_PATTERN:
	  case MONIT_MOYEN:
		PanelInt  (pMonitFen,L_("Valeur minimum"),&(MonitFenLue.axeY.i.min));
		PanelInt  (pMonitFen,L_("Valeur maximum"),&(MonitFenLue.axeY.i.max));
		break;
	  case MONIT_HISTO:
		PanelFloat(pMonitFen,"Bin minimum",&(MonitFenLue.axeX.r.min));
		PanelFloat(pMonitFen,"Bin maximum",&(MonitFenLue.axeX.r.max));
		PanelInt  (pMonitFen,L_("Nombre de bins"),&(MonitFenLue.axeX.pts));
		PanelInt  (pMonitFen,L_("Nb.evts affiches"),&(MonitFenLue.axeY.i.max));
		break;
	  case MONIT_FFT:
		PanelInt  (pMonitFen,L_("Points echantillonnes"),&(MonitFenLue.axeX.pts));
		PanelFloat(pMonitFen,L_("Frequence min (0:auto)"),&(MonitFenLue.axeX.r.min));
		PanelFloat(pMonitFen,L_("Frequence max (0:auto)"),&(MonitFenLue.axeX.r.max));
		PanelFloat(pMonitFen,L_("Bruit minimum"),&(MonitFenLue.axeY.r.min));
		PanelFloat(pMonitFen,L_("Bruit maximum"),&(MonitFenLue.axeY.r.max));
		PanelKeyB (pMonitFen,L_("Axe X"),L_("lineaire/log"),&(MonitFenLue.p.f.xlog),10);
		PanelInt  (pMonitFen,L_("Intervalles sommes"),&(MonitFenLue.p.f.intervalles));
		PanelKeyB (pMonitFen,L_("Exclusion evts"),L_("non/active"),&(MonitFenLue.p.f.excl),8);
		break;
	  case MONIT_FONCTION:
		PanelFloat(pMonitFen,L_("Valeur minimum"),&(MonitFenLue.axeY.r.min));
		PanelFloat(pMonitFen,L_("Valeur maximum"),&(MonitFenLue.axeY.r.max));
		PanelInt  (pMonitFen,L_("Nb.evts affiches"),&(MonitFenLue.axeX.i.max));
		break;
	  case MONIT_BIPLOT:
		PanelFloat(pMonitFen,"X min",&(MonitFenLue.axeX.r.min));
		PanelFloat(pMonitFen,"X max",&(MonitFenLue.axeX.r.max));
		PanelFloat(pMonitFen,"Y min",&(MonitFenLue.axeY.r.min));
		PanelFloat(pMonitFen,"Y max",&(MonitFenLue.axeY.r.max));
		break;
	  case MONIT_2DHISTO:
		PanelFloat(pMonitFen,"Bin X minimum",&(MonitFenLue.axeX.r.min));
		PanelFloat(pMonitFen,"Bin X maximum",&(MonitFenLue.axeX.r.max));
		PanelInt  (pMonitFen,L_("Nombre de bins en X"),&(MonitFenLue.axeX.pts));
		PanelFloat(pMonitFen,"Bin Y minimum",&(MonitFenLue.axeY.r.min));
		PanelFloat(pMonitFen,"Bin Y maximum",&(MonitFenLue.axeY.r.max));
		PanelInt  (pMonitFen,L_("Nombre de bins en Y"),&(MonitFenLue.axeY.pts));
		break;
	}
	PanelKeyB (pMonitFen,L_("Affichage"),L_("non/oui/tjrs"),&(MonitFenLue.affiche),6);
	PanelKeyB (pMonitFen,L_("Avec grille"),L_("non/oui"),&(MonitFenLue.grille),6);
	PanelBool (pMonitFen,L_("Editer les traces"),&MonitEditTraces);
	if(p) { OpiumUse(p->cdr,OPIUM_EXEC); OpiumRefresh(p->cdr); /* PanelRefreshVars(p); */ }
	return(0);
}
/* ========================================================================== */
int MonitFenEdite(Menu menu, MenuItem *item) {
	int i,j,l,m,n,x,y,cols,nbitems; int bolo;
	int posx,posy;
	char existe_deja,manque_infos,replace; int trmt_demande,var_demandee;
	char action[MAXTRACES],titre[256],*nom;
	TypeMonitFenetre *f; TypeMonitTrace *t; // OpiumColor *c;
	char *item_appelant;

	item_appelant = MenuItemGetText(menu,MenuItemNum(menu,item));
	if(!strncmp(item_appelant,"Creer",5)) MonitFenNum = MonitFenNb;
	else if(MonitFenNb > 1) {
		if(OpiumReadList(L_("Quelle fenetre"),MonitFenTitre,&MonitFenNum,32) == PNL_CANCEL) return(0);
	} else MonitFenNum = 0;

	f = &(MonitFen[MonitFenNum]);
	existe_deja = (MonitFenNum < MonitFenNb);
	if(!strncmp(item_appelant,"Organiser",9)) {
		MonitFenMemo(f);
		MonitLarg = f->larg; MonitHaut = f->haut;
		posx = f->posx + f->larg + WND_ASCINT_WIDZ + 6;
		posy = f->posy;
		for(i=0, f=MonitFen; i<MonitFenNb; i++,f++) if(i != MonitFenNum) {
			if(!f->affiche) continue;
			f->larg = MonitLarg; f->haut = MonitHaut;
			f->posx = posx; f->posy = posy;
			posx += (f->larg + WND_ASCINT_WIDZ + 6);
			if(posx > (OpiumServerWidth(0)  - f->larg)) {
				posx = MonitFen[MonitFenNum].posx;
				posy += (MonitHaut + WND_ASCINT_WIDZ + 30);
				if(posy > OpiumServerHeigth(0)) break;
			}
/*			Graph g;
			if((g = f->g)) {
				if(replace = OpiumDisplayed(g->cdr)) OpiumClear(g->cdr);
				GraphResize(g,f->larg,f->haut);
				OpiumSizeGraph(g->cdr,0);
				OpiumPosition(g->cdr,f->posx,f->posy);
				if(replace) OpiumDisplay(g->cdr);
			} */
			MonitFenAffiche(f,1);
		}
		return(0);
	} else if(!strncmp(item_appelant,"Dupliquer",9)) {
		nom = f->titre;
		MonitFenMemo(f);
		memcpy(&MonitFenLue,f,sizeof(TypeMonitFenetre));
		strcpy(MonitFenLue.titre,MonitNouvelle);
		MonitFenLue.posx = MonitFenLue.posy = OPIUM_AUTOPOS;
		MonitFenLue.g = 0;
		MonitFenLue.p.h.histo = 0;
		if(MonitFenLue.type == MONIT_HISTO) MonitFenLue.p.h.histo = 0;
		else if(MonitFenLue.type == MONIT_2DHISTO) { MonitFenLue.p.s.histo = 0; MonitFenLue.nb = 2; }
		else if(MonitFenLue.type == MONIT_FFT) {
			for(i=0; i<MonitFenLue.nb; i++) MonitFenLue.p.f.spectre[i] = 0;
		}
		if(MonitFenLue.nb) {
			MonitBoloAvant = MonitFenLue.trace[0].bolo;
			BoloNum = MonitBoloAvant;
			if(OpiumExec(pMonitFenDetecteur->cdr) != PNL_CANCEL) {
				for(i=0; i<MonitFenLue.nb; i++) {
					if(MonitFenLue.trace[i].bolo == MonitBoloAvant) MonitFenLue.trace[i].bolo = BoloNum;
				}
				l = (int)strlen(nom);
				n = (int)strlen(Bolo[MonitBoloAvant].nom);
				for(i=0; i<=(l - n); i++) {
					if(!strncmp(nom+i,Bolo[MonitBoloAvant].nom,n)) break;
				}
				if(i <= (l - n)) {
					strncpy(MonitFenLue.titre,nom,i);
					strncpy(MonitFenLue.titre+i,Bolo[BoloNum].nom,80-i);
					if((i + n) < l) {
						m = (int)strlen(Bolo[BoloNum].nom);
						strncpy(MonitFenLue.titre+i+m,nom+i+n,80-i-m);
					}
				}
			}
		}
		MonitFenNum = MonitFenNb;
		f = &(MonitFen[MonitFenNum]);
		existe_deja = 0;
	} else if(existe_deja) {
		MonitFenMemo(f);
		MonitFenFree(&(MonitFen[MonitFenNum])); /* avant memcpy de facon a ne pas garder non-nulles les adresses liberees ici */
		memcpy(&MonitFenLue,f,sizeof(TypeMonitFenetre));
	} else MonitFenDefault(&MonitFenLue,MonitNouvelle);
	MonitFenEnCours = &MonitFenLue;
	MonitFenPanel(0,0,0);
	PanelTitle(pMonitFen,MonitFenLue.titre);
	do {
		if(OpiumExec(pMonitFen->cdr) == PNL_CANCEL) return(0);
		if(!strcmp(MonitFenLue.titre,MonitNouvelle)) OpiumError(L_("Titre interdit"));
		else {
			for(i=0; i<MonitFenNb; i++) {
				if((i != MonitFenNum) && !strcmp(MonitFenLue.titre,MonitFen[i].titre)) break;
			}
			if(i == MonitFenNb) break;
			OpiumError(L_("Titre deja utilise par la fenetre #%d"),i+1);
		}
	} forever;
	if(!existe_deja) {
		if(MonitFenNb < MAXMONIT) {
			strcpy(MonitFen[MonitFenNb].titre,MonitNouvelle);
			MonitFenNb++;
			MonitFen[MonitFenNb].titre[0] = '\0';
		} else {
			OpiumError(L_("Il y a deja %d fenetre%s definie%s, maximum atteint"),Accord2s(MonitFenNb));
			return(0);
		}
	}

	if(MonitFenLue.type == MONIT_FFT) { if(MonitFenLue.p.f.intervalles <= 0) MonitFenLue.p.f.intervalles = 1; }

	if(MonitEditTraces) do {
//		float rouge[MAXTRACES],vert[MAXTRACES],bleu[MAXTRACES];
		if(MonitFenLue.nb == 0) {
			if((MonitFenLue.type == MONIT_2DHISTO) || (MonitFenLue.type == MONIT_BIPLOT)) {
				t = &(MonitFenLue.trace[0]); t->bolo = 0; t->cap = 0; t->voie = 0; t->var = 0; MonitFenListeCapteurs(0,0,(void *)0);
				t = &(MonitFenLue.trace[1]); t->bolo = 0; t->cap = 0; t->voie = 0; t->var = 1; MonitFenListeCapteurs(0,0,(void *)1);
				MonitFenLue.nb = 2;
			} else {
				t = &(MonitFenLue.trace[0]); t->bolo = 0; t->cap = 0; t->voie = 0; t->var = 0; MonitFenListeCapteurs(0,0,(void *)0);
				MonitCouleurDefaut(&(t->couleur)); MonitCouleurVersPcent(&(t->couleur),&(t->rgb));
				MonitFenLue.nb = 1;
			}
		}
//		trmt_demande = ((MonitFenLue.type == MONIT_SIGNAL) || (MonitFenLue.type == MONIT_EVENT) || (MonitFenLue.type == MONIT_FFT))? 1: 0;
//		var_demandee = ((MonitFenLue.type == MONIT_HISTO)  || (MonitFenLue.type == MONIT_2DHISTO)
//					 || (MonitFenLue.type == MONIT_BIPLOT) || (MonitFenLue.type == MONIT_FONCTION))? 1: 0;
		trmt_demande = MonitFenTypeAvecTrmt[MonitFenLue.type];
		var_demandee = MonitFenTypeAvecVar[MonitFenLue.type];
		cols = 6 + trmt_demande + var_demandee;
		if(MonitFenLue.type == MONIT_2DHISTO) cols -= 3; // pas de couleur demandee
		nbitems = cols * (MonitFenLue.nb + 1);
		if(pMonitTraces && (PanelItemMax(pMonitTraces) != nbitems)) {
			OpiumGetPosition(pMonitTraces->cdr,&x,&y); replace = 1;
			PanelDelete(pMonitTraces);
			pMonitTraces = 0;
		} else replace = 0;
		if(pMonitTraces == 0) pMonitTraces = PanelCreate(nbitems);
		PanelErase(pMonitTraces);
		PanelColumns(pMonitTraces,cols);
		PanelRemark(pMonitTraces,"action");
		for(i=0; i<MonitFenLue.nb; i++) {
			action[i] = MONIT_GARDE;
			PanelItemColors(pMonitTraces,PanelKeyB(pMonitTraces,0,L_(MONIT_ACTION_CLES),&(action[i]),8),SambaRougeVertJaune,3);
		}
		if(var_demandee) {
			PanelRemark(pMonitTraces,"variable");
			for(i=0; i<MonitFenLue.nb; i++)
				PanelList(pMonitTraces,0,MonitFenVarName,&(MonitFenLue.trace[i].var),12);
		}
		PanelRemark(pMonitTraces,L_("detecteur"));
		for(i=0; i<MonitFenLue.nb; i++) {
			int k;
			k = PanelList(pMonitTraces,0,BoloNom,&(MonitFenLue.trace[i].bolo),8 /* DETEC_NOM */ );
			PanelItemExit(pMonitTraces,k,MonitFenListeCapteurs,(void *)(long)i);
			// PanelItemExit(pMonitTraces,k,DetecteurListeCapteurs,0);
		}
		PanelRemark(pMonitTraces,L_("capteur"));
		for(i=0; i<MonitFenLue.nb; i++) 
//			PanelListB(pMonitTraces,0,ModeleVoieListe,&(MonitFenLue.trace[i].cap),8 /* MODL_NOM */ );
			PanelList(pMonitTraces,0,MonitFenLue.trace[i].liste_voies,&(MonitFenLue.trace[i].cap),8 /* MODL_NOM */ );
		if(trmt_demande) {
			PanelRemark(pMonitTraces,L_("type"));
			for(i=0; i<MonitFenLue.nb; i++)
				PanelList(pMonitTraces,0,TrmtClassesListe,&(MonitFenLue.trace[i].var),12);
		}
		if(MonitFenLue.type != MONIT_2DHISTO) {
			for(i=0; i<MonitFenLue.nb; i++) MonitCouleurVersPcent(&(MonitFenLue.trace[i].couleur),&(MonitFenLue.trace[i].rgb));
			PanelRemark(pMonitTraces,L_("rouge"));
			for(i=0; i<MonitFenLue.nb; i++) {
				// PanelSHex(pMonitTraces,0,&(MonitFenLue.trace[i].couleur.r));
				PanelFormat(pMonitTraces,PanelFloat(pMonitTraces,0,&(MonitFenLue.trace[i].rgb.r)),"%4.0f");
			}
			PanelRemark(pMonitTraces,L_("vert"));
			for(i=0; i<MonitFenLue.nb; i++) {
				// PanelSHex(pMonitTraces,0,&(MonitFenLue.trace[i].couleur.g));
				PanelFormat(pMonitTraces,PanelFloat(pMonitTraces,0,&(MonitFenLue.trace[i].rgb.g)),"%4.0f");
			}
			PanelRemark(pMonitTraces,L_("bleu"));
			for(i=0; i<MonitFenLue.nb; i++) {
				// PanelSHex(pMonitTraces,0,&(MonitFenLue.trace[i].couleur.b));
				PanelFormat(pMonitTraces,PanelFloat(pMonitTraces,0,&(MonitFenLue.trace[i].rgb.b)),"%4.0f");
			}
		}
		sprintf(titre,L_("Traces pour la fenetre '%s'"),MonitFenLue.titre);
		PanelTitle(pMonitTraces,titre);
		if(replace) OpiumPosition(pMonitTraces->cdr,x,y);

		if(OpiumExec(pMonitTraces->cdr) == PNL_CANCEL) {
			if(!existe_deja) MonitFen[--MonitFenNb].titre[0] = '\0';
			return(0);
		}
		manque_infos = 0;
		for(i=0; i<MonitFenLue.nb; i++) {
			if(MonitFenLue.type != MONIT_2DHISTO) MonitPcentVersCouleur(&(MonitFenLue.trace[i].rgb),&(MonitFenLue.trace[i].couleur));
			switch(action[i]) {
			  case MONIT_INSERE:  /* inserer trace */
				if(MonitFenLue.nb >= MAXTRACES) {
					OpiumError(L_("Pas plus de %d traces par fenetre"),MAXTRACES);
					i = MonitFenLue.nb;
					break;
				}
				for(j=MonitFenLue.nb; j>i; --j) {
					memcpy(&(MonitFenLue.trace[j]),&(MonitFenLue.trace[j-1]),sizeof(TypeMonitTrace));
					if(j > (i + 1)) action[j] = action[j-1]; else action[j] = 0;
				}
				i++;
				MonitFenLue.nb += 1;
				manque_infos = 1;
				break;
			  case MONIT_EFFACE:  /* retirer trace */
				MonitFenLue.nb -= 1;
				for(j=i; j<MonitFenLue.nb; j++) {
					memcpy(&(MonitFenLue.trace[j]),&(MonitFenLue.trace[j+1]),sizeof(TypeMonitTrace));
					action[j] = action[j+1];
				}
				i -= 1;
				break;
			}
		}
		for(i=0; i<MonitFenLue.nb; i++) {
			bolo = MonitFenLue.trace[i].bolo;
			MonitFenLue.trace[i].voie = Bolo[bolo].captr[MonitFenLue.trace[i].cap].voie;
		}
	} while(manque_infos);

	memcpy(f,&MonitFenLue,sizeof(TypeMonitFenetre));
	MonitFenModifiees = 1;

	if(f->affiche) MonitFenAffiche(f,1);
	else MonitFenBuild(f);

	return(0);
}
/* ========================================================================== */
float MonitUnitesADU(Graph g, int sens, float val) {
	return(sens? val / ValeurADU: val * ValeurADU);
}
#ifdef UTILISE_SAMBAPLOT_AVEC_NANOPAW
/* ========================================================================== */
Graph MonitFenCreePlot(TypeMonitFenetre *f) {
	int p;

	switch(f->type) {
		case MONIT_SIGNAL: case MONIT_EVENT:
			p = PlotNouveau(PLOT_FONCTION);
			strncpy(Plot[p].titre,f->titre,80);
			Plot[p].g = f->g;
			break;
		case MONIT_HISTO:
			p = PlotNouveau(PLOT_HISTO);
			strncpy(Plot[p].titre,f->titre,80);
			Plot[p].g = f->g;
			Plot[p].h = f->p.h.histo;
			Plot[p].hd = f->p.h.hd[0];
			break;
		default: return(0);
	}
}
#endif
/* ========================================================================== */
Graph MonitFenCreeGraphe(TypeMonitFenetre *f, char nouveau) {
	int i,n,x,y,nbfreq; float horloge;
	unsigned int zoom;
	OpiumColor *c; Graph g;
	// HistoData hd; 
	HistoDeVoie vh,prec;
	int j,kx; int voie; float duree,total; TypeMonitTrace *t;

	if(f->nb <= 0) return(0);
	if(MonitFenTypeAvecTrmt[f->type]) {
		n = 0;
		for(i=0; i<f->nb; i++) {
			voie = f->trace[i].voie; if(!VoieTampon[voie].lue) continue;
			switch(f->trace[i].var) {
			  case TRMT_AU_VOL: if(VoieTampon[voie].brutes->t.sval)   n++; break;
			  case TRMT_PRETRG: if(VoieTampon[voie].traitees->t.rval) n++; break;
			}
		}
		if(n <= 0) return(0);
	}

	if(nouveau) { MonitFenDimMin(f); f->g = GraphCreate(f->larg,f->haut,2 * f->nb); }
	else GraphErase(f->g);
	g = f->g; OpiumTitle(g->cdr,f->titre);
	switch(f->type) {
	  case MONIT_HISTO:
		f->p.h.histo = HistoCreateFloat(f->axeX.r.min,f->axeX.r.max,f->axeX.pts);
		break;
	  case MONIT_2DHISTO:
		f->p.s.histo = H2DCreateFloatFloatToInt(f->axeX.r.min,f->axeX.r.max,f->axeX.pts,f->axeY.r.min,f->axeY.r.max,f->axeY.pts);
		break;
	  case MONIT_FONCTION:
		x = GraphAdd(g,GRF_SHORT|GRF_INDEX|GRF_XAXIS,MonitEvtIndex,f->axeX.i.max);
		GraphDataName(g,x,"Evt#");
		break;
	  default: break;
	}

	for(j=0, t=f->trace; j<f->nb; j++,t++) {
		t->ajuste = 0;
		voie = t->voie; t->numx = -1;
		if(!VoieTampon[voie].lue) continue;
		c = &(t->couleur);
		x = -1; y = -1;
		switch(f->type) {
		  case MONIT_SIGNAL: /* abcisse en millisecondes */
			switch(t->var) {
			  case TRMT_AU_VOL:
				if(!VoieTampon[voie].brutes->t.sval) break;
				x = GraphAdd(g,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,VoieTampon[voie].trmt[TRMT_AU_VOL].ab6,VoieTampon[voie].brutes->max);
				y = GraphAdd(g,GRF_SHORT|GRF_YAXIS,VoieTampon[voie].brutes->t.sval,VoieTampon[voie].brutes->max);
				GraphDataName(g,x,"t(ms)");
				GraphDataName(g,y,VoieManip[voie].nom);
				break;
			  case TRMT_PRETRG:
				if(!VoieTampon[voie].traitees->t.rval) break;
				x = GraphAdd(g,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,VoieTampon[voie].trmt[TRMT_PRETRG].ab6,VoieTampon[voie].traitees->max);
				y = GraphAdd(g,GRF_FLOAT|GRF_YAXIS,VoieTampon[voie].traitees->t.rval,VoieTampon[voie].traitees->max);
				GraphDataName(g,x,"t(ms)");
				GraphDataName(g,y,VoieManip[voie].nom);
				break;
			}
			t->numx = x;
			if(y >= 0) GraphDataRGB(g,y,c->r,c->g,c->b);
			break;
		  case MONIT_FONCTION:
		  case MONIT_HISTO:
			f->p.h.hd[j] = HistoAdd(f->p.h.histo,HST_INT);
			if(f->type == MONIT_HISTO) {
				HistoDataName(f->p.h.hd[j],VoieManip[voie].nom);
				HistoDataSupportRGB(f->p.h.hd[j],WND_Q_ECRAN,c->r,c->g,c->b);
				HistoDataSupportRGB(f->p.h.hd[j],WND_Q_PAPIER,c->r,c->g,c->b);
			}
			vh = VoieHisto[voie]; prec = 0;
			while(vh) { prec = vh; vh = prec->suivant; }
			vh = (HistoDeVoie)malloc(sizeof(TypeHistoDeVoie));
			if(!vh) break;
			if(prec == 0) VoieHisto[voie] = vh;
			else prec->suivant = vh;
			vh->D = 1;
			vh->hd = f->p.h.hd[j];
			vh->Xvar = t->var;
			vh->suivant = 0;
			if(f->type == MONIT_FONCTION) {
//				y = GraphAdd(g,GRF_INT|GRF_YAXIS,(int *)((vh->hd)->adrs),f->axeX.i.max);
//				GraphDataName(g,y,MonitFenVarName[t->var]);
			}
			break;
		  case MONIT_2DHISTO:
			H2DLUT(f->p.s.histo,MonitLUTall,DIM_LUT);
			vh = VoieHisto[voie]; prec = 0;
			while(vh) { prec = vh; vh = prec->suivant; }
			vh = (HistoDeVoie)malloc(sizeof(TypeHistoDeVoie));
			if(!vh) break;
			if(prec == 0) VoieHisto[voie] = vh;
			else prec->suivant = vh;
			vh->D = 2;
			vh->hd = (HistoData)f->p.s.histo;
			vh->Xvar = t->var; t++; j++;
			vh->Yvar = t->var;
			vh->suivant = 0;
			break;
		  case MONIT_FFT:
			horloge = VoieTampon[voie].trmt[t->var].ab6[1];
			if(t->var == TRMT_AU_VOL) { if(!VoieTampon[voie].brutes->t.sval) break; }
			else if(t->var == TRMT_PRETRG) { if(!VoieTampon[voie].traitees->t.rval) break; }
			else break;
			nbfreq = (f->axeX.pts / 2) + 1;
			n = nbfreq * sizeof(float);
			f->p.f.spectre[j] = (float *)malloc(n);
			if(!f->p.f.spectre[j]) {
				OpiumError("Manque de place memoire pour le spectre (%d octets)",n);
				return(0);
			}
			f->p.f.freq[j][0] = 0.0; f->p.f.freq[j][1] = 1.0 / (horloge * f->axeX.pts); /* kHz */
			x = GraphAdd(g,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,f->p.f.freq[j],nbfreq);
			y = GraphAdd(g,GRF_FLOAT|GRF_YAXIS,f->p.f.spectre[j],nbfreq);
//			printf("Ajoute le tableau %08X[%d] a la trace F%d[%d]\n",(hexa)(f->p.f.spectre[j]),nbfreq,num,j);
			GraphDataName(g,x,"f(kHz)");
			GraphDataName(g,y,VoieManip[voie].nom);
			if(y >= 0) GraphDataRGB(g,y,c->r,c->g,c->b);
			GraphAxisTitle(g,GRF_YAXIS,L_("Bruit"));
			GraphAxisTitle(g,GRF_XAXIS,L_("Frequence (kHz)"));
			t->numx = x;
			break;
		  case MONIT_EVENT:
			switch(t->var) {
			  case TRMT_AU_VOL:
				if(!VoieTampon[voie].brutes->t.sval) break;
				x = GraphAdd(g,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,&(MonitEvtAb6[voie][0]),VoieTampon[voie].brutes->max);
				// x = GraphAdd(g,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,VoieTampon[voie].trmt[TRMT_AU_VOL].ab6,PointsEvt);
				y = GraphAdd(g,GRF_SHORT|GRF_YAXIS,VoieTampon[voie].brutes->t.sval,PointsEvt);
				GraphDataName(g,x,"t(ms)");
				GraphDataName(g,y,VoieManip[voie].nom);
				break;
			  case TRMT_PRETRG:
				if(!VoieTampon[voie].traitees->t.rval) break;
				x = GraphAdd(g,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,&(MonitTrmtAb6[voie][0]),VoieTampon[voie].traitees->max);
				// x = GraphAdd(g,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,VoieTampon[voie].trmt[TRMT_PRETRG].ab6,PointsEvt);
				y = GraphAdd(g,GRF_FLOAT|GRF_YAXIS,VoieTampon[voie].traitees->t.rval,PointsEvt);
				GraphDataName(g,x,"t(ms)");
				GraphDataName(g,y,VoieManip[voie].nom);
				break;
			}
			t->numx = x;
			if(y >= 0) GraphDataRGB(g,y,c->r,c->g,c->b);
			break;
		  case MONIT_MOYEN:
			x = GraphAdd(g,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,&(MonitEvtAb6[voie][0]),VoieTampon[voie].somme.taille);
			y = GraphAdd(g,GRF_FLOAT|GRF_YAXIS,VoieTampon[voie].somme.val,VoieTampon[voie].somme.taille);
			GraphDataName(g,x,"t(ms)");
			GraphDataName(g,y,VoieManip[voie].nom);
			if(y >= 0) GraphDataRGB(g,y,c->r,c->g,c->b);
			t->numx = x;
			break;
		  case MONIT_PATTERN:
			x = GraphAdd(g,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,&(MonitEvtAb6[voie][0]),VoieTampon[voie].pattern.dim);
			y = GraphAdd(g,GRF_FLOAT|GRF_YAXIS,VoieTampon[voie].pattern.val,VoieTampon[voie].pattern.dim);
			GraphDataName(g,x,"t(ms)");
			GraphDataName(g,y,VoieManip[voie].nom);
			if(y >= 0) GraphDataRGB(g,y,c->r,c->g,c->b);
			t->numx = x;
			break;
		}
	}
	switch(f->type) {
	  case MONIT_SIGNAL:
		total = 0.0; kx = -1;
		for(j=0, t=f->trace; j<f->nb; j++,t++) {
			voie = t->voie; if(!VoieTampon[voie].lue) continue;
			switch(t->var) {
			  case TRMT_AU_VOL:
				if(!VoieTampon[voie].brutes->t.sval) break;
				if(DureeTampons > total) { total = DureeTampons; kx = t->numx; }
				break;
			  case TRMT_PRETRG:
				if(!VoieTampon[voie].traitees->t.rval) break;
				duree = (float)VoieTampon[voie].traitees->max * VoieTampon[voie].trmt[TRMT_PRETRG].ab6[1];
				if(duree > total) { total = duree; kx = t->numx; }
				break;
			}
		}
		if(f->axeX.r.max > total) f->axeX.r.max = total;
		zoom = total / f->axeX.r.max;
		// printf("Zoom pour %s: %g / %g = %d\n",f->titre,total,f->axeX.r.max,zoom);
		GraphZoom(g,zoom,1);
		if(kx >= 0) GraphAxisDefine(g,2 * kx);
		if(f->axeY.i.min == f->axeY.i.max) GraphAxisAutoRange(g,GRF_YAXIS);
		else GraphAxisIntRange(g,GRF_YAXIS,f->axeY.i.min,f->axeY.i.max);
		GraphAxisAutoGrad(g,GRF_YAXIS);
		if(MonitChgtUnite) GraphAxisChgtUnite(g,GRF_YAXIS,MonitUnitesADU);
		GraphUse(g,0);
		break;
	  case MONIT_EVENT:
		if(f->axeY.i.min == f->axeY.i.max) GraphAxisAutoRange(g,GRF_YAXIS);
		else GraphAxisIntRange(g,GRF_YAXIS,f->axeY.i.min,f->axeY.i.max);
		GraphAxisReset(g,GRF_YAXIS);
		GraphUse(g,0);
	 	if(MonitChgtUnite) GraphAxisChgtUnite(g,GRF_YAXIS,MonitUnitesADU);
		break;
	  case MONIT_HISTO:
		HistoGraphAdd(f->p.h.histo,g);
		GraphAxisDefine(g,0); /* defini deja l'axe des X pour GraphGetFloatRange */
		if(f->axeX.r.min == f->axeX.r.max) GraphAxisAutoRange(g,GRF_XAXIS);
		else GraphAxisFloatRange(g,GRF_XAXIS,f->axeX.r.min,f->axeX.r.max);
	 	if(MonitChgtUnite) GraphAxisChgtUnite(g,GRF_XAXIS,MonitUnitesADU);
		if(f->axeY.i.max) GraphAxisIntRange(g,GRF_YAXIS,0,f->axeY.i.max);
		else GraphAxisAutoRange(g,GRF_YAXIS);
		GraphUse(g,f->axeX.pts);
		break;
	  case MONIT_2DHISTO:
		H2DGraphAdd(f->p.s.histo,g);
		GraphAxisDefine(g,0); /* defini deja l'axe des X pour GraphGetFloatRange */
		GraphAxisDefine(g,1); /* defini deja l'axe des Y pour GraphGetFloatRange */
		if(f->axeX.r.min == f->axeX.r.max) GraphAxisAutoRange(g,GRF_XAXIS);
		else GraphAxisFloatRange(g,GRF_XAXIS,f->axeX.r.min,f->axeX.r.max);
		if(f->axeY.r.min == f->axeY.r.max) GraphAxisAutoRange(g,GRF_YAXIS);
		else GraphAxisFloatRange(g,GRF_YAXIS,f->axeY.r.min,f->axeY.r.max);
		GraphUse(g,f->axeX.pts);
		break;
	  case MONIT_FFT:
		GraphAxisDefine(g,0); /* defini deja l'axe des X pour GraphGetFloatRange */
		GraphAxisDefine(g,1); /* defini deja l'axe des Y pour GraphGetFloatRange */
		if(f->axeX.r.min < 0.0) f->axeX.r.min = 1.0;
		if(f->axeX.r.min > f->axeX.r.max) f->axeX.r.max = f->axeX.r.min * 10.0;
		if(f->axeX.r.min < f->axeX.r.max) GraphAxisFloatRange(g,GRF_XAXIS,f->axeX.r.min,f->axeX.r.max);
		else GraphAxisAutoRange(g,GRF_XAXIS);
		if(f->axeY.r.min == f->axeY.r.max) GraphAxisAutoRange(g,GRF_YAXIS);
		else {
			if(f->axeY.r.min <= 0.0) f->axeY.r.min = 0.1;
			if(f->axeY.r.max <= f->axeY.r.min) f->axeY.r.max = f->axeY.r.min * 10.0;
			GraphAxisFloatRange(g,GRF_YAXIS,f->axeY.r.min,f->axeY.r.max);
		}
		GraphAxisLog(g,GRF_XAXIS,f->p.f.xlog);
		GraphAxisLog(g,GRF_YAXIS,1);
		break;
	  case MONIT_PATTERN:
	  case MONIT_MOYEN:
		if(f->axeY.i.min == f->axeY.i.max) GraphAxisAutoRange(g,GRF_YAXIS);
		else GraphAxisIntRange(g,GRF_YAXIS,f->axeY.i.min,f->axeY.i.max);
		GraphAxisReset(g,GRF_YAXIS);
		GraphUse(g,0);
		if(MonitChgtUnite) GraphAxisChgtUnite(g,GRF_YAXIS,MonitUnitesADU);
		break;
	}

	GraphMode(g,GRF_2AXES | GRF_LEGEND | (f->grille? GRF_GRID: 0));
	GraphParmsSave(g);

	return(g);
}
/* ========================================================================== */
int MonitFenBuild(TypeMonitFenetre *f) {
	Graph g;
	
	MonitFenClear(f);
	MonitFenFree(f);
	g = MonitFenCreeGraphe(f,1);
	if(f->posx == OPIUM_AUTOPOS) f->posx = 20;
	if(f->posy == OPIUM_AUTOPOS) f->posy = 100;
	if(g) OpiumPosition(g->cdr,f->posx,f->posy);
	return(0);
}
/* ========================================================================== */
void MonitFenClear(TypeMonitFenetre *f) {
	Graph g;
	
	if((g = f->g)) {
		if(OpiumDisplayed(g->cdr)) OpiumClear(g->cdr);
		GraphDelete(g);
		f->g = 0;
	}
}
/* ========================================================================== */
void MonitFenFree(TypeMonitFenetre *f) {
	TypeMonitTrace *t;
	HistoData hd; HistoDeVoie vh,prec,svt;
	int j; int voie;

	if(f->type == MONIT_HISTO) {
//		printf("(MonitFenFree) Liberation de la fenetre #%d (%s), histo @%08X\n",num,f->titre,(hexa)(f->p.h.histo));
		if(f->p.h.histo) {
			for(j=0, t=f->trace; j<f->nb; j++,t++) {
				voie = t->voie;
				hd = f->p.h.hd[j]; f->p.h.hd[j] = 0;
				vh = VoieHisto[voie]; prec = 0;
				while(vh) {
					svt = vh->suivant;
					if(vh->hd == hd) {
						if(prec == 0) VoieHisto[voie] = svt;
						else prec->suivant = svt;
						free(vh);
						break; /* une HistoData ne peut etre partagee, donc elle est referencee 1 seule fois */
					} else prec = vh;
					vh = svt;
				}
			}
			HistoErase(f->p.h.histo); HistoDelete(f->p.h.histo); f->p.h.histo = 0;
//			printf("(MonitFenFree) Fenetre #%d (%s) liberee, histo @%08X\n",num,f->titre,(hexa)(f->p.h.histo));
		}
	} else if(f->type == MONIT_2DHISTO) {
			if(f->p.s.histo) {
				hd = (HistoData)f->p.s.histo;
				for(j=0, t=f->trace; j<f->nb; j++,t++) {
					voie = t->voie;
					vh = VoieHisto[voie]; prec = 0;
					while(vh) {
						svt = vh->suivant;
						if(vh->hd == hd) {
							if(prec == 0) VoieHisto[voie] = svt;
							else prec->suivant = svt;
							free(vh);
							break; /* une HistoData ne peut etre partagee, donc elle est referencee 1 seule fois */
						} else prec = vh;
						vh = svt;
					}
				}
				H2DDelete(f->p.s.histo); f->p.s.histo = 0;
			}
	} else if(f->type == MONIT_FFT) {
		for(j=0; j<f->nb; j++) if(f->p.f.spectre[j]) {
			free(f->p.f.spectre[j]);
			f->p.f.spectre[j] = 0;
		}
	}
}
/* ========================================================================== 
void MonitFenNettoie(int voie, char type_tampon, char type_fen) {
	int i,j;
	TypeMonitFenetre *f; TypeMonitTrace *t; Graph g;

	for(i=0, f=MonitFen; i<MonitFenNb; i++,f++) if(f->type == type_fen) {
		for(j=0, t=f->trace; j<f->nb; j++,t++) {
			if((t->voie == voie) && (t->var == type_tampon)) break;
		}
		if(j < f->nb) {
			MonitFenClear(f); MonitFenFree(f);
		}
	}
}
   ========================================================================== */
int MonitFenRetire() {
	int i; Graph g;

	if(MonitFenNb) {
		if(OpiumReadList(L_("Quelle fenetre"),MonitFenTitre,&MonitFenNum,32) == PNL_CANCEL)
			return(0);
	} else MonitFenNum = 0;

	if(MonitFenNum >= MonitFenNb) {
		OpiumError(L_("Choix pas tres logique..."));
		return(0);
	}
	if((g = MonitFen[MonitFenNum].g)) {
		OpiumGetPosition(g->cdr,&(MonitFen[MonitFenNum].posx),&(MonitFen[MonitFenNum].posy));
		OpiumGetSize    (g->cdr,&(MonitFen[MonitFenNum].larg),&(MonitFen[MonitFenNum].haut));
		MonitFenClear(&(MonitFen[MonitFenNum]));
	}
	MonitFenFree(&(MonitFen[MonitFenNum]));
	memcpy(&MonitFenLue,&(MonitFen[MonitFenNum]),sizeof(TypeMonitFenetre));
	MonitFenNb--;
	for(i=MonitFenNum; i<MonitFenNb; i++)
		memcpy(MonitFen + i,MonitFen + i + 1,sizeof(TypeMonitFenetre));
	strcpy(MonitFen[MonitFenNb].titre,MonitNouvelle);
	MonitFen[MonitFenNb + 1].titre[0] = '\0';
	MonitFenModifiees = 1;

	return(0);
}
/* ========================================================================== */
int MonitFenListe() {
	int i,j,l,hlim;
	TypeMonitFenetre *f; TypeMonitTrace *t; TypeMonitRgb *nuance;

	if(!OpiumDisplayed(tMonit->cdr)) OpiumDisplay(tMonit->cdr); /* pour bien afficher des la premiere fois */
	TermEmpty(tMonit);
	TermHold(tMonit);
	TermPrint(tMonit,"------------------------------------------------------------\n");
	l = 2;
	for(i=0, f=MonitFen; i<MonitFenNb; i++,f++) l += (2 + f->nb);
	hlim = WndPixToLig(OpiumServerHeigth(0)) - 5;
	if(l > hlim) l = hlim;
	TermLines(tMonit,l);
	for(i=0, f=MonitFen; i<MonitFenNb; i++,f++) {
		MonitFenMemo(f);
		TermPrint(tMonit,"%2d/ %-32s ",i+1,f->titre);
		if(f->type == MONIT_HISTO)
			TermPrint(tMonit,"[%-5g .. %5g] ",f->axeX.r.min,f->axeX.r.max);
		else if((f->type == MONIT_BIPLOT) || (f->type == MONIT_2DHISTO))
			TermPrint(tMonit,"[%-5g .. %5g] x [%-5g .. %5g] ",f->axeX.r.min,f->axeX.r.max,f->axeY.r.min,f->axeY.r.max);
		else if((f->type == MONIT_FFT) || (f->type == MONIT_FONCTION))
			TermPrint(tMonit,"[%-5g .. %5g] ",f->axeY.r.min,f->axeY.r.max);
		else TermPrint(tMonit,"[%-5d .. %5d] ",f->axeY.i.min,f->axeY.i.max);
		switch(f->type) {
		  case MONIT_SIGNAL:
			TermPrint(tMonit,L_("(%g ms de signal)"),f->axeX.r.max);
			break;
		  case MONIT_HISTO:
			TermPrint(tMonit,L_("(%s sur %d bins)"),MonitFenTypeName[f->type],f->axeX.pts);
			break;
		  case MONIT_FFT:
			TermPrint(tMonit,L_("(%s sur %d x %d echantillons %s evts)"),MonitFenTypeName[f->type],
				f->p.f.intervalles,f->axeX.pts,f->p.f.excl? "avec": "hors");
			break;
		  case MONIT_FONCTION:
		  case MONIT_BIPLOT:
			TermPrint(tMonit,L_("(%s[evts])"),MonitFenTypeName[f->type]);
			break;
		  case MONIT_2DHISTO:
			TermPrint(tMonit,L_("(%s sur %d x %d bins)"),MonitFenTypeName[f->type],f->axeX.pts,f->axeY.pts);
			break;
		  case MONIT_EVENT:
			TermPrint(tMonit,"(%s)",MonitFenTypeName[f->type]);
			break;
		  case MONIT_MOYEN:
		  case MONIT_PATTERN:
			TermPrint(tMonit,"(%s%s)",MonitFenTypeName[f->type],f->nb?"s":"");
			break;
		}
		TermPrint(tMonit,L_(": %d voie%c\n"),f->nb,(f->nb > 1)? 's': ' ');
		t = f->trace;
		for(j=0; j<f->nb; j++,t++) {
			switch(f->type) {
			  case MONIT_SIGNAL:
			  case MONIT_FFT:
			  case MONIT_EVENT:
				TermPrint(tMonit,"    %-16s %-8s",VoieManip[t->voie].nom,TrmtClassesListe[t->var]);
				break;
			  case MONIT_HISTO:
			  case MONIT_2DHISTO:
			  case MONIT_BIPLOT:
			  case MONIT_FONCTION:
				TermPrint(tMonit,"    %-12s %-16s",MonitFenVarName[t->var],VoieManip[t->voie].nom);
				break;
			  case MONIT_MOYEN:
			  case MONIT_PATTERN:
				TermPrint(tMonit,"    %-16s",VoieManip[(int)t->voie].nom);
				break;
			}
			if(f->type != MONIT_2DHISTO) {
				nuance = &(t->rgb);
				TermPrint(tMonit,L_(" (%6.2f%% rouge, %6.2f%% verte, %6.2f%% bleue)"),nuance->r,nuance->g,nuance->b);
			}
			TermPrint(tMonit,"\n");
		}
		TermPrint(tMonit,L_("            fenetre %d x %d en (%d, %d)\n"),
			f->larg,f->haut,f->posx,f->posy);
	}
	TermPrint(tMonit,"------------------------------------------------------------\n");
	TermRelease(tMonit);
//	if((tMonit->cdr)->displayed) OpiumClear(tMonit->cdr);
	OpiumDisplay(tMonit->cdr);
	return(0);
}
/* ========================================================================== */
int MonitFenAutorise() {
	int i,j;
	TypeMonitFenetre *f; Graph g;

	if(MonitFenNb == 0) {
		OpiumError(L_("Il n'y a PAS de fenetre definie"));
		return(0);
	}
	if(pMonitFenAffiche) {
		if(PanelItemMax(pMonitFenAffiche) < MonitFenNb) {
			PanelDelete(pMonitFenAffiche);
			pMonitFenAffiche = 0;
		} else PanelErase(pMonitFenAffiche);
	}
	if(pMonitFenAffiche == 0) pMonitFenAffiche = PanelCreate(MonitFenNb);
	for(i=0; i<MonitFenNb; i++) {
		j = PanelKeyB(pMonitFenAffiche,MonitFen[i].titre,"non/oui/tjrs",&(MonitFen[i].affiche),4);
		PanelItemColors(pMonitFenAffiche,j,SambaRougeVertJaune,3);
	}
	if(OpiumExec(pMonitFenAffiche->cdr) == PNL_CANCEL) return(0);
	MonitFenModifiees = 1;
	for(i=0, f=MonitFen; i<MonitFenNb; i++,f++) {
		if(f->affiche) MonitFenAffiche(f,1);
		else if((g = f->g)) { if(OpiumDisplayed(g->cdr)) OpiumClear(g->cdr); }
	}

	return(0);
}
/* ========================================================================== */
int MonitFenMemo(TypeMonitFenetre *f) {
	Cadre cdr; Graph g;
	
	if((g = f->g)) {
		cdr = g->cdr;
		if(!OpiumDisplayed(cdr)) return(0);
		OpiumGetPosition(cdr,&(f->posx),&(f->posy));
		OpiumGetSize    (cdr,&(f->larg),&(f->haut));
		if(f->posx <  5) f->posx =  5; if(f->posx > (OpiumServerWidth(0)  - 40)) f->posx = OpiumServerWidth(0)  - 40;
		if(f->posy < 15) f->posy = 15; if(f->posy > (OpiumServerHeigth(0) - 40)) f->posy = OpiumServerHeigth(0) - 40;
		if(f->type == MONIT_HISTO) {
			if(f->axeX.r.min != f->axeX.r.max) {
				GraphGetFloatRange(g,GRF_XAXIS,&(f->axeX.r.min),&(f->axeX.r.max));
				if(f->axeX.r.min >= f->axeX.r.max) f->axeX.r.min = f->axeX.r.max / 10.0;
			}
		} else if(f->type == MONIT_FFT) {
			if(f->axeY.r.min != f->axeY.r.max) {
				GraphGetFloatRange(g,GRF_YAXIS,&(f->axeY.r.min),&(f->axeY.r.max));
				if(f->axeY.r.min >= f->axeY.r.max) f->axeY.r.min = f->axeY.r.max / 10.0;
			}
			GraphGetFloatRange(g,GRF_XAXIS,&(f->axeX.r.min),&(f->axeX.r.max));
			f->p.f.xlog = GraphAxisIsLog(g,GRF_XAXIS);
			if(f->p.f.xlog) {
				if(f->axeX.r.min <= 0.0) f->axeX.r.min = 1.0;
				if(f->axeX.r.max <= 0.0) f->axeX.r.max = 1000.0;
				if(f->axeX.r.min >= f->axeX.r.max) f->axeX.r.min = f->axeX.r.max / 10.0;
			}
		} else if((f->type == MONIT_2DHISTO) || (f->type == MONIT_BIPLOT)) {
			if(f->axeX.r.min != f->axeX.r.max) {
				GraphGetFloatRange(g,GRF_XAXIS,&(f->axeX.r.min),&(f->axeX.r.max));
				if(f->axeX.r.min >= f->axeX.r.max) f->axeX.r.min = f->axeX.r.max / 10.0;
			}
			if(f->axeY.r.min != f->axeY.r.max) {
				GraphGetFloatRange(g,GRF_YAXIS,&(f->axeY.r.min),&(f->axeY.r.max));
				if(f->axeY.r.min >= f->axeY.r.max) f->axeY.r.min = f->axeY.r.max / 10.0;
			}
		} else if(f->type == MONIT_FONCTION) {
			if(f->axeY.r.min != f->axeY.r.max) {
				GraphGetFloatRange(g,GRF_YAXIS,&(f->axeY.r.min),&(f->axeY.r.max));
				if(f->axeY.r.min >= f->axeY.r.max) f->axeY.r.min = f->axeY.r.max / 10.0;
			}
		} else {
			if(f->axeY.i.min != f->axeY.i.max) {
				GraphGetIntRange(g,GRF_YAXIS,&(f->axeY.i.min),&(f->axeY.i.max));
				if(f->axeY.i.min >= f->axeY.i.max) f->axeY.i.min = f->axeY.i.max - 1;
			}
		}
	}
	MonitFenDimMin(f);

	return(0);
}
/* ========================================================================== */
void MonitTraceAjusteY(TypeMonitTrace *t, TypeTamponDonnees *donnees, Graph g) {
	int dim; int64 debut;
	TypeDonnee *sptr,sval,smin,smax;
	TypeSignal *rptr,rval,rmin,rmax;
	int i,j; 
	
	dim = donnees->nb / 2;
	if(dim) {
#ifdef EN_AVANCANT
		debut = donnees->prem + (donnees->nb - dim);
		j = Modulo(debut,donnees->max);
		if(donnees->type) {
			rptr = donnees->t.rval + j; 
			rmin = rmax = 0.0;
			for(i=0; i<dim; i++) {
				rval = (double)*rptr++;
				if(i) { if(rval < rmin) rmin = rval; if(rval > rmax) rmax = rval; }
				else rmin = rmax = rval;
				if(++j == donnees->max) { j = 0; rptr = donnees->t.rval; }
			}
			GraphAxisFloatRange(g,GRF_YAXIS,rmin,rmax);
		} else { 
			sptr = donnees->t.sval + j;
			smin = smax = 0;
			for(i=0; i<dim; i++) {
				sval = *sptr++;
				if(i) { if(sval < smin) smin = sval; if(sval > smax) smax = sval; }
				else smin = smax = sval;
				if(++j == donnees->max) { j = 0; sptr = donnees->t.sval; }
			}
			GraphAxisIntRange(g,GRF_YAXIS,smin,smax);
		}
#else  /* EN_AVANCANT */
		debut = donnees->prem + donnees->nb;
		j = Modulo(debut,donnees->max);
		if(donnees->type) {
			rptr = donnees->t.rval + j; 
			rmin = rmax = 0.0;
			for(i=0; i<dim; i++) {
				rval = (double)*(--rptr);
				if(i) { if(rval < rmin) rmin = rval; if(rval > rmax) rmax = rval; }
				else rmin = rmax = rval;
				if(--j < 0) { j = donnees->max; rptr = donnees->t.rval + j; }
			}
			GraphAxisFloatRange(g,GRF_YAXIS,rmin,rmax);
		} else { 
			sptr = donnees->t.sval + j;
			smin = smax = 0;
			for(i=0; i<dim; i++) {
				sval = *(--sptr);
				if(i) { if(sval < smin) smin = sval; if(sval > smax) smax = sval; }
				else smin = smax = sval;
				if(--j < 0) { j = donnees->max; sptr = donnees->t.sval + j; }
			}
			GraphAxisIntRange(g,GRF_YAXIS,smin,smax);
		}
#endif /* EN_AVANCANT */
		t->ajuste = 0;
	}
}
/* ========================================================================== */
int MonitFenAjusteY() {
	int i,j,k,num,nb;
	TypeMonitFenetre *f; TypeMonitTrace *t;
	char *fenetre[MAXMONIT+2]; short origine[MAXMONIT+2];
	char *trace[MAXTRACES+2]; char nomtrace[MAXTRACES][32];

	nb = 0;
	for(i=0, f=MonitFen; i<MonitFenNb; i++,f++) if(f->type == MONIT_SIGNAL) {
		fenetre[nb] = MonitFen[i].titre; origine[nb] = i; nb++;
	}
	fenetre[nb] = "\0";
	if(nb == 0) { OpiumNote(L_("Il n'y a PAS de fenetre concernee")); return(0); }
	else if(nb > 1) {
		num = 0;
		if(OpiumReadList(L_("De quelle fenetre"),fenetre,&num,32) == PNL_CANCEL) return(0);
	} else num = 0;
	f = &(MonitFen[origine[num]]);
	if(f->nb == 0) { OpiumNote(L_("Cette fenetre est vide")); return(0); }
	else if(f->nb > 1) {
		for(j=0, t=f->trace; j<f->nb; j++,t++) {
			snprintf(&(nomtrace[j][0]),32,"%s",VoieManip[t->voie].nom);
			trace[j] = &(nomtrace[j][0]);
		}
		trace[j] = "\0";
		k = 0;
		if(OpiumReadList(L_("d'apres la trace"),trace,&k,32) == PNL_CANCEL) return(0);
	} else k = 0;
	t = &(f->trace[k]);
	t->ajuste = 1;
	if(!Acquis[AcquisLocale].etat.active) {
		int voie; char trmt; TypeTamponDonnees *donnees; Graph g;
		if((g = f->g)) {
			// printf("%s/ Ajustement de l'axe Y demande pour la fenetre '%s', d'apres sa trace #%d\n",DateHeure(),f->titre,k+1);
			voie = t->voie; if(VoieTampon[voie].lue <= 0) return(0);
			trmt = t->var; donnees = &(VoieTampon[voie].trmt[trmt].donnees);
			MonitTraceAjusteY(t,donnees,g);
			// printf("%s/ Ajustement de l'axe Y termine pour la fenetre '%s', d'apres sa trace #%d\n",DateHeure(),f->titre,k+1);
			if(OpiumAlEcran(g)) OpiumRefresh(g->cdr);
		}
	}

	return(0);
}
/* ========================================================================== */
int MonitFenDetecteur() {
	int i,j,k,nb;
	int cap;
	char nomvoie[MODL_NOM];
	TypeMonitFenetre *f; TypeMonitTrace *t;
	
	if(MonitFenNb == 0) {
		OpiumError(L_("Il n'y a PAS de fenetre definie"));
		return(0);
	}

	if(OpiumExec(pMonitFenDetecteur->cdr) == PNL_CANCEL) return(0);
	if(pMonitFenChange) {
		if(PanelItemMax(pMonitFenChange) < MonitFenNb) {
			PanelDelete(pMonitFenChange);
			pMonitFenChange = 0;
		} else PanelErase(pMonitFenChange);
	}
	if(pMonitFenChange == 0) pMonitFenChange = PanelCreate(MonitFenNb);
	nb = 0 ;
	for(i=0, f=MonitFen; i<MonitFenNb; i++,f++) {
		t = f->trace;
		for(j=0; j<f->nb; j++,t++) if(t->bolo == MonitBoloAvant) break;
		if(j < f->nb) {
			k = PanelBool(pMonitFenChange,f->titre,&(f->change));
			PanelItemColors(pMonitFenChange,k,SambaRougeVertJaune,2);
			nb++;
		} else f->change = 0;
	}
	if(!nb) {
		OpiumError(L_("Aucun graphique n'utilise %s"),Bolo[MonitBoloAvant].nom);
		return(0);
	}
	if(OpiumExec(pMonitFenChange->cdr) == PNL_CANCEL) return(0);
	MonitFenModifiees = 1;
	for(i=0, f=MonitFen; i<MonitFenNb; i++,f++) if(f->change && (BoloNum != MonitBoloAvant)) {
		t = f->trace;
		for(j=0; j<f->nb; j++,t++) if(t->bolo == MonitBoloAvant) {
			t->bolo = BoloNum;
			strcpy(nomvoie,VoieManip[t->voie].nom);
			for(cap=0; cap<Bolo[BoloNum].nbcapt; cap++) {
				if(!strcmp(nomvoie,Bolo[BoloNum].captr[cap].nom)) break;
			}
			if(cap == Bolo[BoloNum].nbcapt) {
				OpiumError(L_("Pas de voie '%s' pour le detecteur '%s'"),nomvoie,Bolo[BoloNum].nom);
			} else {
				printf(". Dans %s, %s ",f->titre,VoieManip[t->voie].nom);
				t->cap = cap; t->voie = Bolo[BoloNum].captr[cap].voie;
				printf("est remplacee par %s\n",VoieManip[t->voie].nom);
			}
			for(cap=0; cap<Bolo[BoloNum].nbcapt; cap++) t->liste_voies[cap] = Bolo[BoloNum].captr[cap].nom;
			t->liste_voies[cap] = "\0";
		}
	}
	
	return(0);
}
/* ========================================================================== */
int MonitFenMemoToutes() {
	TypeMonitFenetre *f; int i;

	for(i=0, f=MonitFen; i<MonitFenNb; i++,f++) MonitFenMemo(f);
	return(0);
}
/* ========================================================================== */
int MonitFenZoom() {
	TypeMonitFenetre *f; Graph g;
	unsigned int zoom;
	int j; int voie; float duree,total; TypeMonitTrace *t;

	if(MonitFenNb > 1) {
		if(OpiumReadList(L_("Quelle fenetre"),MonitFenTitre,&MonitFenNum,32) == PNL_CANCEL)
			return(0);
	} else MonitFenNum = 0;
	if(MonitFenNum >= MonitFenNb) {
		OpiumError("Choix pas tres logique..."); // il(elle) a repondu: "(nouvelle fenetre)" !! :(
		return(0);
	}
	f = &(MonitFen[MonitFenNum]);
	MonitFenMemo(f);
	memcpy(&MonitFenLue,f,sizeof(TypeMonitFenetre));
	switch(f->type) {
	  case MONIT_SIGNAL:
//		MonitY.i.min = f->axeY.i.min; MonitY.i.max = f->axeY.i.max;
//		MonitX.r.max = f->axeX.r.max;
		PanelTitle(pMonitFenZoomData,MonitFenTitre[MonitFenNum]);
		if(OpiumExec(pMonitFenZoomData->cdr) == PNL_CANCEL) return(0);
		memcpy(f,&MonitFenLue,sizeof(TypeMonitFenetre));
//		f->axeY.i.min = MonitY.i.min; f->axeY.i.max = MonitY.i.max;
//		f->axeX.r.max = MonitX.r.max;
		total = 0.0;
		for(j=0, t=f->trace; j<f->nb; j++,t++) {
			voie = t->voie;
			switch(t->var) {
			  case TRMT_AU_VOL:
				if(!VoieTampon[voie].brutes->t.sval) break;
				if(total < DureeTampons) total = DureeTampons;
				break;
			  case TRMT_PRETRG:
				if(!VoieTampon[voie].traitees->t.rval) break;
				duree = (float)VoieTampon[voie].traitees->max * VoieTampon[voie].trmt[TRMT_PRETRG].ab6[1];
				if(total < duree) total = duree;
				break;
			}
		}
		if(f->axeX.r.max > total) f->axeX.r.max = total;
		zoom = total / f->axeX.r.max;
		// f->p.b.cadre = f->larg * zoom;
		if((g = f->g)) {
			// GraphResize(g,f->p.b.cadre,f->haut);
			GraphZoom(g,zoom,1);
			GraphAxisReset(g,GRF_XAXIS);
			if(f->axeY.i.min == f->axeY.i.max) GraphAxisAutoRange(g,GRF_YAXIS);
			else GraphAxisIntRange(g,GRF_YAXIS,f->axeY.i.min,f->axeY.i.max);
		}
		break;
	  case MONIT_EVENT:
//		MonitY.i.min = f->axeY.i.min; MonitY.i.max = f->axeY.i.max;
		PanelTitle(pMonitFenZoomEvts,MonitFenTitre[MonitFenNum]);
		if(OpiumExec(pMonitFenZoomEvts->cdr) == PNL_CANCEL) return(0);
		memcpy(f,&MonitFenLue,sizeof(TypeMonitFenetre));
//		f->axeY.i.min = MonitY.i.min; f->axeY.i.max = MonitY.i.max;
		if((g = f->g)) {
			GraphAxisReset(g,GRF_XAXIS);
			if(f->axeY.i.min == f->axeY.i.max) GraphAxisAutoRange(g,GRF_YAXIS);
			else GraphAxisIntRange(g,GRF_YAXIS,f->axeY.i.min,f->axeY.i.max);
		}
		break;
	  case MONIT_PATTERN:
	  case MONIT_MOYEN:
		PanelTitle(pMonitFenZoomMoyen,MonitFenTitre[MonitFenNum]);
		if(OpiumExec(pMonitFenZoomMoyen->cdr) == PNL_CANCEL) return(0);
		memcpy(f,&MonitFenLue,sizeof(TypeMonitFenetre));
		if((g = f->g)) {
			GraphAxisReset(g,GRF_XAXIS);
			if(f->axeY.i.min == f->axeY.i.max) GraphAxisAutoRange(g,GRF_YAXIS);
			else GraphAxisIntRange(g,GRF_YAXIS,f->axeY.i.min,f->axeY.i.max);
		}
		break;
	  case MONIT_HISTO:
//		MonitX.r.min = f->axeX.r.min; MonitX.r.max = f->axeX.r.max;
//		MonitEvts = f->axeY.i.max;
		PanelTitle(pMonitFenZoomHisto,MonitFenTitre[MonitFenNum]);
		if(OpiumExec(pMonitFenZoomHisto->cdr) == PNL_CANCEL) return(0);
		memcpy(f,&MonitFenLue,sizeof(TypeMonitFenetre));
//		f->axeX.r.min = MonitX.r.min; f->axeX.r.max = MonitX.r.max;
//		f->axeY.i.max = MonitEvts;
		if((g = f->g)) {
			if(f->axeX.r.min == f->axeX.r.max) GraphAxisAutoRange(g,GRF_XAXIS);
			else GraphAxisFloatRange(g,GRF_XAXIS,f->axeX.r.min,f->axeX.r.max);
			if(f->axeY.i.max) GraphAxisIntRange(g,GRF_YAXIS,0,f->axeY.i.max);
			else GraphAxisAutoRange(g,GRF_YAXIS);
		}
		break;
	  case MONIT_FONCTION:
//		MonitX.i.max = f->axeX.i.max;
//		MonitY.r.min = f->axeY.r.min; MonitY.r.max = f->axeY.r.max;
		PanelTitle(pMonitFenZoomFctn,MonitFenTitre[MonitFenNum]);
		if(OpiumExec(pMonitFenZoomFctn->cdr) == PNL_CANCEL) return(0);
		memcpy(f,&MonitFenLue,sizeof(TypeMonitFenetre));
//		f->axeX.i.max = MonitX.i.max;
//		f->axeY.r.min = MonitY.r.min; f->axeY.r.max = MonitY.r.max;
		if((g = f->g)) {
			if(f->axeX.i.max > 0.0) GraphAxisIntRange(g,GRF_XAXIS,1,f->axeX.i.max);
			else GraphAxisAutoRange(g,GRF_XAXIS);
			if(f->axeY.r.min == f->axeY.r.max) GraphAxisAutoRange(g,GRF_YAXIS);
			else {
				if(f->axeY.r.max <= f->axeY.r.min) f->axeY.r.max = f->axeY.r.min * 10.0;
				GraphAxisFloatRange(g,GRF_YAXIS,f->axeY.r.min,f->axeY.r.max);
			}
		}
		break;
	  case MONIT_2DHISTO:
		PanelTitle(pMonitFenZoomH2D,MonitFenTitre[MonitFenNum]);
		if(OpiumExec(pMonitFenZoomH2D->cdr) == PNL_CANCEL) return(0);
		memcpy(f,&MonitFenLue,sizeof(TypeMonitFenetre));
		if((g = f->g)) {
			if(f->axeX.r.min == f->axeX.r.max) GraphAxisAutoRange(g,GRF_XAXIS);
			else GraphAxisFloatRange(g,GRF_XAXIS,f->axeX.r.min,f->axeX.r.max);
			if(f->axeY.r.min == f->axeY.r.max) GraphAxisAutoRange(g,GRF_YAXIS);
			else GraphAxisFloatRange(g,GRF_YAXIS,f->axeY.r.min,f->axeY.r.max);
		}
		break;
	  case MONIT_BIPLOT:
		PanelTitle(pMonitFenZoomBiplot,MonitFenTitre[MonitFenNum]);
		if(OpiumExec(pMonitFenZoomBiplot->cdr) == PNL_CANCEL) return(0);
		memcpy(f,&MonitFenLue,sizeof(TypeMonitFenetre));
		if((g = f->g)) {
			if(f->axeX.r.min == f->axeX.r.max) GraphAxisAutoRange(g,GRF_XAXIS);
			else GraphAxisFloatRange(g,GRF_XAXIS,f->axeX.r.min,f->axeX.r.max);
			if(f->axeY.r.min == f->axeY.r.max) GraphAxisAutoRange(g,GRF_YAXIS);
			else GraphAxisFloatRange(g,GRF_YAXIS,f->axeY.r.min,f->axeY.r.max);
		}
		break;
	  case MONIT_FFT:
//		MonitX.r.min = f->axeX.r.min; MonitX.r.max = f->axeX.r.max;
//		MonitY.r.min = f->axeY.r.min; MonitY.r.max = f->axeY.r.max;
		PanelTitle(pMonitFenZoomFft,MonitFenTitre[MonitFenNum]);
		if(OpiumExec(pMonitFenZoomFft->cdr) == PNL_CANCEL) return(0);
		memcpy(f,&MonitFenLue,sizeof(TypeMonitFenetre));
//		f->axeX.r.min = MonitX.r.min; f->axeX.r.max = MonitX.r.max;
//		f->axeY.r.min = MonitY.r.min; f->axeY.r.max = MonitY.r.max;
		if((g = f->g)) {
			if(f->axeX.r.max > 0.0) GraphAxisFloatRange(g,GRF_XAXIS,0.001,f->axeX.r.max);
			else GraphAxisAutoRange(g,GRF_XAXIS);
			if(f->axeY.r.min == f->axeY.r.max) GraphAxisAutoRange(g,GRF_YAXIS);
			else {
				if(f->axeY.r.min <= 0) f->axeY.r.min = 0.1;
				if(f->axeY.r.max <= f->axeY.r.min) f->axeY.r.max = f->axeY.r.min * 10.0;
				GraphAxisFloatRange(g,GRF_YAXIS,f->axeY.r.min,f->axeY.r.max);
			}
		}
		break;
	}

	if((g = f->g)) { if(OpiumDisplayed(g->cdr)) OpiumRefresh(g->cdr); }

	return(0);
}
/* ========================================================================== */
int MonitFenAffiche(TypeMonitFenetre *f, char reellement) {
	int j,voie,numx,numy;
	TypeMonitTrace *t; Graph g;
	TypeTamponDonnees *donnees;

//	if(!(g = f->g)) { /* suppose que 1 fen creee == 1 fen qui a son graphique */
		MonitFenBuild(f);
		if(!(g = f->g)) return(0);
//	}
	if(f->type == MONIT_SIGNAL) {
		if(OpiumDisplayed(g->cdr)) OpiumClear(g->cdr);
		for(j=0, t=f->trace; j<f->nb; j++,t++) {
			voie = t->voie; numx = 2 * j; numy = numx + 1;
		#ifdef ALAMBIQUE
			if(t->var == TRMT_AU_VOL) {
				int point1;
				GraphDataUse(g,numx,VoieTampon[voie].brutes->nb);
				GraphDataUse(g,numy,VoieTampon[voie].brutes->nb);
				//- if((VoieTampon[voie].brutes->nb >= VoieTampon[voie].brutes->max) && (VoieTampon[voie].brutes->max > 0)) {
					//- point1 = Modulo(VoieTampon[voie].trmt[TRMT_AU_VOL].plus_ancien,VoieTampon[voie].brutes->max); /* point1 == brutes.prem? */
					point1 = VoieTampon[voie].brutes->prem;
					GraphDataRescroll(g,numx,point1);
					GraphDataRescroll(g,numy,point1);
				//- }
			} else {  /* (t->var == TRMT_PRETRG) */
				GraphDataUse(g,numx,VoieTampon[voie].traitees->nb);
				GraphDataUse(g,numy,VoieTampon[voie].traitees->nb);
				//- if(VoieTampon[voie].traitees->nb >= VoieTampon[voie].traitees->max) {
					GraphDataRescroll(g,numx,VoieTampon[voie].traitees->prem);
					GraphDataRescroll(g,numy,VoieTampon[voie].traitees->prem);
				//- }
			}
		#else  /* ALAMBIQUE */
			donnees = &(VoieTampon[voie].trmt[t->var].donnees);
			GraphDataUse(g,numx,donnees->nb);
			GraphDataUse(g,numy,donnees->nb);
			GraphDataRescroll(g,numx,donnees->prem);
			GraphDataRescroll(g,numy,donnees->prem);
		#endif /* ALAMBIQUE */
		}
	}
	if(reellement) OpiumDisplay(g->cdr);
	return(1);
}
/* ========================================================================== */
int MonitVoieImprime() {
	int voie,bolo,vm,cap;
	int i,j,nb,dernier,total;
	int min,max,moy;
	TypeDonnee *val;

	if(OpiumExec(pMonitVoiePrint->cdr) == PNL_CANCEL) return(0);
	bolo = BoloNum; vm = ModlNum;
	if(Bolo[BoloNum].lu == DETEC_HS) {
		OpiumError("Le detecteur %s n'est pas lu",Bolo[BoloNum].nom);
		BoloNum = bolo; ModlNum = vm;
		return(0);
	}
	voie = 0;
	for(cap=0; cap<Bolo[BoloNum].nbcapt; cap++) {
		voie = Bolo[BoloNum].captr[cap].voie;
		if(ModlNum == VoieManip[voie].type) break;
	}
	if(cap == Bolo[BoloNum].nbcapt) {
		OpiumError("Pas de voie '%s' pour le detecteur '%s'",ModeleVoie[ModlNum].nom,Bolo[BoloNum].nom);
		BoloNum = bolo; ModlNum = vm;
		return(0);
	}
	VoieNum = voie;

	if(!VoieTampon[voie].brutes->t.sval) {
		OpiumError("Le tampon de la voie %s n'est pas defini",VoieManip[voie].nom);
		return(0);
	}
	TermHold(tMonit);
	dernier = (int)(VoieTampon[voie].trmt[TRMT_AU_VOL].plus_ancien + VoieTampon[voie].brutes->nb);
	if(MonitVoieVal0 < VoieTampon[voie].trmt[TRMT_AU_VOL].plus_ancien)
		MonitVoieVal0 = (int)(VoieTampon[voie].trmt[TRMT_AU_VOL].plus_ancien);
	else if(MonitVoieVal0 >= dernier)
		MonitVoieVal0 = dernier - 1;
	if(MonitVoieValN > dernier )
		MonitVoieValN = dernier;
	total = MonitVoieValN - MonitVoieVal0;
	if(total <= 0) return(0);

	TermPrint(tMonit,"%s a partir de %d (%d/%d points)\n",
		VoieManip[voie].nom,MonitVoieVal0,total,VoieTampon[voie].brutes->nb);
	i= MonitVoieVal0; j = i % VoieTampon[voie].brutes->max;
	val = VoieTampon[voie].brutes->t.sval + j;
	min = 999999; max = 0; moy = 0;
	nb = 0;
	while(nb < total) {
		if(!(nb % 10)) TermPrint(tMonit,"%11d: ",i);
		if(min > *val) min = *val;
		if(max < *val) max = *val;
		moy += *val;
		TermPrint(tMonit," %05X",*val++); i++;
		if(++j == VoieTampon[voie].brutes->max) val = VoieTampon[voie].brutes->t.sval;
		++nb;
		if(!(nb % 10)) TermPrint(tMonit,"\n");
		else if(!(nb % 5)) TermPrint(tMonit,"  ");
	};
	if(nb % 10) TermPrint(tMonit,"\n");
	TermPrint(tMonit,"min: %05X, max: %05X, moy: %05X\n",min,max,moy/total);
	TermRelease(tMonit);
	if(!(tMonit->cdr->displayed)) OpiumDisplay(tMonit->cdr);

	return(0);
}
/* ========================================================================== */
static char MonitVoieCree(int voie) {
	int bolo, y;
	char titre[80];

	
	if(!VoieTampon[voie].brutes->t.sval)
		OpiumError("Le tampon de la voie %s n'est pas defini",VoieManip[voie].nom);
	else {
		gDonnee[voie] = GraphCreate(MonitLarg,MonitHaut,2);
		if(!gDonnee[voie]) return(0);
		sprintf(titre,"%s",VoieManip[voie].nom);
		OpiumTitle(gDonnee[voie]->cdr,titre);
		/* abcisse en millisecondes */
		GraphAdd(gDonnee[voie],GRF_FLOAT|GRF_INDEX|GRF_XAXIS,VoieTampon[voie].trmt[TRMT_AU_VOL].ab6,VoieTampon[voie].brutes->max);
		y = GraphAdd(gDonnee[voie],GRF_SHORT|GRF_YAXIS,
			VoieTampon[voie].brutes->t.sval,VoieTampon[voie].brutes->max);
		GraphDataRGB(gDonnee[voie],y,GRF_RGB_GREEN);
		bolo = VoieManip[voie].det;
		OpiumPosition(gDonnee[voie]->cdr,(bolo * MonitLarg) + 10,
			(voie * (MonitHaut + 40)) + 200);
		GraphParmsSave(gDonnee[voie]);
		return(1);
	}
	return(0);
}
/* ========================================================================== */
static void MonitVoieReset(int voie) {
	int nbpts;
	unsigned int zoom;
	// int larg;

	nbpts = MonitDuree * Echantillonnage;
	if(nbpts > VoieTampon[voie].brutes->max) {
		nbpts = VoieTampon[voie].brutes->max;
		MonitDuree = (float)nbpts / Echantillonnage;
	}
	zoom = ((VoieTampon[voie].brutes->max - 1) / nbpts) + 1;
	// larg = MonitLarg * zoom;
	// GraphResize(gDonnee[voie],larg,MonitHaut);
	GraphZoom(gDonnee[voie],zoom,1);
	GraphAxisIntRange(gDonnee[voie],GRF_YAXIS,VoieManip[voie].min,VoieManip[voie].max);
	GraphUse(gDonnee[voie],VoieTampon[voie].brutes->nb);
	GraphRescroll(gDonnee[voie],Modulo(VoieTampon[voie].trmt[TRMT_AU_VOL].plus_ancien,VoieTampon[voie].brutes->max));
	GraphAxisReset(gDonnee[voie],GRF_XAXIS);
	GraphAxisReset(gDonnee[voie],GRF_YAXIS);
	OpiumDisplay(gDonnee[voie]->cdr);
	VoieManip[voie].affiche = 1;
}
/* ========================================================================== */
int MonitChoisiBoloVoie(int *voie_choisie) {
	int voie,bolo,vm,cap;

	bolo = BoloNum; vm = ModlNum;
	if(OpiumExec(pMonitBoloVoie->cdr) == PNL_CANCEL) return(0);
	if(Bolo[BoloNum].lu == DETEC_HS) {
		OpiumError("Le detecteur %s n'est pas lu",Bolo[BoloNum].nom);
		BoloNum = bolo; ModlNum = vm;
		return(0);
	}
	voie = 0;
	for(cap=0; cap<Bolo[BoloNum].nbcapt; cap++) {
		if((voie = Bolo[BoloNum].captr[cap].voie) < 0) continue;
		if(ModlNum == VoieManip[voie].type) break;
	}
	if(cap == Bolo[BoloNum].nbcapt) {
		OpiumError("Pas de voie '%s' pour le detecteur '%s'",ModeleVoie[ModlNum].nom,Bolo[BoloNum].nom);
		BoloNum = bolo; ModlNum = vm;
		return(0);
	}
	VoieNum = voie;
//	printf("(MonitChoisiBoloVoie) BoloNum=%d/%d, VoieNum=%d/%d\n",BoloNum,BoloNb,VoieNum,VoiesNb);
	*voie_choisie = voie;
	return(1);
}
/* ========================================================================== */
int MonitVoieAffiche() {
	int i,j; int voie;

	if(!MonitChoisiBoloVoie(&voie)) return(0);
	PanelErase(pMonitLimites1Voie);
	OpiumTitle(pMonitLimites1Voie->cdr,VoieManip[voie].nom);
	i = PanelInt(pMonitLimites1Voie,"Valeur minimum (ADU)",&(VoieManip[voie].min)); /* samba.c */
	j = PanelInt(pMonitLimites1Voie,"Valeur maximum (ADU)",&(VoieManip[voie].max)); /* samba.c */
	PanelFloat(pMonitLimites1Voie,"Duree affichee (ms)",&MonitDuree);
	PanelInt  (pMonitLimites1Voie,"Hauteur graphique",&MonitHaut);
	PanelInt  (pMonitLimites1Voie,"Largeur graphique",&MonitLarg);
	PanelItemILimits(pMonitLimites1Voie,i,-VoieManip[voie].zero,VoieManip[voie].zero-1);
	PanelItemILimits(pMonitLimites1Voie,j,-VoieManip[voie].zero,VoieManip[voie].zero-1);

	if(OpiumExec(pMonitLimites1Voie->cdr) == PNL_CANCEL) {
		if(gDonnee[voie]) {
			if(OpiumDisplayed(gDonnee[voie]->cdr)) OpiumClear(gDonnee[voie]->cdr);
		}
		VoieManip[voie].affiche = 0;
	} else {
		if(!gDonnee[voie]) {
			if(!MonitVoieCree(voie)) return(0);
		} else {
			if(OpiumDisplayed(gDonnee[voie]->cdr)) OpiumClear(gDonnee[voie]->cdr);
		}
		MonitVoieReset(voie);
	}
	return(0);
}
/* ========================================================================== */
int MonitVoieFiltrage() {
	int voie;
	char titre[80];
	int i,j,y;
	int nb_brutes,nb_filtrees;
	int64 prems_brute;

	if(!MonitChoisiBoloVoie(&voie)) return(0);
	if(!VoieTampon[voie].traitees->t.rval) {
		OpiumError("Pas de resultat du filtrage pour la voie %s",
			VoieManip[voie].nom);
		return(0);
	}
	PanelErase(pMonitLimites1Voie);
	OpiumTitle(pMonitLimites1Voie->cdr,VoieManip[voie].nom);
	i = PanelInt(pMonitLimites1Voie,"Valeur minimum (ADU)",&(VoieManip[voie].min));
	j = PanelInt(pMonitLimites1Voie,"Valeur maximum (ADU)",&(VoieManip[voie].max));
	PanelInt(pMonitLimites1Voie,"Hauteur graphique",&MonitHaut);
	PanelInt(pMonitLimites1Voie,"Largeur graphique",&MonitLarg);
	PanelItemILimits(pMonitLimites1Voie,i,-VoieManip[voie].zero,VoieManip[voie].zero-1);
	PanelItemILimits(pMonitLimites1Voie,j,-VoieManip[voie].zero,VoieManip[voie].zero-1);

	if(OpiumExec(pMonitLimites1Voie->cdr) == PNL_CANCEL) {
		if(gFiltree[voie]) {
			if(OpiumDisplayed(gFiltree[voie]->cdr)) OpiumClear(gFiltree[voie]->cdr);
		}
	} else {
		if(!gFiltree[voie]) {
			gFiltree[voie] = GraphCreate(MonitLarg,MonitHaut,4);
			if(!gFiltree[voie]) return(0);
			sprintf(titre,"%s filtree",VoieManip[voie].nom);
			OpiumTitle(gFiltree[voie]->cdr,titre);
			/* abcisse en millisecondes */
			GraphAdd(gFiltree[voie],GRF_FLOAT|GRF_INDEX|GRF_XAXIS,VoieTampon[voie].trmt[TRMT_AU_VOL].ab6,VoieTampon[voie].brutes->max);
			y = GraphAdd(gFiltree[voie],GRF_SHORT|GRF_YAXIS,
				VoieTampon[voie].brutes->t.sval,VoieTampon[voie].brutes->max);
			GraphDataRGB(gFiltree[voie],y,GRF_RGB_GREEN);
			GraphAdd(gFiltree[voie],GRF_FLOAT|GRF_INDEX|GRF_XAXIS,FiltreeAb6,VoieTampon[voie].traitees->max);
			y = GraphAdd(gFiltree[voie],GRF_SHORT|GRF_YAXIS,
				VoieTampon[voie].traitees->t.rval,VoieTampon[voie].traitees->max);
			GraphDataRGB(gFiltree[voie],y,GRF_RGB_MAGENTA);
		} else {
			if(OpiumDisplayed(gFiltree[voie]->cdr))
				OpiumClear(gFiltree[voie]->cdr);
		}
		GraphAxisIntRange(gFiltree[voie],GRF_YAXIS,VoieManip[voie].min,VoieManip[voie].max);
		nb_filtrees = VoieTampon[voie].traitees->nb;
		nb_brutes = nb_filtrees * VoieTampon[voie].trmt[TRMT_PRETRG].compactage;
		prems_brute = VoieTampon[voie].lus - (int64)nb_brutes;
		/* sauf que signal[].prems correspond a LectStampsLus au dernier appel a TrmtSurBuffer,
		   il faut donc affiner la synchronisation */
		prems_brute = ((prems_brute / (int64)nb_brutes) * (int64)nb_brutes)
			 + (int64)(VoieTampon[voie].traitees->prem * VoieTampon[voie].trmt[TRMT_PRETRG].compactage);
		GraphDataUse(gFiltree[voie],0,nb_brutes);
		GraphDataUse(gFiltree[voie],1,nb_brutes);
		GraphDataRescroll(gFiltree[voie],0,Modulo(prems_brute,VoieTampon[voie].brutes->max));
		GraphDataRescroll(gFiltree[voie],1,Modulo(prems_brute,VoieTampon[voie].brutes->max));

		FiltreeAb6[1] = VoieEvent[voie].horloge * VoieTampon[voie].trmt[TRMT_PRETRG].compactage;
		FiltreeAb6[0] = ((float)prems_brute * VoieEvent[voie].horloge)
			 - (VoieTampon[voie].traitees->prem * FiltreeAb6[1]);
		GraphDataUse(gFiltree[voie],2,nb_filtrees);
		GraphDataUse(gFiltree[voie],3,nb_filtrees);
		GraphDataRescroll(gFiltree[voie],2,VoieTampon[voie].traitees->prem);
		GraphDataRescroll(gFiltree[voie],3,VoieTampon[voie].traitees->prem);
		GraphAxisReset(gFiltree[voie],GRF_XAXIS);
		GraphAxisReset(gFiltree[voie],GRF_YAXIS);
//		GraphParmsSave(gFiltree[voie]);
		OpiumDisplay(gFiltree[voie]->cdr);
/*		{	int i,j;
			j = VoieTampon[voie].traitees->prem;
			printf("Tampon des valeurs filtrees:");
			for(i=0; i<VoieTampon[voie].traitees->nb; i++) {
				if(!(i%10)) printf("\n%4d:",i+(int)prems_brute);
				printf("%6d",VoieTampon[voie].traitees->t.rval[j]);
				j++;
				if(j >= VoieTampon[voie].traitees->max) j = 0;
			}
			printf("\n");
		} */
	}
	return(0);
}
/* ========================================================================== */
int MonitVoieRafraichi() {
	int voie;

	for(voie=0; voie<VoiesNb; voie++) {
		if(!gDonnee[voie]) continue;
		if(OpiumDisplayed(gDonnee[voie]->cdr)) OpiumClear(gDonnee[voie]->cdr);
		MonitVoieReset(voie);
	}
	return(0);
}
/* ========================================================================== */
int MonitVoieEfface() {
	int voie;

	for(voie=0; voie<VoiesNb; voie++) {
		if(!gDonnee[voie]) continue;
		if(OpiumDisplayed(gDonnee[voie]->cdr)) OpiumClear(gDonnee[voie]->cdr);
		VoieManip[voie].affiche = 0;
	}
	return(0);
}
/* ========================================================================== */
int MonitEvtMoyenVoie(Menu menu, MenuItem *item) {
	int voie; int y;

	if(!MonitChoisiBoloVoie(&voie)) return(0);
	if(VoieTampon[voie].moyen.nb <= 0) {
		OpiumError(L_("Pas d'evenement calcule pour la voie %s"),VoieManip[voie].nom);
		return(0);
	}
	MonitEvtAb6[voie][0] = 0.0;
	if(OpiumDisplayed(gEvtMoyen->cdr)) OpiumClear(gEvtMoyen->cdr); /* pour que le titre change ET que le nom disparaisse dans le menu Fenetres */
	GraphErase(gEvtMoyen);
	sprintf(MonitTitre,"Evenement moyen, voie %s",VoieManip[voie].nom);
	OpiumTitle(gEvtMoyen->cdr,MonitTitre);
	GraphAdd(gEvtMoyen,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,&(MonitEvtAb6[voie][0]),VoieTampon[voie].somme.taille);
	y = GraphAdd(gEvtMoyen,GRF_FLOAT|GRF_YAXIS,VoieTampon[voie].somme.val,VoieTampon[voie].somme.taille);
	GraphDataRGB(gEvtMoyen,y,GRF_RGB_GREEN);
	GraphUse(gEvtMoyen,VoieTampon[voie].somme.taille);
	if(MonitChgtUnite) GraphAxisChgtUnite(gEvtMoyen,GRF_YAXIS,MonitUnitesADU);
	GraphAxisReset(gEvtMoyen,GRF_XAXIS);
	GraphAxisReset(gEvtMoyen,GRF_YAXIS);
	OpiumDisplay(gEvtMoyen->cdr);
	
	return(0);
}
/* ========================================================================== */
int MonitEvtMoyenBolo(Menu menu, MenuItem *item) {
	int bolo,cap,voie; int y;
	float tdeb,tfin,tmin,tmax;
	char prems;
	OpiumColorState s; int k;

	bolo = BoloNum;
	if(OpiumReadList("Pour le detecteur",BoloNom,&bolo,DETEC_NOM) == PNL_CANCEL) return(0);
	BoloNum = bolo;
	if(OpiumDisplayed(gEvtMoyen->cdr)) OpiumClear(gEvtMoyen->cdr); /* pour que le titre change ET que le nom disparaisse dans le menu Fenetres */
	GraphErase(gEvtMoyen);
	OpiumColorInit(&s);
	sprintf(MonitTitre,"Evenement moyen, bolo %s",Bolo[BoloNum].nom);
	OpiumTitle(gEvtMoyen->cdr,MonitTitre);
	prems = 1;
	tmin = tmax = 0.0; /* gnu */
	for(cap=0; cap<Bolo[BoloNum].nbcapt; cap++) {
		voie = Bolo[BoloNum].captr[cap].voie;
		if(VoieTampon[voie].moyen.nb <= 0) continue;
		tdeb = VoieTampon[voie].moyen.decal;
		tfin =  tdeb +  (VoieTampon[voie].somme.taille * VoieEvent[voie].horloge);
		if(prems) { tmin = tdeb; tmax = tfin; prems = 0; }
		else { if(tmin > tdeb) tmin = tdeb; if(tmax < tfin) tmax = tfin; }
		MonitEvtAb6[voie][0] = tdeb;
		GraphAdd(gEvtMoyen,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,&(MonitEvtAb6[voie][0]),VoieTampon[voie].somme.taille);
		y = GraphAdd(gEvtMoyen,GRF_FLOAT|GRF_YAXIS,VoieTampon[voie].somme.val,VoieTampon[voie].somme.taille);
		k = OpiumColorNext(&s);
		GraphDataRGB(gEvtMoyen,y,OpiumColorRGB(k));
	}
//	GraphUse(gEvtMoyen,VoieTampon[voie].somme.taille);
//	GraphAxisReset(gEvtMoyen,GRF_XAXIS);
	GraphAxisFloatRange(gEvtMoyen,GRF_XAXIS,tmin,tmax);
	if(MonitChgtUnite) GraphAxisChgtUnite(gEvtMoyen,GRF_YAXIS,MonitUnitesADU);
	GraphAxisReset(gEvtMoyen,GRF_YAXIS);
	OpiumDisplay(gEvtMoyen->cdr);

	return(0);
}
/* ========================================================================== */
void MonitEvtValeurs(int evtnum) {
	int vt,voietrig; int64 evtmax;
	int secs,msecs,usecs; // a ajouter dans MonitEvtAffiche si re-integration
	TypeVoieArchivee *triggee;

	if(evtnum < 0) {
		BoloTrigge[0] = VoieTriggee[0] = TempsTrigger[0] = '\0';
		MonitEvtAmpl = 0;
		LectCntl.MonitEvtNum = 0;
	} else {
		if(!Evt[evtnum].nbvoies) return; // triggee->num serait indefini meme si vt==0
		vt = Evt[evtnum].voie_evt;
		triggee = &(Evt[evtnum].voie[vt]);
		voietrig = triggee->num;
		evtmax = triggee->debut + (int64)VoieEvent[voietrig].avant_evt;
		strcpy(BoloTrigge,Bolo[VoieManip[voietrig].det].nom);
		strcpy(VoieTriggee,VoieManip[voietrig].prenom);
		SambaTempsEchantillon(evtmax,0 /* -(int)MonitT0 */,0,&secs,&usecs);
		msecs = usecs / 1000; usecs = usecs - (msecs * 1000);
		snprintf(TempsTrigger,15,"%d%03d,%03d",secs,msecs,usecs);
		MonitEvtAmpl = triggee->amplitude;
	}
}
/* ========================================================================== */
static float MonitTraiteeOf7(int voie, TypeTamponDonnees *tampon, int lngr, int64 debut, int nb) {
	/* Calcule l'offset, ou ligne de base, d'un evenement demarrant exactement a <debut> */
	int i,k; double base;
	TypeDonnee *ptr_int16,sval;
	TypeSignal *ptr_float,rval;
	
	k = Modulo(debut,lngr);
	base = 0.0;
	if(tampon->type == TAMPON_INT16) {
		ptr_int16 = tampon->t.sval + k;
		for(i=0; i<nb; i++) {
			sval = *ptr_int16++; base += (double)sval;
			if(++k >= lngr) { k = 0; ptr_int16 = tampon->t.sval; }
		}
	} else {
		ptr_float = tampon->t.rval + k;
		for(i=0; i<nb; i++) {
			rval = *ptr_float++; base += (double)rval;
			if(++k >= lngr) { k = 0; ptr_float = tampon->t.rval; }
		}
	}
	base = (base / (double)nb);
	return((float)base);
}
/* ========================================================================== */
int MonitEvtAffiche(int lequel, void *qui, int affiche) {
	int voietrig,vm,vt;
	int evtprems,evtnum,evtdim,pos_evt,pos_trmt;
	int64 evtstart,evtend;
	int64 trmtstart,trmtend,ldbstart,ldbend; int trmtdim,ldbdim; int64 reduction;
	int64 debut_traitees,fin_traitees,t0_evt,t0_trmt,decalage;
	TypeVoieArchivee *triggee;
	Graph g; int x,y,rc;
#ifdef CARRE_A_VERIFIER
	int64 evtmax;
#endif

	if(!EvtASauver) { if(affiche) OpiumError("Pas d'evenement en memoire"); return(0); }
	if(!lequel) {
		for(evtprems = 0; evtprems < EvtASauver; evtprems++) {
			vt = Evt[evtprems].voie_evt;
			voietrig = Evt[evtprems].voie[vt].num;
			evtstart = Evt[evtprems].voie[vt].debut;
	/*		evtdim = Evt[evtprems].dim / VoieTampon[voietrig].trmt[TRMT_AU_VOL].compactage;
			if((evtstart + evtdim) >= VoieTampon[voietrig].trmt[TRMT_AU_VOL].plus_ancien) break; */
			if(evtstart >= VoieTampon[voietrig].trmt[TRMT_AU_VOL].plus_ancien) break;
		}
		if(evtprems >= EvtASauver) { if(affiche) OpiumError("Plus d'evenement en memoire"); return(0); }
		evtnum = LectCntl.MonitEvtNum - Evt[0].num;
		if((evtnum < evtprems) || (LectCntl.MonitEvtNum > Evt[EvtASauver - 1].num)) {
			if(affiche) OpiumError("Evenement #%d illegal ou archive (accessibles: [%d .. %d])",
				LectCntl.MonitEvtNum,Evt[evtprems].num,Evt[EvtASauver - 1].num);
			if(LectCntl.MonitEvtNum < Evt[evtprems].num) LectCntl.MonitEvtNum = Evt[evtprems].num;
			else if(LectCntl.MonitEvtNum > Evt[EvtASauver - 1].num) LectCntl.MonitEvtNum = Evt[EvtASauver - 1].num;
			if(!affiche) return(0);
			evtnum = LectCntl.MonitEvtNum - Evt[0].num;
		}
	} else {
		evtnum = lequel;
		LectCntl.MonitEvtNum = Evt[evtnum].num;
	}

	vt = Evt[evtnum].voie_evt;
	triggee = &(Evt[evtnum].voie[vt]);
	voietrig = triggee->num;
	vm = VoieManip[voietrig].type;
	evtstart = triggee->debut;
	evtdim = triggee->dim;
	evtend = evtstart + evtdim;
#ifdef CARRE_A_VERIFIER
	evtmax = evtstart + (int64)VoieEvent[voietrig].avant_evt;
#endif
	pos_evt = triggee->adresse;

	if(triggee->stream) {
		if(evtend < VoieTampon[voietrig].trmt[TRMT_AU_VOL].plus_ancien) {
			if(affiche) OpiumError("Cet evenement n'est plus dans le tampon");
			return(0);
		}
		trmtstart = evtstart; trmtend = evtend;
		if(VerifMonitEvts == 1) printf("(%s) Evt#%03d: [%lld .. %lld[ mesure\n",__func__,Evt[evtnum].num,evtstart,evtend);
		if((decalage = evtstart - VoieTampon[voietrig].trmt[TRMT_AU_VOL].plus_ancien) < 0) {
			evtstart = VoieTampon[voietrig].trmt[TRMT_AU_VOL].plus_ancien;
			evtdim = (int)(evtend - evtstart);
			pos_evt -= decalage; // decalage est negatif, ici
		}
		if(evtend > VoieTampon[voietrig].lus) evtdim = (int)(VoieTampon[voietrig].lus - evtstart);
		if(evtdim <= 0) {
			if(affiche) OpiumWarn("Voie %s @%lld + %d: dimension disponible = %d",VoieManip[voietrig].nom,triggee->debut,triggee->dim,evtdim);
			return(0);
		}
	}

	if(VerifMonitEvts == 1) printf("(MonitEvtAffiche)          [%lld .. %lld[ stocke\n",evtstart,evtend);
	MonitEvtValeurs(evtnum);
	
#ifdef CARRE_A_VERIFIER
/* ab6 recalcule seulement si signal voietrig affiche dans une fenetre user. Sinon voir avec MonitT0*1000.0 */
//	MomentEvt[2] = MomentEvt[3] = (((float)evtmax * VoieEvent[voietrig].horloge)) - VoieTampon[voietrig].trmt[TRMT_PRETRG].ab6[0];
	MomentEvt[2] = MomentEvt[3] = (float)(evtmax  - (MonitP0 / VoieTampon[voietrig].trmt[TRMT_AU_VOL].compactage)) * VoieEvent[voietrig].horloge;
	MomentEvt[0] = MomentEvt[1] = MomentEvt[4] = MomentEvt[2] - triggee->montee;
	AmplitudeEvt[0] = AmplitudeEvt[3] = AmplitudeEvt[4] = (short)triggee->base;
	AmplitudeEvt[1] = AmplitudeEvt[2] = AmplitudeEvt[0] + (short)triggee->amplitude;
	if(VerifMonitEvts == 1) printf("(MonitEvtAffiche)          { (%g, %d), (%g, %d), (%g, %d), (%g, %d), (%g, %d) }\n",
		MomentEvt[0],AmplitudeEvt[0],MomentEvt[1],AmplitudeEvt[1],MomentEvt[2],AmplitudeEvt[2],
		MomentEvt[3],AmplitudeEvt[3],MomentEvt[4],AmplitudeEvt[4]);
#endif

//	printf("(%s) mMonit @%08X, mMonitEvts @%08X, appelant @%08X\n",__func__,(hexa)mMonit,(hexa)mMonitEvts,(hexa)qui);
//	printf("(%s) gEvtPlanche @%08X: %s.\n",__func__,(hexa)gEvtPlanche,(OpiumDisplayed((gEvtPlanche->cdr)->planche))? "affiche": "absent");
	if((qui == (void *)mMonit) || (qui == (void *)mMonitEvts)) g = gEvtSolo;
	else {
		if(OpiumDisplayed((gEvtPlanche->cdr)->planche)) g = gEvtPlanche; else g = gEvtSolo;
	}
//	printf("(%s) Affiche graph @%08X (%s)\n",__func__,(hexa)g,(OpiumDisplayed(g->cdr))? "actif": "masque");
	if(g == gEvtSolo) {
		if(OpiumDisplayed(gEvtSolo->cdr)) OpiumClear(gEvtSolo->cdr); /* pour que le titre change ET que le nom disparaisse dans le menu Fenetres */
		sprintf(MonitTitre,"Evenement #%d: voie %s",LectCntl.MonitEvtNum,VoieManip[voietrig].nom);
		OpiumTitle(gEvtSolo->cdr,MonitTitre); 
	 	if(MonitChgtUnite) GraphAxisChgtUnite(gEvtSolo,GRF_YAXIS,MonitUnitesADU);
	}
	/* pos_evt ~ triggee->adresse, et t0_evt sert a fixer une date commune avec trmt */
	// t0_evt = (evtstart / (int64)VoieTampon[voietrig].brutes->max) * (int64)VoieTampon[voietrig].brutes->max;
	// pos_evt = (int)(evtstart - t0_evt);
	t0_evt = evtstart - (int64)pos_evt;
	if(VerifMonitEvts == 1) printf("(MonitEvtAffiche)          au vol[0]= %lld, debut evt +%d\n",t0_evt,pos_evt);
	MonitEvtAb6[voietrig][0] = (float)(((double)t0_evt * (double)VoieTampon[voietrig].trmt[TRMT_AU_VOL].ab6[1]) - (double)(MonitT0 * 1000.0));
	GraphDataReplace(g,0,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,&(MonitEvtAb6[voietrig][0]),VoieTampon[voietrig].brutes->max);
	rc = GraphDataReplace(g,1,GRF_SHORT|GRF_YAXIS,VoieTampon[voietrig].brutes->t.sval,VoieTampon[voietrig].brutes->max);
	GraphAxisAutoRange(g,GRF_XAXIS);
	if(rc < 0) printf("! (MonitEvtAffiche) le pointage sur l'evenement brut a echoue\n");
	else {
		float base_traitee,base_brute;
		// GraphDataIntOffset(g,1,-(int)base);
		GraphDataUse(g,0,evtdim);
		GraphDataUse(g,1,evtdim);
		GraphDataRescroll(g,0,pos_evt);
		GraphDataRescroll(g,1,pos_evt);
		if(VerifMonitEvts == 1) printf("(MonitEvtAffiche)          Ab6[%d] au vol = %g + %d x %g, soit [%g .. %g[\n",
			voietrig,MonitEvtAb6[voietrig][0],pos_evt,MonitEvtAb6[voietrig][1],
			MonitEvtAb6[voietrig][0]+((float)pos_evt*MonitEvtAb6[voietrig][1]),
			MonitEvtAb6[voietrig][0]+((float)(pos_evt+evtdim)*MonitEvtAb6[voietrig][1]));
		#ifdef MONIT_EVT_TRAITE
		if(VoieTampon[voietrig].traitees->t.rval) {
			reduction = (int64)VoieTampon[voietrig].trmt[TRMT_PRETRG].compactage;
			/* unite: point du tampon au vol */
			fin_traitees = ((DernierPointTraite - 1) / (int64)VoieTampon[voietrig].trmt[TRMT_AU_VOL].compactage) + 1;
			debut_traitees = fin_traitees - (VoieTampon[voietrig].traitees->nb * reduction);
			ldbstart = trmtstart + VoieEvent[voietrig].base_dep; ldbend = ldbstart + VoieEvent[voietrig].base_lngr;
			if(ldbstart < debut_traitees) ldbstart = debut_traitees;
			if(ldbend < debut_traitees) ldbend = debut_traitees; else if(ldbend > fin_traitees) ldbend = fin_traitees;
			if(trmtstart < debut_traitees) trmtstart = debut_traitees;
			if(trmtend > fin_traitees) trmtend = fin_traitees;
			/* unite: point du tampon pre-trigger */
			trmtstart /= reduction; trmtend /= reduction; trmtdim = (int)(trmtend - trmtstart);
			ldbstart /= reduction; trmtend /= reduction; ldbdim = (int)(ldbend - ldbstart);
			if(VerifMonitEvts == 1) printf("(MonitEvtAffiche)          [%lld .. %lld[ filtre\n",trmtstart,trmtend);
			x = 2; y = 3;
			if(trmtdim > 0) {
				t0_trmt = (trmtstart / (int64)VoieTampon[voietrig].traitees->max) * (int64)VoieTampon[voietrig].traitees->max;
				pos_trmt = (int)(trmtstart - t0_trmt);
				if(VerifMonitEvts == 1) printf("(MonitEvtAffiche)          traite[0]= %lld, debut evt +%d\n",t0_trmt,pos_trmt);
				MonitTrmtAb6[voietrig][0] = (float)(((double)t0_trmt * (double)VoieTampon[voietrig].trmt[TRMT_PRETRG].ab6[1]) - (double)(MonitT0 * 1000.0));
				// faux: MonitTrmtAb6[voietrig][0] = MonitEvtAb6[voietrig][0] + (float)((double)(t0_trmt - t0_evt) * (double)VoieTampon[voietrig].trmt[TRMT_PRETRG].ab6[1]);
				GraphDataReplace(g,x,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,&(MonitTrmtAb6[voietrig][0]),VoieTampon[voietrig].traitees->max);
				rc = GraphDataReplace(g,y,GRF_FLOAT|GRF_YAXIS,VoieTampon[voietrig].traitees->t.rval,VoieTampon[voietrig].traitees->max);
				if(ldbdim) {
					base_traitee = MonitTraiteeOf7(voietrig,VoieTampon[voietrig].traitees,VoieTampon[voietrig].traitees->max,ldbstart,ldbdim);
					if(triggee->ref == TRMT_AU_VOL) GraphDataFloatOffset(g,y,triggee->base - base_traitee);
					else {
						ldbstart = evtstart + VoieEvent[voietrig].base_dep; ldbend = ldbstart + VoieEvent[voietrig].base_lngr;
						ldbdim = (int)(ldbend - ldbstart);
						base_brute = MonitTraiteeOf7(voietrig,VoieTampon[voietrig].brutes,VoieTampon[voietrig].brutes->max,ldbstart,ldbdim);
						GraphDataFloatOffset(g,y,base_brute - base_traitee);
					}
				}
				if(rc < 0) printf("! (MonitEvtAffiche) le pointage sur l'evenement traite a echoue\n");
				else {
					GraphDataUse(g,x,trmtdim);
					GraphDataUse(g,y,trmtdim);
					GraphDataRescroll(g,x,pos_trmt);
					GraphDataRescroll(g,y,pos_trmt);
					if(VerifMonitEvts == 1) printf("(MonitEvtAffiche)          Ab6[%d] tampon = %g + %d x %g, soit [%g .. %g[\n",
						voietrig,MonitTrmtAb6[voietrig][0],pos_trmt,MonitTrmtAb6[voietrig][1],
						MonitTrmtAb6[voietrig][0]+((float)pos_trmt*MonitTrmtAb6[voietrig][1]),
						MonitTrmtAb6[voietrig][0]+((float)(pos_trmt+trmtdim)*MonitTrmtAb6[voietrig][1]));
				}
			} else { GraphDataUse(g,x,0); GraphDataUse(g,y,0); }
		}
		#endif /* MONIT_EVT_TRAITE */
#ifndef WXWIDGETS
		// don't call graphics commands outside of a refresh event
		if(g == gEvtSolo) OpiumDisplay(gEvtSolo->cdr);
		else { OpiumRefresh(gEvtPlanche->cdr); PanelRefreshVars(pLectEvtNum); PanelRefreshVars(pLectEvtQui);  }
#endif
		MonitEvtAff = LectCntl.MonitEvtNum;
	}
	#ifdef REMETTRE_COMME_LECTDISPLAY
	/* le meme intervalle de temps pour tous ou le dernier evt de chacun? */
	evtduree = evtdim * VoieEvent[voietrig].horloge;
	origine = (evtstart - VoieTampon[voietrig].decal) * VoieTampon[voietrig].trmt[TRMT_AU_VOL].compactage;
	for(i=0, f=MonitFen; i<MonitFenNb; i++,f++) {
		if(f->affiche && (f->type == MONIT_EVENT) && (g = f->g)) {
			for(j=0, t=f->trace; j<f->nb; j++,t++) {
				int bolo,voie;
				bolo = t->bolo;
				voie = t->voie;
				evtstart = (origine / (int64)VoieTampon[voie].trmt[TRMT_AU_VOL].compactage) + VoieTampon[voie].decal;
				// MonitEvtAb6[voie][0] = ((float)evtstart * VoieEvent[voie].horloge) - (MonitT0 * 1000.0);
				// debut = VoieTampon[voie].brutes->t.sval + Modulo(evtstart,VoieTampon[voie].brutes->max);
				evtdim = evtduree / VoieEvent[voie].horloge;
				y = (2 * j) + 1;
				rc = GraphDataReplace(g,y,GRF_SHORT|GRF_YAXIS,debut,VoieTampon[voie].brutes->max);
				if(rc < 0) printf("! (MonitEvtAffiche[fenetre evt]) le pointage sur l'evenement a echoue\n");
				else GraphDataUse(g,y,evtdim);
			}
			if(f->axeY.i.min == f->axeY.i.max) GraphAxisReset(g,GRF_YAXIS);
			OpiumDisplay(g->cdr);
		}
	}
	#endif

	return(0);
}
/* ========================================================================== */
int MonitEvtDemande(Menu menu, MenuItem *item) {
	if(!EvtASauver) {
		OpiumError("Pas d'evenement en memoire");
		return(0);
	}
	if(OpiumReadInt(L_("Numero d'evenement"),&LectCntl.MonitEvtNum) == PNL_CANCEL) return(0);
	return(MonitEvtAffiche(0,menu,1));
}
/* ========================================================================== */
int MonitEvtPrecedent(Menu menu, MenuItem *item) {
	LectCntl.MonitEvtNum--; return(MonitEvtAffiche(0,menu,1));
}
/* ========================================================================== */
int MonitEvtSuivant(Menu menu, MenuItem *item) {
	LectCntl.MonitEvtNum++; return(MonitEvtAffiche(0,menu,1));
}
/* ========================================================================== */
int MonitEvtListe(Menu menu, MenuItem *item) {
	int vt,voietrig,n;
	float taux;
	TypeVoieArchivee *triggee;

	if(!EvtASauver) {
		OpiumError("Pas d'evenement en memoire");
		return(0);
	}
	if(!((tMonit->cdr)->displayed)) OpiumDisplay(tMonit->cdr);
	TermHold(tMonit);

	if(EvtASauver == 1)
		TermPrint(tMonit,L_("========== %s/ 1 evenement en memoire ==========\n"),DateHeure());
	else if(EvtASauver > 1)
		TermPrint(tMonit,L_("========== %s/ %d evenements en memoire: #%d a #%d ==========\n"),DateHeure(),
			EvtASauver,Evt[0].num,Evt[EvtASauver-1].num);
	else { TermRelease(tMonit); return(0); }
	for(n=0; n<EvtASauver; n++) {
//		                                nnnn vvvvvvvvvvvvvvvv -bbbb.b +aaaa.a ppppppppppp sss,mmmmmm 
		if(!(n % 23)) TermPrint(tMonit,"Numero Voie            Base +Amplitude Bruit      Debut     Date(s)  Montee Descente   Delai(ms)\n");
		vt = Evt[n].voie_evt; triggee = &(Evt[n].voie[vt]);
		if(!triggee) {
			TermPrint(tMonit,"%4d ! adresse de la voie triggee: nulle pour l'evt [%d]/[%d]\n",Evt[n].num,n,EvtASauver);
			continue;
		}
//		if(VerifSuiviEtatEvts) printf("(%s)        Voie trig = ajout#%d, pointe sur la voie #%d (%s)\n",__func__,vt+1,triggee->num,VoieManip[triggee->num].nom);
		voietrig = triggee->num;
		TermPrint(tMonit,"%4d %-16s %7.1f %+7.1f %5.1f %5d%06d %3d,%06d",
			Evt[n].num,VoieManip[voietrig].nom,triggee->base,triggee->amplitude,triggee->bruit,
			(int)triggee->debut/1000000,Modulo(triggee->debut,1000000),triggee->sec,triggee->msec);
		TermPrint(tMonit," %7.3f %8.3f   %9.3f\n",triggee->montee,triggee->descente,Evt[n].delai*1000.0);
	}
	taux = (float)(EvtASauver - 1)/
		((float)(Evt[EvtASauver-1].voie[0].sec - Evt[0].voie[0].sec) + ((float)(Evt[EvtASauver-1].voie[0].msec - Evt[0].voie[0].msec) / 1000000.0));
	TermPrint(tMonit,L_("========== %f evenements/seconde ==========\n"),taux);

	TermRelease(tMonit);
	return(0);
}
/* ========================================================================== */
void MonitHistosDynamique() {

	CoupureAmpl[0] = CoupureAmpl[1] = VoieManip[SettingsHistoVoie].def.trgr.minampl;
	if(MonitHamplY[0] == MonitHamplY[1])
		GraphAxisAutoRange(gBruitAmpl,GRF_YAXIS);
	else GraphAxisIntRange(gBruitAmpl,GRF_YAXIS,MonitHamplY[0],MonitHamplY[1]);

	CoupureMontee[0] = CoupureMontee[1] = VoieManip[SettingsHistoVoie].def.trgr.montee;
	if(MonitHmonteeY[0] == MonitHmonteeY[1])
		GraphAxisAutoRange(gBruitMontee,GRF_YAXIS);
	else GraphAxisIntRange(gBruitMontee,GRF_YAXIS,MonitHmonteeY[0],MonitHmonteeY[1]);

	Coup2DAmplX[0] = Coup2DAmplX[1] = VoieManip[SettingsHistoVoie].def.trgr.minampl;
	Coup2DAmplY[0] = SettingsMonteeMin; Coup2DAmplY[1] = SettingsMonteeMax;
	Coup2DMonteeX[0] = SettingsAmplMin; Coup2DMonteeX[0] = SettingsAmplMax;
	Coup2DMonteeY[0] = Coup2DMonteeY[1] = VoieManip[SettingsHistoVoie].def.trgr.montee;
	GraphDataIntGamma(g2D,2, /* a 'variabliser' */ MonitH2DY[0],MonitH2DY[1],0,DIM_LUT - 1);

}
/* ========================================================================== */
int MonitRefresh() {
	int voie;

	for(voie=0; voie<VoiesNb; voie++) {
		if(OpiumDisplayed(gDonnee[voie]->cdr))
			OpiumRefresh(gDonnee[voie]->cdr);
	}
	if(OpiumDisplayed(gBruitAmpl->cdr)) OpiumRefresh(gBruitAmpl->cdr);
	if(OpiumDisplayed(gBruitMontee->cdr)) OpiumRefresh(gBruitMontee->cdr);
	if(OpiumDisplayed(g2D->cdr)) OpiumRefresh(g2D->cdr);

	return(0);
}
/* ========================================================================== */
int MonitEcran() {
	int i,j,nb;
	TypeMonitFenetre *f;

	nb = MonitFenNb + 27;
	if(pMonitEcran) {
		if(PanelItemMax(pMonitEcran) < nb) {
			PanelDelete(pMonitEcran);
			pMonitEcran = 0;
		} else PanelErase(pMonitEcran);
	}
	if(pMonitEcran == 0) pMonitEcran = PanelCreate(nb);
	PanelTitle(pMonitEcran,"Affichages");

		PanelRemark(pMonitEcran,L_("=========== Statique ==========="));
//	j = PanelBool  (pMonitEcran,"Mode d'acquisition",&MonitAffModes);
//		PanelItemColors(pMonitEcran,j,SambaRougeVertJaune,2);
//	j = PanelBool  (pMonitEcran,"Parametres trigger",&MonitAffTrig);
//		PanelItemColors(pMonitEcran,j,SambaRougeVertJaune,2);
//	j = PanelBool  (pMonitEcran,"Etat d'acquisition",&MonitAffEtat);
//			PanelItemColors(pMonitEcran,j,SambaRougeVertJaune,2);
#ifdef MODULES_RESEAU
	j = PanelBool  (pMonitEcran,L_("Etat du reseau"),&MonitAffNet);
			PanelItemColors(pMonitEcran,j,SambaRougeVertJaune,2);
			PanelItemLngr(pMonitEcran,j,14);
#endif
		PanelRemark(pMonitEcran,L_("======== Periodiquement ========"));
		PanelFloat (pMonitEcran,L_("Intervalle (s)"),&MonitIntervSecs); /* concerne aussi les histos et le freq.metre */
	j = PanelListB (pMonitEcran,"Qualite",WndSupportNom,&FondPlots,8); PanelItemColors(pMonitEcran,j,SambaBlancNoir,2);
	j = PanelBool  (pMonitEcran,L_("Lignes de base"),&MonitAffBases);
		PanelItemColors(pMonitEcran,j,SambaRougeVertJaune,2);
		PanelItemLngr(pMonitEcran,j,14);
	j = PanelBool  (pMonitEcran,L_("Panel taux evt"),&MonitAffpTaux);
		PanelItemColors(pMonitEcran,j,SambaRougeVertJaune,2);
		PanelItemLngr(pMonitEcran,j,14);
	j = PanelBool  (pMonitEcran,L_("Graph.taux evt"),&MonitAffgTaux);
		PanelItemColors(pMonitEcran,j,SambaRougeVertJaune,2);
		PanelItemLngr(pMonitEcran,j,14);
	j = PanelBool  (pMonitEcran,L_("Graph.seuils"),&MonitAffgSeuils);
		PanelItemColors(pMonitEcran,j,SambaRougeVertJaune,2);
		PanelItemLngr(pMonitEcran,j,14);
	j = PanelBool  (pMonitEcran,L_("Synchro modulation"),&MonitSynchro);
		PanelItemColors(pMonitEcran,j,SambaRougeVertJaune,2);
		PanelItemLngr(pMonitEcran,j,14);
		PanelSepare(pMonitEcran,L_("Graphiques utilisateur"));
	for(i=0; i<MonitFenNb; i++) {
		j = PanelKeyB(pMonitEcran,MonitFen[i].titre,"non/oui/tjrs",&(MonitFen[i].affiche),6);
			PanelItemColors(pMonitEcran,j,SambaRougeVertJaune,3);
			PanelItemLngr(pMonitEcran,j,14);
	}
		PanelSepare(pMonitEcran,L_("Histogrammes bruit"));
		PanelRemark(pMonitEcran,L_(".... Amplitude ................."));
	j = PanelBool  (pMonitEcran,L_("Affichage"),&MonitHampl);
			PanelItemColors(pMonitEcran,j,SambaRougeVertJaune,2);
			PanelItemLngr(pMonitEcran,j,14);
		PanelInt   (pMonitEcran,L_("Bin mini"),&(MonitHamplY[0]));
		PanelInt   (pMonitEcran,L_("Bin maxi"),&(MonitHamplY[1]));
		PanelRemark(pMonitEcran,L_(".... Temps de montee ..........."));
	j = PanelBool  (pMonitEcran,L_("Affichage"),&MonitHmontee);
			PanelItemColors(pMonitEcran,j,SambaRougeVertJaune,2);
			PanelItemLngr(pMonitEcran,j,14);
		PanelInt   (pMonitEcran,L_("Bin mini"),&(MonitHmonteeY[0]));
		PanelInt   (pMonitEcran,L_("Bin maxi"),&(MonitHmonteeY[1]));
		PanelRemark(pMonitEcran,L_(".... Amplitude x Tps montee ...."));
	j = PanelBool  (pMonitEcran,L_("Affichage"),&MonitH2D);
			PanelItemColors(pMonitEcran,j,SambaRougeVertJaune,2);
			PanelItemLngr(pMonitEcran,j,14);
		PanelInt   (pMonitEcran,L_("Bin mini"),&MonitH2DY[0]);
		PanelInt   (pMonitEcran,L_("Bin maxi"),&MonitH2DY[1]);
		PanelRemark(pMonitEcran,L_("======== Selon arrivage ========"));
	j = PanelBool  (pMonitEcran,L_("Dernier evenement"),&MonitAffEvt);
			PanelItemColors(pMonitEcran,j,SambaRougeVertJaune,2);
			PanelItemLngr(pMonitEcran,j,14);

	if(OpiumExec(pMonitEcran->cdr) == PNL_CANCEL) return(0);
	MonitASauver = 1;
	MonitFenModifiees = 1;
	MonitIntervAff = (int)(MonitIntervSecs * 1000.0);
	GraphQualiteDefaut(FondPlots);

	if(pSettingsModes) {
		if(MonitAffModes) {
			if(OpiumDisplayed(pSettingsModes->cdr)) OpiumRefresh(pSettingsModes->cdr);
			else OpiumDisplay(pSettingsModes->cdr);
		} else if(OpiumDisplayed(pSettingsModes->cdr)) OpiumClear(pSettingsModes->cdr);
	}

	if(pSettingsTrigger) {
		if(MonitAffTrig) {
			if(OpiumDisplayed(pSettingsTrigger->cdr)) OpiumRefresh(pSettingsTrigger->cdr);
			else if(PanelItemNb(pSettingsTrigger)) OpiumDisplay(pSettingsTrigger->cdr);
		} else if(OpiumDisplayed(pSettingsTrigger->cdr)) OpiumClear(pSettingsTrigger->cdr);
	}

	if(bLectBases) {
		if(MonitAffBases) {
			if(OpiumDisplayed(bLectBases)) {
				if(pLectBaseValeurs) PanelRefreshVars(pLectBaseValeurs);
				if(gLectBaseNiveaux) OpiumRefresh(gLectBaseNiveaux->cdr);
				if(gLectBaseBruits) OpiumRefresh(gLectBaseBruits->cdr);
			} else OpiumDisplay(bLectBases);
		} else if(OpiumDisplayed(bLectBases)) OpiumClear(bLectBases);
	}
	if(pTauxDetaille) {
		if(MonitAffpTaux) OpiumDisplay(pTauxDetaille->cdr);
		else if(OpiumDisplayed(pTauxDetaille->cdr)) OpiumClear(pTauxDetaille->cdr);
	}
	if(gTauxDetaille) {
		if(MonitAffgTaux) OpiumDisplay(gTauxDetaille->cdr);
		else if(OpiumDisplayed(gTauxDetaille->cdr)) OpiumClear(gTauxDetaille->cdr);
	}
	if(gSeuils) {
		if(MonitAffgSeuils) OpiumDisplay(gSeuils->cdr);
		else if(OpiumDisplayed(gSeuils->cdr)) OpiumClear(gSeuils->cdr);
	}
	if(pLectEtat) {
		if(MonitAffEtat) OpiumDisplay(pLectEtat->cdr);
		else if(OpiumDisplayed(pLectEtat->cdr)) OpiumClear(pLectEtat->cdr);
	}

#ifdef MODULES_RESEAU
	if((SambaMode == SAMBA_MAITRE) && pAcquisEtat) {
		if(MonitAffNet) OpiumDisplay(pAcquisEtat->cdr);
		else if(OpiumDisplayed(pAcquisEtat->cdr)) OpiumClear(pAcquisEtat->cdr);
	}
#endif

	for(i=0, f=MonitFen; i<MonitFenNb; i++,f++) {
		if(f->affiche) MonitFenAffiche(f,1);
		else if((f->g)) { if(OpiumDisplayed((f->g)->cdr)) OpiumClear((f->g)->cdr); }
	}

	MonitHistosDynamique();
	if(MonitHampl) OpiumDisplay(gBruitAmpl->cdr);
	else if(OpiumDisplayed(gBruitAmpl->cdr)) OpiumClear(gBruitAmpl->cdr);

	if(MonitHmontee) OpiumDisplay(gBruitMontee->cdr);
	else if(OpiumDisplayed(gBruitMontee->cdr)) OpiumClear(gBruitMontee->cdr);

	if(MonitH2D) OpiumDisplay(g2D->cdr);
	else if(OpiumDisplayed(g2D->cdr)) OpiumClear(g2D->cdr);

	return(0);
}
/* ========================================================================== */
int MonitRecupere() {
	if(OpiumReadFile("Depuis le fichier",FichierPrefMonit,MAXFILENAME) == PNL_CANCEL) return(0);
	if(ArgScan(FichierPrefMonit,MonitDesc) <= 0) {
		OpiumError("Lecture sur '%s' impossible",FichierPrefMonit);
	}
	MonitIntervSecs = (float)MonitIntervAff / 1000.0;
	MonitASauver = 0;
	return(0);
}
/* ========================================================================== */
int MonitSauve() {
    if(SettingsSauveChoix) {
        if(OpiumReadFile("Sauver sur le fichier",FichierPrefMonit,MAXFILENAME) == PNL_CANCEL) return(0);
    }
	if(ArgPrint(FichierPrefMonit,MonitDesc) > 0) MonitASauver = 0;
	else OpiumError("Sauvegarde sur '%s' impossible",FichierPrefMonit);
	return(0);
}
/* ========================================================================== */
Menu mMonitDonnees,mMonitParVoie;
MenuItem iMonit[] = {
	{ "Parametres generaux",    MNU_FONCTION MonitGeneral },
	{ "Graphiques utilisateur", MNU_MENU   &mMonitDonnees },
	{ "Par voie",               MNU_MENU   &mMonitParVoie },
#ifdef BCP_EVTS
	{ "Parametres FFT",         MNU_PANEL  &pCalcSelPtNb },
#endif
	{ "Evenements",             MNU_MENU   &mMonitEvts },
	{ "Ecran",                  MNU_FONCTION MonitEcran },
/*	{ "Rafraichir",             MNU_FONCTION MonitRefresh },   ??? */
	{ "Preferences",            MNU_SEPARATION },
	{ "Recuperer",              MNU_FONCTION MonitRecupere },
	{ "Verifier",               MNU_PANEL  &pMonitDesc },
	{ "Sauver",                 MNU_FONCTION MonitSauve },
	{ "\0",                     MNU_SEPARATION },
	{ "Quitter",                MNU_RETOUR },
	MNU_END
};
MenuItem iMonitParVoie[] = {
	{ "Impression voie",     MNU_FONCTION MonitVoieImprime },
	{ "Affichage voie",      MNU_FONCTION MonitVoieAffiche },
	{ "Rafraichir",          MNU_FONCTION MonitVoieRafraichi },
	{ "Filtrage",            MNU_FONCTION MonitVoieFiltrage },
	{ "Tout effacer",        MNU_FONCTION MonitVoieEfface },
	{ "Quitter",             MNU_RETOUR },
	MNU_END
};
int NumeriseurRapport(Menu menu, MenuItem *item);
int CalcSpectreAutoRetrouve(Menu menu, MenuItem *item);
MenuItem iMonitBarre[] = {
	{ "Ecran",                  MNU_FONCTION  MonitEcran },
	{ "Graphiques utilisateur", MNU_MENU    &mMonitDonnees },
//	{ "Voie par voie",          MNU_MENU    &mMonitParVoie },
	{ "Evenements",             MNU_MENU    &mMonitEvts },
	{ "Spectres sauves",        MNU_FONCTION  CalcSpectreAutoRetrouve },
	{ "",                       MNU_SEPARATION },
	{ "Etat Repartiteurs",      MNU_FONCTION  RepartiteursStatus },
	{ "Etat Numeriseurs",       MNU_FONCTION  NumeriseurRapport },
	{ "Etat Detecteurs",        MNU_FORK    &bEtatDetecteurs }, // MNU_PANEL  &(pReglagesConsignes[LECT_CONS_ETAT]) },
	{ "Oscilloscope",           MNU_FONCTION  DetecteurOscillo },
	{ "Generateur",             MNU_FONCTION  DetecteurGegene },
	{ "",                       MNU_SEPARATION },
	{ "Parametres generaux",    MNU_FONCTION  MonitGeneral },
#ifdef BCP_EVTS
	{ "Parametres FFT",         MNU_PANEL   &pCalcSelPtNb },
#endif
	{ "Quitter",                MNU_RETOUR },
	MNU_END
};
MenuItem iMonitDonnees[] = {
	{ "Creer fenetre",        MNU_FONCTION MonitFenEdite },
	{ "Editer fenetre",       MNU_FONCTION MonitFenEdite },
	{ "Dupliquer fenetre",    MNU_FONCTION MonitFenEdite },
	{ "Organiser",            MNU_FONCTION MonitFenEdite },
	{ "Retirer fenetre",      MNU_FONCTION MonitFenRetire },
	{ "Lister fenetres",      MNU_FONCTION MonitFenListe },
	{ "Autoriser affichage",  MNU_FONCTION MonitFenAutorise },
	{ "Valeurs representees", MNU_FONCTION MonitFenZoom },
	{ "Ajuster l'axe Y",      MNU_FONCTION MonitFenAjusteY },
	{ "Changer detecteur",    MNU_FONCTION MonitFenDetecteur },
	{ "Memoriser position",   MNU_FONCTION MonitFenMemoToutes },
	{ "Fichiers",             MNU_SEPARATION },
	{ "Sauver description",   MNU_FONCTION MonitFenEcrit },
	{ "Lire description",     MNU_FONCTION MonitFenLit },
	{ "\0",                   MNU_SEPARATION },
	{ "Quitter",              MNU_RETOUR },
	MNU_END
};
MenuItem iMonitEvts[] = {
	{ "Evenement moyen voie", MNU_FONCTION MonitEvtMoyenVoie },
	{ "Evenement moyen bolo", MNU_FONCTION MonitEvtMoyenBolo },
	{ "Evenement No..",       MNU_FONCTION MonitEvtDemande },
	{ "Precedent",            MNU_FONCTION MonitEvtPrecedent },
	{ "Suivant",              MNU_FONCTION MonitEvtSuivant },
	{ "Liste evenements",     MNU_FONCTION MonitEvtListe },
	{ "Quitter",              MNU_RETOUR },
	MNU_END
};
/* ========================================================================== */
void MonitMenuBarre() { mMonitBarre = MenuLocalise(iMonitBarre); }
/* ========================================================================== */
void MonitInit() {
	int voie;
	int x,y; int i;
	Histo histo;
	H2D H2DCreateIntFloatToInt(int xmin, int xmax, int xnb, 
		float ymin, float ymax, int ynb);

/* Initialisation generale */
	MonitASauver = 0;
	LectCntl.MonitEvtNum = 0;
	for(voie=0; voie<VoiesGerees; voie++) {
		MonitEvtAb6[voie][0] = 0.0;  /* premier point de l'evenement */
		MonitEvtAb6[voie][1] = 1.0;  /* ou plutot, horloge (sera rectifie dans LectConstruitTampons) */
	}
	MonitEvtIndex[0] = 0; MonitEvtIndex[1] = 1;
	MonitBoloAvant = 0;
	MonitVoieVal0 = 0; MonitVoieValN = 0;
	strcpy(MonitNomTypes,MonitFenTypeName[0]);
	for(i=1; i<MONIT_FONCTION; i++) strcat(strcat(MonitNomTypes,"/"),MonitFenTypeName[i]); // MONIT_NBTYPES
	strcpy(MonitNouvelle,L_("(nouvelle fenetre)"));

/* Variables sujettes a modification par l'utilisateur (cf MonitDesc) */
	strcpy(UserFenetres,L_("Graphiques"));
	MonitHaut = 250;        /* hauteur standard d'un graphique */
	MonitLarg = 400;        /* largeur standard d'un graphique */
	MonitDuree = 10.0;      /* duree affichee (ms)             */
	MonitEvts = 0;          /* nb. evenements maxi affiches    */
	MonitSynchro = 1;
	MonitHampl = MonitHmontee = MonitH2D = 0;
	MonitHamplY[0] = 0; MonitHamplY[1] = 200;
	MonitHmonteeY[0] = 0; MonitHmonteeY[1] = 50000;
	MonitH2DY[0] = 0;
	MonitH2DY[1] = 5;
	for(voie=0; voie<VoiesNb; voie++) VoieManip[voie].affiche = 0;
	MonitAffModes = MonitAffTrig = 0;
	MonitAffBases = MonitAffpTaux = MonitAffgTaux = MonitAffgSeuils = MonitAffEtat = MonitAffNet = 0;
	MonitIntervAff = 1000;
	MonitAffEvt = 0;
	MonitChgtUnite = MONIT_ADU;
	ValeurADU = 1.0; /* valeur d'un ADU dans l'unite ci-dessus */
	/* les gains plus amonts dependent de la voie/bolo, et encore, meme la polar des ADC...!
	pos_gain = Bolo[bolo].tension[CMDE_GAIN_VOIE1 + voie];
	if((pos_gain >= 0) && (pos_gain < 6)) sscanf(ReglGains[pos_gain],"%g",&(Bolo[bolo].captr[cap].bb.gain[voie]));
	else Bolo[bolo].captr[cap].bb.gain[voie] = 1.0; */

/* Fenetres de trace des donnees brutes */
	MonitEditTraces = 1;
	MonitFenNb = 0; MonitFenNum = 0;
	strcpy(MonitFen[MonitFenNb].titre,MonitNouvelle);
	MonitFen[MonitFenNb + 1].titre[0] = '\0';
	for(i=0; i<MAXMONIT; i++) MonitFenTitre[i] = MonitFen[i].titre;
	MonitFenDefault(&MonitFenLue,L_("(a definir!)"));

/* Creation de l'interface */
	mMonit = MenuLocalise(iMonit);

	mMonitParVoie = MenuLocalise(iMonitParVoie);
	mMonitDonnees = MenuIntitule(iMonitDonnees,"Graphiques utilisateur");
	mMonitEvts = MenuIntitule(iMonitEvts,"Evenements");

	pMonitDesc = PanelDesc(MonitDesc,1);
	PanelTitle(pMonitDesc,"Preferences");
	PanelOnOK(pMonitDesc,MonitSauve,0);

//	pFichierMonitLect = PanelCreate(1);
//	PanelFile(pFichierMonitLect,"Depuis quel fichier",FichierPrefMonit,MAXFILENAME);
//	pFichierMonitEcri = PanelCreate(1);
//	PanelFile(pFichierMonitEcri,"Sur quel fichier",FichierPrefMonit,MAXFILENAME);

	pMonitGeneral = PanelCreate(5);
	PanelFloat(pMonitGeneral,L_("Duree affichee (ms)"),&MonitDuree);
	PanelKeyB (pMonitGeneral,L_("Unite"),MONIT_UNITE_CLES,&MonitChgtUnite,8);
	PanelFloat(pMonitGeneral,L_("Valeur d'un ADU en keV"),&MonitADUenkeV);
	PanelInt  (pMonitGeneral,L_("Hauteur graphique"),&MonitHaut);
	PanelInt  (pMonitGeneral,L_("Largeur graphique"),&MonitLarg);

	pMonitFenZoomData = PanelCreate(3);
	PanelTitle(pMonitFenZoomData,L_("Fenetre de donnees"));
	PanelFloat(pMonitFenZoomData,L_("Duree affichee (ms)"),&(MonitFenLue.axeX.r.max));
	PanelInt  (pMonitFenZoomData,L_("Valeur minimum"),&(MonitFenLue.axeY.i.min));
	PanelInt  (pMonitFenZoomData,L_("Valeur maximum"),&(MonitFenLue.axeY.i.max));

	pMonitFenZoomHisto = PanelCreate(3);
	PanelTitle(pMonitFenZoomHisto,L_("Histogramme"));
	PanelFloat(pMonitFenZoomHisto,L_("Bin minimum"),&(MonitFenLue.axeX.r.min));
	PanelFloat(pMonitFenZoomHisto,L_("Bin maximum"),&(MonitFenLue.axeX.r.max));
	PanelInt(pMonitFenZoomHisto,L_("#evts maxi"),&(MonitFenLue.axeY.i.max)); // MonitEvts);

	pMonitFenZoomFft = PanelCreate(4);
	PanelTitle(pMonitFenZoomFft,"FFT");
	PanelFloat(pMonitFenZoomFft,L_("Frequence min (0:auto)"),&(MonitFenLue.axeX.r.min));
	PanelFloat(pMonitFenZoomFft,L_("Frequence max (0:auto)"),&(MonitFenLue.axeX.r.max));
	PanelFloat(pMonitFenZoomFft,L_("Bruit minimum"),&(MonitFenLue.axeY.r.min));
	PanelFloat(pMonitFenZoomFft,L_("Bruit maximum"),&(MonitFenLue.axeY.r.max));

	pMonitFenZoomFctn = PanelCreate(3);
	PanelTitle(pMonitFenZoomFctn,L_("Fonction"));
	PanelInt  (pMonitFenZoomFctn,L_("Nb.evts affiches"),&(MonitFenLue.axeX.i.max));
	PanelFloat(pMonitFenZoomFctn,L_("Valeur minimum"),&(MonitFenLue.axeY.r.min));
	PanelFloat(pMonitFenZoomFctn,L_("Valeur maximum"),&(MonitFenLue.axeY.r.max));

	pMonitFenZoomBiplot = PanelCreate(4);
	PanelTitle(pMonitFenZoomBiplot,"Biplot");
	PanelFloat(pMonitFenZoomBiplot,"X min",&(MonitFenLue.axeX.r.min));
	PanelFloat(pMonitFenZoomBiplot,"X max",&(MonitFenLue.axeX.r.max));
	PanelFloat(pMonitFenZoomBiplot,"Y min",&(MonitFenLue.axeY.r.min));
	PanelFloat(pMonitFenZoomBiplot,"Y max",&(MonitFenLue.axeY.r.max));

	pMonitFenZoomH2D = PanelCreate(4);
	PanelTitle(pMonitFenZoomH2D,"Histo 2D");
	PanelFloat(pMonitFenZoomH2D,"Bin X minimum",&(MonitFenLue.axeX.r.min));
	PanelFloat(pMonitFenZoomH2D,"Bin X maximum",&(MonitFenLue.axeX.r.max));
	PanelFloat(pMonitFenZoomH2D,"Bin Y minimum",&(MonitFenLue.axeY.r.min));
	PanelFloat(pMonitFenZoomH2D,"Bin Y maximum",&(MonitFenLue.axeY.r.max));

	pMonitFenZoomEvts = PanelCreate(3);
	PanelTitle(pMonitFenZoomEvts,L_("Fenetre d'evenement"));
	PanelFloat(pMonitFenZoomEvts,L_("Duree affichee (ms)"),&(MonitFenLue.axeX.r.max));
	PanelInt  (pMonitFenZoomEvts,L_("Valeur minimum"),&(MonitFenLue.axeY.i.min));
	PanelInt  (pMonitFenZoomEvts,L_("Valeur maximum"),&(MonitFenLue.axeY.i.max));

	pMonitFenZoomMoyen = PanelCreate(2);
	PanelTitle(pMonitFenZoomMoyen,L_("Donnees moyennes"));
	PanelInt  (pMonitFenZoomMoyen,L_("Valeur minimum"),&(MonitFenLue.axeY.i.min));
	PanelInt  (pMonitFenZoomMoyen,L_("Valeur maximum"),&(MonitFenLue.axeY.i.max));

	pMonitTraces = 0;
	MonitFenModifiees = 0;
	pMonitFenAffiche = 0;
	pMonitFenChange = 0;

	pMonitFenLect = PanelCreate(1);
	PanelFile(pMonitFenLect,"Depuis quel fichier",MonitFenFichier,MAXFILENAME);
	pMonitFenEcri = PanelCreate(1);
	PanelFile(pMonitFenEcri,"Sur quel fichier",MonitFenFichier,MAXFILENAME);

	pMonitEcran = 0;

	tMonit = TermCreate(48,100,16384);

	pMonitFen = PanelCreate(13);
	MonitFenPanel(0,0,0);

	pMonitBoloVoie = PanelCreate(2);
	PanelList(pMonitBoloVoie,L_("Nom du bolo"),BoloNom,&BoloNum,DETEC_NOM);
	PanelList(pMonitBoloVoie,L_("Nom de la voie"),ModeleVoieListe,&ModlNum,MODL_NOM);

	pMonitVoiePrint = PanelCreate(4);
	PanelList(pMonitVoiePrint,L_("Nom du bolo"),BoloNom,&BoloNum,DETEC_NOM);
	PanelList(pMonitVoiePrint,L_("Nom de la voie"),ModeleVoieListe,&ModlNum,MODL_NOM);
	PanelInt (pMonitVoiePrint,L_("Temps initial"),&MonitVoieVal0);
	PanelInt (pMonitVoiePrint,L_("Temps final"),&MonitVoieValN);

	pMonitLimites1Voie = PanelCreate(5);

/* Donnees brutes */
	for(voie=0; voie<VoiesNb; voie++) gDonnee[voie] = gFiltree[voie] = 0;

/* Evenements */
	/* cree aussi dans lect (=gEvtPlanche) pour inclusion dans la planche de commande */
	gEvtSolo = GraphCreate(MonitLarg,MonitHaut,6);
	OpiumTitle(gEvtSolo->cdr,L_("Dernier evenement"));
	x = GraphAdd(gEvtSolo,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,&(MonitEvtAb6[0][0]),PointsEvt);
	y = GraphAdd(gEvtSolo,GRF_SHORT|GRF_YAXIS,VoieTampon[0].trmt[TRMT_AU_VOL].donnees.t.sval,PointsEvt);
	GraphDataName(gEvtSolo,x,"t(ms)");
	GraphDataRGB(gEvtSolo,y,GRF_RGB_GREEN);
#ifdef MONIT_EVT_TRAITE
	x = GraphAdd(gEvtSolo,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,&(MonitTrmtAb6[0][0]),PointsEvt);
	y = GraphAdd(gEvtSolo,GRF_FLOAT|GRF_YAXIS,VoieTampon[0].trmt[TRMT_PRETRG].donnees.t.rval,PointsEvt);
	GraphDataRGB(gEvtSolo,y,GRF_RGB_MAGENTA);
	GraphDataName(gEvtSolo,x,"t(ms)");
	GraphDataName(gEvtSolo,y,L_("filtree"));
#endif
#ifdef CARRE_A_VERIFIER
	x = GraphAdd(gEvtSolo,GRF_FLOAT|GRF_XAXIS,MomentEvt,5);
	y = GraphAdd(gEvtSolo,GRF_SHORT|GRF_YAXIS,AmplitudeEvt,5);
	GraphDataRGB(gEvtSolo,y,GRF_RGB_RED);
	GraphDataName(gEvtSolo,x,"t.trig");
	GraphDataName(gEvtSolo,y,"ADU.trig");
#endif
	OpiumPosition(gEvtSolo->cdr,50,100);
	GraphUse(gEvtSolo,0);
	GraphDataUse(gEvtSolo,x,5); GraphDataUse(gEvtSolo,y,5);

	gEvtMoyen = GraphCreate(MonitLarg,MonitHaut,2*VOIES_TYPES);
	OpiumTitle(gEvtMoyen->cdr,L_("Evenement moyen"));
	OpiumPosition(gEvtMoyen->cdr,50,100);
	GraphParmsSave(gEvtMoyen);
	GraphUse(gEvtMoyen,0);

	gPattern = GraphCreate(MonitLarg,MonitHaut,2*VOIES_TYPES);
	OpiumTitle(gPattern->cdr,"Pattern");
	OpiumPosition(gPattern->cdr,250,100);
	GraphParmsSave(gPattern);
	GraphUse(gPattern,0);

	/* suivi de pics */
	gPicsPositions = GraphCreate(400,MonitHaut,2*MAXPICS);
	OpiumTitle(gPicsPositions->cdr,L_("Positions des pics"));
	OpiumPosition(gPicsPositions->cdr,250,100);
	GraphParmsSave(gPicsPositions);
	GraphUse(gPicsPositions,0);
	gPicsSigmas = GraphCreate(400,MonitHaut,2*MAXPICS);
	OpiumTitle(gPicsSigmas->cdr,L_("Sigmas des pics"));
	OpiumPosition(gPicsSigmas->cdr,250,100+MonitHaut+40);
	GraphParmsSave(gPicsSigmas);
	GraphUse(gPicsSigmas,0);
	
	/* Histos */
	/* histo amplitude */
	histo = HistoCreateFloat(SettingsAmplMin,SettingsAmplMax,SettingsAmplBins);
/*--	histo->r = histo->g = histo->b = 0; */
	hdBruitAmpl = HistoAdd(histo,HST_INT);
	hdEvtAmpl = HistoAdd(histo,HST_INT);
	HistoDataSupportRGB(hdBruitAmpl,WND_Q_ECRAN,GRF_RGB_BLUE);
	HistoDataSupportRGB(hdEvtAmpl,WND_Q_ECRAN,GRF_RGB_YELLOW);
	gBruitAmpl = GraphCreate(MonitLarg,MonitHaut,5);
	OpiumTitle(gBruitAmpl->cdr,"Amplitude");
	HistoGraphAdd(histo,gBruitAmpl);
	GraphAxisFloatRange(gBruitAmpl,GRF_XAXIS,SettingsAmplMin,SettingsAmplMax);
	/* coupure amplitude */
	GraphAdd(gBruitAmpl,GRF_XAXIS|GRF_FLOAT,CoupureAmpl,2);
	y = GraphAdd(gBruitAmpl,GRF_YAXIS|GRF_INT,MonitHamplY,2);
	GraphDataRGB(gBruitAmpl,y,GRF_RGB_RED);
	OpiumPosition(gBruitAmpl->cdr,20,500);

	/* histo temps de montee */
	histo = HistoCreateFloat(SettingsMonteeMin,SettingsMonteeMax,SettingsMonteeBins);
	hdBruitMontee = HistoAdd(histo,HST_INT);
	hdEvtMontee = HistoAdd(histo,HST_INT);
	HistoDataSupportRGB(hdBruitMontee,WND_Q_ECRAN,GRF_RGB_BLUE);
	HistoDataSupportRGB(hdEvtMontee,WND_Q_ECRAN,GRF_RGB_YELLOW);
	gBruitMontee = GraphCreate(MonitLarg,MonitHaut,5);
	OpiumTitle(gBruitMontee->cdr,L_("Temps de montee (us)"));
	HistoGraphAdd(histo,gBruitMontee);
	GraphAxisFloatRange(gBruitMontee,GRF_XAXIS,SettingsMonteeMin,SettingsMonteeMax);
	/* coupure temps de montee */
	/* cMontee = CoupureCreate(HST_FLOAT,HST_INT);
		HistoYrangeInt(histo,0,500000);
		CoupureGraphAdd(gBruitMontee,cMontee); */
	GraphAdd(gBruitMontee,GRF_XAXIS|GRF_FLOAT,CoupureMontee,2);
	y = GraphAdd(gBruitMontee,GRF_YAXIS|GRF_INT,MonitHmonteeY,2);
	GraphDataRGB(gBruitMontee,y,GRF_RGB_RED);
	OpiumPosition(gBruitMontee->cdr,320,500);

	MonitLUTall = GraphLUTCreate(DIM_LUT,GRF_LUT_ALL);
	MonitLUTred = GraphLUTCreate(DIM_LUT,GRF_LUT_RED);

	h2D = H2DCreateFloatFloatToInt(
		SettingsAmplMin,SettingsAmplMax,SettingsAmplBins,
		SettingsMonteeMin,SettingsMonteeMax,SettingsMonteeBins);
	H2DLUT(h2D,MonitLUTred,DIM_LUT);
	g2D = GraphCreate(MonitLarg,MonitHaut,7);
	OpiumTitle(g2D->cdr,L_("Tps montee x Ampl"));
	H2DGraphAdd(h2D,g2D);
	/* coupure amplitude */
	GraphAdd(g2D,GRF_XAXIS|GRF_FLOAT,Coup2DAmplX,2);
	y = GraphAdd(g2D,GRF_YAXIS|GRF_FLOAT,Coup2DAmplY,2);
	GraphDataRGB(g2D,y,GRF_RGB_RED);
	/* coupure temps de montee */
	GraphAdd(g2D,GRF_XAXIS|GRF_FLOAT,Coup2DMonteeX,2);
	y = GraphAdd(g2D,GRF_YAXIS|GRF_FLOAT,Coup2DMonteeY,2);
	GraphDataRGB(g2D,y,GRF_RGB_RED);
	OpiumPosition(g2D->cdr,20,520);

/* Lecture de la configuration (exceptionnellement en local, pour Monit) */
	printf("= Lecture des affichages                dans '%s'\n",FichierPrefMonit);
	if(ArgScan(FichierPrefMonit,MonitDesc) <= 0) {
		if(CreationFichiers) ArgPrint(FichierPrefMonit,MonitDesc);
		else OpiumError("Lecture sur '%s' impossible",FichierPrefMonit);
	}
	MonitIntervSecs = (float)MonitIntervAff / 1000.0;
	if(ImprimeConfigs) {
		printf("#\n### Fichier: '%s' ###\n",FichierPrefMonit);
		ArgPrint("*",MonitDesc);
	}
}
/* ========================================================================== */
int MonitSetup() {
#ifdef NTUPLES_ONLINE
	int i,n,lim,nb,vm;
#endif

/* Doit aussi etre local, mais les bolos sont lus dans CmdesSetup... */
	SambaAjoutePrefPath(MonitFenFichier,UserFenetres);
	MonitFenDecode();
	if(InstalleSamba) return(1);
	
	pMonitFenDetecteur = PanelCreate(2);
	PanelList(pMonitFenDetecteur,L_("Detecteur abandonne"),BoloNom,&MonitBoloAvant,DETEC_NOM);
	PanelList(pMonitFenDetecteur,L_("Detecteur a utiliser"),BoloNom,&BoloNum,DETEC_NOM);

	if(PlotInit(0)) {
		int qual;
		strcpy(&(MonitColrFig[WND_Q_ECRAN][0]),"0000FFFF0000");
		strcpy(&(MonitColrFit[WND_Q_ECRAN][0]),"FFFFFFFF0000");
		strcpy(&(MonitColrFig[WND_Q_PAPIER][0]),"00000000BFFF");
		strcpy(&(MonitColrFit[WND_Q_PAPIER][0]),"BFFF00007FFF");
		for(qual=0; qual<WND_NBQUAL; qual++) PlotColors(qual,&(MonitColrFig[qual][0]),&(MonitColrFit[qual][0]));
		PlotBuildUI();
	} else OpiumError("Parametrisation des histos impossible");
	// printf("  > mPlot @%08X\n",(hexa)mPlot);

#ifdef NTUPLES_ONLINE
	if(CalcNtp == NTP_MODELE) {
		EvtEspace = PlotEspaceCree("Evenements",&NtUsed);
		n = 2; /* 'Date' et 'Delai' */
		if(BolosUtilises > 1) n++; /* 'Det' */
		nb = 0; for(vm=0; vm<ModeleVoieNb; vm++) if(ModeleVoie[vm].utilisee) nb++;
		n += (SAMBA_NTPVOIE_NB * nb); /* SAMBA_NTPVOIE_NB=9 variables par modele de voie dans un detecteur */
		lim = n;
		n++; /* pour la marque de fin de liste */
		printf("  > nTuple de %d variable%s, dont %d par type de voie\n",Accord1s(n),nb);
		Ntuple = (TypePlotVar *)malloc(n * sizeof(TypePlotVar));
		if(Ntuple) {
			char nom[80];
			i = 0;
			if((BolosUtilises > 1) && (i < n)) PlotVarCree(&(Ntuple[i++]),"Det","",&NtDet,EvtEspace);
			for(vm=0; vm<ModeleVoieNb; vm++) if(ModeleVoie[vm].utilisee) {
				if(i >= lim) break;
				strcpy(nom,L_("Ampl")); if(nb > 1) strcat(strcat(nom,"-"),ModeleVoie[vm].nom);
				PlotVarCree(&(Ntuple[i++]),nom,"ADU",&(NtAmpl[vm]),EvtEspace);
				if(i >= lim) break;
				strcpy(nom,L_("Brute")); if(nb > 1) strcat(strcat(nom,"-"),ModeleVoie[vm].nom);
				PlotVarCree(&(Ntuple[i++]),nom,"ADU",&(NtBrute[vm]),EvtEspace);
				if(i >= lim) break;
				//			strcpy(nom,"Max"); if(nb > 1) strcat(strcat(nom,"-"),ModeleVoie[vm].nom);
				//			PlotVarCree(&(Ntuple[i++]),nom,&(NtMax[vm]),EvtEspace);
				//			if(i >= lim) break;
				strcpy(nom,L_("Base")); if(nb > 1) strcat(strcat(nom,"-"),ModeleVoie[vm].nom);
				PlotVarCree(&(Ntuple[i++]),nom,"ADU",&(NtBase[vm]),EvtEspace);
				if(i >= lim) break;
				strcpy(nom,L_("Bruit")); if(nb > 1) strcat(strcat(nom,"-"),ModeleVoie[vm].nom);
				PlotVarCree(&(Ntuple[i++]),nom,"ADU",&(NtBruit[vm]),EvtEspace);
				if(i >= lim) break;
				strcpy(nom,L_("Montee")); if(nb > 1) strcat(strcat(nom,"-"),ModeleVoie[vm].nom);
				PlotVarCree(&(Ntuple[i++]),nom,"ms",&(NtMont[vm]),EvtEspace);
				if(i >= lim) break;
				strcpy(nom,L_("Descente")); if(nb > 1) strcat(strcat(nom,"-"),ModeleVoie[vm].nom);
				PlotVarCree(&(Ntuple[i++]),nom,"ms",&(NtDesc[vm]),EvtEspace);
				if(i >= lim) break;
				strcpy(nom,L_("Energ")); if(nb > 1) strcat(strcat(nom,"-"),ModeleVoie[vm].nom);
				PlotVarCree(&(Ntuple[i++]),nom,"",&(NtEnerg[vm]),EvtEspace);
				if(i >= lim) break;
				strcpy(nom,L_("Seuil")); if(nb > 1) strcat(strcat(nom,"-"),ModeleVoie[vm].nom);
				PlotVarCree(&(Ntuple[i++]),nom,"ADU",&(NtSeuil[vm]),EvtEspace);
				if(i >= lim) break;
				strcpy(nom,L_("Decal")); if(nb > 1) strcat(strcat(nom,"-"),ModeleVoie[vm].nom);
				PlotVarCree(&(Ntuple[i++]),nom,"ms",&(NtDecal[vm]),EvtEspace);
			}
			if(i < lim) PlotVarCree(&(Ntuple[i++]),L_("Date"),"s",&NtDate,EvtEspace);
			if(i < lim) PlotVarCree(&(Ntuple[i++]),L_("Delai"),"s",&NtDelai,EvtEspace);
			Ntuple[i].nom = "\0"; // (i < n) oblige, sinon pas de fin de table, et plantage nanopaw
			PlotVarsDeclare(Ntuple);
			/* i = 0;
			 printf("  * Variables d'analyse:");
			 while(PlotVarList[i][0]) {
				if(!(i % 8)) printf("\n    . ");
				else if(i) printf(", ");
				printf("%s",PlotVarList[i]);
				i++;
			 }
			 printf("\n"); */
			/* printf("  * Ntuple complet:\n");
			 i = 0;
			 while(Ntuple[i].nom[0]) {
				printf("    %2d [%08X]: %s\n",i+1,Ntuple[i].adrs,Ntuple[i].nom);
				i++;
			 } */
			PlotVarsInterface();
		}
	} else MenuItemHide(mCalc,(IntAdrs)"Graphiques evenements");
#endif
	return(1);
}
/* ========================================================================== */
int MonitExit() {
	if(!SambaSauve(&MonitASauver,L_("affichage general"),-1,0,MonitSauve)) return(0);
	return(SambaSauve(&MonitFenModifiees,L_("graphiques utilisateur"),0,1,MonitFenEcrit));
}
