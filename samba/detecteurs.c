#ifdef macintosh
#pragma message(__FILE__)
#endif
#define DETECTEURS_C
#define BRANCHE_CMDES

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <environnement.h>

#include <claps.h>
#include <decode.h>
#include <dateheure.h>
#include <timeruser.h>
#include <opium.h>

#include <samba.h>
#include <repartiteurs.h>
#include <numeriseurs.h>
#include <cablage.h>
#include <detecteurs.h>
#include <objets_samba.h>
#include <monit.h>
#include <autom.h>

#define VERNIER_SYMETRIQUE
#define D2_MAXI 10000
#define MARRON 0xBFFF,0x5FFF,0

/* Menu avec distance entre les boutons
#define CMDE_UPDATE_STD 1
#define CMDE_UPDATE_MEM 3
#define CMDE_UPDATE_SAVE 5
*/
#define CMDE_UPDATE_STD 1
#define CMDE_UPDATE_MEM 2
#define CMDE_UPDATE_SAVE 3

typedef enum { DETEC_VIDE = 0, DETEC_REGISTRE, DETEC_REGLAGE  } DETEC_VARIABLE;
typedef enum { DETEC_STD = 0, DETEC_REEL } DETEC_NATURE;
typedef enum { DETEC_SET = 0, DETEC_GET  } DETEC_SENS;

static char *DetecteurParmsTitre[DETEC_PARM_NBNATURES+2] = {
	"Definition des evenements",
	"Traitement des donnees",
	"Preparation trigger",
	"Recherche des evenements",
	"Regulation du taux",
	"Voies logicielles",
	"Parametres generaux",
	"Environnement",
	"Definition generale"
	"\0"
};
static char *DetecteurParmsUser[DETEC_NBDEFS+1] = {
	"evenement", "trmt-au-vol", "trmt-sur-tampon", "trigger", "regulation",
	"pseudo", "parms", "traitement", "registres", "reglages", "\0"
};

static char ParmASauver = '\0';
static char ParmAJeter = '\1';

typedef struct {
	void (*ref_onglet)(int cap);
	int  (*standardise)(Panel panel, void *val);
	int  (*applique)(Panel panel, void *val);
} TypeDetecteurCB;
static TypeDetecteurCB DetecteurParmsCallback[DETEC_PARM_NBNATURES];

static TypeFigZone DetecteurParmsCadre[DETEC_PARM_NBNATURES+1],DetecteurParmsCadreSoft,DetecteurParmsCadreEnvr;
static int DetecteurParmsBordDroit[DETEC_PARM_NBNATURES+1];
static char DetecteurParmsRassembles;
static void DetecteurParmsOngletModule(int cap, DETEC_PARMS_NATURE nature);
static void DetecteurParmsOngletVoie(int cap);
static Menu mDetecParms,mDetecParmsGenl,mDetecteurParmsGenlApply,mDetecteurParmsSoftApply;

static TypeFigZone DetecCadreRegl,DetecCadreMesr,OscilloCadreMain,OscilloCadreTrgr;

static char DetecteurFichierModeles[MAXFILENAME];
static char DetecDebugStd;

static TypeSignalPhysique ModelePhysDefaut = {
	"physique", 1000.0, 0.0, -1.0, 10.0, 0, 1.0, 100.0, 300.0, 0.5
};
static TypeVoieModele ModeleVoieDefaut = {
//	"signal", 0, 0, 1000.0, 0.0, -1.0, 10.0, 10.0, 50.0, 25.0, 75.0, 10.0, 90.0, { 0.0, 0.0 }, { 0.0, 0.0 }, 0, 1.0, 100.0, 300.0, 0.5, 1000.0, 8000.0 
	"signal", 0, 0, 1000.0, 0.0, -1.0, 10.0, 10.0, 50.0, 25.0, 75.0, 10.0, 90.0, 0.0, 0.0, 0.0, 0.0, 0, 1.0, 100.0, 300.0, 0.5, 1000.0, 8000.0 
};

static float ReglSensibilite;
static char  ReglUsage;
static char  Rbolo;
static ArgDesc CaptAutreDesc[] = {
	{ "usage",       DESC_KEY ARCH_BUFF_CLES, &ReglUsage,       "Utilisation finale de la voie ("ARCH_BUFF_CLES")" },
	{ "sensibilite", DESC_FLOAT               &ReglSensibilite, "nV/keV" },
	{ "Rdetecteur",  DESC_FLOAT               &Rbolo,           "Mohms" },
	DESC_END
};

static ArgDesc CaptEvtsDesc[] = {
	{ "duree",              DESC_FLOAT &CaptParam.def.evt.duree,       "Duree evenement (ms)" },
	{ "delai",              DESC_FLOAT &CaptParam.def.evt.delai,       "Decalage evenement (ms)" },
	{ "interv",             DESC_FLOAT &CaptParam.def.evt.interv,      "Fenetre de recherche (ms)" },
	{ "pre-trigger",        DESC_FLOAT &CaptParam.def.evt.pretrigger,  "Pre-trigger (%)" },
	{ "temps-mort",         DESC_FLOAT &CaptParam.def.evt.tempsmort,   "Temps mort avant nouvelle recherche (milllisec.) (=post-trigger si <0)" },
	{ "base.depart",        DESC_FLOAT &CaptParam.def.evt.base_dep,    "Position du debut de ligne de base dans le pre-trigger (%)" },
	{ "base.arrivee",       DESC_FLOAT &CaptParam.def.evt.base_arr,    "Position de la fin de ligne de base dans le pre-trigger (%)" },
	{ "temps.depart",       DESC_FLOAT &CaptParam.def.evt.ampl_10,     "Debut du temps de montee (%)" },
	{ "temps.arrivee",      DESC_FLOAT &CaptParam.def.evt.ampl_90,     "Fin du temps de montee (%)" },
	{ "rapide",             DESC_NONE },
	{ "phase1.t0",	    	DESC_FLOAT &CaptParam.def.evt.phase[0].t0, "Debut integrale rapide (ms)" },
	{ "phase1.dt",	    	DESC_FLOAT &CaptParam.def.evt.phase[0].dt, "Duree integrale rapide (ms)" },
	{ "phase2.t0",	    	DESC_FLOAT &CaptParam.def.evt.phase[1].t0, "Debut integrale longue (ms)" },
	{ "phase2.dt",	    	DESC_FLOAT &CaptParam.def.evt.phase[1].dt, "Duree integrale longue (ms)" },
	//	{ "base.lissage",       DESC_INT   &CaptParam.def.evt.a_lisser,    "Dimension du lissage pour calcul ligne de base" },
	{ "base.lissage",       DESC_NONE },
	{ "base",               DESC_NONE },
#ifdef RL
	{ "dimtemplate",		DESC_NONE },
	{ "montee",		        DESC_NONE },
	{ "descente1",		    DESC_NONE },
	{ "descente2",		    DESC_NONE },
	{ "facteur2",		    DESC_NONE },
	{ "template.dim",		        DESC_INT     &CaptParam.def.evt.dimtemplate, "Dimension du template (points)" },
	{ "template.temps.montee",		DESC_FLOAT   &CaptParam.def.evt.montee,      "temps de montee (ms)" },
	{ "template.temps.descente1",	DESC_FLOAT   &CaptParam.def.evt.descente1,   "temps de la descente #1 (ms)" },
	{ "template.temps.descente2",	DESC_FLOAT   &CaptParam.def.evt.descente2,   "temps de la descente #2 (ms)" },
	{ "template.facteur.descente2",	DESC_FLOAT   &CaptParam.def.evt.facteur2,    "gain de la descente #2" },
#endif
	{ "moyen.min",		    DESC_INT   &CaptParam.def.evt.min_moyen,             "Amplitude min pour calcul evt moyen" },
	{ "moyen.max",		    DESC_INT   &CaptParam.def.evt.max_moyen,             "Amplitude max pour calcul evt moyen" },
	{ "affichage.min",      DESC_INT   &CaptParam.min,                       "Signal recu minimum (ADU) [pour l'affichage]" },
	{ "affichage.max",		DESC_INT   &CaptParam.max,                       "Signal recu maximum (ADU) [pour l'affichage]" },
	DESC_END
};

static ArgDesc CaptTrmtDesc[] = {
	{ "dim.tampon",			DESC_INT   &CaptParam.duree.traitees,             "Dimension du tampon resultat des traitements" },
	{ "dim.lissage",		DESC_INT   &CaptParam.duree.mesureLdB,            "Dimension de l'evaluation de la ligne de base" },
	{ "periodes.pattern",   DESC_FLOAT &CaptParam.duree.periodes,             "Nb.periodes pour soustraction du pattern" },
	{ "moyenne.archive",    DESC_INT   &CaptParam.duree.post_moyenne,         "Facteur de moyennage avant la sauvegarde des evenements" },
	{ "au-vol",			    DESC_KEY TrmtTypeCles, 
	                                   &CaptParam.def.trmt[TRMT_AU_VOL].type,     "Traitement au vol (neant/demodulation/filtrage/invalidation)" },
    { "au-vol.int",		    DESC_INT   &CaptParam.def.trmt[TRMT_AU_VOL].parm,     "Parametre (int) du traitement au vol" },
	{ "au-vol.texte",       DESC_TEXT   CaptParam.def.trmt[TRMT_AU_VOL].texte,    "Parametre (char[]) du traitement au vol" },
    { "au-vol.gain",		DESC_INT   &CaptParam.def.trmt[TRMT_AU_VOL].gain,     "Gain logiciel au vol" },
	{ "au-vol.execution",   DESC_KEY "au_vol/sur_tampon", &CaptParam.exec,    "La demodulation peut etre differee (au_vol/sur_tampon)" },
	{ "sur-tampon",		    DESC_KEY TrmtTypeCles, 
									  &CaptParam.def.trmt[TRMT_PRETRG].type,      "Traitement pre-trigger (neant/demodulation/filtrage/invalidation)" },
    { "sur-tampon.int",     DESC_INT  &CaptParam.def.trmt[TRMT_PRETRG].parm,      "Parametre (int) du traitement pre-trigger" },
	{ "sur-tampon.texte",   DESC_TEXT  CaptParam.def.trmt[TRMT_PRETRG].texte,     "Parametre (char[]) du traitement pre-trigger" },
	{ "sur-tampon.gain",    DESC_INT  &CaptParam.def.trmt[TRMT_PRETRG].gain,      "Gain logiciel pre-trigger" },
	DESC_END
};

static ArgDesc CaptTvolDesc[] = {
	{ "type",			    DESC_KEY TrmtTypeCles, 
		                              &CaptParam.def.trmt[TRMT_AU_VOL].type,      "Traitement au vol (neant/demodulation/filtrage/invalidation)" },
    { "parametre",		    DESC_INT  &CaptParam.def.trmt[TRMT_AU_VOL].parm,      "Parametre (int) du traitement au vol" },
	{ "texte",              DESC_TEXT  CaptParam.def.trmt[TRMT_AU_VOL].texte,     "Parametre (char[]) du traitement au vol" },
    { "gain",	         	DESC_INT  &CaptParam.def.trmt[TRMT_AU_VOL].gain,      "Gain logiciel au vol" },
	{ "execution",          DESC_KEY "au_vol/sur_tampon", &CaptParam.exec,    "La demodulation peut etre differee (au_vol/sur_tampon)" },
	DESC_END
};

static ArgDesc CaptPrepDesc[] = {
	{ "type",		        DESC_KEY TrmtTypeCles, 
	                                   &CaptParam.def.trmt[TRMT_PRETRG].type,     "Traitement pre-trigger (neant/demodulation/filtrage/invalidation)" },
    { "parametre",          DESC_INT   &CaptParam.def.trmt[TRMT_PRETRG].parm,     "Parametre (int) du traitement pre-trigger" },
	{ "texte",              DESC_TEXT   CaptParam.def.trmt[TRMT_PRETRG].texte,    "Parametre (char[]) du traitement pre-trigger" },
    { "gain",               DESC_INT   &CaptParam.def.trmt[TRMT_PRETRG].gain,     "Gain logiciel pre-trigger" },
	{ "dim.tampon",			DESC_INT   &CaptParam.duree.traitees,             "Dimension du tampon resultat des traitements" },
	{ "dim.lissage",		DESC_INT   &CaptParam.duree.mesureLdB,            "Dimension de l'evaluation de la ligne de base" },
	{ "periodes.pattern",   DESC_FLOAT &CaptParam.duree.periodes,             "Nb.periodes pour soustraction du pattern" },
	{ "moyenne.archive",    DESC_INT   &CaptParam.duree.post_moyenne,         "Facteur de moyennage avant la sauvegarde des evenements" },
#ifdef RL
	{ "template",           DESC_KEY TrmtTemplateCles, 
	                                   &CaptParam.def.trmt[TRMT_PRETRG].template, "Filtrage par template (neant/database/analytique)" },
#endif
	DESC_END
};

static ArgDesc CaptTrgrDesc[] = {
	{ "type",             DESC_KEY TrgrTypeCles,
	                               &CaptParam.def.trgr.type,         "Type de recherche d'evenement (neant/seuil/front/porte/bruit)" },
	{ "signe",            DESC_KEY TRGR_SENS_CLES,
	                               &CaptParam.def.trgr.sens,         "Sens de la variation attendue ("TRGR_SENS_CLES")" },
	{ "origine",          DESC_KEY TrgrOrigineCles,
		                           &CaptParam.def.trgr.origine,      "Origine du trigger (interne/deporte/fpga)" },
	{ "regulation",       DESC_BOOL  &CaptParam.def.trgr.regul_demandee, "Autorisation de reguler automatiquement" },
	{ "temps.mort",       DESC_NONE }, // DESC_KEY TRGR_REPRISE_CLES, &CaptParam.def.trgr.reprise, "Tactique de gestion du temps mort apres evenement" },
	{ "reprise",          DESC_KEY TRGR_REPRISE_CLES,
		                           &CaptParam.def.trgr.reprise,      "Tactique de gestion du temps mort apres evenement ("TRGR_REPRISE_CLES")" },
	{ "amplitude",        DESC_NONE },
	{ "amplitude.min",    DESC_FLOAT &CaptParam.def.trgr.minampl,    "Amplitude minimum pour impulsions positives (ADU)" },
	{ "amplitude.max",    DESC_FLOAT &CaptParam.def.trgr.maxampl,    "Amplitude maximum pour impulsions negatives (ADU)" },
	{ "montee",           DESC_FLOAT &CaptParam.def.trgr.montee,     "Temps de montee minimum (microsecs.)" },
	{ "duree",            DESC_FLOAT &CaptParam.def.trgr.porte,      "Duree d'evenement minimum (microsecs.)" },
	{ DESC_COMMENT "Coupures avant sauvegarde" },
	{ "coupures",         DESC_BOOL  &CaptParam.def.trgr.coupures,   "Vrai si coupures actives" },
	{ "veto",             DESC_BOOL  &CaptParam.def.trgr.veto,       "Vrai si voie en veto" },
	{ "ampl.rejette",     DESC_BOOL  &CaptParam.def.trgr.ampl_inv,   "Inverse la condition de coupure sur l'amplitude" },
	{ "underflow",        DESC_FLOAT &CaptParam.def.trgr.underflow,  "Plancher pour les amplitudes (ADU)" },
	{ "overflow",         DESC_FLOAT &CaptParam.def.trgr.overflow,   "Plafond pour les amplitudes (ADU)" },
	{ "montee.rejette",   DESC_BOOL  &CaptParam.def.trgr.mont_inv,   "Inverse la condition de coupure sur le temps de montee" },
	{ "montee.min",       DESC_FLOAT &CaptParam.def.trgr.montmin,    "Temps de montee minimum (millisecs)" },
	{ "montee.max",       DESC_FLOAT &CaptParam.def.trgr.montmax,    "Temps de montee maximum (millisecs)" },
	{ "bruit.rejette",    DESC_BOOL  &CaptParam.def.trgr.bruit_inv,  "Inverse la condition de coupure sur le bruit" },
	{ "bruit.max",        DESC_FLOAT &CaptParam.def.trgr.maxbruit,   "Bruit maximum sur la ligne de base (ADU)" },
	{ "alpha.amplitude",  DESC_INT   &CaptParam.def.trgr.alphaampl,  "Seuil pour un temps mort special alpha (ADU)" },
	{ "alpha.duree",      DESC_FLOAT &CaptParam.def.trgr.alphaduree, "Temps mort special alpha (ms)" },
	DESC_END
};

static ArgDesc CaptRgulDesc[] = {
	{ "Type",              DESC_KEY "neant/taux/intervalles", 
		&CaptParam.def.rgul.type, "Type de regulation (neant/taux/intervalles)" },
	{ "Plancher.min",      DESC_FLOAT   &CaptParam.def.rgul.mininf,              "Plancher pour le seuil min" },
	{ "Plafond.min",       DESC_FLOAT   &CaptParam.def.rgul.minsup,              "Plafond  pour le seuil min" },
	{ "Plancher.max",      DESC_FLOAT   &CaptParam.def.rgul.maxinf,              "Plancher pour le seuil max" },
	{ "Plafond.max",       DESC_FLOAT   &CaptParam.def.rgul.maxsup,              "Plafond  pour le seuil max" },
	{ DESC_COMMENT 0 },
	{ DESC_COMMENT "Regulation par le taux d'evenements" },
	{ DESC_COMMENT 0 },
	{ "Active.1",          DESC_BOOL    &CaptParam.echelle[0].active,        "Echelle de temps #1 en service" },
	{ "Echelle.1",         DESC_INT     &CaptParam.echelle[0].duree,         "Echelle de temps #1 (mn)" },
	{ "Nb.1.min",          DESC_INT     &CaptParam.echelle[0].min.nb,        "Nombre d'evt minimum a l'echelle #1" },
	{ "Action.1.min.type", DESC_KEY TrmtRegulCles, &CaptParam.echelle[0].min.action, "Type action si nb1 < min1" },
	{ "Action.1.min.parm", DESC_FLOAT   &CaptParam.echelle[0].min.parm,      "Valeur action si nb1 < min1" },
	{ "Nb.1.max",          DESC_INT     &CaptParam.echelle[0].max.nb,        "Nombre d'evt maximum a l'echelle #1" },
	{ "Action.1.max.type", DESC_KEY TrmtRegulCles, &CaptParam.echelle[0].max.action, "Type action si nb1 > max1" },
	{ "Action.1.max.parm", DESC_FLOAT   &CaptParam.echelle[0].max.parm,      "Valeur action si nb1 > max1" },
	{ DESC_COMMENT 0 },
	{ "Active.2",          DESC_BOOL    &CaptParam.echelle[1].active,        "Echelle de temps #2 en service" },
	{ "Echelle.2",         DESC_INT     &CaptParam.echelle[1].duree,         "Echelle de temps #2 (mn)" },
	{ "Nb.2.min",          DESC_INT     &CaptParam.echelle[1].min.nb,        "Nombre d'evt minimum a l'echelle #2" },
	{ "Action.2.min.type", DESC_KEY TrmtRegulCles, &CaptParam.echelle[1].min.action, "Type action si nb2 < min2" },
	{ "Action.2.min.parm", DESC_FLOAT   &CaptParam.echelle[1].min.parm,      "Valeur action si nb2 < min2" },
	{ "Nb.2.max",          DESC_INT     &CaptParam.echelle[1].max.nb,        "Nombre d'evt maximum a l'echelle #2" },
	{ "Action.2.max.type", DESC_KEY TrmtRegulCles, &CaptParam.echelle[1].max.action, "Type action si nb2 > max2" },
	{ "Action.2.max.parm", DESC_FLOAT   &CaptParam.echelle[1].max.parm,      "Valeur action si nb2 > max2" },
	{ DESC_COMMENT 0 },
	{ "Active.3",          DESC_BOOL    &CaptParam.echelle[2].active,        "Echelle de temps #3 en service" },
	{ "Echelle.3",         DESC_INT     &CaptParam.echelle[2].duree,         "Echelle de temps #3 (mn)" },
	{ "Nb.3.min",          DESC_INT     &CaptParam.echelle[2].min.nb,        "Nombre d'evt minimum a l'echelle #3" },
	{ "Action.3.min.type", DESC_KEY TrmtRegulCles, &CaptParam.echelle[2].min.action, "Type action si nb3 < min3" },
	{ "Action.3.min.parm", DESC_FLOAT   &CaptParam.echelle[2].min.parm,      "Valeur action si nb3 < min3" },
	{ "Nb.3.max",          DESC_INT     &CaptParam.echelle[2].max.nb,        "Nombre d'evt maximum a l'echelle #3" },
	{ "Action.3.max.type", DESC_KEY TrmtRegulCles, &CaptParam.echelle[2].max.action, "Type action si nb3 > max3" },
	{ "Action.3.max.parm", DESC_FLOAT   &CaptParam.echelle[2].max.parm,      "Valeur action si nb3 > max3" },
	{ DESC_COMMENT 0 },
	{ DESC_COMMENT "Regulation par intervalles" },
	{ DESC_COMMENT 0 },
	{ "Interv.nb",         DESC_INT     &CaptParam.interv.nb,             "Nombre d'intervalles" },
	{ "Interv.duree",      DESC_FLOAT   &CaptParam.interv.duree,          "Longueur (s)" },
	{ "Interv.ajuste",     DESC_FLOAT   &CaptParam.interv.ajuste,         "Ajustement seuil (ADU)" },
	{ "Interv.evt",        DESC_FLOAT   &CaptParam.interv.seuil,          "Facteur decision" },
	{ "Interv.min",        DESC_INT     &CaptParam.interv.max,            "Delai mini (nb.interv.)" },
	{ "Interv.incr",       DESC_FLOAT   &CaptParam.interv.incr,           "Increment si bruit (ADU)" },
	DESC_END
};
static ArgDesc *CaptDesc[DETEC_PARM_RGUL+1] = {
	CaptEvtsDesc, CaptTvolDesc, CaptPrepDesc, CaptTrgrDesc, CaptRgulDesc
};

// #define DETECTR_NL
#ifdef  DETECTR_NL
#define DETEC_CTLG "liste/ci_dessous"
#define DETEC_MODL "fichier/ci_dessous"
typedef enum {
	DETEC_CTLG_LISTE = 0,
	DETEC_CTLG_ONLINE,
	DETEC_CTLG_MODESNB
} DETEC_CTLG_MODES;
typedef enum {
	DETEC_MODL_FICHIER = 0,
	DETEC_MODL_ONLINE,
	DETEC_MODL_MODESNB
} DETEC_MODL_MODES;

static char DetectrCtlgMode,DetectrModlMode;
static TypeCapteur DetectrCaptrLu;

static ArgDesc DetectrPosDesc[] = {
	{ "position", DESC_SHORT           &DetecteurLu.pos,     "Position dans le support des detecteurs" },
	{ "fichier",  DESC_STR(MAXFILENAME) DetecteurLu.fichier, "Fichier desciptif du detecteur" },
	DESC_END
};
static ArgStruct DetectrListeAS = { (void *)Bolo, (void *)&DetecteurLu, sizeof(TypeDetecteur), DetectrPosDesc };
static ArgDesc DetectrListeDesc[] = {
	{ "liste",    DESC_STRUCT_SIZE(BoloNb,DETEC_MAX) &DetectrListeAS, 0 },
	DESC_END
};
static ArgDesc DetectrModlFicDesc[] = {
	{ "type",     DESC_LISTE ModeleDetListe, &DetecteurLu.type, "nom du modele" },
	DESC_END
};
static ArgDesc DetectrModlLocDesc[] = {
	{ "physique",     DESC_STRUCT_SIZE(ModelePhysNb,PHYS_TYPES)                   &ModelePhysAS, 0 },
	{ "voies",        DESC_STRUCT_SIZE(ModeleVoieNb,VOIES_TYPES)                  &ModeleVoieAS, 0 },
	{ "pseudos",      DESC_STRUCT_SIZE(ModeleDetLu.nbsoft,DETEC_SOFT_MAX)         &ModeleDetSoftAS,  0 },
	{ "reglages",     DESC_STRUCT_SIZE(ModeleDetLu.nbregl,DETEC_REGL_MAX)         &ModeleDetReglAS,  0 },
	{ "associes",     DESC_STR_SIZE(MODL_NOM,ModeleDetLu.nbassoc,DETEC_ASSOC_MAX)  ModeleDetLu.nom_assoc,   0 },
	{ "regeneration", DESC_BOOL                                                   &ModeleDetLu.avec_regen, 0 },
	{ "duree_razfet", DESC_INT                                                    &ModeleDetLu.duree_raz,   0 },
	DESC_END
};
static ArgDesc *DetectrModlDesc[] = { DetectrModlFicDesc, DetectrModlLocDesc };
static ArgDesc DetectrDefDesc[] = {
	{ DESC_COMMENT 0 },
	{ "modelisation", DESC_KEY DETEC_MODL, &DetectrModlMode, "emplacement de la description du modele ("DETEC_MODL")" },
	{ 0,              DESC_VARIANTE(DetectrModlMode) DetectrModlDesc,   0 },
	DESC_END
};
DETECTEUR_VAR ArgDesc DetectrParmsDesc[] = {
	{ DESC_COMMENT 0 },
	{ "masse",     DESC_FLOAT                &DetecteurLu.masse,        "masse du detecteur (g)" },
	{ "mode",      DESC_LISTE ArchModeDetec, &DetecteurLu.mode_arch,    "mode de sauvegarde" },
	{ "demarrage", DESC_STR(MAXFILENAME)      DetecteurLu.start.script, "nom du fichier de demarrage de run" },
	{ "entretien", DESC_STR(MAXFILENAME)      DetecteurLu.regul.script, "nom du fichier d'entretien periodique" },
	{ "voisins",   DESC_STR_SIZE(DETEC_NOM,DetecteurLu.nbvoisins,DETEC_VOISINS_MAX) DetecteurLu.nomvoisins, "detecteurs a sauver aussi en mode 'voisins'" },
	{ "associes",  DESC_STR_SIZE(DETEC_NOM,DetecteurLu.nbassoc,DETEC_ASSOC_MAX)     DetecteurLu.nom_assoc,  "detecteurs a sauver aussi en mode 'individuel'" },
	DESC_END
};
static ArgDesc DetectrCaptrDesc[] = {
	DESC_END
};
static ArgStruct DetectrCaptAS = { (void *)DetecteurLu.captr, (void *)&DetectrCaptrLu, sizeof(TypeCapteur), DetectrCaptrDesc };
static ArgDesc DetectrUniqueDesc[] = {
	{ "nom",        DESC_STR(DETEC_NOM)         DetecteurLu.nom,   0 },
	{ "etat",       DESC_KEY DETEC_STATUTS,    &DetecteurLu.lu,   "statut ("DETEC_STATUTS")" },
	{ "definition", DESC_STRUCT                &DetectrDefDesc,    0 },
	{ "parametres", DESC_STRUCT                &DetecParmsDesc,    0 },
	{ "voies",      DESC_STRUCT_SIZE(DetecteurLu.nbcapt,DETEC_CAPT_MAX) &DetectrCaptAS, 0 },
	DESC_END
};
static ArgDesc *DetectrModeDesc[] = { DetectrListeDesc, DetectrUniqueDesc };
static ArgDesc DetectrDesc[] = {
	{ "inventaire", DESC_KEY DETEC_CTLG,          &DetectrCtlgMode, "forme de l'inventaire ("DETEC_CTLG")" },
	{ 0,            DESC_VARIANTE(DetectrCtlgMode) DetectrModeDesc, 0 },
	DESC_END
};
#endif /* DETECTR_NL */

static FILE *DetecteurBaseFile;
static TypeDetecteur DetecteurAJeter;
static char *DetecteurNomLu;
static int DetecteurParmsListe[DETEC_MAX],DetecteurParmsNb;
static int DetecteurParmsExemple;
static char DetecteurParmsExplics;
#ifdef DELAI_ACTION
static Instrum cDelaiAction;
#endif
static char ReglRazEtat;
// static Menu mReglageMenuVoie,mReglageMenuSuite;
static Panel pDetecVoieModl;
static Panel pDetecGestion;

static WndContextPtr ReglAllumeUpdate,ReglAllumeRazfet;
static int ReglRazDuree; char ReglRazEnCours;
// static unsigned char BoloAdrsCourante;

static Onglet DetecteurParmsFeuilles[DETEC_PARM_NBNATURES];
static Cadre bDetecteurParms[DETEC_PARM_NBNATURES+1];

#define REGLAGE_DET_AVEC_SIGNAL
#define REGLAGE_LARGEUR 22
#define CURSEURS_MINMAX
#ifdef CURSEURS_MINMAX
// demi-glissiere pour en avoir 2 cote a cote: static TypeInstrumGlissiere ReglageGlissMinMax = { 75, 15, 1, 1, 8, 3 };
static TypeInstrumGlissiere ReglageGlissMinMax = { 175, 15, 1, 1, 8, 3 };
#endif
static TypeInstrumGlissiere ReglageGlissAffiche = { 110, 15, 1, 1, 8, 3 };
static TypeInstrumGlissiere ReglageGlissTemps = { 200, 15, 1, 1, 8, 3 };
static TypeInstrumGlissiere ReglageGliss21c = { 150, 15, 1, 1, 8, 3 };
static int OscilloLarg,OscilloHaut;

int NumerModlRessIndex(TypeBNModele *modele_bn, char *nom);

extern MenuItem iNumeriseurRessUpDown[];

static int DetecteurDeclare(TypeDetecteur *detectr, int bolo, char *nom, int dm, char *ref);
static TypeComposantePseudo *DetecteurCompositionDecode(char niveau, void *adrs, char *texte, int *ref, int *nb_erreurs);
static void DetecteurAjoutePseudo(TypeDetecteur *detectr, TypeComposantePseudo *pseudo, char *ref, int vm, int *nb_erreurs);
static void DetecteurSimplifieVoie(short voie);
static char DetecteurAjouteVoies(TypeDetecteur *detectr, int *nb_erreurs);
static void DetecteurAjouteReglages(TypeDetecteur *detectr, int *nb_erreurs, char log);
static short DetecteurVoieCree(TypeDetecteur *detectr, short cap, short vm, char *nom);
static float *DetecteurConsigneAdrs(TypeDetecteur *detectr, int cap, int cmde);

static char OscilloRestartDemande;
static void OscilloDefiniTemps(Oscillo oscillo);
#ifdef REDONDANT
static int OscilloChangeAcq(Instrum instrum, Oscillo oscillo);
static int OscilloChangeModul(Instrum instrum, Oscillo oscillo);
#endif
static int OscilloChangeRefresh();
static Oscillo OscilloCreate(int voie);
static void OscilloVide(Oscillo oscillo);
// static int OscilloChangeTrigger(Oscillo oscillo, void *adrs);
static int OscilloChangeiTrig(Instrum instrum, Oscillo oscillo);
static int OscilloChangepTrig(Panel panel, int item, void *arg);
static int OscilloHistoChangepMin(Panel panel, int item, void *arg);
static int OscilloHistoChangepMax(Panel panel, int item, void *arg);
static void OscilloHistoChangeMinMax(Oscillo oscillo);
static int OscilloHistoChangeiMin(Instrum instrum, Oscillo oscillo);
static int OscilloHistoChangeiMax(Instrum instrum, Oscillo oscillo);
static int OscilloHistoRaz(Menu menu, MenuItem item);
static int OscilloChangeVoie(Instrum instrum, Oscillo oscillo);

static void GegeneMontre(int simu);
static int GegeneSauve(Menu menu, MenuItem *item);

static int ReglagePlancheCree(int voie);
static void ReglagePlancheSupprime(int voie);
static void ReglageDeLaVoie(int bolo, int cap, char avec_trigger);

#ifdef WXWIDGETS
void OpiumRefreshAllWindows();
#endif

/* ========================================================================== */
#ifdef OPIUM_TEST_SELECTEUR
static char DetecteurParmsC1,DetecteurParmsC2,DetecteurParmsC3,DetecteurParmsC4;
static TypeOpiumSelecteur DetecteurParmsSelecteur[] = {
	{ "choix1", &DetecteurParmsC1 },
	{ "choix2", &DetecteurParmsC2 },
	{ "choix3", &DetecteurParmsC3 },
	{ "choix4", &DetecteurParmsC4 },
	OPIUM_SELECT_FIN
};
{
	DetecteurParmsC1 = DetecteurParmsC2 = DetecteurParmsC3 = DetecteurParmsC4 = 0;
	p = PanelSelecteur(DetecteurParmsSelecteur,1);
}
#endif
#ifdef OPIUM_TEST_PANELBOUTON
/* .......................................................................... */
static char *DetecteurParmsSpecial(void *adrs) {
	if(adrs) {
		int n; n = *(int *)adrs; OpiumNote("Il y a %d detecteur%s selectionne%s",Accord2s(n));
	} else return("demander");
}
	PanelBouton(p,"Detecteurs impliques",DetecteurParmsSpecial,(void *)(&DetecteurParmsNb),8);
#endif
/* ========================================================================== */

#pragma mark ---------- modeles ----------

/* .......................................................................... */
/*                                M O D E L E S                               */
/* ========================================================================== */
static void DetecModlStandard(char *nom_voies) {
	char *c,*nom; int l; int vm,ph; int p,i,modl;
	char liste_voies[VOIE_LISTE_STR];
	TypeDetModlReglage *regl;
	
	strcpy(ModelePhys[0].nom,"phonon");
	ModelePhys[0].duree         = 100000.0; /* Evenement chaleur:  1 ms      */
	ModelePhys[0].tempsmort     = -1.0;     /* Temps mort apres max evt (ms) */
	ModelePhys[0].delai         = 0.0;	    /* Delai relatif de l'evt (ms)   */
	ModelePhys[0].interv        = 100.0;	/* jitter propre (ms)            */
#ifdef RL
    ModelePhys[0].dimtemplate	= 64;   /* nombre de point actif pour filtrage template*/
	ModelePhys[0].montee		= 2.0;  /* temps de montée de l'evenement (ms) */ 
	ModelePhys[0].descente1		= 15.0; /* temps de descente rapide (ms)       */
	ModelePhys[0].descente2		= 100;  /* temps de descente lente (ms)        */
	ModelePhys[0].facteur2		= 0.3;  /* fraction de descente lente          */
#endif
	strcpy(ModelePhys[1].nom,"charge");
	ModelePhys[1].duree         = 1000.0; /* Evenement chaleur:  10 ms     */
	ModelePhys[1].tempsmort     = -1.0;   /* Temps mort apres max evt (ms) */
	ModelePhys[1].delai         = -1.0;	  /* Delai relatif de l'evt (ms)   */
	ModelePhys[1].interv        = 1.0;	  /* jitter propre (ms)            */
#ifdef RL
    ModelePhys[1].dimtemplate	= 64;     /* nombre de point actif pour filtrage template*/
	ModelePhys[1].montee		= 0.0;    /* temps de montée de l'evenement (ms) */
	ModelePhys[1].descente1		= 10.0;   /* temps de descente rapide (ms)       */
	ModelePhys[1].descente2		= 0.0;    /* temps de descente lente (ms)        */
	ModelePhys[1].facteur2		= 0.0;    /* fraction de descente lente          */
#endif
	ModelePhysNb = 2;
	
#ifdef VOIES_STYLE_PREFS
	strncpy(liste_voies,nom_voies,VOIE_LISTE_STR);
#endif
#ifdef VOIES_STYLE_NTD
	strncpy(liste_voies,"chaleur~/centre/garde",VOIE_LISTE_STR);
#endif
#ifdef VOIES_STYLE_EDW2
	strncpy(liste_voies,"chaleur~/centre/garde/signal/chaleurA~/chaleurB~/ionisA/ionisB/ionisC/ionisD/ionisG/ionisH/lumiere~",VOIE_LISTE_STR);
	//                     0        1      2     3       4         5      6 7 8 9 10 11 12
#endif
#ifdef VOIES_STYLE_INSTALLE
	strncpy(liste_voies,"chaleurA~/chaleurB~/ionisA/ionisB/ionisC/ionisD/signal",VOIE_LISTE_STR);
	//                     0        1         2        3      4      5      6
#endif
#ifdef VOIES_STYLE_UNIQUE
	strncpy(liste_voies,"signal",VOIE_LISTE_STR);
#endif
	// printf("  Liste des modeles de voies: '%s'\n",liste_voies);
	c = liste_voies; vm = 0;
	while(*c && (vm < (VOIES_TYPES - 1))) {
		nom = ItemSuivant(&c,'/');
		l = strlen(nom);
		if((nom[l - 1] == '~') || (nom[l - 1] == '@')) /* pas trouve le tilde sur le clavier francais... */ {
			nom[l - 1] = '\0';
			// ModeleVoie[vm].modulee = 1;
			p = ModeleVoie[vm].physique = 0;
		} else {
			// ModeleVoie[vm].modulee = 0;
			p = ModeleVoie[vm].physique = 1;
		}
		strncpy(ModeleVoie[vm].nom,nom,MODL_NOM);
		ModeleVoie[vm].duree = ModelePhys[p].duree;
		ModeleVoie[vm].delai = ModelePhys[p].delai;
		ModeleVoie[vm].tempsmort = ModelePhys[p].tempsmort;
		ModeleVoie[vm].interv = ModelePhys[p].interv;
		ModeleVoie[vm].coinc = 20.0;
		ModeleVoie[vm].pretrigger = 25.0;
		ModeleVoie[vm].base_dep = 25.0;
		ModeleVoie[vm].base_arr = 75.0;
		ModeleVoie[vm].ampl_10 = 10.0;
		ModeleVoie[vm].ampl_90 = 90.0;
		for(ph=0; ph<DETEC_PHASES_MAX; ph++) {
			ModeleVoie[vm].phase[ph].t0 = 0.0;
			ModeleVoie[vm].phase[ph].dt = 0.0;
		}
		ModeleVoie[vm].utilisee = 0;
#ifdef RL
		ModeleVoie[vm].dimtemplate = ModelePhys[p].dimtemplate;
		ModeleVoie[vm].montee = ModelePhys[p].montee;
		ModeleVoie[vm].descente1 = ModelePhys[p].descente1;
		ModeleVoie[vm].descente2 = ModelePhys[p].descente2;
		ModeleVoie[vm].facteur2 = ModelePhys[p].facteur2;
#endif
		ModeleVoie[vm].min_moyen = 1000.0;
		ModeleVoie[vm].max_moyen = 8000.0;
		vm++;
	}
	ModeleVoie[vm].nom[0] = '\0';
	ModeleVoieNb = vm;
	//	PrevenirSi(ModeleVoieNb != ModeleVoieNb);
	
	for(i=0; i<DETEC_TYPES; i++) ModeleDet[i].masse = 320.0;

#ifdef VOIES_STYLE_NTD
	strcpy(ModeleDet[DETEC_NTD].nom,"NTD");
	ModeleDet[DETEC_NTD].nbassoc = 0;
	strcpy(ModeleDet[DETEC_NbSiA].nom,"NbSiA");
	ModeleDet[DETEC_NbSiA].nbassoc = 1;
	ModeleDet[DETEC_NbSiA].assoc[0] = DETEC_NbSiB;
	ModeleDet[DETEC_NbSiA].duree_raz = SettingsRazNbSiDuree;
	strcpy(ModeleDet[DETEC_NbSiB].nom,"NbSiB");
	ModeleDet[DETEC_NbSiB].nbassoc = 1;
	ModeleDet[DETEC_NbSiB].assoc[0] = DETEC_NbSiA;
	ModeleDet[DETEC_NbSiB].duree_raz = SettingsRazNbSiDuree;
	strcpy(ModeleDet[DETEC_IASC].nom,"IAS-C");
	ModeleDet[DETEC_IASC].nbassoc = 1;
	ModeleDet[DETEC_IASC].assoc[0] = DETEC_IASL;
	strcpy(ModeleDet[DETEC_IASL].nom,"IAS-L");
	ModeleDet[DETEC_IASL].nbassoc = 1;
	ModeleDet[DETEC_IASL].assoc[0] = DETEC_IASC;
	strcpy(ModeleDet[DETEC_ID_AB].nom,"ID_AB");
	ModeleDet[DETEC_ID_AB].nbassoc = 2;
	ModeleDet[DETEC_ID_AB].assoc[0] = DETEC_ID_CD; ModeleDet[DETEC_ID_AB].assoc[1] = DETEC_ID_GH;
	strcpy(ModeleDet[DETEC_ID_CD].nom,"ID_CD");
	ModeleDet[DETEC_ID_CD].nbassoc = 2;
	ModeleDet[DETEC_ID_CD].assoc[0] = DETEC_ID_AB; ModeleDet[DETEC_ID_CD].assoc[1] = DETEC_ID_GH;
	strcpy(ModeleDet[DETEC_ID_GH].nom,"ID_GH");
	ModeleDet[DETEC_ID_GH].nbassoc = 2;
	ModeleDet[DETEC_ID_GH].assoc[0] = DETEC_ID_AB; ModeleDet[DETEC_ID_GH].assoc[1] = DETEC_ID_CD;
	strcpy(ModeleDet[DETEC_FID_AB].nom,"FID_AB");
	ModeleDet[DETEC_FID_AB].nbassoc = 1;
	ModeleDet[DETEC_FID_AB].assoc[0] = DETEC_FID_CD;
	strcpy(ModeleDet[DETEC_FID_CD].nom,"FID_CD");
	ModeleDet[DETEC_FID_CD].nbassoc = 1;
	ModeleDet[DETEC_FID_CD].assoc[0] = DETEC_FID_AB;
	ModeleDetNb = DETEC_FID_CD + 1;
	
	for(i=0; i<ModeleDetNb; i++) {
		ModeleDet[i].nbcapt = ModeleVoieNb;
		for(vm=0; vm<ModeleVoieNb; vm++) ModeleDet[i].type[vm] = vm;
		ModeleDet[i].avec_regen = 0;
		ModeleDet[i].nbregl = 0;
		regl = &(ModeleDet[i].regl[0]); j = 0;
		//	strcpy(regl->nom,"marge");        regl->capteur = 0; regl->cmde = NEANT; regl++;
		strcpy(regl->nom,"ampl-modul");   regl->capteur = 0; regl->cmde = REGL_MODUL; regl++;  strcpy(&(ModeleDet[i].legende[j++][0]),"exci");
		strcpy(regl->nom,"comp-modul");   regl->capteur = 0; regl->cmde = REGL_COMP; regl++;   strcpy(&(ModeleDet[i].legende[j++][0]),"comp");
		strcpy(regl->nom,"corr-trngl");   regl->capteur = 0; regl->cmde = REGL_CORR; regl++;   strcpy(&(ModeleDet[i].legende[j++][0]),"corr");
		strcpy(regl->nom,"corr-pied");    regl->capteur = 0; regl->cmde = REGL_CORR; regl++;
		strcpy(regl->nom,"polar-centre"); regl->capteur = 1; regl->cmde = REGL_POLAR; regl++;  strcpy(&(ModeleDet[i].legende[j++][0]),"polar1");
		strcpy(regl->nom,"polar-garde");  regl->capteur = 2; regl->cmde = REGL_POLAR; regl++;  strcpy(&(ModeleDet[i].legende[j++][0]),"polar2");
		strcpy(regl->nom,"gain-chaleur"); regl->capteur = 0; regl->cmde = REGL_GAIN; regl++;   strcpy(&(ModeleDet[i].legende[j++][0]),"Gchal");
		strcpy(regl->nom,"gain-centre");  regl->capteur = 1; regl->cmde = REGL_GAIN; regl++;   strcpy(&(ModeleDet[i].legende[j++][0]),"Gcent");
		strcpy(regl->nom,"gain-garde");   regl->capteur = 2; regl->cmde = REGL_GAIN; regl++;   strcpy(&(ModeleDet[i].legende[j++][0]),"Ggard");
		strcpy(regl->nom,"chauffe-fet");  regl->capteur = -1; regl->cmde = REGL_DIVERS; regl++;
		strcpy(regl->nom,"regul-bolo");   regl->capteur = -1; regl->cmde = REGL_DIVERS; regl++;
		if((i == DETEC_NbSiA) || (i == DETEC_NbSiB)) {
			strcpy(regl->nom,"polar-fet");    regl->capteur = 0; regl->cmde = REGL_FET_CC; regl++;    strcpy(&(ModeleDet[i].legende[j++][0]),"FET");
		} else {
			strcpy(regl->nom,"polar-fet");    regl->capteur = 0; regl->cmde = REGL_FET_MASSE; regl++; strcpy(&(ModeleDet[i].legende[j++][0]),"FET");
		}
		strcpy(regl->nom,"d2");           regl->capteur = 0; regl->cmde = REGL_D2;   regl++;   strcpy(&(ModeleDet[i].legende[j++][0]),"d2");
		strcpy(regl->nom,"d3");           regl->capteur = 0; regl->cmde = REGL_D3;   regl++;   strcpy(&(ModeleDet[i].legende[j++][0]),"d3");
		ModeleDet[i].nbregl = regl - ModeleDet[i].regl;
		//	printf("* %d reglages pour les detecteurs de type %s\n",ModeleDet[DETEC_NTD].nbregl,ModeleDet[DETEC_NTD].nom);
	}
#endif
#ifdef VOIES_STYLE_EDW2
	strcpy(ModeleDet[DETEC_NTD].nom,"NTD");
	ModeleDet[DETEC_NTD].nbassoc = 0;
	strcpy(ModeleDet[DETEC_NbSiA].nom,"NbSiA");
	ModeleDet[DETEC_NbSiA].nbassoc = 1; ModeleDet[DETEC_NbSiA].assoc[0] = DETEC_NbSiB;
	ModeleDet[DETEC_NbSiA].duree_raz = SettingsRazNbSiDuree;
	strcpy(ModeleDet[DETEC_NbSiB].nom,"NbSiB");
	ModeleDet[DETEC_NbSiB].nbassoc = 1; ModeleDet[DETEC_NbSiB].assoc[0] = DETEC_NbSiA;
	ModeleDet[DETEC_NbSiB].duree_raz = SettingsRazNbSiDuree;
	strcpy(ModeleDet[DETEC_IASC].nom,"IAS-C");
	ModeleDet[DETEC_IASC].nbassoc = 1; ModeleDet[DETEC_IASC].assoc[0] = DETEC_IASL;
	strcpy(ModeleDet[DETEC_IASL].nom,"IAS-L");
	ModeleDet[DETEC_IASL].nbassoc = 1; ModeleDet[DETEC_IASL].assoc[0] = DETEC_IASC;
	strcpy(ModeleDet[DETEC_ID_AB].nom,"ID_AB");
	ModeleDet[DETEC_ID_AB].nbassoc = 2; ModeleDet[DETEC_ID_AB].assoc[0] = DETEC_ID_CD; ModeleDet[DETEC_ID_AB].assoc[1] = DETEC_ID_GH;
	strcpy(ModeleDet[DETEC_ID_CD].nom,"ID_CD");
	ModeleDet[DETEC_ID_CD].nbassoc = 2; ModeleDet[DETEC_ID_CD].assoc[0] = DETEC_ID_AB; ModeleDet[DETEC_ID_CD].assoc[1] = DETEC_ID_GH;
	strcpy(ModeleDet[DETEC_ID_GH].nom,"ID_GH");
	ModeleDet[DETEC_ID_GH].nbassoc = 2; ModeleDet[DETEC_ID_GH].assoc[0] = DETEC_ID_AB; ModeleDet[DETEC_ID_GH].assoc[1] = DETEC_ID_CD;
	strcpy(ModeleDet[DETEC_FID_AB].nom,"FID_AB");
	ModeleDet[DETEC_FID_AB].nbassoc = 1; ModeleDet[DETEC_FID_AB].assoc[0] = DETEC_FID_CD;
	strcpy(ModeleDet[DETEC_FID_CD].nom,"FID_CD");
	ModeleDet[DETEC_FID_CD].nbassoc = 1; ModeleDet[DETEC_FID_CD].assoc[0] = DETEC_FID_AB;
	/* tous les detecteurs precedents sont NTD-like, d'ou: initialisation globale! */
	for(i=0; i<DETEC_FID_CD+1; i++) {
		ModeleDet[i].nbcapt = 3;
		ModeleDet[i].type[0] = 0;
		ModeleDet[i].type[1] = 1;
		ModeleDet[i].type[2] = 2;
		regl = &(ModeleDet[i].regl[0]);
		//	strcpy(regl->nom,"marge");        regl->capteur = 0; regl->cmde = NEANT; regl++;
		strcpy(regl->nom,"d2");             regl->capteur = 0;  regl->cmde = REGL_D2; regl++;
		strcpy(regl->nom,"d3");             regl->capteur = 0;  regl->cmde = REGL_D3; regl++;
		strcpy(regl->nom,"polar-centre");   regl->capteur = 1;  regl->cmde = REGL_POLAR; regl++;
		strcpy(regl->nom,"polar-garde");    regl->capteur = 2;  regl->cmde = REGL_POLAR; regl++;
		strcpy(regl->nom,"gain-chaleur");   regl->capteur = 0;  regl->cmde = REGL_GAIN; regl++;
		strcpy(regl->nom,"gain-centre");    regl->capteur = 1;  regl->cmde = REGL_GAIN; regl++;
		strcpy(regl->nom,"gain-garde");     regl->capteur = 2;  regl->cmde = REGL_GAIN; regl++;
		strcpy(regl->nom,"chauffe-fet");    regl->capteur = -1; regl->cmde = REGL_DIVERS; regl++;
		strcpy(regl->nom,"regul-bolo");     regl->capteur = -1; regl->cmde = REGL_DIVERS; regl++;
		strcpy(regl->nom,"polar-fet");      regl->capteur = 0;  regl->cmde = REGL_FET_MASSE; regl++;
		strcpy(regl->nom,"ampl-modul");     regl->capteur = 0;  regl->cmde = REGL_MODUL; regl++;
		strcpy(regl->nom,"comp-modul");     regl->capteur = 0;  regl->cmde = REGL_COMP; regl++;
		strcpy(regl->nom,"corr-trngl");     regl->capteur = 0;  regl->cmde = REGL_CORR; regl++;
		strcpy(regl->nom,"corr-pied");      regl->capteur = 0;  regl->cmde = REGL_CORR; regl++;
		ModeleDet[i].nbregl = regl - ModeleDet[i].regl;
		//	printf("* %d reglages pour les detecteurs de type %s\n",ModeleDet[i].nbregl,ModeleDet[i].nom);
	}
	strcpy(ModeleDet[DETEC_NbSi].nom,"NbSi");
	ModeleDet[DETEC_NbSi].nbcapt = 4;
	ModeleDet[DETEC_NbSi].type[0] = 4;
	ModeleDet[DETEC_NbSi].type[1] = 5;
	ModeleDet[DETEC_NbSi].type[2] = 1;
	ModeleDet[DETEC_NbSi].type[3] = 2;
	ModeleDet[DETEC_NbSi].nbassoc = 0;
	ModeleDet[DETEC_NbSi].nbregl = 0;
	ModeleDet[DETEC_NbSi].duree_raz = SettingsRazNbSiDuree;
	strcpy(ModeleDet[DETEC_IAS].nom,"IAS");
	ModeleDet[DETEC_IAS].nbcapt = 2;
	ModeleDet[DETEC_IAS].type[0] = 0;
	ModeleDet[DETEC_IAS].type[1] = 12;
	ModeleDet[DETEC_IAS].nbassoc = 0;
	ModeleDet[DETEC_IAS].nbregl = 0;
	regl = &(ModeleDet[DETEC_IAS].regl[0]);
	//	strcpy(regl->nom,"marge");        regl->capteur = 0; regl->cmde = NEANT; regl++;
	strcpy(regl->nom,"d2");             regl->capteur = 0;  regl->cmde = REGL_D2; regl++;
	strcpy(regl->nom,"d3");             regl->capteur = 0;  regl->cmde = REGL_D3; regl++;
	strcpy(regl->nom,"gain-chal");      regl->capteur = 0;  regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"polar-fet-chal"); regl->capteur = 0;  regl->cmde = REGL_FET_MASSE; regl++;
	strcpy(regl->nom,"ampl-chal");      regl->capteur = 0;  regl->cmde = REGL_MODUL; regl++;
	strcpy(regl->nom,"comp-chal");      regl->capteur = 0;  regl->cmde = REGL_COMP; regl++;
	strcpy(regl->nom,"corr-chal");      regl->capteur = 0;  regl->cmde = REGL_CORR; regl++;
	strcpy(regl->nom,"gain-lum");       regl->capteur = 1;  regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"polar-fet-lum");  regl->capteur = 1;  regl->cmde = REGL_FET_MASSE; regl++;
	strcpy(regl->nom,"ampl-lum");       regl->capteur = 1;  regl->cmde = REGL_MODUL; regl++;
	strcpy(regl->nom,"comp-lum");       regl->capteur = 1;  regl->cmde = REGL_COMP; regl++;
	strcpy(regl->nom,"corr-lum");       regl->capteur = 1;  regl->cmde = REGL_CORR; regl++;
	ModeleDet[DETEC_IAS].nbregl = regl - ModeleDet[DETEC_IAS].regl;
	strcpy(ModeleDet[DETEC_ID].nom,"ID");
	ModeleDet[DETEC_ID].nbcapt = 7;
	ModeleDet[DETEC_ID].type[0] = 0;
	ModeleDet[DETEC_ID].type[1] = 6;
	ModeleDet[DETEC_ID].type[2] = 7;
	ModeleDet[DETEC_ID].type[3] = 8;
	ModeleDet[DETEC_ID].type[4] = 9;
	ModeleDet[DETEC_ID].type[5] = 10;
	ModeleDet[DETEC_ID].type[6] = 11;
	ModeleDet[DETEC_ID].nbassoc = 0;
	ModeleDet[DETEC_ID].nbregl = 0;
	regl = &(ModeleDet[DETEC_ID].regl[0]);
	strcpy(regl->nom,"d2");           regl->capteur = 0; regl->cmde = REGL_D2; regl++;
	strcpy(regl->nom,"d3");           regl->capteur = 0; regl->cmde = REGL_D3; regl++;
	strcpy(regl->nom,"ampl-modul");   regl->capteur = 0; regl->cmde = REGL_MODUL; regl++;
	strcpy(regl->nom,"comp-modul");   regl->capteur = 0; regl->cmde = REGL_COMP; regl++;
	strcpy(regl->nom,"corr-trngl");   regl->capteur = 0; regl->cmde = REGL_CORR; regl++;
	strcpy(regl->nom,"polar-A");      regl->capteur = 1; regl->cmde = REGL_POLAR; regl++;
	strcpy(regl->nom,"polar-B");      regl->capteur = 2; regl->cmde = REGL_POLAR; regl++;
	strcpy(regl->nom,"polar-C");      regl->capteur = 3; regl->cmde = REGL_POLAR; regl++;
	strcpy(regl->nom,"polar-D");      regl->capteur = 4; regl->cmde = REGL_POLAR; regl++;
	strcpy(regl->nom,"polar-G");      regl->capteur = 5; regl->cmde = REGL_POLAR; regl++;
	strcpy(regl->nom,"polar-H");      regl->capteur = 6; regl->cmde = REGL_POLAR; regl++;
	strcpy(regl->nom,"gain-A");       regl->capteur = 1; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"gain-B");       regl->capteur = 2; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"gain-C");       regl->capteur = 3; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"gain-D");       regl->capteur = 4; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"gain-G");       regl->capteur = 5; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"gain-H");       regl->capteur = 6; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"polar-fet-A");  regl->capteur = 1; regl->cmde = REGL_FET_CC; regl++;
	strcpy(regl->nom,"polar-fet-B");  regl->capteur = 2; regl->cmde = REGL_FET_CC; regl++;
	strcpy(regl->nom,"polar-fet-C");  regl->capteur = 3; regl->cmde = REGL_FET_CC; regl++;
	strcpy(regl->nom,"polar-fet-D");  regl->capteur = 4; regl->cmde = REGL_FET_CC; regl++;
	strcpy(regl->nom,"polar-fet-G");  regl->capteur = 5; regl->cmde = REGL_FET_CC; regl++;
	strcpy(regl->nom,"polar-fet-H");  regl->capteur = 6; regl->cmde = REGL_FET_CC; regl++;
	ModeleDet[DETEC_ID].nbregl = regl - ModeleDet[DETEC_ID].regl;
	strcpy(ModeleDet[DETEC_FID].nom,"FID");
	ModeleDet[DETEC_FID].nbcapt = 5;
	ModeleDet[DETEC_FID].type[0] = 0;
	ModeleDet[DETEC_FID].type[1] = 6;
	ModeleDet[DETEC_FID].type[2] = 7;
	ModeleDet[DETEC_FID].type[3] = 8;
	ModeleDet[DETEC_FID].type[4] = 9;
	ModeleDet[DETEC_FID].nbassoc = 0;
	ModeleDet[DETEC_FID].nbregl = 0;
	regl = &(ModeleDet[DETEC_FID].regl[0]);
	strcpy(regl->nom,"d2");           regl->capteur = 0; regl->cmde = REGL_D2; regl++;
	strcpy(regl->nom,"d3");           regl->capteur = 0; regl->cmde = REGL_D3; regl++;
	strcpy(regl->nom,"ampl-modul");   regl->capteur = 0; regl->cmde = REGL_MODUL; regl++;
	strcpy(regl->nom,"comp-modul");   regl->capteur = 0; regl->cmde = REGL_COMP; regl++;
	strcpy(regl->nom,"corr-trngl");   regl->capteur = 0; regl->cmde = REGL_CORR; regl++;
	strcpy(regl->nom,"polar-A");      regl->capteur = 1; regl->cmde = REGL_POLAR; regl++;
	strcpy(regl->nom,"polar-B");      regl->capteur = 2; regl->cmde = REGL_POLAR; regl++;
	strcpy(regl->nom,"polar-C");      regl->capteur = 3; regl->cmde = REGL_POLAR; regl++;
	strcpy(regl->nom,"polar-D");      regl->capteur = 4; regl->cmde = REGL_POLAR; regl++;
	strcpy(regl->nom,"gain-A");       regl->capteur = 1; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"gain-B");       regl->capteur = 2; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"gain-C");       regl->capteur = 3; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"gain-D");       regl->capteur = 4; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"polar-fet-A");  regl->capteur = 1; regl->cmde = REGL_FET_CC; regl++;
	strcpy(regl->nom,"polar-fet-B");  regl->capteur = 2; regl->cmde = REGL_FET_CC; regl++;
	strcpy(regl->nom,"polar-fet-C");  regl->capteur = 3; regl->cmde = REGL_FET_CC; regl++;
	strcpy(regl->nom,"polar-fet-D");  regl->capteur = 4; regl->cmde = REGL_FET_CC; regl++;
	ModeleDet[DETEC_FID].nbregl = regl - ModeleDet[DETEC_FID].regl;
	// printf("  * Un detecteur de type %s a %d reglages\n",ModeleDet[DETEC_FID].nom,ModeleDet[DETEC_FID].nbregl);
	strcpy(ModeleDet[DETEC_1VOIE].nom,"1voie");
	ModeleDet[DETEC_1VOIE].nbcapt = 1;
	ModeleDet[DETEC_1VOIE].type[0] = 3;
	ModeleDet[DETEC_1VOIE].nbassoc = 0;
	ModeleDet[DETEC_1VOIE].nbregl = 0;
	ModeleDetNb = DETEC_NBTYPES;
#endif
#ifdef VOIES_STYLE_INSTALLE
//	strncpy(liste_voies,"chaleurA~/chaleurB~/ionisA/ionisB/ionisC/ionisD/signal",VOIE_LISTE_STR);
	//                     0        1         2        3      4      5      6
	ModeleDetNb = 0;
	strcpy(ModeleDet[ModeleDetNb].nom,"FID");
	ModeleDet[ModeleDetNb].nbcapt = 5;
	ModeleDet[ModeleDetNb].type[0] = 0;
	ModeleDet[ModeleDetNb].type[1] = 2;
	ModeleDet[ModeleDetNb].type[2] = 3;
	ModeleDet[ModeleDetNb].type[3] = 4;
	ModeleDet[ModeleDetNb].type[4] = 5;
	ModeleDet[ModeleDetNb].nbassoc = 0;
	ModeleDet[ModeleDetNb].nbregl = 0;
	regl = &(ModeleDet[ModeleDetNb].regl[0]);
	strcpy(regl->nom,"ampl-modul");   regl->capteur = 0; regl->cmde = REGL_MODUL; regl++;
	strcpy(regl->nom,"comp-modul");   regl->capteur = 0; regl->cmde = REGL_COMP; regl++;
	strcpy(regl->nom,"corr-trngl");   regl->capteur = 0; regl->cmde = REGL_CORR; regl++;
	strcpy(regl->nom,"gain-chaleur"); regl->capteur = 0; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"d2");           regl->capteur = 0; regl->cmde = REGL_D2; regl++;
	strcpy(regl->nom,"d3");           regl->capteur = 0; regl->cmde = REGL_D3; regl++;
	strcpy(regl->nom,"polar-A");      regl->capteur = 2; regl->cmde = REGL_POLAR; regl++;
	strcpy(regl->nom,"polar-B");      regl->capteur = 3; regl->cmde = REGL_POLAR; regl++;
	strcpy(regl->nom,"polar-C");      regl->capteur = 4; regl->cmde = REGL_POLAR; regl++;
	strcpy(regl->nom,"polar-D");      regl->capteur = 5; regl->cmde = REGL_POLAR; regl++;
	strcpy(regl->nom,"gain-A");       regl->capteur = 2; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"gain-B");       regl->capteur = 3; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"gain-C");       regl->capteur = 4; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"gain-D");       regl->capteur = 5; regl->cmde = REGL_GAIN; regl++;
	strcpy(regl->nom,"polar-fet-A");  regl->capteur = 2; regl->cmde = REGL_FET_CC; regl++;
	strcpy(regl->nom,"polar-fet-B");  regl->capteur = 3; regl->cmde = REGL_FET_CC; regl++;
	strcpy(regl->nom,"polar-fet-C");  regl->capteur = 4; regl->cmde = REGL_FET_CC; regl++;
	strcpy(regl->nom,"polar-fet-D");  regl->capteur = 5; regl->cmde = REGL_FET_CC; regl++;
	ModeleDet[ModeleDetNb].nbregl = regl - ModeleDet[ModeleDetNb].regl;
	ModeleDetNb++;
	strcpy(ModeleDet[ModeleDetNb].nom,"1voie");
	ModeleDet[ModeleDetNb].nbcapt = 1;
	ModeleDet[ModeleDetNb].type[0] = 6;
	ModeleDet[ModeleDetNb].nbassoc = 0;
	ModeleDet[ModeleDetNb].nbregl = 0;
	ModeleDetNb++;
#endif
#ifdef VOIES_STYLE_UNIQUE
	strcpy(ModeleDet[DETEC_1VOIE].nom,"1voie");
	ModeleDet[DETEC_1VOIE].nbcapt = 1;
	ModeleDet[DETEC_1VOIE].type[0] = 0;
	ModeleDet[DETEC_1VOIE].nbassoc = 0;
	ModeleDet[DETEC_1VOIE].nbregl = 0;
	ModeleDetNb = DETEC_NBTYPES;
#endif

	for(modl=0; modl<ModeleDetNb; modl++) {
		for(i=0; i<ModeleDet[modl].nbassoc; i++) strcpy(&(ModeleDet[modl].nom_assoc[i][0]),ModeleDet[ModeleDet[modl].assoc[i]].nom);
	}
}
/* ========================================================================== */
void DetecteursModlLit(char *fichier, FILE *g) {
	char nom_complet[MAXFILENAME]; FILE *f; int pm,vm,dm; int nb_erreurs;
	
	nb_erreurs = 0;
	f = 0;
	if(fichier) {
		if(!strcmp(fichier,"*")) {
			strcpy(nom_complet,FichierPrefDetecteurs); f = g;
		} else {
			strcpy(DetecteurFichierModeles,fichier);
			RepertoireIdentique(FichierPrefDetecteurs,DetecteurFichierModeles,nom_complet);
		}
	} else {
		DetecteurFichierModeles[0] = '\0';
		strcpy(nom_complet,FichierModlDetecteurs);
	}
	if(!f) f = fopen(nom_complet,"r");
	if(f) {
		// printf("  . Lecture des modeles de detecteurs   dans '%s'\n",nom_complet);
		SambaLogDef(". Lecture des modeles de detecteurs","dans",FichierModlChassis);
		ArgFromFile(f,DetectionDesc,0); if(!g) fclose(f);
	} else {
		printf("  ! Fichier %s inutilisable (%s). Modeles standards utilises.\n",nom_complet,strerror(errno));
		return;
	}
	for(pm=0; pm<ModelePhysNb; pm++) strcpy(&(ModelePhysNoms[pm][0]),ModelePhys[pm].nom);
	for(vm=0; vm<ModeleVoieNb; vm++) { memcpy(&(VoieStandard[vm].def.evt),&(ModeleVoie[vm]),sizeof(TypeVoieModele)); VoieStandard[vm].vm = vm; }
	// for(vm=0; vm<ModeleVoieNb; vm++) printf("(%s) Modele[%d]: '%s', Voie standard: '%s'\n",__func__,vm,ModeleVoie[vm].nom,VoieStandard[vm].def.evt.nom);
	for(dm=0; dm<ModeleDetNb; dm++) {
		int k,ass,autre; char *c;
		ModeleDet[dm].num = dm;
		for(k=0; k<ModeleDet[dm].nbregl; k++) {
			strncpy(ModeleDet[dm].regl[k].surnom,ModeleDet[dm].regl[k].nom,DETEC_NOM_COURT);
			ModeleDet[dm].regl[k].surnom[DETEC_NOM_COURT-1] = '\0';
			c = ModeleDet[dm].regl[k].surnom;
			ItemJusquA('-',&c);
		}
		for(k=0; k<ModeleDet[dm].nbsoft; k++) {
			ModeleDet[dm].soft[k].pseudo = DetecteurCompositionDecode(0,&(ModeleDet[dm]),ModeleDet[dm].soft[k].def,&(ModeleDet[dm].soft[k].ref),&nb_erreurs);
		}
		for(ass=0; ass<ModeleDet[dm].nbassoc; ass++) {
			for(autre=0; autre<ModeleDetNb; autre++) if(!strcmp(&(ModeleDet[dm].nom_assoc[ass][0]),ModeleDet[autre].nom)) break;
			if(autre < ModeleDetNb) ModeleDet[dm].assoc[ass] = autre;
			else printf("  ! L'associe #%d du modele %s, '%s', n'est pas defini\n",ass,ModeleDet[dm].nom,&(ModeleDet[dm].nom_assoc[ass][0]));
		}
	}
	// printf("  * Fichier des modeles = {\n"); ArgIndent(1); ArgAppend(stdout,0,DetectionDesc); printf("  }\n");
}
/* ========================================================================== */
void DetecteursModlEpilogue() {
	int dm,ass;
	
	if(CompteRendu.modeles) printf("  . Examen des modeles de detecteurs\n");
	if(CompteRendu.detectr.assoc) {
		for(dm=0; dm<ModeleDetNb; dm++) {
			printf("    . Associes des detecteurs de type %s: ",ModeleDet[dm].nom);
			if(ModeleDet[dm].nbassoc == 0) printf("neant\n");
			else {
				for(ass=0; ass<ModeleDet[dm].nbassoc; ass++) { if(ass) printf(", "); printf("#%d: %s",ModeleDet[dm].assoc[ass],ModeleDet[ModeleDet[dm].assoc[ass]].nom); }
				printf("\n");
			}
		}
	}
}
/* ========================================================================== */
void DetecteursModlVerifie() {
	int dm,vm,i; char nom_complet[MAXFILENAME];
	
	if(!ModeleDetNb) {
		DetecModlStandard("");
		for(vm=0; vm<ModeleVoieNb; vm++) { memcpy(&(VoieStandard[vm].def.evt),&(ModeleVoie[vm]),sizeof(TypeVoieModele)); VoieStandard[vm].vm = vm; }
		// for(vm=0; vm<ModeleVoieNb; vm++) printf("(%s) Modele[%d]: '%s', Voie standard: '%s'\n",__func__,vm,ModeleVoie[vm].nom,VoieStandard[vm].def.evt.nom);
		for(i=0; i<ModeleDetNb; i++) {
			int k; char *c;
			for(k=0; k<ModeleDet[i].nbregl; k++) {
				strncpy(ModeleDet[i].regl[k].surnom,ModeleDet[i].regl[k].nom,DETEC_NOM_COURT);
				ModeleDet[i].regl[k].surnom[DETEC_NOM_COURT-1] = '\0';
				c = ModeleDet[i].regl[k].surnom;
				ItemJusquA('-',&c);
			}
		}
		strcpy(DetecteurFichierModeles,"ModelesDetecteurs");
		RepertoireIdentique(FichierPrefDetecteurs,DetecteurFichierModeles,nom_complet);
		printf("  . Modeles ecrits dans %s\n",nom_complet);
		ArgPrint(nom_complet,DetectionDesc);
	}
	for(i=0; i<ModelePhysNb; i++) strcpy(&(ModelePhysNoms[i][0]),ModelePhys[i].nom);
	for(vm=0; vm<ModeleVoieNb; vm++) {
		VoieStandard[vm].def.evt.std = 1;
		VoieStandard[vm].def.trmt[TRMT_AU_VOL].std = 1;
		VoieStandard[vm].def.trmt[TRMT_PRETRG].std = 1;
		VoieStandard[vm].def.trgr.std = 1;
		VoieStandard[vm].def.rgul.std = 1;
	}
	for(dm=0; dm<ModeleDetNb; dm++) ModeleDet[dm].num = dm;
}

#pragma mark --- materiel utilise ---
/* .......................................................................... */
/*                        M A T E R I E L   U T I L I S E                     */
/* ========================================================================== */
static char SambaHoteLocal(char *hote) {
	char tempo[HOTE_NOM]; char *r,*nom;
	char local;

	if(Acquis[AcquisLocale].runs[0] == 'm') return(1);
	else if(*hote) {
		strcpy(tempo,hote); r = tempo;
		do {
			nom = ItemJusquA('+',&r);
			if((local = ((!strcmp(nom,"local") || !strcmp(nom,Acquis[AcquisLocale].code)) || (!strcmp(nom,NomExterne))))) break;
		} while(*r);
	} else local = 1;
	return(local);
}
/* ========================================================================== */
static char DetecteurListeSat(TypeDetecteur *detectr) {
	int sat; char hotes[HOTE_NOM];
	char *r,*nom;

//	printf("* Le detecteur %s est lu par %s\n",detectr->nom,detectr->hote);
	if(!detectr->local) {
//		printf("  . On cherche le nom officiel de cette machine\n");
//+		if(!HostName(detectr->hote,nom)) return(0);
//		printf("  . C'est '%s'\n",nom);
//+		strcpy(detectr->hote,nom);
		strcpy(hotes,detectr->hote);
		r = hotes;
		do {
			nom = ItemJusquA('+',&r);
			for(sat=0; sat<AcquisNb; sat++) if(!strcmp(nom,Acquis[sat].code)) break;
			if(sat >= AcquisNb) {
				if(AcquisNb >= MAXSAT) return(0);
				sat = AcquisNb++;
				strncpy(Acquis[sat].code,nom,CODE_MAX); Acquis[sat].code[CODE_MAX-1] = '\0';
				sprintf(Acquis[sat].adrs,"EDWACQ-%s.local",nom);
				if(*nom == 's') Acquis[sat].runs[0] = 'a' + (*(nom + 1) - '1');
				else Acquis[sat].runs[0] = '_';
				Acquis[sat].nbrep = 0; // ajouter le repartiteur du bolo courant
#ifdef ETAT_RESEAU
				Acquis[sat].id = 0;
#endif
				// printf("  . Acquisition #%d: '%s' (total %d)\n",sat,Acquis[sat].code,AcquisNb);
			}
		} while(*r);
	}
	return(1);
}
/* ========================================================================== */
static void DetecteurParmsDefaut(DETEC_PARMS_NATURE nature) {
	switch(nature) {
		case DETEC_PARM_EVTS:
			memcpy(&(CaptParam.def.evt),&ModeleVoieDefaut,sizeof(TypeVoieModele));
			CaptParam.min = -8192; CaptParam.max = 8191;
			break;
		case DETEC_PARM_TVOL:
			CaptParam.def.trmt[TRMT_AU_VOL].type = NEANT;
			CaptParam.def.trmt[TRMT_AU_VOL].parm = 0;
			CaptParam.def.trmt[TRMT_AU_VOL].gain = 1;
			CaptParam.def.trmt[TRMT_AU_VOL].texte[0] = '\0';
		#ifdef RL
			CaptParam.def.trmt[TRMT_AU_VOL].template = 1;
		#endif
			CaptParam.exec = 0;
			break;
		case DETEC_PARM_PREP:
			CaptParam.def.trmt[TRMT_PRETRG].type = NEANT;
			CaptParam.def.trmt[TRMT_PRETRG].parm = 0;
			CaptParam.def.trmt[TRMT_PRETRG].gain = 1;
			CaptParam.def.trmt[TRMT_PRETRG].texte[0] = '\0';
			CaptParam.duree.traitees = 8192;
			CaptParam.duree.mesureLdB = 50;
			CaptParam.duree.periodes = 0;
			CaptParam.duree.post_moyenne = 1;
		#ifdef RL
			CaptParam.def.trmt[TRMT_PRETRG].template = 0; // TRMT_TEMPL_ANA;
		#endif
			break;
		case DETEC_PARM_TRGR:
			CaptParam.def.trgr.type = NEANT; CaptParam.def.trgr.sens = 1;
			CaptParam.def.trgr.origine = TRGR_INTERNE; CaptParam.def.trgr.regul_demandee = 1;
			CaptParam.def.trgr.reprise = TRGR_REPRISE_FIXE;
			CaptParam.def.trgr.coupures = 0;
			CaptParam.def.trgr.veto = 0;
			CaptParam.def.trgr.ampl_inv = CaptParam.def.trgr.mont_inv = CaptParam.def.trgr.bruit_inv = 0;
			CaptParam.def.trgr.minampl = 100.0; CaptParam.def.trgr.maxampl = -100.0;
			CaptParam.def.trgr.montee = 10; CaptParam.def.trgr.porte = 20;
			CaptParam.def.trgr.alphaampl = 40000; CaptParam.def.trgr.alphaduree = 5000.0;
			break;
		case DETEC_PARM_RGUL:
			//	CaptParam.def.rgul.type = NEANT;
			//	bzero(&CaptParam.echelle,sizeof(TypeRegulTaux)*MAXECHELLES);
			//	bzero(&CaptParam.interv,sizeof(TypeRegulInterv));
			CaptParam.def.rgul.type = TrmtRegulType;
			CaptParam.def.rgul.mininf = 1.0; CaptParam.def.rgul.maxinf = -1.0;
			CaptParam.def.rgul.minsup = 500.0; CaptParam.def.rgul.maxsup = -500.0;
			memcpy(&(CaptParam.echelle),&TrmtRegulTaux,sizeof(TypeRegulTaux)*MAXECHELLES);
			memcpy(&(CaptParam.interv),&TrmtRegulInterv,sizeof(TypeRegulInterv));
			break;
		default: break;
	}
	CaptParam.modulee = 0;
}
/* ========================================================================== */
void DetecteurSauveConfig(int config_ggale) {
	TypeConfigDefinition *config,*utilisee; int voie,classe,globale; short num_config;

	globale = config_ggale + 1;
	if(GestionConfigs) printf("(%s)  ____ Sauvegarde globale #%d (%s)\n",__func__,globale,ConfigCle[globale]);
	for(voie=0; voie<VoiesNb; voie++) {
		utilisee = &(VoieManip[voie].def);
		if((num_config = VoieManip[voie].config.num[utilisee->global.evt].evt) >= 0) {
			config = &(VoieManip[voie].config.def[num_config]);
			if(config->defini.evt) memcpy(&(config->evt),&(utilisee->evt),sizeof(TypeVoieModele));
		}
		for(classe=0; classe<TRMT_NBCLASSES; classe++) {
			num_config = VoieManip[voie].config.num[utilisee->global.trmt[classe]].trmt[classe];
			if(GestionConfigs) printf("(%s) | Traitement %s voie %s: represente globale #%d (%s) et config #%d (%s)\n",__func__,TrmtClasseNom[classe],VoieManip[voie].nom,
				utilisee->global.trmt[classe],ConfigCle[utilisee->global.trmt[classe]],num_config,(num_config >= 0)?VoieManip[voie].config.def[num_config].nom:"indefinie");
			if((num_config = VoieManip[voie].config.num[utilisee->global.trmt[classe]].trmt[classe]) >= 0) {
				config = &(VoieManip[voie].config.def[num_config]);
				if(config->defini.trmt[classe]) {
					if(GestionConfigs) printf("(%s) | Traitement %s voie %s: sauve config utilisee sur config #%d (%s)\n",__func__,TrmtClasseNom[classe],VoieManip[voie].nom,num_config,config->nom);
					memcpy(&(config->trmt[classe]),&(utilisee->trmt[classe]),sizeof(TypeTraitement));
				}
				else if(GestionConfigs) printf("(%s) | Traitement %s voie %s: config utilisee indefinie\n",__func__,TrmtClasseNom[classe],VoieManip[voie].nom);
			}
			else if(GestionConfigs) printf("(%s) | Traitement %s voie %s: config utilisee deconnectee\n",__func__,TrmtClasseNom[classe],VoieManip[voie].nom);
		}
		if((num_config = VoieManip[voie].config.num[utilisee->global.trgr].trgr) >= 0) {
			config = &(VoieManip[voie].config.def[num_config]);
			if(config->defini.trgr) memcpy(&(config->trgr),&(utilisee->trgr),sizeof(TypeTrigger));
		}
		if((num_config = VoieManip[voie].config.num[utilisee->global.rgul].rgul) >= 0) {
			config = &(VoieManip[voie].config.def[num_config]);
			if(config->defini.rgul) memcpy(&(config->rgul),&(utilisee->rgul),sizeof(TypeRegulParms));
		}
	}
	if(GestionConfigs) printf("(%s) |____ sauvegarde terminee\n",__func__);
}
/* ========================================================================== */
static char *DetecteurCanalExporte(int num_config, DETEC_NATURE reel, DETEC_PARMS_NATURE nature, int canal) {
	int vm,voie,bolo,cap,j,k; char *ptr_status;
	TypeDetecteur *detectr; TypeConfigVoie *config_voie; TypeConfigDefinition *exportee,*utilisee;

	ptr_status = &ParmAJeter; voie = vm = 0;
	if(reel) { voie = canal; config_voie = &(VoieManip[voie].config); utilisee = &(VoieManip[voie].def); }
	else { vm = canal; config_voie = &(VoieStandard[vm].config); utilisee = &(VoieStandard[vm].def); }
	if(num_config >= 0) exportee = &(config_voie->def[num_config]); else exportee = utilisee;
	if(GestionConfigs) printf("(%s) %s: exporte config #%d (%s)\n",__func__,DetecteurParmsTitre[nature],num_config,exportee->nom);

	switch(nature) {
	  case DETEC_PARM_EVTS:
		memcpy(&(CaptParam.def.evt),&(exportee->evt),sizeof(TypeVoieModele));
		if(exportee->defini.evt) ptr_status = reel? &(CaptParam.def.evt.std): &ParmASauver;
		else ptr_status = &ParmAJeter;
		if(reel) { CaptParam.min = VoieManip[voie].min; CaptParam.max = VoieManip[voie].max; }
		else { CaptParam.min = VoieStandard[vm].min; CaptParam.max = VoieStandard[vm].max; }
		break;
	  case DETEC_PARM_TVOL:
		memcpy(&(CaptParam.def.trmt[TRMT_AU_VOL]),&(exportee->trmt[TRMT_AU_VOL]),sizeof(TypeTraitement));
		if(exportee->defini.trmt[TRMT_AU_VOL]) ptr_status = reel? &(CaptParam.def.trmt[TRMT_AU_VOL].std): &ParmASauver;
		else ptr_status = &ParmAJeter;
		if(reel) CaptParam.exec = VoieManip[voie].exec; else CaptParam.exec = VoieStandard[vm].exec;
		break;
	  case DETEC_PARM_PREP:
		memcpy(&(CaptParam.def.trmt[TRMT_PRETRG]),&(exportee->trmt[TRMT_PRETRG]),sizeof(TypeTraitement));
		if(exportee->defini.trmt[TRMT_PRETRG]) ptr_status = reel? &(CaptParam.def.trmt[TRMT_PRETRG].std): &ParmASauver;
		else ptr_status = &ParmAJeter;
		if(reel) memcpy(&CaptParam.duree,&(VoieManip[voie].duree),sizeof(TypeVoieDurees));
		else memcpy(&CaptParam.duree,&(VoieStandard[vm].duree),sizeof(TypeVoieDurees));
		break;
	  case DETEC_PARM_TRGR:
		memcpy(&(CaptParam.def.trgr),&(exportee->trgr),sizeof(TypeTrigger));
		if(exportee->defini.trgr) ptr_status = reel? &(CaptParam.def.trgr.std): &ParmASauver;
		else ptr_status = &ParmAJeter;
		break;
	  case DETEC_PARM_RGUL:
		memcpy(&(CaptParam.def.rgul),&(exportee->rgul),sizeof(TypeRegulParms));
		if(exportee->defini.rgul) ptr_status = reel? &(CaptParam.def.rgul.std): &ParmASauver;
		else ptr_status = &ParmAJeter;
		if(reel) {
			for(j=0; j<MAXECHELLES; j++) memcpy(&(CaptParam.echelle[j]),&(VoieManip[voie].echelle[j]),sizeof(TypeRegulTaux));
			memcpy(&(CaptParam.interv),&(VoieManip[voie].interv),sizeof(TypeRegulInterv));
		} else {
			for(j=0; j<MAXECHELLES; j++) memcpy(&(CaptParam.echelle[j]),&(VoieStandard[vm].echelle[j]),sizeof(TypeRegulTaux));
			memcpy(&(CaptParam.interv),&(VoieStandard[vm].interv),sizeof(TypeRegulInterv));
		}
		break;
	  case DETEC_PARM_SOFT:
		if((bolo = canal) >= 0) {
			PseudoNb = 0;
			for(cap=0; cap<Bolo[bolo].nbcapt; cap++) {
				voie = Bolo[bolo].captr[cap].voie;
				if(VoieManip[voie].pseudo) {
					strcpy(Pseudo[PseudoNb].nom,Bolo[bolo].captr[cap].nom);
					strcpy(Pseudo[PseudoNb].composition,DetecteurCompositionEncode(VoieManip[voie].pseudo));
					PseudoNb++;
				}
			}
		}
		break;
	  case DETEC_PARM_GENL:
		if((bolo = canal) >= 0) {
			detectr = &(Bolo[bolo]);
			//- if(!strcmp(detectr->hote,"local")) strcpy(DetecteurLu.hote,detectr->hote);
			//- else strncat(strncpy(DetecteurLu.hote,"EDWACQ-",HOTE_NOM-8),detectr->hote,HOTE_NOM-8);
			// strcpy(DetecteurLu.hote,detectr->hote);
			DetecteurLu.masse = detectr->masse;
			DetecteurLu.mode_arch = detectr->mode_arch;
			if(detectr->start.script[0]) strcpy(DetecteurLu.start.script,detectr->start.script);
			else strcpy(DetecteurLu.start.script,"neant");
			if(detectr->regul.script[0]) strcpy(DetecteurLu.regul.script,detectr->regul.script);
			else strcpy(DetecteurLu.regul.script,"neant");
		}
		break;
	  case DETEC_PARM_ENVR:
		if((bolo = canal) >= 0) {
			detectr = &(Bolo[bolo]);
			if(detectr->assoc_imposes) {
				DetecteurLu.nbassoc = detectr->nbassoc;
				for(k=0; k<detectr->nbassoc; k++) strcpy(&(DetecteurLu.nom_assoc[k][0]),&(detectr->nom_assoc[k][0]));
			} else DetecteurLu.nbassoc = 0;
			if(detectr->voisins_imposes) {
				DetecteurLu.nbvoisins = detectr->nbvoisins;
				for(k=0; k<detectr->nbvoisins; k++) strcpy(&(DetecteurLu.nomvoisins[k][0]),&(detectr->nomvoisins[k][0]));
			} else DetecteurLu.nbvoisins = 0;
		}
		break;
	  default: break;
	}
	if(DetecDebugStd) printf("(%s) %s %s %sstandard [@%08X]\n",__func__,DetecteurParmsTitre[nature],
		   reel?VoieManip[voie].nom:ModeleVoie[vm].nom,ptr_status?(*ptr_status?"au  ":"non-"):"sans ",(hexa)ptr_status);
	return(ptr_status);
}
/* ========================================================================== */
static void DetecteurCanalImporte(int num_config, char ggale, DETEC_NATURE reel, DETEC_PARMS_NATURE nature, int canal) {
	int voie,bolo,vm,j; char globale; TypeDetecteur *detectr;
	TypeConfigVoie *config_voie; TypeConfigDefinition *importee,*utilisee,*usuelle;

	voie = vm = 0; // GCC
	if(reel) {
		voie = canal; config_voie = &(VoieManip[voie].config); utilisee = &(VoieManip[voie].def); // usuelle = &(VoieManip[voie].config.usuelle);
	} else {
		vm = canal; config_voie = &(VoieStandard[vm].config); utilisee = &(VoieStandard[vm].def); // usuelle = &(VoieStandard[vm].config.usuelle);
	}
	if(num_config >= 0) importee = &(config_voie->def[num_config]); else importee = utilisee;
	usuelle = &(config_voie->def[0]);
	globale = ggale + 1;
	if(GestionConfigs) printf("(%s) %s %s: config #%d (%s) = globale #%d (%s)\n",__func__,DetecteurParmsTitre[nature],VoieManip[voie].nom,
		   num_config,importee->nom,globale,ConfigCle[globale]);
	switch(nature) {
	  case DETEC_PARM_EVTS:
		memcpy(&(importee->evt),&(CaptParam.def.evt),sizeof(TypeVoieModele));
		importee->defini.evt = 1; importee->global.evt = globale;
		config_voie->num[globale].evt = num_config;
		if(!utilisee->defini.evt) {
			memcpy(&(utilisee->evt),&(importee->evt),sizeof(TypeVoieModele));
			utilisee->defini.evt = 1; utilisee->global.evt = globale;
		}
		if(reel) {
			VoieManip[voie].min = CaptParam.min; VoieManip[voie].max = CaptParam.max;
			VoieManip[voie].def.evt.std = 0;
			if(DetecDebugStd) printf("(%s) %s %s non-standard [@%08X]\n",__func__,
				DetecteurParmsTitre[nature],VoieManip[voie].nom,(hexa)(&(VoieManip[voie].def.evt.std)));
		} else {
			VoieStandard[vm].min = CaptParam.min; VoieStandard[vm].max = CaptParam.max;
		}
		break;
	  case DETEC_PARM_TVOL:
		memcpy(&(importee->trmt[TRMT_AU_VOL]),&(CaptParam.def.trmt[TRMT_AU_VOL]),sizeof(TypeTraitement));
		importee->defini.trmt[TRMT_AU_VOL] = 1; importee->global.trmt[TRMT_AU_VOL] = globale;
		config_voie->num[globale].trmt[TRMT_AU_VOL] = num_config;
		if(!utilisee->defini.trmt[TRMT_AU_VOL]) {
			memcpy(&(utilisee->trmt[TRMT_AU_VOL]),&(importee->trmt[TRMT_AU_VOL]),sizeof(TypeTraitement));
			utilisee->defini.trmt[TRMT_AU_VOL] = 1; utilisee->global.trmt[TRMT_AU_VOL] = globale;
		}
		if(reel) {
			VoieManip[voie].exec = CaptParam.exec;
			VoieManip[voie].def.trmt[TRMT_AU_VOL].std = 0;
			if(DetecDebugStd) printf("(%s) %s %s non-standard [@%08X]\n",__func__,DetecteurParmsTitre[nature],
				   reel?VoieManip[voie].nom:ModeleVoie[vm].nom,(hexa)(&(VoieManip[voie].def.trmt[TRMT_AU_VOL].std)));
		} else {
			VoieStandard[vm].exec = CaptParam.exec;
		}
		break;
	  case DETEC_PARM_PREP:
		memcpy(&(importee->trmt[TRMT_PRETRG]),&(CaptParam.def.trmt[TRMT_PRETRG]),sizeof(TypeTraitement));
		importee->defini.trmt[TRMT_PRETRG] = 1; importee->global.trmt[TRMT_PRETRG] = globale;
		config_voie->num[globale].trmt[TRMT_PRETRG] = num_config;
		if(!utilisee->defini.trmt[TRMT_PRETRG]) {
			memcpy(&(utilisee->trmt[TRMT_PRETRG]),&(importee->trmt[TRMT_PRETRG]),sizeof(TypeTraitement));
			utilisee->defini.trmt[TRMT_PRETRG] = 1; utilisee->global.trmt[TRMT_PRETRG] = globale;
		}
		if(reel) {
			memcpy(&(VoieManip[voie].duree),&CaptParam.duree,sizeof(TypeVoieDurees));
			VoieManip[voie].def.trmt[TRMT_PRETRG].std = 0;
			if(DetecDebugStd) printf("(%s) %s %s non-standard [@%08X]\n",__func__,DetecteurParmsTitre[nature],
				   reel?VoieManip[voie].nom:ModeleVoie[vm].nom,(hexa)(&(VoieManip[voie].def.trmt[TRMT_PRETRG].std)));
		} else {
			memcpy(&(VoieStandard[vm].duree),&CaptParam.duree,sizeof(TypeVoieDurees));
		}
		// printf("(%s) VoieManip[%s].duree.traitees = %d\n",__func__,VoieManip[voie].nom,VoieManip[voie].duree.traitees);
		break;
	  case DETEC_PARM_TRGR:
		memcpy(&(importee->trgr),&(CaptParam.def.trgr),sizeof(TypeTrigger));
		importee->defini.trgr = 1; importee->global.trgr = globale;
		config_voie->num[globale].trgr = num_config;
		if(!utilisee->defini.trgr) {
			memcpy(&(utilisee->trgr),&(importee->trgr),sizeof(TypeTrigger));
			utilisee->defini.trgr = 1; utilisee->global.trgr = globale;
		}
		if(reel) {
			if(VoieManip[voie].def.trgr.std) strcpy(VoieManip[voie].txttrgr,"standard"); else SambaTrgrEncode(&(VoieManip[voie].def.trgr),VoieManip[voie].txttrgr,TRGR_NOM);
			VoieManip[voie].def.trgr.std = 0;
			if(DetecDebugStd) printf("(%s) %s %s non-standard [@%08X]\n",__func__,DetecteurParmsTitre[nature],
				   reel?VoieManip[voie].nom:ModeleVoie[vm].nom,(hexa)(&(VoieManip[voie].def.trgr.std)));
		} else {
			SambaTrgrEncode(&(VoieStandard[vm].def.trgr),VoieStandard[vm].txttrgr,TRGR_NOM);
		}
		break;
	  case DETEC_PARM_RGUL:
		memcpy(&(importee->rgul),&(CaptParam.def.rgul),sizeof(TypeRegulParms));
		importee->defini.rgul = 1; importee->global.rgul = globale;
		config_voie->num[globale].rgul = num_config;
		if(!utilisee->defini.rgul) {
			memcpy(&(utilisee->rgul),&(importee->rgul),sizeof(TypeRegulParms));
			utilisee->defini.rgul = 1; utilisee->global.rgul = globale;
		}
		if(reel) {
			for(j=0; j<MAXECHELLES; j++) memcpy(&(VoieManip[voie].echelle[j]),&(CaptParam.echelle[j]),sizeof(TypeRegulTaux));
			memcpy(&(VoieManip[voie].interv),&(CaptParam.interv),sizeof(TypeRegulInterv));
			VoieManip[voie].def.rgul.std = 0;
			if(DetecDebugStd) printf("(%s) %s %s non-standard [@%08X]\n",__func__,DetecteurParmsTitre[nature],
				   reel?VoieManip[voie].nom:ModeleVoie[vm].nom,(hexa)(&(VoieManip[voie].def.rgul.std)));
		} else {
			for(j=0; j<MAXECHELLES; j++) memcpy(&(VoieStandard[vm].echelle[j]),&(CaptParam.echelle[j]),sizeof(TypeRegulTaux));
			memcpy(&(VoieStandard[vm].interv),&(CaptParam.interv),sizeof(TypeRegulInterv));
		}
		break;
	  case DETEC_PARM_SOFT:
		break;
	  case DETEC_PARM_GENL:
		if((bolo = canal) >= 0) {
			detectr = &(Bolo[bolo]);
			detectr->masse = DetecteurLu.masse;
			detectr->mode_arch = DetecteurLu.mode_arch;
			if(!strcmp(DetecteurLu.start.script,"neant")) detectr->start.script[0] = '\0';
			else strcpy(detectr->start.script,DetecteurLu.start.script);
			if(!strcmp(DetecteurLu.regul.script,"neant")) detectr->regul.script[0] = '\0';
			else strcpy(detectr->regul.script,DetecteurLu.regul.script);
		}
		break;
	  case DETEC_PARM_ENVR:
		if((bolo = canal) >= 0) {
			detectr = &(Bolo[bolo]);
			detectr->voisins_imposes = (DetecteurLu.nbvoisins > 0);
			if(detectr->voisins_imposes) {
				detectr->nbvoisins = DetecteurLu.nbvoisins;
				memcpy(detectr->nomvoisins,DetecteurLu.nomvoisins,DETEC_VOISINS_MAX*DETEC_NOM);
			}
			detectr->assoc_imposes = (DetecteurLu.nbassoc > 0);
			if(detectr->assoc_imposes) {
				detectr->nbassoc = DetecteurLu.nbassoc;
				memcpy(detectr->nom_assoc,DetecteurLu.nom_assoc,DETEC_ASSOC_MAX*DETEC_NOM);
			}
		}
		break;
	  default: break;
	}
}
/* ========================================================================== */
char *DetecteurCanalStandardise(DETEC_PARMS_NATURE nature, int bolo, int cap, char actn) {
	int voie,vm,j; char *nom_prevu,*nom;
	
	nom = 0;
	if((voie = Bolo[bolo].captr[cap].voie) < 0) return(0);
	if(VoieManip[voie].pseudo) nom_prevu = Bolo[bolo].captr[cap].nom;
	else nom_prevu = VoieManip[voie].def.evt.nom;
	vm = VoieManip[voie].type;
	switch(nature) {
	  case DETEC_PARM_EVTS: 
		if((actn == 1) || (actn == -1 && VoieManip[voie].def.evt.std)) {
			memcpy(&(VoieManip[voie].def.evt),&(VoieStandard[vm].def.evt),sizeof(TypeVoieModele));
			nom = nom_prevu;
		}
		if(DetecDebugStd) printf("(%s) %s %s %sstandard [@%08X]\n",__func__,DetecteurParmsTitre[nature],
			VoieManip[voie].nom,(VoieManip[voie].def.evt.std)?"au  ":"non-",(hexa)(&(VoieManip[voie].def.evt.std)));
		break;
	  case DETEC_PARM_TVOL: 
		if((actn == 1) || (actn == -1 && VoieManip[voie].def.trmt[TRMT_AU_VOL].std)) {
			memcpy(&(VoieManip[voie].def.trmt[TRMT_AU_VOL]),&(VoieStandard[vm].def.trmt[TRMT_AU_VOL]),sizeof(TypeTraitement));
			VoieManip[voie].exec = VoieStandard[vm].exec;
			nom = nom_prevu;
		}
		if(DetecDebugStd) printf("(%s) %s %s %sstandard [@%08X]\n",__func__,DetecteurParmsTitre[nature],
			VoieManip[voie].nom,(VoieManip[voie].def.trmt[TRMT_AU_VOL].std)?"au  ":"non-",(hexa)(&(VoieManip[voie].def.trmt[TRMT_AU_VOL].std)));
		break;
	  case DETEC_PARM_PREP: 
		if((actn == 1) || (actn == -1 && VoieManip[voie].def.trmt[TRMT_PRETRG].std)) {
			memcpy(&(VoieManip[voie].def.trmt[TRMT_PRETRG]),&(VoieStandard[vm].def.trmt[TRMT_PRETRG]),sizeof(TypeTraitement));
			memcpy(&(VoieManip[voie].duree),&(VoieStandard[vm].duree),sizeof(TypeVoieDurees));
			nom = nom_prevu;
		}
		if(DetecDebugStd) printf("(%s) %s %s %sstandard [@%08X]\n",__func__,DetecteurParmsTitre[nature],
			VoieManip[voie].nom,(VoieManip[voie].def.trmt[TRMT_PRETRG].std)?"au  ":"non-",(hexa)(&(VoieManip[voie].def.trmt[TRMT_PRETRG].std)));
		break;
	  case DETEC_PARM_TRGR: 
		if((actn == 1) || (actn == -1 && VoieManip[voie].def.trgr.std)) {
			memcpy(&(VoieManip[voie].def.trgr),&(VoieStandard[vm].def.trgr),sizeof(TypeTrigger));
			nom = nom_prevu;
		}
		if(DetecDebugStd) printf("(%s) %s %s %sstandard [@%08X]\n",__func__,DetecteurParmsTitre[nature],
			VoieManip[voie].nom,(VoieManip[voie].def.trgr.std)?"au  ":"non-",(hexa)(&(VoieManip[voie].def.trgr.std)));
		break;
	  case DETEC_PARM_RGUL: 
		if((actn == 1) || (actn == -1 && VoieManip[voie].def.rgul.std)) {
			memcpy(&(VoieManip[voie].def.rgul),&(VoieStandard[vm].def.rgul),sizeof(TypeRegulParms));
			for(j=0; j<MAXECHELLES; j++) memcpy(&(VoieManip[voie].echelle[j]),&(VoieStandard[vm].echelle[j]),sizeof(TypeRegulTaux));
			memcpy(&(VoieManip[voie].interv),&(VoieStandard[vm].interv),sizeof(TypeRegulInterv));
			nom = nom_prevu;
		}
		if(DetecDebugStd) printf("(%s) %s %s %sstandard [@%08X]\n",__func__,DetecteurParmsTitre[nature],
			VoieManip[voie].nom,(VoieManip[voie].def.rgul.std)?"au  ":"non-",(hexa)(&(VoieManip[voie].def.rgul.std)));
		break;
	  default: break;
	}
	if(nom) Bolo[bolo].a_sauver = 1;
	return(nom);
}
/* ========================================================================== */
static int DetecteurCapteurUtilise(TypeDetecteur *detectr, int dm, char *fichier_lu, char vrai_bolo, char *ref, int *vmvue) {
	int vm,cap; char *org;

	org = ref;
	if(DicoTraduit(SambaDicoDefs + SAMBA_DICO_VOIES,&ref)) printf("  * '%s' a ete traduit par '%s'\n",org,ref);
	if(!strcmp(ref,"detecteur")) { *vmvue = -1; return(0); }
	
	for(cap=0; cap<detectr->nbcapt; cap++) if(!strcmp(detectr->captr[cap].nom,ref)) break;
	if(cap < detectr->nbcapt) { *vmvue = detectr->captr[cap].modele; return(cap); }
	
	for(vm=0; vm<ModeleVoieNb; vm++) if(!strcmp(ModeleVoie[vm].nom,ref)) break;
	*vmvue = vm;
	if(vm == ModeleVoieNb) { printf("  ! Modele de voie inconnu (%s) trouve dans le fichier '%s'\n",ref,fichier_lu); return(-1); }
	if(!vrai_bolo) return(vm);
	
	for(cap=0; cap<detectr->nbcapt; cap++) if(detectr->captr[cap].modele == vm) break;
	if(cap == detectr->nbcapt) {
		printf("  ! Pas de voie '%s' dans le detecteur %s (de type %s), lu dans le fichier '%s'\n",ref,detectr->nom,ModeleDet[dm].nom,fichier_lu);
		return(-1);
	}
	return(cap);
}
/* ========================================================================== */
static char DetecteurCreeTempo(char *nom, int dm, char *hote, int *nb_erreurs) {
	int bolo; TypeDetecteur *detectr; char ok;

	ok = 1;
	bolo = BoloNb++;
	detectr = Bolo + bolo;
	memcpy(detectr,DetecteurStandardLocal,sizeof(TypeDetecteur)); /* ReglageRazBolo(detectr,nom); */
	/* Creation d'un detecteur par defaut: */
	snprintf(detectr->fichier,MAXFILENAME,"%s",nom);
	strncpy(detectr->nom,nom,DETEC_NOM);
	strncpy(detectr->hote,hote,HOTE_NOM);
	detectr->local = SambaHoteLocal(detectr->hote);
	detectr->type = dm;
	detectr->numero = bolo;
	detectr->ref = -1;
	detectr->pos = CABLAGE_INDEFINI;
	detectr->avec_regen = ModeleDet[dm].avec_regen;
	strcpy(detectr->adresse,CablageEncodePosition(detectr->pos));
	if(!DetecteurAjouteVoies(detectr,nb_erreurs)) ok = 0;
	DetecteurAjouteReglages(detectr,nb_erreurs,0);
	detectr->lu = DETEC_OK;
	detectr->a_sauver = 1;
	// printf("  * Fichier prevu: %s, pour le detecteur #%d (%s) [soit %d detecteur%s]\n",detectr->fichier,bolo,detectr->nom,Accord1s(BoloNb));
	return(ok);
}
/* ========================================================================== */
char *DetecteurCompositionEncode(TypeComposantePseudo *pseudo) {
	TypeComposantePseudo *composante; char texte[80]; int k,l; int voie,bolo,cap;

	composante = pseudo; k = 0;
	while(composante) {
		if(composante != pseudo) {
			if(composante->coef > 0.0) l = snprintf(texte,80," +"); else l = snprintf(texte,80," ");
			// printf("(%s) texte[%d] ajoute: {%s} en colonne %d\n",__func__,l,texte,k);
			if((k + l) < DETEC_COMPO_MAX) { strcpy(DetecTexteComposition+k,texte); k += l; } else break;
			// printf("(%s) soit DetecTexteComposition = {%s}\n",__func__,DetecTexteComposition);
		}
		voie = composante->srce; bolo = VoieManip[voie].det; cap = VoieManip[voie].cap;
		if(composante->coef == 1.0)
			l = snprintf(texte,80," 1 * %s",Bolo[bolo].captr[cap].nom); // l = snprintf(texte,80," %s",Bolo[bolo].captr[cap].nom);
		else if(composante->coef == -1.0)
			l = snprintf(texte,80," -1 * %s",Bolo[bolo].captr[cap].nom); // l = snprintf(texte,80," -%s",Bolo[bolo].captr[cap].nom);
		else l = snprintf(texte,80,"%g * %s",composante->coef,Bolo[bolo].captr[cap].nom);
		// printf("(%s) texte[%d] ajoute: {%s} en colonne %d\n",__func__,l,texte,k);
		if((k + l) < DETEC_COMPO_MAX) { strcpy(DetecTexteComposition+k,texte); k += l; } else break;
		// printf("(%s) soit DetecTexteComposition = {%s}\n",__func__,DetecTexteComposition);
		composante = composante->svt;
	}
	// printf("(%s) DetecTexteComposition[%d] = {%s}\n",__func__,k,DetecTexteComposition);
	DetecTexteComposition[k] = '\0';
	return(DetecTexteComposition);
}
/* ========================================================================== */
static TypeComposantePseudo *DetecteurCompositionDecode(char niveau, void *adrs, char *texte, int *ref, int *nb_erreurs) {
	TypeComposantePseudo *pseudo,*composant,*svt; float coef; char signe_svt,signe_prec;
	char *r,*decla,*nom; int vm,cap,nbcapt;
	TypeDetecteur *detectr; TypeDetModele *det_modele; 
	
	det_modele = 0; detectr = 0; // GCC
	if(niveau) { detectr = (TypeDetecteur *)adrs; nbcapt = detectr->nbcapt; }
	else { det_modele = (TypeDetModele *)adrs; nbcapt = det_modele->nbcapt; }
	pseudo = composant = 0;
	*ref = -1;
	signe_prec = '+';
	r = texte; /* '"' F<i> {'x'|'*'} <capteur> [ {'+'|'-'} F<i> {'x'|'*'} <capteur> ] '"' (!! entre guillemets !!) */
	do {
		// sscanf(ItemJusquA('x',&r),"%g",&coef);
		decla = ItemAvant("x*",&signe_svt,&r); /* ou bien: <capteur> */
		if(signe_svt) {
			sscanf(decla,"%g",&coef);
			nom = ItemAvant("+-",&signe_svt,&r);
		} else {
			coef = 1.0;
			nom = decla;
			r = decla; ItemAvant("+-",&signe_svt,&r);
		}
		if(nom[0] == '\0') break;
		for(cap=0; cap<nbcapt; cap++) {
			if(niveau) { if(!strcmp(detectr->captr[cap].nom,nom)) break; }
			else { if(!strcmp(ModeleVoie[det_modele->type[cap]].nom,nom)) break; }
		}
		if(cap < nbcapt) {
			vm = niveau? detectr->captr[cap].modele: det_modele->type[cap];
			if(*ref < 0) *ref = vm;
			else if(ModeleVoie[vm].physique != ModeleVoie[*ref].physique) {
				printf("  ! La voie %s est du type %s, et %s est du type %s\n",
					   nom,ModelePhys[ModeleVoie[vm].physique].nom,
					   ModeleVoie[*ref].nom,ModelePhys[ModeleVoie[*ref].physique].nom);
				*nb_erreurs += 1;
				continue;
			}
			svt = (TypeComposantePseudo *)malloc(sizeof(TypeComposantePseudo));
			if(!svt) break;
			if(composant) composant->svt = svt;
			composant = svt;
			if(!pseudo) pseudo = composant;
			composant->coef = (signe_prec == '-')? -coef: coef;
			composant->srce = cap;
			composant->svt = 0;
		} else {
			printf("  ! %s n'est pas une voie de %s\n",nom,niveau? detectr->nom: det_modele->nom);
			*nb_erreurs += 1;
		}
		signe_prec = signe_svt;
	} while(*r);
	
	return(pseudo);
}
/* ========================================================================== */
static void DetecteurAjoutePseudo(TypeDetecteur *detectr, TypeComposantePseudo *pseudo, char *ref, int vm, int *nb_erreurs) {
	int cap,voie;
	short srce; float mVenADU,gain;
	TypeComposantePseudo *elt_cap,*elt_voie,*svt;
	
	srce = -1; // GCC
	if(pseudo) {
		for(cap=0; cap<detectr->nbcapt; cap++) if(!strcmp(detectr->captr[cap].nom,ref)) break;
		if(cap < detectr->nbcapt) {
			voie = detectr->captr[cap].voie;
			DetecteurSimplifieVoie(voie);
		} else {
			if((voie = DetecteurVoieCree(detectr,detectr->nbcapt,vm,ref)) < 0) {
				printf("  ! Pseudo voie '%s' abandonnee\n",ref);
				*nb_erreurs += 1; return;
			}
		}
		// if(!DetecteurVoieCompose(voie,pseudo)) { nb_erreurs++; continue; }
		mVenADU = 0.0; gain = 0.0;
		cap = VoieManip[voie].cap;
		elt_cap = pseudo; elt_voie = 0;
		while(elt_cap) {
			svt = (TypeComposantePseudo *)malloc(sizeof(TypeComposantePseudo));
			if(!svt) {
				printf("! Manque de memoire pour un element de la pseudo-voie %s %s\n",detectr->captr[cap].nom,detectr->nom);
				*nb_erreurs += 1;
				return;
			}
			if(elt_voie) elt_voie->svt = svt;
			elt_voie = svt;
			elt_voie->coef = elt_cap->coef;
			elt_voie->srce = detectr->captr[elt_cap->srce].voie;
			elt_voie->svt = 0;
			if(!VoieManip[voie].pseudo) { VoieManip[voie].pseudo = elt_voie; srce = elt_voie->srce; }
			if(elt_voie->srce >= 0) {
				if(VoieManip[elt_voie->srce].ADUenmV) mVenADU += (elt_voie->coef / VoieManip[elt_voie->srce].ADUenmV);
				gain += (elt_voie->coef * VoieManip[elt_voie->srce].gain_fixe);
			}
			elt_cap = elt_cap->svt;
		}
		VoieManip[voie].def.trmt[TRMT_AU_VOL].std = 0;
		VoieManip[voie].def.trmt[TRMT_AU_VOL].type = NEANT;
		VoieManip[voie].type_adc = (srce >= 0)? VoieManip[srce].type_adc :0;
		VoieManip[voie].signe = (srce >= 0)? VoieManip[srce].signe :1;
		VoieManip[voie].ADCmask = (srce >= 0)? VoieManip[srce].ADCmask :0xFFFF;
		VoieManip[voie].extens = (srce >= 0)? VoieManip[srce].extens :0;
		VoieManip[voie].zero = (srce >= 0)? VoieManip[srce].zero :0x8000;
		if((mVenADU > 0.0) || (mVenADU < 0.0)) VoieManip[voie].ADUenmV = 1.0 / mVenADU;
		else VoieManip[voie].ADUenmV = (srce >= 0)? VoieManip[srce].ADUenmV :1; /* valeur d'un ADU en mV */
		VoieManip[voie].gain_fixe = (gain > 0.0)? gain: 1.0;
	}
}
/* ========================================================================== */
static int DetecteurDeclare(TypeDetecteur *detectr, int bolo, char *nom, int dm, char *ref) {
	int j,lngr; int prec,dm2; int nb_erreurs;

	nb_erreurs = 0;
	memcpy(detectr,DetecteurStandardLocal,sizeof(TypeDetecteur)); /* ReglageRazBolo(detectr,nom); */
    detectr->numero = bolo;
	detectr->sel = 0;
	detectr->type = dm;
	detectr->masse = ModeleDet[dm].masse;
	detectr->avec_regen = ModeleDet[dm].avec_regen;
	detectr->exec.inst = 0; detectr->exec.boucle = 0; detectr->exec.date = -1;
	/* reference dans la declaration: on rempli avec */
	if(ref) {
		if(*ref != '\0') {
			for(j=0; j<BoloNb; j++) if(!strcmp(ref,Bolo[j].nom)) break;
			if(j < BoloNb) {
				memcpy(detectr,Bolo + j,sizeof(TypeDetecteur));
				detectr->ref = j;
			} else {
				printf("  ! Le detecteur de reference (%s) pour %s n'est pas connu!\n",ref,nom);
				nb_erreurs++;
			}
		}
	}
	detectr->fichier[0] = '\0';
	/* attribution du nom */
	strncpy(detectr->nom,nom,DETEC_NOM);
	lngr = strlen(ModeleDet[dm].nom);
	if(!strncmp(nom,ModeleDet[dm].nom,lngr)) {
		int courant;
		if(nom[lngr] == '_') lngr++;
		sscanf(nom+lngr,"%d",&courant);
		if(courant >= ModeleDet[dm].suivant) ModeleDet[dm].suivant = courant + 1;
	}
	/* association avec un detecteur deja declare */
	if(ModeleDet[dm].nbassoc) {
		int k,m,assoc;
		for(prec=0; prec<BoloNb-1; prec++) {
			dm2 = Bolo[prec].type;
			if(dm2 == dm) continue;
			for(k=0; k<ModeleDet[dm2].nbassoc; k++) {
				if(ModeleDet[dm2].assoc[k] == dm) {
					for(m=0; m<Bolo[prec].nbassoc; m++) {
						assoc = Bolo[prec].assoc[m];
						if(Bolo[assoc].type == dm) break;
					}
					if(m < Bolo[prec].nbassoc) continue;
					Bolo[prec].assoc[Bolo[prec].nbassoc++] = bolo;
					detectr->assoc[detectr->nbassoc++] = prec;
					break;
				}
			}
		}
	}
	BoloNb++;

	return(nb_erreurs);
}
/* ========================================================================== */
void DetecteurInfo(TypeDetecteur *detectr, char *hote, char *nom_partiel, char a_sauver) {
	strncpy(detectr->hote,hote,HOTE_NOM);
	detectr->local = SambaHoteLocal(detectr->hote);
	if(*nom_partiel) strncpy(detectr->fichier,nom_partiel,MAXFILENAME);
	if(CompteRendu.detectr.fichier) {
		if(detectr->fichier[0])
			printf("  . Detecteur %s memorise separement dans '%s'\n",detectr->nom,detectr->fichier);
		else printf("  . Detecteur %s memorise en commun  dans '%s'\n",detectr->nom,FichierPrefDetecteurs);
	}
	detectr->start.script[0] = '\0';
	detectr->regul.script[0] = '\0';
	detectr->a_sauver = a_sauver;
}
/* ========================================================================== */
static char DetecteurAjouteVoies(TypeDetecteur *detectr, int *nb_erreurs) {
	int cap,vm,dm;

	dm = detectr->type;
	/* nombre et modeles des voies a lire */
	if(ModeleDet[dm].nbcapt <= DETEC_CAPT_MAX) detectr->nbcapt = ModeleDet[dm].nbcapt;
	else {
		detectr->nbcapt = DETEC_CAPT_MAX;
		printf("  ! %d voies maxi par detecteur (DETEC_CAPT_MAX)\n",DETEC_CAPT_MAX);
		*nb_erreurs += 1;
	}
	/* creation des voies associees a chaque capteur */
	for(cap=0; cap<detectr->nbcapt; cap++) {
		detectr->captr[cap].bolo = detectr->numero;
		detectr->captr[cap].capt = cap;
		vm = ModeleDet[dm].type[cap];
		if(DetecteurVoieCree(detectr,cap,vm,ModeleVoie[vm].nom) < 0) {
			*nb_erreurs += 1; return(0);
		}
	}

	return(1);
}
/* ========================================================================== */
static void DetecteurAjouteReglages(TypeDetecteur *detectr, int *nb_erreurs, char log) {
	int cap,dm; int i;
	TypeReglage *regl;

	dm = detectr->type;
	detectr->nbreglages = ModeleDet[dm].nbregl;
	if(log) printf("    . %s @%s, %d reglage%s\n",detectr->nom,detectr->adresse,Accord1s(detectr->nbreglages));
	if(detectr->nbreglages) {
		for(i=0, regl=detectr->reglage; i<detectr->nbreglages; i++,regl++) {
			strcpy(regl->nom,ModeleDet[dm].regl[i].nom);
			regl->num = i;
			regl->bolo = detectr->numero;
			for(cap=0; cap<detectr->nbcapt; cap++) if(detectr->captr[cap].modele == ModeleDet[dm].regl[i].capteur) break;
			if(cap >= detectr->nbcapt) cap = 0;
			regl->capt = cap; // ModeleDet[dm].regl[i].capteur;
			regl->cmde = ModeleDet[dm].regl[i].cmde;
			// regl->mval = (regl->cmde == REGL_D2)? 100.0: ((regl->cmde == REGL_D3)? 1000.0: 0.0);
			regl->bb = -1;
			regl->ss_adrs = regl->negatif = regl->identik = 0;
			regl->ress = regl->ressneg = regl->resspos = -1;
			regl->std[0] = regl->user[0] = '\0';
			regl->val.i = 0;
			regl->valeurs = 0;
		}
	}
}
/* ========================================================================== */
static void DetecteurBrancheDirect(TypeDetecteur *detectr, char type_lien, char *item, FILE *fichier, int *nb_erreurs) {
	int bb,cap;
	TypeBNModele *modele_bn;
	TypeCablePosition pos_hexa; int m;
	unsigned short ident,adrs;
	char *c,*d;

	if(!item) {
		printf("  ! Le numeriseur directement branche sur le detecteur %s n'est pas precise\n",detectr->nom);
		*nb_erreurs += 1;
		return;
	} else printf("  * Detecteur %s: %c numeriseur '%s'\n",detectr->nom,type_lien,item);
	detectr->pos = CABLAGE_POS_DIR;
	cap = 0;
	c = item; ItemSuivant(&c,'.'); while(*c && (cap < DETEC_CAPT_MAX)) {
		d = c; ItemSuivant(&c,'.'); sscanf(d,"%hhd",&(detectr->captr[cap++].bb.adc));
	}
	for( ; cap<detectr->nbcapt; cap++) detectr->captr[cap].bb.adc = cap + 1;

	adrs = 0; bb = -1;
	if(type_lien == '#') /* ident impose (on espere qu'il est bien defini pour ce numeriseur) */ {
		detectr->pos = CABLAGE_INDEFINI;
		strcpy(detectr->adresse,"ext");
		// adrs = ident & 0xFF;
		sscanf(item,"%hx",&ident);
		for(bb=0; bb<NumeriseurNb; bb++) if(Numeriseur[bb].ident == ident) break;
		if(bb >= NumeriseurNb) {
			bb = -1;
			detectr->lu = DETEC_HS;
			if(detectr->local) {
				printf("  ! Le numeriseur identifie par %04X, directement branche sur le detecteur %s, est inconnu\n",
					   ident,detectr->nom);
				*nb_erreurs += 1;
			}
		}
	} else if((type_lien == '>') || (type_lien == '<')) /* cablage direct ainsi demande, on attend un numeriseur */ {
		TypeNumeriseurDef def; char nom_local[256],fin;
		char fichier_bb[MAXFILENAME]; FILE *g,*h;
		float r_modul,c_modul;

		if(*item == '@') item++;
		r_modul = 100.0; c_modul = 22.0;
		NumeriseursModlAssure();
		g = NumeriseurDefLit(item,&def,nom_local,fichier_bb,&fin);
		if(g) printf("  . Numeriseur pour %s directement lu dans %s\n",detectr->nom,fichier_bb);
		// fournir <bb> (peut-etre deja lu?) et terminer les affectations
		for(bb=0; bb<NumeriseurNb; bb++) if((Numeriseur[bb].def.type == def.type) && (Numeriseur[bb].def.serial == def.serial)) break;
		if(bb >= NumeriseurNb) {
			int k;
			if(g) h = g; else h = fichier;
			if(NumeriseurNb >= NUMER_MAX) {
				printf("  ! Deja %d numeriseurs lus! Detecteur et numeriseur abandonnes\n",NumeriseurNb);
				*nb_erreurs += 1; detectr->lu = DETEC_HS;
				return;
			}
			modele_bn = &(ModeleBN[def.type]);
			NumeriseurComplete(bb,modele_bn,0,&def,h,nom_local,fin);
			if((k = NumerModlRessIndex(modele_bn,"Rmodul")) >= 0) { Numeriseur[bb].ress[k].val.r = r_modul; NumeriseurRessValChangee(&(Numeriseur[bb]),k); }
			if((k = NumerModlRessIndex(modele_bn,"Cmodul")) >= 0) { Numeriseur[bb].ress[k].val.r = c_modul; NumeriseurRessValChangee(&(Numeriseur[bb]),k); }
			NumeriseurNb++;
			NumeriseurTermineAjout();
		}
		if(g) fclose(g);
		adrs = Numeriseur[bb].def.adrs;
	}
	for(cap=0; cap<detectr->nbcapt; cap++) {
		detectr->captr[cap].bb.num = bb;
		detectr->captr[cap].bb.adrs = adrs;
	}
	if(bb >= 0) {
		for(pos_hexa=0; pos_hexa<CABLAGE_POS_MAX; pos_hexa++) {
			for(m=0; m<CablageSortant[pos_hexa].nb_fischer; m++) {
				if(Numeriseur[bb].fischer == CablageSortant[pos_hexa].fischer[m]) {
					DetecteurPlace(detectr,pos_hexa);
					for(cap=0; cap<detectr->nbcapt; cap++) {
						detectr->captr[cap].bb.adc = CablageSortant[detectr->pos].captr[cap].vbb + 1;
					}
					break;
				}
			}
			if(m < CablageSortant[pos_hexa].nb_fischer) break;
		}
	}
}
/* ========================================================================== */
void DetecteurPlace(TypeDetecteur *detectr, TypeCablePosition pos_hexa) {
	TypeCablePosition pos_prec;

	if(detectr) {
		strcpy(detectr->adresse,CablageEncodePosition(pos_hexa));
		pos_prec = detectr->pos;
		if(pos_prec != pos_hexa) {
			detectr->pos = pos_hexa;
			if(pos_prec != CABLAGE_INDEFINI) CablageSortant[pos_prec].bolo = -1;
			if(pos_hexa != CABLAGE_INDEFINI) CablageSortant[pos_hexa].bolo = detectr->numero;
		}
	}
	// printf("(%s) Cablage[%02X].bolo = %d (%s)\n",__func__,pos_hexa,detectr->numero,detectr->nom);
	// printf("(%s) %s.pos = 0x%x\n",__func__,detectr->nom,detectr->pos);
}
/* ========================================================================== */
TypeCablePosition DetecteurRetire(TypeDetecteur *detectr) {
	TypeCablePosition pos_hexa;
	
	if(detectr) {
		pos_hexa = detectr->pos;
		if(pos_hexa != CABLAGE_INDEFINI) {
			detectr->pos = CABLAGE_INDEFINI;
			strcpy(detectr->adresse,CablageEncodePosition(detectr->pos));
			CablageSortant[pos_hexa].bolo = -1;
		}
	} else pos_hexa = CABLAGE_INDEFINI;
	// printf("(%s) Cablage[%02X].bolo = %d\n",__func__,pos_hexa,CablageSortant[pos_hexa].bolo);
	return(pos_hexa);
}
/* ========================================================================== */
char DetecteurBrancheNumeriseurs(TypeDetecteur *detectr) {
	int cap,bb; char branche; unsigned short adrs;
	TypeCableConnecteur connecteur;

	branche = 0;
	for(cap=0; cap<detectr->nbcapt; cap++) {
		detectr->captr[cap].bb.adc = CablageSortant[detectr->pos].captr[cap].vbb + 1;
		connecteur = CablageSortant[detectr->pos].captr[cap].connecteur;
		// printf("(%s) Cablage sortant %s: [%02X.%d] -> %d.%d\n",__func__,detectr->nom,detectr->pos,cap,connecteur,CablageSortant[detectr->pos].captr[cap].vbb+1);
		bb = -1; adrs = 0;
		if(connecteur) {
			for(bb=0; bb<NumeriseurNb; bb++) if(Numeriseur[bb].fischer == connecteur) break;
			if(bb < NumeriseurNb) { adrs = Numeriseur[bb].def.adrs; branche = 1; } else {
				bb = -1;
				detectr->lu = DETEC_HS;
				if(detectr->local) {
					printf("  * Pas de numeriseur installe en %d (connecteur pour la voie %s)\n",
						connecteur,VoieManip[detectr->captr[cap].voie].nom);
				}
			}
		}
		detectr->captr[cap].bb.num = bb;
		detectr->captr[cap].bb.adrs = adrs;
		if(CompteRendu.cablage.detectr) {
			if(bb >= 0) printf("  . %s capteur %d: voie %d du numeriseur %s (@%02X), branchee\n",
				detectr->nom,cap+1,detectr->captr[cap].bb.adc,Numeriseur[bb].nom,detectr->captr[cap].bb.adrs);
			else printf("  . %s capteur %d: pas de numeriseur branche\n",detectr->nom,cap+1);
		}
	}
	return(branche);
}
/* ========================================================================== */
char DetecteurBrancheCapteur(TypeDetecteur *detectr, short cap, int bb, short vbb, unsigned char adrs, char *explic) {
	short voie,vm,attr;
	TypeCableConnecteur connecteur;
	TypeBNModele *modele_bn;
	char type_adc;

	detectr->captr[cap].bb.num = bb;
	detectr->captr[cap].bb.serial = (bb >= 0)? Numeriseur[bb].def.serial: 0;
	detectr->captr[cap].bb.adrs = adrs;
	detectr->captr[cap].bb.adc = vbb + 1;

	/* printf("(%s) capteur %s.%d numerise par l'adc #%d (voie %d de la %s)\n",__func__,
	 detectr->nom,cap+1,detectr->captr[cap].bb.adc+1,vbb+1,Numeriseur[bb].nom); */
	voie = detectr->captr[cap].voie;
	VoieManip[voie].num_adc = vbb;
	VoieManip[voie].adrs = adrs;

	vm = detectr->captr[cap].modele;
	if(bb < 0) {
		if(detectr->lu) {
			if(detectr->pos == CABLAGE_INDEFINI) {
				sprintf(explic,"Le capteur %s du detecteur %s n'est pas accessible",
						ModeleVoie[vm].nom,detectr->nom);
			} else {
				connecteur = CablageSortant[detectr->pos].captr[cap].connecteur;
				sprintf(explic,"Le capteur %s du detecteur %s, relie au connecteur %s, n'est pas accessible",
						ModeleVoie[vm].nom,detectr->nom,CablageEncodeConnecteur(connecteur));  // ,pos_hexa avec 'soit x%02X '
			}
			return(0);
		} else return(1);
	}
	if((attr = Numeriseur[bb].voie[vbb]) < 0) Numeriseur[bb].voie[vbb] = voie;
	else {
		sprintf(explic,"L'ADC #%d de la %s ayant deja ete attribue a la voie '%s', la voie %s de %s ne sera pas accessible",
				vbb+1,Numeriseur[bb].nom,VoieManip[attr].nom,ModeleVoie[vm].nom,detectr->nom);
		return(0);
	}
	// printf("(%s) %s lit %s sur son ADC%d\n",__func__,Numeriseur[bb].nom,VoieManip[voie].nom,vbb+1);

	modele_bn = &(ModeleBN[(int)Numeriseur[bb].def.type]);
	type_adc = (vbb < modele_bn->nbadc)? modele_bn->adc[vbb]: 0;
	VoieManip[voie].type_adc = type_adc;
	VoieManip[voie].signe = ModeleADC[type_adc].signe;
	VoieManip[voie].ADCmask = (1 << ModeleADC[type_adc].bits) - 1;
	VoieManip[voie].extens = 0xFFFF - (unsigned short)VoieManip[voie].ADCmask;
	VoieManip[voie].zero = (VoieManip[voie].ADCmask + 1) / 2;
	VoieManip[voie].ADUenmV = ModeleADC[type_adc].polar * 1000.0 / (float)(VoieManip[voie].ADCmask + 1); /* valeur d'un ADU en mV */
	// printf("(%s) ADUenmV[%s] = (polar=%d mV) / (mask=%d) = %g\n",__func__,VoieManip[voie].nom,ModeleADC[type_adc].polar * 1000.0, VoieManip[voie].ADCmask + 1,VoieManip[voie].ADUenmV);
	VoieManip[voie].calib.gain_ampli = CablageSortant[detectr->pos].captr[cap].gain;
	VoieManip[voie].calib.couplage = CablageSortant[detectr->pos].captr[cap].capa;
	//!! convention polar: Tension totale donc ADU(mV) independant du signage: if(ModeleADC[type_adc].signe) VoieManip[voie].ADUenmV *= 2.0;
	// printf("* Voie %s: 1 ADU = %g mV (entree ADC), %g keV/nV avant gains\n",VoieManip[voie].nom,VoieManip[voie].ADUenmV,VoieManip[voie].nVenkeV);
	if(bb >= 0) VoieManip[voie].gain_fixe = VoieManip[voie].calib.gain_ampli * Numeriseur[bb].def.gain[vbb];
	// printf("  * Gain fixe de la voie %s: %g (cablage) x %g (%s, ADC %d), soit %g\n",VoieManip[voie].nom,
	//	   CablageSortant[detectr->pos].captr[cap].gain,Numeriseur[bb].def.gain[vbb],Numeriseur[bb].nom,vbb,VoieManip[voie].gain_fixe);
	if(VoieManip[voie].gain_fixe == 0.0) VoieManip[voie].gain_fixe = 1.0;

	return(1);
}
#ifdef UTILISE_OU_PAS
/* ========================================================================== */
char DetecteurBrancheVoies(TypeDetecteur *detectr, char type_lien, char *item, TypeCablePosition pos_defaut, FILE *fichier, int *nb_erreurs) {
	int l; int bb,vbb,cap; char explic[256];
	TypeCablePosition pos_hexa; TypeCableConnecteur connecteur; TypeBNModele *modele_bn;
	unsigned short ident,adrs;
	
	if(type_lien == '#') /* ident impose (on espere qu'il est bien defini pour ce numeriseur) */ {
		detectr->pos = CABLAGE_INDEFINI;
		strcpy(detectr->adresse,"ext");
		sscanf(item,"%x",&l); ident = l & 0xFFFF;
		for(bb=0; bb<NumeriseurNb; bb++) if(Numeriseur[bb].ident == ident) break;
		if(bb < NumeriseurNb) connecteur = Numeriseur[bb].fischer;
		else {
			bb = -1; connecteur = 0;
			detectr->lu = DETEC_HS;
			if(detectr->local) {
				printf("  ! Le numeriseur identifie par %04X, directement branche sur le detecteur %s, est inconnu\n",
					ident,detectr->nom);
				*nb_erreurs += 1;
			}
		}
		for(cap=0; cap<detectr->nbcapt; cap++) {
			if(!DetecteurBrancheCapteur(detectr,cap,bb,cap,ident & 0xFF,explic)) {
				if(bb != -1) { printf("  ! %s\n",explic); *nb_erreurs += 1; }
				detectr->lu = DETEC_HS;
			}
		}
	} else if((type_lien == '>') || (type_lien == '<')) /* cablage direct ainsi demande, on attend un numeriseur */ {
		TypeNumeriseurDef def; char nom_local[256],fin;
		char fichier_bb[MAXFILENAME]; FILE *g,*h;
		float r_modul,c_modul;
		char *c,*d; int adc[DETEC_CAPT_MAX]; int nbcapt;
		
		if(*item == '@') item++;
		c = item; ItemSuivant(&c,'.'); nbcapt = 0;
		while(*c && (nbcapt < DETEC_CAPT_MAX)) {
			d = c; ItemSuivant(&c,'.');
			sscanf(d,"%d",&(adc[nbcapt++]));
		}
		r_modul = 100.0; c_modul = 22.0;
		NumeriseursModlAssure();
		g = NumeriseurDefLit(item,&def,nom_local,fichier_bb,&fin);
		if(g) printf("  . Numeriseur pour %s directement lu dans %s\n",detectr->nom,fichier_bb);
		// fournir <bb> (peut-etre deja lu?) et terminer les affectations
		for(bb=0; bb<NumeriseurNb; bb++) if((Numeriseur[bb].def.type == def.type) && (Numeriseur[bb].def.serial == def.serial)) break;
		if(bb >= NumeriseurNb) {
			int k;
			if(g) h = g; else h = fichier;
			if(NumeriseurNb >= NUMER_MAX) {
				printf("  ! Deja %d numeriseurs lus! Detecteur et numeriseur abandonnes\n",NumeriseurNb);
				*nb_erreurs += 1; detectr->lu = DETEC_HS;
				return(0);
			}
			modele_bn = &(ModeleBN[def.type]);
			NumeriseurComplete(bb,modele_bn,0,&def,h,nom_local,fin);
			if((k = NumerModlRessIndex(modele_bn,"Rmodul")) >= 0) { Numeriseur[bb].ress[k].val.r = r_modul; NumeriseurRessValChangee(&(Numeriseur[bb]),k); }
			if((k = NumerModlRessIndex(modele_bn,"Cmodul")) >= 0) { Numeriseur[bb].ress[k].val.r = c_modul; NumeriseurRessValChangee(&(Numeriseur[bb]),k); }
			NumeriseurNb++;
			NumeriseurTermineAjout();
		}
		if(g) fclose(g);
		adrs = Numeriseur[bb].def.adrs;
		for(cap=0; cap<detectr->nbcapt; cap++) {
			if(cap < nbcapt) vbb = adc[cap] - 1; else vbb = cap;
			if(!DetecteurBrancheCapteur(detectr,cap,bb,vbb,adrs,explic)) {
				printf("  ! %s...\n",explic); *nb_erreurs += 1;
				detectr->lu = DETEC_HS;
			}
		}
		detectr->pos = CABLAGE_POS_DIR;
	} else {
		if(pos_defaut == CABLAGE_INDEFINI) pos_hexa = CablageDecodePosition(item);
		else pos_hexa = pos_defaut;
		detectr->pos = pos_hexa;
		strcpy(detectr->adresse,CablageEncodePosition(detectr->pos));
		/* cap doit boucler de 0 a nombre_de_voies_du_[sous-]detecteur */
		for(cap=0; cap<detectr->nbcapt; cap++) {
			connecteur = CablageSortant[pos_hexa].captr[cap].connecteur;
			vbb = CablageSortant[pos_hexa].captr[cap].vbb;
			// printf("(%s) Cablage sortant %8s: %02X.%d -> %d.%d\n",__func__,detectr->nom,pos_hexa,cap,connecteur,vbb);
			bb = -1; adrs = 0;
			if(connecteur) {
				for(bb=0; bb<NumeriseurNb; bb++) if(Numeriseur[bb].fischer == connecteur) break;
				if(bb < NumeriseurNb) adrs = Numeriseur[bb].def.adrs; else {
					bb = -1;
					detectr->lu = DETEC_HS;
					if(detectr->local) {
						printf("  ! Pas de numeriseur connecte en %d pour la voie %s\n",
							   connecteur,VoieManip[detectr->captr[cap].voie].nom);
						*nb_erreurs += 1;
					}
				}
				/* pas de pb si voie composee
				 } else {
				 detectr->lu = DETEC_HS;
				 if(detectr->local) {
				 printf("  ! La position %s.%d (occupee par la voie %s) n'est cablee a aucun connecteur\n",
				 Bolo[bolo].adresse,cap+1,VoieManip[detectr->captr[cap].voie].nom);
				 *nb_erreurs += 1;
				 } */
			}
			if(!DetecteurBrancheCapteur(detectr,cap,bb,vbb,adrs,explic)) {
				/* if((bb != -1) && connecteur) */ { printf("  ! %s...\n",explic); *nb_erreurs += 1; }
				/* pas de pb si voie composee: detectr->lu = DETEC_HS; */
			}
		}
		// if(*ref) printf(" (copie de %s)\n",ref); else printf("\n");
	}
	return(1);
}
#endif /* UTILISE_OU_PAS */
/* ========================================================================== */
void DetecteurDebrancheCapteur(TypeDetecteur *detectr, short cap) {
	short bb,vbb,voie; unsigned short adrs;

	bb = detectr->captr[cap].bb.num;
	vbb = detectr->captr[cap].bb.adc - 1;
	if((bb >= 0) && (vbb >= 0)) {
		// printf("  . %s capteur %d: numeriseur %s.ADC%d debranche\n",detectr->nom,cap+1,Numeriseur[bb].nom,vbb+1);
		Numeriseur[bb].voie[vbb] = -1;
	}
	bb = -1; vbb = -1; adrs = 0xFF;
	detectr->captr[cap].bb.num = bb;
	detectr->captr[cap].bb.adc = vbb + 1;
	detectr->captr[cap].bb.adrs = adrs;
	detectr->captr[cap].bb.serial = 0; // perime?
	voie = detectr->captr[cap].voie;
	VoieManip[voie].num_adc = vbb;
	VoieManip[voie].adrs = adrs;
}
/* ========================================================================== */
void DetecteurDebrancheNumeriseurs(TypeDetecteur *detectr) {
	int cap;
	
	for(cap=0; cap<detectr->nbcapt; cap++) DetecteurDebrancheCapteur(detectr,cap);
}
/* ========================================================================== */
void DetecteurDeconnecteVoie(TypeDetecteur *detectr, int voie) {
	short cap,v;

	if(voie < 0) return;
	cap = VoieManip[voie].cap;
	DetecteurDebrancheCapteur(detectr,cap);
	VoiesNb--;
	for(v=voie; v<VoiesNb; v++) {
		memcpy(VoieManip+v,VoieManip+v+1,sizeof(TypeVoieAlire));
	}
}
/* ========================================================================== */
static void DetecteurVoieCalibre(TypeDetecteur *detectr, short voie, short cap, short bb, short vbb, char type_adc) {
	VoieManip[voie].type_adc = type_adc;
	VoieManip[voie].signe = ModeleADC[type_adc].signe;
	VoieManip[voie].ADCmask = (1 << ModeleADC[type_adc].bits) - 1;
	VoieManip[voie].extens = 0xFFFF - (unsigned short)VoieManip[voie].ADCmask;
	VoieManip[voie].zero = (VoieManip[voie].ADCmask + 1) / 2;
	VoieManip[voie].ADUenmV = ModeleADC[type_adc].polar * 1000.0 / (float)(VoieManip[voie].ADCmask + 1); /* valeur d'un ADU en mV */
//	printf("(%s) ADUenmV[%s] %s = (polar=%g mV) / (mask %d bits=%d) = %g\n",__func__,VoieManip[voie].nom,(bb >= 0)?"final":"tempo",
//		   ModeleADC[type_adc].polar * 1000.0, ModeleADC[type_adc].bits,VoieManip[voie].ADCmask + 1,VoieManip[voie].ADUenmV);
	//!! convention polar: Tension totale donc ADU(mV) independant du signage: if(ModeleADC[type_adc].signe) VoieManip[voie].ADUenmV *= 2.0;
	// printf("* Voie %s: 1 ADU = %g mV (entree ADC), %g keV/nV avant gains\n",VoieManip[voie].nom,VoieManip[voie].ADUenmV,VoieManip[voie].nVenkeV);
	if(detectr->pos == CABLAGE_INDEFINI) {
		VoieManip[voie].calib.couplage = 1;
		VoieManip[voie].calib.gain_ampli = 1.4;
		VoieManip[voie].gain_fixe =  1.0;
	} else {
		VoieManip[voie].calib.couplage = CablageSortant[detectr->pos].captr[cap].capa;
		VoieManip[voie].calib.gain_ampli = CablageSortant[detectr->pos].captr[cap].gain;
		if((bb >= 0) && (vbb >= 0)) VoieManip[voie].gain_fixe = VoieManip[voie].calib.gain_ampli * Numeriseur[bb].def.gain[vbb];
		else VoieManip[voie].gain_fixe = VoieManip[voie].calib.gain_ampli;
		if(VoieManip[voie].gain_fixe == 0.0) VoieManip[voie].gain_fixe = 1.0;
		// printf("  * Gain fixe de la voie %s: %g (cablage) x %g (%s, ADC %d), soit %g\n",VoieManip[voie].nom,
		//	   CablageSortant[detectr->pos].captr[cap].gain,Numeriseur[bb].def.gain[vbb],Numeriseur[bb].nom,vbb,VoieManip[voie].gain_fixe);
	}
	VoieManip[voie].calib.injection = 100;
	if(VoieManip[voie].ADUenmV == 0.0) VoieManip[voie].calib.mesure = 0.0;
	else VoieManip[voie].calib.mesure = VoieManip[voie].calib.injection * VoieManip[voie].calib.couplage * VoieManip[voie].calib.gain_ampli / VoieManip[voie].ADUenmV;
	// printf("    > %s %s: gain ampli=%g V/pC par defaut\n",__func__,VoieManip[voie].nom,VoieManip[voie].calib.gain_ampli);
}
/* ========================================================================== */
char DetecteurConnecteVoies(TypeDetecteur *detectr, int *nb_erreurs) {
	short cap,bb,vbb,voie,vm,attr; unsigned short adrs; // char nom_capteur[MODL_NOM];
	TypeCableConnecteur connecteur;
	TypeBNModele *modele_bn;
	char type_adc;

	for(cap=0; cap<detectr->nbcapt; cap++) {
		bb = detectr->captr[cap].bb.num;
		vbb = detectr->captr[cap].bb.adc - 1;
		// if(bb >= 0) printf("  . %s capteur %d: numeriseur %s.ADC%d rebranche\n",detectr->nom,cap+1,Numeriseur[bb].nom,vbb+1);
		adrs = detectr->captr[cap].bb.adrs;
		detectr->captr[cap].bb.serial = (bb >= 0)? Numeriseur[bb].def.serial: 0; // perime?
		/* printf("(%s) capteur %s.%d numerise par l'adc #%d (voie %d de la %s)\n",__func__,
			   detectr->nom,cap+1,detectr->captr[cap].bb.adc,vbb+1,Numeriseur[bb].nom); */
		voie = detectr->captr[cap].voie;
		VoieManip[voie].num_adc = vbb;
		VoieManip[voie].adrs = adrs;
		
		vm = detectr->captr[cap].modele;
//		if(detectr->lu) printf("(%s) Connecte une voie de type %s sur le capteur #%d de %s\n",__func__,ModeleVoie[vm].nom,cap,detectr->nom);
		if(bb < 0) {
			if(detectr->lu) {
				if(detectr->pos == CABLAGE_INDEFINI) {
					printf("  ! Pas de cablage defini pour le detecteur %s (capteur %s)\n",detectr->nom,ModeleVoie[vm].nom);
				} else {
					connecteur = CablageSortant[detectr->pos].captr[cap].connecteur;
					printf("  ! Pas de numeriseur connecte a la voie %s (capteur #%d, connecteur %s): lecture impossible\n",
						VoieManip[voie].nom,cap,CablageEncodeConnecteur(connecteur));  // ,pos_hexa avec 'soit x%02X '
				}
				*nb_erreurs += 1; detectr->lu = DETEC_HS;
			}
			continue;
		}
		if((attr = Numeriseur[bb].voie[vbb]) < 0) Numeriseur[bb].voie[vbb] = voie;
		else {
			printf("  ! L'ADC #%d de la %s est deja connecte a la voie %s: la voie %s ne sera pas accessible\n",
					vbb+1,Numeriseur[bb].nom,VoieManip[attr].nom,VoieManip[voie].nom);
			*nb_erreurs += 1; detectr->lu = DETEC_HS;
		}
		// printf("* %s lit %s sur son ADC%d\n",Numeriseur[bb].nom,VoieManip[Numeriseur[bb].voie[vbb]].nom,vbb+1);
		
		modele_bn = &(ModeleBN[(int)Numeriseur[bb].def.type]);
		type_adc = (vbb < modele_bn->nbadc)? modele_bn->adc[vbb]: 0;
		DetecteurVoieCalibre(detectr,voie,cap,bb,vbb,type_adc);
	}

	return(1);
}
/* ========================================================================== */
void DetecteurConnecteReglages(TypeDetecteur *detectr, int *nb_erreurs) {
	TypeReglage *regl; TypeNumeriseur *numeriseur; TypeBNModele *modele_bn;
	TypeModeleCable *cablage; TypeCableConnecteur connecteur; char *nom,*r;
	int cap,bb,voie,dm,type,rang_bb; int i,k,n; char log;
	char nom_vu[NUMER_RESS_NOM],cap_vu[DETEC_CAPT_MAX];

	log = 0;
	for(cap=0; cap<detectr->nbcapt; cap++) cap_vu[cap] = 0;
	for(cap=0; cap<detectr->nbcapt; cap++) if(detectr->captr[cap].bb.num >= 0) break;
	if(log > 1) printf("  %c%s.%d/%d: numeriseur #%d\n",(detectr->local)?'+':'-',detectr->nom,cap,detectr->nbcapt,(cap < detectr->nbcapt)?detectr->captr[cap].bb.num:-1);
	if(cap < detectr->nbcapt) detectr->branche = 1;
	else {
		if(detectr->lu) {
			printf("  ! Suite a l'absence de tout numeriseur, aucun reglage de %s ne sera utilisable ou utile\n",detectr->nom);
			*nb_erreurs += 1;
		}
		detectr->branche = 0;
		return;
	}
	if((type = CablageSortant[detectr->pos].type) < 0) {
		printf("  ! Le cablage utilise pour %s est indetermine\n",detectr->nom);
		*nb_erreurs += 1;
		detectr->branche = 0;
		return;
	}
	cablage = &(ModeleCable[type]);
	if(log) printf("  > cablage modele '%s'\n",cablage->nom);
	dm = detectr->type;
	for(i=0, regl=detectr->reglage; i<detectr->nbreglages; i++,regl++) {
		nom = regl->nom;
		cap = regl->capt;
		if(log > 1) printf("  . %s: cablage du reglage '%s' pour la voie %s\n",detectr->nom,nom,detectr->captr[cap].nom);
		if((n = CablageRessource(cablage,&nom,0))) regl->valeurs = 0; // cablage->connection[n-1].valeurs; quand elles seront lues (cf RAZxxxx)
		else {
			if(detectr->local)
				printf("    ! Le reglage %s du detecteur %s n'est pas cable\n",ModeleDet[dm].regl[i].nom,detectr->nom);
			regl->valeurs = 0;
			continue;
		}
		if(!strcmp(nom,"neant")) continue;
		if(log > 1) printf("    . Recherche d'une ressource '%s'\n",nom);
		// pas encore defini: bb = regl->bb; bb a trouver dans le nom (exemple: 3.<ressource>, a entrer dans le modele de cablage)
		// et calablage reel du style:
		// "/FID-BB1BB23: a1 > 95.5, 94.1, 95.1, 95.2, 95.3, 95.4" devient: { a1 := modele = FID-BB1BB23, connecteurs = (94, 95, <media>) }
		strncpy(nom_vu,nom,NUMER_RESS_NOM);
		r = nom_vu; ItemJusquA('.',&r);
		if(*r != '\0') {
			nom = r;
			sscanf(nom_vu,"%d",&rang_bb); --rang_bb;
			if((connecteur = CablageSortant[detectr->pos].fischer[rang_bb])) {
				for(bb=0; bb<NumeriseurNb; bb++) if(Numeriseur[bb].fischer == connecteur) break;
				if(bb >= NumeriseurNb) bb = -1;
			} else bb = -1;
		} else bb = detectr->captr[cap].bb.num;
		if(bb < 0) {
			if(!(cap_vu[cap]) && detectr->lu && /* detectr->branche && detectr->local && */ !VoieManip[detectr->captr[cap].voie].pseudo) {
				printf("    ! Pas de numeriseur connecte a la voie %s (capteur #%d): reglage impossible\n",
					VoieManip[detectr->captr[cap].voie].nom,cap);  // ,pos_hexa avec 'soit x%02X '
				*nb_erreurs += 1;
			}
			cap_vu[cap] = 1;
			continue;
		}
		numeriseur = &(Numeriseur[bb]);
//		regl->adrs = detectr->captr[cap].bb.adrs;
		modele_bn = &(ModeleBN[numeriseur->def.type]);
		for(k=0; k<modele_bn->nbress; k++) {
			if(log > 1) printf("      . %s: ressource connue #%d: '%s'\n",numeriseur->nom,k,modele_bn->ress[k].nom);
			if(!strcmp(nom,modele_bn->ress[k].nom)) break;
		}
		if(k < modele_bn->nbress) {
            DetecAvecReglages = 1;
			regl->ress = k;
			regl->ss_adrs = modele_bn->ress[k].ssadrs;
			regl->bb = bb;
			numeriseur->ress[k].regl = regl;
			if(log) printf("    | Le reglage %s sera fait par le %s d'un numeriseur de type %s\n",
				ModeleDet[dm].regl[i].nom,nom,modele_bn->nom);
			if(modele_bn->ress[k].categ == RESS_MODUL) {
				voie = detectr->captr[regl->capt].voie;
				if(!VoieManip[voie].modulee) {
					float r_modul,c_modul,t_modul,capa; int k; char circuit_rc;
					circuit_rc = 0;
				#ifdef PB_BBV2_3
					if(modele_bn->ident == 2) /* (!strcmp(modele_bn->nom,"BB2")) */ {
						t_modul = 1000000.0 / Echantillonnage; /* ns car amplitude triangle en mV */
					} else {
						if((k = NumerModlRessIndex(modele_bn,"Rmodul")) >= 0) r_modul = numeriseur->ress[k].val.r; else r_modul = 100.0; /* kO */
						if((k = NumerModlRessIndex(modele_bn,"Cmodul")) >= 0) c_modul = numeriseur->ress[k].val.r; else c_modul = 22.0;  /* nF */
						t_modul = r_modul * c_modul; /* kO x nF soit us */
					}
				#else /* PB_BBV2_3 */
					/* autre methode, une info ModlBN: modulation="neant/rc/pente" avec R et C donnes si rc */
					if((k = NumerModlRessIndex(modele_bn,"Rmodul")) >= 0) {
						circuit_rc = 1;
						r_modul = numeriseur->ress[k].val.r;  /* kO */
						// printf("    * Rmodul(%s)=%g kO\n",numeriseur->nom,r_modul);
						if((k = NumerModlRessIndex(modele_bn,"Cmodul")) >= 0) c_modul = numeriseur->ress[k].val.r; else c_modul = 22.0;  /* nF */
						// printf("    * Cmodul(%s)=%g nF\n",numeriseur->nom,c_modul);
						t_modul = r_modul * c_modul; /* kO x nF soit us */
						// printf("    * soit Tmodul(%s)=%g us\n",VoieManip[voie].nom,t_modul);
					}
					if(!circuit_rc) t_modul = 1000000.0 / Echantillonnage; /* ns, car amplitude triangle en mV */
				#endif /* PB_BBV2_3 */
					if((capa = CablageSortant[detectr->pos].captr[cap].capa) > 0.0)
						VoieManip[voie].Rmodul = t_modul / capa; /* capa en pF d'ou Rmodul en MO(v1) ou kO(v2) */
					else VoieManip[voie].Rmodul = 0.0;
					// printf("    * Rmodul(%s)=%g\n",VoieManip[voie].nom,VoieManip[voie].Rmodul);
					VoieManip[voie].modulee = 1;
					DetecAvecModulation = 1;
				}
			}

			/* Valeurs par defaut du numeriseur (reglages pas encore lus) */
			strncpy(numeriseur->ress[k].cval,regl->std,NUMER_RESS_VALTXT);
			NumeriseurRessUserChange(numeriseur,k);
			strcpy(regl->std,numeriseur->ress[k].cval);
			strcpy(regl->user,numeriseur->ress[k].cval);
			if(ModeleBN[numeriseur->def.type].ress[k].type == RESS_FLOAT) 
				regl->val.r = numeriseur->ress[k].val.r;
			else regl->val.i = numeriseur->ress[k].val.i;
			regl->modifie = 0;
			switch(regl->cmde) {
				case REGL_D2: detectr->d2 = regl->val.i; break;
				case REGL_D3: detectr->d3 = regl->val.i; break;
				case REGL_GAIN: detectr->captr[cap].numgain = regl->val.i; 
					sscanf(numeriseur->ress[k].cval,"%g",&(detectr->captr[cap].gain_reel));
					break;
				case REGL_POLAR: DetecAvecPolar = 1; break;
				case REGL_RESET: detectr->reset = regl->val.i; break;
				case REGL_FET_MASSE: detectr->razfet_en_cours = (regl->val.i > 0);
				case REGL_FET_CC: case REGL_RAZFET: DetecAvecRaz = 1; break;
			}
			
			// if(log) printf("    | La voie %s %s modulee\n",VoieManip[detectr->captr[regl->capt].voie].nom,
			//			   VoieManip[detectr->captr[regl->capt].voie].modulee?"est":"n'est pas (encore)");
			/* Pas de reglage negatif sans un reglage positif */
			nom = ModeleDet[dm].regl[i].nom;
			if(CablageRessource(cablage,&nom,'-')) {
				for(k=0; k<modele_bn->nbress; k++) if(!strcmp(nom,modele_bn->ress[k].nom)) break;
				if(k < modele_bn->nbress) {
					regl->ressneg = k;
					regl->negatif = modele_bn->ress[k].ssadrs;
					if(log) printf("    | Le reglage -%s sera fait par le %s d'un numeriseur de type %s\n",
								   ModeleDet[dm].regl[i].nom,nom,modele_bn->nom);
				}
			}
			if(CablageRessource(cablage,&nom,'+')) {
				for(k=0; k<modele_bn->nbress; k++) if(!strcmp(nom,modele_bn->ress[k].nom)) break;
				if(k < modele_bn->nbress) {
					regl->resspos = k;
					regl->identik = modele_bn->ress[k].ssadrs;
					if(log) printf("    | Le reglage %s sera egalement fait par le %s d'un numeriseur de type %s\n",
								   ModeleDet[dm].regl[i].nom,nom,modele_bn->nom);
				}
			}
		}
		if(regl->ress < 0) {
			if(detectr->local && log) {
				printf("  ! Reglage %s du detecteur %s: %s pas implemente dans le numeriseur de type %s, %s\n",
					   ModeleDet[dm].regl[i].nom,detectr->nom,nom,modele_bn->nom,numeriseur->nom);
				*nb_erreurs += 1;
			}
			continue;
		}
	}
}
/* ========================================================================== */
int DetecteurCree(TypeDetecteur *detectr, char *nom, int dm, char *ref, char *hote, char *nom_partiel) {
	int nb_erreurs;
	int bolo,k;
	
	bolo = BoloNb;
	nb_erreurs = DetecteurDeclare(detectr,bolo,nom,dm,ref);
	DetecteurInfo(detectr,hote,nom_partiel,1);
	DetecteurAjouteVoies(detectr,&nb_erreurs);
	for(k=0; k<ModeleDet[dm].nbsoft; k++) {
		DetecteurAjoutePseudo(detectr,ModeleDet[dm].soft[k].pseudo,ModeleDet[dm].soft[k].nom,ModeleDet[dm].soft[k].ref,&nb_erreurs);
	}
	DetecteurAjouteReglages(detectr,&nb_erreurs,0);
	return(nb_erreurs);
}
/* ========================================================================== */
static void DetecteurConfigRAZ(TypeConfigDefinition *config) {
	int classe;

	config->evt.std = 1;
	for(classe=0; classe<TRMT_NBCLASSES; classe++) config->trmt[classe].std = 1;
	config->trgr.std = 1;
	config->rgul.std = 1;

	config->nom[0] = '\0';
	config->defini.evt = 0;
	for(classe=0; classe<TRMT_NBCLASSES; classe++) config->defini.trmt[classe] = 0;
	config->defini.trgr = 0;
	config->defini.rgul = 0;
	config->global.evt = 0;
	for(classe=0; classe<TRMT_NBCLASSES; classe++) config->global.trmt[classe] = 0;
	config->global.trgr = 0;
	config->global.rgul = 0;
}
/* ========================================================================== */
static short DetecteurVoieCree(TypeDetecteur *detectr, short cap, short vm, char *nom) {
	short voie; char type_adc; int num_config,globale,classe;

	voie = VoiesNb;

	if((cap < 0) || (cap >= DETEC_CAPT_MAX)) {
		printf("  ! Voie '%s' rattachee a un capteur de rang %d/%d, illegal\n",nom,cap+1,DETEC_CAPT_MAX);
		return(-1);
	} else if(cap > detectr->nbcapt) {
		printf("  ! Voie '%s' rattachee a un capteur de rang %d/%d, indefini pour ce detecteur\n",nom,cap+1,detectr->nbcapt);
		return(-1);
	} else if(VoiesNb >= VoiesGerees) {
		detectr->captr[cap].voie = -1; /* pour que toutes les .captr[cap].voie soient definies */
		if(detectr->lu != DETEC_HS) {
			printf("  ! Deja %d/%d voies crees: %s (et suivants) ne sera pas lu\n",VoiesNb,VoiesGerees,detectr->nom);
			detectr->lu = DETEC_HS;
		}
		return(-1);
	}
	if(cap == detectr->nbcapt) detectr->nbcapt = cap + 1;
	detectr->captr[cap].modele = vm;
	detectr->captr[cap].voie = voie;
	strcpy(detectr->captr[cap].nom,nom);
	detectr->captr[cap].bb.num = -1;
	detectr->captr[cap].bb.adc = 0;

	snprintf(VoieManip[voie].nom,VOIE_NOM,"%s %s",nom,detectr->nom);
	VoieManip[voie].nom[VOIE_NOM-1] = '\0';
	snprintf(VoieManip[voie].prenom,MODL_NOM,"%s",nom);
	VoieManip[voie].prenom[MODL_NOM-1] = '\0';
	VoieManip[voie].numero = voie;
	VoieManip[voie].type = vm;
	VoieManip[voie].det = detectr->numero;
	VoieManip[voie].cap = cap;
	VoieManip[voie].physique = ModeleVoie[vm].physique;
	VoieManip[voie].simu = -1;
	VoieManip[voie].pseudo = 0;
	VoieManip[voie].sauvee = ARCH_DEFAUT;
	VoieManip[voie].sensibilite = 1.0;
	VoieManip[voie].modulee = 0;
	VoieManip[voie].Rmodul = 1.0;
	VoieManip[voie].num_adc = 0;
	VoieManip[voie].adrs = 0;
	VoieManip[voie].differe = 0;
	VoieManip[voie].affiche = 0;
	VoieHisto[voie] = 0;
	memcpy(&(VoieManip[voie].def.evt),&(ModeleVoie[vm]),sizeof(TypeVoieModele));
	DetecteurConfigRAZ(&(VoieManip[voie].def));
	for(num_config=0; num_config<CONFIG_MAX; num_config++) DetecteurConfigRAZ(&(VoieManip[voie].config.def[num_config]));
	VoieManip[voie].config.nb = 0;
	for(globale=0; globale<CONFIG_TOT; globale++) {
		VoieManip[voie].config.num[globale].evt = 0;
		for(classe=0; classe<TRMT_NBCLASSES; classe++) VoieManip[voie].config.num[globale].trmt[classe] = 0;
		VoieManip[voie].config.num[globale].trgr = 0;
		VoieManip[voie].config.num[globale].rgul = 0;
	}

	// modele_bn = &(ModeleBN[(int)Numeriseur[bb].def.type]); type_adc = (vbb < modele_bn->nbadc)? modele_bn->adc[vbb]: 0;
	type_adc = 0; /* par defaut */
	DetecteurVoieCalibre(detectr,voie,cap,-1,-1,type_adc);
	
	VoiesNb++;
	VoieManip[VoiesNb].nom[0] = '\0';
	return(voie);
}
/* ========================================================================== */
static void DetecteurSimplifieVoie(short voie) {
	TypeComposantePseudo *elt_voie,*svt;
	
	elt_voie = VoieManip[voie].pseudo;
	while(elt_voie) {
		svt = elt_voie->svt;
		free(elt_voie);
		elt_voie = svt;
	}
	VoieManip[voie].pseudo = 0;
}
/* ========================================================================== 
static char DetecteurVoieCompose(short voie, TypeComposantePseudo *pseudo) {
	TypeDetecteur *detectr; int cap; short ref; float mVenADU,gain;
	TypeComposantePseudo *elt_cap,*elt_voie,*svt;

	if(!pseudo) return(1);
	mVenADU = 0.0; gain = 0.0;
	detectr = &(Bolo[VoieManip[voie].det]); cap = VoieManip[voie].cap;
	elt_cap = pseudo; elt_voie = 0;
	while(elt_cap) {
		svt = (TypeComposantePseudo *)malloc(sizeof(TypeComposantePseudo));
		if(!svt) {
			printf("! Manque de memoire pour un element de la pseudo-voie %s %s\n",detectr->captr[cap].nom,detectr->nom);
			return(0);
		}
		if(elt_voie) elt_voie->svt = svt;
		elt_voie = svt;
		elt_voie->coef = elt_cap->coef;
		elt_voie->srce = detectr->captr[elt_cap->srce].voie;
		elt_voie->svt = 0;
		if(!VoieManip[voie].pseudo) { VoieManip[voie].pseudo = elt_voie; ref = elt_voie->srce; }
		if(elt_voie->srce >= 0) {
			if(VoieManip[elt_voie->srce].ADUenmV) mVenADU += (elt_voie->coef / VoieManip[elt_voie->srce].ADUenmV);
			gain += (elt_voie->coef * VoieManip[elt_voie->srce].gain_fixe);
		}
		elt_cap = elt_cap->svt;
	}
	VoieManip[voie].def.trmt[TRMT_AU_VOL].std = 0;
	VoieManip[voie].def.trmt[TRMT_AU_VOL].type = NEANT;
	VoieManip[voie].type_adc = (ref >= 0)? VoieManip[ref].type_adc :0;
	VoieManip[voie].signe = (ref >= 0)? VoieManip[ref].signe :1;
	VoieManip[voie].ADCmask = (ref >= 0)? VoieManip[ref].ADCmask :0xFFFF;
	VoieManip[voie].extens = (ref >= 0)? VoieManip[ref].extens :0;
	VoieManip[voie].zero = (ref >= 0)? VoieManip[ref].zero :0x8000;
	if((mVenADU > 0.0) || (mVenADU < 0.0)) VoieManip[voie].ADUenmV = 1.0 / mVenADU;
	else VoieManip[voie].ADUenmV = (ref >= 0)? VoieManip[ref].ADUenmV :1;
	VoieManip[voie].gain_fixe = (gain > 0.0)? gain: 1.0;

	return(1);
}
   ========================================================================== */
static void DetecteurRazTampon(int voie) {
	int ph,classe,etg;

	if(voie < 0) return;
	bzero(&(VoieTampon[voie]),sizeof(TypeVoieTampons));
	VoieTampon[voie].brutes = &(VoieTampon[voie].trmt[TRMT_AU_VOL].donnees);
	VoieTampon[voie].brutes->type = TAMPON_INT16;
	// printf("(%s) VoieTampon[%s].brutes @%08X\n",__func__,VoieManip[voie].nom,(hexa)VoieTampon[voie].brutes);
	VoieTampon[voie].traitees = &(VoieTampon[voie].trmt[TRMT_PRETRG].donnees);
	VoieTampon[voie].traitees->type = TAMPON_FLOAT;
	VoieTampon[voie].traitees->dim = 0;
	VoieTampon[voie].trig.special = 0; VoieTampon[voie].trig.trgr = 0;
	
	/* strcpy(VoieTampon[voie].moyen.evt.nom,VoieManip[voie].nom);
	VoieTampon[voie].moyen.evt.horloge = 1.0;
	VoieTampon[voie].moyen.evt.pre_trigger = 0; */
	VoieTampon[voie].norme_evt = 1.0;
	VoieEvent[voie].horloge = 1.0 / Echantillonnage;
	VoieEvent[voie].lngr = VoieManip[voie].def.evt.duree / VoieEvent[voie].horloge;
	VoieTampon[voie].decal = VoieManip[voie].def.evt.delai * Echantillonnage;
	for(ph=0; ph<DETEC_PHASES_MAX; ph++) {
		VoieTampon[voie].phase[ph].t0 = (int)(VoieManip[voie].def.evt.phase[ph].t0 / VoieEvent[voie].horloge); /* debut rapide points stockes (~ p.r. maxi) */
		VoieTampon[voie].phase[ph].dt = (int)(VoieManip[voie].def.evt.phase[ph].dt / VoieEvent[voie].horloge); /* temps rapide points stockes (~ p.r. debut) */
	}
	for(classe=0; classe<TRMT_NBCLASSES; classe++) {
		VoieTampon[voie].trmt[classe].compactage = 1;
		VoieTampon[voie].trmt[classe].nbdata = 0;
		VoieTampon[voie].trmt[classe].filtre = 0;
		for(etg=0; etg<MAXFILTREETG; etg++)
			VoieTampon[voie].trmt[classe].suivi.filtree[etg] = 0;
		VoieTampon[voie].trmt[classe].suivi.nbetg = 0;
		VoieTampon[voie].trmt[classe].suivi.max = 0;
		VoieTampon[voie].trmt[classe].suivi.brute = 0;
	#if defined(A_LA_GG) || defined(A_LA_MARINIERE)
		VoieTampon[voie].trmt[classe].X.max = 0;
		VoieTampon[voie].trmt[classe].Y.max = 0;
		VoieTampon[voie].trmt[classe].Z.max = 0;
	#endif
	}
	VoieTampon[voie].simu.facteurC = 1.0E9 / VoieManip[voie].ADUennV;
	if(VoieTampon[voie].simu.facteurC < 0.0) VoieTampon[voie].simu.facteurC = -VoieTampon[voie].simu.facteurC;
	if(VoieManip[voie].Rmodul > 0.0) VoieTampon[voie].simu.facteurC *= (VoieManip[voie].Rbolo / VoieManip[voie].Rmodul);
	VoieHisto[voie] = 0;
}
/* ========================================================================== */
static void DetecteurVoieVerifieFiltre(int voie, int classe) {
	int k;
	
	if((VoieManip[voie].def.trmt[classe].type == TRMT_FILTRAGE) && VoieManip[voie].def.trmt[classe].texte[0]) {
		for(k=0; k<FiltreNb; k++) if(!strcmp(VoieManip[voie].def.trmt[classe].texte,FiltreGeneral[k].nom)) break;
		if(k < FiltreNb) VoieManip[voie].def.trmt[classe].parm = k;
		else {
			k = VoieManip[voie].def.trmt[classe].parm;
			if((k < 0) || (k >= FiltreNb)) { k = VoieManip[voie].def.trmt[classe].parm = 0; }
			printf("  ! Filtre de la voie %s inconnu (%s), remplace par %s\n",VoieManip[voie].nom,
				VoieManip[voie].def.trmt[classe].texte,FiltreGeneral[k].nom);
			strcpy(VoieManip[voie].def.trmt[classe].texte,FiltreGeneral[k].nom);
		}
	}
}
/* ========================================================================== */
INLINE void DetecteurAdcCalib(int voie, char *prefixe) {
	VoieManip[voie].ADUenV = (VoieManip[voie].ADUenmV * 1.0e-3) / VoieManip[voie].gain_variable;
//	printf("(%s) ADUenV[%s]  = (ADUenmV=%g V) / (Gv=%g) = %g\n",__func__,VoieManip[voie].nom,
//		(VoieManip[voie].ADUenmV * 1.0e-3),VoieManip[voie].gain_variable,VoieManip[voie].ADUenV);
	VoieManip[voie].ADUennV = VoieManip[voie].ADUenmV * 1000000.0 / (VoieManip[voie].gain_variable * VoieManip[voie].gain_fixe);
//	printf("(%s) ADUennV[%s] = (ADUenmV=%g V) / ((Gv=%g) * (Gf=%g)) = %g\n",__func__,VoieManip[voie].nom,
//		   (VoieManip[voie].ADUenmV * 1.0e6),VoieManip[voie].gain_variable,VoieManip[voie].gain_fixe,VoieManip[voie].ADUennV);
	VoieManip[voie].nV_reels = VoieManip[voie].ADUennV;
	VoieManip[voie].ADUenkeV = VoieManip[voie].ADUennV * VoieManip[voie].nVenkeV;
	if(VoieManip[voie].calib.gain_ampli != 0.0) 
		// VoieManip[voie].ADUenE = VoieManip[voie].calib.couplage * (VoieManip[voie].calib.injection * 1.0e-3) / VoieManip[voie].calib.mesure / E_PICOCOULOMB;
		VoieManip[voie].ADUenE = (VoieManip[voie].ADUenmV * 1.0e-3) / VoieManip[voie].calib.gain_ampli / E_PICOCOULOMB;
	else VoieManip[voie].ADUenE = 0.0;
	//+ if(prefixe) printf("%s    > %s: gain ampli=%g V/pC, soit %g e/ADU (%s)\n",prefixe,VoieManip[voie].nom,VoieManip[voie].calib.gain_ampli,VoieManip[voie].ADUenE,__func__);
	// printf("  * Calibration finale de la voie %s: %g (mV/ADU) / (%g (variable) x %g (fixe)), soit %g nV/ADU\n",VoieManip[voie].nom,
	//	   VoieManip[voie].ADUenmV,VoieManip[voie].gain_variable,VoieManip[voie].gain_fixe,VoieManip[voie].ADUennV);
}
/* ========================================================================== */
static char DetecteurVoieFinalise(TypeDetecteur *detectr, int cap) {
	int voie,nature; char *nom;

	voie = detectr->captr[cap].voie; if(voie < 0) return(0);

	VoieManip[voie].nVenkeV = 1.0 / VoieManip[voie].sensibilite;
	VoieManip[voie].gain_variable = detectr->captr[cap].gain_reel;
	if(VoieManip[voie].gain_variable < 0.0001) VoieManip[voie].gain_variable = 1.0;
	VoieManip[voie].compens = (VoieManip[voie].modulee)? DetecteurConsigneAdrs(detectr,cap,REGL_COMP): 0;
	DetecteurAdcCalib(voie,"");
	
	if(BoloNb == 1) {
		if(SettingsVoiesNom == VOIE_NOM_COURT) {
			if(detectr->nbcapt > 1) strncpy(VoieManip[voie].nom,detectr->captr[cap].nom,VOIE_NOM);
			else strncpy(VoieManip[voie].nom,detectr->nom,VOIE_NOM);
		}
		VoieManip[voie].def.evt.std = VoieManip[voie].def.trmt[TRMT_AU_VOL].std = VoieManip[voie].def.trmt[TRMT_PRETRG].std = VoieManip[voie].def.trgr.std = VoieManip[voie].def.rgul.std = 0;
	}

	for(nature=0; nature<DETEC_PARM_RGUL+1; nature++) {
		nom = DetecteurCanalStandardise(nature,detectr->numero,cap,-1);
		if(DetecDebugStd) {
			if(nom)
				printf("  > %s %s du detecteur %s mis au standard\n",DetecteurParmsTitre[nature],nom,detectr->nom);
			else {
				nom = (VoieManip[voie].pseudo)? detectr->captr[cap].nom: VoieManip[voie].def.evt.nom;
				printf("  > %s %s du detecteur %s particuliere\n",DetecteurParmsTitre[nature],nom,detectr->nom);
			}
		}
	}
	if(!VoieManip[voie].modulee) VoieManip[voie].exec = 0;

	DetecteurVoieVerifieFiltre(voie,TRMT_AU_VOL);
	DetecteurVoieVerifieFiltre(voie,TRMT_PRETRG);
	if(VoieManip[voie].def.trgr.maxbruit < 0.001) VoieManip[voie].def.trgr.maxbruit = 99999.9;
	DetecteurRazTampon(voie);

	return(1);
}
/* ========================================================================== */
static void DetecteurScriptEnregistre(TypeDetecteur *detectr, char *usage, TypeDetecteurPrgm *prgm, char log) {
	char nom_complet[MAXFILENAME],abrege[MAXFILENAME]; FILE *f; int b,l;

	ScriptModeExec = 0;
	prgm->bloc = -1; // prgm->date = 0x7FFFFFFF;
	if(prgm->script[0]) {
		SambaAjouteRacine(nom_complet,DetecteurRacine,prgm->script);
		l = strlen(DetecteurRacine);
		if(!strncmp(nom_complet,DetecteurRacine,l)) sprintf(abrege,"<det>/%s",nom_complet+l);
		else strcpy(abrege,nom_complet);
		if(CompteRendu.detectr.maint) printf("  . %s: memorisation du script de %s\n            (d'apres %s)",detectr->nom,usage,abrege);
		for(b=0; b<DetecScriptsLibrairie.nb_blocs; b++) if(!strcmp(nom_complet,DetecScriptNoms[b].nomcomplet)) break;
		if(b < DetecScriptsLibrairie.nb_blocs) {
			if(log) printf(" deja vu (#%d) = {",b);
			if(CompteRendu.detectr.maint) printf("\n");
		} else if((f = fopen(nom_complet,"r"))) {
		#define MAXLIGNE 256
			char ligne[MAXLIGNE]; char bloc_ouvert,ok;
			if((b = ScriptNouveauBloc(&DetecScriptsLibrairie,HW_DETEC,(void *)detectr)) < 0) {
				printf(" impossible !!! (deja %d bloc%s cree%s)\n",Accord2s(DetecScriptsLibrairie.nb_blocs));
				return;
			}
			if(log) printf(" nouveau (#%d) = {",b);
			if(CompteRendu.detectr.maint) printf("\n");
			bloc_ouvert = 1; ok = 1;
			while(LigneSuivante(ligne,MAXLIGNE,f) && ok) ok = (ok && ScriptDecode(&DetecScriptsLibrairie,ligne,&bloc_ouvert,&b,""));
			fclose(f);
			strcpy(DetecScriptNoms[b].nomcomplet,nom_complet);
		} else if(CompteRendu.detectr.maint) printf(" illisible\n");
		if(b >= 0) {
			prgm->bloc = b;
			if(log) {
				detectr->exec.inst = DetecScriptsLibrairie.bloc[b].premiere;
				detectr->exec.boucle = 0;
				detectr->exec.date = 0;
				ScriptExecBatch(DetecScriptsLibrairie.action,HW_DETEC,(void *)detectr,&(detectr->exec),DetecScriptsLibrairie.bloc[b].derniere,0,"");
				printf("    }\n");
			}
		}
	}
	ScriptModeExec = 1;
}
/* ========================================================================== */
void DetecteurTermine(TypeDetecteur *detectr, char log) {
	int cap;
	/* int bb,num,k;
	TypeReglage *regl; TypeNumeriseur *numeriseur;
	TypeBNModele *modele_bn; TypeBNModlRessource *ress; */

	for(cap=0; cap<detectr->nbcapt; cap++) DetecteurVoieFinalise(detectr,cap);
	detectr->a_sauver = 0;
	DetecteurScriptEnregistre(detectr,"demarrage",&(detectr->start),log);
	DetecteurScriptEnregistre(detectr,"maintenance",&(detectr->regul),log);

	/* for(num=0; num<detectr->nbreglages; num++) {
		regl = &(detectr->reglage[num]);
		cap = regl->capt; if(cap < 0) cap = 0;
		if((bb = regl->bb) < 0) continue;
		numeriseur = &(Numeriseur[bb]);
		if((k = regl->ress) >= 0) {
			modele_bn = &(ModeleBN[numeriseur->def.type]); ress = modele_bn->ress + k;
			if(ress->type != RESS_PUIS2) {
				if(ress->type == RESS_FLOAT) sprintf(regl->user,"%g",regl->val.r);
				else sprintf(regl->user,"%d",regl->val.i);
				NumeriseurRessUserDecode(numeriseur,k,regl->user,&(regl->val.i),&(regl->val.r)); // proprifie regl->user
			}
		}
	} */

}
/* ========================================================================== */
static void DetecteurDefiniStdLocal(char *hote) {
	/* Creation d'un detecteur 'standard' par defaut */
	int vm,config,classe;
	
	snprintf(DetecteurStandardLocal->fichier,MAXFILENAME,"%s_%s",DETEC_STANDARD,hote);
	strncpy(DetecteurStandardLocal->nom,DETEC_STANDARD,DETEC_NOM);
	strncpy(DetecteurStandardLocal->hote,hote,HOTE_NOM);
	DetecteurStandardLocal->local = 1;
	DetecteurStandardLocal->branche = 0;
	DetecteurStandardLocal->avec_regen = 0;
	DetecteurStandardLocal->numero = BoloNb;
	DetecteurStandardLocal->ref = -1;
	DetecteurStandardLocal->lu = DETEC_HS;
	DetecteurStandardLocal->pos = CABLAGE_INDEFINI;
	strcpy(DetecteurStandardLocal->adresse,CablageEncodePosition(DetecteurStandardLocal->pos));
	DetecteurStandardLocal->nbcapt = 0;
	DetecteurStandardLocal->captr[0].bb.num = -1;
	DetecteurStandardLocal->captr[0].bb.serial = 0x7F;
	DetecteurStandardLocal->exec.inst = 0; DetecteurStandardLocal->exec.boucle = 0; DetecteurStandardLocal->exec.date = -1;
	DetecteurStandardLocal->a_sauver = 0;
	DetecteurStandardLocal->masse = 800.0; // 320.0;
	for(vm=0; vm<ModeleVoieNb; vm++) {
		memcpy(&(VoieStandard[vm]),&CaptParam,sizeof(TypeVoieStandard));
		memcpy(&(VoieStandard[vm].def.evt),&(ModeleVoie[vm]),sizeof(TypeVoieModele));
		// printf("(%s) Modele[%d]: '%s', Voie standard: '%s'\n",__func__,vm,ModeleVoie[vm].nom,VoieStandard[vm].def.evt.nom);
		for(config=0; config<CONFIG_MAX; config++) {
			VoieStandard[vm].config.def[config].defini.evt = VoieStandard[vm].config.def[config].defini.trgr = VoieStandard[vm].config.def[config].defini.rgul = 0;
			for(classe=0; classe<TRMT_NBCLASSES; classe++) VoieStandard[vm].config.def[config].defini.trmt[classe] = 0;
		}
	}

}
/* ========================================================================== */
void DetecteurAjouteStd() {
	BoloStandard = BoloNb;
	memcpy(Bolo+BoloStandard,DetecteurStandardLocal,sizeof(TypeDetecteur));
	Bolo[BoloStandard].numero = BoloStandard;
	Bolo[BoloStandard + 1].nom[0] = '\0';
	/* si on a declare le repartiteur, il doit etre mis en fin de liste, meme si Cluzel.taille == 0
	BoloRep = ++BoloNb;
	ReglageRazBolo(Bolo+BoloRep,"Repartiteur"); OU BIEN memcpy(Bolo+BoloRep,&Cluzel,sizeof(TypeDetecteur)); */
	/* Bolo[BoloRep].d2 = 100;  arbitraire.. mais il le faut, si! si! */
	
	printf("  . Detecteur standard: #%d (sera sauve dans %s)\n",BoloStandard,Bolo[BoloStandard].fichier);
}
/* ========================================================================== */
static int DetecteurLitAnnexe(char *nom_partiel, char *lien, TypeCablePosition pos_defaut, char stream_deja_lu, char log) {
	char nom_complet[MAXFILENAME];
	FILE *particulier;
	int nb_erreurs;

	if(log > 1) printf("  . Detecteur lu dans %s pour %s\n",nom_partiel,lien);
	if(nom_partiel[0] == SLASH) {
		int lngr;
		strcpy(nom_complet,nom_partiel);
		lngr = strlen(DetecteurRacine);
		if(!strncmp(DetecteurRacine,nom_complet,lngr)) nom_partiel = nom_complet+lngr;
		else nom_partiel = nom_complet;
	} else strcat(strcpy(nom_complet,DetecteurRacine),nom_partiel);
	if(!(particulier = fopen(nom_complet,"r"))) {
		printf("  ! Impossible de lire un detecteur dans '%s'\n  (%s)\n",nom_complet,strerror(errno));
		return(1);
	}
	if(log) printf("  . Lecture d'un detecteur, pour %s, dans le fichier %s\n",lien,nom_complet);
	/* le premier ordi declare est celui qui a le droit de modifier le bolo
	 {	char hote_tempo[HOTE_NOM];
		 strcpy(hote_tempo,hote_global);
		 r = hote_tempo;
		 nom_partiel = ItemJusquA('+',&r); 
		 if((!strcmp(nom_partiel,"local")) || (!strcmp(nom_partiel,Acquis[AcquisLocale].code)) || (!strcmp(nom_partiel,NomExterne))) {
			a_sauver = 1;
		 } else a_sauver = 0;
	 } */
	ArgNomEnCours(nom_complet);
	nb_erreurs = DetecteursLitFichier(particulier,nom_partiel,lien,pos_defaut,stream_deja_lu,log);
	ArgNomEnCours(0);
	return(nb_erreurs);
}
#define MAX_ERREURS 256
/* ========================================================================== */
int DetecteursLitFichier(FILE *fichier, char *nom_partiel, char *lien, TypeCablePosition pos_defaut, char stream_deja_lu, char log) {
	char *hote_global;
	TypeCableConnecteur connecteur; TypeDetecteur *detectr; TypeBNModele *modele_bn; TypeConfigVoie *config_voie;
	TypeADU valeur;
	unsigned short ident;
	int bolo,dm,bb,cap,voie,vm,num_config,canal,sat; // ,vu,vm2
	int num_ligne,nb_erreurs,erreurs_bolo,erreurs_avant,courante,etage;
	int i,j,l;
	char config_ggale;
	char type_decla,type_lien,type_valeurs,peu_importe;
	char a_sauver,deja_declare,adrs_imposee,vrai_bolo,bolo_en_trop,abandon;
	char ligne[256],lue[256],origine[80],srce_defaut[2];
	char *r,*c,*vallue,*item,*decla,*nom,*modl,*ref,*source,*fichier_lu,*nouveau_nom; // char *hote;
	//char partiel_choisi,affect;
	char nom_complet[MAXFILENAME];
	// TypeReglage *regl;

/*
soit  lien: '*' <type_lien> <source>
			alors <hote_global>: (vide)
	
sinon lien: <hote_global>
			alors <type_lien>: '#'
			et    <source>: '1'

ligne: { 'modeles' '@' <fichier_modeles>
       | 'stock'   '@' <repertoire_stock>
       | '@' <hote_global> '=' ['@']<fichier>  -- <hote_global> dans <lien>
	   | '=' <nom> <type_lien> <source>  ':'   -- si <type_lien> <source> pas encore definis
       | '=' <nom> (ignore) (ignore)     ':'   -- si <type_lien> <source> definis avec lien: '*'
       | 'std' <hote> '>' '@'<fichier>   ':'   -- (lit_fichier avec type_lien: 's' et source: <hote>)
       | '<' <source> '>' '@'<fichier>   ':'   -- (lit_fichier avec type_lien: '<' et source)
       | '#' <source> '>' '@'<fichier>   ':'   -- (lit_fichier avec type_lien: '#' et source)
       | <position>   '>' '@'<fichier>   ':'   -- (lit_fichier avec type_lien: '@' et source: <position>)
       | 'std' <hote> '>' <nom> ':'    -- type_lien: 's'
       | '<' <source> '>' <nom> ':'    -- type_lien: '<'
       | '#' <source> '>' <nom> ':'    -- type_lien: '#'
       | <source>     '>' <nom> ':'    -- type_lien: '@'
       | <nom>                  ':'
       }
 
type_lien: { '@' | '#' | '>' | '<' }  -- <source>: { <position> | <adrs_numeriseur> | <fichier_numeriseur> | <fichier_numeriseur> } selon type_lien
nom: <nom_detecteur> [ '=' <detecteur_reference> ] [ '/' <modele> ]
*/
	if(lien[0] == '*') { type_lien = lien[1]; source = lien + 2; hote_global = "\0"; }
	else { type_lien = '#'; strcpy(srce_defaut,"1"); source = srce_defaut; hote_global = lien; } // si source="1" direct, ItemSuivant(&c,source) plante (protege)

	fichier_lu = (*nom_partiel)? nom_partiel: DetecteurNomLu;
	// a_sauver = SambaHoteLocal(hote_global);
	a_sauver = 1; /* tous les ordis ont le droit de modifier le bolo */
	num_ligne = 0;
	bolo_en_trop = 0;
	type_valeurs = DETEC_REGISTRE;
	connecteur = 0; dm = vm = courante = etage = 0; /* gnu */
	detectr = 0; bolo = 0;
	abandon = 0;
	nb_erreurs = 0;
	erreurs_bolo = 0;

/* -----------------------------------------------------------------------------
 *  Lecture du fichier
 */
	while(nb_erreurs < MAX_ERREURS) {
		if(!LigneSuivante(ligne,256,fichier)) break;
		num_ligne++;
		strcpy(lue,ligne);
		l = strlen(lue); if(lue[l-1] == '\n') lue[l-1]='\0';
		r = lue; // ligne;
		if(log > 1) printf("  . Lu: '%s'\n",lue);
	#ifdef DETECTR_NL
		if((num_ligne == 1) && !strncmp(r+2,DESC_SIGNE,strlen(DESC_SIGNE))) {
			ArgFromFile(fichier,DetectrDesc,0); num_ligne += ArgLignesLues();
			break;
		}
	#endif /* DETECTR_NL */
		if((*r == '#') || (*r == '\n') || (*r == '\0')) continue;
		erreurs_avant = nb_erreurs;
		decla = ItemAvant("@=>:",&type_decla,&r);
		if(log > 1) {
			printf("  . Declaration: '%s' (type '%c')\n",decla,type_decla);
			if(log > 2) {
				printf("  . Lue dans '%s'\n",/* nom_partiel? nom_partiel: */ "<indetermine>");
				printf("    > suite: '%s'\n",r);
			}
		}
		if(type_decla == '@') {
			if(!strncmp(decla,"modele",6)) {
				if(ModeleDetNb == 0) DetecteursModlLit(ItemJusquA(' ',&r),0);
				else printf("  * Modeles deja lus.\n");
			} else if(!strncmp(decla,"stock",5)) {
				strcpy(DetecteurStock,ItemJusquA(' ',&r));
				RepertoireTermine(DetecteurStock);
				RepertoireIdentique(FichierPrefDetecteurs,DetecteurStock,nom_complet);
				printf("  . Stock de detecteurs dans '%s'\n",nom_complet);
			} else /* en fait, on doit avoir ici *decla = '\0' */ {
				item = ItemJusquA('=',&r);
				if(log > 1) { printf("  . Hote: '%s'\n",item); if(log > 2) printf("    > suite: '%s'\n",r); }
				if(!strncmp(item,"EDWACQ-",7)) strncpy(hote_global,item+7,HOTE_NOM);
				else strncpy(hote_global,item,HOTE_NOM);
				hote_global[HOTE_NOM-1] = '\0';
				// if(!strcmp(hote_global,"liste")) strcpy(FichierListeDet,FichierPrefDetecteurs);
				nouveau_nom = ItemJusquA(' ',&r);
				if(log > 1) { printf("  . Nom: '%s'\n",nouveau_nom); if(log > 2) printf("    > suite: '%s'\n",r); }
				if(*nouveau_nom == '@') nouveau_nom++;
				erreurs_bolo += DetecteurLitAnnexe(nouveau_nom,hote_global,pos_defaut,stream_deja_lu,log);
			}

		} else if((type_decla == '>') || (type_decla == ':') || ((type_decla == '=') && (*decla == '\0'))) {
			if(type_decla == '=') {
				/* Declaration de detecteur, optionnellement avec infos sur la meme ligne sous la forme:
				   '='<nom>['/'<type>]['='<original>] ['@'<position_interne> | '>'<fichier_numeriseur> | '#'<adresse>] ':' <status> */
				if(lien[0] == '*') {
					nom = ItemAvant("@#><",&peu_importe,&r); ItemJusquA(':',&r);
				} else {
					nom = ItemAvant("@#><",&type_lien,&r);
					if(log > 1) { printf("  . Nom: '%s'\n",nom); if(log > 2) printf("    > suite: '%s'\n",r); }
					source = ItemJusquA(':',&r);
				}
			} else if(type_decla == '>') {
				/* Declaration de detecteur, optionnellement avec infos sur la meme ligne sous la forme:
				 { <position_interne> | '<'<fichier_numeriseur> | '#'<adresse> } '>' { <nom>['/'<type>]['='<original>] ':' <status> | @<fichier> } */
				ref = decla + 1;
				if(!strncmp(decla,"std",3)) { type_lien = 's'; ref = decla + 3; }
				else if(*decla == '<') type_lien = '<'; else if(*decla == '#') type_lien = '#'; else { type_lien = '@'; ref = decla; }
				source = ItemJusquA(0,&ref);
				if(log > 1) printf("  . Source: '%s'\n",source);
				nom = ItemJusquA(':',&r);
				if(*nom == '@') {
					nom++;
					sprintf(origine,"*%c%s",type_lien,source);
					// printf("(%s) Lecture de '%s', origine '%s', pos par defaut 0x%x\n",__func__,nom,origine,pos_defaut);
					erreurs_bolo += DetecteurLitAnnexe(nom,origine,pos_defaut,stream_deja_lu,log);
					continue;
				}
			} else /* donc, if(type_decla == ':') */ nom = decla;
			ref = nom; ItemJusquA('=',&ref);
			c = nom; ItemJusquA('/',&c); modl = ItemJusquA(' ',&c);
			if((type_decla == '=') && (type_lien == '\0')) { type_lien = '@'; source = ItemJusquA(':',&c); }
			if(log > 1) { printf("  . Modele: '%s'\n",modl); }
			adrs_imposee = (type_lien == '#');
			if(detectr) {
				if(detectr->d2 == 0) detectr->d2 = 100;
				if(detectr->d3 == 0) detectr->d3 = 1000;
				detectr->taille = CommandesNb - detectr->debut;
			}
			if(log > 1) printf("  . Trouve la declaration du detecteur '%s' (de type '%s')\n",nom,modl);
			if(fichier == DetecteurBaseFile) DetecteurListeExiste = 0;
			// if(*ref) printf(" (copie de %s)\n",ref); else printf("\n");
			dm = 0; deja_declare = 0;
			// if(!strcmp(nom,NOM_REP)) { detectr = a_sauver? &Cluzel: &DetecteurAJeter; bolo = -1; } else 
			DetecteursModlVerifie();
			if(!strcmp(nom,DETEC_STANDARD)) {
				if((type_lien != 's') || !strcmp(source,".")) { source = Acquis[AcquisLocale].code; sat = AcquisLocale; detectr = DetecteurStandardLocal; }
				else {
					for(sat=0; sat<DetecStandardNb; sat++) if(!strcmp(source,Acquis[sat].code)) break;
					if(sat >= DetecStandardNb) {
						if(DetecStandardNb >= MAXSAT) {
							detectr = &DetecteurAJeter; // pas tres utile
							printf("  ! %d detecteur%s standard%s maximum, nouvelle definition pour %s abandonnee\n",Accord2s(DetecStandardNb),source);
							nb_erreurs++;
							abandon = 1; break;
						} else {
							detectr = &(DetecteurStandard[DetecStandardNb++]);
							strncpy(detectr->hote,source,HOTE_NOM);
							if(*nom_partiel) strncpy(detectr->fichier,nom_partiel,MAXFILENAME);
							else snprintf(detectr->fichier,MAXFILENAME,"%s_%s",DETEC_STANDARD,source);
							printf("  * Nouveau %s, pour %s\n",detectr->nom,detectr->hote);
						}
					} else detectr = &(DetecteurStandard[sat]);
				}
				if(CompteRendu.detectr.fichier) printf("  . Detecteur %s memorise separement dans '%s'\n",detectr->nom,detectr->fichier);
				if(sat == AcquisLocale) DetecteurStandardListe = 1; else { abandon = 1; break; }
				bolo = -1; source = 0;
			} else {
				/* Modele de detecteur (info peut-etre inutile) */
				if(*modl != '\0') {
					for(dm=0; dm<ModeleDetNb; dm++) if(!strcmp(modl,ModeleDet[dm].nom)) break;
					if(dm >= ModeleDetNb) {
						dm = 0;
						printf("  ! Le type de detecteur '%s' est inconnu! (remplace par '%s')\n",modl,ModeleDet[dm].nom);
						nb_erreurs++;
					}
				}
				/* detecteur deja existant? */
				for(bolo=0; bolo<BoloNb; bolo++) if(!strcmp(nom,Bolo[bolo].nom)) break;
				if(!stream_deja_lu) {
					detectr = Bolo + bolo;
					if(bolo < BoloNb) {
						/* deja existant: on s'y raccorde, ou bien c'est interdit. Remplacer a_sauver par a_lire */
						if(detectr->a_sauver || !a_sauver) {
							printf("  ! Le detecteur '%s' est defini en plusieurs fois!\n",nom);
							nb_erreurs++;
						} else /* ici, le bolo a deja ete declare dans la liste mais reste a remplir */ {
							deja_declare = 1;
							dm = detectr->type;
							j = detectr->ref;
							if((j >= 0) && (j < BoloNb)) memcpy(detectr,Bolo + j,sizeof(TypeDetecteur));
						}
					} else {
						/* pas encore existant: on le rajoute et on commence a remplir */
						if(BoloNb >= DetecteursMax) {
							printf("  ! Pas plus de %d detecteurs geres. Derniers detecteurs non relus\n",DetecteursMax);
							nb_erreurs++;
							bolo_en_trop = 1;
							abandon = 1; break;
						}
						nb_erreurs += DetecteurDeclare(detectr,bolo,nom,dm,ref);
					}
				} else {
					if(bolo < BoloNb) { detectr = Bolo + bolo; Bolo[bolo].local = 1; }
					else { detectr = &DetecteurAJeter; ReglageRazBolo(&DetecteurAJeter,"Poubelle"); }
				}
				DetecteurInfo(detectr,hote_global,nom_partiel,a_sauver);
				if(detectr->fichier[0] == '\0') ReglageDetecTousIndiv = 0;
			}
			// if(affect != '=') detectr->fichier[0] = '\0';
			if(bolo < 0) /* Cluzel ou Standard ou AJeter */ {
				detectr->lu = DETEC_HS; // bof
				detectr->pos = CABLAGE_INDEFINI;
				if(source) sscanf(source,"%x",&l); else l = 0xFF; ident = l & 0xFFFF;
				detectr->nbcapt = 0;
				detectr->captr[0].bb.num = -1;
				detectr->captr[0].bb.serial = ident & 0xFF;
			} else {
				if(bolo_en_trop) detectr->lu = DETEC_HS;
				else detectr->lu = ArgKeyGetIndex(DETEC_STATUTS,ItemSuivant(&r,'('));
				if(!deja_declare) {
					/* nouveau detecteur: on tient compte de la localisation et on acheve la definition */
					if(!DetecteurAjouteVoies(detectr,&nb_erreurs)) bolo_en_trop = 1;
					/* liste des reglages selon type de detecteur (NTD, NbSi, ...) */
					DetecteurAjouteReglages(detectr,&nb_erreurs,log);
					/* attribution des adresses (adresse numeriseur, sous-adresse ADC) */
					// printf("(%s) Placement de '%s' sur '%s' type '%c' (par defaut: 0x%x)\n",__func__,detectr->nom,source?source:"rien",type_lien,pos_defaut);
					if((type_lien == '#') || (type_lien == '>') || (type_lien == '<')) 
						DetecteurBrancheDirect(detectr,type_lien,source,fichier,&nb_erreurs);
					else {
						if(!source) pos_defaut = CABLAGE_POS_DIR;
						if(pos_defaut == CABLAGE_INDEFINI) 
							DetecteurPlace(detectr,CablageDecodePosition(source));
						else DetecteurPlace(detectr,pos_defaut);
						if(!DetecteurBrancheNumeriseurs(detectr)) {
							printf("  ! Detecteur %s inaccessible, mis hors service\n",detectr->nom);
							nb_erreurs++;
						}
					}
					DetecteurConnecteVoies(detectr,&nb_erreurs);
					DetecteurConnecteReglages(detectr,&nb_erreurs);
				}
				if(detectr->hote[0] && strcmp(detectr->hote,"HS") && !DetecteurListeSat(detectr)) {
					printf("- Deja %d satellites demandes ou hote inconnu: detecteur %s (lu en %s) inutilisable\n",
						   AcquisNb,detectr->nom,detectr->hote);
					nb_erreurs++;
				}
			}
			detectr->debut = CommandesNb;
			type_valeurs = DETEC_REGISTRE; /* les valeurs suivantes sont des commandes */
			/* on va quand meme attendre "parms detecteur" pour la liste des voisins
			if((affect == '=') && !(detectr->a_sauver)) {
				fclose(fichier);
				fichier = principal_deja_ouvert;
				principal_deja_ouvert = 0; nom_complet[0] = '\0';
				a_sauver = 1;
			} */

		} else {
			char *objet,*nom_config; int reglage; char configurable;
			if(!strncmp(decla,"modele",6)) { DetecteursModlLit("*",fichier); continue; }
			item = decla; nom_config = 0;
			vallue = ItemJusquA(' ',&r);
			objet = ItemSuivant(&item,' '); ref = ItemSuivant(&item,' ');
			if(log > 1) printf("  . Type de valeur attendu: %s, parametrage de %s %s (pour le %s %s) en ligne %d\n",
				(type_valeurs == DETEC_REGISTRE)?"registre":((type_valeurs == DETEC_REGLAGE)?"reglage":"rien de special"),objet,ref,
				(bolo >= 0)?"detecteur":"modele",(bolo >= 0)?Bolo[bolo].nom:((dm >= 0)?ModeleDet[dm].nom:"indefini"),num_ligne);
			num_config = -1; config_ggale = -1;
			for(reglage = DETEC_PARM_EVTS; reglage<DETEC_NBDEFS; reglage++) if(!strcmp(objet,DetecteurParmsUser[reglage])) break;
			configurable = ((reglage < DETEC_PARM_SOFT) || (reglage == DETEC_PARM_TRMT));
			if(!strncmp(ref,"config/",7)) {
				nom_config = ref + 7;
				if(nom_config[0] == '\0') nom_config = CONFIG_STD;
				ref = ItemSuivant(&item,' ');
			} else nom_config = CONFIG_STD;
			vrai_bolo = (bolo >= 0);
			if((reglage < DETEC_PARM_SOFT) || (reglage == DETEC_PARM_GENL) || (reglage == DETEC_PARM_TRMT)) {
				cap = DetecteurCapteurUtilise(detectr,dm,fichier_lu,vrai_bolo,ref,&vm);
				if(vrai_bolo) {
					canal = voie = (cap >= 0)? detectr->captr[cap].voie: -1;
					if(voie >= 0) { if(VoieManip[voie].config.nb) num_config = -1; }
				} else canal = vm;
			} else canal = -1;
			if(canal >= 0) {
				if(configurable) {
					if(GestionConfigs) printf("  . Configuration %s #%d (%s) pour la voie %s\n",objet,num_config,nom_config,VoieManip[voie].nom);
					for(config_ggale=0; config_ggale<ConfigNb; config_ggale++) if(!strcmp(nom_config,ConfigListe[config_ggale])) break;
					if(config_ggale >= ConfigNb) {
						if(GestionConfigs) printf("  . Nouvelle configuration generale: '%s' (%d deja cree%s)\n",nom_config,Accord1s(ConfigNb));
						if(ConfigNb < CONFIG_TOT) {
							strncpy(ConfigListe[ConfigNb++],nom_config,CONFIG_NOM); ConfigNom[ConfigNb][0] = '\0';
						} else {
							printf("! Deja %d configurations definies, au total. En supprimer une, avant nouvel ajout\n",CONFIG_TOT);
							config_ggale = -1; nb_erreurs++;
						}
					}
					if(GestionConfigs) printf("  . Configuration generale #%d (%s) (%d configuration%s au total)\n",config_ggale,ConfigListe[config_ggale],Accord1s(ConfigNb));
					if(vrai_bolo) config_voie = &(VoieManip[voie].config); else config_voie = &(VoieStandard[vm].config);
					for(num_config=0; num_config<config_voie->nb; num_config++) {
						if(!strcmp(config_voie->def[num_config].nom,nom_config)) break;
					}
					if(num_config >= config_voie->nb) {
						if(GestionConfigs) printf("  . Nouvelle configuration lue: '%s'\n",nom_config);
						if(config_voie->nb < CONFIG_MAX) {
							num_config = config_voie->nb;
							strncpy(config_voie->def[num_config].nom,nom_config,CONFIG_NOM);
							config_voie->nb += 1;
						} else {
							printf("! Deja %d configurations definies pour la voie %s. En supprimer une, avant nouvel ajout",CONFIG_MAX,VoieManip[voie].nom);
							nb_erreurs++;
							cap = -1;
						}
					}
				}
			} else cap = -1;
			if(*objet == '}') type_valeurs = DETEC_REGISTRE;
			else if(reglage == DETEC_PARM_GENL) { // (!strncmp(objet,"parms",4)) {
				if(cap < 0) {
					nb_erreurs++;
					ArgFromFile(fichier,CaptAutreDesc,"}"); num_ligne += ArgLignesLues();
					continue;
				}
				if(vm < 0) {
					DetecteurLu.mode_arch = ARCH_NBMODES;
					DetecteurLu.nbvoisins = DETEC_VOISINS_MAX;
					DetecteurLu.nbassoc = DETEC_ASSOC_MAX;
				#ifdef ForSamba
					strcpy(DetecteurLu.start.script,"neant");
					strcpy(DetecteurLu.regul.script,"neant");
				#endif					
					ArgFromFile(fichier,DetecParmsDesc,"}"); num_ligne += ArgLignesLues();
					if(vrai_bolo) {
						if(!ArgVarLue(DetecParmsDesc,"masse")) DetecteurLu.masse = detectr->masse;
						DetecteurCanalImporte(-1,-1,DETEC_REEL,DETEC_PARM_GENL,bolo);
						DetecteurCanalImporte(-1,-1,DETEC_REEL,DETEC_PARM_ENVR,bolo);
					}
					if(CompteRendu.detectr.assoc) {
						printf("  * Associes du detecteur %s: ",detectr->nom);
						if(detectr->nbassoc == 0) printf("neant\n");
						else {
							for(j=0; j<detectr->nbassoc; j++) { if(j) printf(", "); printf("%s",&(detectr->nom_assoc[j][0])); }
							printf("\n");
						}
					}
					/* Bon finalement, c'est plus simple de tout lire (notamment en cas d'hotes multiples) */
					//if(/* (affect == '=') && */ !(detectr->a_sauver)) {
					//	fclose(fichier);
					//	fichier = principal_deja_ouvert;
					//	principal_deja_ouvert = 0; nom_complet[0] = '\0';
					//	a_sauver = 1;
					//	continue;
					//}
				} else {
					ReglUsage = ARCH_DEFAUT;
					ReglSensibilite = 1.0;
					Rbolo = 1.0;
					ArgFromFile(fichier,CaptAutreDesc,"}"); num_ligne += ArgLignesLues();
					if(vrai_bolo) {
						VoieManip[detectr->captr[cap].voie].sauvee = ReglUsage;
						VoieManip[detectr->captr[cap].voie].sensibilite = ReglSensibilite;
						VoieManip[detectr->captr[cap].voie].Rbolo = Rbolo;
					}
				}

			} else if(reglage == DETEC_PARM_EVTS) { // (!strcmp(objet,DetecteurParmsUser[DETEC_PARM_EVTS])) {
				DetecteurParmsDefaut(DETEC_PARM_EVTS);
				ArgFromFile(fichier,CaptEvtsDesc,"}"); num_ligne += ArgLignesLues();
				if(cap < 0) { nb_erreurs++; continue; }
				DetecteurCanalImporte(num_config,config_ggale,vrai_bolo,DETEC_PARM_EVTS,canal);
				if(vrai_bolo) strcpy(VoieManip[voie].def.evt.nom,ModeleVoie[vm].nom);
				else strcpy(VoieStandard[vm].def.evt.nom,ModeleVoie[vm].nom);

			} else if(reglage == DETEC_PARM_TRMT) { // (!strcmp(objet,"traitement")) {
				DetecteurParmsDefaut(DETEC_PARM_TVOL);
				DetecteurParmsDefaut(DETEC_PARM_PREP);
				ArgFromFile(fichier,CaptTrmtDesc,"}"); num_ligne += ArgLignesLues();
				if(cap < 0) { nb_erreurs++; continue; }
				DetecteurCanalImporte(num_config,config_ggale,vrai_bolo,DETEC_PARM_TVOL,canal);
				DetecteurCanalImporte(num_config,config_ggale,vrai_bolo,DETEC_PARM_PREP,canal);

			} else if(reglage == DETEC_PARM_TVOL) { // (!strcmp(objet,DetecteurParmsUser[DETEC_PARM_TVOL])) {
				DetecteurParmsDefaut(DETEC_PARM_TVOL);
				ArgFromFile(fichier,CaptTvolDesc,"}"); num_ligne += ArgLignesLues();
				if(cap < 0) { nb_erreurs++; continue; }
				DetecteurCanalImporte(num_config,config_ggale,vrai_bolo,DETEC_PARM_TVOL,canal);

			} else if(reglage == DETEC_PARM_PREP) { // (!strcmp(objet,DetecteurParmsUser[DETEC_PARM_PREP])) {
				DetecteurParmsDefaut(DETEC_PARM_PREP);
				ArgFromFile(fichier,CaptPrepDesc,"}"); num_ligne += ArgLignesLues();
				if(cap < 0) { nb_erreurs++; continue; }
				DetecteurCanalImporte(num_config,config_ggale,vrai_bolo,DETEC_PARM_PREP,canal);

			} else if(reglage == DETEC_PARM_TRGR) { // (!strcmp(objet,DetecteurParmsUser[DETEC_PARM_TRGR])) {
				DetecteurParmsDefaut(DETEC_PARM_TRGR);
				ArgFromFile(fichier,CaptTrgrDesc,"}"); num_ligne += ArgLignesLues();
				if(cap < 0) { nb_erreurs++; continue; }
				DetecteurCanalImporte(num_config,config_ggale,vrai_bolo,DETEC_PARM_TRGR,canal);

			} else if(reglage == DETEC_PARM_RGUL) { // (!strcmp(objet,DetecteurParmsUser[DETEC_PARM_RGUL])) {
				DetecteurParmsDefaut(DETEC_PARM_RGUL);
				ArgFromFile(fichier,CaptRgulDesc,"}"); num_ligne += ArgLignesLues();
				if(cap < 0) { nb_erreurs++; continue; }
				DetecteurCanalImporte(num_config,config_ggale,vrai_bolo,DETEC_PARM_RGUL,canal);

			} else if(reglage == DETEC_PARM_SOFT) { // (!strcmp(objet,"pseudo")) {
				TypeComposantePseudo *pseudo;
				pseudo = DetecteurCompositionDecode(1,detectr,vallue,&vm,&nb_erreurs);
				DetecteurAjoutePseudo(detectr,pseudo,ref,vm,&nb_erreurs);

			} else if(reglage == DETEC_PARM_REGS) /* (!strncmp(objet,"registres",8)) */ type_valeurs = DETEC_REGISTRE;

			else if(reglage == DETEC_PARM_REGL) { // (!strncmp(objet,"reglages",7)) {
				type_valeurs = DETEC_REGLAGE; /* les valeurs suivantes sont des tensions pour la voie 'modele #vm' */
				if(!strcmp(ref,"=") || !strcmp(ref,"{")) { vm = -1; continue; }
				else if(!strcmp(ref,"detecteur")) {
					vm = -1;
					cap = 0; /* au moins le premier capteur doit etre connecte */
				} else {
					for(vm=0; vm<ModeleVoieNb; vm++) if(!strcmp(ModeleVoie[vm].nom,ref)) break;
					if(vm == ModeleVoieNb) {
						printf("  ! Modele de voie inconnu: '%s'\n",ref);
						nb_erreurs++;
						continue;
					}
					for(cap=0; cap<detectr->nbcapt; cap++) if(detectr->captr[cap].modele == vm) break;
					if(cap >= detectr->nbcapt) {
						printf("  ! Un detecteur de type %s n'a pas de voie de type %s\n",ModeleDet[dm].nom,ref);
						nb_erreurs++;
						continue;
					}
				}
//				printf(". Reglages de la voie %s\n",(vm >= 0)? ModeleVoie[vm].nom: "generale");
				if(detectr->captr[cap].bb.num < 0) {
					if(detectr->lu != DETEC_HS) {
						detectr->lu = DETEC_HS;
						printf("  ! Reglages %s ignores: cette voie n'est lue par aucun numeriseur\n",
							(vm >= 0)? ModeleVoie[vm].nom: "detecteur");
						nb_erreurs++;
					}
				} else if(detectr->nbreglages == 0) {
					printf("  ! Reglages %s ignores (pas de numeriseur, ou bien modele de detecteur sans reglage)\n",
						   (vm >= 0)? ModeleVoie[vm].nom: "detecteur");
					nb_erreurs++;
				}

			} else if(!strcmp(vallue,"{")) {
				if(detectr->a_sauver) printf("  ! Parametrage '%s' ignore\n",objet);
				type_valeurs = DETEC_VIDE;

			} else if(type_valeurs == DETEC_REGISTRE) {         /* tout registre du numeriseur (boitier bolo) */
				if(CommandesNb >= MAXCDES) {
					printf("  ! Pas plus de %d commandes au total. Derniers bolos non relus\n",MAXCDES);
					nb_erreurs++;
					abandon = 1; break;
				}
				if((*objet == 'x') || (*objet == '.'))  /* la valeur du mot a envoyer est directement lue ici */
					 sscanf(objet+1,"%x",&(Commandes[CommandesNb++]));
				else {                                /* le registre a modifier est designe par son nom */
					char ss_adrs; short hval;
					if((*vallue == 'x') || (*vallue == '.')) sscanf(vallue+1,"%hx",&valeur);
					else sscanf(vallue,"%hd",&valeur);
					if(!strcmp(objet,"d2")) {
						ss_adrs = 0x02;
						Commandes[CommandesNb++] = (ss_adrs << 16) | valeur;
						detectr->d2 = valeur;
					} else if(!strcmp(objet,"d3")) {
						ss_adrs = 0x03;
						Commandes[CommandesNb++] = (ss_adrs << 16) | valeur;
						detectr->d3 = valeur;
					} else {
						bb = detectr->captr[0].bb.num;
						modele_bn = &(ModeleBN[Numeriseur[bb].def.type]);
						for(i=0; i<modele_bn->nbress; i++) if(!strcmp(objet,modele_bn->ress[i].nom)) break;
						if(i >= modele_bn->nbress) {
							printf("  ! Ressource inconnue dans un numeriseur de type %s: '%s'\n",modele_bn->nom,objet); i = 0;
							nb_erreurs++;
						}
						ss_adrs = modele_bn->ress[i].ssadrs;
						Commandes[CommandesNb++] = (ss_adrs << 16) | valeur;
						Numeriseur[bb].ress[i].hval = valeur;
						NumeriseurRessHexaChange(&(Numeriseur[bb]),i);
						hval = (short)Numeriseur[bb].ress[i].val.i;
						// cap = modele_bn->ress[i].voie;
						switch(modele_bn->ress[i].categ) {
						  case RESS_D2: detectr->d2 = hval; break;
						  case RESS_D3: detectr->d3 = hval; break;
						  /* cap indefini:
						  case RESS_GAIN: detectr->captr[cap].numgain = hval;
							sscanf(&(Numeriseur[bb].cval[i][0]),"%g",&(detectr->captr[cap].gain_reel));
							break;
							  */
						  case REGL_RESET: detectr->reset = hval; break;
						  case RESS_FET_MASSE: detectr->razfet_en_cours = (hval > 0); break;
						}
					}
				}

			} else if(type_valeurs == DETEC_REGLAGE) {  /* tensions et autres reglages du bolo */
				if(!stream_deja_lu) {
					int bb; short k; TypeNumeriseur *numeriseur;
					// printf("? Reglage lu: %s\n",ref);
					DicoTraduit(SambaDicoDefs + SAMBA_DICO_REGLAGES,&objet);
					for(i=0; i<detectr->nbreglages; i++) {
						// printf("? Reglage demande: %s, reglage existant: %s\n",objet,ModeleDet[dm].regl[i].nom);
						if(!strcmp(objet,detectr->reglage[i].nom)) break;
					}
					if(i == detectr->nbreglages) {
						printf("  ! Reglage illegal pour le detecteur %s: '%s'\n",detectr->nom,objet);
						nb_erreurs++;
					} else {
						TypeReglage *regl;
						if(vm == ModeleVoieNb) {
							vm = ModeleDet[dm].regl[i].capteur;
							printf("  ! Le type de voie '%s' est assigne au reglage '%s'\n",ModeleVoie[vm].nom,objet);
						} else if((vm >= 0) && (ModeleDet[dm].regl[i].capteur != vm)) {
							printf("  ! Le reglage '%s' ne concerne pas la voie %s\n",objet,ModeleVoie[vm].nom);
							nb_erreurs++;
						}
						// printf("  ! Reglage trouve pour la voie %s\n",ModeleVoie[vm].nom);
						regl = &(detectr->reglage[i]);
//-						printf("(%s) Lu: %s.%s = %s\n",__func__,detectr->nom,regl->nom,vallue);
						cap = regl->capt;
						if((bb = regl->bb) >= 0) numeriseur = &(Numeriseur[bb]); else numeriseur = 0;
						if(numeriseur) {
							k = regl->ress;
							if((k >= 0) && (bb >= 0)) {
								strncpy(numeriseur->ress[k].cval,vallue,NUMER_RESS_VALTXT);
								NumeriseurRessUserChange(numeriseur,k);
								strcpy(regl->std,numeriseur->ress[k].cval);
								strcpy(regl->user,numeriseur->ress[k].cval);
//-								printf("(%s) Standard: %s.%s = %s\n",__func__,detectr->nom,regl->nom,regl->std);
								if(ModeleBN[numeriseur->def.type].ress[k].type == RESS_FLOAT)
									regl->val.r = numeriseur->ress[k].val.r;
								else regl->val.i = numeriseur->ress[k].val.i;
								regl->modifie = 0;
								// cap = modele_bn->ress[k].voie;
								switch(regl->cmde) {
									case REGL_D2: detectr->d2 = regl->val.i; break;
									case REGL_D3: detectr->d3 = regl->val.i; break;
									case REGL_GAIN: detectr->captr[cap].numgain = regl->val.i; 
										sscanf(numeriseur->ress[k].cval,"%g",&(detectr->captr[cap].gain_reel));
										break;
									case REGL_RESET: detectr->reset = regl->val.i; break;
									case REGL_FET_MASSE: detectr->razfet_en_cours = (regl->val.i > 0); break;
								}
							} else {
								if(detectr->branche && (bb >= 0)) {
									printf("  ! Reglage inaccessible pour le detecteur %s: '%s'\n",detectr->nom,objet);
									nb_erreurs++;
								}
								strncpy(regl->std,vallue,NUMER_RESS_VALTXT);
								strncpy(regl->user,vallue,NUMER_RESS_VALTXT);
								regl->modifie = 0;
							}
							k = regl->ressneg;
							if((k >= 0) && (bb >= 0)) {
								strcat(strcpy(numeriseur->ress[k].cval,"-"),vallue);
								NumeriseurRessUserChange(numeriseur,k);
							}
							k = regl->resspos;
							if((k >= 0) && (bb >= 0)) {
								strcpy(numeriseur->ress[k].cval,vallue);
								NumeriseurRessUserChange(numeriseur,k);
							}
						}
					}
				}
			}
		}
		if(erreurs_avant < (nb_erreurs - 1)) printf("  ! %d erreur%s dans la ligne #%d (concernant %s):\n    '%s'\n",
			Accord1s(nb_erreurs-erreurs_avant),num_ligne,detectr->nom,lue);
		erreurs_avant = nb_erreurs;
	}
	if(!feof(fichier) && !abandon) {
		printf("  ! Lecture du fichier <det>/%s en erreur (%s)\n",fichier_lu,strerror(errno));
		nb_erreurs++;
	}
	if(nb_erreurs) {
		printf("  ! %d erreur%s dans le fichier <det>/%s\n",Accord1s(nb_erreurs),fichier_lu);
	}
	fclose(fichier);
	if(detectr) {
		if(detectr->d2 == 0) detectr->d2 = 100;
		if(detectr->d3 == 0) detectr->d3 = 1000;
		detectr->taille = CommandesNb - detectr->debut;
	}
	return(nb_erreurs + erreurs_bolo);
}
/* ========================================================================== */
void DetecteursStructInit() {
	int i,vm;
	
	strcpy(DetecteurStock,"./");
	ReglageDetecTousIndiv = 1;
	ReglagesEtatDemande = 0;
	DetecteurBaseFile = 0;
	DetecteurListeExiste = 1;

	for(i=DETEC_PARM_EVTS; i<=DETEC_PARM_RGUL; i++) DetecteurParmsDefaut(i);
	// ReglageRazBolo(&Cluzel,NOM_REP);
	// for(cap=0; cap<DETEC_CAPT_MAX; cap++) Cluzel.captr[cap].bb.adrs = 0x7F;
	// Cluzel.taille = 0; 
	for(i=0; i<MAXSAT; i++) ReglageRazBolo(&(DetecteurStandard[i]),DETEC_STANDARD);
	ReglageRazBolo(&DetecteurAJeter,"Poubelle");
	DetecStandardNb = 0;
	DetecAvecReglages = DetecAvecPolar = DetecAvecRaz = DetecAvecModulation = 0;
	BolosUtilises = 0; BolosAdemarrer = BolosAentretenir = 0;
	ModlDetecChoisi = -1;

	ModelePhysNb = 0;
	for(i=0; i<PHYS_TYPES; i++) {
		ModelePhys[i].nom[0] = '\0';
		ModelePhysListe[i] = ModelePhys[i].nom;
	}
	ModelePhysListe[i] = "\0";
	ModeleVoieNb = 0;
	for(vm=0; vm<VOIES_TYPES; vm++) {
		ModeleVoie[vm].nom[0] = '\0';
		ModeleVoieListe[vm] = ModeleVoie[vm].nom;
		ModeleVoie[vm].utilisee = 0;
	}
	ModeleVoieListe[vm] = "\0";
	ModeleDetNb = 0;
	for(i=0; i<DETEC_TYPES; i++) {
		ModeleDet[i].nom[0] = '\0';
		ModeleDet[i].num = i;
		ModeleDet[i].avec_regen = 0;
		ModeleDet[i].suivant = 1;
		ModeleDet[i].duree_raz = SettingsRazNtdDuree;
		ModeleDetListe[i] = ModeleDet[i].nom;
	}
	ModeleDetListe[i] = "\0";
	
}
/* ========================================================================== */
int DetecteursLit(char *nom_principal, char stream_deja_lu) {
	int bolo,cap,voie,i,l,sat;
	int bolo_avant,cdes_avant,nb_erreurs,erreurs_avant;
	char il_faut_un_std;
	char log;
	char *nom_partiel;
	char hote_global[HOTE_NOM];
	FILE *fichier;

	log = 0;
	hote_global[0] = '\0'; //- strcpy(hote_global,"local");

/*
 *  Initialisation du tableau
 */
	DetecteurNomLu = rindex(FichierPrefDetecteurs,SLASH) + 1;
	l = DetecteurNomLu - FichierPrefDetecteurs;
	strncpy(DetecteurRacine,FichierPrefDetecteurs,l); DetecteurRacine[l] = '\0';

	for(sat=0; sat<AcquisNb; sat++) {
		snprintf(DetecteurStandard[sat].fichier,MAXFILENAME,"%s_%s",DETEC_STANDARD,Acquis[sat].code);
		strncpy(DetecteurStandard[sat].hote,Acquis[sat].code,HOTE_NOM);
	}
	DetecteurStandardLocal = &(DetecteurStandard[AcquisLocale]);
	DetecteurDefiniStdLocal(Acquis[AcquisLocale].code);
	DetecStandardNb = AcquisNb;
	DetecteurStandardListe = 0;	
	
	fichier = fopen(FichierPrefDetecteurs,"r");
	if(!fichier && InstalleSamba) {
		char nom[256];
		printf("> Installe des detecteurs  dans '%s'\n",DetecteurRacine);
		DetecteursModlVerifie();
		BoloNb = 0;
		strcat(strcpy(nom,ModeleDet[0].nom),"001");
		DetecteurCreeTempo(nom,0,Acquis[AcquisLocale].code,&nb_erreurs);
		strcat(strcpy(nom,ModeleDet[1].nom),"001");
		DetecteurCreeTempo(nom,1,Acquis[AcquisLocale].code,&nb_erreurs);
		DetecteurAjouteStd();
		ReglageDetecTousIndiv = 0;
		DetecteurEcritTout(0,0);
		fichier = fopen(FichierPrefDetecteurs,"r");
	}
	if(!stream_deja_lu) {
		BoloNb = 0 ; CommandesNb = 0; /* Vidage prealable, mais pas obligatoire ... */
		BoloStandard = -1;
	}
	printf("\n= Lecture des detecteurs                dans '%s'\n",FichierPrefDetecteurs);
	printf("  . Repertoire racine (<det>):    %s\n",DetecteurRacine);
	if(!fichier) {
		printf("! Impossible de lire les detecteurs d'apres:\n  %s\n  (%s)\n",
			FichierPrefDetecteurs,strerror(errno));
		return(1);
	}
	DetecteurBaseFile = fichier;
	bolo_avant = BoloNb;
	cdes_avant = CommandesNb;
	erreurs_avant = nb_erreurs;
	VoiesNb = 0;
	nb_erreurs = 0;
	nom_partiel = "\0";

	nb_erreurs += DetecteursLitFichier(fichier,nom_partiel,hote_global,CABLAGE_INDEFINI,stream_deja_lu,log);
	printf("  . %d detecteur%s connu%s\n",Accord2s(BoloNb));
	DetecteursModlVerifie();
	/* si on a declare un bloc global, il doit etre mis en fin de liste, meme si DetecteurStandardLocal.taille == 0 */
	il_faut_un_std = (!stream_deja_lu && (BoloNb > 1));
	if(il_faut_un_std) DetecteurAjouteStd();
	else {
		if(CompteRendu.detectr.compte) printf("  . Creation d'un standard: inutile\n");
		for(bolo=0; bolo<BoloNb; bolo++) {
			for(cap=0; cap<Bolo[bolo].nbcapt; cap++) {
				voie = Bolo[bolo].captr[cap].voie;
				VoieManip[voie].def.evt.std = 0;
				// printf("(%s) '%s'.evt.std @%08X = %x\n",__func__,VoieManip[voie].nom,(hexa)(&(VoieManip[voie].def.evt.std)),VoieManip[voie].def.evt.std);
				VoieManip[voie].def.trmt[TRMT_AU_VOL].std = 0;
				VoieManip[voie].def.trmt[TRMT_PRETRG].std = 0;
				VoieManip[voie].def.trgr.std = 0;
				VoieManip[voie].def.rgul.std = 0;
			}
		}
	}
	printf("  . %d configuration%s declaree%s\n",Accord2s(ConfigNb));
	for(i=0; i<ConfigNb; i++) printf("    | %s\n",ConfigListe[i]);

	ScriptRaz(&DetecScriptsLibrairie);
	for(bolo=0; bolo<BoloNb; bolo++) DetecteurTermine(&(Bolo[bolo]),log);
	if(il_faut_un_std && !DetecteurStandardListe) {
		DetecteurEcrit(0,&(Bolo[BoloStandard]),BoloStandard,0,DETEC_REGLAGES);
		DetecteurEcritTous(DETEC_LISTE,-1);
	}
	DetecteursDecompte();
#ifdef DETECTR_NL
	DetectrCtlgMode = DETEC_CTLG_ONLINE;
	DetectrModlMode = DETEC_MODL_ONLINE;
	memcpy(&ModeleDetLu,&(ModeleDet[Bolo[0].type]),sizeof(TypeDetModele));
	memcpy(&DetecteurLu,&(Bolo[0]),sizeof(TypeDetecteur));
	ArgPrint("/Users/gros/Desktop/Detecteur",DetectrDesc);
#endif

	if(nb_erreurs) printf("! %d erreur%s detectee%s au total dans la description des detecteurs\n",nb_erreurs,(nb_erreurs>1)?"s":"",(nb_erreurs>1)?"s":"");
	return(nb_erreurs);
}
/* ========================================================================== */
static INLINE char DetecteurAvecRegen() {
	int bolo; int i;
	TypeReglage *regl;
	
	for(bolo=0; bolo<BoloNb; bolo++) if(Bolo[bolo].a_lire && Bolo[bolo].avec_regen) {
		for(i=0; i<Bolo[bolo].nbreglages; i++) {
			regl = &(Bolo[bolo].reglage[i]);
			if((regl->cmde == REGL_POLAR) || (regl->cmde == REGL_FET_CC) || (regl->cmde == REGL_FET_MASSE)) return(1);
		}
	}
	return(0);
}
/* ========================================================================== */
void DetecteursDecompte() {
	TypeDetecteur *detectr;
	int bolo,premier_local,premier_lu; 

	BoloNum = 0;
	premier_local = premier_lu = -1;
	BolosUtilises = 0; BolosAdemarrer = BolosAentretenir = 0;
	BolosPresents = 0;
	for(bolo=0; bolo<BoloNb; bolo++) {
		detectr = &(Bolo[bolo]);
		if(detectr->local && (premier_local < 0)) premier_local = bolo;
		detectr->a_lire = (detectr->local && detectr->lu);
		if(detectr->a_lire) {
			if(premier_lu < 0) premier_lu = bolo;
			detectr->num_hdr = BolosPresents++;
			BolosUtilises++;
			if(Bolo[bolo].start.bloc >= 0) BolosAdemarrer++;
			if(Bolo[bolo].regul.bloc >= 0) BolosAentretenir++;
		} else if(SettingsHdrGlobal) detectr->num_hdr = BolosPresents++;
		else detectr->num_hdr = -1;

	}
	if(CompteRendu.detectr.compte) {
		printf("  . %d detecteur%s avec un script de demarrage\n",Accord1s(BolosAdemarrer));
		printf("  . %d detecteur%s avec un script d'entretien\n",Accord1s(BolosAentretenir));
	}
	if(premier_lu >= 0) BoloNum = premier_lu; else if(premier_local >= 0) BoloNum = premier_local;
	if((BoloNb > 1) && CompteRendu.detectr.compte) printf("  . Detecteur par defaut: %s\n",Bolo[BoloNum].nom);
	DetecteurListeCapteurs(0,0,0);
}
/* ========================================================================== */
void DetecteursVoisinage(char log) {
	/* On suppose detectr->local = SambaHoteLocal(detectr->hote); correctement a jour */
	TypeDetecteur *detectr;
	int bolo,ba,bv,autre,nb,max,k,n;
	short galette,tour,branche,gmilieu,tmilieu;
	char tout_seul;

	DetecteursDecompte();

	GaletteMax = 0; GaletteMin = 999999;
	for(bolo=0; bolo<BoloNb; bolo++) {
		detectr = &(Bolo[bolo]);
		if(detectr->assoc_imposes) {
			nb = 0;
			for(k=0; k<detectr->nbassoc; k++) {
				// printf("(%s) associe demande #%d: %s\n",__func__,k,&(detectr->nom_assoc[k][0]));
				for(autre=0; autre<BoloNb; autre++) if(!strcmp(&(detectr->nom_assoc[k][0]),Bolo[autre].nom)) break;
				if(autre < BoloNb) {
					// printf("(%s) associe accepte #%d: %s\n",__func__,nb-1,Bolo[autre].nom);
					if(detectr->a_lire && (Bolo[autre].lu != DETEC_OK)) 
						printf("! Le detecteur %s, associe de %s, n'est pas en service\n",Bolo[autre].nom,detectr->nom);
					detectr->assoc[nb++] = autre;
				} else {
					printf("! Le detecteur %s, declare associe de %s, n'est pas connu\n",&(detectr->nomvoisins[k][0]),detectr->nom);
					autre = -1;
				}
			}
		}
		CablageAnalysePosition(detectr->pos,&galette,&tour,&branche);
		if(galette > GaletteMax) GaletteMax = galette;
		if(galette < GaletteMin) GaletteMin = galette;
	}
	
	SettingsRegen = DetecteurAvecRegen();
	// printf("(%s) Galettes: [%d, %d]\n",__func__,GaletteMin,GaletteMax);
	if(GaletteMin == 999999) return;
	
	for(bolo=0; bolo<BoloNb; bolo++) {
		detectr = &(Bolo[bolo]);
		// printf("(%s) %s: %d voisins %s\n",__func__,detectr->nom,detectr->nbvoisins,detectr->voisins_imposes?"imposes":"libres");
		if(!detectr->voisins_imposes) {
			if(detectr->pos == CABLAGE_INDEFINI) {
				tout_seul = 1;
				for(k=0; k<detectr->nbassoc; k++) if((ba = detectr->assoc[k]) >= 0) {
					if(Bolo[ba].pos != CABLAGE_INDEFINI) {
						CablageAnalysePosition(detectr->pos,&gmilieu,&tmilieu,&branche);
						tout_seul = 0; break;
					}
				}
				if(tout_seul) { detectr->voisin[0] = bolo; detectr->nbvoisins = 1; continue; }
			} else CablageAnalysePosition(detectr->pos,&gmilieu,&tmilieu,&branche);
			// printf("(%s) %d/ %s en %d x %d\n",__func__,bolo,detectr->nom,gmilieu,tmilieu);
			nb = 0; max = 3;
			if(gmilieu == GaletteMin) --max;
			if(gmilieu == GaletteMax) --max;
			if(max > DETEC_VOISINS_MAX) max = DETEC_VOISINS_MAX;
			for(autre=0; autre<BoloNb; autre++) {
				if(Bolo[autre].pos == CABLAGE_INDEFINI) {
					tout_seul = 1;
					for(k=0; k<Bolo[autre].nbassoc; k++) if((ba = Bolo[autre].assoc[k]) >= 0) {
						if(Bolo[ba].pos != CABLAGE_INDEFINI) {
							CablageAnalysePosition(detectr->pos,&galette,&tour,&branche);
							tout_seul = 0; break;
						}
					}
					if(tout_seul) tour = -1;
				} else CablageAnalysePosition(detectr->pos,&galette,&tour,&branche);
				// printf("(%s)    %d. %s en %d x %d\n",__func__,autre,Bolo[autre].nom,galette,tour);
				if((tour == tmilieu) && (abs(gmilieu - galette) <= 1)) {
					// printf("(%s)    -> %s a un voisin #%d, le bolo #%d\n",__func__,detectr->nom,nb,autre);
					for(n=0; n<nb; n++) {
						bv = detectr->voisin[n];
						for(k=0; k<Bolo[bv].nbassoc; k++) if((ba = Bolo[bv].assoc[k]) == autre) break;
						if(k < Bolo[bv].nbassoc) break;
					}
					if(n >= nb) {
						// printf("(%s)    => voisin retenu\n",__func__);
						detectr->voisin[nb] = autre;
						if(++nb >= max) break;
					} // else printf("(%s)    => voisin abandonne\n",__func__);
				}
			}
			detectr->nbvoisins = nb;
		} else {
			nb = 0;
			for(k=0; k<detectr->nbvoisins; k++) {
				// printf("(%s) voisin demande #%d: %s\n",__func__,k,&(detectr->nomvoisins[k][0]));
				for(autre=0; autre<BoloNb; autre++) {
					if(!strcmp(&(detectr->nomvoisins[k][0]),Bolo[autre].nom)) break;
				}
				if(autre < BoloNb) {
					detectr->voisin[nb++] = autre;
					// printf("(%s) voisin accepte #%d: %s\n",__func__,nb-1,Bolo[autre].nom);
					if(detectr->a_lire && !Bolo[autre].a_lire) 
						printf("! Le detecteur %s, voisin de %s, n'est pas a lire\n",Bolo[autre].nom,detectr->nom);
				} else printf("! Le detecteur %s, declare voisin de %s, n'est pas connu\n",&(detectr->nomvoisins[k][0]),detectr->nom);
			}
		}
	}

	if(pEtatBolos) { if(OpiumDisplayed(pEtatBolos->cdr)) PanelRefreshVars(pEtatBolos); }
}
/* ========================================================================== */
int DetecteurActive(void *adrs) {
	if(OpiumExec(pDetecGestion->cdr) == PNL_CANCEL) return(0);
	DetecteursVoisinage(0);
	if(OpiumCheck(L_("Sauver cette liste"))) DetecteurEcritTous(DETEC_REGLAGES,-1);
	return(1);
}
/* ========================================================================== */
int DetecteursImprime(char tous) {
	int bolo,bb,ba,cap,cp,vbb,vm; int k,l,nb,autre,prec_num,prec_conn,rz,r1,r2,r3; char hote[20];
	char associes,associations,voisins,pb,un_seul_bb;
	TypeDetecteur *detectr; TypeRepartiteur *repart;
	unsigned short connecteur;
	
	l = 0; // GCC
	if(SettingsImprBolosVsn == MONIT_IMPR_BOLOS_BB1) {
		printf(" ____________________________________________________________________________________\n");
		printf("|                              Connection des detecteurs                             |\n");
		printf("|____________________________________________________________________________________|\n");
		//	printf("|                        | position |  prise   |    numeriseur    | systeme d'       |\n");
		//	printf("|       detecteur        | interne  | cryostat | #serie   adresse | acquisition      |\n");
		printf("|                        | position |           |  prise   |        numeriseur       |\n");
		printf("|       detecteur        | interne  |   type    | cryostat | #serie   adresse   hote |\n");
		printf("|________________________|__________|___________|__________|________|_________|______|\n");
		for(bolo=0; bolo<BoloNb; bolo++) if(Bolo[bolo].local || tous) {
			detectr = &(Bolo[bolo]);
			strncpy(hote,detectr->hote,16); hote[16] = '\0';
			for(cap=0; cap<detectr->nbcapt; cap++) {
				for(cp=0; cp<(cap-1); cp++) if(detectr->captr[cp].bb.num == detectr->captr[cap].bb.num) break;
				if(cp < (cap - 1)) continue;
				vm = detectr->captr[cap].modele;
				bb = detectr->captr[cap].bb.num;
				if(bb >= 0) { connecteur = Numeriseur[bb].fischer; vbb = detectr->captr[cap].bb.adc - 1; }
				else if(detectr->pos != CABLAGE_INDEFINI) {
					connecteur = CablageSortant[detectr->pos].captr[cap].connecteur;
					vbb = CablageSortant[detectr->pos].captr[cap].vbb;
				} else { connecteur = 0; vbb = -1; }
				if(cap == 0) printf("| %2s: %-18s |   %-6s | %-9s |",
					detectr->lu? "ok":"HS",detectr->nom,Bolo[bolo].adresse,ModeleDet[detectr->type].nom);
				else printf("| %2s  %-18s |   %-4s   | %-9s |","","","","");
				if(connecteur) printf(" %6s   |",CablageEncodeConnecteur(connecteur)); else printf("     -    |");
				if(detectr->captr[cap].bb.serial == 0xFF) printf("        |");
				else if(detectr->captr[cap].bb.serial) printf("  #%03d  |",detectr->captr[cap].bb.serial);
				else printf("  NEANT |");
				if(bb >= 0) {
					if(Numeriseur[bb].ident) printf("   %04X  |",Numeriseur[bb].ident);
					else printf("   ---   |");
				} else if(detectr->captr[cap].bb.adrs) printf("    %02X   |",detectr->captr[cap].bb.adrs);
				else printf(" ABSENTE |");
				if(cap == 0) { if(detectr->local) printf("  .   |"); else printf(" %-5s|",hote); } else printf("      |");
				printf("\n");
			}
		}
		//	printf("|________________________|__________|__________|________|_________|__________________|\n");
		printf("|________________________|__________|___________|__________|________|_________|______|\n");
		
	} else if(SettingsImprBolosVsn == MONIT_IMPR_BOLOS_MIXTE) {
		printf(" ____________________________________________________________________________________\n");
		printf("|                              Connection des detecteurs                             |\n");
		printf("|____________________________________________________________________________________|\n");
		printf("|                        | position |           | prise |    numeriseur   |     |    |\n");
		printf("|       detecteur        | interne  |  capteur  | cryo  | nom         ADC | Rep |hote|\n"); // adrs   ADC
		printf("|________________________|__________|___________|_______|____________|____|_____|____|\n");
		prec_num = -2; prec_conn = -2;
		for(bolo=0; bolo<BoloNb; bolo++) if(Bolo[bolo].local || tous) {
			detectr = &(Bolo[bolo]);
			strncpy(hote,detectr->hote,16); hote[16] = '\0';
			un_seul_bb = 0;
			bb = -1;
			for(cap=0; cap<detectr->nbcapt; cap++) if(detectr->captr[cap].bb.num >= 0) {
				if(bb < 0) bb = detectr->captr[cap].bb.num;
				else if(detectr->captr[cap].bb.num != bb) break;
			}
			if(cap >= detectr->nbcapt) un_seul_bb = 1;
			for(cap=0; cap<detectr->nbcapt; cap++) {
				vm = detectr->captr[cap].modele;
				bb = detectr->captr[cap].bb.num;
				if(bb >= 0) { connecteur = Numeriseur[bb].fischer; vbb = detectr->captr[cap].bb.adc - 1; repart = Numeriseur[bb].repart; }
				else if(detectr->pos != CABLAGE_INDEFINI) {
					connecteur = CablageSortant[detectr->pos].captr[cap].connecteur;
					vbb = CablageSortant[detectr->pos].captr[cap].vbb;
					repart = 0;
				} else { connecteur = 0; vbb = -1; prec_conn = -2; repart = 0; }
				// if((bb == prec_num) && (connecteur == prec_conn)) continue;
				if(cap == 0) {
					printf("| %2s: %-18s |   %-6s |",detectr->lu?"ok":"HS",detectr->nom,Bolo[bolo].adresse);
				} else printf("|                        |          |");
				if(un_seul_bb) printf(" %-9s |","(tous)");
				else printf(" %-9s |",ModeleVoie[detectr->captr[cap].modele].nom);
				if(connecteur != prec_conn) {
					if(connecteur) printf(" %5s |",CablageEncodeConnecteur(connecteur)); else printf("    -  |");
				} else printf("       |");
				if(bb != prec_num) {
					if(bb >= 0) {
						printf(" %-10s |",Numeriseur[bb].nom); // " %3.1f #%03d   |",Numeriseur[bb].version,detectr->captr[cap].bb.serial);
					} else printf("    NEANT   |");
					// if(detectr->captr[cap].bb.adrs) printf("  %02X |",detectr->captr[cap].bb.adrs); else printf("  -- |");
				} else printf("            |");
				if(un_seul_bb) printf("  * |"); else if(vbb >= 0) printf(" %2d |",vbb+1); else printf(" -- |");
				if((bb >= 0) && (bb != prec_num)) {
					if(repart) printf(" %-4s|",(bb >= 0)? Numeriseur[bb].code_rep: repart->code_rep);
					else printf(" --- |");
				} else printf("     |");
				if((bb != prec_num) || un_seul_bb) { if(detectr->local) printf(" .  |"); else printf(" %2s |",hote); } else printf("    |");
				printf("\n");
				prec_num = bb; prec_conn = connecteur;
				if(un_seul_bb) break;
			}
		}
		printf("|________________________|__________|___________|_______|____________|____|_____|____|\n");
	}

	if(BoloNb > 1) {
		associations = 0;
		if(tous && (BoloNb > 1)) {
			for(bolo=0; bolo<BoloNb; bolo++) if(Bolo[bolo].local || tous) {
				associes = 0;
				for(k=0; k<Bolo[bolo].nbassoc; k++) if((ba = Bolo[bolo].assoc[k]) >= 0) {
					if(!associations) {
						printf("|                             Associations de detecteurs                             |\n");
						printf("|____________________________________________________________________________________|\n");
						associations = 1;
					}
					if(!associes) l = printf("|       %-16s | ",Bolo[bolo].nom); else l += printf(", ");
					associes++;
					l += printf("%s",Bolo[ba].nom);
				}
				if(associes) SambaCompleteLigne(86,l);
			}
			if(!associations) printf("| Associations           | neant                                                     |\n");
		}
		voisins = 0;
		rz = 0;
		r1 = r2 = r3 = -1;
		for(bolo=0; bolo<BoloNb; bolo++) if(((Bolo[bolo].a_lire) || tous) && Bolo[bolo].nbvoisins) { voisins = 1; break; }
		if(voisins) {
			if(tous && (BoloNb > 1)) printf("|________________________|___________________________________________________________|\n");
			printf("|                              Voisinage des detecteurs                              |\n");
			printf("|____________________________________________________________________________________|\n");
			for(bolo=0; bolo<BoloNb; bolo++) if((Bolo[bolo].a_lire) || tous) {
				char liste_en_cours;
				l = printf("|   ");
				if(Bolo[bolo].a_lire) l += printf("    "); else { if(r2 < 0) r2 = ++rz; l += printf("[%d] ",r2); }
				l += printf("%-16s | ",Bolo[bolo].nom); liste_en_cours = 0;
				pb = 0;
				if(!Bolo[bolo].nbvoisins) { if(r3 < 0) r3 = ++rz; l += printf("pas de voisin!! [%d]",r3); }
				else for(nb=0; nb<Bolo[bolo].nbvoisins; nb++) {
					autre = Bolo[bolo].voisin[nb];
					if(liste_en_cours) l += printf(", ");
					if((Bolo[bolo].local && !Bolo[autre].local) || (!Bolo[bolo].local && Bolo[autre].local)) pb = 1;
					l += printf("%s",Bolo[autre].nom); liste_en_cours = 1;
					for(k=0; k<Bolo[autre].nbassoc; k++) if((ba = Bolo[autre].assoc[k]) >= 0) {
						if(k == 0) l += printf(" ("); else l += printf(", ");
						l += printf("%s",Bolo[ba].nom);
						if(k == (Bolo[autre].nbassoc - 1)) {
							l += printf("),");
							SambaCompleteLigne(86,l);
							l = printf("|       %-16s | "," ");
							liste_en_cours = 0;
						}
					}
				}
				if(Bolo[bolo].a_lire && pb) { if(r1 < 0) r1 = ++rz; l += printf(" [%d]",r1); }
				SambaCompleteLigne(86,l);
			}
		} else {
			if(associations) printf("|________________________|___________________________________________________________\n");
			printf("| Voisinages             | neant                                                     |\n");
		}
		printf("|________________________|___________________________________________________________|\n");
		for(k=0; k<rz; ) {
			k++;
			if(r1 == k) {
				l = printf("| [%d] Tous les detecteurs de cette liste ne sont pas lus par le meme systeme",r1); SambaCompleteLigne(86,l);
				l = printf("|     d'acquisition, et posent un probleme pour la sauvegarde des coincidences"); SambaCompleteLigne(86,l);
			} else if(r2 == k) {
				l = printf("| [%d] Ce detecteur n'est pas, de toutes facons, utilise dans ce run",r2); SambaCompleteLigne(86,l);
			} else if(r3 == k) {
				l = printf("| [%d] Ceci est anormal et traduit une erreur dans le logiciel",r3); SambaCompleteLigne(86,l);
			}
		}
		if(rz > 0) printf("|____________________________________________________________________________________|\n");
	}
	printf("\n");
	return(0);
}
/* ========================================================================== */
static void DetecteurDumpUnSeul(int bolo, char premier, char tout_seul) {
	int bb,cap,vbb,vm,voie; int i,k,lngr,prec_num,prec_conn;
	char nomBN[NUMER_NOM];
	TypeDetecteur *detectr; TypeReglage *regl;
	TypeDetModele *det_modele; TypeBNModlRessource *ress;
	unsigned short connecteur;

	lngr = 86; // GCC
	if(tout_seul) {
		lngr = 
		printf(" ____________________________________________________________________________________ \n");
	} else if(premier) {
		lngr = 
		printf(" ____________________________________________________________________________________\n");
		printf("|                           Cablage des reglages des detecteurs                      |\n");
		printf("|____________________________________________________________________________________|\n");
	}
	if(premier || tout_seul) {
		printf("|                   detecteur                 |                 numeriseur           |\n");
		printf("|  nom       | pos.|  capteur  | reglage      |prise type #serie  adrs   registre    |\n");
		printf("|____________|_____|___________|______________|____|_____________|____|______________|\n");
	}
	
	prec_num = -2; prec_conn = -2;
	detectr = &(Bolo[bolo]);
	det_modele = &(ModeleDet[detectr->type]);
	for(cap=0; cap<detectr->nbcapt; cap++) {
		vm = detectr->captr[cap].modele;
		bb = detectr->captr[cap].bb.num;
		if((bb != prec_num) && (prec_num != -2)) 
			printf("|            |     |           |              |----|-------------|----|--------------|\n");
		if(bb >= 0) { connecteur = Numeriseur[bb].fischer; vbb = detectr->captr[cap].bb.adc - 1; }
		else if(detectr->pos != CABLAGE_INDEFINI) {
			connecteur = CablageSortant[detectr->pos].captr[cap].connecteur;
			vbb = CablageSortant[detectr->pos].captr[cap].vbb;
		} else { connecteur = 0; vbb = -1; prec_conn = -2; }
		// if((bb == prec_num) && (connecteur == prec_conn)) continue;
		if(cap == 0) {
			k = printf("| %c%-10s| %-4s|",detectr->lu?' ':'!',detectr->nom,Bolo[bolo].adresse);
		} else k = printf("|            |     |");
		if((voie = detectr->captr[cap].voie) >= 0) {
			if(VoieManip[voie].pseudo) {
				TypeComposantePseudo *composant; float coef;
				k += printf(" %-9s | %-13s| ",detectr->captr[cap].nom,"(composee)");
				composant = VoieManip[voie].pseudo;
				do {
					coef = composant->coef;
					if(composant != VoieManip[voie].pseudo) {
						if(coef > 0.0) k += printf(" + ");
						else { k += printf(" - "); coef = -coef; }
					}
					k += printf("%g x %s",coef,detectr->captr[composant->srce].nom);
					composant = composant->svt;
				} while(composant);
				SambaLigneTable(' ',k,lngr);
				continue;
			}
		}
		k += printf(" %-9s | %-13s|",ModeleVoie[vm].nom,"(mesure)");
		if(connecteur != prec_conn) {
			if(connecteur) k += printf(" %-3s|",CablageEncodeConnecteur(connecteur)); else k += printf(" -- |");
			prec_conn = connecteur;
		} else k += printf("    |");
		if(bb != prec_num) {
			// if(detectr->captr[cap].bb.serial) {
			// 	sprintf(" %3.1f  #%03d  |",numeriseur,Numeriseur[bb].version,detectr->captr[cap].bb.serial);
			// } else strcpy(numeriseur,"NEANT");
			if(bb >= 0) strcpy(nomBN,Numeriseur[bb].nom); else strcpy(nomBN,"inconnu");
			k += printf("%-13s|",nomBN);
			prec_num = bb;
			if(Numeriseur[bb].def.adrs && (Numeriseur[bb].def.adrs != 0xFF)) k += printf(" %02X |",Numeriseur[bb].def.adrs); else k += printf(" -- |");
		} else k += printf("             |    |");
		if(vbb >= 0) k += printf(" ADC%-2d        |",vbb+1); else k += printf("              |");
		printf("\n");
		forever {
			for(i=0; i<detectr->nbreglages; i++) {
				regl = &(detectr->reglage[i]);
				if(cap != regl->capt) continue;
				bb = regl->bb;
				k = printf("|            |     |");
				if(cap >= 0) k += printf("           |"); else k += printf(" (general) |");
				k += printf(" %-13s|",det_modele->regl[i].nom);
				if(bb != prec_num) {
					if(connecteur != prec_conn) {
						if(connecteur) k += printf(" %-3s|",CablageEncodeConnecteur(connecteur)); else k += printf(" -- |");
						prec_conn = connecteur;
					} else k += printf("    |");
					if(bb >= 0) strcpy(nomBN,Numeriseur[bb].nom); else strcpy(nomBN,"inconnu");
					k += printf("%-13s|",nomBN);
					prec_num = bb;
					if(Numeriseur[bb].def.adrs && (Numeriseur[bb].def.adrs != 0xFF)) 
						k += printf(" %02X |",Numeriseur[bb].def.adrs); else k += printf(" -- |");
				} else k += printf("    |             |    |");

				if(regl->ress >= 0) {
					ress = &(ModeleBN[Numeriseur[bb].def.type].ress[regl->ress]);
					k += printf(" %-13s|\n",ress->nom);
				} else printf(" --           |\n");
				if(regl->ressneg >= 0) {
					ress = &(ModeleBN[Numeriseur[bb].def.type].ress[regl->ressneg]);
					k += printf("|              |     |           |              |    |           |    |-%-13s|\n",ress->nom);
				}
				if(regl->resspos >= 0) {
					ress = &(ModeleBN[Numeriseur[bb].def.type].ress[regl->resspos]);
					k += printf("|              |     |           |              |    |           |    |+%-13s|\n",ress->nom);
				}
			}
			if(cap == (detectr->nbcapt - 1)) cap = -1;
			else { if(cap == -1) cap = detectr->nbcapt; break; }
		}
	}
	printf("|____________|_____|___________|______________|____|_____________|____|______________|\n");
}
/* ========================================================================== */
int DetecteursDump(Menu menu, MenuItem *item) {
	int bolo; char premier;
	
	premier = 1;
	for(bolo=0; bolo<BoloNb; bolo++) if(Bolo[bolo].local) /* ou si "tous" */ {
		DetecteurDumpUnSeul(bolo,premier,0);
		premier = 0;
	}
	if(premier) printf("* PAS DE DETECTEUR LU PAR CET ORDINATEUR\n");
	else printf("\n");
	return(0);
}
/* ========================================================================== */
static void DetecteurParmsStandardise(DETEC_PARMS_NATURE nature, int voie, char actn) {
	int i,bolo,cap; char *nom;
	
	cap = VoieManip[voie].cap;
	for(i=0; i<DetecteurParmsNb; i++) {
		bolo = DetecteurParmsListe[i];
		if((nom = DetecteurCanalStandardise(nature,bolo,cap,actn)))
			printf("  > %s %s du detecteur %s remis au standard\n",DetecteurParmsTitre[nature],nom,Bolo[bolo].nom);
	}
	if(DetecteurParmsRassembles) DetecteurParmsOngletVoie(cap);
	else DetecteurParmsOngletModule(cap,nature);
}
/* ========================================================================== */
static int DetecteurParmsStandardiseEvts(Panel panel, void *val) {
	DetecteurParmsStandardise(DETEC_PARM_EVTS,(int)val,VoieManip[(int)val].def.evt.std);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsStandardiseTvol(Panel panel, void *val) {
	DetecteurParmsStandardise(DETEC_PARM_TVOL,(int)val,VoieManip[(int)val].def.trmt[TRMT_AU_VOL].std);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsStandardisePrep(Panel panel, void *val) {
	DetecteurParmsStandardise(DETEC_PARM_PREP,(int)val,VoieManip[(int)val].def.trmt[TRMT_PRETRG].std);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsStandardiseTrgr(Panel panel, void *val) {
	DetecteurParmsStandardise(DETEC_PARM_TRGR,(int)val,VoieManip[(int)val].def.trgr.std);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsStandardiseRgul(Panel panel, void *val) {
	DetecteurParmsStandardise(DETEC_PARM_RGUL,(int)val,VoieManip[(int)val].def.rgul.std);
	return(0);
}
/* ========================================================================== */
static void DetecteurParmsApplique(DETEC_PARMS_NATURE nature, int cap) {
	int i,bolo,voie; char *nom;
	
	for(i=0; i<DetecteurParmsNb; i++) {
		bolo = DetecteurParmsListe[i];
		nom = 0;
		if((voie = Bolo[bolo].captr[cap].voie) < 0) continue;
		DetecteurCanalImporte(-1,-1,DETEC_REEL,nature,voie);
		if(VoieManip[voie].pseudo) nom = Bolo[bolo].captr[cap].nom;
		else nom = VoieManip[voie].def.evt.nom;
		printf("  > %s %s du detecteur %s modifie\n",DetecteurParmsTitre[nature],nom,Bolo[bolo].nom);
	}
}
/* ========================================================================== */
static int DetecteurParmsAppliqueEvts(Panel p, void *val) {
	DetecteurParmsApplique(DETEC_PARM_EVTS,(int)val);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsAppliqueTvol(Panel p, void *val) {
	DetecteurParmsApplique(DETEC_PARM_TVOL,(int)val);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsAppliquePrep(Panel p, void *val) {
	DetecteurParmsApplique(DETEC_PARM_PREP,(int)val);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsAppliqueTrgr(Panel p, void *val) {
	DetecteurParmsApplique(DETEC_PARM_TRGR,(int)val);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsAppliqueRgul(Panel p, void *val) {
	DetecteurParmsApplique(DETEC_PARM_RGUL,(int)val);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsAppliqueSoft(Menu menu, MenuItem *item) {
	int i,j,k,bolo,cap,voie,vm,nb_erreurs; TypeDetecteur *detectr;
	TypeComposantePseudo *pseudo;
	int liste[DETEC_CAPT_MAX]; int nbsoft;
	char conserve[DETEC_CAPT_MAX];

	voie = -1;
	for(i=0; i<DetecteurParmsNb; i++) {
		bolo = DetecteurParmsListe[i];
		detectr = &(Bolo[bolo]);
		nbsoft = 0;
		for(cap=0; cap<Bolo[bolo].nbcapt; cap++) {
			voie = Bolo[bolo].captr[cap].voie;
			if((voie >= 0) && VoieManip[voie].pseudo) { liste[nbsoft] = cap; conserve[nbsoft] = 0; nbsoft++; }
		}
		//? DetecteurCanalImporte(-1,-1,DETEC_REEL,DETEC_PARM_SOFT,bolo);
		if(voie >= 0) for(j=0; j<PseudoNb; j++) {
			//- printf("  . Composition de %s: [%s]\n",Pseudo[j].nom,Pseudo[j].composition);
			pseudo = DetecteurCompositionDecode(1,detectr,Pseudo[j].composition,&vm,&nb_erreurs);
			DetecteurAjoutePseudo(detectr,pseudo,Pseudo[j].nom,vm,&nb_erreurs);
			strcpy(Pseudo[j].composition,DetecteurCompositionEncode(pseudo)); // le decodage a laisse des \0 partout
			for(k=0; k<nbsoft; k++) if(!strcmp(Bolo[bolo].captr[liste[k]].nom,Pseudo[j].nom)) {
				printf("    . Voie logicielle %s conservee (%s)\n",Bolo[bolo].captr[liste[k]].nom,
					DetecteurCompositionEncode(VoieManip[Bolo[bolo].captr[liste[k]].voie].pseudo));
				conserve[k] = 1;
				break;
			}
			if(k >= nbsoft) printf("    . Voie logicielle %s ajoutee (%s)\n",Pseudo[j].nom,
				DetecteurCompositionEncode(pseudo));
		}
		for(k=0; k<nbsoft; k++) if(!conserve[k]) {
			cap = liste[k];
			printf("    . Voie logicielle %s supprimee\n",Bolo[bolo].captr[cap].nom);
			DetecteurSimplifieVoie(Bolo[bolo].captr[cap].voie);
			DetecteurDeconnecteVoie(detectr,voie);
		}
		printf("  > %s du detecteur %s modifiees\n",DetecteurParmsTitre[DETEC_PARM_SOFT],Bolo[bolo].nom);
		MenuItemAllume(mDetecteurParmsSoftApply,1,"Appliquer",GRF_RGB_GREEN);
	}
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsAppliqueGenl(Panel p, void *val) {
	int i,bolo;
	
	for(i=0; i<DetecteurParmsNb; i++) {
		bolo = DetecteurParmsListe[i];
		DetecteurCanalImporte(-1,-1,DETEC_REEL,DETEC_PARM_GENL,bolo);
		printf("  > %s du detecteur %s modifies\n",DetecteurParmsTitre[DETEC_PARM_GENL],Bolo[bolo].nom);
	}
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsAppliqueEnvr(Menu menu, MenuItem *item) {
	int i,bolo;
	
	for(i=0; i<DetecteurParmsNb; i++) {
		bolo = DetecteurParmsListe[i];
		DetecteurCanalImporte(-1,-1,DETEC_REEL,DETEC_PARM_ENVR,bolo);
		DetecteursVoisinage(1);
		printf("  > %s du detecteur %s modifie\n",DetecteurParmsTitre[DETEC_PARM_ENVR],Bolo[bolo].nom);
		MenuItemAllume(mDetecteurParmsGenlApply,1,"Appliquer",GRF_RGB_GREEN);
	}
	return(0);
}
/* ========================================================================== */
static void DetecteurParmsAjoute(Cadre planche, DETEC_PARMS_NATURE nature, int voie, char *std, ArgDesc *desc) {
	Panel p; int cap,i,y; Onglet onglet;

	cap = VoieManip[voie].cap;
	onglet = DetecteurParmsFeuilles[nature] = OngletDefini(CapteurNom,DetecteurParmsCallback[nature].ref_onglet);
	OngletMaxCol(onglet,6);
	OngletDemande(onglet,cap);
	BoardAddOnglet(planche,onglet,0);
	y = (OngletRangees(onglet) * (Dy + 2)) + (2 * Dy);
	if(BoloNb > 1) {
		p = PanelCreate(1); PanelSupport(p,WND_CREUX);
		i = PanelKeyB(p,"Definition",L_("particuliere/standard"),std,13);
		PanelOnApply(p,DetecteurParmsCallback[nature].standardise,(void *)(long)voie);
		BoardAddPanel(planche,p,Dx,y,0);
		y += (3 * Dy);
	}
	p = PanelDesc(desc,DetecteurParmsExplics); PanelSupport(p,WND_CREUX);
	if(*std) PanelMode(p,PNL_DIRECT|PNL_RDONLY);
	PanelOnApply(p,DetecteurParmsCallback[nature].applique,(void *)(long)cap);
	BoardAddPanel(planche,p,Dx,y,0);
}
/* ========================================================================== */
void DetecteurParmsConstruitVoie(Cadre planche, int voie, int *x0, int *y0, int cap) {
	int x,y,xm,ym,x1,xm1,xm2; int nature; char *ptr_status;
	Panel p;
	
	x1 = *x0; y = *y0; xm = 0; ym = 0; xm1 = 0;
	for(nature=0; nature<DETEC_PARM_RGUL+1; nature++) {
		x = x1;
		BoardAreaOpen(DetecteurParmsTitre[nature],WND_RAINURES);
		ptr_status = DetecteurCanalExporte(-1,DETEC_REEL,nature,voie);
		if(BoloNb > 1) {
			p = PanelCreate(1); PanelSupport(p,WND_CREUX);
			PanelKeyB(p,"Definition",L_("particuliere/standard"),ptr_status,13);
			PanelOnApply(p,DetecteurParmsCallback[nature].standardise,(void *)(long)voie);
			BoardAddPanel(planche,p,x,y,0);
			y += (3 * Dy);
		}
		p = PanelDesc(CaptDesc[nature],DetecteurParmsExplics); PanelSupport(p,WND_CREUX);
		if(*ptr_status == ParmAJeter) {
			int i,n;
			PanelMode(p,PNL_DIRECT|PNL_RDONLY);
			n = PanelItemNb(p);
			for(i=0; i<n; ) PanelItemSelect(p,++i,0);
		}
		PanelOnApply(p,DetecteurParmsCallback[nature].applique,(void *)(long)cap);
		BoardAddPanel(planche,p,x,y,0);
		if(*ptr_status == ParmAJeter) BoardAreaMargesVert(Dy/2,Dy/2); else BoardAreaMargesVert(Dy/2,3*Dy/2);
		BoardAreaClose(planche,&(DetecteurParmsCadre[nature]),&x,&y);
		DetecteurParmsBordDroit[nature] = x;
		y += (3 * Dy);
		if(xm < x) xm = x; if(ym < y) ym = y;
		if(nature == DETEC_PARM_PREP) { xm1 = xm; x1 = xm + (3 * Dx); y = *y0; }
	}
	xm2 = xm;
	for(nature=0; nature<DETEC_PARM_RGUL+1; nature++) {
		xm = (nature <= DETEC_PARM_PREP)? xm1: xm2;
		BoardAreaDeplaceBas(&(DetecteurParmsCadre[nature]),xm - DetecteurParmsBordDroit[nature],0);
	}
	*x0 = xm; *y0 = ym;
}
/* ========================================================================== */
static void DetecteurParmsOngletVoie(int cap) {
	int bolo,voie,nature; Cadre planche;
	int x0,y0; Onglet onglet;

	DetecteurParmsRassembles = 1;
	nature = DETEC_PARM_NBNATURES;
	if((planche = bDetecteurParms[nature])) {
		if(OpiumDisplayed(planche)) OpiumClear(planche);
		BoardTrash(planche);
	} else {
		bDetecteurParms[nature] = planche = BoardCreate();
		if(!planche) return;
	}
	OpiumTitle(planche,DetecteurParmsTitre[nature]);
	onglet = DetecteurParmsFeuilles[nature] = OngletDefini(CapteurNom,DetecteurParmsOngletVoie);
	// OngletMaxCol(onglet,6);
	OngletDemande(onglet,cap);
	BoardAddOnglet(planche,onglet,0);
	
	bolo = DetecteurParmsListe[DetecteurParmsExemple];
	voie = Bolo[bolo].captr[cap].voie;
	x0 = 2 * Dx; y0 = (OngletRangees(onglet) * (Dy + 2)) + (2 * Dy);
	DetecteurParmsConstruitVoie(planche,voie,&x0,&y0,cap);
	
	OpiumFork(planche);
}
/* ========================================================================== */
static void DetecteurParmsOngletModule(int cap, DETEC_PARMS_NATURE nature) {
	int bolo,voie; Cadre planche;
	char *ptr_status;
	
	DetecteurParmsRassembles = 0;
	if((planche = bDetecteurParms[nature])) {
		if(OpiumDisplayed(planche)) OpiumClear(planche);
		BoardTrash(planche);
	} else {
		bDetecteurParms[nature] = planche = BoardCreate();
		if(!planche) return;
	}
	OpiumTitle(planche,DetecteurParmsTitre[nature]);
	
	bolo = DetecteurParmsListe[DetecteurParmsExemple];
	voie = Bolo[bolo].captr[cap].voie;
	
	ptr_status = DetecteurCanalExporte(-1,DETEC_REEL,nature,voie);
	if(DetecDebugStd) printf("(%s) %s %s %sstandard [@%08X]\n",__func__,DetecteurParmsTitre[nature],
		   VoieManip[voie].nom,(*ptr_status == ParmAJeter)?"au  ":"non-",(hexa)ptr_status);
	if(nature <= DETEC_PARM_RGUL) 
		DetecteurParmsAjoute(planche,nature,voie,ptr_status,CaptDesc[nature]);
		
	else if(nature == DETEC_PARM_SOFT) {
	}
	
	// OpiumExec(planche);
	OpiumFork(planche);
}
/* ========================================================================== */
static void DetecteurParmsOngletEvts(int cap) {
	DetecteurParmsOngletModule(cap,DETEC_PARM_EVTS);
}
/* ========================================================================== */
static void DetecteurParmsOngletTvol(int cap) {
	DetecteurParmsOngletModule(cap,DETEC_PARM_TVOL);
}
/* ========================================================================== */
static void DetecteurParmsOngletPrep(int cap) {
	DetecteurParmsOngletModule(cap,DETEC_PARM_PREP);
}
/* ========================================================================== */
static void DetecteurParmsOngletTrgr(int cap) {
	DetecteurParmsOngletModule(cap,DETEC_PARM_TRGR);
}
/* ========================================================================== */
static void DetecteurParmsOngletRgul(int cap) {
	DetecteurParmsOngletModule(cap,DETEC_PARM_RGUL);
}
/* ========================================================================== */
static void DetecteurParmsOngletSoft(int cap) {
	DetecteurParmsOngletModule(cap,DETEC_PARM_SOFT);
}
/* ========================================================================== */
static int DetecteurParmsVsn(Menu menu, MenuItem *item) {
	int bolo; char titre[80];
	
	bolo = DetecteurParmsListe[DetecteurParmsExemple];
	sprintf(titre,"Detecteurs voisins de %s",Bolo[bolo].nom);
	if(OpiumTableExec(titre,ARG_TYPE_TEXTE,
		(void *)(&(DetecteurLu.nomvoisins[0][0])),&(DetecteurLu.nbvoisins),1,DetecParmsVsnDesc) == PNL_OK) {
		MenuItemAllume(mDetecteurParmsGenlApply,1,"Appliquer",GRF_RGB_YELLOW);
	}
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsAsc(Menu menu, MenuItem *item) {
	int bolo; char titre[80];
	
	bolo = DetecteurParmsListe[DetecteurParmsExemple];
	sprintf(titre,"Detecteurs associes a %s",Bolo[bolo].nom);
	if(OpiumTableExec(titre,ARG_TYPE_TEXTE,
		(void *)(&(DetecteurLu.nom_assoc[0][0])),&(DetecteurLu.nbassoc),1,DetecParmsAscDesc) == PNL_OK) {
		MenuItemAllume(mDetecteurParmsGenlApply,1,"Appliquer",GRF_RGB_YELLOW);
	}
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsSoft(Menu menu, MenuItem *item) {
	int bolo; char titre[80];
	
	bolo = DetecteurParmsListe[DetecteurParmsExemple];
	sprintf(titre,"Voies logicielles pour %s",Bolo[bolo].nom);
	if(OpiumTableExec(titre,ARG_TYPE_STRUCT,(void *)(&DetecPseudoAS),&PseudoNb,1,Detec1PseudoDesc) == PNL_OK) {
		MenuItemAllume(mDetecteurParmsSoftApply,1,"Appliquer",GRF_RGB_YELLOW);
		if(!menu) return(1);
	}
	return(0);
}
/* ========================================================================== */
static MenuItem iDetecParmsGenl[] = {
	{ "Voisins",      MNU_FONCTION DetecteurParmsVsn },
	{ "Associes",     MNU_FONCTION DetecteurParmsAsc },
	MNU_END
};
/* ========================================================================== */
static int DetecteurParmsGenl(Menu menu, MenuItem *item) {
	int bolo,nature; Cadre planche;
	Panel p; Cadre cdr; int x0,y0,x,y,xm,ym;
	
	nature = DETEC_PARM_GENL;
	if((planche = bDetecteurParms[nature])) {
		if(OpiumDisplayed(planche)) OpiumClear(planche);
		BoardTrash(planche);
	} else {
		bDetecteurParms[nature] = planche = BoardCreate();
		if(!planche) return(0);
	}
	OpiumTitle(planche,DetecteurParmsTitre[nature]);
	x0 = Dx; y0 = Dy;
	x = x0; y = y0;
	
	bolo = DetecteurParmsListe[DetecteurParmsExemple];
	DetecteurCanalExporte(-1,DETEC_REEL,DETEC_PARM_GENL,bolo);
	p = PanelDesc(DetecParmsStdDesc,DetecteurParmsExplics); PanelSupport(p,WND_CREUX);
	PanelOnApply(p,DetecteurParmsAppliqueGenl,(void *)(long)bolo);
	BoardAddPanel(planche,p,x,y,0);
	y += ((PanelItemNb(p) + 4) * Dy);

	DetecteurCanalExporte(-1,DETEC_REEL,DETEC_PARM_SOFT,bolo);
	BoardAreaOpen("Ajouts",WND_RAINURES);
	BoardAddBouton(planche,"Voies logicielles",DetecteurParmsSoft,x+(2*Dx),y,0);
	cdr = BoardAddBouton(planche,"Appliquer",DetecteurParmsAppliqueSoft,x+(6*Dx),y+(2*Dy),0);
	mDetecteurParmsSoftApply = (Menu)(cdr->adrs);
	BoardAreaMargesHori(Dx/2,0); BoardAreaMargesVert(Dy/2,0);
	BoardAreaClose(planche,&DetecteurParmsCadreSoft,&xm,&ym);
	x += (25 * Dx);

	if(DetecteurParmsNb == 1) {
		DetecteurCanalExporte(-1,DETEC_REEL,DETEC_PARM_ENVR,bolo);
		mDetecParmsGenl = MenuLocalise(iDetecParmsGenl); MenuColumns(mDetecParmsGenl,MenuItemNb(mDetecParmsGenl));
		OpiumSupport(mDetecParmsGenl->cdr,WND_PLAQUETTE);
		BoardAreaOpen("Environnement",WND_RAINURES);
		BoardAddMenu(planche,mDetecParmsGenl,x,y,0);
		cdr = BoardAddBouton(planche,"Appliquer",DetecteurParmsAppliqueEnvr,x+(4*Dx),y+(2*Dy),0);
		mDetecteurParmsGenlApply = (Menu)(cdr->adrs);
		BoardAreaMargesHori(Dx/2,0); BoardAreaMargesVert(Dy/2,0);
		BoardAreaClose(planche,&DetecteurParmsCadreEnvr,&xm,&ym);
	}
		
	OpiumFork(planche);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsVoie(Menu menu, MenuItem *item) {
	DetecteurParmsOngletVoie(0);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsEvts(Menu menu, MenuItem *item) {
	DetecteurParmsOngletModule(0,DETEC_PARM_EVTS);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsTvol(Menu menu, MenuItem *item) {
	DetecteurParmsOngletModule(0,DETEC_PARM_TVOL);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsPrep(Menu menu, MenuItem *item) {
	DetecteurParmsOngletModule(0,DETEC_PARM_PREP);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsTrgr(Menu menu, MenuItem *item) {
	DetecteurParmsOngletModule(0,DETEC_PARM_TRGR);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsRgul(Menu menu, MenuItem *item) {
	DetecteurParmsOngletModule(0,DETEC_PARM_RGUL);
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsSoftDirect(Menu menu, MenuItem *item) {
	if(DetecteurParmsSoft(0,0)) {
		if(OpiumCheck(L_("Appliquer les modification"))) {
			DetecteurParmsAppliqueSoft(0,0);
		}
	}
	return(0);
}
/* ========================================================================== */
static int DetecteurParmsOptn(Menu menu, MenuItem *item) {
	int i,k,bolo,cap; Panel p; char **noms;

	noms = 0;
	if(DetecteurParmsNb > 1) {
		noms = (char **)malloc((DetecteurParmsNb + 1) * sizeof(char *));
		if(noms) {
			for(i=0; i<DetecteurParmsNb; i++) noms[i] = Bolo[DetecteurParmsListe[i]].nom;
            noms[i] ="\0";
		}
		k = DetecteurParmsExemple;
		p = PanelCreate(2);
		PanelList(p,L_("Detecteur modele"),noms,&DetecteurParmsExemple,DETEC_NOM);
	}
	if(noms == 0) p = PanelCreate(1);
	PanelBool(p,L_("Avec explications"),&DetecteurParmsExplics);
	OpiumExec(p->cdr);
	PanelDelete(p);
	if(noms) {
		free(noms);
		if(DetecteurParmsExemple != k) {
			bolo = DetecteurParmsListe[DetecteurParmsExemple];
			for(cap=0; cap<Bolo[bolo].nbcapt; cap++) CapteurNom[cap] = Bolo[bolo].captr[cap].nom;
			CapteurNom[cap] = "\0";
		}
	}
	
	return(0);
}
/* ========================================================================== */
int DetecteurParmsSave(Menu menu, MenuItem *item) {
	int bolo,i;

	for(i=0; i<DetecteurParmsNb; i++) {
		bolo = DetecteurParmsListe[i];
		DetecteurEcrit(0,&(Bolo[bolo]),bolo,1,DETEC_REGLAGES);
	}
	
	return(0);
}
/* ========================================================================== */
int DetecteurParmsEntree(Menu menu, MenuItem *item);
static MenuItem iDetecParms[] = {
	{ "Niveau detecteur",     MNU_FONCTION DetecteurParmsGenl },
	{ "Niveau voies",         MNU_FONCTION DetecteurParmsVoie },
	{ "Niveau module",        MNU_SEPARATION },
	{ "Evenements",           MNU_FONCTION DetecteurParmsEvts },
	{ "Traitements initiaux", MNU_FONCTION DetecteurParmsTvol },
	{ "Preparation trigger",  MNU_FONCTION DetecteurParmsPrep },
	{ "Triggers",             MNU_FONCTION DetecteurParmsTrgr },
	{ "Regulation du taux",   MNU_FONCTION DetecteurParmsRgul },
	{ "Voies logicielles",    MNU_FONCTION DetecteurParmsSoftDirect },
	{ "\0",                   MNU_SEPARATION },
	{ "Options",              MNU_FONCTION DetecteurParmsOptn },
	{ "Sauver",               MNU_FONCTION DetecteurParmsSave },
	{ "Autre selection",      MNU_FONCTION DetecteurParmsEntree },
	{ "\0",                   MNU_SEPARATION },
	{ "Quitter",              MNU_RETOUR },
	MNU_END
};
/* ========================================================================== */
char DetecteurSelectionne() {
	int bolo;

	if(!BoloNb) { OpiumError("Pas de detecteur defini"); return(0); }
	else if(BoloNb > 1) {
		OrgaSelectionDetecteurConstruit(1);
		OrgaDetecteursChoisis = 0;
		OpiumFork(bDetecteurSelecte);
		until(OrgaDetecteursChoisis) { OpiumUserAction(); usleep(100000); }
		if(OrgaDetecteursChoisis < 0) return(0);
	} else Bolo[0].sel = 1;
	
	DetecteurParmsNb = 0;
	for(bolo=0; bolo<BoloNb; bolo++) if(Bolo[bolo].sel) DetecteurParmsListe[DetecteurParmsNb++] = bolo;
	if(!DetecteurParmsNb) { OpiumError("Aucun detecteur selectionne"); return(0); }
	return(1);
}
/* ========================================================================== */
int DetecteurParmsEntree(Menu menu, MenuItem *item) {
	int bolo,cap;

	if(!DetecteurSelectionne()) return(0);
	bolo = DetecteurParmsListe[0];
	for(cap=0; cap<Bolo[bolo].nbcapt; cap++) CapteurNom[cap] = Bolo[bolo].captr[cap].nom;
	/* for(cap=0; cap<Bolo[bolo].nbcapt; cap++) {
		int voie;
		voie = Bolo[bolo].captr[cap].voie;
		if(VoieManip[voie].pseudo) {
			printf("%s = %s\n",VoieManip[voie].nom,DetecteurCompositionEncode(VoieManip[voie].pseudo));
		}
	} */
	CapteurNom[cap] = "\0";
	OpiumFork(mDetecParms->cdr);

	return(0);
}
/* ========================================================================== */
static void DetecteurEncodeSource(TypeDetecteur *detectr, char *nom) {
	int cap; char num_adc[8];

	if(detectr->pos == CABLAGE_INDEFINI) {
		if(detectr->captr[0].bb.adrs != 0xFF) sprintf(nom,"#%02X",detectr->captr[0].bb.adrs);
		else strcpy(nom,"---");
	} else if(detectr->pos == CABLAGE_POS_DIR) {
		sprintf(nom,"<@%s",Numeriseur[detectr->captr[0].bb.num].fichier);
		for(cap=0; cap<detectr->nbcapt; cap++) if((detectr->captr[cap].bb.adc - 1) != cap) break;
		if(cap < detectr->nbcapt) for(cap=0; cap<detectr->nbcapt; cap++) {
			sprintf(num_adc,".%d",detectr->captr[cap].bb.adc); strcat(nom,num_adc);
		}
	} else strcpy(nom,CablageEncodePosition(detectr->pos));
}
/* ========================================================================== */
void DetecteurEcrit(FILE *global, TypeDetecteur *detectr, int bolo, char avec_reglages, char quoi) {
	int num,cmde; char *nom; TypeReglage *regl; char *ptr_status;
	int i,j,k,l; int cap,vm,voie,num_config; char ancien_format,bloc_non_vide;
	TypeConfigDefinition *config;
	FILE *fichier; char nom_complet[MAXFILENAME],abrege[MAXFILENAME]; char source[32],etat[8];

	if(bolo < 0) return;
//	if(detectr->a_sauver) printf(". Le detecteur %s sera sauve dans %s\n",
//		detectr->nom,(detectr->fichier[0])? detectr->fichier: (global? FichierPrefDetecteurs: "rien"));
//	else printf(". Le detecteur %s n'a PAS ete modifie\n",detectr->nom);
	if((quoi == DETEC_REGLAGES) && !(detectr->a_sauver)) return;
	DetecteurEncodeSource(detectr,source);
//	ancien_format = (detectr->hote[0]);
	ancien_format = 0;
	if((quoi != DETEC_REGLAGES) && global && detectr->fichier[0]) {
		if(ancien_format) fprintf(global,"@%s = %s\n",detectr->hote,detectr->fichier);
		else if(bolo == BoloStandard) fprintf(global,"std %s > @%s\n",detectr->hote,detectr->fichier);
		else if(detectr->pos == CABLAGE_INDEFINI) {
			if(quoi != DETEC_LISTE) fprintf(global,"\"%s\" > @%s\n",source,detectr->fichier);
		} else fprintf(global,"%s > @%s\n",source,detectr->fichier);
	}
	if(quoi == DETEC_LISTE) return;
	if((bolo == BoloStandard) && strcmp(detectr->hote,Acquis[AcquisLocale].code)) return;
	if(detectr->fichier[0]) {
		if(detectr->fichier[0] == SLASH) strcpy(nom_complet,detectr->fichier);
		else strcat(strcpy(nom_complet,DetecteurRacine),detectr->fichier);
		RepertoireCreeRacine(nom_complet);
		if(!(fichier = fopen(nom_complet,"w"))) {
			OpiumError("Sauvegarde du detecteur %s impossible",detectr->nom);
			printf("! Impossible de sauver le detecteur %s sur:\n    %s\n    (%s)\n",
				detectr->nom,nom_complet,strerror(errno));
			return;
		}
	} else {
		if(!global) {
			OpiumError("Sauvegarde du detecteur %s impossible",detectr->nom);
			printf("! Impossible de sauver le detecteur %s (global=0 dans %s)\n",
				detectr->nom,__func__);
			return;
		}
		fichier = global;
	}
	fprintf(fichier,"#\n");
	if(ancien_format) fprintf(fichier,"=%s",detectr->nom);
	else fprintf(fichier,"%s",detectr->nom);
	/* if(detectr->type) */ if(bolo != BoloStandard) fprintf(fichier,"/%s",ModeleDet[detectr->type].nom);
	if(detectr->ref >= 0) fprintf(fichier," =%s",Bolo[detectr->ref].nom);
	if(ancien_format) {
		if((detectr->pos == CABLAGE_INDEFINI) || (detectr->pos == CABLAGE_POS_DIR)) fprintf(fichier," %s",source);
		else fprintf(fichier," @%s",source);
	}
	/* if(bolo == BoloRep) { fprintf(fichier,"\n"); goto prgm; } else */
	if(bolo == BoloStandard) /* pas les memes boucles que pour les vrais numeriseurs */ {
		fprintf(fichier,":\n");
		ArgIndent(2);
		for(vm=0; vm<ModeleVoieNb; vm++) {
			fprintf(fichier,"#\n#\t---------- Parametrage de la voie %s ----------\n#\n",ModeleVoie[vm].nom);
			for(i=DETEC_PARM_EVTS; i<=DETEC_PARM_RGUL; i++) {
				DetecteurCanalExporte(-1,DETEC_STD,i,vm);
				fprintf(fichier,"\t%s %s = {\n",DetecteurParmsUser[i],ModeleVoie[vm].nom);
				ArgAppend(fichier,0,CaptDesc[i]);
				fprintf(fichier,"\t}\n");
			}
		}
		ArgIndent(0);
		goto prgm;
	}
/*	Compatible avec ancienne syntaxe mais double info avec DetecParmsDesc
	if(detectr->masse > 0.0) { fprintf(fichier," %gg",detectr->masse); if(detectr->hote[0]) fprintf(fichier," /%s",detectr->hote); } */
	ArgKeyGetText(DETEC_STATUTS,detectr->lu,etat,8);
	fprintf(fichier,": %s ("DETEC_STATUTS")\n",etat);
	detectr->a_sauver = 0;
	DetecteurCanalExporte(-1,DETEC_REEL,DETEC_PARM_GENL,detectr->numero);
	DetecteurCanalExporte(-1,DETEC_REEL,DETEC_PARM_ENVR,detectr->numero);
	ArgIndent(2);
	fprintf(fichier,"\tparms detecteur = {\n");
	ArgAppend(fichier,0,DetecParmsDesc);
	fprintf(fichier,"\t}\n");

	if(avec_reglages) {
		bloc_non_vide = 0;
		for(num=0; num<detectr->nbreglages; num++) {
			regl = &(detectr->reglage[num]);
			if(regl->capt == -1) { bloc_non_vide = 1; break; }
		}
		if(bloc_non_vide) {
			fprintf(fichier,"\treglages detecteur = {\n");
			for(num=0; num<detectr->nbreglages; num++) {
				regl = &(detectr->reglage[num]);
				if((regl->capt == -1) && regl->std[0]) fprintf(fichier,"\t\t%s = %s\n",ModeleDet[detectr->type].regl[num].nom,regl->std);
			}
			fprintf(fichier,"\t}\n");
		}
	}
	for(cap=0; cap<detectr->nbcapt; cap++) {
		voie = detectr->captr[cap].voie;
		if(voie < 0) continue;
		if(VoieManip[voie].pseudo) {
			nom = detectr->captr[cap].nom;
			fprintf(fichier,"#\n#\t---------- Parametrage de la voie %s ----------\n#\n",nom);
			fprintf(fichier,"\tpseudo %s = \"%s\"\n",nom,DetecteurCompositionEncode(VoieManip[voie].pseudo));
		} else {
			vm = detectr->captr[cap].modele;
			nom = ModeleVoie[vm].nom;
			fprintf(fichier,"#\n#\t---------- Parametrage de la voie %s ----------\n#\n",nom);
		}
		fprintf(fichier,"\tparms %s = {\n",nom);
		ReglUsage = VoieManip[voie].sauvee;
		ReglSensibilite = VoieManip[voie].sensibilite;
		ArgAppend(fichier,0,CaptAutreDesc);
		fprintf(fichier,"\t}\n");
		if(avec_reglages) {
			bloc_non_vide = 0;
			for(num=0; num<detectr->nbreglages; num++) {
				regl = &(detectr->reglage[num]);
				if(regl->capt == cap) { bloc_non_vide = 1; break; }
			}
			if(bloc_non_vide) {
				fprintf(fichier,"\treglages %s = {\n",nom);
				for(num=0; num<detectr->nbreglages; num++) {
					regl = &(detectr->reglage[num]);
					if((regl->capt == cap) && regl->std[0]) fprintf(fichier,"\t\t%s = %s\n",ModeleDet[detectr->type].regl[num].nom,regl->std);
				}
				fprintf(fichier,"\t}\n");
			}
		}
		SambaNommeTrmt(voie,TRMT_AU_VOL);
		SambaNommeTrmt(voie,TRMT_PRETRG);
		DetecteurSauveConfig(ConfigNum);
		for(i=DETEC_PARM_EVTS; i<=DETEC_PARM_RGUL; i++) {
			for(num_config=0; num_config<VoieManip[voie].config.nb; num_config++) {
				ptr_status = DetecteurCanalExporte(num_config,DETEC_REEL,i,voie);
				if(GestionConfigs) printf("(%s)        %s: config #%d (%s) %s\n",__func__,DetecteurParmsTitre[i],num_config,VoieManip[voie].config.def[num_config].nom,
					(*ptr_status == ParmASauver)? "a sauver": "a jeter");
				if(*ptr_status == ParmASauver) {
					config = &(VoieManip[voie].config.def[num_config]);
					if(!strcmp(config->nom,CONFIG_STD)) fprintf(fichier,"\t%s %s = {\n",DetecteurParmsUser[i],nom);
					else fprintf(fichier,"\t%s config/%s %s = {\n",DetecteurParmsUser[i],config->nom,nom);
					ArgAppend(fichier,0,CaptDesc[i]);
					fprintf(fichier,"\t}\n");
				}
			}
		}
	}
#ifdef FILTRE_PAR_BOLO
	if(detectr->filtre_declare) /* ce test vaut pour toutes les voies! */ {
		fprintf(fichier,"\tfiltres = {\n");
		for(cap=0; cap<detectr->nbcapt; cap++) if(!VoieManip[detectr->captr[cap].voie].pseudo) {
			vm = detectr->captr[cap].modele;
			if((j = detectr->general[vm]) >= 0) {
				fprintf(fichier,"\t\tvoie %s = %s\n",ModeleVoie[vm].nom,FiltreGeneral[j].nom);
			} else if(detectr->filtre[vm].etage[0].nbdir || detectr->filtre[vm].etage[0].nbinv) {
				fprintf(fichier,"\t\tvoie %s =\n",ModeleVoie[vm].nom);
				suite = ' ';
				for(etage=0; etage<detectr->filtre[vm].nbetg; etage++) {
					if((k = detectr->filtre[vm].etage[etage].nbdir)) {
						fprintf(fichier,"\t\t  %c directs=",suite);
						for(j=0; j<k; j++) fprintf(fichier," %g",detectr->filtre[vm].etage[etage].direct[j]);
						fprintf(fichier,"\n");
						suite = ' ';
					}
					if((l = detectr->filtre[vm].etage[etage].nbinv)) {
						fprintf(fichier,"\t\t  %c inverses=",suite);
						for(j=0; j<l; j++) fprintf(fichier," %g",detectr->filtre[vm].etage[etage].inverse[j]);
						fprintf(fichier,"\n");
						suite = ' ';
					}
					suite = '+';
				}
			}
		}
		fprintf(fichier,"\t}\n");
	}
#endif
	ArgIndent(0);

prgm:
	if(detectr->taille) {
		fprintf(fichier,"\tregistres = {\n");
		for(j=0, k=detectr->debut; j<detectr->taille; j++,k++) {
			cmde = (Commandes[k] >> 16) & 0xFF;
			/* Regarder les ressources numeriseur?
			int l,valeur;
			for(l=0; l<CMDE_NBTYPES; l++) if(cmde == CmdeDef[l].ss_adrs) break;
			if(l < CMDE_NBTYPES) {
				valeur = Commandes[k] & 0xFFFF;
				if(l < RESS_MODUL) fprintf(fichier,"\t\t%s=%d\n",CmdeDef[l].nom,valeur);
				else fprintf(fichier,"\t\t%s=x%04X\n",CmdeDef[l].nom,valeur);
			} else */
				fprintf(fichier,"\t\tx%06X\n",Commandes[k]);
		}
		fprintf(fichier,"\t}\n");
	/* OBSOLETE
	} else if(bolo == BoloRep) {
		fprintf(fichier,"\tregistres = {\n");
		fprintf(fichier,"\t\t%s=%d\n",CmdeDef[RESS_D2].nom,detectr->d2);
		fprintf(fichier,"\t\t%s=%d\n",CmdeDef[RESS_D3].nom,detectr->d3);
		fprintf(fichier,"\t}\n"); */
	}
	detectr->a_sauver = 0;
	if(detectr->fichier[0]) {
		fclose(fichier);
		l = strlen(DetecteurRacine);
		if(!strncmp(nom_complet,DetecteurRacine,l)) sprintf(abrege,"<det>/%s",nom_complet+l);
		else strcpy(abrege,nom_complet);
		printf("* Detecteur %s sauve sur %s\n",detectr->nom,abrege);
		SambaNote("Detecteur %s sauve sur %s\n",detectr->nom,nom_complet);
	}
}
/* ========================================================================== */
int DetecteurEcritTous(char quoi, int qui) {
	/* quoi:  DETEC_LISTE    (0) : liste seulement
	          DETEC_REGLAGES (1) : reglages
	          DETEC_TOUT     (2) : les deux..
	   qui:   (-1): tous
	        <bolo>: un detecteur particulier */
	int bolo; int sat,l; char *nom;
	FILE *fichier;

	if((quoi == DETEC_REGLAGES) && ReglageDetecTousIndiv) fichier = 0;
	else {
		RepertoireCreeRacine(FichierPrefDetecteurs);
		if(!(fichier = fopen(FichierPrefDetecteurs,"w"))) {
			OpiumError("Sauvegarde des detecteurs impossible");
			printf("! Impossible de sauver les detecteurs sur:\n    %s\n    (%s)\n",
				FichierPrefDetecteurs,strerror(errno));
			return(0);
		}
		nom = rindex(FichierPrefDetecteurs,SLASH); l = nom - FichierPrefDetecteurs + 1;
		if(DetecteurFichierModeles[0]) {
			if(!strncmp(FichierPrefDetecteurs,DetecteurFichierModeles,l)) 
				fprintf(fichier,"modeles @%s\n#\n",DetecteurFichierModeles+l);
			else fprintf(fichier,"modeles @%s\n#\n",DetecteurFichierModeles);
		}
		if(strcmp(DetecteurStock,"./")) {
			if(!strncmp(FichierPrefDetecteurs,DetecteurStock,l)) 
				fprintf(fichier,"stock @%s\n#\n",DetecteurStock+l);
			else fprintf(fichier,"stock @%s\n#\n",DetecteurStock);
		}
	}
	if(qui < 0) {
		// if(fichier) { if(BoloRep >= 0) DetecteurEcrit(fichier,&(Bolo[BoloRep]),BoloRep,0,DETEC_REGLAGES); }
		for(sat=0; sat<DetecStandardNb; sat++) if(DetecteurStandard[sat].a_sauver)
			DetecteurEcrit(fichier,&(DetecteurStandard[sat]),BoloStandard,0,quoi);
		for(bolo=0; bolo<BoloNb; bolo++) if(fichier || Bolo[bolo].local) 
			DetecteurEcrit(fichier,&(Bolo[bolo]),bolo,1,quoi);
	} else DetecteurEcrit(fichier,&(Bolo[qui]),qui,(qui < BoloNb),quoi);
	if(fichier) {
		fclose(fichier);
		printf("* Liste des detecteurs egalement sauvee sur %s\n",FichierPrefDetecteurs);
		SambaNote("Liste des detecteurs egalement sauvee sur %s\n",FichierPrefDetecteurs);
	}

	return(0);
}
/* ========================================================================== */
int DetecteurEcritTout(Menu menu, MenuItem *item) {
	if(!OpiumChoix(2,SambaNonSi,L_("Pas de modification en cours sur une autre acquisition")))
		DetecteurEcritTous(DETEC_TOUT,-1);
	return(0);
}
/* ========================================================================== */
void DetecteurSauveSelectReglages(char avec_standard) {
	int j,bolo,max,nb,sv,rc; char std; char log;
	Panel psel;

	// if(OpiumCheck("Sauver %d detecteur%s",Accord1s(nb))) {
	// 	DetecteurEcritTous(DETEC_REGLAGES,-1); mais pas toujours la peine de sauver le standard, qui n'a pas de consigne
	// }
	if(!ReglageDetecTousIndiv) { DetecteurEcritTous(DETEC_REGLAGES,-1); return; }
	log = 0;
	std = (avec_standard && (BoloStandard >= 0));
	/* Mais on peut ne pas sauver sur fichier */
	if(log) printf("* Detecteurs locaux:");
	nb = 0; sv = 0; max = BoloNb;
	if(std) max++;
	for(j=0; j<max; j++) {
		if(j < BoloNb) bolo = OrdreDetec[j].num; else bolo = BoloStandard;
		if(Bolo[bolo].local) {
			if(log) {
				if(nb) printf(","); if(!(nb % 6)) printf("\n  |");
				printf(" %8s%3s",Bolo[bolo].nom,(Bolo[bolo].a_sauver)? "(*)": " ");
			}
			nb++; if(Bolo[bolo].a_sauver) sv++;
		}
	}
	if(log) {
		printf("\n");
		if(sv) printf("  (*) %d detecteur%s modifie%s\n",Accord2s(sv));
	}
	psel = PanelCreate(nb);
	for(j=0; j<BoloNb; j++) {
		bolo = OrdreDetec[j].num;
		if(Bolo[bolo].local) PanelItemColors(psel,PanelBool(psel,Bolo[bolo].nom,&(Bolo[bolo].a_sauver)),SambaRougeVertJaune,2);
	}
	PanelTitle(psel,"Selection a sauver");
	rc = OpiumExec(psel->cdr);
	PanelDelete(psel);
	if(rc == PNL_CANCEL) return;
	if(std) DetecteurEcrit(0,&(Bolo[BoloStandard]),BoloStandard,1,DETEC_REGLAGES);
	for(j=0; j<BoloNb; j++) {
		bolo = OrdreDetec[j].num;
		if(Bolo[bolo].local) DetecteurEcrit(0,&(Bolo[bolo]),bolo,1,DETEC_REGLAGES);
	}
	// ReglageVerifieStatut();
}
/* ========================================================================== */
int DetecteurSauveTousReglages() { DetecteurSauveSelectReglages(1); return(0); }
/* ========================================================================== */
int DetecteurSauveReglages(int bolo) {
	if((Bolo[bolo].fichier[0] == '\0') && !OpiumCheck("Tout sauver pour sauver %s",Bolo[bolo].nom)) return(0);
	if(Bolo[bolo].fichier[0]) {
		DetecteurEcrit(0,&(Bolo[bolo]),bolo,1,DETEC_REGLAGES);
		return(1);
	} else { DetecteurEcritTout(0,0); return(-1); }
	return(0);
}
/* ========================================================================== */
void DetecteurSauveListe() {
	int j,bolo; int sat,l; char *nom;
	FILE *fichier;

	RepertoireCreeRacine(FichierPrefDetecteurs);
	if(!(fichier = fopen(FichierPrefDetecteurs,"w"))) {
		OpiumFail("Sauvegarde des detecteurs impossible");
		printf("! Impossible de sauver les detecteurs sur:\n    %s\n    (%s)\n",
			   FichierPrefDetecteurs,strerror(errno));
		return;
	}
	nom = rindex(FichierPrefDetecteurs,SLASH); l = nom - FichierPrefDetecteurs + 1;
	if(DetecteurFichierModeles[0]) {
		if(!strncmp(FichierPrefDetecteurs,DetecteurFichierModeles,l)) 
			fprintf(fichier,"modeles @%s\n#\n",DetecteurFichierModeles+l);
		else fprintf(fichier,"modeles @%s\n#\n",DetecteurFichierModeles);
	}
	if(strcmp(DetecteurStock,"./")) {
		if(!strncmp(FichierPrefDetecteurs,DetecteurStock,l)) 
			fprintf(fichier,"stock @%s\n#\n",DetecteurStock+l);
		else fprintf(fichier,"stock @%s\n#\n",DetecteurStock);
	}
	for(sat=0; sat<DetecStandardNb; sat++) if(DetecteurStandard[sat].a_sauver)
		DetecteurEcrit(fichier,&(DetecteurStandard[sat]),BoloStandard,0,DETEC_LISTE);
	pour_tout(j,BoloNb) {
		bolo = OrdreDetec[j].num;
		if(Bolo[bolo].local) DetecteurEcrit(fichier,&(Bolo[bolo]),bolo,1,DETEC_LISTE);
	}
	fclose(fichier);
	printf("* Liste des detecteurs seule sauvee sur %s\n",FichierPrefDetecteurs);
	SambaNote("Liste des detecteurs seule sauvee sur %s\n",FichierPrefDetecteurs);
}
#define AVANT
/* ========================================================================== */
int ReglageNum(int bolo, int voie, int cmde) {
	int i,cap,vregl; TypeDetecteur *detectr;

	detectr = &(Bolo[bolo]);
	for(i=0; i<detectr->nbreglages; i++) {
        cap = detectr->reglage[i].capt;
	#ifdef AVANT
        if(voie < 0) vregl = -1; else if(cap < 0) vregl = -1; else vregl = detectr->captr[cap].voie;
		if((vregl == voie) && (detectr->reglage[i].cmde == cmde)) break;
	#else
		if(voie < 0) vregl = voie; else vregl = VoieManip[voie].num_adc - 1;
		if((cap == vregl) && (detectr->reglage[i].cmde == cmde)) break;
	#endif
	}
	if(i < detectr->nbreglages) return(i); else return(-1);
}
/* ========================================================================== */
TypeReglage *ReglagePtr(int voie, int cmde) {
	int i,bolo,cap; TypeDetecteur *detectr; TypeReglage *regl;

	if((cap = VoieManip[voie].cap) < 0) return(0);
	if((bolo = VoieManip[voie].det) < 0) return(0);
	detectr = &(Bolo[bolo]);
	for(i=0; i<detectr->nbreglages; i++) {
		regl = &(detectr->reglage[i]);
		if((regl->capt == cap) && (regl->cmde == cmde)) return(regl);
	}
	return(0);

/*	int num,bolo,cap,vregl; TypeDetecteur *detectr;
	for(num=0; num<detectr->nbreglages; num++) {
        cap = detectr->reglage[num].capt;
	#ifdef AVANT
        if(voie < 0) vregl = -1; else if(cap < 0) vregl = -1; else vregl = detectr->captr[cap].voie;
		if((vregl == voie) && (detectr->reglage[num].cmde == cmde)) break;
	#else
		if(voie < 0) vregl = voie; else vregl = VoieManip[voie].num_adc - 1;
		if((cap == vregl) && (detectr->reglage[num].cmde == cmde)) break;
	#endif
	}
	if(num < detectr->nbreglages) return(&(detectr->reglage[num])); else return(0); */
}
/* ========================================================================== */
float *DetecteurConsigneAdrs(TypeDetecteur *detectr, int cap, int cmde) {
	int i,bb,num; float *adrs;
	TypeReglage *regl; TypeBNModele *modele_bn; TypeBNModlRessource *ress;

	adrs = 0;
	for(i=0; i<detectr->nbreglages; i++) {
		regl = &(detectr->reglage[i]);
		if((regl->capt == cap) && (regl->cmde == cmde)) break;
	}
	if(i < detectr->nbreglages) {
		if((bb = regl->bb) >= 0) {
			num = regl->ress;
			modele_bn = &(ModeleBN[Numeriseur[bb].def.type]); ress = modele_bn->ress + num;
			if(ress->type == RESS_FLOAT) adrs = &(Numeriseur[bb].ress[num].val.r);
		}
	}
	return(adrs);
}
/* ========================================================================== */
float DetecteurConsigneCourante(int voie, int cmde) {
	int bb,num;
	TypeReglage *regl; TypeBNModele *modele_bn; TypeBNModlRessource *ress;
	float rval;

	rval = 0.0;
	if((regl = ReglagePtr(voie,cmde))) {
		// cap = regl->capt; bb = Bolo[regl->bolo].captr[cap].bb.num;
		if((bb = regl->bb) < 0) return(0.0);
		num = regl->ress;
		modele_bn = &(ModeleBN[Numeriseur[bb].def.type]); ress = modele_bn->ress + num;
		if(ress->type == RESS_FLOAT) rval = Numeriseur[bb].ress[num].val.r;
		else rval = (float)Numeriseur[bb].ress[num].val.i;
	}
	return(rval);

	/*	int i,cap,vregl; float rval;
	 int bb,num;
	 TypeReglage *regl;
	 TypeBNModele *modele_bn; TypeBNModlRessource *ress;

	 for(i=0; i<detectr->nbreglages; i++) {
	 cap = detectr->reglage[i].capt;
	 if(cap < 0) vregl = -1; else vregl = detectr->captr[cap].voie;
		if((vregl == voie) && (detectr->reglage[i].cmde == cmde)) break;
	 }
	 if(i < detectr->nbreglages) {
		regl = &(detectr->reglage[i]);
		cap = regl->capt;
		// bb = Bolo[regl->bolo].captr[cap].bb.num;
		if((bb = regl->bb) < 0) return(0.0);
		num = regl->ress;
		modele_bn = &(ModeleBN[Numeriseur[bb].def.type]); ress = modele_bn->ress + num;
		if(ress->type == RESS_FLOAT) rval = Numeriseur[bb].ress[num].val.r;
		else rval = (float)Numeriseur[bb].ress[num].val.i;
		return(rval);
	 } else return(0.0); */
}
/* ========================================================================== */
float ReglageValeurFloat(TypeReglage *reglage) {
	// int bolo,cap;
	int bb,num;
	TypeBNModele *modele_bn; TypeBNModlRessource *ress;
	
	// bolo = reglage->bolo; cap = reglage->capt; bb = Bolo[bolo].captr[cap].bb.num;
	if((bb = reglage->bb) < 0) return(0.0);
	num = reglage->ress;
	if(num >= 0) {
		modele_bn = &(ModeleBN[Numeriseur[bb].def.type]);
		ress = modele_bn->ress + num;
		if(ress->type == RESS_FLOAT) return(Numeriseur[bb].ress[num].val.r);
		else return((float)Numeriseur[bb].ress[num].val.i);
	}
	return(0.0);
}
/* ========================================================================== */
INLINE void ReglageEffetDeBord(TypeReglage *regl, char *prefixe) {
	int cap,voie,cmde,bb,k; int ival; float rval; float compens;
	TypeDetecteur *detectr; TypeBNModlRessource *ress;

// essayer dladdr();
	// if(DebugRestant) { printf("***** Effets de bord pour %s\n",regl->nom); ImprimePileAppels; --DebugRestant; }
	if(!regl) return;
	k = regl->ress; if(k < 0) return;
	detectr = Bolo + regl->bolo;
	cap = regl->capt; if(cap < 0) cap = 0;
	cmde = regl->cmde;
	voie = detectr->captr[cap].voie;
	// bb = detectr->captr[cap].bb.num;
	bb = regl->bb;
	if(bb < 0) return;
	ress = ModeleBN[Numeriseur[bb].def.type].ress + k;
	if(ress->type == RESS_FLOAT) { rval = Numeriseur[bb].ress[k].val.r; ival = (int)(rval + 0.5); }
	else { ival = Numeriseur[bb].ress[k].val.i; rval = (float)ival; }
	/* if(prefixe) { char type[80];
		ArgKeyGetText(RESS_TYPETEXTE,ress->type,type,80);
		printf("%s     * Numeriseur[%d].ress[%d].val = %s.%s:%d = (%s)%g @%08X pour %s\n",prefixe,bb,k,Numeriseur[bb].nom,ress->nom,ress->type,type,rval,
			   (hexa)(Numeriseur[bb].ress[k].val.i),regl->nom);
	} */
	switch(cmde) {
	  case REGL_D2: 
		if(!ival) break;
		detectr->d2 = ival;
		if(prefixe) {
			printf("%s     > La modulation sur %s est de %6.2f ms\n",prefixe,detectr->nom,(float)detectr->d2 / Echantillonnage);
			printf("%s     > La synchronisation   est de %6.2f ms\n",prefixe,(float)(detectr->d3 * detectr->d2) / Echantillonnage);
		}
		if(voie >= 0) {
			//if(OscilloReglage[OscilloNum]->acq && (voie == OscilloReglage[OscilloNum]->voie[0]) && (OscilloReglage[OscilloNum]->demodule == TRMT_DEMODUL))
			//	VoieTampon[voie].trmt[TRMT_AU_VOL].compactage = detectr->d2;
			VoieTampon[voie].trmt[TRMT_AU_VOL].nbdata = (detectr->d2 / 2) - VoieManip[voie].def.trmt[TRMT_AU_VOL].parm;
		}
		break;
	  case REGL_D3: 
		if(!ival) break;
		detectr->d3 = ival;
		if(prefixe) printf("%s     > La synchronisation sur %s est de %6.2f ms\n",prefixe,detectr->nom,(float)(detectr->d3 * detectr->d2) / Echantillonnage);
		break;
	  case REGL_GAIN: 
		detectr->captr[cap].numgain = ival;
		sscanf(Numeriseur[bb].ress[k].cval,"%g",&(detectr->captr[cap].gain_reel));
		if(voie >= 0) {
			VoieManip[voie].gain_variable = detectr->captr[cap].gain_reel;
			if(prefixe) printf("%s     > Le gain variable de la voie %s est de %g\n",prefixe,VoieManip[voie].nom,VoieManip[voie].gain_variable);
			DetecteurAdcCalib(voie,prefixe);
			// VoieTampon[voie].simu.facteurC = VoieManip[voie].gain_fixe * VoieManip[voie].gain_variable / (COMP_COEF * VoieManip[voie].ADUenmV * 1.0e-3);
			VoieTampon[voie].simu.facteurC = 1.0E9 / VoieManip[voie].ADUennV;
			if(VoieTampon[voie].simu.facteurC < 0.0) VoieTampon[voie].simu.facteurC = -VoieTampon[voie].simu.facteurC;
			if(VoieManip[voie].Rmodul > 0.0) VoieTampon[voie].simu.facteurC *= (VoieManip[voie].Rbolo / VoieManip[voie].Rmodul);
			if(prefixe) printf("%s     > facteurC[%s] = %g\n",prefixe,VoieManip[voie].nom,VoieTampon[voie].simu.facteurC);
		}
		break;
	  case REGL_FET_MASSE: 
		detectr->razfet_en_cours = ival? 1: 0;
		if(prefixe) printf("%s     > Le FET de %s est %s\n",prefixe,detectr->nom,detectr->razfet_en_cours? "a la masse": "libre");
		break;
	  case REGL_RESET: detectr->reset = ival;
		if(prefixe) printf("%s     > %s recoit un reset toutes les %d secondes\n",prefixe,detectr->nom,detectr->reset);
		break;
	  case REGL_FET_CC: 
		detectr->razfet_en_cours = ival? 1: 0;
		if(prefixe) printf("%s     > Le FET de %s est %s\n",prefixe,detectr->nom,detectr->razfet_en_cours? "en court-circuit": "libre");
		break;
	  case REGL_MODUL: 
		if(voie >= 0) {
			if(VoieManip[voie].Rmodul != 0.0) VoieInfo[voie].I = 1000.0 * rval / VoieManip[voie].Rmodul; /* nA */
			else VoieInfo[voie].I = 0.0;
			// BBv2 seulement: if(prefixe) printf("%s     > I[%s] = %g mV/s / %g kO = %g nA\n",VoieManip[voie].nom,rval,VoieManip[voie].Rmodul,VoieInfo[voie].I);
			if(prefixe) printf("%s     > I[%s] = %g nA [Rmodul=%g MO]\n",prefixe,VoieManip[voie].nom,VoieInfo[voie].I,VoieManip[voie].Rmodul);
			ReglIbolo = VoieInfo[voie].I;
			if(VoieInfo[voie].I != 0.0) VoieInfo[voie].R = VoieInfo[voie].V / VoieInfo[voie].I; /* MO */
			else VoieInfo[voie].R = 0.0;
			// VoieTampon[voie].simu.modul = Numeriseur[bb].ress[k].hval * VoieTampon[voie].simu.facteurC; facteurC sans ADUennV
			VoieTampon[voie].simu.modul = rval * VoieTampon[voie].simu.facteurC;
			// printf("(%s) modul[%s] = (rval=%g) * (facteurC=%g) = %d\n",__func__,VoieManip[voie].nom,rval,VoieTampon[voie].simu.facteurC,VoieTampon[voie].simu.modul);
			if(prefixe) printf("%s     > modul[%s] = %d\n",prefixe,VoieManip[voie].nom,VoieTampon[voie].simu.modul);
			SambaAttends(CompensationAttente);
		}
		break;
	  case REGL_COMP: 
		if(voie >= 0) {
			compens = rval;
			if(Acquis[AcquisLocale].etat.active) compens += (COMP_COEF * VoieInfo[voie].mesure_enV);
			VoieInfo[voie].V = 1000.0 * compens / VoieManip[voie].gain_fixe; /* mV */
			if(prefixe) printf("%s     > V[%s] = %g mV\n",prefixe,VoieManip[voie].nom,VoieInfo[voie].V);
			ReglVbolo = VoieInfo[voie].V;
			if(VoieInfo[voie].I != 0.0) VoieInfo[voie].R = VoieInfo[voie].V / VoieInfo[voie].I; /* MO */
			else VoieInfo[voie].R = 0.0;
			// VoieTampon[voie].simu.compens = Numeriseur[bb].ress[k].hval;
			// printf("(%s) compens = (rval=%g) / (ADUenmV=%g V) = %g\n",__func__,rval,VoieManip[voie].ADUenmV/1.0E3,rval * 1.0E3 / VoieManip[voie].ADUenmV);
			ival = rval * 1.0E3 / VoieManip[voie].ADUenmV;
			if(ival < 0) ival = -ival; ival &= 0xFFFF;
			VoieTampon[voie].simu.compens = ival;
			if(prefixe) printf("%s     > compens[%s] = %d\n",prefixe,VoieManip[voie].nom,VoieTampon[voie].simu.compens);
		}
		break;
	}
}
/* ========================================================================== */
void ReglageChangeFloat(TypeReglage *regl, float fval, char *prefixe) {
	int bolo,cap,bb; int num,k;
	TypeBNModele *modele_bn; TypeBNModlRessource *ress;
	
	bolo = regl->bolo; cap = regl->capt; // bb = Bolo[bolo].captr[cap].bb.num;
	// printf("     | %s[%s] = %g\n",Bolo[bolo].nom,regl->nom,fval);
	if((bb = regl->bb) < 0) return;
	num = regl->ress;
	if(num >= 0) {
		modele_bn = &(ModeleBN[Numeriseur[bb].def.type]);
		ress = modele_bn->ress + num;
		if(ress->type == RESS_FLOAT) Numeriseur[bb].ress[num].val.r = fval;
		else Numeriseur[bb].ress[num].val.i = (int)fval;
		/* { char type[80];
			ArgKeyGetText(RESS_TYPETEXTE,ress->type,type,80);
			printf("     | Numeriseur[%d].ress[%d].val = (%s)%g pour %s\n",bb,num,type,fval,regl->nom);
		} */
		
		NumeriseurRessValChangee(&(Numeriseur[bb]),num);
		// printf("(%s) demande: %.3f, retenu: %.3f, code: %04X\n",__func__,fval,Numeriseur[bb].ress[num].val.r,Numeriseur[bb].ress[num].hval);
		NumeriseurChargeRessource(&(Numeriseur[bb]),num,0);
		strcpy(regl->user,Numeriseur[bb].ress[num].cval);
		if((k = regl->ressneg) >= 0) {
			ress = modele_bn->ress + k;
			if(ress->type == RESS_FLOAT) Numeriseur[bb].ress[k].val.r = -fval;
			else Numeriseur[bb].ress[k].val.i = -(int)fval;
			NumeriseurRessValChangee(&(Numeriseur[bb]),k);
			NumeriseurChargeRessource(&(Numeriseur[bb]),k,0);
		}
		if((k = regl->resspos) >= 0) {
			ress = modele_bn->ress + k;
			if(ress->type == RESS_FLOAT) Numeriseur[bb].ress[k].val.r = fval;
			else Numeriseur[bb].ress[k].val.i = (int)fval;
			NumeriseurRessValChangee(&(Numeriseur[bb]),k);
			NumeriseurChargeRessource(&(Numeriseur[bb]),k,0);
		}
		ReglageRafraichiTout(bb,num,regl);
	}
	ReglageEffetDeBord(regl,prefixe);
	if((regl->cmde == REGL_MODUL) || (regl->cmde == REGL_COMP)) 
		VoieTampon[Bolo[bolo].captr[cap].voie].signal.mesure = 0;

}
/* ========================================================================== */
void ReglageChargeUser(TypeDetecteur *detectr, TypeReglage *regl, int *bb_autorise, char *prefixe, char *cval) {
	int cap,bb,num; char texte[80];
	TypeNumeriseur *numeriseur; unsigned short hval;
	
	if((regl->ss_adrs > 0) && ((num = regl->ress) >= 0)) {
		cap = regl->capt; if(cap < 0) cap = 0;
		// bb = detectr->captr[cap].bb.num;
		if((bb = regl->bb) < 0) return;
		numeriseur = &(Numeriseur[bb]);
		if(bb != *bb_autorise) {
			if(*bb_autorise >= 0) {
				NumeriseurAutoriseAcces(&(Numeriseur[*bb_autorise]),0,2);
				Numeriseur[*bb_autorise].status.a_copier = 1;
			}
			numeriseur->status.a_copier = 0; NumeriseurAutoriseAcces(numeriseur,1,2);
			*bb_autorise = bb;
		}
		strncpy(numeriseur->ress[num].cval,cval,NUMER_RESS_VALTXT);
		NumeriseurRessUserChange(numeriseur,num);
		hval = NumeriseurChargeRessource(numeriseur,num,prefixe);
		NumeriseurRessHexaEncode(numeriseur,num,hval,texte,0,0);
		printf("%s > %s.%s = %s\n",prefixe?prefixe:"",detectr->nom,ModeleDet[detectr->type].regl[regl->num].nom,texte);
	}
}
/* ========================================================================== */
void ReglageChargeIval(TypeDetecteur *detectr, TypeReglage *regl, int *bb_autorise, char *prefixe, int ival) {
	int cap,bb,num; char texte[80];
	TypeNumeriseur *numeriseur; unsigned short hval;
	
	if((regl->ss_adrs > 0) && ((num = regl->ress) >= 0)) {
		cap = regl->capt; if(cap < 0) cap = 0;
		// bb = detectr->captr[cap].bb.num;
		if((bb = regl->bb) < 0) return;
		numeriseur = &(Numeriseur[bb]);
		if(bb != *bb_autorise) {
			if(*bb_autorise >= 0) {
				NumeriseurAutoriseAcces(&(Numeriseur[*bb_autorise]),0,2);
				Numeriseur[*bb_autorise].status.a_copier = 1;
			}
			numeriseur->status.a_copier = 0; NumeriseurAutoriseAcces(numeriseur,1,2);
			*bb_autorise = bb;
		}
		numeriseur->ress[num].val.i = ival;
		NumeriseurRessValChangee(numeriseur,num);
		hval = NumeriseurChargeRessource(numeriseur,num,prefixe);
		NumeriseurRessHexaEncode(numeriseur,num,hval,texte,0,0);
		printf("%s > %s.%s = %s\n",prefixe?prefixe:"",detectr->nom,ModeleDet[detectr->type].regl[regl->num].nom,texte);
	}
}
/* ========================================================================== 
void ReglageChargeHexa(TypeDetecteur *detectr, TypeReglage *regl, int *bb_autorise, char *prefixe, TypeADU hval) {
	int cap,bb; char texte[80];
	TypeNumeriseur *numeriseur;
	
	if(regl->ss_adrs > 0) {
		cap = regl->capt; if(cap < 0) cap = 0;
		bb = detectr->captr[cap].bb.num;
		numeriseur = &(Numeriseur[bb]);
		if(bb != *bb_autorise) {
			if(*bb_autorise >= 0) {
				NumeriseurAutoriseAcces(&(Numeriseur[*bb_autorise]),0,2);
				Numeriseur[*bb_autorise].status.a_copier = 1;
			}
			numeriseur->status.a_copier = 0; NumeriseurAutoriseAcces(numeriseur,1,2);
			*bb_autorise = bb;
		}
		if(NumeriseurChargeValeurBB(numeriseur,regl->ss_adrs,hval)) {
			NumeriseurRessHexaEncode(numeriseur,regl->ress,hval,texte,0,0);
			printf("%s  | %s |  %8s.%-12s = %s\n",prefixe?prefixe:"",
				   RepartiteurValeurEnvoyee,detectr->nom,ModeleDet[detectr->type].regl[regl->num].nom,texte);
		} else printf("%s  | %s\n",prefixe?prefixe:"",RepartiteurValeurEnvoyee);
	}
}
   ========================================================================== */
void ReglageChargeFlottant(TypeDetecteur *detectr, TypeReglage *regl, int *bb_autorise, char *prefixe, float valeur) {
	/* utilise dans les RazFet et les entrees/sorties de regen hardcoded */
	int cap,bb; TypeADU hval;
	TypeNumeriseur *numeriseur;
	
	cap = regl->capt; if(cap < 0) cap = 0;
	// printf("standard avant: %04X\n",Numeriseur[detectr->captr[cap].bb.num].ress[regl->ress].hval);
	// bb = detectr->captr[cap].bb.num;
	if((bb = regl->bb) < 0) return;
	numeriseur = &(Numeriseur[bb]);
	if(bb != *bb_autorise) {
		if(*bb_autorise >= 0) {
			NumeriseurAutoriseAcces(&(Numeriseur[*bb_autorise]),0,2);
			Numeriseur[*bb_autorise].status.a_copier = 1;
		}
		numeriseur->status.a_copier = 0; NumeriseurAutoriseAcces(numeriseur,1,2);
		*bb_autorise = bb;
	}
	if(regl->ress >= 0) {
		NumeriseurRessFloatDecode(numeriseur,regl->ress,valeur,&hval);
		if(NumeriseurChargeValeurBB(numeriseur,regl->ss_adrs,hval))
			printf("%s  | %s |  %8s.%-12s = %.3f\n",prefixe?prefixe:"",
			   RepartiteurValeurEnvoyee,detectr->nom,ModeleDet[detectr->type].regl[regl->num].nom,valeur);
		else printf("%s  | %s\n",prefixe?prefixe:"",RepartiteurValeurEnvoyee);
	}
	if(regl->ressneg >= 0) {
		NumeriseurRessFloatDecode(numeriseur,regl->ressneg,-valeur,&hval);
		if(NumeriseurChargeValeurBB(numeriseur,regl->negatif,hval))
			printf("%s  | %s | -%8s.%-12s = %.3f\n",prefixe?prefixe:"",
			   RepartiteurValeurEnvoyee,detectr->nom,ModeleDet[detectr->type].regl[regl->num].nom,-valeur);
		else printf("%s  | %s\n",prefixe?prefixe:"",RepartiteurValeurEnvoyee);
	}
	if(regl->resspos >= 0) {
		NumeriseurRessFloatDecode(numeriseur,regl->resspos,valeur,&hval);
		if(NumeriseurChargeValeurBB(numeriseur,regl->identik,hval))
			printf("%s  | %s | +%8s.%-12s = %.3f\n",prefixe?prefixe:"",
			   RepartiteurValeurEnvoyee,detectr->nom,ModeleDet[detectr->type].regl[regl->num].nom,valeur);
		else printf("%s  | %s\n",prefixe?prefixe:"",RepartiteurValeurEnvoyee);
	}
	// printf("standard apres: %04X\n",Numeriseur[detectr->captr[cap].bb.num].ress[regl->ress].hval);
}
/* ========================================================================== */
void ReglageChargeStandard(TypeDetecteur *detectr, TypeReglage *regl, int *bb_autorise, char *prefixe) {
	TypeNumeriseur *numeriseur; TypeBNModele *modele_bn; TypeBNModlRessource *ress;
	int ival; float fval; unsigned short hval;
	int voie,cap,bb,k; // float vsn;

	if((cap = regl->capt) < 0) cap = 0;
	// if((bb = detectr->captr[cap].bb.num) < 0) return;
	if((bb = regl->bb) < 0) return;
	numeriseur = &(Numeriseur[bb]);
	modele_bn = &(ModeleBN[numeriseur->def.type]);
	NumeriseursBB1Debloque(numeriseur,0,prefixe);
	strcpy(regl->user,regl->std);
	
	if(bb != *bb_autorise) {
		if(*bb_autorise >= 0) {
			NumeriseurAutoriseAcces(&(Numeriseur[*bb_autorise]),0,2);
			Numeriseur[*bb_autorise].status.a_copier = 1;
		}
		numeriseur->status.a_copier = 0; NumeriseurAutoriseAcces(numeriseur,1,2);
		*bb_autorise = bb;
	}
	ival = 0; fval = 0.0;
	k = regl->ress;
	if(k >= 0) {
		ress = modele_bn->ress + k;
		strncpy(numeriseur->ress[k].cval,regl->std,NUMER_RESS_VALTXT);
		NumeriseurRessUserChange(numeriseur,k);
		hval = NumeriseurChargeRessource(numeriseur,k,0);
		if(ress->type == RESS_FLOAT) fval = numeriseur->ress[k].val.r; else ival = numeriseur->ress[k].val.i;
		printf("%s  | %s |  %8s.%-12s = %s\n",prefixe?prefixe:"",
			   RepartiteurValeurEnvoyee,detectr->nom,ModeleDet[detectr->type].regl[regl->num].nom,numeriseur->ress[k].cval);
		SambaAttends(CLUZEL_DELAI_MSG);
		if(regl->cmde == REGL_GAIN) {
			voie = detectr->captr[cap].voie;
			sscanf(numeriseur->ress[k].cval,"%g",&(VoieManip[voie].gain_variable));
			DetecteurAdcCalib(voie,prefixe);
		}
		k = regl->ressneg;
		if(k >= 0) {
			if(ress->type == RESS_FLOAT) numeriseur->ress[k].val.r = -fval; else numeriseur->ress[k].val.i = -ival;
			NumeriseurRessValChangee(numeriseur,k);
			hval = NumeriseurChargeRessource(numeriseur,k,0);
			printf("%s  | %s | -%8s.%-12s = %s\n",prefixe?prefixe:"",
				   RepartiteurValeurEnvoyee,detectr->nom,ModeleDet[detectr->type].regl[regl->num].nom,numeriseur->ress[k].cval);
			SambaAttends(CLUZEL_DELAI_MSG);
		}
		k = regl->resspos;
		if(k >= 0) {
			if(ress->type == RESS_FLOAT) numeriseur->ress[k].val.r = fval; else numeriseur->ress[k].val.i = ival;
			NumeriseurRessValChangee(numeriseur,k);
			hval = NumeriseurChargeRessource(numeriseur,k,0);
			printf("%s  | %s | +%8s.%-12s = %s\n",prefixe?prefixe:"",
				   RepartiteurValeurEnvoyee,detectr->nom,ModeleDet[detectr->type].regl[regl->num].nom,numeriseur->ress[k].cval);
			SambaAttends(CLUZEL_DELAI_MSG);
		}
	}
	ReglageEffetDeBord(regl,prefixe);
}
/* ========================================================================== */
INLINE void ReglageChargeTerminee(int bb_autorise) {
	if(bb_autorise >= 0) {
		NumeriseurAutoriseAcces(&(Numeriseur[bb_autorise]),0,2);
		Numeriseur[bb_autorise].status.a_copier = 1;
	}
}
#ifdef ForSamba
/* ========================================================================== */
void ReglageFinalise(TypeReglage *regl) {
	/* 1.- met a jour la valeur utilisateur associee au reglage (la ressource numeriseur etant a jour)
	   2.- met a jour la ressource numeriseur opposee si le cablage est differentiel
	   3.- met a jour les variables utilisees pendant l'acquisition ()
	   4.- met a jour les affichages contenant ces reglages ()
	   5.- retourne le numero de la ressource opposee (-1 si non differentiel)
	*/
	int bolo,cap,bb,voie; int num,k; float fval; int ival;
	TypeBNModele *modele_bn; TypeBNModlRessource *ress;
	
	ival = 0; fval = 0.0;
	bolo = regl->bolo; cap = regl->capt; // bb = Bolo[bolo].captr[cap].bb.num;
	if((bb = regl->bb) < 0) return;
	modele_bn = &(ModeleBN[Numeriseur[bb].def.type]);
	/* 1.- valeur utilisateur */
	if((num = regl->ress) >= 0) {
		strcpy(regl->user,Numeriseur[bb].ress[num].cval);
		ress = modele_bn->ress + num;
		if(ress->type == RESS_FLOAT) fval = regl->val.r = Numeriseur[bb].ress[num].val.r;
		else ival = regl->val.i = Numeriseur[bb].ress[num].val.i;
	}
	/* 2.- ressource opposee (si differentiel) */
	if((k = regl->ressneg) >= 0) {
		ress = modele_bn->ress + k;
		if(ress->type == RESS_FLOAT) Numeriseur[bb].ress[k].val.r = -fval;
		else Numeriseur[bb].ress[k].val.i = -ival;
		NumeriseurRessValChangee(&(Numeriseur[bb]),k);
		NumeriseurChargeRessource(&(Numeriseur[bb]),k,"  |");
	}
	/* 3.- ressource identique (si differentiel) */
	if((k = regl->resspos) >= 0) {
		ress = modele_bn->ress + k;
		if(ress->type == RESS_FLOAT) Numeriseur[bb].ress[k].val.r = fval;
		else Numeriseur[bb].ress[k].val.i = ival;
		NumeriseurRessValChangee(&(Numeriseur[bb]),k);
		NumeriseurChargeRessource(&(Numeriseur[bb]),k,"  |");
	}
	/* 4.- variables de l'acquisition */
	ReglageEffetDeBord(regl,"  |");
	/* 5.- Affichages */
	ReglageRafraichiTout(bb,num,regl);
	if((voie = Bolo[bolo].captr[cap].voie) >= 0) {
		if(VoieInfo[voie].menu_voie) {
			if(OpiumAlEcran(VoieInfo[voie].menu_voie)) {
				MenuItemAllume(VoieInfo[voie].menu_voie,(IntAdrs)"Nouveau std",0,GRF_RGB_YELLOW);
				MenuItemAllume(VoieInfo[voie].menu_voie,(IntAdrs)"Charger std",0,GRF_RGB_YELLOW);
			}
		}
	}
	
	return;
}
#endif
/* ========================================================================== */
void ReglageRafraichiTout(int bb, int num, TypeReglage *regl) {
	int i,j,nb;

	if(bb >= 0) {
		if(regl) /* trop fin en general, mais des fois.. */ {
			int bolo,cap,voie; int j;
			TypeNumeriseurCmde *interface;
		
			interface = &(Numeriseur[bb].interface);
			if(interface->planche && OpiumDisplayed(interface->planche)) {
				for(i=0, j=interface->prems; i<interface->nb; i++,j++) {
					if((NumeriseurInstr[j].num == regl->ress) || (NumeriseurInstr[j].num == regl->ressneg) || (NumeriseurInstr[j].num == regl->resspos)) {
						//- NumeriseurRessHexaChange(&(Numeriseur[bb]),regl->ress);
						switch(NumeriseurInstr[j].type) {
							case OPIUM_PANEL: PanelRefreshVars((Panel)NumeriseurInstr[j].instrum); break;
							case OPIUM_INSTRUM: InstrumRefreshVar((Instrum)NumeriseurInstr[j].instrum); break;
						}
					}
				}
			}
			bolo = regl->bolo; cap = regl->capt;
			voie = Bolo[bolo].captr[cap].voie;
			if(VoieInfo[voie].planche && OpiumDisplayed(VoieInfo[voie].planche)) {
				for(i=0, j=VoieInfo[voie].prems; i<VoieInfo[voie].nb; i++,j++) {
					if((NumeriseurInstr[j].numeriseur == &(Numeriseur[bb])) 
					&& ((NumeriseurInstr[j].num == regl->ress) || (NumeriseurInstr[j].num == regl->ressneg) || (NumeriseurInstr[j].num == regl->resspos))) {
						//- NumeriseurRessHexaChange(&(Numeriseur[bb]),regl->ress);
						switch(NumeriseurInstr[j].type) {
							case OPIUM_PANEL: PanelRefreshVars((Panel)NumeriseurInstr[j].instrum); break;
							case OPIUM_INSTRUM: InstrumRefreshVar((Instrum)NumeriseurInstr[j].instrum); break;
						}
					}
				}
			}
		} else {
			for(j=0; j<NumeriseurInstrNb; j++) if((NumeriseurInstr[j].numeriseur == &(Numeriseur[bb])) && (NumeriseurInstr[j].num == num)) {
				switch(NumeriseurInstr[j].type) {
					case OPIUM_INSTRUM: if(OpiumDisplayed(((Instrum)NumeriseurInstr[j].instrum)->cdr)) InstrumRefreshVar((Instrum)NumeriseurInstr[j].instrum); break;
					case OPIUM_PANEL: if(OpiumDisplayed(((Panel)NumeriseurInstr[j].instrum)->cdr)) PanelRefreshVars((Panel)NumeriseurInstr[j].instrum); break;
				}
			}
		}
	}
	if(OpiumDisplayed(pReglagesConsignes[LECT_CONS_ETAT]->cdr)) PanelRefreshVars(pReglagesConsignes[LECT_CONS_ETAT]);
	if(OpiumDisplayed(pReglagesConsignes[LECT_CONS_CMDE]->cdr)) {
		if(regl) {
			nb = PanelItemNb(pReglagesConsignes[LECT_CONS_CMDE]);
			for(i=0; i<nb; i++) if(DetecReglagesLocaux[i] == regl) break;
			if(i < nb) PanelItemUpdate(pReglagesConsignes[LECT_CONS_CMDE],i+1,1);
		} else PanelRefreshVars(pReglagesConsignes[LECT_CONS_CMDE]);
	}
}
/* ========================================================================== */
int DetecteurChargeStandard(int bolo, char vrai_bolo, char *prefixe) {
	TypeDetecteur *detectr;
	int reste,num,bb_autorise;
	int32 *prgm,instr;

	detectr = Bolo + bolo;	
	if(vrai_bolo) {
		if(prefixe) printf("%s . Detecteur %s\n",prefixe,detectr->nom); else printf(". Detecteur %s\n",detectr->nom);
		bb_autorise = -1;
		for(num=0; num<detectr->nbreglages; num++) 
			ReglageChargeStandard(detectr,&(detectr->reglage[num]),&bb_autorise,prefixe);
		ReglageChargeTerminee(bb_autorise);
	}
	
	prgm = Commandes + detectr->debut;
	if((reste = detectr->taille)) {
		NumeriseurAutoriseAcces(&(Numeriseur[detectr->captr[0].bb.num]),1,2);
		while(reste-- > 0) {
			TypeADU hval;
			instr = *prgm++;
			hval = (instr & 0xFFFF); instr = instr >> 16;
			if(NumeriseurChargeValeurBB(&(Numeriseur[detectr->captr[0].bb.num]),instr & 0x3F,hval))
				printf("%s  |  transmis: %s\n",prefixe?prefixe:"",RepartiteurValeurEnvoyee);
			else printf("%s  | %s\n",prefixe?prefixe:"",RepartiteurValeurEnvoyee);
			SambaAttends(CLUZEL_DELAI_MSG);
		}
		NumeriseurAutoriseAcces(&(Numeriseur[detectr->captr[0].bb.num]),0,2);
	}
	
	printf("%s  | initialisation terminee\n",prefixe?prefixe:"");
	return(0);
}
/* ========================================================================== */
int DetecteurChargeTous(char *prefixe) {
	int bolo;
	
	if(prefixe) {
		if(strlen(prefixe) >= 8) printf("%s/ Mise en service des detecteurs\n",DateHeure());
		else printf("%sMise en service des detecteurs\n",prefixe);
	} // else TermHold(tCmdes);
	
	for(bolo=0; bolo<BoloNb; bolo++) if(Bolo[bolo].a_lire) DetecteurChargeStandard(bolo,1,prefixe);
	
	if(prefixe) {
		if(strlen(prefixe) >= 8) printf("%s/ Mise en service terminee\n",DateHeure());
		else printf("%sMise en service terminee\n",prefixe);
	} // else TermRelease(tCmdes);
	SambaNote("Mise en service des detecteurs\n");
	return(0);
}
/* ========================================================================== */
int DetecteurMiseEnService() {
	int bb,bolo,cap,voie,rep,n; TypeRepartiteur *repart;
	char log;

	log = 1;
	/* Ils viennent peut-etre d'etre echanges ou rallumes */
	for(bb=0; bb<NumeriseurNb; bb++) Numeriseur[bb].bloque = ModeleBN[Numeriseur[bb].def.type].a_debloquer;
	/* D'autre part, il faut indiquer les repartiteurs a adresser */
	if(log) printf("* Liste des repartiteurs a lire:\n");
	for(rep=0; rep<RepartNb; rep++) Repart[rep].utile = 0;
	n = 0;
	for(bolo=0; bolo<BoloNb; bolo++) if(Bolo[bolo].a_lire) {
		for(cap=0; cap<Bolo[bolo].nbcapt; cap++) {
			voie = Bolo[bolo].captr[cap].voie;
			VoieTampon[voie].lue = 1;
			if((bb = Bolo[bolo].captr[cap].bb.num) < 0) {
				if(log) printf("  ! Pas de numeriseur pour transmettre la voie %s\n",VoieManip[voie].nom);
				n++; continue;
			}
			if(!(repart = Numeriseur[bb].repart)) {
				if(log) printf("  ! Pas de repartiteur pour transmettre le numeriseur %s\n",Numeriseur[bb].nom);
				n++; continue;
			}
			if(!(repart->utile)) {
				if(log) printf("  + %s\n",repart->nom);
				if(!RepartiteurAdopte(repart,1)) n++;
			}
		}
	};
	if(n && log) printf("  !!! %d repartiteur%s en defaut\n",Accord1s(n));
	RepartiteursSelectionneVoiesLues(0,1);
	
	return(DetecteurChargeTous("*"));
}
/* ========================================================================== */
void DetecteurRazFet(TypeDetecteur *detectr, TypeReglage *regl, int *bb_autorise, char *prefixe, char raz) {
	char valeurs[256]; char *r,*off,*on,*val;
	
	if(regl->valeurs) {
		strncpy(valeurs,regl->valeurs,256); valeurs[255] = '\0';
	} else valeurs[0] = '\0';
	if(valeurs[0]) {
		r = valeurs; off = ItemJusquA('/',&r);
		if(raz) { on = ItemJusquA('/',&r); r = on; } else r = off;
		while(*r) {
			val = ItemJusquA(',',&r);
			if(*val) ReglageChargeUser(detectr,regl,bb_autorise,prefixe,val);
		}
	} else ReglageChargeIval(detectr,regl,bb_autorise,prefixe,raz);
}
/* ========================================================================== */
int DetecteurRazFetEnDur(Menu menu, MenuItem *item, char raz) {
	int cap,voie,bolo,num,bb_autorise; int m;
	char affiche;
	Cadre cdr;
	WndContextPtr allume;

	if(ReglRazEnCours) { ReglRazEnCours = 0; return(0); }
	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_raz == menu) break;
		if(voie >= VoiesNb) return(0);
	} else voie = (int)item;
	bolo = VoieManip[voie].det;
	cap = VoieManip[voie].cap;
	// printf("(%s) recherche du reglage de type %d sur le bolo #%d, voie #%d, capteur #%d\n",__func__,RESS_FET_MASSE,bolo,voie,cap);
	if(bolo < 0) return(0);
	if((num = ReglageNum(bolo,voie,raz)) < 0) {
		OpiumError("La voie %s ne contient pas de reglage permettant un RAZ_%s",VoieManip[voie].nom,ReglCmdeListe[raz]);
		return(0);
	}
	ReglRazEnCours = 1;
	affiche = 0; cdr = 0; allume = 0;
	if(OpiumDisplayed(VoieInfo[voie].planche) && menu) /* anciennement bBoloReglageBB1 */ {
		cdr = menu->cdr; affiche = 1;
		allume = MenuItemGetContext(menu,1);
		if(!allume) {
			if(!ReglAllumeRazfet) OpiumDrawingRGB(cdr,&ReglAllumeRazfet,0,0,0);
			MenuItemSetContext(menu,1,ReglAllumeRazfet);
			allume = MenuItemGetContext(menu,1);
		}
		affiche = (allume != 0);
	}
	if(VoieInfo[voie].raz) PanelItemSelect(VoieInfo[voie].raz,1,0);
	if(menu) MenuItemSetText(menu,1,"Stoppe RAZ FET");
	bb_autorise = -1;
	if(raz == REGL_FET_MASSE)
	//	ReglageChargeHexa(&(Bolo[bolo]),&(Bolo[bolo].reglage[num]),&bb_autorise,DateHeure(),0x0004);
		ReglageChargeUser(&(Bolo[bolo]),&(Bolo[bolo].reglage[num]),&bb_autorise,DateHeure(),"masse");
	else if(raz == REGL_FET_CC)
		ReglageChargeFlottant(&(Bolo[bolo]),&(Bolo[bolo].reglage[num]),&bb_autorise,DateHeure(),0.0);
	m = ReglRazDuree;
	while(ReglRazDuree-- && ReglRazEnCours) {
		if(affiche) {
			WndContextBgndRGB(cdr->f,allume,WND_COLOR_MAX,WND_COLOR_DEMI,0);
			OpiumRefresh(cdr);
			OpiumUserAction();
		}
		SambaAttends(500);
		if(affiche) {
			WndContextBgndRGB(cdr->f,allume,WND_COLOR_QUART,WND_COLOR_QUART,WND_COLOR_QUART);
			OpiumRefresh(cdr); if(VoieInfo[voie].raz) PanelRefreshVars(VoieInfo[voie].raz);
			OpiumUserAction();
		}
		SambaAttends(500);
	}
	if(raz == REGL_FET_MASSE)
	//	ReglageChargeHexa(&(Bolo[bolo]),&(Bolo[bolo].reglage[num]),&bb_autorise,DateHeure(),0x0000);
		ReglageChargeUser(&(Bolo[bolo]),&(Bolo[bolo].reglage[num]),&bb_autorise,DateHeure(),"libre");
	else if(raz == REGL_FET_CC)
		ReglageChargeFlottant(&(Bolo[bolo]),&(Bolo[bolo].reglage[num]),&bb_autorise,DateHeure(),-10.0); /* anciennement, +10.0 */
	ReglageChargeTerminee(bb_autorise);
	Bolo[bolo].razfet_en_cours = 0;
	ReglRazDuree = m;
	if(menu) {
		MenuItemSetText(menu,1,"Lancer RAZ FET");
		MenuItemSetContext(menu,1,0);
	}
	if(VoieInfo[voie].raz) PanelItemSelect(VoieInfo[voie].raz,1,1);
	if(affiche) { OpiumRefresh(cdr); if(VoieInfo[voie].raz) PanelRefreshVars(VoieInfo[voie].raz); }
	ReglRazEnCours = 0;
	return(0);
}
/* ========================================================================== */
int DetecteurRazFetMasse(Menu menu, MenuItem *item) {
	return(DetecteurRazFetEnDur(menu,item,REGL_FET_MASSE));
}
/* ========================================================================== */
int DetecteurRazFetCC(Menu menu, MenuItem *item) {
	return(DetecteurRazFetEnDur(menu,item,REGL_FET_CC));
}
/* ========================================================================== */
int DetecteurRazFetBouton(Menu menu, MenuItem *item) {
	int cap,voie,bolo,num,bb_autorise;
	int m; char affiche;
	TypeDetecteur *detectr; TypeReglage *regl;
	char valeurs[256]; char *r,*off,*on,*val;
	//- TypeModeleCable *cablage; char *nom; int modl,n;
	Cadre cdr; WndContextPtr allume;
	
	if(ReglRazEnCours) { ReglRazEnCours = 0; return(0); }
	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_raz == menu) break;
		if(voie >= VoiesNb) return(0);
	} else voie = (int)item;
	bolo = VoieManip[voie].det;
	cap = VoieManip[voie].cap;
	// printf("(%s) recherche du reglage de type %d sur le bolo #%d, voie #%d, capteur #%d\n",__func__,RESS_FET_MASSE,bolo,voie,cap);
	if(bolo < 0) return(0);
	if((num = ReglageNum(bolo,voie,REGL_RAZFET)) < 0) {
		OpiumError("La voie %s ne contient pas de reglage de type %s",VoieManip[voie].nom,ReglCmdeListe[REGL_RAZFET]);
		return(0);
	}
	cdr = 0; allume = 0; off = 0; // GCC
	printf("%s/ RAZ FET demande\n",DateHeure());
	detectr = &(Bolo[bolo]); regl = &(detectr->reglage[num]);
	ReglRazEnCours = 1;
	affiche = 0;
	if(OpiumDisplayed(VoieInfo[voie].planche) && menu) /* anciennement bBoloReglageBB1 */ {
		cdr = menu->cdr; affiche = 1;
		allume = MenuItemGetContext(menu,1);
		if(!allume) {
			if(!ReglAllumeRazfet) OpiumDrawingRGB(cdr,&ReglAllumeRazfet,0,0,0);
			MenuItemSetContext(menu,1,ReglAllumeRazfet);
			allume = MenuItemGetContext(menu,1);
		}
		affiche = (allume != 0);
	}
	if(VoieInfo[voie].raz) PanelItemSelect(VoieInfo[voie].raz,1,0);
	if(menu) MenuItemSetText(menu,1,"Stoppe RAZ FET");
	if(regl->valeurs) {
		strncpy(valeurs,regl->valeurs,256); valeurs[255] = '\0';
	} else valeurs[0] = '\0';
	if(valeurs[0]) { r = valeurs; off = ItemJusquA('/',&r); on = ItemJusquA('/',&r); }
	
	bb_autorise = -1;
	if(valeurs[0]) {
		r = on; while(*r) {
			val = ItemJusquA(',',&r);
			if(*val) ReglageChargeUser(detectr,regl,&bb_autorise,DateHeure(),val);
		}
	} else ReglageChargeIval(detectr,regl,&bb_autorise,DateHeure(),1);
	detectr->razfet_en_cours = 1;
	m = ReglRazDuree;
	while(ReglRazDuree-- && ReglRazEnCours) {
		if(affiche) {
			WndContextBgndRGB(cdr->f,allume,WND_COLOR_MAX,WND_COLOR_DEMI,0);
			OpiumRefresh(cdr);
			OpiumUserAction();
		}
		SambaAttends(500);
		if(affiche) {
			WndContextBgndRGB(cdr->f,allume,WND_COLOR_QUART,WND_COLOR_QUART,WND_COLOR_QUART);
			OpiumRefresh(cdr); if(VoieInfo[voie].raz) PanelRefreshVars(VoieInfo[voie].raz);
			OpiumUserAction();
		}
		SambaAttends(500);
	}
	if(valeurs[0]) {
		r = off; while(*r) {
			val = ItemJusquA(',',&r);
			if(*val) ReglageChargeUser(detectr,regl,&bb_autorise,DateHeure(),val);
		}
	} else ReglageChargeIval(detectr,regl,&bb_autorise,DateHeure(),0);
	ReglageChargeTerminee(bb_autorise);
	detectr->razfet_en_cours = 0;
	printf("%s/ RAZ FET termine\n",DateHeure());
	detectr->razfet_en_cours = 0;
	ReglRazDuree = m;
	if(menu) {
		MenuItemSetText(menu,1,"Lancer RAZ FET");
		MenuItemSetContext(menu,1,0);
	}
	if(VoieInfo[voie].raz) PanelItemSelect(VoieInfo[voie].raz,1,1);
	if(affiche) { OpiumRefresh(cdr); if(VoieInfo[voie].raz) PanelRefreshVars(VoieInfo[voie].raz); }
	ReglRazEnCours = 0;
	return(0);
}
/* ========================================================================== */
int DetecteurRazFetEDW() { 
/*	anciennement ReglageRazNTD(1); DetecteurRazNbSi(1); (= raz en serie), mais on tente le parallele */
	int bolo,num,bb_autorise; int duree_raz,ecoule; char trouve;
	TypeReglage *regl_cc_fet;

#ifndef VOIES_STYLE_UNIQUE
	trouve = 0; duree_raz = 0;
	for(bolo=0; bolo<BoloNb; bolo++) if(Bolo[bolo].local && strcmp(Bolo[bolo].nom,DETEC_STANDARD)) {
		if((ReglageNum(bolo,-1,REGL_RAZFET) >= 0) || (ReglageNum(bolo,-1,REGL_FET_MASSE) >= 0) || (ReglageNum(bolo,-1,REGL_FET_CC) >= 0)) {
		   trouve = 1;
		   if(ModeleDet[Bolo[bolo].type].duree_raz > duree_raz) duree_raz = ModeleDet[Bolo[bolo].type].duree_raz;
		}
	}
	if(!trouve) {
		OpiumError("Aucun detecteur n'a de reglage pour faire un RAZ FET");
		return(0);
	}
	if(OpiumReadInt("Duree RAZ detecteurs (s)",&duree_raz) == PNL_CANCEL) return(0);
	printf("%s/ Debut RAZ detecteurs\n",DateHeure());
	bb_autorise = -1;
	for(bolo=0; bolo<BoloNb; bolo++) if(Bolo[bolo].local && strcmp(Bolo[bolo].nom,DETEC_STANDARD)) {
		if((num = ReglageNum(bolo,-1,REGL_RAZFET)) >= 0) 
			DetecteurRazFet(&(Bolo[bolo]),&(Bolo[bolo].reglage[num]),&bb_autorise,"         ",1);
		else if((num = ReglageNum(bolo,-1,REGL_FET_MASSE)) >= 0) 
			ReglageChargeUser(&(Bolo[bolo]),&(Bolo[bolo].reglage[num]),&bb_autorise,"          ","masse");
		else if((regl_cc_fet = Bolo[bolo].regl_cc_fet)) 
			ReglageChargeFlottant(&(Bolo[bolo]),regl_cc_fet,&bb_autorise,"          ",0.0);
	}
	printf("          . Attente de %d secondes\n",duree_raz);
	OpiumProgresInit(duree_raz); ecoule = 0;
	while(duree_raz--) {
		SambaAttends(1000);
		if(!OpiumProgresRefresh(++ecoule)) break;
		OpiumUserAction();
	}
	OpiumProgresClose();
	if(duree_raz > 0) printf("          . Attente reduite a %d secondes\n",ecoule);
	for(bolo=0; bolo<BoloNb; bolo++) if(Bolo[bolo].local && strcmp(Bolo[bolo].nom,DETEC_STANDARD)) {
		if((num = ReglageNum(bolo,-1,REGL_RAZFET)) >= 0) 
			DetecteurRazFet(&(Bolo[bolo]),&(Bolo[bolo].reglage[num]),&bb_autorise,"         ",0);
		else if((num = ReglageNum(bolo,-1,REGL_FET_MASSE)) >= 0) 
			ReglageChargeUser(&(Bolo[bolo]),&(Bolo[bolo].reglage[num]),&bb_autorise,"          ","libre");
		else if((regl_cc_fet = Bolo[bolo].regl_cc_fet)) 
			ReglageChargeFlottant(&(Bolo[bolo]),regl_cc_fet,&bb_autorise,"          ",-10.0);
	}
	printf("%s/ Fin RAZ detecteurs\n",DateHeure());
	ReglageChargeTerminee(bb_autorise);
	if(duree_raz > 0) OpiumWarn("RAZ detecteurs interrompu au bout de %d/%d secondes",ecoule,ecoule+duree_raz);
	SambaNote("RAZ FET general d'une duree de %s secondes\n",ecoule);
#endif /* VOIES_STYLE_UNIQUE */
	return(0);
}
/* ========================================================================== */
int DetecteurFlip() {
	int bolo,assoc; int num,nb,i,j,k,n,rc; float valeur;
	char assoc_aussi,reste_a_sauver;
	int cap,bb,bb_autorise;
	char *liste[DETEC_MAX]; int index[DETEC_MAX]; char designation[80];
	TypeDetecteur *detectr; TypeReglage *regl;
	Panel p;
	char log;
	
	if(AccesRestreint()) return(0);
	log = 4;
	num = 0;
	assoc_aussi = 0;
	nb = 0;
	for(bolo=0; bolo<BoloNb; bolo++) if(Bolo[bolo].a_lire) {
		detectr = Bolo + bolo;
		for(i=0; i<detectr->nbreglages; i++) {
			// printf("%s.%s de type %s\n",detectr->nom,ModeleDet[detectr->type].regl[i].nom,RessCategListe[detectr->reglage[i].cmde]);
			if(detectr->reglage[i].cmde == REGL_POLAR) break;
		}
		if(i >= detectr->nbreglages) continue;
		if(!(detectr->assoc_imposes)) {
			for(k=0; k<detectr->nbassoc; k++) {
				for(n=0; n<nb; n++) if(detectr->assoc[k] == index[n]) break;
				if(n < nb) break;
			}
			if(k < detectr->nbassoc) continue;
		}
		index[nb] = bolo;
		liste[nb] = detectr->nom;
		if(detectr->nbassoc && !assoc_aussi) assoc_aussi = (detectr->assoc_imposes)? -1: 1;
		nb++;
	}
	if(nb == 0) {
		OpiumError("Pas de detecteur candidat pour cet exercice");
		return(0);
	} else if(nb > 1) {
		liste[nb] = "\0";
		if(assoc_aussi) {
			p = PanelCreate(2);
			PanelList(p,"Detecteur a inverser",liste,&num,DETEC_NOM);
			PanelBool(p,"Y compris associes",&assoc_aussi);
			if(assoc_aussi < 0) assoc_aussi = 0;
			rc = OpiumExec(p->cdr);
			PanelDelete(p);
			if(rc == PNL_CANCEL) return(0);
		} else {
			if(OpiumReadList("Detecteur a inverser",liste,&num,DETEC_NOM) == PNL_CANCEL) return(0);
		}
	}
	bolo = index[num];
	detectr = Bolo + bolo;
	if(assoc_aussi) sprintf(designation,"%s",detectr->nom);
	else sprintf(designation,"%s (+ %d associe%s)",detectr->nom,Accord1s(detectr->nbassoc));
	printf("%s/ Inversion de la polarisation du detecteur %s\n",DateHeure(),designation);
	SambaNote("Inversion de la polarisation du detecteur %s\n",designation);
	reste_a_sauver = 0;
	/* Ci-dessous, on suppose que les ressources liees aux polars sont representees par des float */
	bb_autorise = -1;
	for(i=0; i<detectr->nbreglages; i++) {
		regl = &(detectr->reglage[i]);
		if(regl->cmde == REGL_POLAR) {
			cap = regl->capt; if(cap < 0) cap = 0;
			// bb = detectr->captr[cap].bb.num;
			if((bb = regl->bb) < 0) continue;
			if(bb != bb_autorise) {
				if(bb_autorise >= 0) NumeriseurAutoriseAcces(&(Numeriseur[bb_autorise]),0,2);
				NumeriseurAutoriseAcces(&(Numeriseur[bb]),1,2);
				bb_autorise = bb;
			}
			num = regl->ress;
			if(num >= 0) {
				valeur = Numeriseur[bb].ress[num].val.r;
				Numeriseur[bb].ress[num].val.r = -valeur;
				// printf("          . BB[%d].ress[%d]=%g\n",bb,num,Numeriseur[bb].ress[num].val.r);
				NumeriseurRessValChangee(&(Numeriseur[bb]),num);
				NumeriseurChargeRessource(&(Numeriseur[bb]),num,"          |");
				strcpy(regl->user,Numeriseur[bb].ress[num].cval);
				strcpy(regl->std,Numeriseur[bb].ress[num].cval);
				if((k = regl->ressneg) >= 0) {
					valeur = Numeriseur[bb].ress[k].val.r;
					Numeriseur[bb].ress[k].val.r = -valeur;
					NumeriseurRessValChangee(&(Numeriseur[bb]),k);
					NumeriseurChargeRessource(&(Numeriseur[bb]),k,"          |");
				}
				if((k = regl->resspos) >= 0) {
					valeur = Numeriseur[bb].ress[k].val.r;
					Numeriseur[bb].ress[k].val.r = -valeur;
					NumeriseurRessValChangee(&(Numeriseur[bb]),k);
					NumeriseurChargeRessource(&(Numeriseur[bb]),k,"          |");
				}
				ReglageRafraichiTout(bb,num,regl);
			}
		}
	}
	if(bb_autorise >= 0) NumeriseurAutoriseAcces(&(Numeriseur[bb_autorise]),0,2);
	if(Bolo[bolo].fichier[0]) DetecteurEcrit(0,&(Bolo[bolo]),bolo,1,DETEC_REGLAGES); else reste_a_sauver = 1;
	if(assoc_aussi) {
		for(j=0; j<detectr->nbassoc; j++) if((assoc = detectr->assoc[j]) >= 0) {
			bb_autorise = -1;
			for(i=0; i<Bolo[assoc].nbreglages; i++) {
				regl = &(Bolo[assoc].reglage[i]);
				if(regl->cmde == REGL_POLAR) {
					cap = regl->capt; if(cap < 0) cap = 0;
					// bb = Bolo[assoc].captr[cap].bb.num;
					if((bb = regl->bb) < 0) continue;
					num = regl->ress;
					if(bb != bb_autorise) {
						if(bb_autorise >= 0) NumeriseurAutoriseAcces(&(Numeriseur[bb_autorise]),0,2);
						NumeriseurAutoriseAcces(&(Numeriseur[bb]),1,2);
						bb_autorise = bb;
					}
					if(num >= 0) {
						valeur = Numeriseur[bb].ress[num].val.r;
						Numeriseur[bb].ress[num].val.r = -valeur;
						// printf("          . BB[%d].ress[%d]=%g\n",bb,num,Numeriseur[bb].ress[num].val.r);
						NumeriseurRessValChangee(&(Numeriseur[bb]),num);
						NumeriseurChargeRessource(&(Numeriseur[bb]),num,"          |");
						strcpy(regl->user,Numeriseur[bb].ress[num].cval);
						strcpy(regl->std,Numeriseur[bb].ress[num].cval);
						if((k = regl->ressneg) >= 0) {
							valeur = Numeriseur[bb].ress[k].val.r;
							Numeriseur[bb].ress[k].val.r = -valeur;
							NumeriseurRessValChangee(&(Numeriseur[bb]),k);
							NumeriseurChargeRessource(&(Numeriseur[bb]),k,"          |");
						}
						if((k = regl->resspos) >= 0) {
							valeur = Numeriseur[bb].ress[k].val.r;
							Numeriseur[bb].ress[k].val.r = -valeur;
							NumeriseurRessValChangee(&(Numeriseur[bb]),k);
							NumeriseurChargeRessource(&(Numeriseur[bb]),k,"          |");
						}
						ReglageRafraichiTout(bb,num,regl);
					}
				}
			}
			if(bb_autorise >= 0) NumeriseurAutoriseAcces(&(Numeriseur[bb_autorise]),0,2);
			if(Bolo[assoc].fichier[0]) DetecteurEcrit(0,&(Bolo[assoc]),assoc,1,DETEC_REGLAGES); else reste_a_sauver = 1;
		}
	}
	if(reste_a_sauver) { printf("%s/ ",DateHeure()); DetecteurEcritTous(DETEC_REGLAGES,-1); }
	
	return(0);
}
/* ========================================================================== */
static int DetecteurViInsere() {
	int n,m;
    
	if(ReglTabNb < MAXVI) {
		n = 0;
		for( ; n<ReglTabNb; n++) if(ReglIbolo < ReglTabI[n]) break; 
		for(m=ReglTabNb; m>n; m--) {
			ReglTabI[m] = ReglTabI[m - 1];
			ReglTabV[m] = ReglTabV[m - 1];
			ReglTabR[m] = ReglTabR[m - 1];
			ReglTabT[m] = ReglTabT[m - 1];
		}
		ReglTabI[n] = ReglIbolo;
		ReglTabV[n] = ReglVbolo;
		ReglTabR[n] = (ReglIbolo != 0.0)? ReglVbolo / ReglIbolo: 0.0;
		ReglTabT[n] = ReglTbolo;
		ReglTabNb++;
        /* printf("  n      I            V            R\n");
		for(n=0; n<ReglTabNb; n++) 
			printf("%3d  %-11.7g  %-11.7g  %-11.7g\n",n,ReglTabI[n],ReglTabV[n],ReglTabR[n]); */
        GraphUse(gGestVi,ReglTabNb);
        if(OpiumDisplayed(gGestVi->cdr)) OpiumRefresh(gGestVi->cdr);
	} else OpiumError("Deja %d points entres!",MAXVI);
	return(0);
}
/* ========================================================================== */
void DetecteurViMesuree(int voie) {
	float compens;
	
	// compens = DetecteurConsigneCourante(voie,REGL_COMP) + (COMP_COEF * VoieInfo[voie].mesure_enV);
	compens = COMP_COEF * VoieInfo[voie].mesure_enV;
	if(VoieManip[voie].compens) compens += *(VoieManip[voie].compens);
	VoieInfo[voie].V = 1000.0 * compens / VoieManip[voie].gain_fixe; /* mV */
	ReglVbolo = VoieInfo[voie].V;
	if(VoieInfo[voie].I != 0.0) VoieInfo[voie].R = VoieInfo[voie].V / VoieInfo[voie].I; /* MO */
	else VoieInfo[voie].R = 0.0;
	ReglTbolo = *T_Bolo;
	if(VoieInfo[voie].vi) { if(OpiumDisplayed((VoieInfo[voie].vi)->cdr)) PanelRefreshVars(VoieInfo[voie].vi); }
}
/* ========================================================================== */
int DetecteurCompenseVoie(Menu menu, MenuItem *item) {
	/* modifie le reglage de type RESS_COMP selon le niveau mesure */
    int voie,bolo,cap,bb,k,num;
	float rval;
	TypeReglage *regl;
	TypeBNModele *modele_bn; TypeBNModlRessource *ress;

    if(!Acquis[AcquisLocale].etat.active) {
		OpiumError("La compensation ne peut se faire que pendant la lecture");
		return(0);
	}
	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_modul == menu) break;
		if(voie >= VoiesNb) return(0);
	} else voie = (int)item;
	bolo = VoieManip[voie].det;
	if((num = ReglageNum(bolo,voie,REGL_COMP))  < 0) {
		OpiumError("Pas de compensation sur la voie %s",VoieManip[voie].nom);
		DetecteurDumpUnSeul(bolo,1,1);
		return(0);
	}
	regl = &(Bolo[bolo].reglage[num]);
	cap = regl->capt; k = regl->ress; // bb = Bolo[regl->bolo].captr[cap].bb.num;
	if((bb = regl->bb) < 0) return(0);
	modele_bn = &(ModeleBN[Numeriseur[bb].def.type]); ress = modele_bn->ress + k;
	if(ress->type == RESS_FLOAT) rval = Numeriseur[bb].ress[k].val.r;
	else rval = (float)Numeriseur[bb].ress[k].val.i;
	VoieInfo[voie].compens = (COMP_COEF * VoieInfo[voie].mesure_enV) + rval;
	if((VoieInfo[voie].mesure_enV != 0.0) && VoieInfo[voie].menu_voie) {
		if(OpiumAlEcran(VoieInfo[voie].menu_voie)) {
			MenuItemAllume(VoieInfo[voie].menu_voie,(IntAdrs)"Nouveau std",0,GRF_RGB_YELLOW);
			MenuItemAllume(VoieInfo[voie].menu_voie,(IntAdrs)"Charger std",0,GRF_RGB_YELLOW);
		}
	}
	if(VerifCompens) {
		if((num = ReglageNum(bolo,voie,REGL_MODUL))  < 0) {
			OpiumError("Pas d'excitation sur la voie %s",VoieManip[voie].nom);
			DetecteurDumpUnSeul(bolo,1,1);
			return(0);
		}
		TypeReglage *ampl;
		ampl = &(Bolo[bolo].reglage[num]); k = ampl->ress;
//		VoieManip[voie].ADUenmV = ModeleADC[type_adc].polar * 1000.0 / (float)(VoieManip[voie].ADCmask + 1); /* valeur d'un ADU en mV */
//		VoieManip[voie].ADUenV = VoieManip[voie].ADUenmV / VoieManip[voie].gain_variable / 1000.0;
//	!! convention polar: Tension totale donc ADU(mV) independant du signage: if(ModeleADC[type_adc].signe) VoieManip[voie].ADUenmV *= 2.0;
//		printf("* Voie %s: 1 ADU = %g mV (entree ADC), %g keV/nV avant gains\n",VoieManip[voie].nom,VoieManip[voie].ADUenmV,VoieManip[voie].nVenkeV);
//		VoieManip[voie].gain_fixe = CablageSortant[detectr->pos].captr[cap].gain * Numeriseur[bb].def.gain[vbb];
//		if(VoieManip[voie].Rmodul != 0.0) VoieInfo[voie].I = 1000.0 * rval / VoieManip[voie].Rmodul; /* nA */
//		else VoieInfo[voie].I = 0.0;

		printf("%s/ Elements de calcul de l'impedance du capteur %s:\n",DateHeure(),VoieManip[voie].nom);
		printf("          * Tension mesuree en l'etat\n");
		printf("              Polarisation ADC (V)       : %8.1f\n", ModeleADC[VoieManip[voie].type_adc].polar);
		printf("            / ADU maxi                   : %6d\n",   VoieManip[voie].ADCmask + 1);
		printf("            / gain variable              : %8.1f\n", VoieManip[voie].gain_variable);
		printf("            = Volts/ADU                  : %11.4f\n",VoieManip[voie].ADUenV);
		printf("            x Moyenne du signal(ADU)     : %8.1f\n", VoieInfo[voie].bruit);
		printf("            = Tension mesuree (V)          %10.3f\n",VoieInfo[voie].mesure_enV);
		printf("            x amplitude/crete-crete      : %8.1f\n",COMP_COEF);
		printf("            + Compensation appliquee (V) : %10.3f\n",rval);
		printf("          = Tension totale (V)           : %10.3f\n",VoieInfo[voie].compens);
		printf("            / Gain fixe ADC%d             : %8.1f\n",VoieManip[voie].num_adc+1,VoieManip[voie].gain_fixe);
		printf("          = Tension capteur (mV)         : ........    %10.3f\n",1000.0 * VoieInfo[voie].compens / VoieManip[voie].gain_fixe);
		printf("          * Intensite appliquee\n");
		printf("            Excitation demandee (V)      : %10.3f\n",Numeriseur[bb].ress[k].val.r);
		printf("          / RC[BB] / Cfroide (MO)        : %8.1f\n", VoieManip[voie].Rmodul);
		printf("          = Intensite capteur (nA)       : .......  / %10.3f\n",VoieInfo[voie].I);
		printf("          => Impedance (kO)              :          = %10.3f\n",VoieInfo[voie].R * 1000.0);
	}
	// printf("(%s) VoieInfo[%s].compens = %.3f + %.3f = %.3f\n",__func__,VoieManip[voie].nom,(COMP_COEF * VoieInfo[voie].mesure_enV),rval,VoieInfo[voie].compens);
	NumeriseurAutoriseAcces(&(Numeriseur[bb]),1,0);
	if(ReglVi.actif) ReglageChangeFloat(regl,VoieInfo[voie].compens,0);
	else ReglageChangeFloat(regl,VoieInfo[voie].compens,"");
	NumeriseurAutoriseAcces(&(Numeriseur[bb]),0,0);
	return(0);
}
/* ========================================================================== */
int DetecteurRegenLimitee() {
	int duree,ecoule,secondes;

	if(RegenEnCours) {
		OpiumError("Regeneration deja en cours");
		return(0);
	}
	duree = 30;
	if(OpiumReadInt("Duree (mn)",&duree) == PNL_CANCEL) return(0);
	LectRegenChange();
	secondes = duree * 60;
	OpiumProgresInit(secondes); ecoule= 0;
	printf("          . Attente de %d secondes\n",secondes);
	while(secondes--) {
		SambaAttends(1000);
		if(!OpiumProgresRefresh(++ecoule)) break;
		OpiumUserAction();
	}
	OpiumProgresClose();
	if(secondes > 0) printf("          . Attente reduite a %d secondes\n",ecoule);
	LectRegenChange();
	if(secondes > 0) OpiumWarn("Regeneration interrompue au bout de %.2f/%d minutes",
		(float)ecoule/60.0,duree);
	return(0);
}
/* ========================================================================== */
static Oscillo OscilloCreate(int voie) {
	Oscillo oscillo;

	oscillo = (Oscillo)malloc(sizeof(TypeOscillo));
	if(!oscillo) return(0);
	OscilloInit(oscillo,voie);
	OscilloVide(oscillo);
	oscillo->num = OscilloNb;
	OscilloReglage[OscilloNb++] = oscillo;

	return(oscillo);
}
/* ========================================================================== */
void OscilloInit(Oscillo oscillo, int voie) {
	int bolo,i;
	int echy,min,max;

	if(!oscillo) return;
	oscillo->num = -1;
	oscillo->nbvoies = 1;
	bolo = VoieManip[voie].det;
	oscillo->bolo[0] = bolo; oscillo->voie[0] = voie;
	oscillo->mode = 1;
	oscillo->grille = 2; // 2: si grille mise en fond plutot que devant les traces, sinon: 1
	oscillo->traces = 1; 
	oscillo->moyenne = 0;
	oscillo->ampl = 5000; // 32768 / 5;
	oscillo->zero = 0;
	{
		echy = 5 * oscillo->ampl;
		min = oscillo->zero - echy; if(min < SambaDonneeMin) min = SambaDonneeMin;
		max = oscillo->zero + echy; if(max > SambaDonneeMax) max = SambaDonneeMax;
	}
	oscillo->acq = 0;
	oscillo->ecran = 0;
	oscillo->dim = 0;
	oscillo->defile = 1;
	oscillo->bZero = 0;
	for(i=0; i<MAXREPET+1; i++) oscillo->ab6[i][0] = 0.0;
    oscillo->demodule = (VoieManip[voie].def.trmt[TRMT_AU_VOL].type == TRMT_DEMODUL)? VoieManip[voie].modulee: 0;
    oscillo->trmt_cur = -1;
    OscilloDefiniTemps(oscillo);
	oscillo->marqueX[0] = oscillo->marqueX[1] = oscillo->horloge;
	oscillo->marqueY[0] = (TypeDonnee)min; oscillo->marqueY[1] = (TypeDonnee)max;
	oscillo->marque = -1;
	oscillo->niveauX[0] = oscillo->ab6[0][0]; oscillo->niveauX[1] = oscillo->niveauX[0] + (oscillo->horloge * oscillo->dim);
	oscillo->niveauY[0] = oscillo->niveauY[1] = 0.0;
	oscillo->niveau = -1;
    oscillo->evt = -1;
	oscillo->moy = -1;
	oscillo->centrage = 0;
    oscillo->h.histo = 0; oscillo->h.hd = 0; oscillo->h.graph = 0;
    oscillo->h.min = 0.0; oscillo->h.max = 200.0;
}
/* ========================================================================== */
void OscilloVide(Oscillo oscillo) {
	if(!oscillo) return;
	oscillo->iNb = oscillo->iAcq = oscillo->iAff = 0;
	oscillo->iTemps = oscillo->iAmpl = oscillo->iZero = oscillo->iTrig = 0;
	oscillo->pTemps = oscillo->pAmpl = oscillo->pZero = oscillo->pTrig = 0;
    oscillo->ecran = 0; oscillo->iActif = 0;
	oscillo->marque = oscillo->niveau = -1;
}
/* ========================================================================== 
static char OscilloTrouveMilieu(Menu menu, MenuItem *item) {
	int voie,num; float val;

	for(num=0; num<OscilloNb; num++) if(OscilloReglage[num]->bZero) {
		oscillo = OscilloReglage[num];
		voie = oscillo->voie[0];
		val = VoieInfo[voie].moyenne;
		if(!oscillo->demodule) val /= 2.0;
		oscillo->zero = (int)(val + 0.5);
		PanelRefreshVars(oscillo->pZero);
		InstrumRefreshVar(oscillo->iZero);
		OscilloChangeMinMax(oscillo);
		OscilloRefresh(0,oscillo);
		oscillo->bZero = 0;
	}
	return(0);
}
   ========================================================================== */
MenuItem iOscilloHistoRaz[] = {
	{ "R", MNU_FONCTION OscilloHistoRaz },
	{ "A", MNU_FONCTION OscilloHistoRaz },
	{ "Z", MNU_FONCTION OscilloHistoRaz },
	MNU_END
};
/* ========================================================================== */
static int OscilloGegene(Menu menu, MenuItem *item) {
	TypeDetecteur *detectr; int voie,simu;

	detectr = &(Bolo[BoloNum]);
	voie = detectr->captr[CapteurNum].voie;
	if(voie < 0) OpiumWarn(L_("Pas de voie connectee au capteur #%d"),CapteurNum);
	else if(((simu = VoieManip[voie].simu) < 0) || (simu >= SimuNb)) {
		OpiumWarn(L_("Pas de simulation prevue pour la voie %s"),VoieManip[voie].nom);
	} else GegeneMontre(simu);

	return(0);
}
/* ========================================================================== */
static void OscilloInsere(Oscillo oscillo, Cadre planche, int min, int max,
	int gche_oscillo, int haut_oscillo, char avec_modulation, char avec_trigger) {
	/* Integration d'un oscillo dans le boitier de reglage des detecteurs */
	int x,y,xg,yg,yt,xm,ym,k; int voie;
	Panel panel; Instrum instrum;

    oscillo->avec_trigger = avec_trigger;

    xg = Gliss2lignes.largeur + Gliss2lignes.graduation + ((Gliss2lignes.nbchars + 4) * Dx);
	yg = InstrumRectLngr(&Gliss2lignes) + (2 * Dy);

	x = gche_oscillo; y = haut_oscillo;
	if(avec_trigger) {
        float ymin,ymax;
        Graph g; int larg,haut,nbins;
        HistoData hd; HistoDeVoie vh,prec,svt;
		Panel p; int i; int nbcols_histo;
//		Menu menu;

		Trigger.enservice = 1; Trigger.actif = 0;
		voie = oscillo->voie[0];
		BoardAreaOpen("Trigger",WND_PLAQUETTE);
#define PLACE_EN_HAUT
	#ifdef PLACE_EN_HAUT
		y -= (6 * Dy);
		p = VoieInfo[voie].taux_evts = PanelCreate(4);
		PanelInt  (p,L_("Evenements"),&Acquis[AcquisLocale].etat.evt_trouves);
		PanelFloat(p,L_("Amplitude"),&MonitEvtAmpl); // PanelItemSelect(p,i,0);
		PanelFloat(p,L_("Taux actuel (Hz)"),&LectCntl.EvtTaux); // PanelItemSelect(p,i,0);
		PanelFloat(p,L_("Taux global (Hz)"),&LectCntl.TauxGlobal); // PanelItemSelect(p,i,0);
		PanelMode(p,PNL_DIRECT|PNL_RDONLY); PanelSupport(p,WND_CREUX);
		BoardAddPanel(planche,p,x-Dx,y,0); y += (5 * Dy);
	#endif /* PLACE_EN_HAUT */
		nbcols_histo = 30;
		// menu = MenuLocalise(iOscilloHistoRaz); OpiumSupport(menu->cdr,WND_PLAQUETTE); oscillo->h.cRaz = menu->cdr;
		// BoardAddMenu(planche,menu,(nbcols_histo+4)*Dx,y-(5*Dy),0); y += (2 * Dy);
		oscillo->h.cRaz = BoardAddBouton(planche,"RAZ",OscilloHistoRaz,x+(nbcols_histo-13)*Dx,y,0);  y += (2 * Dy);

        if(oscillo->h.histo) {
            hd = oscillo->h.hd; oscillo->h.hd = 0;
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
            HistoErase(oscillo->h.histo); // libere oscillo->h.hd
            HistoDelete(oscillo->h.histo); // libere oscillo->h.histo
            oscillo->h.histo = 0;
        }
        larg = nbcols_histo * Dx; haut = 14 * Dy;
        nbins = larg / 2;
        oscillo->h.histo = HistoCreateFloat(oscillo->h.min,oscillo->h.max,nbins);
        oscillo->h.hd = HistoAdd(oscillo->h.histo,HST_INT);
        HistoDataName(oscillo->h.hd,"Amplitude");
		if(FondNoir) {
			HistoDataSupportRGB(oscillo->h.hd,WND_Q_ECRAN,GRF_RGB_GREEN);
			HistoDataSupportRGB(oscillo->h.hd,WND_Q_PAPIER,0x0000,0x0000,0xBFFF);
		} else {
			HistoDataSupportRGB(oscillo->h.hd,WND_Q_ECRAN,0x0000,0x0000,0xBFFF);
			HistoDataSupportRGB(oscillo->h.hd,WND_Q_PAPIER,GRF_RGB_GREEN);
		}
        vh = VoieHisto[voie]; prec = 0;
        while(vh) { prec = vh; vh = prec->suivant; }
        vh = (HistoDeVoie)malloc(sizeof(TypeHistoDeVoie));
        if(vh) {
            if(prec == 0) VoieHisto[voie] = vh;
            else prec->suivant = vh;
            vh->hd = oscillo->h.hd;
            vh->Xvar = MONIT_ABS;
			vh->D = 1;
            vh->suivant = 0;
        }
        
        oscillo->h.graph = g = GraphCreate(larg,haut,2);
        HistoGraphAdd(oscillo->h.histo,g);
        GraphAxisDefine(g,0); /* defini deja l'axe des X pour GraphGetFloatRange */
        if(oscillo->h.min <= oscillo->h.max) GraphAxisAutoRange(g,GRF_XAXIS);
        else GraphAxisFloatRange(g,GRF_XAXIS,oscillo->h.min,oscillo->h.max);
        if(MonitChgtUnite) GraphAxisChgtUnite(g,GRF_XAXIS,MonitUnitesADU);
        GraphAxisAutoRange(g,GRF_YAXIS);
        // GraphDataTrace(g,1,GRF_PNT,0,;
        GraphUse(g,nbins);
        OpiumSupport(g->cdr,WND_CREUX);
        GraphMode(g,GRF_2AXES);
 		OpiumName(g->cdr,"Histo ampl.");
		BoardAddGraph(planche,g,x,y,0);
        y += (haut + Dy);
        
		p = oscillo->h.pAmpl = PanelCreate(2); PanelColumns(p,2);
		oscillo->h.vMin = i = PanelFloat(p,"min",&oscillo->h.min); PanelFormat(p,i,"%5.1f"); PanelLngr(p,i,7);
        PanelItemExit(p,i,OscilloHistoChangepMin,(void *)oscillo);
        oscillo->h.vMax = i = PanelFloat(p,"max",&oscillo->h.max); PanelFormat(p,i,"%5.1f"); PanelLngr(p,i,7);
        PanelItemExit(p,i,OscilloHistoChangepMax,(void *)oscillo);
        PanelMode(p,PNL_DIRECT); PanelSupport(p,WND_CREUX);
        i = (larg - ((2 * (1+3+1+7)) * Dx)) / 2;
        BoardAddPanel(planche,p,x+i,y,0);
        y += (2 * Dy);

	#ifdef CURSEURS_MINMAX
		InstrumRectDim(&ReglageGlissMinMax,larg,15);
		// instrum = oscillo->h.iMin = InstrumFloat(INS_GLISSIERE,(void *)&ReglageGlissMinMax,&oscillo->h.min,0.0,oscillo->h.max);
		instrum = oscillo->h.iMin = InstrumFloat(INS_GLISSIERE,(void *)&ReglageGlissMinMax,&oscillo->h.min,1.0,65535.0);
		InstrumGradLog(instrum,1);
		InstrumOnChange(instrum,OscilloHistoChangeiMin,(void *)oscillo);
		BoardAddInstrum(planche,instrum,x,y,0); y += (3 * Dy);
		// instrum = oscillo->h.iMax = InstrumFloat(INS_GLISSIERE,(void *)&ReglageGlissMinMax,&oscillo->h.max,oscillo->h.min,65535.0);
		instrum = oscillo->h.iMax = InstrumFloat(INS_GLISSIERE,(void *)&ReglageGlissMinMax,&oscillo->h.max,1.0,65535.0);
		InstrumGradLog(instrum,1);
		InstrumOnChange(instrum,OscilloHistoChangeiMax,(void *)oscillo);
		BoardAddInstrum(planche,instrum,x,y,0); y += (3 * Dy);
		instrum = InstrumInt(INS_GLISSIERE,(void *)&ReglageGlissMinMax,&oscillo->centrage,-50,50);
		InstrumTitle(instrum,L_("Centrage trace"));
		BoardAddInstrum(planche,instrum,x,y,0);
	#endif

	#ifdef PLACE_EN_BAS
		p = VoieInfo[voie].taux_evts = PanelCreate(4);
        PanelInt  (p,L_("Evenements"),&Acquis[AcquisLocale].etat.evt_trouves);
        PanelFloat(p,L_("Amplitude"),&MonitEvtAmpl); // PanelItemSelect(p,i,0);
        PanelFloat(p,L_("Taux actuel (Hz)"),&LectCntl.EvtTaux); // PanelItemSelect(p,i,0);
        PanelFloat(p,L_("Taux global (Hz)"),&LectCntl.TauxGlobal); // PanelItemSelect(p,i,0);
        PanelMode(p,PNL_DIRECT|PNL_RDONLY); PanelSupport(p,WND_CREUX);
        BoardAddPanel(planche,p,x-Dx,y,0); // x+(10*Dx): alignement a gauche avec la valeur du seuil
		oscillo->h.cRaz = BoardAddBouton(planche,"RAZ",OscilloHistoRaz,33*Dx,y+Dy,0); // 19*Dx,y+(5*Dy)
	#endif /* PLACE_EN_BAS */

		x += (larg + (2 * Dx)); y = haut_oscillo;
        ym = 4 * Dy / 3; // GraphAxisMarge(g,GRF_YAXIS); exact, mais pas encore defini, seulement si trace
        yt = y - (Dy / 2) + ym;
        // printf("(%s) marge[%d]=%d\n",__func__,GRF_YAXIS,ym);
		Glissiere.longueur = OscilloHaut - (2 * ym);
        ymin = (float)oscillo->marqueY[0]; ymax = (float)oscillo->marqueY[1];
		instrum = InstrumFloat(INS_GLISSIERE,(void *)&Glissiere,&(VoieInfo[voie].trigger),ymin,ymax);
		InstrumOnChange(instrum,OscilloChangeiTrig,(void *)oscillo);
		oscillo->iTrig = instrum;
		BoardAddInstrum(planche,instrum,x,yt,0); yt += (Glissiere.longueur + Dy);

		PanelMode(panel = PanelCreate(1),PNL_DIRECT); PanelSupport(panel,WND_CREUX);
		k = PanelFloat(panel,0,&(VoieInfo[voie].trigger));
		PanelLngr(panel,k,7);
		PanelItemExit(panel,k,OscilloChangepTrig,(void *)oscillo);
		oscillo->pTrig = panel;
		BoardAddPanel(planche,panel,x-Dx,yt,0); yt += (3 * Dy);

		// instrum = InstrumKeyB(INS_GLISSIERE,(void *)&Gliss2lignes,&VoieInfo[voie].trig_actif,L_("arret/actif")); si bouton individuel
		// InstrumOnChange(instrum,OscilloChangeTrigActif,(void *)oscillo); si bouton individuel
		oscillo->iActif = instrum = InstrumKeyB(INS_GLISSIERE,(void *)&Gliss2lignes,&Trigger.actif,L_("arret/actif")); // si bouton global
		InstrumTitle(instrum,L_("Trigger"));
		OpiumSupport(instrum->cdr,WND_RAINURES);
		BoardAddInstrum(planche,instrum,x,yt,0); yt += (3 * Dy);
		// x += (Glissiere.largeur + Glissiere.graduation + ((Glissiere.nbchars + 2) * Dx));

		PanelMode(panel = PanelCreate(1),PNL_RDONLY|PNL_DIRECT); PanelSupport(panel,WND_CREUX);
		PanelLngr(panel,PanelFloat(panel,"ampl",&(VoieInfo[voie].deltaADU)),7);
		oscillo->pDelta = panel;
		BoardAddPanel(planche,panel,x-(2*Dx),yt,0); yt += (2 * Dy);

		BoardAreaClose(planche,&OscilloCadreTrgr,&x,&ym); x += (2 * Dx);
		gche_oscillo = x;
	} else { Trigger.enservice = 0; Trigger.actif = 0; }

	BoardAreaOpen("Oscilloscope",WND_PLAQUETTE);

	oscillo->ecran = GraphCreate(OscilloLarg,OscilloHaut,2 * (MAXREPET + 1) + 6);
	OpiumSupport((oscillo->ecran)->cdr,WND_CREUX);
	OpiumName((oscillo->ecran)->cdr,"Oscillo");
	if(oscillo->grille == 2) GraphMode(oscillo->ecran,GRF_2AXES|GRF_GRID);
	else if(oscillo->grille == 1) GraphMode(oscillo->ecran,GRF_2AXES);
	else GraphMode(oscillo->ecran,GRF_0AXES|GRF_NOGRAD);
	BoardAddGraph(planche,oscillo->ecran,x,y,0);
	
	y += (OscilloHaut + Dy);

#ifdef REDONDANT
	instrum = InstrumKeyB(INS_GLISSIERE,(void *)&Gliss2lignes,&oscillo->acq,L_("arret/lecture"));
	InstrumOnChange(instrum,OscilloChangeAcq,(void *)oscillo);
	OpiumSupport(instrum->cdr,WND_RAINURES);
	// InstrumTitle(iOscilloAcq,"Acquisition");
	oscillo->iAcq = instrum;
	BoardAddInstrum(planche,instrum,x,y,0);
	yt = y + yg;
	if(avec_modulation) {
		instrum = InstrumKeyB(INS_GLISSIERE,(void *)&Gliss2lignes,&(oscillo->demodule),L_("brut/demodule"));
		// InstrumTitle(instrum,"Demodulation");
		InstrumOnChange(instrum,OscilloChangeModul,(void *)oscillo);
		OpiumSupport(instrum->cdr,WND_RAINURES);
		oscillo->iDemod = instrum;
		BoardAddInstrum(planche,instrum,x,yt,0); yt += yg;
	} else oscillo->iDemod = 0;
#else
	yt = y;
#endif

	instrum = InstrumInt(INS_POTAR,(void *)&Vernier,&oscillo->traces,1,MAXREPET);
	InstrumTitle(instrum,L_("# Traces"));
	InstrumOnChange(instrum,OscilloChangeVoie,(void *)oscillo);
	BoardAddInstrum(planche,instrum,x,y,0);
	x += ((Vernier.rayon + Vernier.graduation + Dx) * 2) + (2 * Dx);
	
	instrum = InstrumKeyB(INS_GLISSIERE,(void *)&Gliss3lignes,&oscillo->mode,L_("moyenne/traces/tout"));
	InstrumTitle(instrum,L_("Type trace"));
	InstrumOnChange(instrum,OscilloChangeVoie,(void *)oscillo);
	OpiumSupport(instrum->cdr,WND_RAINURES);
	oscillo->iNb = instrum;
	BoardAddInstrum(planche,instrum,x,y,0);
	x += Gliss3lignes.largeur + Gliss3lignes.graduation + ((Gliss3lignes.nbchars + 2) * Dx);

	instrum = InstrumKeyB(INS_GLISSIERE,(void *)&Gliss3lignes,&oscillo->grille,L_("masquee/axes/affichee"));
	InstrumTitle(instrum,L_("Grille"));
	InstrumOnChange(instrum,OscilloRefresh,(void *)oscillo);
	OpiumSupport(instrum->cdr,WND_RAINURES);
	BoardAddInstrum(planche,instrum,x,y,0);
	x += Gliss3lignes.largeur + Gliss3lignes.graduation + ((Gliss3lignes.nbchars + 2) * Dx);

	InstrumRectDim(&ReglageGlissTemps,OscilloLarg-(x-gche_oscillo),Dy); // change pour toutes les fenetres de reglage...
	PanelMode(panel = PanelCreate(1),PNL_DIRECT);
	k = PanelFloat(panel,0,&oscillo->temps);
	// insuffisant: PanelItemExit(panel,k,SambaUpdateInstrumFromItem,instrum);
	PanelItemExit(panel,k,OscilloChangepTemps,(void *)oscillo);
	PanelItemRLimits(panel,1,0.01,1000.0);
	PanelLngr(panel,k,10);
	PanelSupport(panel,WND_CREUX);
	oscillo->pTemps = panel;
	BoardAddPanel(planche,panel,x+((InstrumRectLngr(&ReglageGlissTemps) - (10*Dx))/2),y,0);

	instrum = InstrumFloat(INS_GLISSIERE,(void *)&ReglageGlissTemps,&oscillo->temps,0.01,1000.0);
	InstrumGradLog(instrum,1);
	InstrumTitle(instrum,"ms/div");
	InstrumOnChange(instrum,OscilloChangeiTemps,(void *)oscillo);
	oscillo->iTemps = instrum;
	BoardAddInstrum(planche,instrum,x,y + (2 * Dy),0);
	x += InstrumRectLngr(&ReglageGlissTemps) + (3 * Dx);
	
	instrum = InstrumFloat(INS_GLISSIERE,(void *)&Gliss4lignes,&MonitIntervSecs,0.05,2.0);
	InstrumGradSet(instrum,1.0);
	InstrumOnChange(instrum,OscilloChangeRefresh,(void *)oscillo);
	OpiumSupport(instrum->cdr,WND_RAINURES);
	oscillo->iAff = instrum;
	BoardAddInstrum(planche,instrum,x,y,0);
	
	instrum = InstrumKeyB(INS_GLISSIERE,(void *)&Gliss2lignes,&oscillo->defile,L_("fige/defile"));
	OpiumSupport(instrum->cdr,WND_RAINURES);
	BoardAddInstrum(planche,instrum,x,y+InstrumRectLngr(&Gliss4lignes) + (2 * Dy),0);
	
	x = gche_oscillo + OscilloLarg + (2 * Dx); y = haut_oscillo;

	instrum = InstrumInt(INS_GLISSIERE,(void *)&Gliss6lignes,&oscillo->ampl,2,10000); // (max + 1) / 5);
//	instrum = InstrumInt(INS_POTAR,(void *)&Vernier,&oscillo->ampl,1,10000);
	InstrumGradLog(instrum,-1);
	InstrumTitle(instrum,"ADU/div");
	InstrumOnChange(instrum,OscilloChangeiAmpl,(void *)oscillo);
	oscillo->iAmpl = instrum;
	BoardAddInstrum(planche,instrum,x,y,0);
	
	PanelMode(panel = PanelCreate(1),PNL_DIRECT);
	k = PanelInt(panel,0,&oscillo->ampl);
//	PanelItemExit(panel,k,SambaUpdateInstrumFromItem,instrum); // insuffisant
	PanelItemExit(panel,k,OscilloChangepAmpl,(void *)oscillo);
	PanelItemILimits(panel,k,2,10000); // (int)((max + 1) / 5));
	//	PanelFormat(panel,k,"%5d");
	PanelLngr(panel,k,7);
	PanelSupport(panel,WND_CREUX);
	oscillo->pAmpl = panel;
	BoardAddPanel(planche,panel,x,y+InstrumRectLngr(&Gliss6lignes)+3*Dy,0);

//	x += Gliss6lignes.largeur + Gliss6lignes.graduation + ((Gliss6lignes.nbchars + 2) * Dx);
	y += InstrumRectLngr(&Gliss6lignes) + (6 * Dy);
	
	instrum = InstrumInt(INS_GLISSIERE,(void *)&Gliss6lignes,&oscillo->zero,min,max);
	//	InstrumGradSet(instrum,(float)((int)(max / 4)));
	InstrumTitle(instrum,L_("Milieu ecran"));
	InstrumOnChange(instrum,OscilloChangeiZero,(void *)oscillo);
	oscillo->iZero = instrum;
	BoardAddInstrum(planche,instrum,x,y,0);
	
	PanelMode(panel = PanelCreate(1),PNL_DIRECT);
	k = PanelInt(panel,0,&oscillo->zero);
	// insuffisant: PanelItemExit(panel,k,SambaUpdateInstrumFromItem,instrum);
	PanelItemExit(panel,k,OscilloChangepZero,(void *)oscillo);
	PanelItemILimits(panel,k,min,max);
	PanelLngr(panel,k,7);
	PanelSupport(panel,WND_CREUX);
	oscillo->pZero = panel;
	BoardAddPanel(planche,panel,x,y+InstrumRectLngr(&Gliss6lignes)+3*Dy,0);
//	BoardAddBouton(planche,"-",OscilloTrouveMilieu,OPIUM_A_DROITE_DE 0);
	BoardAddPoussoir(planche,"-",&(oscillo->bZero),OPIUM_A_DROITE_DE 0);

	BoardAreaClose(planche,&OscilloCadreMain,&xm,&ym);

}
/* ========================================================================== */
INLINE char OscilloAffiche(Oscillo oscillo) {
	return(oscillo && oscillo->ecran && OpiumDisplayed((oscillo->ecran)->cdr));
}
/* ========================================================================== */
void OscilloConnecte(Oscillo oscillo) {
	int i,y; int v,voie; char modulee;
	
	if(!oscillo->ecran) return;
	if(!OpiumDisplayed((oscillo->ecran)->cdr)) return; // seulement dans lect?
	voie = 0;
	GraphErase(oscillo->ecran);
	modulee = 0;
	for(v=0; v<oscillo->nbvoies; v++) {
		voie = oscillo->voie[v];
		if(VoieManip[voie].modulee) modulee = 1;
		/* VoieTampon[voie].brutes->max sur X aussi pour le rescroll */
		GraphAdd(oscillo->ecran,GRF_FLOAT|GRF_INDEX|GRF_XAXIS,oscillo->ab6[0],oscillo->dim);
		// printf("(%s) axe X: %d points par pas de %g, de %g a %g\n",__func__,oscillo->dim,oscillo->ab6[0][1],oscillo->ab6[0][0],oscillo->ab6[0][0]+(oscillo->ab6[0][1] * (float)oscillo->dim));
		if(oscillo->mode > 0) for(i=0; i<oscillo->traces; i++) {
			// printf("Affiche VoieTampon[%d:%s].tampon @%08X pour %d donnees\n",voie,VoieManip[voie].nom,VoieTampon[voie].brutes->t.sval,VoieTampon[voie].brutes->max);
			y = GraphAdd(oscillo->ecran,GRF_SHORT|GRF_YAXIS,VoieTampon[voie].brutes->t.sval,VoieTampon[voie].brutes->max);
			if(i == 0) GraphDataRGB(oscillo->ecran,y,GRF_RGB_GREEN);
			else GraphDataRGB(oscillo->ecran,y,GRF_RGB_YELLOW);
		}
	}
	if((oscillo->mode != 1) && (oscillo->traces > 1) && (oscillo->dim > 0) && oscillo->moyenne) {
		//- i = (oscillo->mode > 0)? oscillo->traces: 0;
		oscillo->moy = GraphAdd(oscillo->ecran,GRF_FLOAT|GRF_YAXIS,oscillo->moyenne,oscillo->dim);
		GraphDataRGB(oscillo->ecran,oscillo->moy,GRF_RGB_RED);
	}
	if(oscillo->avec_trigger) {
		oscillo->evt = GraphAdd(oscillo->ecran,GRF_SHORT|GRF_YAXIS,VoieTampon[voie].brutes->t.sval,VoieTampon[voie].brutes->max);
		GraphDataRGB(oscillo->ecran,oscillo->evt,GRF_RGB_ORANGE);
		GraphAdd(oscillo->ecran,GRF_FLOAT|GRF_XAXIS,oscillo->niveauX,2);
		oscillo->niveau = GraphAdd(oscillo->ecran,GRF_FLOAT|GRF_YAXIS,oscillo->niveauY,2);
		GraphDataRGB(oscillo->ecran,oscillo->niveau,GRF_RGB_VIOLET);
		GraphDataTrace(oscillo->ecran,oscillo->niveau,GRF_LIN,GRF_TOUT);
	}
	if(modulee && (oscillo->demodule == NEANT)) {
		GraphAdd(oscillo->ecran,GRF_FLOAT|GRF_XAXIS,oscillo->marqueX,2);
		oscillo->marque = GraphAdd(oscillo->ecran,GRF_SHORT|GRF_YAXIS,oscillo->marqueY,2);
		GraphDataRGB(oscillo->ecran,oscillo->marque,GRF_RGB_BLUE);
		GraphDataTrace(oscillo->ecran,oscillo->marque,GRF_LIN,GRF_TOUT);
	}

	if(oscillo->grille == 2) GraphMode(oscillo->ecran,GRF_2AXES|GRF_GRID);
	else if(oscillo->grille == 1) GraphMode(oscillo->ecran,GRF_2AXES);
	else GraphMode(oscillo->ecran,GRF_0AXES|GRF_NOGRAD);
	GraphAxisFloatGrad(oscillo->ecran,GRF_XAXIS,oscillo->temps);
	OpiumName((oscillo->ecran)->cdr,VoieManip[oscillo->voie[0]].nom);
	OscilloChangeMinMax(oscillo);
	GraphParmsSave(oscillo->ecran);
}
/* ========================================================================== */
static int OscilloChangeRefresh() {
	int num; Instrum instrum;
	
	MonitIntervAff = (int)(MonitIntervSecs * 1000.0);
	LectIntervData = (int64)MonitIntervAff * SynchroD2_kHz;
	for(num=0; num<OscilloNb; num++) {
		instrum = OscilloReglage[num]->iAff;
		if(!instrum) continue;
		if(OpiumDisplayed(instrum->cdr)) InstrumRefreshVar(instrum);
	}
	return(0);
}
/* ========================================================================== */
void OscilloChangeTemps(Oscillo oscillo) {
	float largeur;

	largeur = oscillo->temps * 10.0;
	oscillo->dim = (int)(largeur / oscillo->horloge);
	if(oscillo->traces > 1) {
		if(oscillo->moyenne) free(oscillo->moyenne);
		oscillo->moyenne = (float *)malloc(oscillo->dim * sizeof(float));
	}
	
	oscillo->ab6[0][0] = 0.0;
	oscillo->ab6[0][1] = oscillo->horloge;
	oscillo->niveauX[0] = oscillo->ab6[0][0]; oscillo->niveauX[1] = oscillo->niveauX[0] + largeur;
	if(Acquis[AcquisLocale].etat.active) OscilloConnecte(oscillo);

	return;
}
/* ========================================================================== */
void OscilloChangeMinMax(Oscillo oscillo) {
	int echy,min,max;
	
	echy = 5 * oscillo->ampl;
	min = oscillo->zero - echy; if(min < SambaDonneeMin) min = SambaDonneeMin;
	max = oscillo->zero + echy; if(max > SambaDonneeMax) max = SambaDonneeMax;
	oscillo->marqueY[0] = (TypeDonnee)min; oscillo->marqueY[1] = (TypeDonnee)max;
	if(oscillo->ecran) {
		GraphAxisIntRange(oscillo->ecran,GRF_YAXIS,min,max);
		GraphAxisIntGrad(oscillo->ecran,GRF_YAXIS,(int)oscillo->ampl);
	}
	if(oscillo->iTrig) {
		InstrumILimits(oscillo->iTrig,min,max);
		if(OpiumAlEcran(oscillo->iTrig)) OpiumRefresh((oscillo->iTrig)->cdr);
	}
	return;
}
/* ========================================================================== */
int64 OscilloCourbesDebut(Oscillo oscillo, int *recul) {
	int voie,bolo; int64 debut,periode,d3_avant,duree_d3;

	voie = oscillo->voie[0]; bolo = VoieManip[voie].det; // GCC
	*recul = oscillo->dim; debut = VoieTampon[voie].lus - (int64)oscillo->dim;
	// printf("(%s) VoieManip[%s].det=%s, d2=%d\n",__func__,VoieManip[voie].nom,Bolo[bolo].nom,Bolo[bolo].d2);
	if(VoieManip[voie].modulee && (oscillo->demodule == NEANT) && ((bolo = VoieManip[voie].det) >= 0)) {
		/* Bolo[bolo].d2<=0 si appel lors de l'initialisation */
		if((periode = Bolo[bolo].d2) > 0) {
			*recul = (int)((((oscillo->dim - 1) / periode) + 1) * periode); /* recul pour superposition de traces */
			d3_avant = LectDerniereD3;
			debut = debut - d3_avant;
			duree_d3 = Bolo[bolo].d2 * Bolo[bolo].d3;
			if(duree_d3 > 0) while(debut < 0) {
				d3_avant -= duree_d3;
				debut += d3_avant;
			};
			debut = ((debut / periode) * periode) + d3_avant;         /* Synchro affichage sur modulation   */
		}
	}
	if(debut < 0) debut = 0;
	// printf("(%s) VoieManip[%s].det=%s, d2=%d, recul=%d, d3@%lld, debut=%lld\n",__func__,VoieManip[voie].nom,Bolo[bolo].nom,Bolo[bolo].d2,*recul,LectDerniereD3,debut);
	
	return(debut);
}
/* ========================================================================== */
void OscilloCourbesTrace(Oscillo oscillo, int64 debut, int recul) {
    char tracer_moyenne,evt_affiche;
	int voie,point1,nbpts; int i,j,k,l,n;
	int64 premier,tour,of7,pmax;
	float t0,t1; //,ta,dt;
	double val;
	TypeDonnee *adrs;
	
	if(debut < 0) return;
	voie = oscillo->voie[0];
	if((VoieTampon[voie].brutes->t.sval == 0) || (oscillo->dim <= 0)) return;
	tour = VoieTampon[voie].brutes->max;
	of7 = (VoieTampon[voie].trmt[TRMT_AU_VOL].plus_ancien / tour) * tour;
	t0 = (float)(debut - of7) * VoieEvent[voie].horloge;
	t1 = t0 + ((float)oscillo->dim * VoieEvent[voie].horloge);
	oscillo->ab6[0][0] = t0;
	// printf("(%s) Tracer avec le debut a %lld, tampon[%d], decalage=%lld soit t0 a %lld x %g = %g\n",__func__,debut,VoieTampon[voie].brutes->max,of7,debut - of7,VoieEvent[voie].horloge,t0);
	// printf("(%s) VoieTampon[%s].brutes->t.sval=%08X, oscillo->dim=%d, debut=%lld, t0=%g\n",__func__,VoieManip[voie].nom,(hexa)VoieTampon[voie].brutes->t.sval,oscillo->dim,debut,t0);

	evt_affiche = 0;
	if(oscillo->avec_trigger) {
		// printf("(%s) regarde evt voie %d: #%d\n",__func__,voie,VoieTampon[voie].dernier_evt);
		if(Trigger.actif && ((n = VoieTampon[voie].dernier_evt) >= 0)) {
			if(n < EvtASauver) {
				pmax = (((int64)Evt[n].gigastamp * 1000000000) + (int64)Evt[n].stamp - LectTimeStamp0) / (int64)VoieTampon[voie].trmt[TRMT_AU_VOL].compactage;
				// pmax -= (recul / 2); // pour etre toujpurs au milieu
				pmax -= (int64)((oscillo->centrage + 50) * recul / 100); if(pmax < 0) pmax = 0;
				if((pmax + oscillo->dim) > VoieTampon[voie].lus) pmax = (int)(VoieTampon[voie].lus - oscillo->dim);
				if(pmax >= VoieTampon[voie].trmt[TRMT_AU_VOL].plus_ancien) {
					point1 = Modulo(pmax,VoieTampon[voie].brutes->max);
					GraphDataRescroll(oscillo->ecran,oscillo->evt,point1);
					if(n != VoieTampon[voie].evt_affiche) {
						GraphDataUse(oscillo->ecran,oscillo->evt,oscillo->dim);
						GraphDataRGB(oscillo->ecran,oscillo->evt,GRF_RGB_ORANGE);
						VoieTampon[voie].evt_affiche = n;
					} else GraphDataRGB(oscillo->ecran,oscillo->evt,GRF_RGB_VIOLET);
					evt_affiche = 1;
				}
			}
		}
		if(!evt_affiche) {
			GraphDataUse(oscillo->ecran,oscillo->evt,0);
			VoieTampon[voie].evt_affiche = -1;
		}
		if(oscillo->niveau >= 0) {
			oscillo->niveauX[0] = t0; oscillo->niveauX[1] = t1; // oscillo->niveauX[0] + (oscillo->temps * 10.0);
			//1 dim = VoieInfo[voie].trig_actif? 2: 0; bouton individuel
			//2 dim = Trigger.actif? 2: 0; GraphDataUse(oscillo->ecran,oscillo->niveau,dim); // bouton global
			if(Trigger.actif) GraphDataRGB(oscillo->ecran,oscillo->niveau,GRF_RGB_VIOLET);
			else {
				if(FondNoir) GraphDataRGB(oscillo->ecran,oscillo->niveau,GRF_RGB_YELLOW);
				else GraphDataRGB(oscillo->ecran,oscillo->niveau,MARRON);
			}
		}
	}

	tracer_moyenne = ((oscillo->mode != 1) && (oscillo->traces > 1) && (oscillo->dim > 0));
	//?? j'ai du aller trop vite...:	nbpts = (oscillo->dim < oscillo->dim)? oscillo->dim: oscillo->dim;
	//?? texte original: nbpts = (OscilloLngr < OscilloDim)? OscilloLngr: OscilloDim; avec OscilloLngr=maxi alloue jusqu'ici (usage: alloc de .moyenne)
	nbpts = oscillo->dim;
	// a ne pas commettre:
	// point1 = Modulo(debut,VoieTampon[voie].brutes->max);
	// printf("(%s) | rescroll(%d)\n",__func__,point1); GraphDataRescroll(oscillo->ecran,0,point1);
	GraphDataUse(oscillo->ecran,0,oscillo->dim);
	j = 1; premier = debut;
	for(i=0; i<oscillo->traces; i++) {
		point1 = Modulo(premier,VoieTampon[voie].brutes->max);
		if(tracer_moyenne && oscillo->moyenne) {
			adrs = VoieTampon[voie].brutes->t.sval + point1;
			for(k=0, l=point1; k<nbpts; k++) {
				val = (double)*adrs++; l++;
				if(i == 0) oscillo->moyenne[k] = (float)val;
				else oscillo->moyenne[k] += (float)val;
				if(l == VoieTampon[voie].brutes->max) { l = 0; adrs = VoieTampon[voie].brutes->t.sval; }
			}
		}
		if(oscillo->mode > 0) {
			GraphDataRescroll(oscillo->ecran,j,point1); GraphDataUse(oscillo->ecran,j,oscillo->dim); j++;
		}
		premier -= recul;
		if(premier < 0) break;
	}
	if(tracer_moyenne && oscillo->moyenne) {
		for(k=0; k<nbpts; k++) oscillo->moyenne[k] /= (float)oscillo->traces;
		GraphDataUse(oscillo->ecran,oscillo->moy,nbpts); j++;
		// if(oscillo->mode <= 0) GraphAxisReset(oscillo->ecran,GRF_XAXIS); inclus dans GraphUse
	}
	if(VoieManip[voie].modulee && (oscillo->demodule == NEANT) && (oscillo->marque >= 0)) {
		oscillo->marqueX[0] = oscillo->marqueX[1] = t0 + ((float)VoieManip[voie].def.trmt[TRMT_AU_VOL].parm * oscillo->horloge);
		GraphDataUse(oscillo->ecran,oscillo->marque,2);
	} // else GraphDataUse(oscillo->ecran,oscillo->marque,0);
	GraphAxisReset(oscillo->ecran,GRF_XAXIS);

#ifndef WXWIDGETS
	OpiumRefresh((oscillo->ecran)->cdr);
#else
	// Only tell the windows to refresh rather than actually calling graphics commands
	OpiumRefreshAllWindows();
#endif
}
/* ========================================================================== */
int OscilloRefresh(Instrum instrum, Oscillo oscillo) {
	int64 debut; int recul;
	
	if(oscillo->grille == 2) GraphMode(oscillo->ecran,GRF_2AXES|GRF_GRID);
	else if(oscillo->grille == 1) GraphMode(oscillo->ecran,GRF_2AXES);
	else GraphMode(oscillo->ecran,GRF_0AXES|GRF_NOGRAD);
	debut = OscilloCourbesDebut(oscillo,&recul);
//	AutomDataGet();
	ReglTbolo = *T_Bolo;
	EtatTubes = *PulseTubes;
	if(oscillo->defile) OscilloCourbesTrace(oscillo,debut,recul);
	else OpiumRefresh((oscillo->ecran)->cdr);
	return(0);
}
/* ========================================================================== */
static void OscilloDefiniTemps(Oscillo oscillo) {
    if(oscillo->trmt_cur != oscillo->demodule) {
        if(oscillo->demodule) {
            int bolo;
            bolo = VoieManip[oscillo->voie[0]].det;
            oscillo->horloge = (float)Bolo[bolo].d2 / Echantillonnage;
        } else oscillo->horloge = 1.0 / Echantillonnage;
        oscillo->temps = 50.0 * oscillo->horloge;  /* il y a dix de ces graduations... */
        oscillo->trmt_cur = oscillo->demodule;
    }
    return;
}
/* ========================================================================== */
static int OscilloChangeVoie(Instrum instrum, Oscillo oscillo) {
	int bolo,cap,bb;

//--	printf("(%s) Changement de voie pour l'oscillo #%d (voie %s)\n",__func__,oscillo->num,VoieManip[oscillo->voie[0]].nom);
	if((oscillo->traces < 2) || (oscillo->dim <= 0)) {
		oscillo->mode = 1;
		if(oscillo->iNb) {
			if(OpiumDisplayed((oscillo->iNb)->cdr)) OpiumRefresh((oscillo->iNb)->cdr);
		}
	}
	bolo = VoieManip[oscillo->voie[0]].det;
	cap = VoieManip[oscillo->voie[0]].cap;
	if((bb = Bolo[bolo].captr[cap].bb.num) >= 0) Numeriseur[bb].status.a_copier = 1;
	PanelRefreshVars(oscillo->pTemps);
	InstrumRefreshVar(oscillo->iTemps);
	OscilloChangeTemps(oscillo); /* car les axes redeviennent indefinis */
	OscilloChangeMinMax(oscillo);
	OscilloRefresh(instrum,oscillo);
	return(0);
}
/* ========================================================================== */
int OscilloChangeiAmpl(Instrum instrum, Oscillo oscillo) {

#ifdef AMPLITUDE_LINEAIRE
	int quanta; int nb;
	quanta = (int)OpiumGradsLineaires((double)((SambaDonneeMax + 1) / 20));
	nb = oscillo->ampl / quanta;
	oscillo->ampl = nb * quanta;
#endif
	PanelRefreshVars(oscillo->pAmpl);
	OscilloChangeMinMax(oscillo);
	OscilloRefresh(instrum,oscillo);
	return(0);
}
/* ========================================================================== */
int OscilloChangepAmpl(Panel panel, int item, void *arg) {
	Oscillo oscillo;

	oscillo = (Oscillo)arg;
	InstrumRefreshVar(oscillo->iAmpl);
	OscilloChangeMinMax(oscillo);
	OscilloRefresh(0,oscillo);

	return(0);
}
/* ========================================================================== */
int OscilloChangeiZero(Instrum instrum, Oscillo oscillo) {
	PanelRefreshVars(oscillo->pZero);
	OscilloChangeMinMax(oscillo);
	OscilloRefresh(instrum,oscillo);
	return(0);
}
/* ========================================================================== */
int OscilloChangepZero(Panel panel, int item, void *arg) {
	Oscillo oscillo;

	oscillo = (Oscillo)arg;
	InstrumRefreshVar(oscillo->iZero);
	OscilloChangeMinMax(oscillo);
	OscilloRefresh(0,oscillo);

	return(0);
}
/* ========================================================================== */
int OscilloChangeiTemps(Instrum instrum, Oscillo oscillo) {
	PanelRefreshVars(oscillo->pTemps);
	OscilloChangeTemps(oscillo);
	OscilloRefresh(instrum,oscillo);
	return(0);
}
/* ========================================================================== */
int OscilloChangepTemps(Panel panel, int item, void *arg) {
	Oscillo oscillo;

	oscillo = (Oscillo)arg;
	InstrumRefreshVar(oscillo->iTemps);
	OscilloChangeTemps(oscillo);
	OscilloRefresh(0,oscillo);

	return(0);
}
/* ========================================================================== */
static void OscilloChangeTrigActif(Oscillo oscillo) {
	int voie;
    
    voie = oscillo->voie[0]; oscillo->niveauY[0] = oscillo->niveauY[1] = VoieInfo[voie].trigger;
    if(VoieTampon[voie].trig.trgr) {
        VoieTampon[voie].trig.trgr->minampl = VoieTampon[voie].trig.trgr->maxampl = VoieInfo[voie].trigger;
        VoieTampon[voie].trig.trgr->sens = (VoieInfo[voie].trigger >= VoieInfo[voie].moyenne)? 2: 0;
        VoieTampon[voie].trig.signe = VoieTampon[voie].trig.trgr->sens - 1;
    }
    OscilloRefresh(0,oscillo);
}
/* ========================================================================== */
static int OscilloChangeiTrig(Instrum instrum, Oscillo oscillo) {
	PanelRefreshVars(oscillo->pTrig);
    OscilloChangeTrigActif(oscillo);
	return(0);
}
/* ========================================================================== */
static int OscilloChangepTrig(Panel panel, int item, void *arg) {
	Oscillo oscillo = (Oscillo)arg;
	InstrumRefreshVar(oscillo->iTrig);
    OscilloChangeTrigActif(oscillo);
	return(0);
}
/* ========================================================================== */
static int OscilloHistoChangepMin(Panel panel, int item, void *arg) {
	return(OscilloHistoChangeiMin(0,(Oscillo)arg));
}
/* ========================================================================== */
static int OscilloHistoChangepMax(Panel panel, int item, void *arg) {
	return(OscilloHistoChangeiMax(0,(Oscillo)arg));
}
/* ========================================================================== */
static int OscilloHistoChangeiMin(Instrum instrum, Oscillo oscillo) {
	if(oscillo->h.min > oscillo->h.max) oscillo->h.max = oscillo->h.min + 1.0;
	OscilloHistoChangeMinMax(oscillo);
	return(0);
}
/* ========================================================================== */
static int OscilloHistoChangeiMax(Instrum instrum, Oscillo oscillo) {
	if(oscillo->h.max < oscillo->h.min) oscillo->h.min = oscillo->h.max - 1.0;
	OscilloHistoChangeMinMax(oscillo);
	return(0);
}
/* ========================================================================== */
static void OscilloHistoChangeMinMax(Oscillo oscillo) {
	Graph g;

	HistoRaz(oscillo->h.histo);
	HistoLimitsFloat(oscillo->h.histo,oscillo->h.min,oscillo->h.max);
	g = oscillo->h.graph;
	//++ if(oscillo->h.min == oscillo->h.max) GraphAxisAutoRange(g,GRF_XAXIS); // pas la bonne fonction
	GraphAxisFloatRange(g,GRF_XAXIS,oscillo->h.min,oscillo->h.max);
	GraphAxisReset(g,GRF_XAXIS);
	GraphAxisReset(g,GRF_YAXIS);
	if(OpiumAlEcran(g)) OpiumRefresh(g->cdr);
//	PanelItemRLimits(oscillo->h.pAmpl,oscillo->h.vMin,0.0,oscillo->h.max);
//	PanelItemRLimits(oscillo->h.pAmpl,oscillo->h.vMax,oscillo->h.min,65535.0);
	PanelRefreshVars(oscillo->h.pAmpl);
#ifdef CURSEURS_MINMAX
//	InstrumRLimits(oscillo->h.iMin,0.0,oscillo->h.max);
//	InstrumRLimits(oscillo->h.iMax,oscillo->h.min,65535.0);
//	OpiumRefreshInstrum((Cadre)((oscillo->h.iMin)->cdr));
//	OpiumRefreshInstrum((Cadre)((oscillo->h.iMax)->cdr));
	InstrumRefreshVar(oscillo->h.iMin);
	InstrumRefreshVar(oscillo->h.iMax);
#endif

}
/* ========================================================================== */
static int OscilloHistoRaz(Menu menu, MenuItem item) {
	Oscillo oscillo; Graph g; int num;

	for(num=0; num<OscilloNb; num++) {
		oscillo = OscilloReglage[num];
		if(OpiumAlEcran(oscillo->ecran) && (oscillo->h.cRaz == menu->cdr)) {
			// voie = oscillo->voie[0];
			Acquis[AcquisLocale].etat.evt_recus = Acquis[AcquisLocale].etat.evt_trouves = 0;
			Acquis[AcquisLocale].etat.evt_ecrits = 0;
			LectCntl.EvtTaux = 0.0;
			LectCntl.TauxGlobal = 0.0;
			HistoRaz(oscillo->h.histo);
			g = oscillo->h.graph;
			if(OpiumAlEcran(g)) OpiumRefresh(g->cdr);
			break;
		}
	}
	return(0);
}
#ifdef REDONDANT
/* ========================================================================== */
static int OscilloChangeAcq(Instrum instrum, Oscillo oscillo) {
	if(!oscillo->acq) LectStop();
	else if(!Acquis[AcquisLocale].etat.active) do {
        OscilloDefiniTemps(oscillo); OscilloChangeVoie(0,oscillo); OscilloRestartDemande = 0; LectAcqStd();
    } while(OscilloRestartDemande);
	return(0);
}
/* ========================================================================== */
static int OscilloChangeModul(Instrum instrum, Oscillo oscillo) {
	if(oscillo->acq) {
//		if(OscilloAcqAllume) MenuItemEteint(OscilloAcqAllume,1,0);
		OscilloRestartDemande = 1;
		/* il faut sortir de la boucle d'evenements ,i.e. de LectAcqStd .. */
		LectStop();
		/* mais on reste dans le 'do{}' de OscilloChangeAcq!! */
	}
	return(0);
}
#endif
/* ========================================================================== */
static void OscilloAcqRun(char acq, char demodule, Menu menu) {
	Oscillo oscillo; int num; char redemarre;

	if(OscilloAcqAllume) MenuItemEteint(OscilloAcqAllume,1,0); OscilloAcqAllume = 0;
	oscillo = 0;
	redemarre = 0;
	for(num=0; num<OscilloNb; num++) if(OpiumAlEcran(OscilloReglage[num]->ecran)) {
		oscillo = OscilloReglage[num];
		oscillo->acq = acq;
//?		InstrumRefreshVar(oscillo->iAcq);
		if(demodule >= 0) {
			if(acq && Acquis[AcquisLocale].etat.active && (oscillo->demodule != demodule)) redemarre = 1;
			oscillo->demodule = demodule;
//?			if(oscillo->iDemod) InstrumRefreshVar(oscillo->iDemod);
		}
	}
	if(redemarre) {
		OscilloRestartDemande = 1;
		/* il faut sortir de la boucle d'evenements ,i.e. de LectAcqStd .. */
		LectStop(); /* mais on reste dans le 'do{}' ci-dessous!! */
	} else if(acq) {
		if(!Acquis[AcquisLocale].etat.active) do {
			MenuItemAllume(menu,1,0,GRF_RGB_GREEN); OscilloAcqAllume = menu;
            for(num=0; num<OscilloNb; num++) if(OpiumAlEcran(OscilloReglage[num]->ecran)) {
                OscilloDefiniTemps(oscillo); OscilloChangeVoie(0,OscilloReglage[num]);
            }
			OscilloRestartDemande = 0; LectAcqStd();
		} while(OscilloRestartDemande);
		if(OscilloAcqAllume) { MenuItemEteint(OscilloAcqAllume,1,0); OscilloAcqAllume = 0; }
	} else LectStop();
}
/* ========================================================================== */
static int OscilloAcqBrute(Menu menu, MenuItem *item) {
	OscilloAcqRun(1,0,menu);
	return(0);
}
/* ========================================================================== */
static int OscilloAcqDemod(Menu menu, MenuItem *item) {
	OscilloAcqRun(1,1,menu);
	return(0);
}
/* ========================================================================== */
static int OscilloAcqStop(Menu menu, MenuItem *item) {
	OscilloAcqRun(0,-1,menu);
	return(0);
}
#ifdef COMPLIQUE
#define COLS_MENU_VOIE 4
/* ========================================================================== */
int DetecteurDebloqueNumeriseur(Menu menu, MenuItem *item) {
	int voie,bolo,cap;

	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_voie == menu) break;
		if(voie >= VoiesNb) return(0);
	} else voie = (int)item;
	bolo = VoieManip[voie].det; if((cap = VoieManip[voie].cap) < 0) cap = 0;
	NumeriseurAutoriseAcces(&(Numeriseur[Bolo[bolo].captr[cap].bb.num]),1,1);
	return(0);
}
/* ========================================================================== */
int DetecteurBloqueNumeriseur(Menu menu, MenuItem *item) {
	int voie,bolo,cap;
	
	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_voie == menu) break;
		if(voie >= VoiesNb) return(0);
	} else voie = (int)item;
	bolo = VoieManip[voie].det; if((cap = VoieManip[voie].cap) < 0) cap = 0;
	NumeriseurAutoriseAcces(&(Numeriseur[Bolo[bolo].captr[cap].bb.num]),0,1);
	return(0);
}
#else
#define COLS_MENU_VOIE 3
#endif
/* ========================================================================== */
static int DetecteurCourantStandardise(Menu menu, MenuItem *item) {
	int voie,bolo,cap,bb_autorise; int num;
	TypeReglage *regl;
	char courante_seule; char *bouton[2];

	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_voie == menu) break;
		if(voie >= VoiesNb) return(0);
		bouton[0] = "tous"; bouton[1] = VoieManip[voie].nom;
		courante_seule = 1; // OpiumChoix(2,bouton,"Tous les reglages, ou %s seulement?",VoieManip[voie].nom);
	} else { voie = (int)item; courante_seule = 0; }
	bolo = VoieManip[voie].det;
	bb_autorise = -1;
	for(num=0; num<Bolo[bolo].nbreglages; num++) {
		regl = &(Bolo[bolo].reglage[num]);
		cap = regl->capt;
		if((Bolo[bolo].captr[cap].voie != voie) && courante_seule) continue;
		ReglageChargeStandard(&(Bolo[bolo]),regl,&bb_autorise,"  | ");
		// ReglageRafraichiTout(Bolo[bolo].captr[cap].bb.num,regl->ress,regl);
		ReglageRafraichiTout(regl->bb,regl->ress,regl);
	}
	ReglageChargeTerminee(bb_autorise);
	if(menu) {
		MenuItemEteint(menu,(IntAdrs)"Charger std",0);
		MenuItemEteint(menu,(IntAdrs)"Nouveau std",0);
	//	MenuItemEteint(menu,(IntAdrs)"Enregistrer",0);
	}
	VoieInfo[voie].validite = 1; PanelRefreshVars(VoieInfo[voie].status);
	return(0);
}
/* ========================================================================== */
static int DetecteurCourantMemorise(Menu menu, MenuItem *item) {
	int voie,bolo,cap; int i;

	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_voie == menu) break;
		if(voie >= VoiesNb) return(0);
	} else voie = (int)item;
	if((bolo = VoieManip[voie].det) < 0) return(0);
	for(i=0; i<Bolo[bolo].nbreglages; i++) {
		cap = Bolo[bolo].reglage[i].capt;
		if(Bolo[bolo].captr[cap].voie != voie) continue;
		strcpy(Bolo[bolo].reglage[i].std,Bolo[bolo].reglage[i].user);
	}
	Bolo[bolo].a_sauver = 1;
	if(menu) {
		MenuItemEteint(menu,(IntAdrs)"Charger std",0);
		MenuItemEteint(menu,(IntAdrs)"Nouveau std",0);
		MenuItemAllume(menu,(IntAdrs)"Enregistrer",0,GRF_RGB_YELLOW);
	}
	return(0);
}
/* ========================================================================== */
static int DetecteurCourantEnregistre(Menu menu, MenuItem *item) {
	int voie,bolo;

	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_voie == menu) break;
		if(voie >= VoiesNb) return(0);
	} else voie = (int)item;
	bolo = VoieManip[voie].det;
	DetecteurSauveReglages(bolo);
	if(menu) {
		MenuItemEteint(menu,(IntAdrs)"Charger std",0);
		MenuItemEteint(menu,(IntAdrs)"Nouveau std",0);
		MenuItemAllume(menu,(IntAdrs)"Enregistrer",0,GRF_RGB_GREEN);
	}
	return(0);
}
/* ========================================================================== */
static int DetecteurChangeVoieCourante(Menu menu, MenuItem *item) {
	int voie;
	
	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_voie == menu) break;
		if(voie >= VoiesNb) return(0);
	} else voie = (int)item;
	ReglagePlancheSupprime(voie);
	DetecteurOscillo(0,0);
	return(0);
}
/* ========================================================================== */
static int DetecteurCourantPrecedent(Menu menu, MenuItem *item) {
	int voie,bolo,cap,vm,dm; char complete;
	int bolo_initial,cap_initial;
	
	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_voie == menu) break;
		if(voie >= VoiesNb) return(0);
	} else voie = (int)item;
	complete = VoieInfo[voie].complete;
	ReglagePlancheSupprime(voie);
	bolo = VoieManip[voie].det;
	bolo_initial = bolo;
	cap_initial = VoieManip[voie].cap;
	vm = ModeleDet[Bolo[bolo].type].type[CapteurNum];
	while(bolo-- > 0) {
		dm = Bolo[bolo].type;
		for(cap=0; cap<ModeleDet[dm].nbcapt; cap++) {
			if(ModeleDet[dm].type[cap] == vm) break;
		}
		if(cap < ModeleDet[dm].nbcapt) {
			CapteurNum = cap;
			VoieInfo[Bolo[bolo].captr[cap].voie].complete = complete;
			for(cap=0; cap<ModeleDet[dm].nbcapt; cap++) {
				CapteurNom[cap] = ModeleVoie[ModeleDet[dm].type[cap]].nom;
			}
			CapteurNom[cap] = "\0";
			ReglageDeLaVoie(bolo,CapteurNum,-1);
			break;
		}
	}
	if(bolo < 0) {
		if(OpiumChoix(2,SambaNonOui,"Pas d'autre detecteur. Reprendre le precedent?")) {
			BoloNum = bolo_initial;
			CapteurNum = cap_initial;
			ReglageDeLaVoie(BoloNum,CapteurNum,-1);
		}
	}
	
	return(0);
}
/* ========================================================================== */
static int DetecteurCourantSuivant(Menu menu, MenuItem *item) {
	int voie,bolo,cap,vm,dm; char complete;
	int bolo_initial,cap_initial;
	
	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_voie == menu) break;
		if(voie >= VoiesNb) return(0);
	} else voie = (int)item;
	complete = VoieInfo[voie].complete;
	ReglagePlancheSupprime(voie);
	bolo = VoieManip[voie].det;
	bolo_initial = bolo;
	cap_initial = VoieManip[voie].cap;
	vm = ModeleDet[Bolo[bolo].type].type[CapteurNum];
	while(++bolo < BoloNb) {
		dm = Bolo[bolo].type;
		for(cap=0; cap<ModeleDet[dm].nbcapt; cap++) {
			if(ModeleDet[dm].type[cap] == vm) break;
		}
		if(cap < ModeleDet[dm].nbcapt) {
			BoloNum = bolo;
			CapteurNum = cap;
			VoieInfo[Bolo[bolo].captr[cap].voie].complete = complete;
			for(cap=0; cap<ModeleDet[dm].nbcapt; cap++) {
				CapteurNom[cap] = ModeleVoie[ModeleDet[dm].type[cap]].nom;
			}
			CapteurNom[cap] = "\0";
			ReglageDeLaVoie(BoloNum,CapteurNum,-1);
			break;
		}
	}
	if(bolo >= BoloNb) {
		if(OpiumChoix(2,SambaNonOui,"Pas d'autre detecteur. Reprendre le precedent?")) {
			BoloNum = bolo_initial;
			CapteurNum = cap_initial;
			ReglageDeLaVoie(BoloNum,CapteurNum,-1);
		}
	}
	
	return(0);
}
/* ========================================================================== */
static void DetecteurVoieAdjacente(Menu menu, MenuItem *item, char vers_le_haut) {
	int voie,bolo,cap,vm,phys,dm,prem; char complete,trouve;
	int bolo_initial,cap_initial;
	
	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_suite == menu) break;
		if(voie >= VoiesNb) return;
	} else voie = (int)item;
	complete = VoieInfo[voie].complete;
	ReglagePlancheSupprime(voie);
	bolo = VoieManip[voie].det;
	bolo_initial = bolo;
	cap_initial = VoieManip[voie].cap;
	vm = ModeleDet[Bolo[bolo].type].type[CapteurNum];
	phys = ModeleVoie[vm].physique;
	prem = cap_initial + (vers_le_haut? 1: -1);
	do {
		dm = Bolo[bolo].type;
		if(vers_le_haut) {
			for(cap=prem; cap<ModeleDet[dm].nbcapt; cap++) {
				if(ModeleVoie[ModeleDet[dm].type[cap]].physique == phys) break;
			}
			trouve = (cap < ModeleDet[dm].nbcapt);
		} else {
			for(cap=prem; cap>=0; --cap) {
				if(ModeleVoie[ModeleDet[dm].type[cap]].physique == phys) break;
			}
			trouve = (cap >= 0);
		}
		if(trouve) {
			BoloNum = bolo;
			CapteurNum = cap;
			VoieInfo[Bolo[bolo].captr[cap].voie].complete = complete;
			for(cap=0; cap<ModeleDet[dm].nbcapt; cap++) {
				CapteurNom[cap] = ModeleVoie[ModeleDet[dm].type[cap]].nom;
			}
			CapteurNom[cap] = "\0";
			ReglageDeLaVoie(BoloNum,CapteurNum,-1);
			break;
		} else {
			if(BoloNb > 1) {
				if(vers_le_haut) {
					do { if(++bolo >= BoloNb) bolo = 0; if(bolo == bolo_initial) break; } while(!Bolo[bolo].local);
				} else {
					do { if(--bolo < 0) bolo = BoloNb - 1; if(bolo == bolo_initial) break; } while(!Bolo[bolo].local);
				}
			}
			if(bolo == bolo_initial) {
				if(OpiumChoix(2,SambaNonOui,"Pas d'autre voie de type '%s'. Reprendre la voie %s?",ModelePhys[phys].nom,VoieManip[voie].nom)) {
					BoloNum = bolo_initial;
					CapteurNum = cap_initial;
					ReglageDeLaVoie(BoloNum,CapteurNum,-1);
				}
				break;
			}
			prem = 0;
		}
	} while(1);
}
/* ========================================================================== */
static int DetecteurVoiePrecedente(Menu menu, MenuItem *item) {
//	EnConstruction();
	DetecteurVoieAdjacente(menu,item,0);
	return(0);
}
/* ========================================================================== */
static int DetecteurVoieSuivante(Menu menu, MenuItem *item) {
#ifdef FUTUR_EX
	int voie,bolo,cap,vm,phys,dm,prem; char complete;
	int bolo_initial,cap_initial;
	
	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_suite == menu) break;
		if(voie >= VoiesNb) return(0);
	} else voie = (int)item;
	complete = VoieInfo[voie].complete;
	ReglagePlancheSupprime(voie);
	bolo = VoieManip[voie].det;
	bolo_initial = bolo;
	cap_initial = VoieManip[voie].cap;
	vm = ModeleDet[Bolo[bolo].type].type[CapteurNum];
	phys = ModeleVoie[vm].physique;
	prem = cap_initial + 1;
	do {
		dm = Bolo[bolo].type;
		for(cap=prem; cap<ModeleDet[dm].nbcapt; cap++) {
			if(ModeleVoie[ModeleDet[dm].type[cap]].physique == phys) break;
		}
		if(cap < ModeleDet[dm].nbcapt) {
			BoloNum = bolo;
			CapteurNum = cap;
			VoieInfo[Bolo[bolo].captr[cap].voie].complete = complete;
			for(cap=0; cap<ModeleDet[dm].nbcapt; cap++) {
				CapteurNom[cap] = ModeleVoie[ModeleDet[dm].type[cap]].nom;
			}
			CapteurNom[cap] = "\0";
			ReglageDeLaVoie(BoloNum,CapteurNum,-1);
			break;
		} else {
			do { if(++bolo >= BoloNb) bolo = 0; } while(!Bolo[bolo].local || (bolo == bolo_initial));
			if(bolo == bolo_initial) {
				if(OpiumChoix(2,SambaNonOui,"Pas d'autre voie de type '%s'. Reprendre la precedente?",ModelePhys[phys].nom)) {
					BoloNum = bolo_initial;
					CapteurNum = cap_initial;
					ReglageDeLaVoie(BoloNum,CapteurNum,-1);
				}
				break;
			}
			prem = 0;
		}
	} while(1);
#else
	DetecteurVoieAdjacente(menu,item,1);
#endif
	
	return(0);
}
/* ========================================================================== */
static int DetecteurViVoieAuto(Menu menu, MenuItem *item) {
    int voie,bolo,cap,bb;
    TypeReglage *rampl,*rcomp/* ,*rcorr */;
	
	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_modul == menu) break;
		if(voie >= VoiesNb) return(0);
	} else voie = (int)item;

    if(!(rampl = ReglagePtr(voie,REGL_MODUL))) {
        OpiumError("Pas de reglage de modulation sur la voie %s",VoieManip[voie].nom);
        return(0);
    }
    if(!(rcomp = ReglagePtr(voie,REGL_COMP))) {
        OpiumError("Pas de reglage de compensation sur la voie %s",VoieManip[voie].nom);
        return(0);
    }
    if(OpiumExec(pReglViAuto->cdr) == PNL_CANCEL) return(0);

	printf("%s/ * Mesure des V(I) de %gV a %gV par %s %gV\n",DateHeure(),ReglVi.dep,ReglVi.arr,ReglVi.oper?"facteur":"pas de",ReglVi.pas);
	ReglVi.nb_voies = 1; ReglVi.voie[0].num = voie;
    ReglVi.nb_mesures = 0;
	ReglVi.cur = ReglVi.dep;
	if((cap = VoieManip[voie].cap) < 0) cap = 0;
	if((bolo = VoieManip[voie].det) < 0) bolo = 0;
	bb = Bolo[bolo].captr[cap].bb.num;
	NumeriseurAutoriseAcces(&(Numeriseur[bb]),1,0);
	ReglageChangeFloat(rampl,ReglVi.cur,0); printf("%s/   . %s.%s = %g\n",DateHeure(),Bolo[bolo].nom,rampl->nom,ReglVi.cur);
 	ReglageChangeFloat(rcomp,ReglVi.cur,0); printf("%s/   . %s.%s = %g\n",DateHeure(),Bolo[bolo].nom,rcomp->nom,ReglVi.cur);
	/* laisser ce reglage a une valeur passe-partout: 
	if((rcorr = ReglagePtr(voie,REGL_CORR))) {
		ReglageChangeFloat(rcorr,0.0,0); printf("%s/   . %s.%s = %g\n",DateHeure(),Bolo[bolo].nom,rcorr->nom,0.0);
	} */
	SambaAttends(ReglVi.attente*1000);
	NumeriseurAutoriseAcces(&(Numeriseur[bb]),0,0);
	ReglVi.actif = 1;
    if(!Acquis[AcquisLocale].etat.active) {
		ReglVi.arrete = 1;
		// LectAcqElementaire(NUMER_MODE_ACQUIS);
		if(!VoieInfo[voie].oscillo) LectAcqStd();
		else if(!(VoieInfo[voie].oscillo)->ecran) LectAcqStd();
		else do { OscilloChangeVoie(0,VoieInfo[voie].oscillo); OscilloRestartDemande = 0; LectAcqStd(); } while(OscilloRestartDemande);
	} else ReglVi.arrete = 0;
	
	return(0);
}
/* ========================================================================== */
MenuItem iReglageInsere[] = {
    { "Insere dans V(I)", MNU_FONCTION DetecteurViInsere },
    { "Gestion des V(I)", MNU_MENU   &mGestVI },
    MNU_END };

MenuItem iReglageMenuSolo[] = {
	{ "Charger std", MNU_FONCTION DetecteurCourantStandardise },
	{ "Nouveau std", MNU_FONCTION DetecteurCourantMemorise },
	{ "Enregistrer", MNU_FONCTION DetecteurCourantEnregistre },
	MNU_END
};
MenuItem iReglageMenuVoie[] = {
	#ifdef COMPLIQUE
	{ "Debloquer",    MNU_FONCTION DetecteurDebloqueNumeriseur },
	{ "Bloquer",      MNU_FONCTION DetecteurBloqueNumeriseur },
	#endif
	{ "Charger std", MNU_FONCTION DetecteurCourantStandardise },
	{ "Precedent  ", MNU_FONCTION DetecteurCourantPrecedent },
	{ "Nouveau std", MNU_FONCTION DetecteurCourantMemorise },
	{ "Suivant    ", MNU_FONCTION DetecteurCourantSuivant },
	{ "Enregistrer", MNU_FONCTION DetecteurCourantEnregistre },
	{ "Autre voie ", MNU_FONCTION DetecteurChangeVoieCourante },
	MNU_END
};
MenuItem iReglageMenuSuite[] = {
	{ "Voie precedente", MNU_FONCTION DetecteurVoiePrecedente },
	{ "Voie suivante",   MNU_FONCTION DetecteurVoieSuivante },
	MNU_END
};
MenuItem iReglageMenuModul[] = {
	{ "Compenser",         MNU_FONCTION DetecteurCompenseVoie },
    { "Inserer dans V(I)", MNU_FONCTION DetecteurViInsere },
    { "V(I) auto",         MNU_FONCTION DetecteurViVoieAuto },
	MNU_END
};

/* ========================================================================== */
void ReglageCreePanneauTotal() {
	int bolo; int x,y;
	TypeDetecteur *detectr;

	/* Bouton '1er bolo=?' avec apply=cette fonction */
	/* Memorisation+enregistrement en fin d'affichage */
	x = 10; y = 100;
	for(bolo=BoloNum; bolo<BoloNb; bolo++) {
		detectr = Bolo + bolo;
	}
}
/* ========================================================================== */
static int ReglageAutoriseAcces(Instrum instrum, int voie) {
	int cap,bb;

	if((cap = VoieManip[voie].cap) < 0) cap = 0;
	if((bb = Bolo[VoieManip[voie].det].captr[cap].bb.num) >= 0) 
		NumeriseurAutoriseAcces(&(Numeriseur[bb]),VoieInfo[voie].modifiable,1);
	return(0);
}
/* ========================================================================== */
static int ReglageDebloqueNumeriseur(Menu menu, MenuItem *item) {
	int voie,cap,bb;

	if(menu) {
		for(voie=0; voie<VoiesNb; voie++) if(VoieInfo[voie].menu_debloque == menu) break;
		printf("* Deblocage demande pour le numeriseur en charge de la voie %s\n",(voie < VoiesNb)? VoieManip[voie].nom:"(inconnue)");
		if(voie >= VoiesNb) return(0);
	} else voie = (int)item;
	if((cap = VoieManip[voie].cap) < 0) cap = 0;
	if((bb = Bolo[VoieManip[voie].det].captr[cap].bb.num) >= 0) 
		NumeriseursBB1Debloque(&(Numeriseur[bb]),1,"*");
	return(0);
}
/* ========================================================================== */
static int ReglagePlancheStoppe(Cadre cdr, void *arg) {
	int voie;
//+	int bolo,cap,bb;

	voie = (int)arg;
	printf("%s/ ----- Reglages de la voie %s termines  -----\n",DateHeure(),VoieManip[voie].nom);
//+	bolo = VoieManip[voie].det; cap = VoieManip[voie].cap;
//+	bb = Bolo[bolo].captr[cap].bb.num;
//+	if(bb >= 0) NumeriseurAutoriseAcces(&(Numeriseur[bb]),0,1);
//+	if(VoieInfo[voie].oscillo && (VoieInfo[voie].oscillo)->acq) {
//+		OscilloRestartDemande = 1;
//+		LectStop(); /* il faut sortir de la boucle d'evenements ,i.e. de LectAcqStd!! */
//+	}
	return(1);
}
/* ========================================================================== */
static void ReglagePlancheEfface(int voie) {
	int j,nb,v,bb;

	if(VoieInfo[voie].planche) {
		OpiumClear(VoieInfo[voie].planche);
		BoardTrash(VoieInfo[voie].planche);
	}
	OscilloVide(VoieInfo[voie].oscillo);
	// gReglageSignal[voie] = 0;
	//? VoieInfo[voie].nb = NumeriseurInstrNb - VoieInfo[voie].prems;
	nb = VoieInfo[voie].nb;
	NumeriseurInstrNb -= nb;
	for(j=VoieInfo[voie].prems; j<NumeriseurInstrNb; j++) {
		memcpy(NumeriseurInstr+j,NumeriseurInstr+j+nb,sizeof(TypeNumeriseurInstrum));
	}
	for(v=0; v<VOIES_MAX; v++) {
		if(VoieInfo[v].prems > VoieInfo[voie].prems) VoieInfo[v].prems -= nb;
	}
	for(bb=0; bb<NumeriseurNb; bb++) {
		if(Numeriseur[bb].interface.prems > VoieInfo[voie].prems) Numeriseur[bb].interface.prems -= nb;
	}
	VoieInfo[voie].nb = 0;
	VoieInfo[voie].prems = NumeriseurInstrNb;
	if(VoieInfo[voie].planche) BoardDelete(VoieInfo[voie].planche);
	VoieInfo[voie].planche = 0;
	VoieInfo[voie].menu_voie = VoieInfo[voie].menu_suite = VoieInfo[voie].menu_modul = VoieInfo[voie].menu_debloque = VoieInfo[voie].menu_raz = 0;
	VoieInfo[voie].stats = VoieInfo[voie].taux_evts = VoieInfo[voie].vi = 0;
	VoieInfo[voie].raz = VoieInfo[voie].status = VoieInfo[voie].electrons = 0;
}
/* ========================================================================== */
static int ReglagePulseTest(Panel panel, int num, void *arg) {
	int voie; float charge_fC; // 'injection' en mV, 'couplage' en pF
	
//	voie = (int)arg;
//	if((charge_fC = VoieManip[voie].calib.injection * VoieManip[voie].calib.couplage) != 0.0) {
//		VoieManip[voie].calib.gain_ampli = VoieManip[voie].ADUenmV * VoieManip[voie].calib.mesure / charge_fC;
//		if(OpiumAlEcran(VoieInfo[voie].electrons)) PanelRefreshVars(VoieInfo[voie].electrons);
//	}

	voie = (int)arg;
	if(VoieManip[voie].ADUenmV != 0.0) {
		charge_fC = VoieManip[voie].calib.injection * VoieManip[voie].calib.couplage;
		VoieManip[voie].calib.mesure = VoieManip[voie].calib.gain_ampli * charge_fC / VoieManip[voie].ADUenmV;
		if(OpiumAlEcran(VoieInfo[voie].electrons)) PanelRefreshVars(VoieInfo[voie].electrons);
	}
	
	return(0);
}
/* ========================================================================== */
static void ReglagePlancheSupprime(int voie) {
	int num,n,v; Oscillo oscillo;

	ReglagePlancheStoppe(0,(void *)(long)voie);
	ReglagePlancheEfface(voie);
	if((oscillo = VoieInfo[voie].oscillo)) {
		do {
			for(v=0; v<oscillo->nbvoies; v++) if(oscillo->voie[v] == voie) break;
			if(v < oscillo->nbvoies) {
				oscillo->nbvoies -= 1;
				for( ; v<oscillo->nbvoies; v++) oscillo->voie[v] = oscillo->voie[v + 1];
			} else break;
		} while(1);
		VoieInfo[voie].oscillo = 0;
		if(oscillo->nbvoies == 0) {
			num = oscillo->num;
			free(oscillo);
			OscilloNb--;
			for(n=num; n<OscilloNb; n++) { OscilloReglage[n] = OscilloReglage[n + 1]; OscilloReglage[n]->num = n; }
		}
	}
}
/* ==========================================================================
static int ReglageModeBB(Panel panel, void *arg) {
	int voie,cap,bb;
	TypeDetecteur *detectr; TypeNumeriseur *numeriseur; TypeRepartiteur *repart;
	NUMER_MODE mode;
	
	voie = *(int *)arg;
	if((voie < 0) || (voie >= VoiesNb)) return(0);
	cap = VoieManip[voie].cap;
	detectr = &(Bolo[VoieManip[voie].det]);
	if(detectr && (cap >= 0) && (cap < detectr->nbcapt)) {
		bb = detectr->captr[cap].bb.num;
		if(bb >= 0) {
			numeriseur = &(Numeriseur[bb]);
			if((repart = numeriseur->repart)) {
				mode = (VoieInfo[voie].mode == 0)? NUMER_MODE_ACQUIS: NUMER_MODE_IDENT;
				RepartiteurDemarreNumeriseurs(repart,mode,numeriseur->def.adrs,1);
			}
		}
	}
	return(1);
}
   ========================================================================== */
static int ReglageSimuRbolo(Panel panel, int itemnum, void *arg) {
	int voie,cap; TypeDetecteur *detectr; TypeReglage *reglage;

	reglage = (TypeReglage *)arg;
	detectr = Bolo + reglage->bolo; cap = reglage->capt; if(cap < 0) cap = 0;
	voie = detectr->captr[cap].voie; if((voie < 0) || (voie >= VoiesNb)) return(0);
	VoieTampon[voie].simu.facteurC = 1.0E9 / VoieManip[voie].ADUennV;
	if(VoieTampon[voie].simu.facteurC < 0.0) VoieTampon[voie].simu.facteurC = -VoieTampon[voie].simu.facteurC;
	if(VoieManip[voie].Rmodul > 0.0) VoieTampon[voie].simu.facteurC *= (VoieManip[voie].Rbolo / VoieManip[voie].Rmodul);

	ReglageEffetDeBord(reglage,"  |");

	return(1);
}
/* ========================================================================== */
static int ReglageReplaceEcran(Instrum instrum, int voie) {
	ReglagePlancheEfface(voie);
	ReglagePlancheCree(voie);
	OpiumUse(VoieInfo[voie].planche,OPIUM_FORK);
	OpiumUserAction();
	return(0);
}
#define MAXREGLAGECOLS 4
/* ========================================================================== 
static int ReglageShuffler() {
	TypeDetecteur *detectr; TypeNumeriseur *numeriseur; TypeRepartiteur *repart;
	int voie,cap,bb; int k,l; // unsigned int reg;
	char texte[256];
	
	voie = VoieEnReglage;
	detectr = &(Bolo[VoieManip[voie].det]);
	cap = VoieManip[voie].cap;
	if((cap >= 0) && (cap < detectr->nbcapt)) bb = detectr->captr[cap].bb.num;
	if(bb < 0) return(0);
	numeriseur = &(Numeriseur[bb]); repart = numeriseur->repart;
	
	l = sprintf((char *)texte,"%s_0x%08X_0x%08X",IPE_SLT_WRITE,IPE_SLT_CONTROL,0x0000); texte[l] = '\0';
	k = RepartIpEcrit(repart,texte,l); SambaMicrosec(ModeleRep[repart->type].delai_msg);
	printf("%s/ Ecrit: %s <- %s [%d/%d]\n",DateHeure(),repart->nom,texte,k,l);
	sleep(2);
	l = sprintf((char *)texte,"%s_0x%08X_0x%08X",IPE_SLT_WRITE,IPE_SLT_CONTROL,0x4000); texte[l] = '\0';
	k = RepartIpEcrit(repart,texte,l); SambaMicrosec(ModeleRep[repart->type].delai_msg);
	printf("%s/ Ecrit: %s <- %s [%d/%d]\n",DateHeure(),repart->nom,texte,k,l);
	
	return(0);
}
   ========================================================================== */
static int ReglagePlancheCree(int voie) {
	/* Les valeurs utilisees ici sont celles des numeriseurs:
	   A la lecture du cablage puis des reglages des detecteurs, les
	   ressources des numeriseurs recoivent les valeurs correspondantes.
	   Si ceux-ci sont relus (p.exe BBv2 par BO), on a ici les vraies valeurs;
	   sinon, ca revient au meme qu'avant.
	   Sauf qu'il faut reporter les valeurs des ressources sur les reglages
	   a la fin de cette fonction.. */
	int i,j,k,l,m,n;
	int cap,bb,vm,vbb,num;
	float fval; int ival;
	char *nom;
	TypeDetecteur *detectr; TypeDetModele *modele_det; TypeReglage *regl;
	TypeBNModele *modele_bn; TypeBNModlRessource *ress; TypeNumeriseur *numeriseur;
	int x,y,xb,ysaut; char ysaut2; // ,x0,y0;
	int xl[MAXREGLAGECOLS],xc[MAXREGLAGECOLS],yc[MAXREGLAGECOLS],xm,xp,xs,ys,col,xf,yf;
	int bas_boutons,bord_gauche;
	Menu menu; Panel panel; char titre[80];
	TypeInstrumGlissiere *glissiere_affiche;
	unsigned char mode;
	char charge_auto,raz;
#ifdef REGLAGE_DET_AVEC_SIGNAL
	Instrum instrum;
#endif
	
	modele_bn = 0; numeriseur = 0; charge_auto = 0; fval = 0.0; ival = 0;// GCC
	VoieEnReglage = voie;
	vm = VoieManip[voie].type;
	detectr = &(Bolo[VoieManip[voie].det]);
	modele_det = &(ModeleDet[detectr->type]);
	cap = VoieManip[voie].cap;
	if((cap >= 0) && (cap < detectr->nbcapt)) {
		bb = detectr->captr[cap].bb.num;
		vbb = detectr->captr[cap].bb.adc - 1;
	} else { bb = -1; vbb = 0; }
	if(bb >= 0) {
		numeriseur = &(Numeriseur[bb]);
		modele_bn = &(ModeleBN[numeriseur->def.type]);
		printf("%s/ ----- Reglage de la voie %s via %s  -----\n",DateHeure(),VoieManip[voie].nom,numeriseur->nom);
		if(numeriseur->avec_status) { charge_auto = 0; LectLitStatus = 1; printf("                Le status de %s sera relu\n",numeriseur->nom); } // modele_bn->ident >= 2
		else if(detectr->nbreglages) {
			charge_auto = SettingsChargeReglages; /* LectLitStatus = 0; */ 
			if(charge_auto) printf("                Chargement prealable des reglages\n");
			else printf("                Les reglages effectifs ne sont pas affiches\n");
		} else printf("                Pas de reglage pour cette voie\n");
	} else printf("%s/ ----- Affichage de la voie %s -----\n",DateHeure(),VoieManip[voie].nom);

	VoieInfo[voie].validite = 0;
	VoieInfo[voie].moyenne = VoieInfo[voie].bruit = VoieInfo[voie].compens = 0.0;	
	VoieInfo[voie].planche = BoardCreate();
	// sprintf(titre,L_("Reglage %s %s"),ModeleVoie[vm].nom,detectr->nom);
	sprintf(titre,L_("Connexion sur la voie %s"),VoieManip[voie].nom);
	OpiumTitle(VoieInfo[voie].planche,titre);
	y = Dy;
    bas_boutons = y;
	bord_gauche = (2 * Dx);
	if(VoieInfo[voie].complete == 2) bord_gauche += (Glissiere.largeur + Glissiere.graduation + ((Glissiere.nbchars + 33) * Dx));

	j = NumeriseurInstrNb;
	VoieInfo[voie].prems = j;
	
	if(bb >= 0) PanelColumns(panel = PanelCreate(6),3); else panel = PanelCreate(2);
	PanelMode(panel,PNL_RDONLY|PNL_DIRECT);
	PanelSupport(panel,WND_CREUX);
	PanelItemSelect(panel,PanelText(panel,L_("Detecteur"),detectr->nom,DETEC_NOM),0);
	PanelItemSelect(panel,PanelText(panel,L_("voie"),ModeleVoie[vm].nom,MODL_NOM),0);
	if(bb >= 0) {
		PanelItemSelect(panel,PanelText (panel,L_("Numeriseur"),modele_bn->nom,MODL_NOM),0);
		PanelItemSelect(panel,PanelOctet(panel,L_("numero"),&(numeriseur->def.serial)),0);
		PanelItemSelect(panel,PanelOctet(panel,L_("canal #"),&(detectr->captr[cap].bb.adc)),0);
		for(num=0; num<modele_bn->nbress; num++) {
			ress = modele_bn->ress + num;
			if(ress->categ == RESS_ALIM) {
				//1 i = PanelItemSelect(panel,PanelSHex(panel,"alim",&(numeriseur->ress[num].hval)),0);
				//2 i = PanelKey(panel,"alim",ress->lim_k.cles,&(numeriseur->ress[num].val.i),NUMER_RESS_VALTXT);
				//2 PanelItemModif(panel,i,NumeriseurRessEnCours,(void *)(long)j);
				//2 PanelItemExit(panel,i,NumeriseurRessFromKey,(void *)(long)j);
				PanelItemSelect(panel,PanelKey(panel,"alim",ress->lim_k.cles,&(numeriseur->ress[num].val.i),NUMER_RESS_VALTXT),0);
				NumeriseurInstr[j].type = OPIUM_PANEL; NumeriseurInstr[j].instrum = (void *)panel;
				NumeriseurInstr[j].numeriseur = numeriseur; NumeriseurInstr[j].num = num; NumeriseurInstr[j].reglage = 0;
				j++;
				break;
			}
		}
	}
	// PanelItemSelect(panel,PanelChar(panel,"alim",&(detectr->captr[cap].bb.adc)),0);
	BoardAddPanel(VoieInfo[voie].planche,panel,0,y,0);
	yf = y + 2*Dy;
	if(bb < 0) goto reglages_vus;

	ysaut = 0;
	for(i=0; i<numeriseur->nbprocs; i++) if(!strcmp(numeriseur->proc[i].nom,"deblocage")) break;
	if(i < numeriseur->nbprocs) {
		if(ModeleBN[numeriseur->def.type].ident >= 2) {
			instrum = InstrumKeyB(INS_GLISSIERE,(void *)&Gliss2lignes,&(VoieInfo[voie].modifiable),L_("bloquer/liberer"));
			InstrumOnChange(instrum,ReglageAutoriseAcces,(void *)(long)voie);
			OpiumSupport(instrum->cdr,WND_RAINURES);
			// BoardAddInstrum(VoieInfo[voie].planche,instrum,OPIUM_ADJACENT,2*Dy,panel->cdr); // un peu trop colle
			BoardAddInstrum(VoieInfo[voie].planche,instrum,76 * Dx,Dy,panel->cdr);
			ysaut = 3 * Dy;
		} else {
			menu = MenuBouton(L_("Debloquer"),MNU_FONCTION ReglageDebloqueNumeriseur);
			BoardAddMenu(VoieInfo[voie].planche,menu,76 * Dx,y+Dy,0);
			VoieInfo[voie].menu_debloque = menu;
			ysaut = 2 * Dy;
		}
	} else VoieInfo[voie].menu_debloque = 0;
	raz = 0;
	if(ReglageNum(detectr->numero,voie,REGL_RAZFET) >= 0) {
		menu = MenuBouton(L_("Lancer RAZ FET"),MNU_FONCTION DetecteurRazFetBouton);
		raz = 1;
	} else if(ReglageNum(detectr->numero,voie,REGL_FET_MASSE) >= 0) {
		menu = MenuBouton(L_("Lancer RAZ FET"),MNU_FONCTION DetecteurRazFetMasse);
		raz = 1;
	} else if(ReglageNum(detectr->numero,voie,REGL_FET_CC) >= 0) {
		menu = MenuBouton(L_("Lancer RAZ FET"),MNU_FONCTION DetecteurRazFetCC);
		raz = 1;
	}
	if(raz) {
		OpiumSupport(menu->cdr,WND_PLAQUETTE);
		BoardAddMenu(VoieInfo[voie].planche,menu,90*Dx,y,0); // 80 pour coller au curseur precedent, 85 pour coller a l'oscillo
		VoieInfo[voie].menu_raz = menu;
		panel = PanelCreate(1);
		PanelMode(panel,PNL_DIRECT);
		PanelSupport(panel,WND_CREUX);
		PanelInt(panel,L_("duree"),&ReglRazDuree); PanelItemLngr(panel,1,4);
		VoieInfo[voie].raz = panel;
		BoardAddPanel(VoieInfo[voie].planche,panel,90*Dx,y+(2*Dy),0);
		if(ysaut < (2 * Dy)) ysaut = 2 * Dy;
	}
	xb = 12 * Dx;
	y += ysaut;
	if(ysaut) { xm = Dx; glissiere_affiche = &Gliss3lignes; ysaut = 0; }
	else { xm = 69 * Dx; glissiere_affiche = &ReglageGlissAffiche; ysaut = 3 * Dy; }
	ysaut2 = 0;
#ifdef REGLAGE_DET_AVEC_SIGNAL
	instrum = InstrumKeyB(INS_GLISSIERE,(void *)glissiere_affiche,&(VoieInfo[voie].complete),"DACs/+signal/+trigger");
	InstrumOnChange(instrum,ReglageReplaceEcran,(void *)(long)voie);
	if(xm == Dx) OpiumSupport(instrum->cdr,WND_RAINURES);
	// BoardAddInstrum(VoieInfo[voie].planche,instrum,OPIUM_A_DROITE_DE OPIUM_PRECEDENT); // 77*Dx
	BoardAddInstrum(VoieInfo[voie].planche,instrum,xm,y,0);
	if(xm == Dx) xm = xb = 14 * Dx; else { xm = Dx; ysaut2 = 2 * Dy; }; y += ysaut;
#endif
	ysaut = 0;
	if(VoiesNb > 1) {
		menu = MenuLocalise(iReglageMenuSuite); OpiumSupport(menu->cdr,WND_PLAQUETTE); // = mReglageMenuSuite
		VoieInfo[voie].menu_suite = menu;
		if(!detectr->nbreglages) { MenuColumns(menu,2); xm = xb; }
		BoardAddMenu(VoieInfo[voie].planche,menu,xm,y,0);
		xm += (17 * Dx);
		if(detectr->nbreglages) { ysaut = 3 * Dy; }
		else { xm += (17 * Dx); xb = xm; ysaut2 = 3 * Dy; }
	} else VoieInfo[voie].menu_suite = 0;

	if(detectr->nbreglages) {
		menu = MenuLocalise(iReglageMenuVoie); OpiumSupport(menu->cdr,WND_PLAQUETTE); // = mReglageMenuVoie
		col = COLS_MENU_VOIE;
		if(BoloNb < 2) {
			MenuItemHide(menu,(IntAdrs)"Precedent  ");
			MenuItemHide(menu,(IntAdrs)"Suivant    ");
			col--;
			if(VoiesNb < 2) MenuItemHide(menu,(IntAdrs)"Autre voie ");
		}
		MenuColumns(menu,col);
		VoieInfo[voie].menu_voie = menu;
		BoardAddMenu(VoieInfo[voie].planche,menu,xm,y,0);
		panel = PanelCreate(1); PanelSupport(panel,WND_CREUX); PanelMode(panel,PNL_DIRECT|PNL_RDONLY);
		PanelKeyB(panel,0,L_("Reglages effectifs: inconnus/Reglages affiches: effectifs"),&(VoieInfo[voie].validite),29);
		PanelItemColors(panel,1,SambaRougeVert,2);
		VoieInfo[voie].status = panel;
		// BoardAddPanel(VoieInfo[voie].planche,panel,68*Dx,y+Dy,0);
		BoardAddPanel(VoieInfo[voie].planche,panel,OPIUM_APRES,y+Dy,menu->cdr);
		ysaut = 3 * Dy;
	} else VoieInfo[voie].menu_voie = 0;
	y += ysaut;
	
#ifdef VERIFIE_COMMUNICATION
	panel = PanelCreate(1); PanelSupport(panel,WND_CREUX);
	PanelKeyB(panel,"Mode BB",L_("donnees/ident"),&(VoieInfo[voie].mode),8);
	PanelItemColors(panel,1,SambaVertJauneOrangeRouge,2);
	PanelOnApply(panel,ReglageModeBB,(void *)(long)voie); // ,(void *)(&VoieEnReglage));
	// ou bien PanelMode(panel,PNL_DIRECT); avec PanelItemExit(panel,1,ReglageModeBB,(void *)(&VoieEnReglage));
	BoardAddPanel(VoieInfo[voie].planche,panel,0,y,0);
#endif
	if(VoieInfo[voie].complete) {
		xb += (2 * Dx); // pour l'anglois
		BoardAddMenu(VoieInfo[voie].planche,MenuBouton(L_("Signal brut"),MNU_FONCTION OscilloAcqBrute),xb,y,0); xb += (14 * Dx);
		BoardAddMenu(VoieInfo[voie].planche,MenuBouton(L_("Arret"),      MNU_FONCTION OscilloAcqStop), xb,y,0); xb += ( 8 * Dx);
		if(VoieManip[voie].modulee) {
			BoardAddMenu(VoieInfo[voie].planche,MenuBouton(L_("Signal demodule"),MNU_FONCTION OscilloAcqDemod),xb,y,0); xb += (18 * Dx);
		}
		BoardAddBouton(VoieInfo[voie].planche,L_("Pulseur"),OscilloGegene,xb+(2*Dx),y,0);
		if(VoieManip[voie].modulee) {
			float rval;
			for(n=0; n<detectr->nbreglages; n++) {
				regl = &(detectr->reglage[n]);
				if(regl->capt != cap) continue;
				if((bb = regl->bb) < 0) continue;
				if(regl->cmde == REGL_MODUL) {
					numeriseur = &(Numeriseur[bb]);
					modele_bn = &(ModeleBN[numeriseur->def.type]);
					num = regl->ress;
					if((num < 0) || (num >= modele_bn->nbress)) continue;
					ress = modele_bn->ress + num;
					if(ress->type == RESS_FLOAT) rval = Numeriseur[bb].ress[num].val.r;
					else rval = (float)(Numeriseur[bb].ress[num].val.i);
					VoieTampon[voie].simu.modul = rval * VoieTampon[voie].simu.facteurC;
					panel = PanelCreate(1); PanelSupport(panel,WND_CREUX);
					PanelFloat(panel,L_("Rdetec simulee (MO)"),&(VoieManip[voie].Rbolo));
					PanelItemLngr(panel,1,6);
					PanelMode(panel,PNL_DIRECT); PanelItemReturn(panel,1,ReglageSimuRbolo,(void *)regl);
					BoardAddPanel(VoieInfo[voie].planche,panel,xb+(13*Dx),y,0);
					break;
				}
			}
		}
		y += Dy;
	}
	y += ysaut2; yf = y;
	if(VoieInfo[voie].complete != 2) yf += Dy;
//	y += Dy;
	//- BoardAddMenu(VoieInfo[voie].planche,MenuBouton("Magique",MNU_FONCTION ReglageShuffler),60*Dx,y,0);
	if(detectr->nbreglages) {
		// y += (3 * Dy); // 2 sans BoardAreaOpen(L_("Reglages"),WND_PLAQUETTE)
		bas_boutons = y;

		for(col=0; col<MAXREGLAGECOLS; col++) { xl[col] = 0; yc[col] = y; }
		col = 0;
		for(n=0; n<detectr->nbreglages; n++) {
			regl = &(detectr->reglage[n]);
			if(regl->capt != cap) continue;
			if((bb = regl->bb) < 0) continue;
			nom = modele_det->regl[n].nom;
			xm = strlen(nom) + 15; if(xm < REGLAGE_LARGEUR) xm = REGLAGE_LARGEUR;
			if(xm > xl[col]) xl[col] = xm;
			col++; if(col >= MAXREGLAGECOLS) col = 0;
		}
		xc[0] = 2; for(col=1; col<MAXREGLAGECOLS; col++) xc[col] = xc[col - 1] + xl[col - 1];

		if(charge_auto) NumeriseurAutoriseAcces(numeriseur,1,2);
		BoardAreaOpen(L_("Reglages"),WND_PLAQUETTE);
		col = 0;
		for(n=0; n<detectr->nbreglages; n++) {
			regl = &(detectr->reglage[n]);
			if(regl->capt != cap) continue;
			if((bb = regl->bb) < 0) continue;
			numeriseur = &(Numeriseur[bb]);
			modele_bn = &(ModeleBN[numeriseur->def.type]);
			nom = modele_det->regl[n].nom;
			// printf("(%s) Reglage %s %s: Numeriseur %s, au format %d\n",__func__,nom,VoieManip[voie].nom,numeriseur->nom,modele_bn->acces.mode);
			num = regl->ress;
			if((num < 0) || (num >= modele_bn->nbress)) {
				printf("! le reglage '%s' de %s n'a pas de ressource numeriseur associee\n",nom,detectr->nom);
				continue;
			}
			NumeriseurRessHexaChange(numeriseur,num);
			// if(regl->cmde == REGL_MODUL) printf("(%s) modulation[%s] = 0x%04X\n",__func__,VoieManip[voie].nom,VoieTampon[voie].simu.modul);
			// else if(regl->cmde == REGL_COMP) printf("(%s) compensation[%s] = 0x%04X\n",__func__,VoieManip[voie].nom,VoieTampon[voie].simu.compens);
			ress = modele_bn->ress + num;
			/* chargement du detecteur */
			if(charge_auto) {
				NumeriseurChargeRessource(numeriseur,num,"          |");
				/* y.c. ressource opposee (si differentiel) */
				if(ress->type == RESS_FLOAT) fval = numeriseur->ress[num].val.r;
				else ival = numeriseur->ress[num].val.i;
				if((k = regl->ressneg) >= 0) {
					if(modele_bn->ress[k].type == RESS_FLOAT) numeriseur->ress[k].val.r = -fval;
					else numeriseur->ress[k].val.i = -ival;
					NumeriseurRessValChangee(numeriseur,k);
					NumeriseurChargeRessource(numeriseur,k,"          |");
				}
				if((k = regl->resspos) >= 0) {
					if(modele_bn->ress[k].type == RESS_FLOAT) numeriseur->ress[k].val.r = fval;
					else numeriseur->ress[k].val.i = ival;
					NumeriseurRessValChangee(numeriseur,k);
					NumeriseurChargeRessource(numeriseur,k,"          |");
				}
			}
			if(col < 0) col = 0; else if(col >= MAXREGLAGECOLS) col = 0;
			x = xc[col];
			y = yc[col];
			if(j >= NUMER_INSTRM_MAX) break;
			mode = (regl->ss_adrs == 0xFF)? PNL_RDONLY|PNL_DIRECT: PNL_DIRECT;
			PanelMode(panel = PanelCreate(2),mode); PanelSupport(panel,WND_CREUX);
			if(modele_bn->acces.mode != NUMER_ACCES_TXT) {
				/* valeur envoyee telle quelle */
				i = PanelSHex(panel,nom,(short *)&(numeriseur->ress[num].hval));
				PanelLngr(panel,i,NUMER_DIGITS_DAC);
				if(regl->ss_adrs == 0xFF) PanelItemSelect(panel,i,0);
				else PanelItemExit(panel,i,NumeriseurRessFromADU,(void *)(long)j);
			}
			/* valeur au format utilisateur */
			if(ress->type == RESS_CLES) {
				i = PanelKey(panel,ress->unite,ress->lim_k.cles,&(numeriseur->ress[num].val.i),NUMER_RESS_VALTXT);
				if(regl->ss_adrs == 0xFF) PanelItemSelect(panel,i,0);
				else {
					PanelItemModif(panel,i,NumeriseurRessEnCours,(void *)(long)j);
					PanelItemExit(panel,i,NumeriseurRessFromKey,(void *)(long)j);
				}
			} else {
				i = PanelText(panel,ress->unite,numeriseur->ress[num].cval,NUMER_RESS_VALTXT);
				if(regl->ss_adrs == 0xFF) PanelItemSelect(panel,i,0);
				else {
					PanelItemModif(panel,i,NumeriseurRessEnCours,(void *)(long)j);
					PanelItemExit(panel,i,NumeriseurRessFromUser,(void *)(long)j);
				}
			}
			PanelLngr(panel,i,NUMER_DIGITS_DAC);
			/* place dans la planche */
			m = strlen(ress->unite); l = strlen(nom); if(l < m) l = m;
			xp = xl[col] - 15 - l; if(xp < 0) xp = 0;
			// printf("    position #%d, soit p=(%d, %d)\n",col+1,(x+xp)*Dx,y);
			BoardAddPanel(VoieInfo[voie].planche,panel,(x+xp)*Dx,y,0);
			NumeriseurInstr[j].type = OPIUM_PANEL; NumeriseurInstr[j].instrum = (void *)panel;
			NumeriseurInstr[j].numeriseur = numeriseur; NumeriseurInstr[j].num = num; NumeriseurInstr[j].reglage = regl;
			j++;
			xs = xp + l + NUMER_DIGITS_DAC;
			if(ress->type == RESS_FLOAT) {
				instrum = InstrumFloat(INS_GLISSIERE,(void *)&ReglageGliss21c,&(numeriseur->ress[num].val.r),ress->lim_r.min,ress->lim_r.max);
				InstrumOnChange(instrum,NumeriseurRessFromVal,(void *)(long)j);
				// BoardAddInstrum(VoieInfo[voie].planche,instrum,(x+xs+3-InstrumRectLngr(&ReglageGliss21c))*Dx,y+(2*Dy)+5,0);
				BoardAddInstrum(VoieInfo[voie].planche,instrum,(x+xp)*Dx,y+(2*Dy)+5,0);
				NumeriseurInstr[j].type = OPIUM_INSTRUM; NumeriseurInstr[j].instrum = (void *)instrum;
				NumeriseurInstr[j].numeriseur = numeriseur; NumeriseurInstr[j].num = num; NumeriseurInstr[j].reglage = regl;
				j++;
				ys = 2;
			} else ys = 0;
			if(regl->ss_adrs != 0xFF) {
				if(j >= NUMER_INSTRM_MAX) break;
				menu = MenuLocalise(iNumeriseurRessUpDown); OpiumSupport(menu->cdr,WND_PLAQUETTE);
				BoardAddMenu(VoieInfo[voie].planche,menu,(x+xs+3)*Dx,y,0);
				NumeriseurInstr[j].type = OPIUM_MENU; NumeriseurInstr[j].instrum = (void *)menu;
				NumeriseurInstr[j].numeriseur = numeriseur; NumeriseurInstr[j].num = num; NumeriseurInstr[j].reglage = regl;
				j++;
			}

			yc[col] += ((ys + 3) * Dy); /* deux lignes dans le panel [+ys si glissiere] + 1 pour separer */
			if(bas_boutons < yc[col]) bas_boutons = yc[col];
			col++; if(col >= MAXREGLAGECOLS) col = 0;
			if(col == 0) {
				for(col=0; col<MAXREGLAGECOLS; col++) yc[col] = bas_boutons; /* toutes les lignes au meme niveau, tant pis pour la perte de place eventuelle */
				col = 0;
			}
		}
		BoardAreaClose(VoieInfo[voie].planche,&DetecCadreRegl,&xf,&yf); yf += (2 * Dy);
		if(charge_auto) {
			NumeriseurAutoriseAcces(numeriseur,0,2);
			VoieInfo[voie].validite = 1;
		}
	} else VoieInfo[voie].status = 0;
reglages_vus:

	NumeriseurInstrNb = j;
	VoieInfo[voie].nb = NumeriseurInstrNb - VoieInfo[voie].prems;

	x = bord_gauche + (2 * Dx); y = yf; // bas_boutons;
	if(VoieInfo[voie].complete == 2) x += (3 * Dx);
	if(VoieInfo[voie].complete) {
		BoardAreaOpen(L_("Mesure du signal"),WND_PLAQUETTE);
		if(VoieManip[voie].modulee) {
			PanelMode(panel = PanelCreate(4),PNL_DIRECT); PanelSupport(panel,WND_CREUX);
			PanelItemSelect(panel,PanelFloat(panel,L_("moyenne"),&VoieInfo[voie].moyenne),0);
			PanelItemSelect(panel,PanelFloat(panel,L_("bruit"),&VoieInfo[voie].bruit),0);
			PanelItemSelect(panel,PanelFloat(panel,L_("compensation"),&VoieInfo[voie].compens),0);
			// PanelItemSelect(panel,PanelKeyB (panel,"PulseTubes",L_("arretes/actifs"),&EtatTubes,10),(AutomLink == -2));
			VoieInfo[voie].stats = panel;
			BoardAddPanel(VoieInfo[voie].planche,panel,x,y,0);
			x += (30 * Dx);

			PanelMode(panel = PanelCreate(5),PNL_DIRECT);
			//	PanelItemSelect(panel,PanelChar (panel,"Adresse",&BoloAdrsCourante),0);
			PanelItemSelect(panel,PanelFloat(panel,"Ibolo (nA)",&VoieInfo[voie].I),0);
			PanelItemSelect(panel,PanelFloat(panel,"Vbolo (mV)",&VoieInfo[voie].V),0);
			PanelItemSelect(panel,PanelFloat(panel,"Rbolo (MO)",&VoieInfo[voie].R),0);
			PanelItemSelect(panel,PanelFloat(panel,"Temper (K)",T_Bolo),(AutomLink == -2));
			PanelSupport(panel,WND_CREUX);
			VoieInfo[voie].vi = panel;
			BoardAddPanel(VoieInfo[voie].planche,panel,x,y,0);
			x += (28 * Dx);

			menu = MenuLocalise(iReglageMenuModul); OpiumSupport(menu->cdr,WND_PLAQUETTE);
			VoieInfo[voie].menu_modul = menu;
			BoardAddMenu(VoieInfo[voie].planche,menu,x,y,0);

			PanelMode(panel = PanelCreate(1),PNL_DIRECT);
			i = PanelInt(panel,L_("marge"),&(VoieManip[voie].def.trmt[TRMT_AU_VOL].parm));
			PanelLngr(panel,i,5);
			PanelSupport(panel,WND_CREUX);
			BoardAddPanel(VoieInfo[voie].planche,panel,x+(18*Dx),y,0); // x,y+(5*Dx)
			y += (5 * Dy);

			// BoardAddMenu(VoieInfo[voie].planche,MenuBouton("Shuffler",MNU_FONCTION ReglageShuffler),x+(22*Dx),y+(2*Dy),0);
		} else {
			PanelMode(panel = PanelCreate(5),PNL_DIRECT); PanelSupport(panel,WND_CREUX); PanelColumns(panel,2);
			PanelItemSelect(panel,PanelFloat(panel,L_("moyenne (ADU)"),&VoieInfo[voie].moyenne),0);
			PanelItemSelect(panel,PanelFormat(panel,PanelFloat(panel,L_("bruit (ADU)"),&VoieInfo[voie].bruit),"%.2f"),0);
			PanelItemSelect(panel,PanelFormat(panel,PanelFloat(panel,L_("pic-pic (ADU)"),&VoieInfo[voie].pic2pic),"%.2f"),0);
			PanelItemSelect(panel,PanelLngr(panel,PanelFormat(panel,PanelFloat(panel,"(V)",&VoieInfo[voie].mesure_enV),"%.6f"),10),0);
			PanelItemSelect(panel,PanelLngr(panel,PanelFormat(panel,PanelFloat(panel,"(mV)",&VoieInfo[voie].bruit_mV),"%.3f"),10),0);
			VoieInfo[voie].stats = panel;
			BoardAddPanel(VoieInfo[voie].planche,panel,x,y,0); x += (45 * Dx);

			ReglagePulseTest(0,0,(void *)(long)voie);
			PanelMode(panel = PanelCreate(3),PNL_DIRECT); PanelSupport(panel,WND_CREUX); // PanelColumns(panel,3);
			i = PanelFloat(panel,L_("Pulse injecte (mV)"),&(VoieManip[voie].calib.injection));
			PanelItemExit(panel,i,ReglagePulseTest,(void *)(long)voie);
			i = PanelFloat(panel,L_("Capa couplage (pF)"),&(VoieManip[voie].calib.couplage));
			PanelItemExit(panel,i,ReglagePulseTest,(void *)(long)voie);
			i = PanelFloat(panel,L_("Gain ampli (V/pC)"),&VoieManip[voie].calib.gain_ampli);
			PanelItemExit(panel,i,ReglagePulseTest,(void *)(long)voie);
			BoardAddPanel(VoieInfo[voie].planche,panel,x,y,0); x += (33 * Dx);

			PanelMode(panel = PanelCreate(2),PNL_DIRECT); PanelSupport(panel,WND_CREUX);
			PanelItemSelect(panel,PanelLngr(panel,PanelFloat(panel,"Evt (ADU)",&(VoieManip[voie].calib.mesure)),8),0);
			PanelItemSelect(panel,PanelLngr(panel,PanelFloat(panel,L_("Bruit (e)"),&VoieInfo[voie].bruit_e),8),0);
			VoieInfo[voie].electrons = panel;
			BoardAddPanel(VoieInfo[voie].planche,panel,x,y,0);
			y += (4 * Dy);
		}
		BoardAreaClose(VoieInfo[voie].planche,&DetecCadreMesr,&xf,&yf);
	}
	y = yf + Dy;

#ifdef REGLAGE_DET_AVEC_SIGNAL
	if(VoieInfo[voie].complete) {
		if(!VoieInfo[voie].oscillo) VoieInfo[voie].oscillo = OscilloCreate(voie);
		if(VoieInfo[voie].oscillo) {
			int gche_oscillo,haut_oscillo; int numadc,min,max; char avec_trigger;
			gche_oscillo = 2 * Dx; haut_oscillo = y + Dy; avec_trigger = (VoieInfo[voie].complete == 2);
			if(bb >= 0) {
				numadc = modele_bn->adc[vbb];
				max = (2 << (ModeleADC[numadc].bits - 1)) - 1;
				if(ModeleADC[numadc].signe) min = -max - 1; else min = 0;
			} else { max = (2 << 13) - 1; min = -max - 1; } /* trouver le no commun d'ADC */
			OscilloInsere(VoieInfo[voie].oscillo,VoieInfo[voie].planche,min,max,gche_oscillo,haut_oscillo,VoieManip[voie].modulee,avec_trigger);
			(VoieInfo[voie].oscillo)->acq = Acquis[AcquisLocale].etat.active;
			// gReglageSignal[voie] = (VoieInfo[voie].oscillo)->ecran;
		}
	}
#endif
	OpiumExitFctn(VoieInfo[voie].planche,ReglagePlancheStoppe,(void *)(long)voie);

	return(0);
}
/* ========================================================================== */
static char DetecteurVoieChoisie(char *titre) {
    DetecteurListeCapteurs(0,0,0);
    if((BoloNb > 1) || (Bolo[BoloNum].nbcapt > 1)) {
		OpiumTitle(pDetecteurVoie->cdr,titre);
        if(OpiumExec(pDetecteurVoie->cdr) == PNL_CANCEL) return(0);
    } else BoloNum = CapteurNum = 0;
    return(1);
}
/* ========================================================================== */
int DetecteurOscillo(Menu menu, MenuItem *item) {
    if(DetecteurVoieChoisie(item->traduit)) ReglageDeLaVoie(BoloNum,CapteurNum,2);
    return(0);
}
/* ========================================================================== */
int DetecteurReglages(Menu menu, MenuItem *item) {
    if(DetecteurVoieChoisie(item->traduit)) ReglageDeLaVoie(BoloNum,CapteurNum,-1);
    return(0);
}
/* ========================================================================== */
static void ReglageDeLaVoie(int bolo, int cap, char avec_trigger) {
	TypeDetecteur *detectr; TypeReglage *regl; int n,voie,bb,num; float fval;
	TypeNumeriseur *numeriseur; TypeBNModele *modele_bn; TypeBNModlRessource *ress;
	
	detectr = &(Bolo[bolo]);
	voie = detectr->captr[cap].voie;
//	if((bb = detectr->captr[cap].bb.num)) {
//		numeriseur = &(Numeriseur[bb]);
//		modele_bn = &(ModeleBN[Numeriseur[bb].def.type]);
		for(n=0; n<detectr->nbreglages; n++) {
			regl = &(detectr->reglage[n]);
			if((regl->capt == cap) && ((num = regl->ress) >= 0) && ((regl->cmde == REGL_MODUL) || (regl->cmde == REGL_COMP))) {
				if((bb = regl->bb) < 0) continue;
				numeriseur = &(Numeriseur[bb]);
				modele_bn = &(ModeleBN[numeriseur->def.type]);
				ress = modele_bn->ress + num;
				if(ress->type == RESS_FLOAT) fval = numeriseur->ress[num].val.r;
				else fval = (float)numeriseur->ress[num].val.i;
				if(regl->cmde == REGL_MODUL) {
					if(VoieManip[voie].Rmodul != 0.0) VoieInfo[voie].I = 1000.0 * fval / VoieManip[voie].Rmodul; /* nA */
					else VoieInfo[voie].I = 0.0;
					ReglIbolo = VoieInfo[voie].I;
				} else /* (regl->cmde == REGL_COMP) */{
					if(Acquis[AcquisLocale].etat.active) fval += (COMP_COEF * VoieInfo[voie].mesure_enV);
					VoieInfo[voie].V = 1000.0 * fval / VoieManip[voie].gain_fixe; /* mV */
					ReglVbolo = VoieInfo[voie].V;
					if(VoieInfo[voie].I != 0.0) VoieInfo[voie].R = VoieInfo[voie].V / VoieInfo[voie].I; /* MO */
					else VoieInfo[voie].R = 0.0;
					ReglTbolo = *T_Bolo;
					if(VoieInfo[voie].vi) { if(OpiumDisplayed((VoieInfo[voie].vi)->cdr)) PanelRefreshVars(VoieInfo[voie].vi); }
				}
				numeriseur->status.a_copier = 1;
			}
		}
		//? Numeriseur[bb].status.a_copier = 1;
//	}
    if(avec_trigger >= 0) {
        VoieInfo[voie].complete = avec_trigger? 2: 1;
        if(VoieInfo[voie].planche) OpiumRefresh(VoieInfo[voie].planche);
    }
    if(!VoieInfo[voie].planche) ReglagePlancheCree(voie);
	//- NumeriseurAutoriseAcces(&(Numeriseur[bb]),1,1);
	OpiumUse(VoieInfo[voie].planche,OPIUM_FORK);
	OpiumUserAction();
}
/* ========================================================================== */
int ReglageClique(Figure fig, WndUserRequest *e) {
	int voie,bolo,cap,bb;

	voie = (int)(fig->adrs);
	if(voie < 0) return(0);
	if(!VoieInfo[voie].planche) ReglagePlancheCree(voie);
	bolo = VoieManip[voie].det;
	cap = VoieManip[voie].cap;
	bb = Bolo[bolo].captr[cap].bb.num;
	if(bb >= 0) Numeriseur[bb].status.a_copier = 1;
	//- NumeriseurAutoriseAcces(&(Numeriseur[bb]),1,1);
	OpiumUse(VoieInfo[voie].planche,OPIUM_FORK);
	OpiumUserAction();

	return(0);
}
/* ========================================================================== */
int DetecteurVoieModelesActifs() {
	int nb,vm;

	nb = 3;
	if(ModeleVoieNb <= nb) {
		for(vm=0; vm<ModeleVoieNb; vm++) ModeleVoie[vm].utilisee = 1;
		nb = ModeleVoieNb;
	} else if(pDetecVoieModl) {
		if(OpiumExec(pDetecVoieModl->cdr) == PNL_CANCEL) return(0);
		nb = 0; for(vm=0; vm<ModeleVoieNb; vm++) if(ModeleVoie[vm].utilisee) nb++;
	} else {
		OpiumWarn(L_("Fenetre manquante, affichage restreint aux %d premieres voies"),nb);
		for(vm=0; vm<ModeleVoieNb; vm++) ModeleVoie[vm].utilisee = (vm < nb);
	}
	return(nb);
}
/* ========================================================================== */
int DetecteurListeCapteurs(Panel panel, int num, void *arg) {
	int cap;
#ifdef UTILISE_MODELES
	int dm;
#endif

	if(panel) { /* if(PanelItemEstModifie(panel,num)) */ PanelItemRelit(panel,num); }
	if((BoloNum < 0) || (BoloNum >= BoloNb)) { BoloNum = 0; return(0); }
#ifdef UTILISE_MODELES
	dm = Bolo[BoloNum].type;
	for(cap=0; cap<ModeleDet[dm].nbcapt; cap++) {
		CapteurNom[cap] = ModeleVoie[ModeleDet[dm].type[cap]].nom;
	}
	CapteurNom[cap] = "\0";
	if(CapteurNum >= ModeleDet[dm].nbcapt) CapteurNum = 0;
#else /* utilise les noms "locaux" */
	for(cap=0; cap<Bolo[BoloNum].nbcapt; cap++) {
		CapteurNom[cap] = Bolo[BoloNum].captr[cap].nom;
	}
	CapteurNom[cap] = "\0";
	if(CapteurNum >= Bolo[BoloNum].nbcapt) CapteurNum = 0;
#endif
	if(panel) { if(OpiumDisplayed(panel->cdr)) PanelRefreshVars(panel); }
	return(1);
}
/* ========================================================================== */
static void GegeneChangeFait(int simu) {
	SimuModifiees = 1;
	if(OpiumDisplayed(Simu[simu].panneau.bouton.svg)) {
		MenuItemAllume((Menu)(Simu[simu].panneau.bouton.svg->adrs),1,0,GRF_RGB_YELLOW);
	}
}
/* ========================================================================== */
static int GegeneChangeInstrumLdb(Instrum instrum, int simu) {
	PanelRefreshVars(Simu[simu].panneau.texte.ldb);
	GegeneChangeFait(simu);
	return(0);
}
/* ========================================================================== */
static int GegeneChangePanelLdb(Panel panel, int item, void *arg) {
	int simu;

	simu = (int)arg;
	if(Simu[simu].ldb > 32000.0) Simu[simu].ldb = 32000.0;
	else if(Simu[simu].ldb < -32000.0) Simu[simu].ldb = -32000.0;
	PanelRefreshVars(Simu[simu].panneau.texte.ldb);
	InstrumRefreshVar(Simu[simu].panneau.glissiere.ldb);
	GegeneChangeFait(simu);
	return(0);
}
/* ========================================================================== */
static int GegeneChangeInstrumBrt(Instrum instrum, int simu) {
	PanelRefreshVars(Simu[simu].panneau.texte.brt);
	GegeneChangeFait(simu);
	return(0);
}
/* ========================================================================== */
static int GegeneChangePanelBrt(Panel panel, int item, void *arg) {
	int simu;

	simu = (int)arg;
	if(Simu[simu].bruit > 500.0) Simu[simu].bruit = 500.0;
	else if(Simu[simu].bruit < 0.0) Simu[simu].bruit = 0.0;
	PanelRefreshVars(Simu[simu].panneau.texte.brt);
	InstrumRefreshVar(Simu[simu].panneau.glissiere.brt);
	GegeneChangeFait(simu);
	return(0);
}
/* ========================================================================== */
static int GegeneChangeInstrumFrq(Instrum instrum, int simu) {

	if(Simu[simu].freq > 1000.0) Simu[simu].freq = 1000.0;
	else if(Simu[simu].freq < 0.01) Simu[simu].freq = 0.01;
	Simu[simu].periode = 1.4 * Echantillonnage / (Simu[simu].freq / 1000.0);
	PanelRefreshVars(Simu[simu].panneau.texte.frq);
	GegeneChangeFait(simu);
	return(0);
}
/* ========================================================================== */
static int GegeneChangePanelFrq(Panel panel, int item, void *arg) {
	int simu;

	simu = (int)arg;
	if(Simu[simu].freq > 1000.0) Simu[simu].freq = 1000.0;
	else if(Simu[simu].freq < 0.01) Simu[simu].freq = 0.01;
	Simu[simu].periode = 1.4 * Echantillonnage / (Simu[simu].freq / 1000.0);
	PanelRefreshVars(Simu[simu].panneau.texte.frq);
	InstrumRefreshVar(Simu[simu].panneau.glissiere.frq);
	GegeneChangeFait(simu);
	return(0);
}
/* ========================================================================== */
static int GegeneChangeInstrumAmp(Instrum instrum, int simu) {
	PanelRefreshVars(Simu[simu].panneau.texte.amp);
	GegeneChangeFait(simu);
	return(0);
}
/* ========================================================================== */
static int GegeneChangePanelAmp(Panel panel, int item, void *arg) {
	int simu;

	simu = (int)arg;
	if(Simu[simu].pic > 32000.0) Simu[simu].pic = 32000.0;
	else if(Simu[simu].pic < 1.0) Simu[simu].pic = 1.0;
	PanelRefreshVars(Simu[simu].panneau.texte.amp);
	InstrumRefreshVar(Simu[simu].panneau.glissiere.amp);
	GegeneChangeFait(simu);
	return(0);
}
/* ========================================================================== */
static int GegeneChangeInstrumRes(Instrum instrum, int simu) {
	PanelRefreshVars(Simu[simu].panneau.texte.res);
	GegeneChangeFait(simu);
	return(0);
}
/* ========================================================================== */
static int GegeneChangePanelRes(Panel panel, int item, void *arg) {
	int simu;

	simu = (int)arg;
	if(Simu[simu].reso > 100.0) Simu[simu].reso = 50.0;
	else if(Simu[simu].reso < 0.0) Simu[simu].reso = 0.0;
	PanelRefreshVars(Simu[simu].panneau.texte.res);
	InstrumRefreshVar(Simu[simu].panneau.glissiere.res);
	GegeneChangeFait(simu);
	return(0);
}
/* ========================================================================== */
static void GegeneMontre(int simu) {
	int i,j,nb; Panel panel; char titre[80];
	int voie; char afficher_entete;

	if(Simu[simu].panneau.avant == 0) {
		Simu[simu].panneau.avant = BoardCreate();
		snprintf(titre,80,L_("Generateur %s"),Simu[simu].nom);
		OpiumTitle(Simu[simu].panneau.avant,titre);

		nb = 0; for(j=0; j<Simu[simu].nbvoies; j++) if(Simu[simu].voie[j] >= 0) nb++;
		if(!nb) { OpiumWarn(L_("Le generateur %s ne genere de donnees dans aucune voie"),Simu[simu].nom); return; }
		afficher_entete = 1;
		if(nb > 1) nb++;
		panel = PanelCreate(2*nb); PanelColumns(panel,2);
		PanelMode(panel,PNL_RDONLY|PNL_DIRECT|PNL_BYLINES); PanelSupport(panel,WND_CREUX);
		for(j=0; j<Simu[simu].nbvoies; j++) if((voie = Simu[simu].voie[j]) >= 0) {
			if(nb == 1) {
				PanelItemSelect(panel,PanelText(panel,L_("Detecteur"),Bolo[VoieManip[voie].det].nom,DETEC_NOM),0);
				PanelItemSelect(panel,PanelText(panel,L_("voie"),VoieManip[voie].prenom,MODL_NOM),0);
			} else {
				if(afficher_entete) {
					i = PanelItemSelect(panel,PanelText(panel,0,L_("Detecteur"),DETEC_NOM),0); PanelItemSouligne(panel,i,1);
					i = PanelItemSelect(panel,PanelText(panel,0,L_("voie"),MODL_NOM),0); PanelItemSouligne(panel,i,1);
					afficher_entete = 0;
				}
				// PanelItemSelect(panel,PanelText(panel,0,Bolo[VoieManip[voie].det].nom,DETEC_NOM),0);
				// PanelItemSelect(panel,PanelText(panel,0,VoieManip[voie].prenom,MODL_NOM),0);
				PanelText(panel,0,Bolo[VoieManip[voie].det].nom,DETEC_NOM);
				PanelText(panel,0,VoieManip[voie].prenom,MODL_NOM);
			}
		}
		BoardAddPanel(Simu[simu].panneau.avant,panel,0,Dy,0);

		// avertir si VoieTampon[voie].unite.taille == 0
		panel = Simu[simu].panneau.texte.msg = PanelCreate(1);
		PanelMode(panel,PNL_RDONLY|PNL_DIRECT);
		PanelSupport(panel,WND_CREUX);
		PanelItemSelect(panel,PanelText(panel,0,Simu[simu].panneau.message,GEGENE_MSG_LNGR),0);
		PanelItemColors(panel,1,SambaVertJauneOrangeRouge,2);
		BoardAddPanel(Simu[simu].panneau.avant,panel,OPIUM_EN_DESSOUS_DE 0);

		// periode = Simu[simu].periode; pic = Simu[simu].pic; reso = Simu[simu].reso;
		Simu[simu].panneau.glissiere.ldb = InstrumFloat(INS_GLISSIERE,(void *)&Gliss12lignes,&(Simu[simu].ldb),-32000.0,32000.0);
		InstrumTitle(Simu[simu].panneau.glissiere.ldb,L_("LdB"));
		InstrumOnChange(Simu[simu].panneau.glissiere.ldb,GegeneChangeInstrumLdb,(void *)(long)simu);
		BoardAddInstrum(Simu[simu].panneau.avant,Simu[simu].panneau.glissiere.ldb,OPIUM_EN_DESSOUS_DE 0); // 0,5 * Dy,0);
		panel = Simu[simu].panneau.texte.ldb = PanelCreate(1);
		PanelMode(panel,PNL_DIRECT); PanelSupport(panel,WND_CREUX);
		PanelFloat(panel,0,&(Simu[simu].ldb)); PanelFormat(panel,1,"%5.1f"); PanelLngr(panel,1,7);
		PanelItemExit(Simu[simu].panneau.texte.ldb,1,GegeneChangePanelLdb,(void *)(long)simu);
		BoardAddPanel(Simu[simu].panneau.avant,panel,OPIUM_EN_DESSOUS_DE 0);

		Simu[simu].panneau.glissiere.brt = InstrumFloat(INS_GLISSIERE,(void *)&Gliss12lignes,&(Simu[simu].bruit),0.0,500.0); // bruit = Simu[simu].bruit * 12.0;
		InstrumTitle(Simu[simu].panneau.glissiere.brt,L_("Bruit"));
		InstrumOnChange(Simu[simu].panneau.glissiere.brt,GegeneChangeInstrumBrt,(void *)(long)simu);
		BoardAddInstrum(Simu[simu].panneau.avant,Simu[simu].panneau.glissiere.brt,OPIUM_A_DROITE_DE Simu[simu].panneau.glissiere.ldb->cdr);
		panel = Simu[simu].panneau.texte.brt = PanelCreate(1);
		PanelMode(panel,PNL_DIRECT); PanelSupport(panel,WND_CREUX);
		PanelFloat(panel,0,&(Simu[simu].bruit)); PanelFormat(panel,1,"%5.1f"); PanelLngr(panel,1,7);
		PanelItemExit(Simu[simu].panneau.texte.brt,1,GegeneChangePanelBrt,(void *)(long)simu);
		BoardAddPanel(Simu[simu].panneau.avant,panel,OPIUM_EN_DESSOUS_DE 0);

		Simu[simu].panneau.glissiere.frq = InstrumFloat(INS_GLISSIERE,(void *)&Gliss12lignes,&(Simu[simu].freq),0.01,1000.0);
		InstrumTitle(Simu[simu].panneau.glissiere.frq,L_("Frequence")); InstrumGradLog(Simu[simu].panneau.glissiere.frq,1);
		InstrumOnChange(Simu[simu].panneau.glissiere.frq,GegeneChangeInstrumFrq,(void *)(long)simu);
		BoardAddInstrum(Simu[simu].panneau.avant,Simu[simu].panneau.glissiere.frq,OPIUM_A_DROITE_DE Simu[simu].panneau.glissiere.brt->cdr);
		panel = Simu[simu].panneau.texte.frq = PanelCreate(1);
		PanelMode(panel,PNL_DIRECT); PanelSupport(panel,WND_CREUX);
		PanelFloat(panel,0,&(Simu[simu].freq)); PanelFormat(panel,1,"%6.2f"); PanelLngr(panel,1,7);
		PanelItemExit(Simu[simu].panneau.texte.frq,1,GegeneChangePanelFrq,(void *)(long)simu);
		BoardAddPanel(Simu[simu].panneau.avant,panel,OPIUM_EN_DESSOUS_DE 0);

		Simu[simu].panneau.glissiere.amp = InstrumFloat(INS_GLISSIERE,(void *)&Gliss12lignes,&(Simu[simu].pic),1.0,32000.0);
		InstrumTitle(Simu[simu].panneau.glissiere.amp,L_("Amplitude")); InstrumGradLog(Simu[simu].panneau.glissiere.amp,1);
		InstrumOnChange(Simu[simu].panneau.glissiere.amp,GegeneChangeInstrumAmp,(void *)(long)simu);
		BoardAddInstrum(Simu[simu].panneau.avant,Simu[simu].panneau.glissiere.amp,OPIUM_A_DROITE_DE Simu[simu].panneau.glissiere.frq->cdr);
		panel = Simu[simu].panneau.texte.amp = PanelCreate(1);
		PanelMode(panel,PNL_DIRECT); PanelSupport(panel,WND_CREUX);
		PanelFloat(panel,0,&(Simu[simu].pic)); PanelFormat(panel,1,"%5.1f"); PanelLngr(panel,1,7);
		PanelItemExit(Simu[simu].panneau.texte.amp,1,GegeneChangePanelAmp,(void *)(long)simu);
		BoardAddPanel(Simu[simu].panneau.avant,panel,OPIUM_EN_DESSOUS_DE 0);

		Simu[simu].panneau.glissiere.res = InstrumFloat(INS_GLISSIERE,(void *)&Gliss12lignes,&(Simu[simu].reso),0.0,50.0);
		InstrumTitle(Simu[simu].panneau.glissiere.res,L_("Resolution"));
		InstrumOnChange(Simu[simu].panneau.glissiere.res,GegeneChangeInstrumRes,(void *)(long)simu);
		BoardAddInstrum(Simu[simu].panneau.avant,Simu[simu].panneau.glissiere.res,OPIUM_A_DROITE_DE Simu[simu].panneau.glissiere.amp->cdr);
		panel = Simu[simu].panneau.texte.res = PanelCreate(1);
		PanelMode(panel,PNL_DIRECT); PanelSupport(panel,WND_CREUX);
		PanelFloat(panel,0,&(Simu[simu].reso)); PanelFormat(panel,1,"%5.1f"); PanelLngr(panel,1,7);
		PanelItemExit(Simu[simu].panneau.texte.res,1,GegeneChangePanelRes,(void *)(long)simu);
		BoardAddPanel(Simu[simu].panneau.avant,panel,OPIUM_EN_DESSOUS_DE 0);

		Simu[simu].panneau.bouton.manu = BoardAddPoussoir(Simu[simu].panneau.avant,L_("Manuel"),&(Simu[simu].mode_manu),OPIUM_EN_DESSOUS_DE (Simu[simu].panneau.texte.frq)->cdr);
		BoardAddPoussoir(Simu[simu].panneau.avant,"Auto",&(Simu[simu].mode_auto),OPIUM_A_DROITE_DE OPIUM_PRECEDENT);
		Simu[simu].panneau.bouton.svg = BoardAddBouton(Simu[simu].panneau.avant,L_("Sauver"),GegeneSauve,OPIUM_EN_DESSOUS_DE (Simu[simu].panneau.texte.res)->cdr);
	}
	OpiumUse(Simu[simu].panneau.avant,OPIUM_FORK);
	OpiumUserAction();
}
/* ========================================================================== */
static int GegeneSauve(Menu menu, MenuItem *item) {
	int simu;

	SambaSauveSimu();
	for(simu=0; simu<SimuNb; simu++) if(OpiumDisplayed(Simu[simu].panneau.bouton.svg)) {
		MenuItemEteint((Menu)(Simu[simu].panneau.bouton.svg->adrs),1,0);
	}
	return(0);
}
/* ========================================================================== */
int DetecteurGegene(Menu menu, MenuItem *item) {
	if(!SimuNb) OpiumWarn(L_("Pas de generateur defini"));
	else {
		if(SimuNb > 1) {
			if(OpiumReadList(L_("Nom du generateur"),SimuListe,&SimuNum,23) == PNL_CANCEL) return(0);
		} else SimuNum = 0;
		GegeneMontre(SimuNum);
	}
	return(0);
}
/* ========================================================================== */
int DetecteursInit() {
	int voie,n;

/* Initialisation generale */
	/* ATTENTION: SambaLitMateriel (donc, DetecteursLit) est appele AVANT DetecteursInit() */
	/* Initialisation des variables scalaires */
	//- CmdeNum = 0;
	//- CmdeAdrs = 0;
	//- CmdeAction = CLUZEL_IMMEDIAT;
	//- CmdeDef[CMDE_D0].valeur = Diviseur0;
	//- CmdeDef[CMDE_D1].valeur = Diviseur1;

    ReglVi.dep = 0.05; ReglVi.arr = 0.5; ReglVi.pas = 2.0; ReglVi.oper = 1;
	ReglVi.cur = 0.0;
	ReglVi.attente = 2;
	ReglVi.actif = 0;
	ReglVi.arrete = 1;
	ReglVi.nb_mesures = 0; ReglVi.nb_voies = 0;
	for(n=0; n<DETEC_MAX; n++) ReglVi.g[n] = 0;
	ReglRazEtat = 0; /* libre */
    ReglageLog = 1;
	ReglAllumeUpdate = 0;
	ReglAllumeRazfet = 0;
	ReglRazDuree = 20; ReglRazEnCours = 0;
	ReglageComplet = 0;
	DetecDebugStd = 0;
	
	pEtatBolos = 0;
	OscilloOuvert = 0;
	OscilloRestartDemande = 0;
	OscilloAcqAllume = 0;
	OscilloLarg = 85 * Dx; OscilloHaut = 20 * Dy;
	for(voie=0; voie<VOIES_MAX; voie++) {
		VoieInfo[voie].planche = 0;
		VoieInfo[voie].menu_voie = VoieInfo[voie].menu_suite = VoieInfo[voie].menu_modul = VoieInfo[voie].menu_debloque = VoieInfo[voie].menu_raz = 0;
		VoieInfo[voie].stats = VoieInfo[voie].taux_evts = VoieInfo[voie].vi = 0;
		VoieInfo[voie].raz = VoieInfo[voie].status = VoieInfo[voie].electrons = 0;
		VoieInfo[voie].prems = 0;
		VoieInfo[voie].nb = 0;
		VoieInfo[voie].trigger = VoieInfo[voie].deltaADU = 0;
		VoieInfo[voie].trig_actif = 0;
		VoieInfo[voie].complete = 1;
		VoieInfo[voie].moyenne = VoieInfo[voie].bruit = VoieInfo[voie].mesure_enV = VoieInfo[voie].compens = 0.0;
		VoieInfo[voie].V = VoieInfo[voie].I = VoieInfo[voie].R = 0.0;
		VoieInfo[voie].mode = 0;
	#ifdef REGLAGE_DET_AVEC_SIGNAL
		VoieInfo[voie].oscillo = 0;
		// gReglageSignal[voie] = 0;
	#endif
	}
	memcpy(&ModelePhysLu,&ModelePhysDefaut,sizeof(TypeSignalPhysique));
	memcpy(&ModeleVoieLu,&ModeleVoieDefaut,sizeof(TypeVoieModele));
	
	DetecteurParmsRassembles = 0;
	DetecteurParmsNb = 0;
	DetecteurParmsExemple = 0;
	DetecteurParmsExplics = 0;
	mDetecParms = MenuLocalise(iDetecParms);
	mDetecParmsGenl = mDetecteurParmsGenlApply = mDetecteurParmsSoftApply = 0;
	for(n=0; n<DETEC_PARM_NBNATURES+1; n++) bDetecteurParms[n] = 0;
	
	DetecteurParmsCallback[DETEC_PARM_EVTS].ref_onglet = DetecteurParmsOngletEvts;
	DetecteurParmsCallback[DETEC_PARM_TVOL].ref_onglet = DetecteurParmsOngletTvol;
	DetecteurParmsCallback[DETEC_PARM_PREP].ref_onglet = DetecteurParmsOngletPrep;
	DetecteurParmsCallback[DETEC_PARM_TRGR].ref_onglet = DetecteurParmsOngletTrgr;
	DetecteurParmsCallback[DETEC_PARM_RGUL].ref_onglet = DetecteurParmsOngletRgul;
	DetecteurParmsCallback[DETEC_PARM_SOFT].ref_onglet = DetecteurParmsOngletSoft;

	DetecteurParmsCallback[DETEC_PARM_EVTS].standardise = DetecteurParmsStandardiseEvts;
	DetecteurParmsCallback[DETEC_PARM_TVOL].standardise = DetecteurParmsStandardiseTvol;
	DetecteurParmsCallback[DETEC_PARM_PREP].standardise = DetecteurParmsStandardisePrep;
	DetecteurParmsCallback[DETEC_PARM_TRGR].standardise = DetecteurParmsStandardiseTrgr;
	DetecteurParmsCallback[DETEC_PARM_RGUL].standardise = DetecteurParmsStandardiseRgul;
//	DetecteurParmsCallback[DETEC_PARM_SOFT].standardise = DetecteurParmsStandardiseSoft;
	
	DetecteurParmsCallback[DETEC_PARM_EVTS].applique = DetecteurParmsAppliqueEvts;
	DetecteurParmsCallback[DETEC_PARM_TVOL].applique = DetecteurParmsAppliqueTvol;
	DetecteurParmsCallback[DETEC_PARM_PREP].applique = DetecteurParmsAppliquePrep;
	DetecteurParmsCallback[DETEC_PARM_TRGR].applique = DetecteurParmsAppliqueTrgr;
	DetecteurParmsCallback[DETEC_PARM_RGUL].applique = DetecteurParmsAppliqueRgul;
//	DetecteurParmsCallback[DETEC_PARM_SOFT].applique = DetecteurParmsAppliqueSoft;
	
	
#ifdef ATTENTE_AVEC_TIMER
	CmdeCadenceMsg = TimerCreate(msg_fini,0);
#endif
	SambaDonneeMin = -32768; SambaDonneeMax = 32767;
	
	DetecParmsStdPtr = DetecParmsStdDesc;
	DetecParmsVsnPtr = DetecParmsVsnDesc;
	DetecParmsAscPtr = DetecParmsAscDesc;

/* Creation de l'interface */
	tCmdes = TermCreate(24,80,32 * KB);
	OpiumTitle(tCmdes->cdr,"Compte-rendu des commandes d'initialisation");

	pDetecteurVoie = PanelCreate(2);
	n = PanelList(pDetecteurVoie,L_("Detecteur"),BoloNom,&BoloNum,DETEC_NOM);
	PanelList(pDetecteurVoie,L_("Voie"),CapteurNom,&CapteurNum,MODL_NOM);
	PanelItemExit(pDetecteurVoie,n,DetecteurListeCapteurs,0);
	
	pDetecVoieModl = 0;

	InstrumRectDim(&ReglageGliss21c,(REGLAGE_LARGEUR - 3)*Dx,Dy);
	
    pReglViAuto = PanelCreate(5);
    PanelFloat(pReglViAuto,L_("Amplitude depart (V)"),&ReglVi.dep);
	PanelKeyB (pReglViAuto,L_("puis"),"increment/facteur",&ReglVi.oper,10);
    PanelFloat(pReglViAuto,L_("de"),&ReglVi.pas);
    PanelFloat(pReglViAuto,L_("jusqu'a (V)"),&ReglVi.arr);
	PanelInt  (pReglViAuto,L_("avec pause de (s)"),&ReglVi.attente);

	return(1);
}
/* ========================================================================== */
int DetecteursSetup() {
	int bolo,vm; int i,k,cols; char fin_de_serie;

	pDetecGestion = PanelCreate(BoloNb);
	for(bolo=0; bolo<BoloNb; bolo++) {
		i = PanelKeyB(pDetecGestion,Bolo[bolo].nom,DETEC_STATUTS,&(Bolo[bolo].lu),4);
		PanelItemColors(pDetecGestion,i,SambaRougeVertJaune,3);
	}
	
	pDetecVoieModl = PanelCreate(ModeleVoieNb);
	for(vm=0; vm<ModeleVoieNb; vm++) PanelItemColors(pDetecVoieModl,PanelBool(pDetecVoieModl,ModeleVoie[vm].nom,&(ModeleVoie[vm].utilisee)),SambaRougeVertJaune,2);

	cols = 3;
	pEtatBolos = PanelCreate(cols * (1 + BoloNb));
	PanelColumns(pEtatBolos,cols);
	PanelMode(pEtatBolos,PNL_DIRECT);
	i = PanelItemSelect(pEtatBolos,PanelText(pEtatBolos,0,L_("etat"),4),0); // le texte 'etat' est protege d'ou erreur dans PanelCode..
	//	PanelRemark(pEtatBolos,"etat");
	PanelItemSouligne(pEtatBolos,i,1);
	for(bolo=0, k=0; bolo<BoloNb; bolo++) {
		fin_de_serie = !((k++ + 1) % 3);
		i = PanelItemSelect(pEtatBolos,PanelKeyB(pEtatBolos,Bolo[bolo].nom,DETEC_STATUTS,&(Bolo[bolo].lu),4),0);
		PanelItemColors(pEtatBolos,i,SambaRougeVertJaune,3);
		PanelItemSouligne(pEtatBolos,i,fin_de_serie);
	}
	i = PanelItemSelect(pEtatBolos,PanelText(pEtatBolos,0,"cryo",4),0); //	PanelRemark(pEtatBolos,"cryo");
	PanelItemSouligne(pEtatBolos,i,1);
	for(bolo=0, k=0; bolo<BoloNb; bolo++) {
		fin_de_serie = !((k++ + 1) % 3);
		i = PanelItemSelect(pEtatBolos,PanelText(pEtatBolos,0,Bolo[bolo].adresse,5),0);
		PanelItemSouligne(pEtatBolos,i,fin_de_serie);
	}
	i = PanelItemSelect(pEtatBolos,PanelText(pEtatBolos,0,L_("hote"),4),0); //	PanelRemark(pEtatBolos,"hote");
	PanelItemSouligne(pEtatBolos,i,1);
	for(bolo=0, k=0; bolo<BoloNb; bolo++) {
		fin_de_serie = !((k++ + 1) % 3);
		i = PanelItemSelect(pEtatBolos,PanelText(pEtatBolos,0,Bolo[bolo].hote,12),0);
		PanelItemSouligne(pEtatBolos,i,fin_de_serie);
	}
	
	return(1);
}
/* ========================================================================== */
int DetecteursExit() {
	int bolo; char a_sauver;
	for(bolo=0; bolo<BoloNb; bolo++) if(Bolo[bolo].a_sauver) break;
	if(bolo >= BoloNb) return(1);
	else {
		a_sauver = 1;
		return(SambaSauve(&a_sauver,L_("reglages des detecteurs"),0,1,DetecteurSauveTousReglages));
	}
}
