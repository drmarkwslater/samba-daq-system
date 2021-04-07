#ifdef macintosh
	#pragma message(__FILE__)
#endif

// #define MODULES_RESEAU defini (ou non) dans environnement.h
// #define PAROLE
// #define BRANCHE_TESTS

#include <environnement.h>

/*  #include <stdio.h> */
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#ifdef CODE_WARRIOR_VSN
	#include <string.h>
	#include <time.h>
	#include <utsname.h>
#else
	#include <strings.h>
	#include <sys/time.h>
	#include <signal.h>
	#include <sys/utsname.h> /* pour uname */
	/* pour sysctl */
	#include <sys/types.h>
	#ifdef HAVE_SYS_SYSCTL_H
	#include <sys/sysctl.h>
	#else
	#include <linux/sysctl.h>
	#endif
#endif
#include <sys/resource.h>
#include <sys/shm.h>
#include <math.h>

/* backtrace */
#include <execinfo.h>
#include <stdio.h>

#include <sys/param.h>
#include <sys/mount.h>

#ifdef PAROLE
	#include <speech.h>
#endif

//- #pragma message("Compilation pour "chaine(APPLI_NAME))
#ifdef ForGasol
	#define ForOpiumGL
	#pragma message("Compilation avec OPENGL")
#endif

#define SAMBA_C
#define ARCHIVE_C
#define SNCF_C
#define WND_C

#include <defineVAR.h>
#include <claps.h>
#include <dateheure.h>
#include <timeruser.h>
#include <decode.h>
#include <dico.h>
#include <opium.h>
#include <impression.h>
#include <nanopaw.h>
unsigned long TickCount(void);

#ifdef macintosh
	#define ANNEE_2000 100
	#define SKTLIB
#endif /* macintosh */

#ifdef UNIX
	/* #define ANNEE_2000 2000*/
	#define ANNEE_2000 100
	#define SKTLIB
#endif /* UNIX */

static char *SambaAppli
#ifdef APPLI_DIR
	= chaine(APPLI_DIR)
#else  /* APPLI_DIR */
	= "executables/Samba"
#endif /* APPLI_DIR */
;

static char *SambaInfoSource
#ifdef COMPIL_DIR
	 = chaine(COMPIL_DIR)
#else  /* COMPIL_DIR */
	#ifdef APPLI_DIR
		= chaine(APPLI_DIR)"/Contents/Resources/"
	#else  /* APPLI_DIR */
		= "Info/Samba"
	#endif /* APPLI_DIR */
#endif /* COMPIL_DIR */
;

static char *SambaVersionCompilo
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

#ifdef CHAINE_VSN_EXTERNE
	#ifdef QUICKDRAW
		#define SambaVersionTexte ProjetSambaVersionStringAuto
	#endif /* QUICKDRAW */
	#ifdef OPENGL
		#define SambaVersionTexte ProjetGasolVersionStringAuto
	#endif /* QUICKDRAW */
	#ifdef X11
		#define SambaVersionTexte ProjetSamdixVersionStringAuto
	#endif  /* X11 */
	extern char SambaVersionTexte[];
#else  /* CHAINE_VSN_EXTERNE */
	static const unsigned char *SambaVersionTexte
	#ifdef SAMBA_C
		 = 
		#ifdef macintosh
			#ifdef CW5
				"@(#)PROGRAM:Samba/CW5, PROJECT:Acquisition EDELWEISS, DEVELOPER:MiG, BUILT:" __DATE__  " " __TIME__ "" "\n"
			#else
				"@(#)PROGRAM:Samba V" chaine(VERSION_PROJET) ", PROJECT:Acquisition EDELWEISS, DEVELOPER:MiG, BUILT:" __DATE__  " " __TIME__ "" "\n"
			#endif /* CW5 */
		#else  /* macintosh */
			"@(#)PROGRAM:Samba , PROJECT:Acquisition EDELWEISS, DEVELOPER:MiG, BUILT:" __DATE__  " " __TIME__ "" "\n"
		#endif /* macintosh */
	#endif /* SAMBA_C */
	;
#endif /* CHAINE_VSN_EXTERNE */

static char SambaAideDemandee;

#define GADGET
#ifdef GADGET
	static Info SambaPresentation;
#endif

#ifdef CODE_WARRIOR_VSN
	#include <SIOUX.h>
	#include <Memory.h>
#endif /* CODE_WARRIOR_VSN */

#include <samba.h>
#include <organigramme.h>
#include <repartiteurs.h>
#include <numeriseurs.h>
#include <cablage.h>
#include <detecteurs.h>
#include <export.h>
#include <sequences.h>
#include <archive.h>
#include <monit.h>
#include <autom.h>
#include <banc.h>
#include <diags.h>
#include <sncf.h>

#ifdef AVEC_PCI
	#ifdef XCODE
		#define UTILISE_DRIVER
		#define PCI_ID_DIRECT
		#include <PCI/Driver/DriverLib.h>
	#endif /* XCODE */
	#include <PciLib.h>
	#include <IcaDefine.h>
#endif /* AVEC_PCI */
#ifdef NIDAQ
	#include <NIDAQmxBase.h>
#endif /* NIDAQ */

#ifdef MODULES_RESEAU
	/* UNIX deactive dans wnd.h pour utiliser CARBON...?
	#ifndef UNIX
	#define UNIX
	#endif */
	#include <tcp/serveur.h>
	#include <carla/data.h>
	#include <carla/msgs.h>
#endif /* MODULES_RESEAU */
#include <objets_samba.h>

// extern char *__progname;

static struct timeval SambaHeureDebut,SambaHeureFin;
// static TypeItemVar SambaVar[4];
static int FrequenceTick;

static int  TailleSvr;
static int  PortEcoute;
static char InitBolos,DemandeInstalle;
static char  AdresseUtilisee[HOTE_NOM];
static char XcodeSambaDebugTxt[8];

#ifdef AVEC_PCI
	static char DmaDemande;
#endif

/*
 * Organisation generale du systeme; parametres propres a Samba
 */
static char SetupPrefs[MAXFILENAME],MaitrePrefs[MAXFILENAME];
static char NomGeneral[MAXFILENAME],CtlgPrefs[MAXFILENAME],RelecPrefs[MAXFILENAME];

ARCHIVE_VAR ArgDesc SambaDesc[] = {
	{ DESC_COMMENT "Systeme d'Acquisition Multi-Bolometres sur Apple" },
	{ DESC_COMMENT 0 },
	{ "boite",    DESC_INT       &NumDefaut,      "Numero de boite (repartiteur IP/carte ADC) par defaut, -1 si a ne pas utiliser" },
	{ DESC_COMMENT 0 },
	{ "P", DESC_STR(MAXFILENAME)  SetupPrefs,     "Repertoire des preferences (relatif a la racine, ou absolu)" },
	{ "H", DESC_DEVIENT("T") }, // (prochainement deprecie)
	{ "T", DESC_STR(MAXFILENAME)  TopDir,         "Racine des fichiers d'acquisition ('$top', relatif au repertoire de SambaArgs ou absolu)" },
	{ "M", DESC_STR(MAXFILENAME)  MaitrePrefs,    "Repertoire commun ('$maitre', relatif a la racine ou absolu)" },
	{ DESC_COMMENT 0 },
	{ "run",      DESC_STR(MAXFILENAME) SrceName, "Fichier a relire" },
	{ "origine",  DESC_KEY "simu/reel",&SrceType, "Origine des donnees (simu/reel)" },
	{ "o", DESC_NONE }, // DESC_KEY SrceCles,    &SrceType,   "Origine des donnees (neant/pci/edw0/edw1/edw2)" },
	{ "F", DESC_NONE }, // DESC_STR(MAXFILENAME)  SrceName,   "Fichier de donnees si origine=edw" },
//	{ "d", DESC_NONE }, // DESC_STR(MAXFILENAME)  SrceName,   "Fichier de donnees si origine=edw" },
	{ DESC_COMMENT 0 },
	{ "s", DESC_STR(MAXFILENAME)  ArchDir[STRM],  "Repertoire des donnees brutes [streams] (relatif a l'espace de sauvegarde)" },
	{ "e", DESC_STR(MAXFILENAME)  ArchDir[EVTS],  "Repertoire des evenements sauvegardes   (relatif a l'espace de sauvegarde)" },
	{ "K", DESC_STR(MAXFILENAME)  CtlgPrefs,      "Repertoire du catalogue   (relatif a la racine, ou absolu)" },
	{ "S", DESC_STR(MAXFILENAME)  RelecPrefs,     "Espace des donnees en relecture si origine=edw (relatif a la racine, ou absolu)" },
	{ DESC_COMMENT 0 },
	{ "conf",     DESC_STR(MAXFILENAME)      NomGeneral,       "Configuration generale (relatif au repertoire des preferences)" },
	{ "type",     DESC_NONE }, // DESC_KEY "local/maitre/s1/s2/s3/s4/s5/s6/s7/s8/s9", &HoteNum, "Type d'hote (local/maitre/s1/s2/s3/s4/s5/s6/s7/s8/s9)" },
	{ "ip",       DESC_STR(HOTE_NOM)         AdresseUtilisee,  "Adresse IP de l'hote sur le reseau des repartiteurs (celle de la 1ere interface, par defaut)" },
	{ "mode",     DESC_KEY SAMBA_MODE_CLES, &SambaMode,        "Mode de fonctionnement de SAMBA ("SAMBA_MODE_CLES")" },
	{ "ecoute",   DESC_INT                  &PortEcoute,       "Port d'entree des commandes du maitre" },
	{ "starter",  DESC_STR(MAXFILENAME)      Starter,          "Nom de l'hote declenchant la lecture si multitache inactif" },
	{ "x",        DESC_BOOL                 &ExpertConnecte,   "Execution en mode expert" },
	{ "init",     DESC_BOOL                 &InitBolos,        "Chargement des detecteurs au demarrage de SAMBA" },
	{ "installe", DESC_BOOL                 &DemandeInstalle,  "Creation du setup par defaut (-create inclus)" },
	{ "debugs",   DESC_INT                  &DebugRestant,     "Nombre maximum d'impressions de deverminage" },
	{ "orga",     DESC_STR(MAXFILENAME)     &Organisation,     "Copie prealable du fichier .samba_top_<orga> dans .samba_top" },
	{ "create",   DESC_BOOL                 &CreationFichiers, "Creation fichiers si absents" },
	{ "batch",    DESC_BOOL                 &ModeBatch,        "Execution en mode batch" },

	/* Organisation transmise a TANGO, si appele via SAMBA */
	{ "E",        DESC_STR(MAXFILENAME)      SauvPrefs,        "Espace de sauvegarde des donnees (relatif a la racine, ou absolu)" },
	{ "R",        DESC_STR(MAXFILENAME)      ResuPrefs,        "Repertoire des resultats  (relatif a la racine, ou absolu)" },
	{ "G",        DESC_STR(MAXFILENAME)      FigsPrefs,        "Repertoire des graphiques (relatif a la racine, ou absolu)" },
	{ "D",        DESC_STR(MAXFILENAME)      DbazPrefs,        "Racine de la base de donnees des detecteurs (relatif a la racine, ou absolu)" },
	{ "unix",     DESC_BOOL                 &SousUnix,         "Execution sous UNIX (SambaArgs optionnel)" },
	{ DESC_COMMENT "Si pas batch:" },
	{ "display",  DESC_STR(MAXFILENAME)      ServeurX11,       "Serveur X11 a utiliser pour l'affichage" },
	{ "info",     DESC_STR(MAXFILENAME)      InfoPrefs,        "Repertoire des informations complementaires (relatif a la racine, ou absolu)" },
	{ "langages", DESC_NONE }, // DESC_STR(MAXFILENAME)  LangDir, "Repertoire des dictionnaires (relatif a la racine, ou absolu)" },
	{ "langue",   DESC_STR(MAXFILENAME)      LangueDemandee,   "Langage a utiliser dans les dialogues" },
	{ "dico",     DESC_KEY "net/fixe/maj",  &LangueEtendDico,  "maj: complete avec les derniers ajouts/ net: retire les termes non traduits (net/fixe/maj)" },
	{ "noir",     DESC_BOOL                 &FondNoir,         "Fond noir, texte blanc pour les fenetres de dialogue" },
	{ "fontname", DESC_STR(MAXFILENAME)      SambaFontName,    "Type de caracteres" },
	{ "fontsize", DESC_INT                  &SambaFontSize,    "Taille des caracteres" },
	{ "qualite",  DESC_KEY "papier/ecran",  &FondPlots,        "Qualite des graphiques (papier/ecran)" },
	{ "linesize", DESC_OCTET                &SambaTraits,      "Largeur des traits des graphiques en qualite papier" },
	{ "aide",     DESC_STR(MAXFILENAME)      SubDirAide,       "Sous-repertoire de l'aide (relatif au repertoire 'info')" },
	{ "trad",     DESC_STR(MAXFILENAME)      SubDirTrad,       "Sous-repertoire des dictionnaires de langue (relatif au repertoire 'info')" },

	{ "NSDocumentRevisionsDebugMode", DESC_STR(8) XcodeSambaDebugTxt,  "Mode d'execution par Xcode" },
#ifdef PCI_DIRECTIF
	{ "P", DESC_HEXA &(PCIadrs[0]), "Adresse de la carte PCI" },
#endif /* PCI_DIRECTIF */
	DESC_END
};

/*
 * Description du montage experimental
 */

static ArgDesc AcquisDesc[] = {
	{ 0, /* "nom" */  DESC_STR(HOTE_NOM)       AcquisLue.code, 0 },
	{ "ip",           DESC_STR(HOTE_NOM)       AcquisLue.adrs, 0 },
	{ "runs",         DESC_STR(2)              AcquisLue.runs, 0 },
	{ "repartiteurs", DESC_LISTE_SIZE(AcquisLue.nbrep,ACQUIS_MAXREP) RepartListe, AcquisLue.rep, 0 },
	DESC_END
};
static ArgStruct AcquisAS = { (void *)Acquis, (void *)&AcquisLue, sizeof(TypeAcquis), AcquisDesc };
static ArgDesc PartitDesc[] = {
	{ "maitre",     DESC_STR(HOTE_NOM)                                   PartitLue.mtr, 0 },
	{ "satellites", DESC_LISTE_SIZE(PartitLue.nbsat,MAXSAT) AcquisListe, PartitLue.sat, 0 },
	DESC_END
};
static ArgStruct PartitAS = { (void *)Partit, (void *)&PartitLue, sizeof(TypePartit), PartitDesc };

static ArgDesc ReseauDesc[] = {
	{ "acquisition", DESC_STRUCT_SIZE(AcquisNb,MAXSAT)    &AcquisAS, 0 },
	{ "partitions",  DESC_STRUCT_SIZE(PartitNb,MAXMAITRE) &PartitAS, 0 },
	DESC_END
};

//static ArgDesc ChassisDescNeant[] = { DESC_END };
//static ArgDesc ChassisDescNoms[] = { DESC_END };
//static ArgDesc *ChassisDescCode[] = { ChassisDescNeant, ChassisDescNeant, ChassisDescNeant, ChassisDescNeant, ChassisDescNoms };
static ArgDesc ChassisDimDesc[] = {
	{ "nb",   DESC_SHORT                     &ChassisLu.nb,     0 },
	{ "code", DESC_KEY CHASSIS_CODE_CLES,    &ChassisLu.codage, 0 },
//	{ "noms", DESC_VARIANTE(ChassisLu.codage) ChassisDescCode,  0 },
  	{ "noms", DESC_STR_PTR                   &ChassisLu.cles,   0 },
	DESC_END
};
static ArgStruct ChassisDetecAS = { (void *)ChassisDetec, (void *)&ChassisLu, sizeof(TypeChassisAxe), ChassisDimDesc };
static ArgStruct ChassisNumerAS = { (void *)ChassisNumer, (void *)&ChassisLu, sizeof(TypeChassisAxe), ChassisDimDesc };
static ArgDesc ChassisDesc[] = {
	{ "Support",     DESC_NONE }, // DESC_STRUCT_SIZE(ChassisDetecDim,CHASSIS_DIMMAX) &ChassisDetecAS, "Definition du support des detecteurs" },
	{ "Detecteurs",  DESC_STRUCT_SIZE(ChassisDetecDim,CHASSIS_DIMMAX) &ChassisDetecAS, "Definition du chassis des detecteurs" },
	{ "Digitiseurs", DESC_STRUCT_SIZE(ChassisNumerDim,CHASSIS_DIMMAX) &ChassisNumerAS, "Definition du chassis des numeriseurs" },
	DESC_END
};

static ArgDesc SimuModeDesc[] = {
	{ 0,               DESC_STR(MODL_NOM) SimuLue.nom, 0 },
	{ "voies",         DESC_LISTE_SIZE(SimuLue.nbvoies,VOIES_MAX) VoieNom, SimuLue.voie, 0 },
	{ "niveau_DC",     DESC_FLOAT &SimuLue.ldb,   0 },
	{ "bruit",         DESC_FLOAT &SimuLue.bruit, 0 },
	{ "frequence_evt", DESC_FLOAT &SimuLue.freq,  0 },
	{ "energie_ADU",   DESC_FLOAT &SimuLue.pic,   0 },
	{ "resolution",    DESC_FLOAT &SimuLue.reso,  0 },
	DESC_END
};
static ArgStruct SimuAS = { (void *)Simu, (void *)&SimuLue, sizeof(TypeSimu), SimuModeDesc };
static ArgDesc SimuDesc[] = {
	{ "Simulations", DESC_STRUCT_SIZE(SimuNb,SIMU_MAX) &SimuAS, 0 },
	DESC_END
};

static char NomModlChassis[MAXFILENAME],NomModlDetecteurs[MAXFILENAME],NomModlNumeriseurs[MAXFILENAME];
static char NomModlRepartiteurs[MAXFILENAME],NomModlCablage[MAXFILENAME],NomModlEnvir[MAXFILENAME];
static ArgDesc ModelesDirDesc[] = {
	{ "Detecteurs",    DESC_STR(MAXFILENAME) NomModlDetecteurs,   0 },
	{ "Numeriseurs",   DESC_STR(MAXFILENAME) NomModlNumeriseurs,  0 },
	{ "Cablages",      DESC_STR(MAXFILENAME) NomModlCablage,      0 },
	{ "Repartiteurs",  DESC_STR(MAXFILENAME) NomModlRepartiteurs, 0 },
	{ "Environnement", DESC_STR(MAXFILENAME) NomModlEnvir,        0 },
	{ "Support",       DESC_DEVIENT("Chassis")                      },
	{ "Chassis",       DESC_STR(MAXFILENAME) NomModlChassis,      0 },
	DESC_END
};

static char NomModeles[MAXFILENAME],NomMedia[MAXFILENAME],NomSimu[MAXFILENAME],NomDico[MAXFILENAME];
static char NomDetecteurs[MAXFILENAME],NomRepartiteurs[MAXFILENAME],NomCablage[MAXFILENAME],NomNumeriseurs[MAXFILENAME];
static char NomProcedures[MAXFILENAME],NomRegulEvt[MAXFILENAME],NomVi[MAXFILENAME],NomEvtUnite[MAXFILENAME];
static char NomCr[MAXFILENAME],NomRuns[MAXFILENAME],NomEtatElec[MAXFILENAME],NomEnvironnement[MAXFILENAME],NomCaract[MAXFILENAME];
static char CalendrierRef[MAXFILENAME];
static int  CalendrierNum;

static char NomLecture[MAXFILENAME],NomMonit[MAXFILENAME],NomAutom[MAXFILENAME];
static char NomReseau[MAXFILENAME],NomSatellites[MAXFILENAME];
static char NomCalcul[MAXFILENAME],NomFinesses[MAXFILENAME],NomBasic[MAXFILENAME],NomVerif[MAXFILENAME],NomExports[MAXFILENAME],NomBanc[MAXFILENAME];
ArgDesc Setup[] = {
	{ "Intitule",               DESC_TEXT   SambaIntitule,     "Designation de la configuration" },
	{ DESC_COMMENT 0 },
	{ DESC_COMMENT "===== Electronique =====" },
#ifdef PCI_DIRECTIF
	{ "PCI.bus",                DESC_TEXT   PCIbus,            "Chemin d'acces (ressource Mac) au bus PCI" },
	{ "PCI.adresse",            DESC_HEXA  &(PCIadrs[0]),      "Adresse de la carte PCI" },
#endif
	{ "PCI.connecte",           DESC_NONE }, // DESC_BOOL  &PCIdemande,         "Vrai si carte PCI connectee" },
	{ "IP.port",                DESC_INT   &PortLectRep,       "Valeur plancher du port de lecture via IP" },
	{ "Sat.port",               DESC_INT   &PortLectSat,       "Valeur plancher du port de lecture des satellites" },
	{ "FIFO.dim",               DESC_INT   &FIFOdim,           "Profondeur des FIFO IP ou NI (16 bits)" },
	{ "Prgm.repartiteur",       DESC_NONE }, // DESC_HEXA  &PrgmRepartiteur,   "Page de programmation des IBB" },
	{ "Lect.DMA",               DESC_NONE }, // DESC_BOOL  &DmaDemande,        "Lecture en DMA si c'est possible" },
	{ "Oscillateur",            DESC_NONE }, // DESC_FLOAT &HorlogeSysteme,    "Horloge fondamentale du systeme (MHz)" },
	{ "Diviseur.0",             DESC_NONE }, // DESC_INT   &Diviseur0,         "Valeur du diviseur 0" },
	{ "Diviseur.1",             DESC_NONE }, // DESC_INT   &Diviseur1,         "Valeur du diviseur 1 (nombre de bits a produire par echantillon)" },
	{ "Version.repartiteur",    DESC_NONE }, // DESC_FLOAT &VersionRepartiteur, "Version du repartiteur" },
	{ "ADC.bits",               DESC_NONE }, // DESC_INT   &ADCbits,      "Largeur des ADC" },
	{ "Bolo.transmis",          DESC_NONE }, // "Nombre de detecteurs maximum transmis via PCI"
	{ "Bolo.voies",             DESC_NONE }, // DESC_INT   &ModeleVoieNb, "Nombre de voies par detecteur" },
	{ "Voies",                  DESC_NONE }, // DESC_TEXT   VoiesConnues, "Noms de voies (termine par + si modulee)" },
	{ "Voies.liste",            DESC_NONE }, // DESC_TEXT   VoiesConnues, "Noms de voies (termine par @ si modulee)" },
	{ "Voie.status",            DESC_TEXT   VoieStatus,        "Nom de la voie avec status" },
	{ DESC_COMMENT 0 },
	{ DESC_COMMENT "===== Fichiers de Gestion =====" },
	{ "Bolo.max",               DESC_INT   &BoloGeres,         "Nombre maximum de detecteurs geres" },
	{ "Voie.max",               DESC_INT   &VoiesGerees,       "Nombre maximum de voies gerees" },
	{ "Run.types",              DESC_STR(80) RunCategCles,     "Noms des types de run" },
	{ "Run.conditions",         DESC_NONE }, //   DESC_TEXT RunEnvir,          "Noms des conditions de run" },
	{ "Run.start.script",       DESC_TEXT   NomScriptStart,    "Script de demarrage des runs (relatif au repertoire des procedures)" },
	{ "Run.stop.script",        DESC_TEXT   NomScriptStop,     "Script d'arret des runs (relatif au repertoire des procedures)" },
	{ "Run.regen.entree",       DESC_TEXT   NomRegenStart,     "Script de mise en regeneration (relatif au repertoire des procedures)" },
	{ "Run.regen.sortie",       DESC_TEXT   NomRegenStop,      "Script de fin de regeneration (meme remarque!)" },
	{ "Entretien.start.script", DESC_TEXT   NomEntretienStart, "Script de debut de l'entretien periodique (relatif au repertoire des procedures)" },
	{ "Entretien.stop.script",  DESC_TEXT   NomEntretienStop,  "Script de fin de l'entretien periodique (relatif au repertoire des procedures)" },
	{ "Calendrier.place",       DESC_TEXT   CalendrierRef,     "Description de l'emplacement des calendriers" },
	{ "Calendrier.nom",         DESC_TEXT   CalendrierNom,     "Nom du calendrier des evenements destines a SAMBA" },
	{ "Calendrier.run",         DESC_TEXT   CalendrierRun,     "Nom des evenements du calendrier definissant les periodes de run" },
	{ "Imprime.configs",        DESC_BOOL  &ImprimeConfigs,    "Imprime les configs" },
	{ "Database.serveur",       DESC_TEXT   EdbServeur,        "Adresse de la Base de Donnees" }, // "http://134.158.176.27:5984/testdb"
	{ "Cahier.manip",           DESC_KEY "neant/texte/elog", &LoggerType, "Type de cahier de manip (neant/texte/elog)" },
	{ "Cahier.adrs",            DESC_TEXT   LoggerAdrs,        "Nom du cahier de manip (fichier ou serveur, selon type)" },
	{ "Cahier.cible",           DESC_TEXT   LoggerUser,        "Coordonnees d'acces au cahier de manip (si serveur)" },
	{ DESC_COMMENT "             (typiquement, -u acquis darkmatr -l STPC -a Author=SAMBA -a Category=Acquisition)" },
	{ "Gardiens",               DESC_TEXT   Gardiens,          "Personnes a avertir en cas d'arret acquisition (cf 'mail')" },
	{ "Acces",                  DESC_NONE },  /* DESC_BOOL &ExpertConnecte */
	{ DESC_COMMENT 0 },
	{ DESC_COMMENT "===== Fichiers de Description du Montage =====" },
	{ "Modeles",                DESC_NONE }, // DESC_STRUCT &ModelesDirDesc,    "Ensemble des modeles de materiel" },
	{ "Fichier.modeles",        DESC_TEXT   NomModeles,        "Liste des modeles (materiel, chassis, environnement)" },
	{ "Fichier.media",          DESC_TEXT   NomMedia,          "Liste des media auxiliaires" },
	{ "Fichier.simu",           DESC_TEXT   NomSimu,           "Liste des modes de simulation" },
	{ "Fichier.reseau",         DESC_TEXT   NomReseau,         "Liste des adresses IP des satellites" },
	{ "Fichier.satellites",     DESC_TEXT   NomSatellites,     "Liste des satellites a controler" },
	{ "Fichier.compte-rendu",   DESC_TEXT   NomCr,             "Options de compte-rendu de demarrage" },
	{ "Fichier.parametres",     DESC_TEXT   NomParms,          "Parametrisation generale" },
	{ "Fichier.dictionnaire",   DESC_TEXT   NomDico,           "Dictionnaires de synonymes" },
//	{ "Fichier.detecteurs",     DESC_DEVIENT("Run.configurations") },
	{ "Fichier.detecteurs",     DESC_TEXT   NomDetecteurs,     "Definition des detecteurs" },
	{ "Fichier.repartiteurs",   DESC_TEXT   NomRepartiteurs,   "Liste des repartiteurs" },
	{ "Fichier.cablage",        DESC_TEXT   NomCablage,        "Definition du cablage" },
	{ "Fichier.numeriseurs",    DESC_TEXT   NomNumeriseurs,    "Liste des numeriseurs" },
	{ "Fichier.procedures",     DESC_TEXT   NomProcedures,     "Repertoire des procedures" },
	{ "Fichier.filtres",        DESC_TEXT   NomFiltres,        "Definition des filtres" },
	{ "Fichier.sequences",      DESC_TEXT   NomSequences,      "Definition des sequences" },
	{ "Fichier.regul.evt",      DESC_TEXT   NomRegulEvt,       "Definition de la regulation du taux d'evenements" },
	{ "Fichier.VI",             DESC_TEXT   NomVi,             "Jeu de V(I)" },
	{ "Fichier.evt-unite",      DESC_TEXT   NomEvtUnite,       "Evenements unite" },
	{ "Fichier.lecture",        DESC_TEXT   NomLecture,        "Parametrisation de la lecture" },
	{ "Fichier.affichage",      DESC_TEXT   NomMonit,          "Parametrisation des affichages" },
	{ "Fichier.automates",      DESC_DEVIENT("Fichier.support") },
	{ "Fichier.support",        DESC_TEXT   NomAutom,          "Parametrisation du support" },
	{ "Fichier.calculs",        DESC_TEXT   NomCalcul,         "Parametrisation des calculs" },
	{ "Fichier.finesses",       DESC_TEXT   NomFinesses,       "Parametrisation secondaire" },
	{ "Fichier.basique",        DESC_TEXT   NomBasic,          "Parametrisation des acces basiques" },
	{ "Fichier.verifs",         DESC_TEXT   NomVerif,          "Options de verification" },
	{ "Fichier.exports",        DESC_TEXT   NomExports,        "Fichier d'exportation de variables" },
	{ "Fichier.banc",           DESC_TEXT   NomBanc,           "Parametrisation du banc de tests des numeriseurs" },
	{ "Fichier.comptage",       DESC_TEXT   NomRuns,           "Gestion automatique des noms des runs" },
	{ "Fichier.etat-elec",      DESC_TEXT   NomEtatElec,       "Etat de l'electronique" },
	{ "Fichier.conditions",     DESC_TEXT   NomEnvironnement,  "Conditions de run courantes" },
	{ "Fichier.caract",         DESC_TEXT   NomCaract,         "Caracteristiques des run" },

	DESC_END
};

static ArgDesc ModelePhysDescEdit[] = {
	{ 0,                DESC_STR(MODL_NOM) ModelePhysLu.nom,    0 },
	{ "duree(ms)",      DESC_FLOAT &ModelePhysLu.duree,         0 }, // "duree evenement (ms)" },
	{ "temps-mort(ms)", DESC_FLOAT &ModelePhysLu.tempsmort,     0 }, // "temps mort apres maxi evenement (ms)" },
	{ "fenetre(ms)",    DESC_FLOAT &ModelePhysLu.interv,        0 }, // "fenetre de recherche (ms)" },
	{ "delai(ms)",      DESC_FLOAT &ModelePhysLu.delai,         0 }, // "delai evenement (ms)" },
#ifdef RL
	{ "template(pts)",	DESC_INT   &ModelePhysLu.dimtemplate,	0 }, // "dim. template (points)" },
	{ "montee(ms)",	    DESC_FLOAT &ModelePhysLu.montee,		0 }, // "montee (ms)" },
	{ "descente1(ms)",	DESC_FLOAT &ModelePhysLu.descente1,		0 }, // "descente1 (ms)" },
	{ "descente2(ms)",	DESC_FLOAT &ModelePhysLu.descente2,		0 }, // "descente2 (ms)" },
	{ "facteur2",		DESC_FLOAT &ModelePhysLu.facteur2,		0 }, // "facteur2 " },
#endif
	DESC_END
};
static ArgStruct ModelePhysEditAS = { (void *)ModelePhys, (void *)&ModelePhysLu, sizeof(TypeSignalPhysique), ModelePhysDescEdit };
static ArgDesc ModeleVoieDescEdit[] = {
	{ 0,                      DESC_STR(MODL_NOM) ModeleVoieLu.nom, 0 },
	{ "physique",             DESC_LISTE ModelePhysListe, &ModeleVoieLu.physique, 0 }, // "signal physique mesure par ce type de voie" },
//	{ "modulee",              DESC_BOOL  &ModeleVoieLu.modulee,    0 }, // "vrai si l'electronique genere une modulation sur cette voie" },
	{ "temps-mort(ms)",       DESC_FLOAT &ModeleVoieLu.tempsmort,  0 }, // "temps mort apres maxi evt (ms)" },
	{ "coincidence(ms)",      DESC_FLOAT &ModeleVoieLu.coinc,      0 }, // "duree possible de coincidence (ms)" },
	{ "pretrigger(%evt)",     DESC_FLOAT &ModeleVoieLu.pretrigger, 0 }, // "proportion de l'evenement avant le trigger (%)" },
	{ "base.debut(%pretrig)", DESC_FLOAT &ModeleVoieLu.base_dep,   0 }, // "position du debut de ligne de base dans le pre-trigger (%)" },
	{ "base.fin(%pretrig)",   DESC_FLOAT &ModeleVoieLu.base_arr,   0 }, // "position de fin de ligne de base dans le pre-trigger (%)" },
	DESC_END
};
static ArgStruct ModeleVoieEditAS = { (void *)ModeleVoie, (void *)&ModeleVoieLu, sizeof(TypeVoieModele), ModeleVoieDescEdit };

static ArgDesc ModeleDetDescEdit[] = {
	{ 0,            DESC_STR(MODL_NOM)                                                    ModeleDetLu.nom,   0 },
	{ "capteurs",   DESC_LISTE_SIZE(ModeleDetLu.nbcapt,DETEC_CAPT_MAX) ModeleVoieListe,   ModeleDetLu.type,  0 },
	{ "reglages",   DESC_STRUCT_SIZE(ModeleDetLu.nbregl,DETEC_REGL_MAX)                  &ModeleDetReglAS,   0 },
	{ "associes",   DESC_LISTE_SIZE(ModeleDetLu.nbassoc,DETEC_ASSOC_MAX) ModeleDetListe,  ModeleDetLu.assoc, 0 },
	DESC_END
};
static ArgStruct ModeleDetEditAS = { (void *)ModeleDet, (void *)&ModeleDetLu, sizeof(TypeDetModele), ModeleDetDescEdit };

static ArgDesc ModeleRepDescEdit[] = {
	{ "code",           DESC_STR(REPART_CODE_NOM)       ModeleRepLu.code,           0 },
  	{ "famille",        DESC_LISTE RepartFamilleListe, &ModeleRepLu.famille,        0 },
	{ "type_selecteur", DESC_LISTE RepartNomSelecteur, &ModeleRepLu.selecteur.type, "type de selection des donnees (aucun/numeriseurs/canaux)" },
	{ "dim_selecteur",  DESC_SHORT                     &ModeleRepLu.selecteur.max,  "nb registres de selection" },
	{ "revision",       DESC_BYTE                      &ModeleRepLu.revision,       "niveau de realisation dans cette famille" },
	{ "horloge",        DESC_FLOAT                     &ModeleRepLu.horloge,        "horloge fondamentale du systeme (MHz)" },
	{ "interface",      DESC_LISTE InterfaceListe,     &ModeleRepLu.interface,      0 },
	{ "max_numer",      DESC_SHORT                     &ModeleRepLu.max_numer,      "maxi numeriseurs transmissibles" },
	DESC_END
};

static ArgDesc ConnectReglDescMaxi[] = {
	{ "reglage",   DESC_STR(DETEC_REGL_NOM) ConnectReglLue.reglage,   0 },
	{ "ressource", DESC_STR(NUMER_NOM)      ConnectReglLue.ressource, 0 },
	DESC_END
};
static ArgStruct ConnectCaptASMaxi = { (void *)ModeleCableLu.capteur, (void *)&ConnectCaptLue, sizeof(TypeCablageConnectCapt), ConnectCaptDesc };
static ArgStruct ConnectReglASMaxi = { (void *)ModeleCableLu.connection, (void *)&ConnectReglLue, sizeof(TypeCablageConnectRegl), ConnectReglDescMaxi };
static ArgDesc ModeleCableDescMaxi[] = {
	{ "type",           DESC_STR(CABLAGE_MODL_NOM)                                               ModeleCableLu.nom, 0 },
	{ "detecteur",      DESC_LISTE ModeleDetListe,                                              &ModeleCableLu.modl_bolo, 0 },
	{ "numeriseurs",    DESC_LISTE_SIZE(ModeleCableLu.nb_numer,DETEC_CAPT_MAX) ModeleNumerListe, ModeleCableLu.modl_numer, 0 },
	{ "capteurs",       DESC_STRUCT_SIZE(ModeleCableLu.nb_capt,DETEC_CAPT_MAX)                  &ConnectCaptASMaxi, 0 },
	{ "reglages",       DESC_STRUCT_SIZE(ModeleCableLu.nbconnections,DETEC_REGL_MAX)            &ConnectReglASMaxi, 0 },
	DESC_END
};
static ArgStruct ModeleCablageASMaxi = { (void *)ModeleCable, (void *)&ModeleCableLu, sizeof(TypeModeleCable), ModeleCableDescMaxi };

/* elegant informatiquement, mais un peu lourd ergonomiquement:
 ArgDesc ModelesDesc[] = {
	{ "Physique",     DESC_STRUCT_SIZE(ModelePhysNb,REPART_TYPES)      &ModelePhysAS,    0 },
	{ "Voies",        DESC_STRUCT_SIZE(ModeleVoieNb,VOIES_TYPES)       &ModeleVoieAS,    0 },
	{ "Detecteurs",   DESC_STRUCT_SIZE(ModeleDetNb,DETEC_TYPES)        &ModeleDetAS,     0 },
	{ "ADC",          DESC_STRUCT_SIZE(ModeleADCNb,NUMER_ADC_TYPES)    &ModeleADCAS,     0 },
	{ "Numeriseurs",  DESC_STRUCT_SIZE(ModeleBNNb,NUMER_TYPES)         &ModeleBNAS,      0 },
	{ "Cablages",     DESC_STRUCT_SIZE(ModeleCableNb,CABLAGE_TYPE_MAX) &ModeleCablageAS, 0 },
	{ "Repartiteurs", DESC_STRUCT_SIZE(ModeleRepNb,REPART_TYPES)       &ModeleRepAS,     0 },
	DESC_END
}; */

TypeCablageCaptr CapteurLu;
ArgDesc CapteurDesc[] = {
	{ "gain",       DESC_FLOAT &CapteurLu.gain, 0 },
	{ "capa(pF)",   DESC_FLOAT &CapteurLu.capa, 0 },
#if (NUMER_MAX < 256)
	{ "connecteur", DESC_BYTE  &CapteurLu.connecteur, 0 },
#else
	{ "connecteur", DESC_SHORT &CapteurLu.connecteur, 0 },
#endif
	{ "adc",        DESC_BYTE  &CapteurLu.vbb, 0 },
	DESC_END
};
TypeCablage CablageLu;
ArgStruct CapteurAS = { (void *)CablageLu.captr, (void *)&CapteurLu, sizeof(TypeCablageCaptr), CapteurDesc };
ArgDesc CablageDesc[] = {
	{ 0,          DESC_STR(CABLAGE_NOM)     CablageLu.nom, 0 },
	{ "type",     DESC_LISTE ModeleCableListe, &CablageLu.type, 0 },
	{ "capteurs", DESC_STRUCT_SIZE(CablageLu.nbcapt,DETEC_CAPT_MAX) &CapteurAS, 0 },
	DESC_END
};
ArgStruct CablageAS = { (void *)CablageSortant, (void *)&CablageLu, sizeof(TypeCablage), CablageDesc };

TypeNumeriseur NumeriseurLu;
ArgDesc NumeriseurDescEdit[] = {
 	{ "type",        DESC_LISTE ModeleNumerListe, &NumeriseurLu.def.type,   0 },
	{ "numero",      DESC_BYTE   &NumeriseurLu.def.serial, 0 },
	{ "ident",       DESC_SHEX   &NumeriseurLu.ident,      0 },
	{ "slot-cryo",   DESC_BYTE   &NumeriseurLu.fischer,      0 },
//	{ "Rmodul",      DESC_FLOAT  &NumeriseurLu.Rmodul,     0 },
//	{ "Cmodul",      DESC_FLOAT  &NumeriseurLu.Cmodul,     0 },
	{ "gains",       DESC_FLOAT_SIZE(NumeriseurLu.nbadc,NUMER_ADC_MAX)  NumeriseurLu.def.gain,     0 },
	DESC_END
};
ArgStruct NumeriseurAS = { (void *)Numeriseur, (void *)&NumeriseurLu, sizeof(TypeNumeriseur), NumeriseurDescEdit };

/* ... Variantes selon la famille ... */
extern ArgDesc *RepartOptnFamDesc[]; 
/* ... Variantes selon l'interface ... */
ArgDesc RepartRegDescEdit[] = {
	{ "numeriseur", DESC_SHEX &(RepartRegLu.adrs), 0 },
	{ "registre",   DESC_SHEX &(RepartRegLu.ssadrs), 0 },
	{ "valeur",     DESC_SHEX &(RepartRegLu.valeur), 0 },
	DESC_END
};
ArgStruct RepartRegsInitASEdit = { (void *)RepartLu.i.pci.reg, (void *)&RepartRegLu, sizeof(TypePciReg), RepartRegDescEdit };
ArgStruct RepartRegsRazASEdit  = { (void *)RepartLu.d.pci.reg, (void *)&RepartRegLu, sizeof(TypePciReg), RepartRegDescEdit };
ArgDesc RepartPciDescEdit[] = {
	{ "init",    DESC_STRUCT_SIZE(RepartLu.i.pci.nbregs,REPART_PCIREG_MAX) &RepartRegsInitASEdit, 0 },
	{ "start",   DESC_STRUCT_SIZE(RepartLu.d.pci.nbregs,REPART_PCIREG_MAX) &RepartRegsRazASEdit, 0 },
	DESC_END
};
ArgDesc RepartActn1fichierDescEdit[] = {
	{ "source", DESC_STR(MAXFILENAME)         RepartActnLue.fichier, 0 },
	DESC_END
};
ArgDesc RepartActn2fichiersDescEdit[] = {
	{ "source", DESC_STR(MAXFILENAME)         RepartActnLue.fichier, 0 },
	{ "dest", DESC_STR(MAXFILENAME)    RepartActnLue.dest, 0 },
	DESC_END
};
ArgDesc *RepartActnParmDescEdit[] = { RepartActn2fichiersDescEdit, RepartActn1fichierDescEdit, RepartActn1fichierDescEdit, 0 };
ArgDesc RepartActnDescEdit[] = {
	{ "protocole", DESC_KEY "ftp/telnet/shell", &RepartActnLue.type, 0 },
	{ "fichier",   DESC_VARIANTE(RepartActnLue.type) RepartActnParmDescEdit, 0 },
	DESC_END
};
ArgStruct RepartActnInitASEdit = { (void *)RepartLu.i.ip.actn, (void *)&RepartActnLue, sizeof(TypeIpActn), RepartActnDescEdit };
ArgStruct RepartActnRazASEdit  = { (void *)RepartLu.d.ip.actn, (void *)&RepartActnLue, sizeof(TypeIpActn), RepartActnDescEdit };
ArgDesc RepartIpDescEdit[] = {
	{ "port",    DESC_INT            &RepartLu.ecr.port, 0 },
	{ "user",    DESC_STR(REPART_LOGIN_NOM) RepartLu.p.ip.user, 0 },
	{ "pswd",    DESC_STR(REPART_LOGIN_NOM) RepartLu.p.ip.pswd, 0 },
	{ "init",    DESC_STRUCT_SIZE(RepartLu.i.ip.nbactn,REPART_IPACTN_MAX) &RepartActnInitASEdit, 0 },
	{ "start",   DESC_STRUCT_SIZE(RepartLu.d.ip.nbactn,REPART_IPACTN_MAX) &RepartActnRazASEdit, 0 },
	DESC_END
};
ArgDesc RepartUsbDescEdit[] = { DESC_END };
ArgDesc RepartFicDescEdit[] = { DESC_END };
ArgDesc *RepartOptnIfDescEdit[]  = { RepartPciDescEdit, RepartIpDescEdit, RepartUsbDescEdit, RepartFicDescEdit, 0 }; 
extern ArgDesc *RepartOptnSrceDesc[];
ArgDesc RepartDescEdit[] = {
	{ "type",         DESC_LISTE ModeleRepListe,      &RepartLu.type,        0 },
	{ "adresse",      DESC_STR(MAXFILENAME)            RepartLu.parm,        0 },
	{ "etat",         DESC_KEY REP_STATUTS_CLES,      &RepartLu.valide,      0 },
	{ "maintenance",  DESC_STR(MAXFILENAME)            RepartLu.maintenance, 0 },
	{ "numerisation", DESC_KEY REPART_DONNEES_CLES,   &RepartLu.categ,       0 },
	{ "liste",        DESC_VARIANTE(RepartLu.categ)    RepartOptnSrceDesc,   0 },
	{ "config",       DESC_VARIANTE(RepartLu.famille)  RepartOptnFamDesc,    0 },
	{ "demarrage",    DESC_VARIANTE(RepartLu.interf)   RepartOptnIfDescEdit, 0 },
	DESC_END
};
ArgStruct RepartiteurAS = { (void *)Repart, (void *)&RepartLu, sizeof(TypeRepartiteur), RepartDescEdit };

//printf("sizeof(CapteurLu) = %d/%d\n",sizeof(CapteurLu),sizeof(TypeCablageCaptr));
//printf("sizeof(NumeriseurLu) = %d/%d\n",sizeof(NumeriseurLu),sizeof(TypeNumeriseur));

ArgDesc SambaTermeDesc[] = {
	{ "accepte",  DESC_STR(MAXSAMBATERME) SambaTermeTempo.accepte, 0 },
	{ "officiel", DESC_STR(MAXSAMBATERME) SambaTermeTempo.officiel, 0 },
	DESC_END
};
ArgStruct SambaTermeAS = { (void *)SambaDicoTempo.terme, (void *)&SambaTermeTempo, sizeof(TypeSambaTerme), SambaTermeDesc };
ArgDesc SambaDicoDesc[] = {
	{ "nom",     DESC_STR(MAXDICONOM) SambaDicoTempo.nom, 0 },
	{ "lexique", DESC_STRUCT_SIZE(SambaDicoTempo.nbtermes,MAXSAMBALEXIQUE) &SambaTermeAS, 0 },
	DESC_END
};
ArgStruct SambaDicoAS = { (void *)SambaDicoDefs, (void *)&SambaDicoTempo, sizeof(TypeSambaDico), SambaDicoDesc };

OpiumTableDeclaration SambaTableModeles,SambaTableMatos;

;

// #define SAUVE_AVEC_MENU
#ifdef SAUVE_AVEC_MENU
static Menu mSambaQuitte;
static Info iSambaSauve;
#endif /* SAUVE_AVEC_MENU */

#define MAX_FOLDERS 50
static struct {
	char *nom;
	char titre[80];
} FolderRef[MAX_FOLDERS];
static int FolderNb,FolderNum;
static char FolderActn;
static char *FolderListe[MAX_FOLDERS+1];
static Panel pFolderActn; // pFolder deja pris (voir OpenScripting/FinderRegistry.h)

static char ConfigGeneral[MAXFILENAME];
static char VolumeManquant[MAXFILENAME];
static Panel pArgs,pSetup;
extern int OpiumNbCdr;
static char NomTache[12];
#ifdef MODULES_RESEAU
static int64 SambaProchainAffichage;
#endif
#define MAXLANGUES 8
static char *SambaLangueDispo[MAXLANGUES]; static int SambaLanguesNb,SambaLangueChoisie;
static char SambaNumerSelect;

static TypeSocket SktEntreeMaitre,SktSortieMaitre;

static Term tClasse[CLASSE_NBOBJETS];
static Panel pSambaImprimeForme,pSambaImprimeSvr;
static int  NbTirages;
static char NumPrinter;
static char ListePrinters[32];

void CalcMenuBarre(),GestMenuBarre();
void LectMenuBarre(),MonitMenuBarre();

static int SambaConfigDuplic(Menu menu, MenuItem *item);
static int SambaConfigRenomme(Menu menu, MenuItem *item);
static int SambaConfigRefresh(Panel panel);
static int SambaOuvreOreillesSat();
static int SambaEcouteMaitre();
static int SambaOuvreOreillesMaitre(int sat);

static void SambaLinkPCI();
static char SambaConnecteSat(int sat);

static Panel pTri;

void SettingsInit(); int SettingsSetup(); int SettingsExit();
// void CmdesInit(); int CmdesSetup(); int CmdesExit();
void DetecteursInit(); int DetecteursSetup(); int DetecteursExit();
void TestsInit(); int TestsSetup(); int TestsExit();
void LectInit();  int LectSetup();  int LectExit();
void MonitInit(); int MonitSetup(); int MonitExit();
int  AutomInit(); int AutomSetup(); int AutomExit();
void CalcInit();  int CalcSetup();  int CalcExit();
void DiagInit();  int DiagSetup();  int DiagExit();
void BasicInit(); int BasicSetup(); int BasicExit();

int BasicOperaTousMaj();
void SettingsVerifParms();
/* Jusqu'a SambaAlloc, les fonctions suivantes sont a mettre dans utils ou opium */

// #define TEST_ACCES_DB

#ifdef TEST_ACCES_DB
#include <curl/curl.h>

/* ========================================================================== */
static void SambaTest(int nb, ...) {
	va_list arg; int b; int i;
	
	printf("? Ecrit: '");
	va_start(arg,nb);
	for(i=0; i<nb; i++) {
		b = va_arg(arg,int);
		printf("%c",b);
	}
	va_end(arg);
	printf("'\n");
//	SmbTest(6,'C','o','u','c','o','u');
//	SmbTest(2,'O','K');
}
/* ========================================================================== */
int EdbLit() {
	CURL *curl; CURLcode res;
	
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "http://edwdbik.fzk.de:5984");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, 0);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);
		res = curl_easy_perform(curl);
		// cout << "the curl return : " << res << endl;
		printf("(%s) curl_easy_perform renvoie %d\n",__func__,res);
		/* always cleanup */ 
		curl_easy_cleanup(curl);
	}
	return 0;
}
#endif /* TEST_ACCES_DB */

#ifdef ATTENTE_AVEC_TIMER
Timer SambaAttente;
char SambaAttenteFinie;
/* ========================================================================== */
static void SambaAttenteFin() { SambaAttenteFinie = 1; }
#endif /* ATTENTE_AVEC_TIMER */
/* ========================================================================== */
long SambaSecondes() {
#ifdef CODE_WARRIOR_VSN
	return(1);
#else
	struct timeval tp; struct timezone tzp;
	gettimeofday(&tp,&tzp);
	return(tp.tv_sec);
#endif
}
/* ========================================================================== */
void SambaAttends(int ms) {
	
#ifdef ATTENTE_AVEC_USLEEP
	//	printf("Attente UNIX de %d ms\n",ms);
	usleep(ms*1000);
#else /* ATTENTE_AVEC_USLEEP */
	#ifdef ATTENTE_AVEC_TIMER
		SambaAttenteFinie = 0;
	//	printf("Attente sur timers de %d ms\n",ms);
		TimerStart(SambaAttente,ms * TIMER_MILLISEC);
		until(SambaAttenteFinie) { TimerMilliSleep(1); }
		SambaAttenteFinie = 0;
	#else /* ATTENTE_AVEC_TIMER */
		int temps,delai,decompte;
		temps = TickCount(); /* 1 tick = en principe 1/60 seconde, soit FrequenceTick=60 */
		delai = ms * FrequenceTick; if(delai < 1000) delai = 1; else delai /= 1000;
	//	printf("Attente APPLE de %d ticks\n",delai);
		delai += temps; decompte = 1000000000;
		while((TickCount() < delai) && decompte) decompte--;
	#endif /* ATTENTE_AVEC_TIMER */
#endif /* ATTENTE_AVEC_USLEEP */
}
/* ========================================================================== */
int HttpTraiteRequete() { return(1); }
/* ========================================================================== */
void SambaMicrosec(int microsec) {
#ifdef ATTENTE_AVEC_USLEEP
//	printf("Attente UNIX de %d us\n",microsec);
	usleep(microsec);
#else /* ATTENTE_AVEC_USLEEP */
	/* UnsignedWide est une structure, or on veut tester <temps> sur 64 bits .. */
	UInt64 limite; int decompte;
	union {
		UInt64 bits64;
		UnsignedWide mac;
	} temps;

//	printf("Attente APPLE de %d us\n",microsec);
//-	Microseconds((UnsignedWide *)(&temps));
	Microseconds(&temps.mac);
	limite = temps.bits64 + (UInt64)microsec; decompte = 1000000000;
	while((temps.bits64 < limite) && --decompte) {
//-		Microseconds((UnsignedWide *)(&temps));
		Microseconds(&temps.mac);
	}
#endif /* ATTENTE_AVEC_USLEEP */
}
/* ========================================================================== */
void attends(char *texte) {
	char c;

	if(texte) { printf(texte); printf(". "); }
	printf("<Return> pour continuer: "); fflush(stdout);
	do c = getchar(); while((c != 0x0a) && (c != EOF));
}
/* ========================================================================== */
int lire(char *reponse) {
	char *c;

	c = reponse;
	do {
		*c = getchar();
		if((*c == 0x0a) || (*c == EOF)) { *c = '\0'; return(c - reponse); }
		c++;
	} forever;
}
/* ========================================================================== */
static void sortie(char *commentaire) {
/* Procedure de fin d'execution */
	int i; int64 t; float reel,cpu;
#ifdef MODULES_RESEAU
	TypeSvrBox cpulink; int rc;
#endif
	
	i = 86; while(i--) printf("="); printf("\n");
	if(errno) printf("\n%s\nExecution terminee avec l'erreur %d (%s)\n",
		commentaire? commentaire: "",errno,strerror(errno));
	else printf("%s\nExecution terminee\n",commentaire? commentaire: "");
	if((SambaPartageId != -1) && SambaPartage) {
		i = 256; while((shmdt(SambaPartage) != -1) && i) i-- ;
	}
#ifdef MODULES_RESEAU
	if(SambaMode != SAMBA_MONO) {
		CarlaDataExit();
		/* de-assignation du role des CPU en jeu (central, satellite) */
		rc = SvrOpen(&cpulink,NomExterne,PortSvr,TailleSvr);
		if(!rc) {
			OpiumError("Connexion avec %s impossible",NomExterne);
			printf("Connexion avec le serveur CARLA local impossible (%s)\n",strerror(errno));
		} else {
			SvrSend(&cpulink,"stop",0,0);
			SvrRecv(&cpulink);
			SvrClose(&cpulink);
		}
	}
#endif
#ifdef UTILISE_DRIVER
	if(SambaMode != SAMBA_DISTANT) {
		for(i=0; i<PCInb; i++) DriverDisconnect(PCIacces[i],ICA_CLOSE);
	}
#endif
	OpiumExit();
#ifdef PAROLE
	SpeakString("\pBye");
#endif
//#ifdef CODE_WARRIOR_VSN
//	if(commentaire) printf("Faire <Command-Q><Return>\n");
//#endif
	gettimeofday(&SambaHeureFin,0);
	t = (((int64)(SambaHeureFin.tv_sec - SambaHeureDebut.tv_sec)) * 1000000) + (int64)(SambaHeureFin.tv_usec - SambaHeureDebut.tv_usec);
	reel = (float)((double)t / 1000000.0);
	cpu = (float)clock()/(float)CLOCKS_PER_SEC; // (float)sysconf(_SC_CLK_TCK);
	printf("Temps reel utilise: %13.3f s\n",reel);
	printf("Temps CPU  utilise: %13.3f s (%5.1f %%%%)\n",cpu,100.0 * cpu / reel);
	exit(errno);
}
/* ========================================================================== */
void PMD() {
}

#pragma mark ---- utilitaires generalistes ----
/* ========================================================================== */
static void SambaLogTrait(char c, char *titre) {
	int i; char *t;

	putchar('\n'); i = 86;
	if(titre) {
		while(i-- > 81) putchar(c); i--; putchar(' ');
		t = titre; while(*t) {  i--; putchar(*t++); }; i--; putchar(' ');
	}
	while(i--) putchar(c); putchar('\n');
}
/* ======================================================================= */
void SambaLogDef(char *definition, char *mode, char *nom) {
	char fmt[64]; int lngr,liaison;

	lngr = strlen(definition);
//-if(definition[0] == '.') lngr += 2;
	liaison = (mode)? strlen(mode): 4;
	if((lngr + liaison) > SambaDefMax) SambaDefMax = lngr + liaison;
	else lngr = SambaDefMax - liaison;
	if(definition[0] == '.')
		sprintf(fmt,"  %%-%ds %%s '%%s'\n",lngr);
	else if(definition[0] == '=')
		sprintf(fmt,"%%-%ds %%s '%%s'\n",lngr+2);
	else sprintf(fmt,"%c %%-%ds %%s '%%s'\n",mode?' ':(nom?'=':' '),lngr);
	printf(fmt,definition,mode? mode:(nom?"dans":" "),nom?nom:" ");
}
/* ========================================================================== */
void SambaCompleteLigne(int tot, int cur) {
	int i; for(i=cur; i<(tot-1); i++) printf(" "); printf("|\n");
}
/* ========================================================================== */
void SambaLigneTable(char c, int cur, int tot) {
	int i;
	if(!cur) cur = printf("|");
	for(i=cur; i<(tot-1); i++) printf("%c",c); printf("|\n");
}
/* ========================================================================== */
int SambaUpdateInstrumFromItem(Panel p, int num, void *arg) {
//	void *arg; arg = PanelItemArg(p,num); 
	InstrumRefreshVar((Instrum)arg); return(0);
}
/* ========================================================================== */
int SambaUpdatePanel(Instrum i) {
	void *arg; arg = InstrumChangeArg(i); PanelRefreshVars(arg); return(0);
}
/* ========================================================================== */
void SambaRefreshPanel(Instrum i) {
	void *arg; arg = InstrumChangeArg(i); OpiumRefresh(((Panel)arg)->cdr);
}
/* ========================================================================== */
static int SambaInsere(TypeClassement *liste, int nb, CLASSE_TYPE type, void *ref) {
	int j,n,dn,inf,sup,diff,num; int val; char *txt; char init,identique;

	/* if(type == CLASSE_ENTIER) 
		printf("(%s) ___ Insertion #%d de '%d'\n",__func__,nb,(int)ref);
	else printf("(%s) ___ Insertion #%d de '%s'\n",__func__,nb,(char *)ref); */
	val = (int)ref; txt = (char *)ref; // GCC
	identique = 0;
	if(nb == 0) num = 0;
	else if(type == CLASSE_INDEFINIE) num = nb;
	else {
		if(type == CLASSE_ENTIER) val = (int)ref; else txt = (char *)ref;
		inf = 0; sup = nb - 1; init = 1;
		n = sup;
		do {
			if(type == CLASSE_ENTIER) diff = val - (int)(liste[n].ref);
			else diff = strcmp(txt,(char *)(liste[n].ref));
			/* if(type == CLASSE_ENTIER) 
				printf("(%s) | Compare avec liste[%d]->%d='%d': %d\n",__func__,n,liste[n].num,(int)(liste[n].ref),diff);
			else printf("(%s) | Compare avec liste[%d]->%d='%s': %d\n",__func__,n,liste[n].num,(char *)(liste[n].ref),diff); */
			if(diff > 0) {
				if(n >= (nb - 1)) { num = nb; break; }
				dn = (sup - n) / 2; 
				if(dn == 0) { num = n + 1; break; }
				inf = n; n += dn;
			} else if(diff < 0) {
				if(n <= 0) { num = n; break; }
				if(init) dn = sup - inf; else dn = (n - inf) / 2;
				if(dn == 0) { num = n; break; }
				sup = n; n -= dn; init = 0;
			} else /* (diff == 0) */ { num = n; identique = 1; break; }
			// printf("(%s) | recherche dans [%d - %d]\n",__func__,inf,sup);
		} while(1);
	}
	if(identique) {
		while(++num < nb) {
			if(type == CLASSE_ENTIER) diff = val - (int)(liste[num].ref);
			else diff = strcmp(txt,(char *)(liste[num].ref));
			if(diff) break;
		}
	}
	/* printf("(%s) | Insertion en position %d\n",__func__,num); */
	for(j=nb; j>num; --j) memcpy(&(liste[j]),&(liste[j - 1]),sizeof(TypeClassement));
	liste[num].num = nb; liste[num].ref = ref;
	nb++;
	return(nb);
}
#ifdef A_TERMINER
/* ========================================================================== */
static void SambaClasseEvalueNumeriseur(int bb, CLASSE_INFO critere, void *eval, char *type) {
	int bolo,cap; short galette,tour,branche; int val;

	cas_ou(critere) {
	  vaut CLASSE_NOM:      eval = (void *)(Numeriseur[bb].nom); *type = CLASSE_TEXTE; break;
	  vaut CLASSE_ENTREE:  
		eval = (void *)((Numeriseur[bb].repart)? ((((Numeriseur[bb].repart)->rep & 0x7FFF) << 16) | Numeriseur[bb].entree): 0x0fff0000);
		*type = CLASSE_ENTIER; 
		break;
	  vaut CLASSE_POSITION: vaut CLASSE_TOUR:
		for(bolo=0; bolo<BoloNb; bolo++) {
			for(cap=0; cap<Bolo[bolo].nbcapt; cap++) if(Bolo[bolo].captr[cap].bb.num == bb) break;
			if(cap < Bolo[bolo].nbcapt) break;
		}
		if(bolo < BoloNb) {
			if(critere == CLASSE_POSITION) val = Bolo[bolo].pos;
			else if(critere == CLASSE_TOUR) {
				CablageAnalysePosition(Bolo[bolo].pos,&galette,&tour,&branche);
				val = (((tour & 0xF) << 4) | (galette & 0xF));
			}
		} else val = CABLAGE_INDEFINI;
		eval = (void *)val;
		*type = CLASSE_ENTIER; 
		break;
	  vaut CLASSE_FISCHER:  eval = (void *)(int)(Numeriseur[bb].fischer); *type = CLASSE_ENTIER; break;
	  vaut CLASSE_FIBRE:    eval = (void *)(Numeriseur[bb].def.fibre); *type = CLASSE_TEXTE; break;
	}
}
#endif
/* ========================================================================== */
void SambaClasseMontre(CLASSE_OBJET classe, CLASSE_INFO critere) {
	int i,bb,bolo; char serial[4];
	
	if(!OpiumDisplayed(tClasse[classe]->cdr)) OpiumDisplay(tClasse[classe]->cdr); /* pour bien afficher des la premiere fois */
	TermEmpty(tClasse[classe]);
	TermHold(tClasse[classe]);
	TermPrint(tClasse[classe],"----------------------------------------------------\n");
	selon_que(classe) {
	  vaut CLASSE_NUMER:
		serial[3] = '\0';
		TermPrint(tClasse[classe],"| Classement de %d numeriseur%s par %s\n",Accord1s(NumeriseurNb),ClasseInfo[critere]);
		TermPrint(tClasse[classe],"|---------------------------------------------------\n");
		TermPrint(tClasse[classe],"|     rang fischer > numeriseur > [fbr] > rep.entree\n");
		TermPrint(tClasse[classe],"|\n");
		pour_tout(i,NumeriseurNb) {
			if((bb = OrdreNumer[i].num) < 0)
				TermPrint(tClasse[classe],"| %3d) {%3d}:      >        #        > [   ] > (invalide)\n",i+1,bb+1);
			else {
				if(Numeriseur[bb].def.serial == 0xFF) serial[0] = '\0'; else snprintf(serial,3,"%02d",Numeriseur[bb].def.serial);
				TermPrint(tClasse[classe],"| %3d) {%3d}: %4s > %7s#%-3s > [%3s] > ",i+1,bb+1,CablageEncodeConnecteur(Numeriseur[bb].fischer),
						  ModeleBN[(int)Numeriseur[bb].def.type].nom,serial,Numeriseur[bb].def.fibre);
				if(Numeriseur[bb].repart) TermPrint(tClasse[classe],"%s.%d\n",Repart[(Numeriseur[bb].repart)->rep].code_rep,Numeriseur[bb].entree+1);
				else TermPrint(tClasse[classe],"<neant>\n");
			}
		}
		sors;
	  vaut CLASSE_DETEC:
		TermPrint(tClasse[classe],"| Classement de %d detecteur%s par %s\n",Accord1s(BoloNb),ClasseInfo[critere]);
		TermPrint(tClasse[classe],"|---------------------------------------------------\n");
		TermPrint(tClasse[classe],"|    rang position > detecteur\n");
		TermPrint(tClasse[classe],"|\n");
		pour_tout(i,BoloNb) {
			if((bolo = OrdreDetec[i].num) < 0)
				TermPrint(tClasse[classe],"| %3d) {%3d}:      > (invalide)\n",i+1,bolo+1);
			else TermPrint(tClasse[classe],"| %3d) {%3d}: %4s > %s\n",i+1,bolo+1,CablageEncodePosition(Bolo[bolo].pos),Bolo[bolo].nom);
		}
		sors;
	  au_pire: sors;
	}
	TermPrint(tClasse[classe],"----------------------------------------------------\n");
	TermRelease(tClasse[classe]);
	//	if((tClasse[classe]->cdr)->displayed) OpiumClear(tClasse[classe]->cdr);
	OpiumDisplay(tClasse[classe]->cdr);
}
/* ========================================================================== */
void SambaClasseImprime(CLASSE_OBJET classe, CLASSE_INFO critere) {
	int i,bb,bolo; char serial[4];

	printf("\n");
	selon_que(classe) {
	  vaut CLASSE_NUMER:
		serial[3] = '\0';
		printf("* Classement de %d numeriseur%s par %s:\n",Accord1s(NumeriseurNb),ClasseInfo[critere]);
		printf("  |  rang fischer > numeriseur  > [fbr] > rep.entree\n");
		printf("  |----------------------------------------------------\n");
		pour_tout(i,NumeriseurNb) {
			bb = OrdreNumer[i].num;
			if((bb = OrdreNumer[i].num) < 0)
				printf("  | %2d) {%2d}:      >        #         > [   ] > (invalide\n)",i+1,bb+1);
			else {
				if(Numeriseur[bb].def.serial == 0xFF) serial[0] = '\0'; else snprintf(serial,3,"%02d",Numeriseur[bb].def.serial);
				printf("  | %2d/ {%2d}: %3s > %7s#%-3s > [%3s] > ",i,bb,CablageEncodeConnecteur(Numeriseur[bb].fischer),
					   ModeleBN[(int)Numeriseur[bb].def.type].nom,serial,Numeriseur[bb].def.fibre
					   );
				if(Numeriseur[bb].repart) printf("%s.%d\n",Repart[(Numeriseur[bb].repart)->rep].code_rep,Numeriseur[bb].entree+1);
				else printf("<neant>\n");
			}
		}
		printf("  -----------------------------------------------------\n");
		sors;
	  vaut CLASSE_DETEC:
		printf("* Classement de %d detecteur%s par %s\n",Accord1s(BoloNb),ClasseInfo[critere]);
		printf("  |    rang position > detecteur\n");
		printf("  |-----------------------------\n");
		pour_tout(i,BoloNb) {
			if((bolo = OrdreDetec[i].num) < 0)
				printf("  | %3d) {%3d}:      > (invalide)\n",i+1,bolo+1);
			else printf("  | %3d) {%3d}: %4s > %s\n",i+1,bolo+1,CablageEncodePosition(Bolo[bolo].pos),Bolo[bolo].nom);
		}
		printf("  ------------------------------\n");
		sors;
	  au_pire: sors;
	}
}
/* ========================================================================== */
void SambaClasse(CLASSE_OBJET classe, CLASSE_INFO critere, char modifie) {
	TypeClassement *liste; char change; int i; int val;
	int bb,bolo,cap; short galette,tour,branche; char serial[4];
	
	val = 0; // GCC
	change = 0;
	serial[3] = '\0';
	selon_que(classe) {
	  vaut CLASSE_NUMER:
		if(critere == CLASSE_NEANT) critere = ClassementCourant[classe];
		else {
			liste = Malloc(NumeriseurNb,TypeClassement);
			if(!liste) sors;
			pour_tout(bb,NumeriseurNb) {
				//- bb = OrdreNumer[i].num;
				// printf("(%s) Insertion #%d de la BB[%d] (%s)\n",__func__,i,bb,Numeriseur[bb].nom);
				selon_que(critere) {
				  vaut CLASSE_NOM:      SambaInsere(liste,bb,CLASSE_TEXTE, (void *)(Numeriseur[bb].nom)); sors;
				  vaut CLASSE_ENTREE:  
					val = (Numeriseur[bb].repart)? ((((Numeriseur[bb].repart)->rep & 0x7FFF) << 16) | Numeriseur[bb].entree): 0x0fff0000;
					SambaInsere(liste,bb,CLASSE_ENTIER,(void *)val);
					sors;
				  vaut CLASSE_POSITION: vaut CLASSE_TOUR:
					for(bolo=0; bolo<BoloNb; bolo++) {
						for(cap=0; cap<Bolo[bolo].nbcapt; cap++) if(Bolo[bolo].captr[cap].bb.num == bb) break;
						if(cap < Bolo[bolo].nbcapt) break;
					}
					if(bolo < BoloNb) {
						if(critere == CLASSE_POSITION) val = Bolo[bolo].pos;
						else if(critere == CLASSE_TOUR) {
							CablageAnalysePosition(Bolo[bolo].pos,&galette,&tour,&branche);
							val = (((tour & 0xF) << 4) | (galette & 0xF));
						}
					} else val = CABLAGE_INDEFINI;
					SambaInsere(liste,bb,CLASSE_ENTIER,(void *)val);
					sors;
				  vaut CLASSE_FISCHER:  SambaInsere(liste,bb,CLASSE_ENTIER,(void *)(int)(Numeriseur[bb].fischer)); sors;
				  vaut CLASSE_FIBRE:    SambaInsere(liste,bb,CLASSE_TEXTE, (void *)(Numeriseur[bb].def.fibre)); sors;
				  au_pire:              SambaInsere(liste,bb,CLASSE_INDEFINIE,0); sors;
				}
				/* { int j; pour_tout(j,(i+1)) {
					bb = liste[j].num;
					printf("(%s) | liste[%d].num=%d {%-10s > %3d} / .ref=%8X\n",__func__,j,bb,Numeriseur[bb].nom,Numeriseur[bb].fischer,(hexa)(liste[j].ref));
				} } */
			}
			pour_tout(i,NumeriseurNb) memcpy(&(OrdreNumer[i]),&(liste[i]),sizeof(TypeClassement));
			free(liste);
			change = 1;
		}
		if(CompteRendu.numer.classmt) SambaClasseImprime(classe,critere);
		if(modifie) {
			SambaClasseMontre(classe,critere);
			if(change && NumeriseurListeExiste) { if(OpiumCheck("Sauver ce classement des numeriseurs")) NumeriseursEcrit(FichierPrefNumeriseurs,NUMER_CONNECTION,NUMER_TOUS); }
		}
		if(change && OpiumDisplayed(bNumeriseurEtat)) NumeriseurRapport(0,0);
		sors;

	  vaut CLASSE_DETEC:
		if(critere == CLASSE_NEANT) critere = ClassementCourant[classe];
		else {
			liste = Malloc(BoloNb,TypeClassement);
			if(!liste) sors;
			pour_tout(bolo,BoloNb) {
				//- bolo = OrdreDetec[i].num;
				selon_que(critere) {
				  vaut CLASSE_NOM:      SambaInsere(liste,bolo,CLASSE_TEXTE, (void *)(Bolo[bolo].nom)); break;
				  vaut CLASSE_POSITION: SambaInsere(liste,bolo,CLASSE_ENTIER,(void *)(int)(Bolo[bolo].pos)); break;
				  vaut CLASSE_TOUR:
					CablageAnalysePosition(Bolo[bolo].pos,&galette,&tour,&branche);
					val = (((tour & 0xF) << 4) | (galette & 0xF));
					SambaInsere(liste,bolo,CLASSE_ENTIER,(void *)val);
					break;
				  vaut CLASSE_FISCHER: vaut CLASSE_FIBRE: vaut CLASSE_ENTREE:
				  au_pire:              SambaInsere(liste,bolo,CLASSE_INDEFINIE,0); break;
				}
			}
			pour_tout(i,BoloNb) memcpy(&(OrdreDetec[i]),&(liste[i]),sizeof(TypeClassement));
			free(liste);
			change = 1;
		}
		if(CompteRendu.detectr.classmt) SambaClasseImprime(classe,critere);
		if(modifie) {
			SambaClasseMontre(classe,critere);
			if(change && DetecteurListeExiste) { if(OpiumCheck("Sauver ce classement des detecteurs")) DetecteurSauveListe(); }
		}
		sors;
	  au_pire: sors;
	}
	if(change) ClassementCourant[classe] = critere;
}
/* ========================================================================== */
int SambaTri(Menu menu, MenuItem *item) {
	if(OpiumExec(pTri->cdr) == PNL_OK) SambaClasse(ClasseObjetATrier,ClasseCritereDeTri,1);
	return(0);
}
/* ========================================================================== */
void EnvirVarPrintVal(TypeEnvirVar *envir, char *texte, int max) {
	*texte = '\0';
	switch(envir->type) {
		case ARG_TYPE_INT:    snprintf(texte,max,"%d",*(int *)&(envir->var)); break;
		case ARG_TYPE_HEXA:   snprintf(texte,max,"%#x",*(int *)&(envir->var)); break;
		case ARG_TYPE_SHORT:  snprintf(texte,max,"%hd",*(short *)&(envir->var)); break;
		case ARG_TYPE_SHEX:   snprintf(texte,max,"%#hx",*(short *)&(envir->var)); break;
		case ARG_TYPE_FLOAT:  snprintf(texte,max,"%g",*(float *)&(envir->var)); break;
		case ARG_TYPE_CHAR:   snprintf(texte,max,"%d",*(char *)&(envir->var)); break;
		case ARG_TYPE_OCTET:  snprintf(texte,max,"%d",*(unsigned char *)&(envir->var)); break;
		case ARG_TYPE_BYTE:   snprintf(texte,max,"%#x",*(unsigned char *)&(envir->var)); break;
		case ARG_TYPE_LETTRE: snprintf(texte,max,"%c",*(char *)&(envir->var)); break;
		case ARG_TYPE_BOOL:   snprintf(texte,max,"%s",*(char *)&(envir->var)? "oui":"non"); break;
		case ARG_TYPE_KEY:    ArgKeyGetText(envir->mot_cles,envir->var,texte,max); break;
	}
	*(texte+max-1) = '\0';
}
/* ========================================================================== */
void SambaPositionDecode(TypeChassisAxe *direction, int dim, char *code, short *position) {
	int i,j,k,l,n; char c,nom[8];

	k = 0; i = 0;
	while((c = code[k])) {
		if(i >= dim) break;
		if(direction[i].codage == CHASSIS_CODE_NOMS) {
			position[i] = 0;
			n = 0;
			while(ArgKeyGetText(direction[i].cles,n,nom,8)) {
				l = strlen(nom);
				if(!strncmp(code+k,nom,l)) { position[i] = n; k += l; break; }
				n++;
			}
		} else if((c >= 'a') && (c <= 'z')) { position[i] = c - 'a'; k++; }
		else if((c >= 'A') && (c <= 'Z')) { position[i] = c - 'A'; k++; }
		else {
			if((direction[i].codage == CHASSIS_CODE_0) || (direction[i].codage == CHASSIS_CODE_1)) {
				j = k;
				while((c = code[k])) if((c < '0') || (c > '9')) break; else k++;
				sscanf(code+j,"%d",&n);
				position[i] = (direction[i].codage == CHASSIS_CODE_0)? n: n-1;
			}
		}
		i++;
	}
}
/* ========================================================================== */
void SambaPositionEncode(TypeChassisAxe *direction, int dim, short *position, char *code) {
	int i,l,k,m,n; char nom[8];
	char prececent_numerique,actuel_numerique;
	
	prececent_numerique = 0; l = 0; m = 0;
	for(i=0; i<dim; i++) {
		actuel_numerique = 0;
		k = m;
		switch(direction[i].codage) {
			case CHASSIS_CODE_a: code[l++] = 'a' + position[i]; break;
			case CHASSIS_CODE_A: code[l++] = 'A' + position[i]; break;
			case CHASSIS_CODE_z: if(position[i]) code[l++] = 'a' + position[i] - 1; break;
			case CHASSIS_CODE_Z: if(position[i]) code[l++] = 'A' + position[i] - 1; break;
			case CHASSIS_CODE_1: k = m + 1;
			case CHASSIS_CODE_0:
				if(prececent_numerique) code[l++] = '.';
				n = sprintf(code+l,"%d",position[i]+k); l += n;
				actuel_numerique = 1;
				break;
			case CHASSIS_CODE_NOMS: 
				ArgKeyGetText(direction[i].cles,position[i],nom,8);
				n = sprintf(code+l,"%s",nom); l += n;
				break;
			case CHASSIS_CODE_RANG: 
				m += position[i] * direction[i].dim;
				break;
		}
		prececent_numerique = actuel_numerique;
	}
	code[l] = '\0';
}
/* ========================================================================== */
char SambaTrgrDecode(TypeTrigger *trgr, char *texte) {
	char *r,*c; char dlm1,dlm2,dlm3; int i; // char regul;
	 char txttrgr[TRGR_NOM];
	char std;
	
	std = 0;
	// regul = trgr->regul_demandee;
	r = texte;
	strncpy(txttrgr,texte,TRGR_NOM); txttrgr[TRGR_NOM - 1] = '\0';
	r = txttrgr;
	c = ItemTrouve(&r,"<>@",&dlm1);
	if(!strcmp(c,"standard")) std = 1;
	else if(strcmp(c,"--") && strcmp(c,"indetermine")) {
		trgr->sens = 1;
		if(!strcmp(c,"montee")) { i = TRGR_FRONT; trgr->sens = 2; }
		else if(!strcmp(c,"desc.")) { i = TRGR_FRONT; trgr->sens = 0; }
		else for(i=0; i<TRGR_NBCABLAGES; i++) if(!strcmp(c,TrgrTypeListe[i])) break;
		if(i >= TRGR_NBCABLAGES) trgr->type = NEANT;
		else {
			trgr->type = i;
			if((i == TRGR_SEUIL) || (i == TRGR_DERIVEE)) {
				c = ItemTrouve(&r,"<>=",&dlm2);
				if(dlm1 == '<') { sscanf(c,"%g",&(trgr->maxampl)); }
				else if(dlm1 == '>') { sscanf(c,"%g",&(trgr->minampl)); }
				if((dlm2 != '=') && (dlm2 != '\0')) {
					c = ItemTrouve(&r,"<>=",&dlm3);
					if(dlm2 == '<') { sscanf(c,"%g",&(trgr->maxampl)); }
					else if(dlm2 == '>') { sscanf(c,"%g",&(trgr->minampl)); }
				}
				if(((dlm1 == '<') && (dlm2 != '>')) || ((dlm2 == '<') && (dlm1 != '>'))) trgr->sens = 0;
				else if(((dlm1 == '>') && (dlm2 != '<')) || ((dlm2 == '>') && (dlm1 != '<'))) trgr->sens = 2;
				if((i == TRGR_DERIVEE) && ((dlm2 == '=') || (dlm3 == '='))) {
					c = ItemJusquA(0,&r);
					sscanf(c,"%g",&(trgr->montee));
				}
			} else if(i == TRGR_FRONT) {
				c = ItemJusquA('>',&r);
				sscanf(c,"%g",&(trgr->minampl));
				c = ItemJusquA(0,&r);
				sscanf(c,"%g",&(trgr->montee));
			} else if(i == TRGR_ALEAT) {
				c = ItemJusquA('*',&r);
				sscanf(c,"%g",&(trgr->porte));
				// c = ItemJusquA(0,&r);
				// sscanf(c,"%g",&(trgr->fluct));
			}
		}
	}
	// trgr->regul_demandee = regul;
	return(std);
}
/* ========================================================================== */
void SambaTrgrEncode(TypeTrigger *trgr, char *texte, int max) {
	char tempo[1024];
	int l;

	if(trgr->std) strcpy(tempo,"standard");
	else selon_que(trgr->type) {
	  vaut NEANT: strcpy(tempo,"--"); break;
	  vaut TRGR_SEUIL:
		l = sprintf(tempo,"%s",TrgrTypeListe[(int)trgr->type]);
		if(trgr->sens <= 1) l += sprintf(tempo+l," < %-4g ",trgr->maxampl);
		if(trgr->sens >= 1) l += sprintf(tempo+l," > %g ",trgr->minampl);
		// printf("* Trigger %s: '%s' (%d/%d chars)\n",VoieManip[voie].nom,tempo,l,TRGR_NOM);
		break;
	  vaut TRGR_FRONT:
		if(trgr->sens < 1) l = sprintf(tempo,"%s","desc.");
		else if(trgr->sens > 1) l = sprintf(tempo,"%s","montee");
		else l = sprintf(tempo,"%s",TrgrTypeListe[(int)trgr->type]);
		l += sprintf(tempo+l," >%g ",trgr->minampl);
		l += sprintf(tempo+l," >%.3f ms ",trgr->montee);
		break;
	  vaut TRGR_DERIVEE:
		l = sprintf(tempo,"%s",TrgrTypeListe[(int)trgr->type]);
		if(trgr->sens <= 1) l += sprintf(tempo+l," < %-4g ",trgr->maxampl);
		if(trgr->sens >= 1) l += sprintf(tempo+l," > %g ",trgr->minampl);
		l += sprintf(tempo+l," =%.4f ms ",trgr->montee);
		break;
	  vaut TRGR_ALEAT:
		l = sprintf(tempo,"%s",TrgrTypeListe[(int)trgr->type]);
		l += sprintf(tempo+l," @%g Hz",trgr->porte);
		// m = l; l += sprintf(tempo+l," * %g %%",trgr->fluct);
		break;
	  au_pire: strcpy(tempo,"indetermine"); break;
	}
	strncpy(texte,tempo,max); texte[max-1] = '\0';
}
/* ========================================================================== */
static void SambaImprimeSupport(char *objet, TypeChassisAxe *direction, int dim) {
	int i; TypeChassisAxe *axe;

#ifdef V1
	printf("    . Chassis %s: ( ",objet);
	for(i=0,axe=direction; i<dim; i++,axe++) {
		if(i) printf(", ");
		switch(axe->codage) {
			case CHASSIS_CODE_a: printf("a-%c",'a'+axe->nb-1); break;
			case CHASSIS_CODE_A: printf("A-%c",'A'+axe->nb-1); break;
			case CHASSIS_CODE_z: printf("[a-%c]",'a'+axe->nb-1); break;
			case CHASSIS_CODE_Z: printf("[A-%c]",'A'+axe->nb-1); break;
			case CHASSIS_CODE_0: printf("0-%d",axe->nb-1); break;
			case CHASSIS_CODE_1: printf("1-%d",axe->nb); break;
			case CHASSIS_CODE_NOMS: printf("%s",axe->cles); break;
		}
	}
	printf(" )\n");
#else
	char prececent_numerique,actuel_numerique;
	printf("    . Chassis %s: ",objet);
	prececent_numerique = 0;
	for(i=0,axe=direction; i<dim; i++,axe++) {
		actuel_numerique = 0;
		switch(axe->codage) {
			case CHASSIS_CODE_a: printf("(a-%c)",'a'+axe->nb-1); break;
			case CHASSIS_CODE_A: printf("(A-%c)",'A'+axe->nb-1); break;
			case CHASSIS_CODE_z: printf("[a-%c]",'a'+axe->nb-1); break;
			case CHASSIS_CODE_Z: printf("[A-%c]",'A'+axe->nb-1); break;
			case CHASSIS_CODE_0: if(prececent_numerique) printf("."); printf("(0-%d)",axe->nb-1); actuel_numerique = 1; break;
			case CHASSIS_CODE_1: if(prececent_numerique) printf("."); printf("(1-%d)",axe->nb); actuel_numerique = 1; break;
			case CHASSIS_CODE_NOMS: printf("(%s)",axe->cles); break;
		}
		prececent_numerique = actuel_numerique;
	}
	printf("\n");
#endif
}
/* ========================================================================== */
INLINE void SambaTempsEchantillon(int64 echant, int s0, int us0, int *sec, int *usec) {
	int64 mega_echant;
	double fsecs,freste;
	int s,us,retenue;

	mega_echant = echant / 1000000;
	fsecs = (double)mega_echant * 1000.0 / Echantillonnage; /* secondes */
	s = (int)fsecs;            /* pas d'arrondi du style (fsecs + 0.5) */
	freste = fsecs - (double)s;
	us = (int)(((double)(echant - (mega_echant * 1000000)) * 1000.0 / Echantillonnage) + (freste * 1000000.0)) + us0; /* microsecondes */
	retenue = us / 1000000;
	*sec = s0 + s + retenue;
	*usec = us - (retenue * 1000000);
}
#ifdef NIDAQ
/* ========================================================================== */
int SambaTesteDAQmx(char *titre, int k, char log) {
	#define LECT_NI_MSGLNGR 256
	char explic[LECT_NI_MSGLNGR];
	int i,nbcols;

	if(log > 1) nbcols = log; else nbcols = 12;
	DAQmxBaseGetExtendedErrorInfo(explic,LECT_NI_MSGLNGR);
	if(k < 0) {
		for(i=0; i<nbcols; i++) printf(" ");
		if(titre) {
			printf("! %s en erreur #%d\n",titre,k);
			for(i=0; i<nbcols; i++) printf(" ");
			printf("  (%s)\n",explic);
		} else printf("! en erreur #%d: %s\n",k,explic);
	} else if(k > 0) {
		for(i=0; i<nbcols; i++) printf(" ");
		if(titre) printf("* %s: %s (code retour: %d)\n",titre,explic,k);
		else printf("* %s (code retour: %d)\n",explic,k);
	} else if(titre && log) {
		for(i=0; i<nbcols; i++) printf(" ");
		printf(". %s\n",titre);
	}
	return(k);
}
#endif
/* ========================================================================== */
int AccesRestreint() {
	char texte[16];
	
	if(ExpertConnecte) return(0);
	texte[0] = '\0';
	OpiumReadPswd("Code d'acces",texte,16);
	if(!strcmp(texte,"darkmatr")) return(0);
	else {
		OpiumError("La commande en cours est abandonnee");
		return(1);
	}
}
static char SambaBoutonChoisi;
/* ========================================================================== */
int SambaBoutonSauve(Menu menu, MenuItem *item) { SambaBoutonChoisi = 's'; return(1); }
/* ========================================================================== */
int SambaBoutonSort(Menu menu, MenuItem *item) { SambaBoutonChoisi = 'a'; return(1); }
/* ========================================================================== */
int SambaBoutonRevient(Menu menu, MenuItem *item) { SambaBoutonChoisi = 'r'; return(1); }
/* ========================================================================== */
int SambaSauve(char *a_sauver, char *objet, char genre, char nombre, TypeFctn fctn) {
	/* genre:  0  masculin (le)
	           1  feminin  (la)
	          -1  neutre   (l')
	  nombre:  0  un seul
	           1  plusieurs */
	char msg[64];

	if(*a_sauver) {
		snprintf(msg,63,L_("%s %s %s ete modifie%s%s"),
			nombre? L_("Les"): (genre? ((genre>0)? L_("La"): L_("L'")): L_("Le")),
			objet,
			nombre? L_("ont"): L_("a"),
			(genre > 0)? L_("e"): "",
			nombre? L_("s"):"");
#ifdef SAUVE_AVEC_MENU
		int rep; int x,y,h,v;
		InfoWrite(iSambaSauve,1,msg);
		OpiumPosition(iSambaSauve->cdr,200,200); /* enqueter: pourquoi pas au centre?? */
		OpiumDisplay(iSambaSauve->cdr);
		OpiumGetPosition(iSambaSauve->cdr,&x,&y); OpiumGetSize(iSambaSauve->cdr,&h,&v);
		OpiumPosition(mSambaQuitte->cdr,x,y+v+45);
		rep = OpiumExec(mSambaQuitte->cdr);
		OpiumClear(iSambaSauve->cdr);
		switch(rep) {
			case 's': (*fctn)(); break; /* si rend 0, return(0) */
			case 'a': *a_sauver = 0; break;
			case 'r': return(0);
		}
#else /* !SAUVE_AVEC_MENU */
		Cadre planche; Panel p; Menu m_s,m_a,m_r; int x,y;
		do {
			planche = BoardCreate();
			x = Dx; y = Dy;
			p = PanelCreate(1);
			PanelText(p,"ATTENTION!",msg,64); PanelMode(p,PNL_DIRECT|PNL_RDONLY);
			PanelSupport(p,WND_CREUX);
			BoardAddPanel(planche,p,x,y,0); y += 2 * Dy;
			SambaBoutonChoisi = '\0';
			m_s = MenuBouton(L_("Sauver les modifications"), MNU_FONCTION SambaBoutonSauve); MenuItemAllume(m_s,1,0,GRF_RGB_YELLOW);
			m_a = MenuBouton(L_("Abandonner les modifications"), MNU_FONCTION SambaBoutonSort); MenuItemAllume(m_a,1,0,GRF_RGB_GREEN);
			m_r = MenuBouton(L_("Revenir a SAMBA"), MNU_FONCTION SambaBoutonRevient); MenuItemAllume(m_r,1,0,WND_COLOR_DEMI,WND_COLOR_DEMI,WND_COLOR_MAX);
			BoardAddMenu(planche,m_s,x,y,0); x += 27 * Dx;
			BoardAddMenu(planche,m_a,x,y,0); x += 31 * Dx;
			BoardAddMenu(planche,m_r,x,y,0);
			
			OpiumExec(planche);
			BoardTrash(planche);
			BoardDelete(planche);
			selon_que(SambaBoutonChoisi) {
				vaut 's': (*fctn)(); sors; /* si rend 0, return(0) */
				vaut 'a': *a_sauver = 0; sors;
				vaut 'r': return(0);
				au_pire: OpiumError("Aucun choix n'a ete precise!");
			}
		} while(SambaBoutonChoisi == '\0');
#endif /* !SAUVE_AVEC_MENU */
	}
	return(1);
}
/* ========================================================================== */
static int SambaChangeArgs() {

	if(AccesRestreint()) return(0);
	if(OpiumExec(pArgs->cdr) == PNL_OK) {
		if(OpiumChoix(2,SambaNonOui,"Sauvegarde de ces arguments")) {
			ArgPrint("SambaArgs",SambaDesc);
			/* SambaAlloc(); */
			WndQueryExit = 1; /* fait sortir de SAMBA */
		}
	}
	return(0);
}
/* ========================================================================== */
static int SambaChangeSetup(Menu menu, MenuItem *item) {

	if(AccesRestreint()) return(0);
	if(OpiumExec(pSetup->cdr) == PNL_OK) {
		if(OpiumChoix(2,SambaNonOui,"Sauvegarde de cette organisation")) {
#ifdef ADRS_PCI_DISPO
#ifdef PCI_DIRECTIF
			if(PCIadrs[0] == SimulPCI) PCIadrs[0] = (Ulong *)-1;
#endif
#endif
			ArgPrint(ConfigGeneral,Setup);
			if((LoggerType == CAHIER_ELOG) && LoggerAdrs[0]) {
				char adrs[MAXFILENAME]; char *r,*ip,*port;
				strcpy(adrs,LoggerAdrs);
				r = adrs;
				ip = ItemSuivant(&r,':');
				port = ItemSuivant(&r,' ');
				if(*port == '\0') port = "8080";
				sprintf(LoggerCmde,"%s/executables/elog/elog -h %s -p %s %s",FullTopDir,ip,port,LoggerUser);
			}
			/* SambaAlloc(); */
			WndQueryExit = 1; /* fait sortir de SAMBA */
#ifdef ADRS_PCI_DISPO
#ifdef PCI_DIRECTIF
		} else {
			if(!PCIconnecte) PCIadrs[0] = SimulPCI;
#endif
#endif
		}
	}
	return(0);
}
/* ========================================================================== */
void SambaNommeTrmt(int voie, int classe) {
	int k;
	
	switch(VoieManip[voie].def.trmt[classe].type) {
	  case TRMT_DEMODUL:
		snprintf(VoieManip[voie].def.trmt[classe].texte,TRMT_NOM,"marge %d",VoieManip[voie].def.trmt[classe].parm);
		break;
	#ifdef TRMT_ADDITIONNELS
	  case TRMT_LISSAGE:
	  case TRMT_MOYENNE:
	  case TRMT_MAXIMUM:
		snprintf(VoieManip[voie].def.trmt[classe].texte,TRMT_NOM,"sur %d pts",VoieManip[voie].def.trmt[classe].parm);
		break;
	  case TRMT_DECONV:
		break;
	#endif
	  case TRMT_FILTRAGE:
		k = VoieManip[voie].def.trmt[classe].parm;
		if((k >= 0) && (k < FiltreNb)) strncpy(VoieManip[voie].def.trmt[classe].texte,FiltreGeneral[k].nom,TRMT_NOM);
		break;
	}
}
/* ========================================================================== */
void SambaNommeTousTrmt() {
	int voie,classe;
	
	for(voie=0; voie<VoiesNb; voie++) {
		for(classe=0; classe<TRMT_NBCLASSES; classe++) SambaNommeTrmt(voie,classe);
	}
}
/* ==========================================================================
static int SambaDecaleT0() {
	OpiumReadFloat("Origine des temps",&(VoieTampon[0].trmt[TRMT_AU_VOL].ab6[0]));
	return(0);
}
   ========================================================================== */
// static int SambaSauveModeles() { PasImplementee(); return(0); }
/* ========================================================================== */
// static int SambaMontreModelesCablage() { CablageModlEcrit(stdout); return(0); }
/* ========================================================================== */
int SambaChargeFpga(Menu menu, MenuItem *item) {
	char message[256];
	
	if(!NumeriseurSelection(1)) return(0);
	if((ModlNumerChoisi < 0) || (ModlNumerChoisi >= ModeleBNNb)) return(0);
	RepartChargeFpga(message);
	OpiumError(message);
	return(0);
}
/* ========================================================================== */
int SambaScriptLance(Menu menu, MenuItem *item) {
	if(bScriptCntl) OpiumFork(bScriptCntl);
	else OpiumWarn(L_("Aucun script n'a ete defini"));
	return(0);
}
/* ========================================================================== */
int SambaScriptExec(Panel p, void *arg) {
	char nom_complet[MAXFILENAME];

	if(SambaNumerSelect) { if(!NumeriseurSelection(0)) return(0); }
	strcat(strcpy(nom_complet,FichierPrefProcedures),SambaProcedure[SambaProcsNum]);
	ScriptExec(nom_complet,SambaProcedure[SambaProcsNum],"");
	return(0);
}
/* ========================================================================== */
static int SambaRepartCharge (Menu menu) { return(RepartiteursInitTous(REPART_CHARGE,Echantillonnage,menu)); }
/* ========================================================================== */
static int SambaRepartRestart(Menu menu) { return(RepartiteursInitTous(REPART_DEMARRE,Echantillonnage,menu)); }
/* ========================================================================== */
static int SambaRepartParms  (Menu menu) { return(RepartiteursInitTous(REPART_PARMS,Echantillonnage,menu)); }
/* ========================================================================== */
static int SambaRepartStoppe (Menu menu) {
	int rc;
	rc = RepartiteursInitTous(REPART_STOPPE,Echantillonnage,menu);
	BoardRefreshVars(bLecture);
	return(rc);
}
/* ==========================================================================
static int SambaRepartStoppe(Menu menu, MenuItem *item) {
	char ok; int rep;

	ok = 1;
	printf("%s/ Arret des repartiteurs locaux:\n",DateHeure());
	for(rep=0; rep<RepartNb; rep++) if(Repart[rep].local) {
		if(!RepartiteurStoppeTransmissions(repart,1)) ok = 0;
	}
	printf("          ... %s\n",ok?"termine.": "EN ERREUR !");

	return(0);
}
   ========================================================================== */
static int SambaFromMaitre(Menu menu, MenuItem *item) {
	int n;
		unsigned int lngr;
		struct {
			unsigned char cmde,voide;
			unsigned char c1,c2;
			unsigned short valeur;
			unsigned char vide[1500];
		} message;

	if(LectureMaitre < 0) printf("%s/ Pas de maitre a l'ecoute\n",DateHeure());
	else {
		printf("%s/ Message recu via <%d>: ",DateHeure(),LectureMaitre);
		n = recvfrom(LectureMaitre,(void *)&message,sizeof(message),0,&SktEntreeMaitre,&lngr);
		if(n > 0) 
			printf("%d octet%s, %02X=%c.%02X.%02X.%04X\n",Accord1s(n),message.cmde,message.cmde,message.c1,message.c2,message.valeur);
		else if(n == 0) printf("vide\n");
		else if(errno == EAGAIN) printf("neant [%d]\n",n);
		else printf("en erreur, %s [%d]\n",strerror(errno),n);
	}
	return(0);
}

#pragma mark ---- menus ----
/* ========================================================================== */
Menu mSambaReglagesBB1,mSambaReglages,mHardware,mSambaModeles,mSambaMatos;

Menu mSambaBarre,mSambaPlus;
Cadre cSambaClassique;
MenuItem iSambaBarre[] = {
	{ "Procedures",      MNU_MENU &mSambaProcs },
	{ "Reglages",        MNU_MENU &mSambaReglages },
	{ "Affichages",      MNU_MENU &mMonitBarre },
	{ "Runs",            MNU_MENU &mLectRuns },
	{ "Analyse",         MNU_MENU &mCalc },
	{ "Complements",     MNU_MENU &mSambaPlus },
	{ "Terminer",        MNU_RETOUR },
	MNU_END
};
MenuItem iSambaPlus[] = {
	{ "Gestion",         MNU_MENU &mGest },
	{ "Impressions",     MNU_MENU &mImprim },
	{ "Hardware",        MNU_MENU &mHardware },
//	{ "SAMBA classique", MNU_FORK &cSambaClassique },
	{ "Quitter",         MNU_RETOUR },
	MNU_END
};

int LectAcqStd(),LectStop();
#ifdef LECT_MODE_ABANDONNE
int LectCompensation();
#endif
int LectVI(),LectRT();
int RepartiteurContacte(),RepartiteurStoppe(),DetecteurRegenLimitee();
MenuItem iSambaProcsComplet[] = {
	{ "Mise en route",              MNU_SEPARATION },
//	{ "Verification electronique",  MNU_FONCTION DiagComplet },
	{ "Demarrage repartiteurs",     MNU_FONCTION RepartiteurDemarre },
	{ "Reconnexion repartiteurs",   MNU_FONCTION RepartiteurContacte },
	{ "Arret repartiteurs",         MNU_FONCTION RepartiteurStoppe },
	{ "Chargement FPGA numeriseur", MNU_FONCTION SambaChargeFpga },
	{ "Reconnaissance numeriseurs", MNU_FONCTION LectIdentBB },
	{ "Demarrage numeriseurs",      MNU_FONCTION NumeriseursDemarre },
	{ "Parametrage detecteurs",     MNU_FONCTION DetecteurParmsEntree },
	{ "Mise en service detecteurs", MNU_FONCTION DetecteurMiseEnService },
	{ "Operations particulieres",   MNU_SEPARATION },
	{ "Scripts",                    MNU_FONCTION SambaScriptLance },
	{ "Inversion polarisation",     MNU_FONCTION DetecteurFlip },
	{ "RAZ FETs",                   MNU_FONCTION DetecteurRazFetEDW },
	{ "Regeneration",               MNU_FONCTION DetecteurRegenLimitee },
	{ "Procedures complexes",       MNU_SEPARATION },
//	{ "Spectres multiples",         MNU_FONCTION LectSpectres },
	{ "Spectres de bruit",          MNU_FORK   &bLectSpectres },
	{ "Courbes V(I)",               MNU_FONCTION LectVI },
	{ "Courbes R(T)",               MNU_FONCTION LectRT },
//	{ "Arret d'urgence",            MNU_FONCTION LectStop },
	{ "\0",                         MNU_SEPARATION },
	{ "Quitter",                    MNU_RETOUR },
	MNU_END
};
MenuItem iSambaProcsRestreint[] = {
	{ "Reconnexion repartiteur",    MNU_FONCTION RepartiteurContacte },
	{ "Mise en service detecteurs", MNU_FONCTION DetecteurMiseEnService },
	{ "Inversion polarisation",     MNU_FONCTION DetecteurFlip },
	{ "Scripts",                    MNU_FONCTION SambaScriptLance },
	{ "Spectres de bruit",          MNU_FORK   &bLectSpectres },
	{ "Quitter",                    MNU_RETOUR },
	MNU_END
};
int SettingsTrmtAffiche(Menu menu, MenuItem *item);
int SettingsEvtsAffiche(Menu menu, MenuItem *item);
int SettingsTrgrAffiche(Menu menu, MenuItem *item);
int SettingsRgulAffiche(Menu menu, MenuItem *item);
MenuItem iSambaReglages[] = {
	{ "Reglage Voie",          MNU_FONCTION  DetecteurReglages },
//	{ "Prise de donnees",      MNU_FORK  &bLecture },
	{ "Conditions de lecture", MNU_SEPARATION },
	{ "Traitements",           MNU_FONCTION  SettingsTrmtAffiche },
	{ "Definition evenements", MNU_FONCTION  SettingsEvtsAffiche },
	{ "Trigger",               MNU_FONCTION  SettingsTrgrAffiche },
	{ "Regulation du taux",    MNU_FONCTION  SettingsRgulAffiche },
	{ "Options",               MNU_FORK  &bOptions },
	{ "Tableaux de commande",  MNU_SEPARATION },
	{ "Controles",             MNU_FORK  &bCntrl },
    { "Automates",             MNU_FORK  &bAutomate },
#ifdef BRANCHE_TESTS
	{ "Tests",                 MNU_FORK  &bTests },
#endif
/*	{ "Tous les menus",        MNU_FORK  &cSambaClassique }, */
	{ "\0",                    MNU_SEPARATION },
	{ "Quitter",               MNU_RETOUR },
	MNU_END
};
int BasicCommandes();
int RepartDiffuse();
int NumeriseurExec();
Menu mSambaRepartiteurs,mSambaNumeriseurs,mSambaDetecteurs,mDetecConfig,mSambaDefinitions;
MenuItem iSambaRepartiteurs[] = {
	{ "Rechercher",                 MNU_FONCTION RepartiteurChercheIP         },
	{ "Selection",                  MNU_FONCTION RepartiteurRedefiniPerimetre },
	{ "Charger",                    MNU_FONCTION SambaRepartCharge            },
	{ "Demarrer",                   MNU_FONCTION SambaRepartRestart           },
	{ "Parametrer",                 MNU_FONCTION SambaRepartParms             },
	{ "Commandes",                  MNU_FONCTION RepartDiffuse                },
	{ "Etat",                       MNU_FONCTION RepartiteursStatus           },
	{ "Connexion",                  MNU_FONCTION RepartiteursCommande         },
	{ "Stopper",                    MNU_FONCTION SambaRepartStoppe            },
	{ "Quitter",                    MNU_RETOUR },
	MNU_END
};
MenuItem iSambaNumeriseurs[] = {
	{ "Magasin",                    MNU_FONCTION OrgaEspaceNumeriseur         },
//	{ "Banc de tests",              MNU_FONCTION BancAffichePlanche           },
	{ "Selection",                  MNU_FONCTION NumeriseursSelectionneLarge  },
	{ "Deblocage",                  MNU_FONCTION NumeriseursDebloqueTous      },
	{ "Initialisation",             MNU_FONCTION NumeriseursInitTous          },
	{ "Reblocage",                  MNU_FONCTION NumeriseursRebloqueTous      },
	{ "Etat",                       MNU_FONCTION NumeriseurRapport            },
	{ "Connexion",                  MNU_FONCTION NumeriseurExec               },
	{ "Quitter",                    MNU_RETOUR },
	MNU_END
};
MenuItem iSambaDetecteurs[] = {
	{ "Magasin",                    MNU_FONCTION OrgaEspaceDetecteur  },
	{ "Activation",                 MNU_FONCTION DetecteurActive      },
	{ "Cablage",                    MNU_FONCTION OrgaDetecteurMontre  },
	{ "Etat",                       MNU_PANEL  &pEtatBolos            },
	{ "Parametrage",                MNU_FONCTION DetecteurParmsEntree },
	{ "Configurations",             MNU_MENU   &mDetecConfig          },
	{ "Sauver",                     MNU_FONCTION DetecteurEcritTout   },
	{ "Quitter",                    MNU_RETOUR },
	MNU_END
};
MenuItem iDetecConfig[] = {
	{ "Dupliquer courante",         MNU_FONCTION SambaConfigDuplic  },
	{ "Renommer",                   MNU_FONCTION SambaConfigRenomme },
	{ "Quitter",                    MNU_RETOUR },
	MNU_END
};
MenuItem iSambaDefinitions[] = {
//	{ "Impression modeles cablage", MNU_FONCTION SambaMontreModelesCablage },
	{ "Arguments d'appel",          MNU_FONCTION SambaChangeArgs   },
	{ "Organisation",               MNU_FONCTION SambaChangeSetup  },
//	{ "Sauver fichier modeles",     MNU_FONCTION SambaSauveModeles },
	{ "Gestion materiel directe",   MNU_SEPARATION                 },
	{ "Import CSV",                 MNU_FONCTION OrgaImporte       },
	{ "Edition",                    MNU_MENU   &mSambaMatos        },
	{ "Export CSV",                 MNU_FONCTION OrgaExporte       },
	{ "Perimetre",                  MNU_FONCTION RepartiteurRedefiniPerimetre },
	{ "Ecoute maitre",              MNU_FONCTION SambaFromMaitre   },
//	{ "Origine des temps",          MNU_FONCTION SambaDecaleT0 },
// { "Tests graphiques",           MNU_FONCTION OrgaTraceTout      },
	{ "Quitter",                    MNU_RETOUR },
	MNU_END
};
MenuItem iSwDump[] = {
	{ "Contenu Fifo",               MNU_FONCTION LectDumpFifo   },
	{ "Contenu tampons",            MNU_FONCTION LectBuffDump   },
	{ "Pointeurs tampons",          MNU_FONCTION LectBuffStatus },
	{ "Quitter",                    MNU_RETOUR },
	MNU_END
};
int BancAffichePlanche();
MenuItem iHardware[] = {
	{ "Conception des modeles",     MNU_MENU   &mSambaModeles       },
	{ "Organigramme",               MNU_FONCTION OrgaMagasinVisite  },
	{ "",                           MNU_SEPARATION                  },
	{ "Detecteurs",                 MNU_MENU   &mSambaDetecteurs    },
	{ "Cablage",                    MNU_FONCTION OrgaMagasinVisite  },
	{ "Numeriseurs",                MNU_MENU   &mSambaNumeriseurs   },
	{ "Repartiteurs",               MNU_MENU   &mSambaRepartiteurs  },
	{ "Support",                    MNU_MENU   &mGestAutom          },
#ifdef macintosh
	{ "Etat reseau",       MNU_COMMANDE "open /System/Library/PreferencePanes/Network.prefPane/"  },
#else
	{ "Etat reseau",       MNU_COMMANDE "bash ifconfig &"           },
#endif
	{ "",                           MNU_SEPARATION                  },
	{ "Fonctions basiques",         MNU_FONCTION BasicCommandes     },
	{ "Contenus",                   MNU_MENU   &mSwDump             },
	{ "Diagnostic",                 MNU_MENU   &mDiag               },
	{ "Acces fichiers",             MNU_MENU   &mSambaDefinitions   },
	{ "Quitter",                    MNU_RETOUR },
	MNU_END
};
MenuItem iSambaQuitte[] = {
	{ "Sauver les modifications",     MNU_CONSTANTE 's' },
	{ "Abandonner les modifications", MNU_CONSTANTE 'a' },
	{ "Revenir a SAMBA",              MNU_CONSTANTE 'r' },
	MNU_END
};

Menu mSambaClassique;
MenuItem iSambaClassique[] = {
#ifndef MENU_BARRE
	{ "Procedures",      MNU_MENU &mSambaProcs },
	{ "Boitiers",        MNU_MENU &mSambaReglages },
#endif
	{ "Affichages",      MNU_MENU &mMonit },
	{ "Analyse",         MNU_MENU &mCalc },
	{ "Gestion",         MNU_MENU &mGest },
	{ "Parametres",      MNU_MENU &mSettings },
	{ "Lecture",         MNU_MENU &mLect },
//	{ "Origine des temps",  MNU_FONCTION SambaDecaleT0 },
	{ "Hardware",        MNU_MENU &mHardware },
	{ "Quitter",         MNU_RETOUR },
	MNU_END
};

//#define FENETRE_SPECIALE
#define FENETRE_DIRECTE
/* ========================================================================== */
void APropos() {
#ifdef FENETRE_SPECIALE
	Info pancarte;

	pancarte = InfoCreate(3,60);
	InfoWrite(pancarte,1,"                 S  A  M B  A");
	InfoWrite(pancarte,2,"(Systeme d'Acquisition Multi-Bolometres sur Apple)");
	InfoWrite(pancarte,3," Tu utilises actuellement la version %s",SambaVersion);
	InfoTitle(pancarte,"A propos de SAMBA");
	OpiumSub(pancarte->cdr);
#else
#ifdef FENETRE_DIRECTE
	WndFrame f;
	int nbligs,nbcols,larg,haut,lig,i;

	nbligs = 2;
	nbcols = 45;
	larg = WndColToPix(nbcols); haut = WndLigToPix(nbligs);
/* 1/ creer une fenetre sans barre
   2/ obligation de cliquer dessus pour continuer (en l'effacant) */
	f = WndDisplay(1,(WndCurSvr->larg - larg)/2,(WndCurSvr->haut - haut)/2,larg,haut);
	WndTitle(f,"A propos de SAMBA");
	lig = 0;
	WndWrite(f,WND_CURSOR,lig,0,"S");
	WndWrite(f,WND_ASCR,lig,1,"ysteme d'");
	WndWrite(f,WND_CURSOR,lig,10,"A");
	WndWrite(f,WND_ASCR,lig,11,"cquisition ");
	WndWrite(f,WND_CURSOR,lig,22,"M");
	WndWrite(f,WND_ASCR,lig,23,"ulti-");
	WndWrite(f,WND_CURSOR,lig,28,"B");
	WndWrite(f,WND_ASCR,lig,29,"olometre sur ");
	WndWrite(f,WND_CURSOR,lig,42,"A");
	WndWrite(f,WND_ASCR,lig,43,"pple");
	lig++;
	i = nbcols + 2; while(i--) WndWrite(f,WND_ASCR,lig,i," "); /* +2    pour la marge ascenseur */
	WndWrite(f,WND_SELECTED,lig,18,"Version ");
	WndWrite(f,WND_SELECTED,lig,26,SambaVersion);
	lig++;                                                     /* lig++ pour la marge ascenseur */
	i = nbcols + 2; while(i--) WndWrite(f,WND_ASCR,lig,i," "); /* +2    pour la marge ascenseur */
	until(OpiumGetChar()) TimerMilliSleep(500);
	WndClear(f);

#else
	OpiumError("Systeme d'Acquisition Multi-Bolometres sur Apple, version %s",SambaVersion);
#endif
#endif
}
/* ========================================================================== */
int SambaAfficheVersion(Menu menu, MenuItem *item) {
#ifdef GADGET
	OpiumDisplay(SambaPresentation->cdr);
	OpiumUserAction();
#endif
	APropos();
	return(0);
}
/* ======================================================================= */
int SambaNote(char *fmt, ...) {
	char texte[1024]; va_list argptr;
	FILE *f; int l;

	va_start(argptr, fmt);
	vsprintf(texte, fmt, argptr);
	va_end(argptr);

	l = 0;
	f = fopen(SambaJournal,"a");
	if(f) { l = fprintf(f,"%s/ %s",DateHeure(),texte); fclose(f); }
	return(l);
}
/* ======================================================================= */
int SambaNoteCont(char *fmt, ...) {
	char texte[1024]; va_list argptr;
	FILE *f; int l;

	va_start(argptr, fmt);
	vsprintf(texte, fmt, argptr);
	va_end(argptr);

	l = 0;
	f = fopen(SambaJournal,"a");
	if(f) { l = fprintf(f,"%s",texte); fclose(f); }
	return(l);
}
/* ======================================================================= */
int SambaNoteSup(char *fmt, ...) {
	char texte[1024]; va_list argptr;
	FILE *f; int l;

	va_start(argptr, fmt);
	vsprintf(texte, fmt, argptr);
	va_end(argptr);

	l = 0;
	f = fopen(SambaJournal,"a");
	if(f) { l = fprintf(f,"          %s",texte); fclose(f); }
	return(l);
}
/* ========================================================================== */
void SambaRunDate() {
	time_t temps;
	struct tm date;

	time(&temps);
	memcpy(&date,localtime(&temps),sizeof(struct tm));
	ExecAn = date.tm_year - ANNEE_2000;
	ExecMois = date.tm_mon;
	ExecJour = date.tm_mday;
	sprintf(DateCourante,"%c%c%02d",'a' + ExecAn,'a' + ExecMois,ExecJour);
	ExecHeure = date.tm_hour;
	ExecMin = date.tm_min;
	ExecSec = date.tm_sec;
}
/* ========================================================================== */
void SambaRunNomNouveau() {
	char dateprec[8]; int fmt; int run_type;
	FILE *f; char ligne[256],*r;

	strcpy(RunDateBanc,DateCourante); RunTypeLu[0] = '\0';
	f = fopen(FichierRunPrecedent,"r");
	if(f) {
		while(LigneSuivante(ligne,256,f)) {
			if((ligne[0] == '#') || (ligne[0] == '\n')) continue;
			r = ligne;
			// sscanf(ligne,"%s %d %s %d %s %d",dateprec,&NumeroLect,RunDateBanc,&NumeroBanc,RunTypeLu,&NumeroManip);
			strcpy(dateprec,ItemSuivant(&r,' ')); sscanf(ItemSuivant(&r,' '),"%d",&NumeroLect);
			strcpy(RunDateBanc,ItemSuivant(&r,' ')); sscanf(ItemSuivant(&r,' '),"%d",&NumeroBanc);
			strcpy(RunTypeLu,ItemSuivant(&r,' ')); sscanf(ItemSuivant(&r,' '),"%d",&NumeroManip);
			break;
		}
		fclose(f);
		printf("* Lu dans %s: %s %d %s %d \"%s\" %d\n",FichierRunPrecedent,
			dateprec,NumeroLect,RunDateBanc,NumeroBanc,RunTypeLu,NumeroManip);
		run_type = ArgKeyGetIndex(RunCategCles,RunTypeLu);
	} else {
		dateprec[0] = '\0';
		printf("* Fichier %s illisible (%s)\n",FichierRunPrecedent,strerror(errno));
		run_type = LectCntl.RunCategNum;
	}
	if(!strcmp(dateprec,DateCourante)) NumeroLect++; else NumeroLect = 0;

	if(SettingsRunFamille == RUN_FAMILLE_BANC) {
		if(LectCntl.LectRunNouveau) {
			strcpy(RunDateBanc,DateCourante);
			NumeroBanc = NumeroLect;
		}
		sprintf(Acquis[AcquisLocale].etat.nom_run,"%s%c%03d",RunDateBanc,Acquis[AcquisLocale].runs[0],NumeroBanc);
	} else {  /* SettingsRunFamille == RUN_FAMILLE_MANIP */
		if(LectCntl.RunCategNum != run_type) LectCntl.LectRunNouveau = 1;
		if(LectCntl.LectRunNouveau) {
			LectCntl.RunCategNum = run_type;
			NumeroManip++;
		}
		ArgKeyGetText(RunCategCles,LectCntl.RunCategNum,RunCategName,TYPERUN_NOM);
		snprintf(Acquis[AcquisLocale].etat.nom_run,RUN_NOM,"%s%c%03d",RunCategName,Acquis[AcquisLocale].runs[0],NumeroManip);
	}
	ArchiveSurRepert = (LectDureeActive || (LectAutoriseSequences && (SequenceNb > 1)));
	for(fmt=0; fmt<ARCH_TYPEDATA; fmt++) sprintf(ArchiveChemin[fmt],"%s%s",ArchZone[fmt],Acquis[AcquisLocale].etat.nom_run);
	snprintf(ArchiveId,DB_LNGRTEXTE,"run_%s",Acquis[AcquisLocale].etat.nom_run);
	DbRazLastId();
//	printf("* Chemin d'archivage: %s\n",ArchiveChemin[EVTS]);
}
/* ========================================================================== */
void SambaRunNomEnregistre() {
	FILE *f;

	RepertoireCreeRacine(FichierRunPrecedent);
	f = fopen(FichierRunPrecedent,"w");
	if(f) {
		ArgKeyGetText(RunCategCles,LectCntl.RunCategNum,RunCategName,TYPERUN_NOM);
		fprintf(f,"%s %d %s %d \"%s\" %d",
			DateCourante,NumeroLect,RunDateBanc,NumeroBanc,RunCategName,NumeroManip);
		printf("* Ecrit dans %s: %s %d\n",FichierRunPrecedent,DateCourante,NumeroLect);
		fclose(f);
	}
}
#ifdef BRANCHE_TESTS
/* ========================================================================== */
int SambaCalculeTemplates() {
	int vm, dim, l;

	for(vm=0; vm<VOIES_TYPES; vm++) {
		dim = TemplMontee[vm]; /* nombre de points de montee */
		for(l=0; l<dim; l++) Template[vm][l] = 0.5 * (1.0 - (float)cos(PI * (double)l / (double)dim));
		dim = TemplDescente[vm]; /* nombre de points pour decroitre de 1/2,718 */
		while(l < MAXTEMPLATE) {
			Template[vm][l] = Template[vm][l - 1] * (float)exp(-1.0 / (double)dim);
			if(Template[vm][l] == 0) break;
			l++;
		}
		TemplDim[vm] = l;
	}
	return(0);
}
#endif
/* ========================================================================== */
int SambaPeriodiqueDistant() {
#ifdef MODULES_RESEAU
	int64 maintenant;
	
	if((maintenant = DateMicroSecs()) >= SambaProchainAffichage) {
		if(OpiumDisplayed(bLecture)) LectDisplay();
		SambaProchainAffichage = maintenant + ((int64)MonitIntervAff * 1000);
//			printf("Affichage a %d,%06d s, prochain a %d,%06d s\n",
//				(int)(maintenant/1000000),Modulo(maintenant,1000000),
//				(int)(SambaProchainAffichage/1000000),Modulo(SambaProchainAffichage,1000000));
	}
#endif
	return(1);
}
/* ========================================================================== */
int SambaPeriodiqueAcquis() {
#ifdef MODULES_RESEAU
	int64 maintenant;
	int sat;
	char msg[256];

	if(SambaMode == SAMBA_SATELLITE) {
		if((maintenant = DateMicroSecs()) >= SambaProchainAffichage) {
			if(Acquis[AcquisLocale].etat.status < SATELLITE_ATTENTE) Acquis[AcquisLocale].etat.status = SATELLITE_ATTENTE;
			//-	CarlaDataUpdate(IdLocal);
			SambaProchainAffichage = maintenant + ((int64)MonitIntervAff * 1000);
		}
	} else  if(SambaMode == SAMBA_MAITRE) {
		if((maintenant = DateMicroSecs()) >= SambaProchainAffichage) {
			if(pAcquisEtat) {
				if(OpiumDisplayed(pAcquisEtat->cdr)) {
					for(sat=0; sat<AcquisNb; sat++) if((sat != AcquisNb) && (Acquis[sat].etat.status > SATELLITE_ABSENT)) {
						OpiumRefresh(pAcquisEtat->cdr);
					#ifdef ETAT_RESEAU
						CarlaDataRequest(Acquis[sat].adrs,Acquis[sat].id);
						Acquis[sat].etat.status = SATELLITE_ABSENT;
						CarlaDataUpdate(Acquis[sat].id);
					#endif
					}
				}
			}
			SambaProchainAffichage = maintenant + ((int64)5000000);
		}
	}
#ifdef MSGS_RESEAU
	if(CarlaMsgGetCmde(msg,CARLA_TEXTMAX)) CarlaMsgExec(SambaCommandes,msg);
#endif
#endif
	return(1);
}
/* ========================================================================== */
int SambaPeriodiqueServeur() {
	if(SambaInfos > 0) {
		if(SambaInfos->lance_run) {
			SambaInfos->lance_run = 0;
			Trigger.demande = SambaInfos->trigger_demande;
			Archive.demandee = SambaInfos->archive_demandee;
			if(OpiumAlEcran(pLectMode)) PanelRefreshVars(pLectMode);
			LectChangeParms = 0;
			LectDemarre();
		}
	}
	return(1);
}
/* ========================================================================== */
int extensions_menu(Cadre cdr) {
	OpiumKeyEnable(cdr,26,HelpMenu);   /* Ctrl-Z */
	return(0);
}
/* ========================================================================== */
int extensions_ps(Cadre cdr) {
#ifdef macintosh
	OpiumKeyEnable(cdr,OPIUM_ALT | 0x70,OpiumPSprint);  /* Pomme-P */
#else
	OpiumKeyEnable(cdr,16,OpiumPSprint);  /* Ctrl-P */
#endif
	return(0);
}
/* ========================================================================== */
int extensions_edit(Cadre cdr) {
#ifdef macintosh
	OpiumKeyEnable(cdr,OPIUM_ALT | 0x70,OpiumPSprint);      /* Pomme-P */
//	OpiumKeyEnable(cdr,OPIUM_ALT | 0x63,OpiumEditCopier);   /* Pomme-C */
//	OpiumKeyEnable(cdr,OPIUM_ALT | 0x76,OpiumEditColler);   /* Pomme-V */
//	OpiumKeyEnable(cdr,OPIUM_ALT | 0x78,OpiumEditEffacer);  /* Pomme-X */
#else
	OpiumKeyEnable(cdr,16,OpiumPSprint);     /* Ctrl-P */
//	OpiumKeyEnable(cdr, 3,OpiumEditCopier);  /* Ctrl-C */
//	OpiumKeyEnable(cdr,22,OpiumEditColler);  /* Ctrl-V */
//	OpiumKeyEnable(cdr,24,OpiumEditEffacer); /* Ctrl-X */
#endif
	return(0);
}
#pragma mark ---- Dictionnaires ----
/* ========================================================================== */
void SambaDicoImprime(Dico dico) {
	int i;
	
	printf("  . Dictionnaire de %s:\n",dico->nom);
	for(i=0; i<dico->nbtermes; i++) {
		printf("    | '%s' est accepte pour '%s'\n",dico->terme[i].accepte,dico->terme[i].officiel);
	}
	printf("    | %d termes\n",dico->nbtermes);
}
/* ========================================================================== */
char SambaDicoStocke() {
	int d,i;
	DicoLexique terme;
	
	for(d=0; d<SAMBA_DICO_NB; d++) if(!strcmp(SambaDicoTempo.nom,SambaDicoNom[d])) break;
	if(d >= SAMBA_DICO_NB) {
		printf("! Ce dictionnaire n'est pas utile dans SAMBA:\n");
		printf("  = %s\n",SambaDicoTempo.nom);
		for(i=0; i<SambaDicoTempo.nbtermes; i++) {
			printf("    \"%s\": \"%s\"\n",SambaDicoTempo.terme[i].accepte,SambaDicoTempo.terme[i].officiel);
		}
		printf("    (%d termes)\n",SambaDicoTempo.nbtermes);
		SambaDicoTempo.nbtermes = 0;
		return(0);
	}
	terme = DicoInit(SambaDicoDefs+d,SambaDicoTempo.nom,SambaDicoTempo.nbtermes);
	for(i=0; i<SambaDicoTempo.nbtermes; i++) {
		DicoAjouteTerme(terme++,SambaDicoTempo.terme[i].accepte,SambaDicoTempo.terme[i].officiel);
	}
	SambaDicoImprime(SambaDicoDefs + d);
	// printf("* Dictionnaires sauvegardes:\n");
	// SambaDicoEcrit(SambaDicoDefs	+ SAMBA_DICO_VOIES,"*");
	// SambaDicoEcrit(SambaDicoDefs	+ SAMBA_DICO_REGLAGES,"*");
	SambaDicoTempo.nbtermes = 0;
	return(1);
}
/* ========================================================================== */
int SambaDicoLitV1() {
	int nb_erreurs; int d; FILE *f; char ligne[256],*r,*item;
	
	for(d=0; d<SAMBA_DICO_NB; d++) DicoInit(SambaDicoDefs+d,SambaDicoNom[d],0);
	nb_erreurs = 0;
	SambaDicoTempo.nbtermes = 0;
	if((f = fopen(FichierPrefDico,"r"))) {
		printf("\n= Lecture de dictionnaires             dans '%s'\n",FichierPrefDico);
		while(LigneSuivante(ligne,256,f)) {
			if((ligne[0] == '#') || (ligne[0] == '\n')) continue;
			r = ligne;
			if(*r == '=') {
				if(SambaDicoTempo.nbtermes) { if(!SambaDicoStocke()) nb_erreurs++; }
				r++;
				item = ItemSuivant(&r,' ');
				strncpy(SambaDicoTempo.nom,item,MAXDICONOM);
			} else {
				if(SambaDicoTempo.nbtermes >= MAXSAMBALEXIQUE) {
					printf("* Deja %d termes lus! Termes suivants abandonnes\n",SambaDicoTempo.nbtermes);
					++nb_erreurs;
					continue;
				}
				item = ItemSuivant(&r,':');
				strncpy(SambaDicoTempo.terme[SambaDicoTempo.nbtermes].accepte,item,MAXSAMBATERME);
				item = ItemSuivant(&r,' ');
				strncpy(SambaDicoTempo.terme[SambaDicoTempo.nbtermes].officiel,item,MAXSAMBATERME);
				SambaDicoTempo.nbtermes += 1;
			}
		}
		fclose(f);
		if(SambaDicoTempo.nbtermes) { if(!SambaDicoStocke()) nb_erreurs++; }
	} else { /* Dictionnaire inaccessible ou, plutot, inexistant */
		printf("! Dictionnaire illisible: %s (%s)\n",FichierPrefDico,strerror(errno));
		return(-1);
	}
	if(nb_erreurs) printf("  ! %d erreur%s detectee%s au total dans la description des dictionnaires\n",nb_erreurs,(nb_erreurs>1)?"s":"",(nb_erreurs>1)?"s":"");
	return(nb_erreurs);
}
/* ========================================================================== */
void SambaDicoEcrit(Dico dico, char *fichier) {
	int i;
	
	//	DicoCopie(&SambaDicoTempo,dico);
	strncpy(SambaDicoTempo.nom,dico->nom,MAXDICONOM);
	SambaDicoTempo.nbtermes = dico->nbtermes;
	if(SambaDicoTempo.nbtermes > MAXSAMBALEXIQUE) SambaDicoTempo.nbtermes = MAXSAMBALEXIQUE;
	for(i=0; i<SambaDicoTempo.nbtermes; i++) {
		strncpy(SambaDicoTempo.terme[i].accepte,dico->terme[i].accepte,MAXSAMBATERME);
		strncpy(SambaDicoTempo.terme[i].officiel,dico->terme[i].officiel,MAXSAMBATERME);
	}
	ArgPrint(fichier,SambaDicoDesc);
//	DicoVide(&SambaDicoTempo);
}
/* ========================================================================== */
int SambaDicoLit() {
	int d; FILE *f; char ligne[256],*r,sep,*accepte,*officiel;

	for(d=0; d<SAMBA_DICO_NB; d++) DicoInit(SambaDicoDefs+d,SambaDicoNom[d],0);
	d = 0;
	if((f = fopen(FichierPrefDico,"r"))) {
		printf("\n");
		while(LigneSuivante(ligne,256,f)) {
			if((ligne[0] == '#') || (ligne[0] == '\n')) continue;
			r = ligne;
			if(*r == '=') {
				r++;
				accepte = ItemSuivant(&r,' ');
				for(d=0; d<SAMBA_DICO_NB; d++) if(!strcmp(accepte,SambaDicoNom[d])) break;
				if(d >= SAMBA_DICO_NB) printf("  ! Un dictionnaire de %s n'est pas utile dans SAMBA\n",accepte);
			} else if(d < SAMBA_DICO_NB) {
				accepte = ItemAvant(":=",&sep,&r);
				officiel = ItemAvant(";",&sep,&r);
				DicoInsere(SambaDicoDefs+d,accepte,officiel);
			}
		}
		fclose(f);
	} else { /* Dictionnaire inaccessible ou, plutot, inexistant */
		// printf("! Dictionnaire illisible: %s (%s)\n",FichierPrefDico,strerror(errno));
		return(-1);
	}
	if(CompteRendu.initialisation) for(d=0; d<SAMBA_DICO_NB; d++) if(SambaDicoDefs[d].nbtermes) DicoImprime(SambaDicoDefs+d,"  . ");

	return(0);
}
#undef printf
#pragma mark --- Localisation ---
/* ========================================================================== */
void SambaLangueUtilise(char nouvelle) {
	/* Prise en compte de la langue */
	// Si pas de dico, (re)creer les tables quand meme
	if(!OpiumDicoUtilise(Dictionnaire,LangueEtendDico)) {
		if(Dictionnaire[0]) printf("    ! Dictionnaire %s illisible (%s)\n",Dictionnaire,strerror(errno));
	}
	if(nouvelle) {
		TrmtTypeNoms = LL_(TrmtTypeListe);
		TrmtClasseTrad = LL_(TrmtClasseTitre); // utilise
		TrmtCalculNoms = LL_(TrmtCalculListe);
		TrmtLibelleNoms = LL_(TrmtLibelleListe);
		TrmtTemplateNoms = LL_(TrmtTemplateListe);
		TrgrTypeNoms = LL_(TrgrTypeListe); // utilise
		TrgrDefNoms = LL_(TrgrDefListe);
		TrgrOrigineNoms = LL_(TrgrOrigineListe); // utilise
		TrgrDateNoms = LL_(TrgrDateListe);
		SambaSelectionText = LL_(SambaSelectionReponses); // utilise
		/* etc... */
	} else {
		TrmtTypeNoms = LC_(TrmtTypeListe,TrmtTypeNoms);
		TrmtClasseTrad = LC_(TrmtClasseTitre,TrmtClasseTrad); // utilise
		TrmtCalculNoms = LC_(TrmtCalculListe,TrmtCalculNoms);
		TrmtLibelleNoms = LC_(TrmtLibelleListe,TrmtLibelleNoms);
		TrmtTemplateNoms = LC_(TrmtTemplateListe,TrmtTemplateNoms);
		TrgrTypeNoms = LC_(TrgrTypeListe,TrgrTypeNoms); // utilise
		TrgrDefNoms = LC_(TrgrDefListe,TrgrDefNoms);
		TrgrOrigineNoms = LC_(TrgrOrigineListe,TrgrOrigineNoms); // utilise
		TrgrDateNoms = LC_(TrgrDateListe,TrgrDateNoms);
		SambaSelectionText = LC_(SambaSelectionReponses,SambaSelectionText); // utilise
	}
	strcpy(TrmtRegulTrad,L_(TrmtRegulCles)); // utilise

}
/* ========================================================================== */
static void SambaLangueListe() {
#define MAXFICPROJ 32
	char *fichier[MAXFICPROJ]; int nb_ficproj; char nom_complet[MAXFILENAME];
	int i,j,k,l;

	RepertoireListeCree(1,SambaInfoDir,SambaLangueDispo,MAXLANGUES,&SambaLanguesNb);
	// printf("Trouve dans %s: %d repertoire%s\n",SambaInfoDir,Accord1s(SambaLanguesNb));
	for(i=0; i<SambaLanguesNb; i++) {
		// printf(". Repertoire %2d: '%s'\n",i+1,SambaLangueDispo[i]);
		sprintf(nom_complet,"%s%s",SambaInfoDir,SambaLangueDispo[i]);
		RepertoireListeCree(0,nom_complet,fichier,MAXFICPROJ,&nb_ficproj);
		// printf(". Trouve dans %s: %d fichier%s\n",nom_complet,Accord1s(nb_ficproj));
		// for(k=0; k<nb_ficproj; k++) printf("  . Fichier %2d: '%s'\n",k+1,fichier[k]);
		for(k=0; k<nb_ficproj; k++) if(!strcmp(fichier[k],SubDirTrad)) break;
		if(k >= nb_ficproj) {
			free(SambaLangueDispo[i]);
			SambaLanguesNb--; for(j=SambaLanguesNb; j>=i; j--) SambaLangueDispo[j] = SambaLangueDispo[j+1];
			--i;
		} else {
		#ifdef macintosh
			l = strlen(SambaLangueDispo[i]) - 6;
			if((l > 0) && !strcmp(SambaLangueDispo[i]+l,".lproj")) *(SambaLangueDispo[i]+l) = '\0';
		#endif
		}
	}

	if(SambaLanguesNb < (MAXLANGUES - 1)) {
		SambaLangueDispo[SambaLanguesNb + 1] = SambaLangueDispo[SambaLanguesNb]; // fin de liste
		SambaLangueDispo[SambaLanguesNb] = (char *)malloc(7);
		strcpy(SambaLangueDispo[SambaLanguesNb],"native");
		SambaLanguesNb++;
	}
	SambaLangueChoisie = SambaLanguesNb - 1;

}
/* ========================================================================== */
int SambaLangueDemande(Menu menu, MenuItem *item) {
	Panel p;
	
	SambaLangueListe();
	if(SambaLanguesNb > 1) {
		p = PanelCreate(2);
		PanelList(p,"Langue a utiliser",SambaLangueDispo,&SambaLangueChoisie,12);
		PanelKeyB(p,"Enrichir le dictionnaire","simplifier/non/oui",&LangueEtendDico,12);
		if(OpiumExec(p->cdr) == PNL_OK) {
			strcpy(LangueDemandee,SambaLangueDispo[SambaLangueChoisie]);
			SambaLangueDefini(SambaInfoSource,SambaInfoDir); SambaLangueUtilise(0);
		}
		PanelDelete(p);
	} else OpiumError("Il n'y a qu'une seule langue prevue (%s)",SambaLangueDispo[0]);
	return(0);
}
#pragma mark --- Tables et Impressions PDF ---
/* ========================================================================== */
int SambaMontreFolders(Menu menu, MenuItem *item) {
	char base[MAXFILENAME],commande[MAXFILENAME];

	if(OpiumExec(pFolderActn->cdr) == PNL_CANCEL) return(0);
	if(FolderActn == 1) {
		RepertoireExtrait(FolderRef[FolderNum].nom,base,0);
		strcat(strcpy(commande,"open "),base);
	} else strcat(strcpy(commande,"open "),FolderRef[FolderNum].nom);
	system(commande);
	return(0);
}
/* ========================================================================== */
void SambaImprimeGeneral() {
	int voie,l,virg;
#ifdef MODULES_RESEAU
	int sat;
#endif
	
	printf(" ____________________________________________________________________________________\n");
	printf("| Configuration | %-66s |\n",SambaIntitule);
	printf("|_______________|____________________________________________________________________|\n");
#ifdef MODULES_RESEAU
	for(sat=0; sat<AcquisNb; sat++) {
		if(sat == 0) l = printf("| Reseau        | "); else l = printf("|               | ");
		l += printf("%6s: %-32s (%s)",Acquis[sat].code,Acquis[sat].adrs,AcquisTexte[(int)Acquis[sat].etat.status]);
		SambaCompleteLigne(86,l);
	}
	printf("|_______________|____________________________________________________________________|\n");
#endif
	printf("\n");
	if(CompteRendu.config.elec) {
		printf(" ____________________________________________________________________________________\n");
		printf("|                           Definition de l'electronique                             |\n");
		printf("|____________________________________________________________________________________|\n");
		//	printf("| Identification | version %3.1f",VersionIdentification); SambaCompleteLigne(86,l);
		l = printf("| Echantillonnage | %g kHz (%g us)",Echantillonnage,1000.0/Echantillonnage); SambaCompleteLigne(86,l);
		l = printf("| Synchro D2      | %d echantillons (%g kHz)",Diviseur2,Echantillonnage/(float)Diviseur2); SambaCompleteLigne(86,l);
		l = printf("| Detecteurs      | %-3d gere%s",Accord1s(BoloGeres)); SambaCompleteLigne(86,l);
		if(PasMaitre) { l = printf("|                 | %-3d connu%s",Accord1s(BoloNb)); SambaCompleteLigne(86,l); }
		//	printf("|                | %-3d voie(s) transmise(s) par detecteur                            |\n",
		//		   ModeleVoieNb);
		l = printf("| Types de voies  |");
		virg = 0;
		for(voie = 0; voie<ModeleVoieNb; voie++) {
			if((l + 3 + strlen(ModeleVoie[voie].nom)) >= 85) {
				l += printf(","); SambaCompleteLigne(86,l);
				l = printf("|                 |"); virg = 0;
			}
			l += printf("%s%s",virg?", ":" ",ModeleVoie[voie].nom);
			virg = 1;
		}
		SambaCompleteLigne(86,l);
		l = printf("| Etat sur voie   | %-16s",VoieStatus); SambaCompleteLigne(86,l);
		l = printf("| Total voies     | %-3d geree%s",VoiesGerees,(VoiesGerees>1)?"s":" "); SambaCompleteLigne(86,l);
		if(PasMaitre) { l = printf("|                 | %-3d connue%s",VoiesNb,(VoiesNb>1)?"s":" "); SambaCompleteLigne(86,l); }
		l = printf("| FIFO repart.    | %-7d valeurs",FIFOdim); SambaCompleteLigne(86,l);
		printf("|________________|___________________________________________________________________|\n\n");
	}
	
}
/* ========================================================================== */
int SambaImprimeFiltres() {
	int i,j,l,etage;
	
    printf(" ____________________________________________________________________________________\n");
    printf("|                                  Liste des filtres                                 |\n");
    printf("|____________________________________________________________________________________|\n");
	if(!FiltreNb) {
		printf("|                                       neant                                        |\n");
		printf("|____________________________________________________________________________________|\n");
	} else for(i=0; i<FiltreNb; i++) {
		l = printf("| %-20s |",FiltreGeneral[i].nom);
		if(FiltreGeneral[i].commentaire[0]) {
			printf(" %-59s |\n",FiltreGeneral[i].commentaire);
			l = printf("|                      |");
		}
		if(FiltreGeneral[i].modele >= 0) {
			l += printf(" %-12s %-16s *%-2d ",
						FiltreModeles[(int)FiltreGeneral[i].modele],FiltreTypes[(int)FiltreGeneral[i].type],
						FiltreGeneral[i].degre);
			switch(FiltreGeneral[i].type) {
				case FILTRE_PBAS: case FILTRE_PHAUT:
					l += printf("@ %5.4f%-3s           ",FiltreGeneral[i].omega1,FiltreUnites[FiltreGeneral[i].unite1]);
					break;
				case FILTRE_PASSEB: case FILTRE_COUPEB:
					l += printf("@[%5.4f%-3s, %5.4f%-3s]",FiltreGeneral[i].omega1,FiltreUnites[FiltreGeneral[i].unite1],
						FiltreGeneral[i].omega2,FiltreUnites[FiltreGeneral[i].unite2]);
					break;
				default: l += printf("                     ");
			}
			/* printf("          |\n"); */ SambaCompleteLigne(86,l);
		} else {
			for(etage=0; etage<FiltreGeneral[i].coef.nbetg; etage++) {
				/* if(etage) l = printf("|                      |"); */
				l += printf(" %cdirects=",etage?'+':' ');
				for(j=0; j<FiltreGeneral[i].coef.etage[etage].nbdir; j++) {
					if(j && !(j % 3)) /* printf("\n"); */ {
						SambaCompleteLigne(86,l);
						l = printf("|                      |");
					}
					l += printf(" %14g",FiltreGeneral[i].coef.etage[etage].direct[j]);
				}
				/* printf("\n|                      | inverses="); */
				SambaCompleteLigne(86,l);
				l = printf("|                      | inverses=");
				for(j=0; j<FiltreGeneral[i].coef.etage[etage].nbinv; j++) {
					if(j && !(j % 3)) /* printf("\n"); */ {
						SambaCompleteLigne(86,l);
						l = printf("|                      |");
					}
					l += printf(" %14g",FiltreGeneral[i].coef.etage[etage].inverse[j]);
				}
				/* printf("\n"); */
				SambaCompleteLigne(86,l);
			}
		}
	}
	printf("|______________________|_____________________________________________________________|\n\n");
	return(0);
}
/* ========================================================================== */
int SambaDocImprimeCablage() {
	int bolo; char premier;
	int trait_nom,trait_pos,trait_cap,trait_rgl,trait_fis,trait_num,trait_res; // int trait_adr;
	int col_nom,col_pos,col_cap,col_rgl,col_fis,col_num,col_res; // int col_adr;
	int trait_dernier;
	
	ImprimeSautPage();

	trait_dernier = 0;
	ImpressionPlaceColonne(&trait_nom,16,&col_nom,&trait_dernier);
	ImpressionPlaceColonne(&trait_pos,4,&col_pos,&trait_dernier);
	ImpressionPlaceColonne(&trait_cap,8,&col_cap,&trait_dernier);
	ImpressionPlaceColonne(&trait_rgl,12,&col_rgl,&trait_dernier);
	ImpressionPlaceColonne(&trait_fis,3,&col_fis,&trait_dernier);
	ImpressionPlaceColonne(&trait_num,12,&col_num,&trait_dernier);
	// ImpressionPlaceColonne(&trait_adr,3,&col_adr,&trait_dernier);
	ImpressionPlaceColonne(&trait_res,16,&col_res,&trait_dernier);
	
	ImprimeLimite(0,trait_dernier,0);
	premier = 1;
	for(bolo=0; bolo<BoloNb; bolo++) /* if(Bolo[bolo].local) ou si "tous" */ {
		int bb,cap,vbb,vm,voie; int i,prec_num,prec_conn;
		char nomBN[NUMER_NOM];
		TypeDetecteur *detectr; TypeReglage *regl;
		TypeDetModele *det_modele; TypeBNModlRessource *ress;
		unsigned short connecteur;
		
		if(premier) {
			ImprimeColonnes(2,0,trait_dernier);
			ImprimeEnCol((trait_dernier - 22) / 2,"Cablage des detecteurs");
			ImprimeLimite(0,trait_dernier,1);
			ImprimeNouvelleLigne(1);
			ImprimeColonnes(3,0,trait_fis,trait_dernier);
			ImprimeEnCol((trait_fis-9)/2,"detecteur");
			ImprimeEnCol((trait_dernier-trait_fis-10)/2+col_fis,"numeriseur");
			ImprimeLimite(0,trait_dernier,1);
			ImprimeNouvelleLigne(1);
			// ImprimeColonnes(9,0,trait_pos,trait_cap,trait_rgl,trait_fis,trait_num,trait_adr,trait_res,trait_dernier);
			ImprimeColonnes(8,0,trait_pos,trait_cap,trait_rgl,trait_fis,trait_num,trait_res,trait_dernier);
			ImprimeEnCol(col_nom," nom");
			ImprimeEnCol(col_pos,"pos.");
			ImprimeEnCol(col_cap,"capteur");
			ImprimeEnCol(col_rgl,"reglage");
			ImprimeEnCol(col_fis,"pos.");
			ImprimeEnCol(col_num,"nom");
			// ImprimeEnCol(col_adr,"adrs");
			ImprimeEnCol(col_res," ressource");
			ImprimeLimite(0,trait_dernier,1);
		}
		
		prec_num = -2; prec_conn = -2;
		detectr = &(Bolo[bolo]);
		det_modele = &(ModeleDet[detectr->type]);
		for(cap=0; cap<detectr->nbcapt; cap++) {
			vm = detectr->captr[cap].modele;
			bb = detectr->captr[cap].bb.num;
			if((bb != prec_num) && (prec_num != -2)) {
				ImprimeLimite(trait_fis,trait_dernier-trait_fis,1);
			}
			ImprimeNouvelleLigne(1);
			if(bb >= 0) { connecteur = Numeriseur[bb].fischer; vbb = detectr->captr[cap].bb.adc - 1; }
			else if(detectr->pos != CABLAGE_INDEFINI) {
				connecteur = CablageSortant[detectr->pos].captr[cap].connecteur;
				vbb = CablageSortant[detectr->pos].captr[cap].vbb;
			} else { connecteur = 0; vbb = -1; prec_conn = -2; }
			// if((bb == prec_num) && (connecteur == prec_conn)) continue;
			if(cap == 0) {
				ImprimeEnCol(col_nom,"%c%-12s",detectr->lu?' ':'!',detectr->nom);
				ImprimeEnCol(col_pos,detectr->adresse);
			}
			if((voie = detectr->captr[cap].voie) >= 0) {
				if(VoieManip[voie].pseudo) {
					TypeComposantePseudo *composant; float coef;
					char def[256]; int k;
					ImprimeEnCol(col_cap,detectr->captr[cap].nom);
					ImprimeStyle(IMPRESSION_ITALIQUE); ImprimeEnCol(col_rgl,"(composee)"); ImprimeStyle(IMPRESSION_REGULIER);
					composant = VoieManip[voie].pseudo; k = 0;
					do {
						coef = composant->coef;
						if(composant != VoieManip[voie].pseudo) {
							if(coef >= 0.0) k += sprintf(def+k," + ");
							else { k += sprintf(def+k," - "); coef = -coef; }
						}
						k += sprintf(def+k,"%g x %s",coef,VoieManip[composant->srce].nom);
						composant = composant->svt;
					} while(composant);
					def[k] = '\0';
					ImprimeEnCol(col_fis,def);
					continue;
				}
			}
			ImprimeEnCol(col_cap,ModeleVoie[vm].nom);
			ImprimeStyle(IMPRESSION_ITALIQUE); ImprimeEnCol(col_rgl,"(mesure)"); ImprimeStyle(IMPRESSION_REGULIER);
			if(connecteur != prec_conn) {
				if(connecteur) ImprimeEnCol(col_fis,CablageEncodeConnecteur(connecteur)); else ImprimeEnCol(col_fis," --");
				prec_conn = connecteur;
			}
			if(bb != prec_num) {
				if(bb >= 0) strcpy(nomBN,Numeriseur[bb].nom); else strcpy(nomBN,"inconnu");
				ImprimeEnCol(col_num,nomBN);
				prec_num = bb;
				// if(detectr->captr[cap].bb.adrs) ImprimeEnCol(col_adr,"%02X",detectr->captr[cap].bb.adrs); else ImprimeEnCol(col_adr," --");
			}
			if(vbb >= 0) { ImprimeStyle(IMPRESSION_ITALIQUE);  ImprimeEnCol(col_res," ADC%d",vbb+1); ImprimeStyle(IMPRESSION_REGULIER); }
			forever {
				int n;
				for(i=0; i<detectr->nbreglages; i++) {
					regl = &(detectr->reglage[i]);
					if(regl->capt != cap) continue;
					ImprimeNouvelleLigne(1);
					if(cap < 0) ImprimeEnCol(col_cap,"(general)");
					ImprimeEnCol(col_rgl,det_modele->regl[i].nom);
					bb = regl->bb;
					if(bb != prec_num) {
						if(bb >= 0) strcpy(nomBN,Numeriseur[bb].nom); else strcpy(nomBN,"inconnu");
						ImprimeEnCol(col_num,nomBN);
						prec_num = bb;
					}
					n = 0;
					if(regl->ress >= 0) {
						ress = &(ModeleBN[Numeriseur[bb].def.type].ress[regl->ress]);
						ImprimeEnCol(col_res," %-13s",ress->nom); n++;
					}
					if(regl->ressneg >= 0) {
						ress = &(ModeleBN[Numeriseur[bb].def.type].ress[regl->ressneg]);
						if(n) ImprimeNouvelleLigne(1);
						ImprimeEnCol(col_res,"-%-13s",ress->nom); n++;
					}
					if(regl->resspos >= 0) {
						ress = &(ModeleBN[Numeriseur[bb].def.type].ress[regl->resspos]);
						if(n) ImprimeNouvelleLigne(1);
						ImprimeEnCol(col_res,"+%-13s",ress->nom); n++;
					}
					if(!n) ImprimeEnCol(col_res," ...");
				}
				if(cap == (detectr->nbcapt - 1)) cap = -1;
				else { if(cap == -1) cap = detectr->nbcapt; break; }
			}
		}
		ImprimeLimite(0,trait_dernier,1);
		premier = 0;
	}
	if(premier) {
		ImprimeColonnes(2,0,trait_dernier);
		ImprimeEnCol(3,"! PAS DE DETECTEUR LU PAR CET ORDINATEUR");
		ImprimeFond(5,38,0xFFFF,0x0000,0x0000);
	}
	ImprimeColonnes(0);
	ImprimeNouvelleLigne(1);
	return(0);
}
/* ========================================================================== */
int SambaDocImprimeModeles() {
	int i,cap,k; char autre;
	
	ImprimeSautPage();
	ImprimeTableauLargeurs(0,4,20,20,20,15);
	ImprimeTableauChapeau("Modeles de detecteurs");
	ImprimeTableauCol(0,"nom");
	ImprimeTableauCol(1,"capteur");
	ImprimeTableauCol(2,"reglage");
	ImprimeTableauCol(3,"type");
	ImprimeTableauLimite(0,3);
	for(i=0; i<ModeleDetNb; i++) {
		if(i) ImprimeTableauLimite(0,3); // ImprimeTableauTrait(0,3,1);
		ImprimeTableauCol(0,ModeleDet[i].nom);
		for(cap=0; cap<ModeleDet[i].nbcapt; cap++) {
			if(cap) { ImprimeNouvelleLigne(1); ImprimeTableauTrait(1,3,1); }
			ImprimeTableauCol(1,ModeleVoie[ModeleDet[i].type[cap]].nom);
			autre = 0;
			for(k=0; k<ModeleDet[i].nbregl; k++) if(ModeleDet[i].regl[k].capteur == ModeleDet[i].type[cap]) {
				if(autre) ImprimeNouvelleLigne(1);
				ImprimeTableauCol(2,ModeleDet[i].regl[k].nom);
				ImprimeTableauCol(3,ReglCmdeListe[ModeleDet[i].regl[k].cmde]);
				autre = 1;
			}
			if(!autre) ImprimeTableauCol(2,"--");
		}
	}
	ImprimeTableauFin();

	ImprimeSautPage();
	ImprimeTableauLargeurs(0,6,12,17,14,40,7,7);
	ImprimeTableauChapeau("Modeles de numeriseurs");
	ImprimeTableauCol(0,"nom");
	ImprimeTableauCol(1,"ressource");
	ImprimeTableauCol(2,"categorie");
	ImprimeTableauCol(3,"valeurs"); /* RESS_TYPETEXTE */
	ImprimeTableauCol(4,"adrs"); /* adrs:bit0 */
	ImprimeTableauCol(5,"masque");
	ImprimeTableauLimite(0,5);
	for(i=0; i<ModeleBNNb; i++) {
		TypeBNModlRessource *ress; char textetype[8],texteadrs[4]; char vu;
		char textevaleur[40];
		if(i) ImprimeTableauLimite(0,5); // ImprimeTableauTrait(0,5,1);
		ImprimeTableauCol(0,ModeleBN[i].nom);
		vu = 0;
		for(k=0; k<ModeleBN[i].nbress; k++) {
			if(k) ImprimeNouvelleLigne(1);
			ress = &(ModeleBN[i].ress[k]);
			ImprimeTableauCol(1,ress->nom);
			ImprimeTableauCol(2,RessCategListe[ress->categ]);
			ArgKeyGetText(RESS_TYPETEXTE,ress->type,textetype,8);
			switch(ress->type) {
				case RESS_INT: 
					snprintf(textevaleur,40,"%d .. %d",ress->lim_i.min,ress->lim_i.max);
					break;
				case RESS_FLOAT: 
					snprintf(textevaleur,40,"%g .. %g",ress->lim_r.min,ress->lim_r.max);
					break;
				case RESS_CLES: 
					strncpy(textevaleur,ress->lim_k.cles,40);
					break;
				case RESS_PUIS2:
					if(ress->lim_i.max == 1) {
						if(ress->lim_i.min) snprintf(textevaleur,40,"2^(x-%d)",ress->lim_i.min);
						else strcpy(textevaleur,"2^x");
					} else {
						if(ress->lim_i.min) snprintf(textevaleur,40,"%d * 2^(x-%d)",ress->lim_i.max,ress->lim_i.min);
						else snprintf(textevaleur,40,"%d * 2^x",ress->lim_i.max);
					}
					break;
			}
			if(ress->unite[0]) ImprimeTableauCol(3,"%s: %s %s",textetype,textevaleur,ress->unite);
			else ImprimeTableauCol(3,"%s: %s",textetype,textevaleur);
			if(ress->ssadrs == 0xFF) strcpy(texteadrs,"..");
			else sprintf(texteadrs,"%02x",ress->ssadrs);
			if(ress->bit0 < 0) ImprimeTableauCol(4,texteadrs);
			else ImprimeTableauCol(4,"%2s:%02d",texteadrs,ress->bit0);
			ImprimeTableauCol(5,"%4X",ress->masque);
			vu = 1;
		}
		if(!vu) ImprimeTableauCol(1,"--");
	}
	ImprimeTableauFin();

	return(0);
}
/* ========================================================================== */
int SambaDocImprimeAutom() {
	int i; char texte[80];
	
	ImprimeSautPage();
	ImprimeTableauLargeurs(0,4,20,15,14,14);
	ImprimeTableauChapeau("Variables de l'automate");
	ImprimeTableauCol(0,"nom");
	ImprimeTableauCol(1,"type");
	ImprimeTableauCol(2,"etat");
	ImprimeTableauCol(3,"valeur");
	ImprimeTableauLimite(0,3);
	for(i=0; i<AutomVarNb; i++) {
		ImprimeTableauCol(0,AutomVar[i].nom);
		ImprimeTableauCol(1,"%-13s",AutomTypeTexte(AutomVar[i].type));
		ImprimeTableauCol(2,"%-12s",AutomEtatTexte(AutomVar[i].etat));
		switch(AutomVar[i].type) {
		  case AUTOM_BOOL:  ImprimeTableauCol(3,"%12s",AutomVar[i].val.b? "oui": "non"); break;
		  case AUTOM_CLE:   ArgKeyGetText(AutomVar[i].cles,AutomVar[i].val.b,texte,80);
		                    ImprimeTableauCol(3,"%12s",texte); break;
		  case AUTOM_INT:   ImprimeTableauCol(3,"%12d",AutomVar[i].val.i); break;
		  case AUTOM_FLOAT: ImprimeTableauCol(3,"%12g",AutomVar[i].val.r); break;
		}
		ImprimeNouvelleLigne(1);
	}
	ImprimeTableauFin();
	return(0);
}
/* ========================================================================== */
int SambaDocImprimeScripts() {
	int trait_dernier,i; char *ligne;

	ImprimeSautPage();
	trait_dernier = 102;
	ImprimeLimite(0,trait_dernier,0);
	ImprimeColonnes(2,0,trait_dernier);
	i = 0; ligne = ScriptSyntaxe[i];
	ImprimeNouvelleLigne(1); ImprimeEnCol((trait_dernier-19)/2,ligne); ImprimeLimite(0,trait_dernier,1); ImprimeNouvelleLigne(1);
	ligne = ScriptSyntaxe[++i];
	while(*ligne) { ImprimeNouvelleLigne(1); ImprimeEnCol(1,"%s",ligne); ligne = ScriptSyntaxe[++i]; }
	ImprimeLimite(0,trait_dernier,1); ImprimeColonnes(0); ImprimeNouvelleLigne(1);

	return(0);
}
/* ========================================================================== */
int SambaDocImprime() {
	char fichier[MAXFILENAME]; int k,l;
	int hauteur_page; int trait_premier,trait_dernier,lngr;
	
	hauteur_page = 54;
	if(OpiumExec(pSambaImprimeForme->cdr) == PNL_CANCEL) return(0);

	if((SambaDocSupport == IMPRESSION_TERMINAL) && (SambaDocFormat == IMPRESSION_TEXTE)) {
		SambaDocSupport = IMPRESSION_FICHIER;
		strcpy(fichier,"*");
	} else {
		if(Acquis[AcquisLocale].etat.active && Archive.enservice) {
			if(ArchiveSurRepert) sprintf(fichier,"%s%s%s_info",ArchiveChemin[EVTS],FILESEP,Acquis[AcquisLocale].etat.nom_run);
			else strcat(strcpy(fichier,ArchiveChemin[EVTS]),"_info");
		} else strcat(strcat(strcat(strcpy(fichier,PrefPath),"Info"),"_"),Acquis[AcquisLocale].code); // ResuPath?
		if(SambaDocSupport == IMPRESSION_PAPIER) {
			char nom_imprimante[32];
			if(OpiumExec(pSambaImprimeSvr->cdr) == PNL_CANCEL) return(0);
			ArgKeyGetText(ListePrinters,NumPrinter,nom_imprimante,32);
			ImpressionServeur(nom_imprimante,fichier,NbTirages);
		}
		if(SambaDocFormat != IMPRESSION_TEXTE) strcat(fichier,".ps");
	}
	ImpressionFormatte(SambaDocFormat);
	ImpressionSurSupport(SambaDocSupport,fichier);
	if(!ImpressionPrete("landscape")) return(0);
	// if(hauteur_page < ImpressionHauteurCourante()) ImpressionHauteurPage(hauteur_page);

	printf("-------------------------- Documentation pour le materiel --------------------------\n");
	/* Entete generale */
	trait_dernier = 110; // ImpressionLargeurCourante();
	ImprimeEnCol(trait_dernier - 20,DateCivile()); ImprimeEnCol(trait_dernier - 11,DateHeure());

	ImprimeNouvelleLigne((ImpressionHauteurCourante() / 2) - 5);
	trait_premier = 20; lngr = trait_dernier - (2 * trait_premier);
	ImprimeLimite(trait_premier,lngr,0); ImprimeColonnes(2,trait_premier,trait_premier+lngr);
	ImprimeNouvelleLigne(2);
	
	l = strlen(SambaIntitule);
	k = (trait_dernier - l) / 2;
	ImprimeEnCol(k,SambaIntitule); ImprimeFond(k,l,0xFFFF,0xF000,0xD000);
	ImprimeNouvelleLigne(2);
	k = (trait_dernier - 22) / 2;
	ImprimeEnCol(k,"Documentation generale");
	ImprimeNouvelleLigne(2);
	ImprimeLimite(trait_premier,lngr,1); ImprimeColonnes(0);

	/* Documentations demandees */
	if(SambaDocCablage) SambaDocImprimeCablage();
	if(SambaDocModeles) SambaDocImprimeModeles();
	if(SambaDocAutom)   SambaDocImprimeAutom();
	if(SambaDocScripts) SambaDocImprimeScripts();

	ImpressionFin(1);
	printf("%s/ Documentation ecrite dans %s\n",DateHeure(),fichier);
	
	return(0);
}
#pragma mark ---- Demarrage ----
/* ========================================================================== */
static void SambaConfigHwSw() {
	struct utsname systeme;
#ifdef XCODE
	#define MESURE_VITESSE
#endif
#ifdef MESURE_VITESSE
	int nbticks,k; float dt;
#endif

	CompteRendu.config.hw = CompteRendu.config.sw = 1;
#ifdef XCODE
	printf("%s\n",SambaVersionTexte);
#endif
	/* Initialisation generale du logiciel */
	strcpy(SambaRelease,SambaVersion); /* adrs SambaVersion pas determinee a la compil de samba.c */
	strncpy(SambaMoiMeme,NomApplication(),80);
	/* sscanf(SambaVersion,"%f",&Version); */
	{
		char vsn[32]; char *r; float vsn_ppale,ss_vsn;
		strncpy(vsn,SambaVersion,32); vsn[31] = '\0';
		r = vsn;
		sscanf(ItemJusquA('.',&r),"%g",&vsn_ppale);
		sscanf(ItemJusquA('.',&r),"%g",&ss_vsn);
		Version = vsn_ppale + (ss_vsn / 1000.0);
		printf("%s version %s (version de reference: %.3f), compilee le %s\n",SambaMoiMeme,SambaVersion,Version,SambaCompile);
		printf("Version d'apres le compilateur: %s\n",SambaVersionCompilo);
	}
#ifdef PAROLE
	SpeakString("\pSAMBA is ready");
#endif
	/* { double v = 1352.0; printf("V%.3f -> 0x%08X\n",v,(hexa)v); } */

	uname(&systeme);
	BigEndianOrder = EndianIsBig();
	RepertoireInit(HomeDir);

	printf("Materiel:\n");
#ifdef _SYS_SYSCTL_H_
    printf("  Hote              : %s\n",System_texte("kern.hostname"));
#endif
	printf("  Systeme %-9s : %s\n",systeme.machine,systeme.nodename);
	/* le "nom externe" est en principe le vrai nom IP */
#ifdef SKTLIB
	CodeIpLocal = HostLocal(NomExterne,HOTE_NOM,AdresseIP);
#else  /* SKTLIB */
	CodeIpLocal = 0x0201a8c0;
	strcpy(NomExterne,"local");
	strcpy(AdresseIP,"192.168.1.2");
#endif /* SKTLIB */
    printf("  Reference IP      : %s (%s)\n",NomExterne,AdresseIP);
	if(CompteRendu.config.hw) {
		#ifdef _SYS_SYSCTL_H_
		/* utilise sysctlbyname */ {
			int ncpu,dim;
			/* a verifier soigneusement:
			 int lngr,mib[4]; mib[0] = CTL_HW; mib[1] = HW_BYTEORDER; lngr = sizeof(ordre); sysctl(mib,2,&ordre,&lngr,NULL,0); */
            printf("  Modele            : %s",System_texte("hw.model")); // System_texte("hw.targettype"): meme texte
                printf(" sous %s",System_texte("kern.ostype"));
                printf(" version %s\n",System_texte("kern.osrelease"));
            printf("   |                  [%s]\n",System_texte("kern.version"));
            printf("  CPU\n");
            printf("   | Denomination   : %s\n",System_texte("machdep.cpu.brand_string"));
			printf("   | Type           : %d.%d (%d bits, %s endian)\n",System_int32("hw.cputype"),System_int32("hw.cpusubtype"),
				   System_int32("hw.cpu64bit_capable")? 64: 32,(System_int32("hw.byteorder") > 2000)? "big": "little");
			printf("   | Frequence      : %.3f GHz, bus: %.3f GHz\n",(float)System_int64("hw.cpufrequency")/1e9,(float)System_int64("hw.busfrequency")/1e9);
			ncpu = System_int32("hw.activecpu"); dim = System_int32("hw.ncpu");
			printf("   | Disponibilite  : %d actif%s / %d existant%s\n",Accord1s(ncpu),Accord1s(dim));
			printf("  Memoire totale    : %.3f Gb\n",(float)System_int64("hw.memsize")/1024.0/1024.0/1024.0);
		}
		#endif /* _SYS_SYSCTL_H_ */
		printf("  Mode memoire      : %s endian\n",BigEndianOrder? "big": "little");
	#ifdef __ALTIVEC__
	#ifdef ALTIVEC
		printf("  AltiVec utilisable\n");
	#else
		printf("  AltiVec present mais masque\n");
	#endif
	#else
		printf("  AltiVec absent\n");
	#endif
	}
	SambaLinkPCI();

	CpuActifs = 1;
#ifdef _SC_NPROCESSORS_CONF
	CpuActifs = sysconf(_SC_NPROCESSORS_CONF);
#endif
#ifdef _SC_NPROCESSORS_ONLN
	CpuActifs = sysconf(_SC_NPROCESSORS_ONLN);
#endif
#ifdef _SYS_SYSCTL_H_
	FifoIP = System_int32("net.inet.udp.recvspace");
#endif
#ifdef MESURE_VITESSE
	{
		int c0,c1,nbsecs,nbus;
		struct timeval t0,t1;
		nbticks = 100;
		k = 0; c0 = TickCount(); c1 = c0 + nbticks; gettimeofday(&t0,0);
		while(TickCount() < c1) if(k++ > 1000000000) break;
		gettimeofday(&t1,0); nbsecs = t1.tv_sec - t0.tv_sec; nbus = t1.tv_usec - t0.tv_usec;
		dt = (float)((nbsecs * 1000000) + nbus);
		FrequenceTick = (int)((float)nbticks / (dt / 1000000.0));
	}
#else
	FrequenceTick = 100;
#endif
	if(CompteRendu.config.sw) {		
		printf("Configuration logicielle:\n");
		printf("  Systeme d'exploitation : %s, version %s.%d\n",systeme.sysname,systeme.release,atoi(systeme.version));
	#ifdef _SC_NPROCESSORS_CONF
		printf("  Processeurs configures : %ld\n",sysconf(_SC_NPROCESSORS_CONF));
	#endif
	#ifdef _SC_NPROCESSORS_ONLN
		printf("  Processeurs en ligne   : %ld\n",sysconf(_SC_NPROCESSORS_ONLN));
	#endif
		printf("  Temps processeur       : 1/%d s\n",CLOCKS_PER_SEC);
		printf("  Frequence ordonnanceur : %ld Hz\n",sysconf(_SC_CLK_TCK));
	#ifdef MESURE_VITESSE
		printf("  Duree d'un tick        : %6.3f ms [%d Hz]\n",dt / (float)nbticks / 1000.0,FrequenceTick);
		printf("  Duree d'une mesure     : %.1f ns\n",dt / (float)k * 1000.0);
	#endif
		printf("  Fichiers ouverts maxi  : %ld\n",sysconf(_SC_OPEN_MAX));
	#ifdef _SYS_SYSCTL_H_
		printf("  Pile IP totale         : %d octets\n",System_int32("kern.ipc.maxsockbuf"));
		printf("  Pile de reception UDP  : %d octets (%ld trames)\n",FifoIP,FifoIP/sizeof(TypeOperaTrame)); // 1518 maxi par trame
	#endif
		printf("  Version Posix.1        : %ld\n",sysconf(_SC_VERSION));
		printf("  Version Posix.2        : %ld\n",sysconf(_SC_2_VERSION));
		printf("  Session                : %s\n",getlogin());
		printf("  Variable $HOME         : %s\n",HomeDir);
		if(!RepertoireExiste(HomeDir)) printf("  ! Le repertoire %s n'existe pas\n",HomeDir);
		printf("\n");
	}
	CompteRendu.config.hw = CompteRendu.config.sw = 0;
}
/* ========================================================================== */
static void SambaLinkPCI() {
	int i; char log;
#ifdef AVEC_PCI
	char type_ok; int ica_nb;
	int taille,n;
#ifdef PCI_ID_DIRECT
	unsigned int id;
#endif
#endif

	log = CompteRendu.config.hw;
	/* Initialisation de l'interface PCI */
	if(log) printf("  Interface PCI     :\n");
	PCInb = 0;
	
#ifdef AVEC_PCI
#define UTILISE_FIFO_AUTO

	if(log) {
	#ifdef ACCES_PCI_DIRECT
		printf("    Le PCI est accede par affectation.\n");
	#else
	#ifdef UTILISE_DRIVER
		printf("    Le PCI est accede via un driver: ICADriver.\n");
	#else
		printf("    Le PCI est accede via IcaLib.\n");
	#endif
	#endif
	}

#ifdef UTILISE_DRIVER
	if(DriverScan("ICADriver",ICA_OPEN,PCIacces,&ica_nb)) {
		//	PCIacces[0] = DriverConnect("ICADriver",ICA_OPEN); ica_nb = 1;
		//	if(PCIacces[0]) {
		//		if(log) printf("    Ce driver est en service, et a detecte %d carte%s PCI.\n",ica_nb,(ica_nb > 1)?"s":"");
		//	}
		if(log) printf("    %d carte%s PCI detectee%s.\n",ica_nb,(ica_nb > 1)?"s":"",(ica_nb > 1)?"s":"");
		PCIconnecte = 1;
	} else {
		if(log) printf("    Ce driver est hors service (%s).\n",DriverLastMessage());
		ica_nb = 0;
		PCIconnecte = 0;
	}
#else
	ica_nb = 1;
	IcaInit(&(PCIacces[0]));
	PCIconnecte = 1;
#endif
	PCInb = ica_nb;
#ifdef CLUZEL_FIFO_INTERNE
	if(log) printf("    La Fifo interne est utilisee.\n");
#else
	if(log) printf("    La Fifo interne n'est pas utilisee.\n");
#endif
	n = 0;
	for(PCInum=0; PCInum<PCInb; PCInum++) {
		PCIedw[PCInum].type = CARTE_UNKN;
		PCIedw[PCInum].port = &(PCIacces[PCInum]);
		PCIedw[PCInum].fifo = 0;
		type_ok = 1;
		if(log) printf("    . Interface #%d\n",PCInum+1);
	#ifdef PCI_ID_DIRECT
		id = PciDeviceID(PCIedw[PCInum].port);
		if(log) printf("      | CSR#0B lu: %08X\n",id);
		if((id == 0x905410B5) || (id == 0x540610B5) || (id == 0xCEA010B5)) {
			PCIedw[PCInum].type = CARTE_MIG;
			DmaDemande = 1;
			IcaXferInit(PCIedw[PCInum].port,DmaDemande);
		} else if(id == 0xCEA110B5) {
			PCIedw[PCInum].type = CARTE_ICA;
			DmaDemande = 1;
			IcaXferInit(PCIedw[PCInum].port,DmaDemande);
		} else if(id == 0x434e5253) /* 'CNRS' */ {
			PCIedw[PCInum].type = CARTE_AB;
			IcaXferInit(PCIedw[PCInum].port,0);
		} else PCIconnecte = 0;
	#else /* PCI_ID_DIRECT */
		if(PciDeviceFind(0x10B5,0x9054,0,PCIedw[PCInum].port) || PciDeviceFind(0x10B5,0x5406,0,PCIedw[PCInum].port) || PciDeviceFind(0x10B5,0xCEA0,0,PCIedw[PCInum].port)) {
			PCIedw[PCInum].type = CARTE_MIG;
		#ifndef UTILISE_DRIVER
			IcaRegisterUse(PCIedw[PCInum].port,2);
			DmaDemande = 1;
		#else
			DmaDemande = 1;
		#endif
			IcaXferInit(PCIedw[PCInum].port,DmaDemande);
		} else if(PciDeviceFind(0x10B5,0xCEA1,0,PCIedw[PCInum].port)) {
			PCIedw[PCInum].type = CARTE_ICA;
		#ifndef UTILISE_DRIVER
			IcaRegisterUse(PCIedw[PCInum].port,2);
			DmaDemande = 1;
		#else
			DmaDemande = 1;
		#endif
			IcaXferInit(PCIedw[PCInum].port,DmaDemande);
		} else if(PciDeviceFind(0x5253,0x434e,0,PCIedw[PCInum].port)) {
			PCIedw[PCInum].type = CARTE_AB;
		#ifndef UTILISE_DRIVER
			IcaRegisterUse(PCIedw[PCInum].port,0);
		#endif
			IcaXferInit(PCIedw[PCInum].port,0);
		} else {
			if(log) printf("      | Carte inconnue (inutilisable)\n");
			#ifdef CLUZEL_FIFO_INTERNE
			PCIedw[PCInum].octets = FIFOdim * sizeof(Ulong);
			PCIedw[PCInum].fifo = (Ulong *)malloc(PCIedw[PCInum].octets);
			if(log) printf("      | FIFO allouee @%08X +%08X (soit %d valeurs)\n",(hexa)PCIedw[PCInum].fifo,PCIedw[PCInum].octets,FIFOdim);
			// FifoPrecedente = (Ulong *)malloc(PCIedw[PCInum].octets);
			continue;
			#endif
		}
	#endif /* PCI_ID_DIRECT */
		PCIedw[PCInum].dma = IcaDmaDispo(PCIedw[PCInum].port);
		if(log) printf("      | Carte trouvee: type %s, %s DMA\n",
			   LectViaNom[PCIedw[PCInum].type],PCIedw[PCInum].dma? "avec": "sans");
		n++;
	#ifdef ADRS_PCI_DISPO
		//-		IcaAdrsGet(PCIedw[PCInum].port,ICA_ACCES_PCI,(Ulong **)&(PCIadrs[PCInum]),&taille);
		IcaAdrsGet(PCIedw[PCInum].port,ICA_ACCES_PCI,&(PCIadrs[PCInum]),&taille);
		if(log) printf("      | Acces direct @%08X +%08X\n",(hexa)PCIadrs[PCInum],taille);
	#endif
	#ifdef CLUZEL_FIFO_INTERNE
		#ifdef UTILISE_FIFO_AUTO
			//-		IcaAdrsGet(PCIedw[PCInum].port,ICA_MEM_INTERNE,(Ulong **)&PCIedw[PCInum].fifo,&PCIedw[PCInum].octets);
			IcaAdrsGet(PCIedw[PCInum].port,ICA_MEM_INTERNE,&(PCIedw[PCInum].fifo),&(PCIedw[PCInum].octets));
			FIFOdim = PCIedw[PCInum].octets / sizeof(Ulong);
			if(log) printf("      | FIFO interne @%08X +%08X (soit %d valeurs)\n",(hexa)PCIedw[PCInum].fifo,PCIedw[PCInum].octets,FIFOdim);
		#else /* UTILISE_FIFO_AUTO */
			PCIedw[PCInum].octets = FIFOdim * sizeof(Ulong);
			PCIedw[PCInum].fifo = (Ulong *)malloc(PCIedw[PCInum].octets);
			if(log) printf("      | FIFO allouee @%08X +%08X (soit %d valeurs)\n",(hexa)PCIedw[PCInum].fifo,PCIedw[PCInum].octets,FIFOdim);
		#endif /* UTILISE_FIFO_AUTO */
		FifoPrecedente = (Ulong *)malloc(PCIedw[PCInum].octets); // a commenter, en general
	#else /* CLUZEL_FIFO_INTERNE */
		if(log) printf("      | Pas de FIFO disponible\n");
	#endif /* CLUZEL_FIFO_INTERNE */
	}
	if(n == 0) {
		PCInb = 0;
		if(log) printf("    Pas de carte PCI utilisable\n");
	}
	
#else  /* AVEC_PCI */
	PCIconnecte = 0;
	if(log) printf("    Pas de PCI utilisable\n");
	PCInum = 0; PCInb = 1;
#ifdef ADRS_PCI_DISPO
	PCIadrs[PCInum] = SimulPCI; /* pour la branche CMDES */
	PCIedw[PCInum].dma = 0;
	if(log) printf("    Acces PCI #%d simule @%08X\n",PCInum+1,(hexa)PCIadrs[PCInum]);
#else
	if(log) printf("    L'adresse du PCI #%d n'est pas accessible.\n",PCInum+1);
#endif
//+?	PCIacces[PCInum].id = PCInum;
	PCIedw[PCInum].type = CARTE_UNKN;
	PCIedw[PCInum].port = &(PCIacces[PCInum]);
#ifdef CLUZEL_FIFO_INTERNE
    PCIedw[PCInum].octets = FIFOdim * sizeof(Ulong);
	PCIedw[PCInum].fifo = (Ulong *)malloc(PCIedw[PCInum].octets);
	if(log) printf("    FIFO #%d allouee @%08X +%08X (soit %d valeurs)\n",PCInum+1,(hexa)PCIedw[PCInum].fifo,PCIedw[PCInum].octets,FIFOdim);
	FifoPrecedente = (Ulong *)malloc(PCIedw[PCInum].octets);
#endif
#endif  /* AVEC_PCI */
	
	for(PCInum=0; PCInum<PCInb; PCInum++) {
		if(PCIedw[PCInum].fifo) {
			int dim;
			dim = PCIedw[PCInum].octets / sizeof(Ulong);
			for(i=0; i<dim; i++) PCIedw[PCInum].fifo[i] = 0;
		}
	}
	PCInum = 0;
	if(log) printf("\n");
}
/* ========================================================================== */
static void SambaInitBasique() {
	int voie,sat,ptn,simu; int i;

	BigEndianSource = BigEndianOrder;
	DebugRestant = 0;
	BitTriggerNiveau = SAMBA_BITNIV_VOIE;
	BitTriggerNums = SAMBA_BITNUM_LOCAL;
	BitTriggerBloc = 0;
	SambaMode = SAMBA_AUTOGERE;
	LectureMaitre = -1;
#ifdef MODULES_RESEAU
	PortSvr = 64400;
#endif
	TailleSvr = 1024;
	PortEcoute = 1953;
	PortLectRep = 2048;
	PortLectSat = 3072;
	InstalleSamba = 0;
	ModeBatch = 0;
	CreationFichiers = 1;
	ImprimeConfigs = 0;
	FifoPrecedente = 0;
	TrmtPanicMax = 32;
	TrgrRegulClock = 0x7FFFFFFF;
	EvtNbMax = MAXEVTS;
	Evt = 0;
    EvtASauver = 0;
	VoieHisto = 0;
	TrmtCsnsmNb = 0;
    TrgrGardeSigne = 1;
	pSettingsModes = pSettingsTrigger = 0;
	FichierNtuples[0] = '\0';
	FichierSeuils[0] = '\0';
	FichierStatusBB[0] = '\0';
	ArchLngrEvt = ArchLngrVoie = 0;
	ArchMoyenne = 0; ArchMoyenneMax = 0;
	ArchPosEvt = 0;
	ArchRevInclus = 0;
	BancEnTest = 0;
	SimuModifiees = 0;
	gusecToKgJ = 1.0 / (1000.0 * 86400.0 * 1000000.0);
	LectArretExterieur = 0;
	VolumeManquant[0] = '\0';
	// strcpy(EdbServeur,"http://edwdbik.fzk.de:5984");
	// strcpy(EdbServeur,"http://134.158.176.27:5984/testdb");
	strcpy(EdbServeur,"neant"); EdbActive = 0;
	strcpy(ArchMoiMeme,"dacrunheader");
	LangueEtendDico = DICO_FIXE;
	SambaDefMax = 41;
	SambaExploreHW = 0;
	SambaNumerSelect = 0;
	SambaSelectionText = 0;
	SambaIdentSauvable = 0;
    SettingsSauveChoix = 0;
	SettingsTramesMaxi = 0;
	SambaRunCourant = -1;
	LectSurFichier = 0;
	LectModeStatus = 0;
	LectStampMini = 0;
	SettingsRegen = 1;
	LoggerType = 0;
	Organisation[0] = '\0';
	Starter[0] = '\0';
	LoggerAdrs[0] = '\0';
	strcpy(LoggerUser,"-u acquis darkmatr -l STPC -a Author=SAMBA -a Category=Acquisition");
	Demarreur = 0;
	ExpertConnecte = 0;
	PCIconnecte = 0;
	PCInb = 0;
	PCInum = 0;
	NumDefaut = -1;
	VoiesLues = 0;
	VoieIdentMax = 0;
	RepartFile = -1;
	PCIdemande = 0; IPdemande = 0;
	NtDim = 0; NtUsed = 0;
	for(i=0; i<CLASSE_NBOBJETS; i++) ClassementDemande[i] = CLASSE_TOUR;
	ClasseObjetATrier = CLASSE_NUMER; ClasseCritereDeTri = CLASSE_NOM;

	AdresseUtilisee[0] = '\0';
	AcquisNb = 0;
	for(sat=0; sat<MAXSAT; sat++) {
		Acquis[sat].code[0] = '\0';
		Acquis[sat].adrs[0] = '\0';
		strcpy(Acquis[sat].runs,"_");
		Acquis[sat].synchro.path = -1;
		Acquis[sat].ecoute.path = -1;
		Acquis[sat].nbrep = 0;
		Acquis[sat].etat.status = SATELLITE_ATTENTE;
		Acquis[sat].etat.active = 0;
		Acquis[sat].etat.nom_run[0] = '\0';
		Acquis[sat].etat.duree_lue[0] = '\0';
		Acquis[sat].etat.evt_trouves = 0;
		Acquis[sat].etat.KgJ = 0.0;
#ifdef ETAT_RESEAU
		Acquis[sat].id = IdLocal;
#endif
		AcquisListe[sat] = Acquis[sat].code;
	}
	AcquisListe[sat] = "\0";
	PartitNb = 0;
	for(ptn=0; ptn<MAXMAITRE; ptn++) {
		Partit[ptn].mtr[0] = '\0';
		Partit[ptn].nbsat = 0;
	}
	EcritureMaitre = -1;
	SambaPartage = 0; SambaPartageDim = sizeof(TypeSambaPartagee); SambaPartageId = -1;

	NumeroLect = NumeroBanc = NumeroManip = 0;
	SambaProcsNum = 0;
	SambaDocFormat = IMPRESSION_PDF;
	SambaDocSupport = IMPRESSION_FICHIER;
	SambaDocCablage = SambaDocModeles = SambaDocAutom = SambaDocScripts = 1;
	NbTirages = 1; NumPrinter = 0; strcpy(ListePrinters,"OCE/Jet");

	for(voie=0; voie<VOIES_MAX; voie++) VoieLue[voie] = -1;
	CapteurNom[0] = "(voie1)"; CapteurNom[1] = "\0"; CapteurNum = 0;
	VoieNum = 0;

	for(i=0; i<CONFIG_TOT; i++) ConfigCle[i+1] = ConfigListe[i] = &(ConfigNom[i][0]);
	ConfigCle[i+1] = ConfigListe[i+1] = "\0";
	ConfigNb = 0; /* strcpy(ConfigListe[ConfigNb++],CONFIG_STD); */ ConfigNom[ConfigNb][0] = '\0';
	ConfigCle[0] = "indefinie";
	ConfigAvant = ConfigNum = 0;

	for(simu=0; simu<SIMU_MAX; simu++) SimuListe[simu] = Simu[simu].nom; SimuListe[simu] = "\0";
	SimuNum = 0;

	SambaGC = 0;
#ifdef LINUX
	strcpy(SambaFontName,"*fixed-medium-r-*%d*"); // "*courier-medium-r-*%d*"
	SambaFontSize = 10;
#endif
#ifdef XCODE
	strcpy(SambaFontName,"Courier");
	SambaFontSize = 10;
#endif
	SambaTraits = 1;
	/* Jeu de 256 couleurs */
	OpiumColorArcEnCiel(SambaArcEnCiel,256);

	TestSimul = 0;
#ifdef BRANCHE_TESTS
	{	int vm;
		for(vm=0; vm<VOIES_TYPES; vm++) {
			TemplMontee[vm] = 5;
			TemplDescente[vm] = 40;
			AmplDeposee[vm] = 500.0;
		}
	/*	SambaCalculeTemplates(); */
		for(vm=0; vm<VOIES_TYPES; vm++) {
			dim = TemplMontee[vm]; /* nombre de points de montee */
			for(l=0; l<dim; l++) Template[vm][l] = 0.5 * (1.0 - (float)cos(PI * (double)l / (double)dim));
			dim = TemplDescente[vm]; /* nombre de points pour decroitre de 1/2,718 */
			while(l < MAXTEMPLATE) {
				Template[vm][l] = Template[vm][l - 1] * (float)exp(-1.0 / (double)dim);
				if(Template[vm][l] == 0) break;
				l++;
			}
			TemplDim[vm] = l;
		}
		VoieTouchee = -1; BoloTouche = 0; EvtGenere = 0; TemplTemps = 0;
	}
#endif
	PointsEvt = 65536;  /* pour etre tranquille avec les index dans les graphes des evts */
	for(i=0; i<MAXUSERNUM; i++) SambaUserNum[i] = i + 1;

	SambaTrmtCouleurs = Malloc(TRMT_NBTYPES+1,OpiumColor);
	OpiumColorAssign(SambaTrmtCouleurs+NEANT,WND_COLOR_DEMI, WND_COLOR_DEMI, WND_COLOR_MAX);
	OpiumColorAssign(SambaTrmtCouleurs+TRMT_DEMODUL,GRF_RGB_ORANGE);
#ifdef TRMT_ADDITIONNELS
	OpiumColorAssign(SambaTrmtCouleurs+TRMT_LISSAGE,GRF_RGB_TURQUOISE);
	OpiumColorAssign(SambaTrmtCouleurs+TRMT_MOYENNE,0, WND_COLOR_DEMI, WND_COLOR_MAX);
	OpiumColorAssign(SambaTrmtCouleurs+TRMT_MAXIMUM,WND_COLOR_3QUARTS, WND_COLOR_MAX, 0);
	OpiumColorAssign(SambaTrmtCouleurs+TRMT_DECONV, WND_COLOR_3QUARTS, WND_COLOR_QUART, WND_COLOR_MAX);
	#ifdef A_LA_GG
		OpiumColorAssign(SambaTrmtCouleurs+TRMT_INTEDIF,GRF_RGB_MAGENTA);
	#endif /* A_LA_GG */
	#ifdef A_LA_MARINIERE
		OpiumColorAssign(SambaTrmtCouleurs+TRMT_CSNSM,GRF_RGB_VIOLET);
	#endif /* A_LA_MARINIERE */
#endif /* TRMT_ADDITIONNELS */
	OpiumColorAssign(SambaTrmtCouleurs+TRMT_FILTRAGE,GRF_RGB_GREEN); // emplacement de "standard"
	OpiumColorAssign(SambaTrmtCouleurs+TRMT_INVALID,GRF_RGB_RED);
	OpiumColorAssign(SambaTrmtCouleurs+TRMT_NBTYPES,GRF_RGB_YELLOW);  // emplacement pour tout filtrage

#ifdef DICO_A_LA_MAIN
	terme = DicoInit(&SambaDicoVoies,"voies",2);
//	DicoAjoute(terme++,"centre","ionis1");
//	DicoAjoute(terme++,"garde","ionis2");
	DicoAjoute(terme++,"ionis1","centre");
	DicoAjoute(terme++,"ionis2","garde");

	terme = DicoInit(&SambaDicoReglages,"reglages",8);
	DicoAjoute(terme++,"ampl-tri-d9","ampl-modul");
	DicoAjoute(terme++,"comp-car-v5","comp-modul");
	DicoAjoute(terme++,"ampl-car-d9","corr-trngl");
	DicoAjoute(terme++,"DAC1","polar-ionis");
	DicoAjoute(terme++,"DAC2","polar-voie3");
	DicoAjoute(terme++,"gain-v1","gain-voie1");
	DicoAjoute(terme++,"gain-v2","gain-voie2");
	DicoAjoute(terme++,"gain-v3","gain-voie3");
#endif

}
/* ========================================================================== */
void SambaAjouteRacine(char *complet, char *dir, char *declare) {
	/* Racine = PrefPath par defaut */
	char *traduit,*ajout; char nom_complet[MAXFILENAME];

	if(!strncmp("$HOME/",declare,6)) {
		strcat(strcpy(nom_complet,HomeDir),declare+6);
		traduit = nom_complet;
	} else if(!strncmp("$top/",declare,5)) {
		strcat(strcpy(nom_complet,FullTopDir),declare+5);
		traduit = nom_complet;
	} else if(!strncmp("$maitre/",declare,8)) {
		strcat(strcpy(nom_complet,MaitreDir),declare+8);
		traduit = nom_complet;
	} else traduit = declare;
	if(traduit[0] == SLASH) { if(complet != traduit) strcpy(complet,traduit); }
	else {
		ajout = (dir)? dir: PrefPath;
		if(complet == traduit) {
			strcat(strcpy(nom_complet,ajout),traduit);
			strcpy(complet,nom_complet);
		} else strcat(strcpy(complet,ajout),traduit);
	}
	RepertoireSimplifie(complet);
}
/* ========================================================================== */
void SambaAjoutePrefPath(char *complet, char *declare) {
	SambaAjouteRacine(complet,0,declare);
}
/* ========================================================================== */
static char SambaParms(int argc,char *argv[]) {
	char existe; int j; char start_dir[MAXFILENAME];

	/*
	 * Initialisation du logiciel (y.c. certains parametres ajustables)
	 */
    XcodeSambaDebug = 0;
	VoieTampon = 0;
	
	/*
	 *  Configuration globale effective
	 */
	/* Repertoire de travail a priori */
	if(!SambaAideDemandee) {
		getcwd(start_dir,MAXFILENAME);
		printf("Mise en route:\n");
		printf("  Repertoire de travail initial: '%s'\n",start_dir);
		printf("  Appel: '"); j = 0; while(j < argc) { if(j) printf(" "); printf("%s",argv[j]); j++; } printf("'\n");
	}
	if((argc > 1) && !strncmp(argv[1],"-orga",4)) {
		char orga[80],topfile[MAXFILENAME],texte[1024]; int rc;
		if(strlen(argv[1]) > 6) strncpy(orga,argv[1]+6,80);
		else if(argc > 2) strncpy(orga,argv[2],80);
		strcat(strcpy(topfile,HomeDir),".samba_top");
		sprintf(texte,"cp -v %s_%s %s",topfile,orga,topfile);
		if((rc = system(texte))) {
			printf("!!! Organisation de Samba non definie: %s (%s)\n",orga,strerror(rc));
			exit(rc);
		}
	}

	/* Modifications utilisateur de l'environnement au moment de l'appel */
	AnnexesDefaults();

	TangoDefaults();
	SambaTopDirAt(TopDir,"TangoArgs");
	printf("  Les parametres par defaut d'appel de TANGO sont dans '%s%s'\n",TopDir,"TangoArgs");
	ArgScan("TangoArgs",TangoDesc);

	/* Valeurs par defaut de l'environnement d'execution */
	SambaDefaults();
//	PrgmRepartiteur = 0x1E;
	SambaTopDirAt(TopDir,"SambaArgs"); strcpy(start_dir,TopDir);
#ifdef macintosh
	strcat(strcpy(InfoPrefs,SambaAppli),"/Contents/Resources/");
#else
	strcpy(InfoPrefs,"Info/Samba/");
#endif
    strcpy(XcodeSambaDebugTxt,"NO");

	strcpy(MaitrePrefs,".");
	strcpy(SetupPrefs,"prefs");
	strcpy(NomGeneral,"ConfigGenerale");
	strcpy(CtlgPrefs,TopDir);
	strcpy(RelecPrefs,"donnees");
	strcpy(ArchDir[STRM],"streams");
	strcpy(ArchDir[EVTS],"events");
	SambaSrceUtilisee = 0;
	Gardiens[0] = '\0';
	InitBolos = 0;
	DemandeInstalle = 0;
	CreationFichiers = 1;
	ExpertConnecte = 0;
	SettingsRunFamille = RUN_FAMILLE_BANC; /* re-initialise plus tard... */
	RunCategNum = 0; /* re-initialise plus tard... */

	printf("  Les parametres par defaut d'appel de SAMBA sont dans '%s%s'\n",TopDir,"SambaArgs");
	existe = (ArgScan("SambaArgs",SambaDesc) != 0);
    // printf("Complements:\n"); ArgPrint("*",SambaDesc);
#ifdef CODE_WARRIOR_VSN
	argc = ccommand(&argv);
#endif /* CODE_WARRIOR_VSN */
    ArgListChange(argc,argv,SambaDesc); /* Les booleens tel que LangueEtendDico pas modifies? */
	if(DemandeInstalle) {
		InstalleSamba = 1;
		CreationFichiers = 1;
		ModeBatch = 0;
		DemandeInstalle = 0;
	}
	Organisation[0] = '\0';
	if(!existe) ArgPrint("SambaArgs",SambaDesc);
	// ArgPrint("*",SambaDesc);
	if(InstalleSamba) ModeBatch = 1;
    XcodeSambaDebug = !strcmp(XcodeSambaDebugTxt,"YES");
    printf("  Execution en mode %s (NSDocumentRevisionsDebugMode: %s)\n",XcodeSambaDebug? "debug sous Xcode": "production",XcodeSambaDebugTxt);

	RepertoireTermine(TopDir);
	RepertoireNomComplet(start_dir,TopDir,1,FullTopDir); RepertoireSimplifie(FullTopDir);
	SambaAjouteTopDir(MaitrePrefs,MaitreDir); RepertoireSimplifie(MaitreDir);
	SambaAjouteTopDir(SetupPrefs,PrefPath);
	SambaAjouteTopDir(CtlgPrefs,CtlgPath);
	SambaAjouteTopDir(SauvPrefs,SauvVol);
	SambaAjouteTopDir(RelecPrefs,RelecPath);
	SambaAjouteTopDir(ResuPrefs,ResuPath);
	SambaAjouteTopDir(DbazPrefs,DbazPath);
	SambaAjouteTopDir(FigsPrefs,FigsPath);
	SambaAjouteTopDir(InfoPrefs,SambaInfoDir);

/*	{	ItemVar var;
  	var = SambaVar;
	ItemVarSet(var++,"$HOME",HomeDir);
	ItemVarSet(var++,"$top",FullTopDir);
	ItemVarSet(var++,"$maitre",MaitreDir);
	ItemVarSet(var++,ITEMVAR_END);
	printf("  Variables definies:"); var = SambaVar; while(var->nom) { printf(" %s",var->nom); var++; }; printf("\n");
	ArgUserVars(SambaVar); } */

	SambaRunDate();
	TrmtNomme();
	IPserial = BNserial = NumDefaut;

//	if     (HoteNum == 1) { Acquis[AcquisLocale].runs[0] = 'm'; strcpy(NomEDW,"m"); }
//	else if(HoteNum >  1) { Acquis[AcquisLocale].runs[0] = 'a' + (HoteNum - 2); sprintf(NomEDW,"s%d",HoteNum-1); }
//	else                  { Acquis[AcquisLocale].runs[0] = '_'; strcpy(NomEDW,"local"); } /* cas ou HoteNum == 0 */
//	printf("  Code pour l'acquisition: %s\n",NomEDW);

	printf("  SambaArgs pris en compte\n");
	return(existe);
}
/* ========================================================================== */
int SambaDisque() {
	int fmt;
	
	if(OpiumReadFile("Espace de sauvegarde",SauvVol,80) == PNL_CANCEL) return(0);
	ArgPrint("SambaArgs",SambaDesc);
	RepertoireTermine(SauvVol);
	strcat(strcpy(BBstsPath,SauvVol),BBstsDir); RepertoireTermine(BBstsPath);
	printf("- Les status de BBs seront maintenant sauvegardes dans:\n   '%s'\n",BBstsPath);
	for(fmt=0; fmt<ARCH_TYPEDATA; fmt++) {
		strcat(strcpy(ArchZone[fmt],SauvVol),ArchDir[fmt]); RepertoireTermine(ArchZone[fmt]);
		printf("- Les %s seront maintenant sauvegardes dans:\n   '%s'\n",ArchFmtNom[fmt],ArchZone[fmt]);
	}
	return(0);
}
/* ========================================================================== */
static int SambaConfigDuplic(Menu menu, MenuItem *item) {

	if(ConfigNb >= CONFIG_TOT) {
		OpiumFail(L_("Deja %d configurations definies. En supprimer une, avant nouvel ajout"),CONFIG_TOT);
		return(0);
	}
	if(OpiumExec(pDetecConfigDup->cdr) == PNL_CANCEL) return(0);
	ConfigNum = ConfigNb;
	strcpy(&(ConfigNom[ConfigNb++][0]),ConfigNouv); ConfigNom[ConfigNb][0] = '\0';
	//+	DetecteurSauveConfig(ConfigNum);

	return(0);
}
/* ========================================================================== */
static int SambaConfigRenomme(Menu menu, MenuItem *item) {
	int voie,vm,num_config; TypeConfigVoie *config_voie;

	if(OpiumExec(pDetecConfigMov->cdr) == PNL_CANCEL) return(0);

	for(voie=0; voie<VoiesNb; voie++) {
		if(!strcmp(VoieManip[voie].def.nom,&(ConfigNom[ConfigNum][0])))
			strcpy(VoieManip[voie].def.nom,ConfigNouv);
		config_voie = &(VoieManip[voie].config);
		for(num_config=0; num_config<config_voie->nb; num_config++) {
			if(!strcmp(config_voie->def[num_config].nom,&(ConfigNom[ConfigNum][0])))
				strcpy(config_voie->def[num_config].nom,ConfigNouv);
		}
	}

	for(vm=0; vm<ModeleVoieNb; vm++) {
		if(!strcmp(VoieStandard[vm].def.nom,&(ConfigNom[ConfigNum][0])))
			strcpy(VoieStandard[vm].def.nom,ConfigNouv);
		config_voie = &(VoieStandard[vm].config);
		for(num_config=0; num_config<config_voie->nb; num_config++) {
			if(!strcmp(config_voie->def[num_config].nom,&(ConfigNom[ConfigNum][0])))
				strcpy(config_voie->def[num_config].nom,ConfigNouv);
		}
	}
	strcpy(&(ConfigNom[ConfigNum][0]),ConfigNouv);
	SambaConfigRefresh(0);

	return(0);
}
/* ========================================================================== */
int SambaConfigChange(Panel panel, int item, void *arg) {
	TypeConfigDefinition *config,*utilisee;
	int voie,classe,globale; short num_config; short defini;

	if(ConfigNum == ConfigAvant) return(0);
	DetecteurSauveConfig(ConfigAvant);

	globale = ConfigNum + 1;
	if(GestionConfigs) printf("(%s)  ____ Installe globale #%d (%s)\n",__func__,globale,ConfigCle[globale]);
	// printf("(%s) Config#%d (%s) demandee\n",__func__,globale,ConfigListe[ConfigNum]);
	for(voie=0; voie<VoiesNb; voie++) {
		utilisee = &(VoieManip[voie].def);
		defini = 0;
		if((num_config = VoieManip[voie].config.num[globale].evt) >= 0) {
			config = &(VoieManip[voie].config.def[num_config]); defini = config->defini.evt;
		}
		if(defini) {
			memcpy(&(utilisee->evt),&(config->evt),sizeof(TypeVoieModele));
			utilisee->defini.evt = defini; utilisee->global.evt = config->global.evt;
		}
		for(classe=0; classe<TRMT_NBCLASSES; classe++) {
			defini = 0;
			if((num_config = VoieManip[voie].config.num[globale].trmt[classe]) >= 0) {
				config = &(VoieManip[voie].config.def[num_config]); defini = config->defini.trmt[classe];
			}
			if(defini) {
				if(GestionConfigs) printf("(%s) | Traitement %s voie %s: recopie config #%d (%s) sur config utilisee\n",__func__,TrmtClasseNom[classe],VoieManip[voie].nom,
										  num_config,config->nom);
				memcpy(&(utilisee->trmt[classe]),&(config->trmt[classe]),sizeof(TypeTraitement));
				utilisee->defini.trmt[classe] = defini; utilisee->global.trmt[classe] = config->global.trmt[classe];
			}
			else if(GestionConfigs) printf("(%s) | Traitement %s voie %s: config utilisee inchangee\n",__func__,TrmtClasseNom[classe],VoieManip[voie].nom);
			if(GestionConfigs) printf("(%s) | Traitement %s voie %s: represente globale #%d (%s)\n",__func__,TrmtClasseNom[classe],VoieManip[voie].nom,
									  utilisee->global.trmt[classe],ConfigCle[utilisee->global.trmt[classe]]);
		}
		defini = 0;
		if((num_config = VoieManip[voie].config.num[globale].trgr) >= 0) {
			config = &(VoieManip[voie].config.def[num_config]); defini = config->defini.trgr;
		}
		if(defini) {
			memcpy(&(utilisee->trgr),&(config->trgr),sizeof(TypeTrigger));
			utilisee->defini.trgr = defini; utilisee->global.trgr = config->global.trgr;
		}
		defini = 0;
		if((num_config = VoieManip[voie].config.num[globale].rgul) >= 0) {
			config = &(VoieManip[voie].config.def[num_config]); defini = config->defini.rgul;
		}
		if(defini) {
			memcpy(&(utilisee->rgul),&(config->rgul),sizeof(TypeRegulParms));
			utilisee->defini.rgul = defini; utilisee->global.rgul = config->global.rgul;
		}
	}
	ConfigAvant = ConfigNum;
	if(GestionConfigs) printf("(%s) |____ Installation terminee\n",__func__);

	return(SambaConfigRefresh(panel));
}
/* ========================================================================== */
static int SambaConfigRefresh(Panel panel) {
	char reglage; int rc;

	rc = 0;
	LectTrmtEnter(0,0);
	if(OpiumDisplayed(pLectConfigRun->cdr)) PanelRefreshVars(pLectConfigRun);
	if(OpiumDisplayed(pLectTraitements->cdr)) PanelRefreshVars(pLectTraitements);
	if(OpiumDisplayed(bDefEvts)) {
		if(panel) { if(OpiumEstContenuDans(panel->cdr,bDefEvts)) { panel = 0; rc = 1; } }
		SettingsEvtsCree(0,0);
	}
	for(reglage=SETTINGS_TRMT; reglage<SETTINGS_NBTYPES; reglage++) {
		if(OpiumDisplayed(SettingsVar[reglage].planche)) {
			if(panel) { if(OpiumEstContenuDans(panel->cdr,SettingsVar[reglage].planche)) { panel = 0; rc = 1; } }
			SettingsDetecCree(reglage,0,0);
		}
	}

	return(rc);
}
/* ========================================================================== */
int SambaSauveSettings() { ArgPrint(ConfigGeneral,Setup); return(0); }
/* ========================================================================== */
int SambaSauveArgs() { ArgPrint("SambaArgs",SambaDesc); return(0); }
/* ========================================================================== */
static char SambaInitOpium() {
	int ligs,cols,k; char titre[80];

	printf("  Initialisation de l'interface utilisateur avec OPIUM\n"); fflush(stdout);
	WndLog(500,45,60,100,"Journal de SAMBA");
	WndSetFontName(SambaFontName);
	WndSetFontSize(SambaFontSize);
	if(FondNoir) WndSetBgndBlack(1);
#ifdef X11
	if(!strcmp(ServeurX11,".")) strcpy(ServeurX11,getenv("DISPLAY"));
	if(!ModeBatch) printf("  Affichage X11 demande sur %s\n",ServeurX11);
#endif
    if(ModeBatch) OpiumInit(WND_NONE);
	else if(!OpiumInit(ServeurX11)) { printf("  ! Affichage impossible\n"); return(0); }
#ifdef X11
	// interessant, a ameliorer: WndNewRoot("SAMBA",0,0);
	// il faut alors introduire et gerer des boutons de deplacement, agrandissement et fermeture
	// sans compter l'affichage initial (effectif seulement sur evt refresh?) a mieux gerer
#endif
	ImpressionInit();
	OrgaInit();
	LogInit();
	ScriptInit();
	//	LogOnTerm("Journal de SAMBA",60,100,32768);
	WndAPropos("SAMBA",APropos);					
	PetitEcran = (OpiumServerHeigth(0) < 870);
	Dx = WndColToPix(1); Dy = WndLigToPix(1);
	ligs = OpiumServerHeigth(0) / Dy;
	cols = OpiumServerWidth(0) / Dx;
	printf("  Caracteres %d x %d dans un ecran %d x %d, soit %d lignes de %d colonnes\n",
		Dx,Dy,OpiumServerWidth(0),OpiumServerHeigth(0),ligs,cols);
	
	/*	OpiumDebug(OPIUM_DEBUG_BOARD,2); */
	OpiumOptions(OPIUM_MENU,extensions_menu);
	/*	OpiumOptions(OPIUM_PANEL,extensions_menu); */
	OpiumOptions(OPIUM_PANEL,extensions_edit);
	OpiumOptions(OPIUM_TERM,extensions_ps);
	OpiumOptions(OPIUM_GRAPH,extensions_ps);
	PanelMaxChamp(80);
	GraphQualiteDefaut(FondPlots? WND_Q_ECRAN: WND_Q_PAPIER);
	GraphLargeurTraits(WND_Q_PAPIER,SambaTraits);
	GraphLargeurTraits(WND_Q_ECRAN,1);

	/* Prise en compte de la langue */
	SambaLangueDefini(SambaInfoSource,SambaInfoDir);
	SambaLangueUtilise(1);

	/* Initialisation et creation de l'interface */
	mSambaClassique = MenuIntitule(iSambaClassique,"Samba");
	cSambaClassique = mSambaClassique->cdr;
	mHardware = MenuLocalise(iHardware);
	mSambaRepartiteurs = MenuLocalise(iSambaRepartiteurs);
	mSambaNumeriseurs = MenuLocalise(iSambaNumeriseurs);
	mSambaDetecteurs = MenuLocalise(iSambaDetecteurs);
	mDetecConfig = MenuLocalise(iDetecConfig);
	mSwDump = MenuLocalise(iSwDump);
	mSambaDefinitions = MenuLocalise(iSambaDefinitions);
	mSambaPlus = MenuIntitule(iSambaPlus,"Complements");
	mSambaReglages = MenuIntitule(iSambaReglages,"Tableaux de bord");
#ifdef SAUVE_AVEC_MENU
	iSambaSauve = InfoCreate(1,80);
	InfoTitle(iSambaSauve,"Dernier avis avant liquidation");
	OpiumSize(iSambaSauve->cdr,40*Dx,50);
	mSambaQuitte = MenuIntitule(iSambaQuitte,"Ta derniere carte:");
#endif /* SAUVE_AVEC_MENU */
	OpiumProgresCreate();
	OpiumPosition(OpiumCdrPrompt,5+20*Dx,48);
	
	pTri = PanelCreate(3);
	PanelListB(pTri,L_("Objet"),ClasseObjet,&ClasseObjetATrier,16);
	PanelListB(pTri,L_("Critere"),ClasseInfo,&ClasseCritereDeTri,16);

	pSambaImprimeForme = PanelCreate(6);
	PanelListB(pSambaImprimeForme,"Media",ImpressionSupports,&SambaDocSupport,IMPRESSION_SUPPORT_LNGR);
	PanelListB(pSambaImprimeForme,"Format",ImpressionFormats,&SambaDocFormat,IMPRESSION_FORMAT_LNGR);
	PanelBool (pSambaImprimeForme,"Cablage",&SambaDocCablage);
	PanelBool (pSambaImprimeForme,"Modeles",&SambaDocModeles);
	PanelBool (pSambaImprimeForme,"Support",&SambaDocAutom);
	PanelBool (pSambaImprimeForme,"Scripts",&SambaDocScripts);
	pSambaImprimeSvr = PanelCreate(2);
	PanelKeyB(pSambaImprimeSvr,"Sur quelle imprimante",ListePrinters,&NumPrinter,16);
	PanelInt(pSambaImprimeSvr,"En combien d'exemplaires",&NbTirages);
	OpiumTitle(pSambaImprimeSvr->cdr,"Options d'impression");

	pDetecConfigDup = PanelCreate(1);
	PanelText(pDetecConfigDup,L_("Nouvelle configuration"),ConfigNouv,CONFIG_NOM);
	pDetecConfigMov = PanelCreate(2);
	PanelList(pDetecConfigMov,L_("Configuration a renommer"),ConfigListe,&ConfigNum,CONFIG_NOM);
	PanelText(pDetecConfigMov,L_("Nouveau nom"),ConfigNouv,CONFIG_NOM);

	InstrumRectDim(&Gliss2lignes,  Dy,2*Dx);  /* 2 lignes = 1 interligne  */
	InstrumRectDim(&Gliss3lignes,2*Dy,2*Dx);  /* 3 lignes = 2 interlignes */
	InstrumRectDim(&Gliss4lignes,3*Dy,2*Dx);  /* 4 lignes = 3 interlignes */
	InstrumRectDim(&Gliss5lignes,4*Dy,2*Dx);  /* 5 lignes = 4 interlignes */
	InstrumRectDim(&Gliss6lignes,5*Dy,2*Dx);  /* 6 lignes = 5 interlignes */
	InstrumRectDim(&Colonne,10*Dy,Dx);

	pArgs = PanelDesc(SambaDesc,1);
	pSetup = PanelDesc(Setup,1);
	pour_tout(k,CLASSE_NBOBJETS) {
		tClasse[k] = TermCreate(48,60,16384);
		sprintf(titre,L_("Classement des %s"),ClasseObjet[k]);
		TermTitle(tClasse[k],titre);
	}

	OpiumTableCreate(&SambaTableModeles);
	OpiumTableAdd(&SambaTableModeles,"Physique",ARG_TYPE_STRUCT,(void *)&ModelePhysEditAS,&ModelePhysNb,0);
	OpiumTableAdd(&SambaTableModeles,"Voies",ARG_TYPE_STRUCT,(void *)&ModeleVoieEditAS,&ModeleVoieNb,0);
	OpiumTableAdd(&SambaTableModeles,"Detecteurs",ARG_TYPE_STRUCT,(void *)&ModeleDetEditAS,&ModeleDetNb,0);
	OpiumTableAdd(&SambaTableModeles,"Cablages",ARG_TYPE_STRUCT,(void *)&ModeleCablageASMaxi,&ModeleCableNb,0);
	OpiumTableAdd(&SambaTableModeles,"ADC",ARG_TYPE_STRUCT,(void *)&ModeleADCAS,&ModeleADCNb,0);
	OpiumTableAdd(&SambaTableModeles,"Numeriseurs",ARG_TYPE_STRUCT,(void *)&ModeleBNAS,&ModeleBNNb,0);
	OpiumTableAdd(&SambaTableModeles,"Repartiteurs",ARG_TYPE_STRUCT,(void *)&ModeleRepAS,&ModeleRepNb,ModeleRepDescEdit);
	OpiumTableAdd(&SambaTableModeles,"Simulations",ARG_TYPE_STRUCT,(void *)&SimuAS,&SimuNb,0);
	mSambaModeles = OpiumTableMenuCreate(SambaTableModeles);

	LectMenuBarre();
	MonitMenuBarre();
	CalcMenuBarre();
	GestMenuBarre();
	
	mSambaBarre = MenuIntitule(iSambaBarre,"Samba");

#ifdef ATTENTE_AVEC_TIMER
	SambaAttente = TimerCreate(SambaAttenteFin,0);
#endif /* ATTENTE_AVEC_TIMER */
	return(1);
}
/* ========================================================================== */
void SambaVerifieProcess() {
	FILE *f; pid_t ancien,actuel;
	int reponse; char **boutons;
	char *choix[] = {
		" Ah zut! j'abandonne ",
		" Eh bien, on le supprime! ",
		" Je ne vois rien.. ",
		"\0"
	};

	actuel = getpid();
	f = fopen("SambaPid","r");
	if(f) {
		fscanf(f,"%d",&ancien);
		fclose(f);
		if((kill(ancien,0) == 0) && (ancien != actuel)) {
			boutons = LL_(choix);
			if((reponse = OpiumChoix(3,boutons,L_("Un process SAMBA est deja en cours: que choisis-tu?"))) == 1) {
				kill(ancien,SIGTERM); /* software termination signal from kill */
				sleep(1);
				if(kill(ancien,0) == 0) kill(ancien,SIGKILL); /* kill (cannot be caught or ignored) */
			} else if(reponse == 2) {
				remove("SambaPid");
			} else {
				OpiumNote(L_("Tu peux maintenant reprendre l'ancien process"));
				exit(0);
			}
		}
	}
	f = fopen("SambaPid","w");
	if(f) {
		fprintf(f,"%d\n",actuel);
		fclose(f);
	}
}
/* ========================================================================== */
static void SambaPrepareFolder() {
	int i,fmt;

	i = 0;
	FolderRef[i].nom = FullTopDir; strcpy(FolderRef[i].titre,L_("Dossier principal")); i++;
	FolderRef[i].nom = MaitreDir; strcpy(FolderRef[i].titre,L_("Dossier maitre")); i++;
	FolderRef[i].nom = ConfigGeneral; strcpy(FolderRef[i].titre,L_("Liste des fichiers")); i++;
	FolderRef[i].nom = FichierPrefModeles; strcpy(FolderRef[i].titre,L_("Liste des modeles")); i++;
	FolderRef[i].nom = FichierModlChassis; strcpy(FolderRef[i].titre,L_("Definition du chassis")); i++;
	FolderRef[i].nom = FichierModlNumeriseurs; strcpy(FolderRef[i].titre,L_("Modeles de numeriseurs")); i++;
	FolderRef[i].nom = FichierModlCablage; strcpy(FolderRef[i].titre,L_("Modeles de cablage")); i++;
	FolderRef[i].nom = FichierModlDetecteurs; strcpy(FolderRef[i].titre,L_("Modeles de detecteurs")); i++;
	FolderRef[i].nom = FichierModlRepartiteurs; strcpy(FolderRef[i].titre,L_("Modeles de repartiteurs")); i++;
	FolderRef[i].nom = FichierModlEnvir; strcpy(FolderRef[i].titre,L_("Modele d'environnement")); i++;
	FolderRef[i].nom = FichierPrefParms; strcpy(FolderRef[i].titre,L_("Parametres")); i++;
	FolderRef[i].nom = FichierRunEnvir; strcpy(FolderRef[i].titre,L_("Environnement courant")); i++;
	FolderRef[i].nom = FichierPrefProcedures; strcpy(FolderRef[i].titre,L_("Procedures generales")); i++;
	FolderRef[i].nom = FichierPrefDetecteurs; strcpy(FolderRef[i].titre,L_("Detecteurs utilises")); i++;
	FolderRef[i].nom = FichierPrefCablage; strcpy(FolderRef[i].titre,L_("Cablage utilise")); i++;
	FolderRef[i].nom = FichierPrefNumeriseurs; strcpy(FolderRef[i].titre,L_("Numeriseurs utilises")); i++;
	FolderRef[i].nom = FichierPrefRepartiteurs; strcpy(FolderRef[i].titre,L_("Repartiteurs utilises")); i++;
	FolderRef[i].nom = FichierPrefMedia; strcpy(FolderRef[i].titre,L_("Media")); i++;
	FolderRef[i].nom = FichierPrefReseau; strcpy(FolderRef[i].titre,L_("Reseau des acquisitions")); i++;
	FolderRef[i].nom = FichierPrefAutom; strcpy(FolderRef[i].titre,L_("Support electromecanique")); i++;
	FolderRef[i].nom = FichierPrefFiltres; strcpy(FolderRef[i].titre,L_("Filtres")); i++;
	FolderRef[i].nom = FichierPrefMonit; strcpy(FolderRef[i].titre,L_("Graphique")); i++;
	FolderRef[i].nom = MonitFenFichier; strcpy(FolderRef[i].titre,L_("Fenetres utilisateurs")); i++;
	FolderRef[i].nom = FichierPrefSimu; strcpy(FolderRef[i].titre,L_("Simulation")); i++;
	FolderRef[i].nom = FichierPrefCalcul; strcpy(FolderRef[i].titre,L_("Calculs de FFTs")); i++;
	FolderRef[i].nom = FichierPrefExports; strcpy(FolderRef[i].titre,L_("Exportations de variables")); i++;
	FolderRef[i].nom = FichierCatalogue; strcpy(FolderRef[i].titre,L_("Catalogue des runs sauvegardes")); i++;
	for(fmt=0; fmt<ARCH_TYPEDATA; fmt++) {
		FolderRef[i].nom = ArchZone[fmt]; sprintf(FolderRef[i].titre,L_("Sauvegarde des %s"),ArchFmtNom[fmt]); i++;
	}
	FolderRef[i].nom = BBstsPath; strcpy(FolderRef[i].titre,L_("Sauvegarde des registres des numeriseurs")); i++;
	FolderNb = i;
	for(i=0; i<FolderNb; i++) FolderListe[i] = FolderRef[i].titre;
	FolderListe[i] = "\0";
	FolderNum = 0;
	FolderActn = 0;
	pFolderActn = PanelCreate(2);
	PanelList(pFolderActn,L_("Fichier ou repertoire"),FolderListe,&FolderNum,32);
	PanelKeyB(pFolderActn,L_("Montrer"),L_("contenu/dossier"),&FolderActn,8);
}
/* ========================================================================== */
static void SambaNomFichiers() {
	SambaAjoutePrefPath(FichierPrefModeles,NomModeles);
	SambaAjoutePrefPath(FichierPrefMedia,NomMedia);
	SambaAjoutePrefPath(FichierPrefSimu,NomSimu);
	SambaAjoutePrefPath(FichierPrefReseau,NomReseau);
	SambaAjoutePrefPath(FichierPrefSatellites,NomSatellites);
	SambaAjoutePrefPath(FichierCrParms,NomCr);
	SambaAjoutePrefPath(FichierPrefParms,NomParms);
	SambaAjoutePrefPath(FichierPrefDico,NomDico);
	SambaAjoutePrefPath(FichierPrefDetecteurs,NomDetecteurs);
	SambaAjoutePrefPath(FichierPrefRepartiteurs,NomRepartiteurs);
	SambaAjoutePrefPath(FichierPrefCablage,NomCablage);
	SambaAjoutePrefPath(FichierPrefNumeriseurs,NomNumeriseurs);
	SambaAjoutePrefPath(FichierPrefProcedures,NomProcedures); RepertoireTermine(FichierPrefProcedures); /* c'est un repertoire */
	SambaAjouteRacine(FichierScriptStart,FichierPrefProcedures,NomScriptStart);
	SambaAjouteRacine(FichierScriptStop,FichierPrefProcedures,NomScriptStop);
	SambaAjouteRacine(FichierEntretienStart,FichierPrefProcedures,NomEntretienStart);
	SambaAjouteRacine(FichierEntretienStop,FichierPrefProcedures,NomEntretienStop);
	SambaAjouteRacine(FichierRegenStart,FichierPrefProcedures,NomRegenStart);
	SambaAjouteRacine(FichierRegenStop,FichierPrefProcedures,NomRegenStop);
	SambaAjoutePrefPath(FichierPrefFiltres,NomFiltres);
	SambaAjoutePrefPath(FichierPrefSequences,NomSequences);
	SambaAjoutePrefPath(FichierPrefRegulEvt,NomRegulEvt);
	SambaAjoutePrefPath(FichierPrefVi,NomVi);
	SambaAjoutePrefPath(FichierPrefEvtUnite,NomEvtUnite);
	SambaAjoutePrefPath(FichierPrefLecture,NomLecture);
	SambaAjoutePrefPath(FichierPrefMonit,NomMonit);
	SambaAjoutePrefPath(FichierPrefAutom,NomAutom);
	SambaAjoutePrefPath(FichierPrefCalcul,NomCalcul);
	SambaAjoutePrefPath(FichierPrefFinesses,NomFinesses);
	SambaAjoutePrefPath(FichierPrefBanc,NomBanc);
	SambaAjoutePrefPath(FichierPrefBasic,NomBasic);
	SambaAjoutePrefPath(FichierPrefVerif,NomVerif);
	SambaAjoutePrefPath(FichierPrefExports,NomExports);

	SambaAjoutePrefPath(FichierRunPrecedent,NomRuns);
	SambaAjoutePrefPath(FichierRunEnvir,NomEnvironnement);
	SambaAjoutePrefPath(FichierRunCaract,NomCaract);
	SambaAjoutePrefPath(FichierEtatElec,NomEtatElec);
	SambaAjoutePrefPath(FichierCaldrRef,CalendrierRef);
}
/* ========================================================================== */
int SambaLitSimu() {
	int nb_erreurs; FILE *f;
	int j,simu,voie;

	if((f = fopen(FichierPrefSimu,"r"))) {
		SambaLogDef("= Lecture de la simulation","dans",FichierPrefSimu);
		ArgFromFile(f,SimuDesc,0);
		fclose(f);
		// ArgPrint("*",SimuDesc);
	} else { /* Fichier de simulation inaccessible */
		nb_erreurs = 0;
		printf("* Pas de simulation definie via '%s' (%s)\n",FichierPrefSimu,strerror(errno));
		// simulation pas obligatoire: nb_erreurs++;
		Simu[0].nom[0] = '\0';
		Simu[0].voie[0] = 0;
		Simu[0].nbvoies = 1;
		Simu[0].ldb = 0.0;
		Simu[0].bruit = 5.0;
		Simu[0].freq = 1.0;
		Simu[0].pic = 1000.0;
		Simu[0].reso = 10.0;
		SimuNb = 1;
		SambaSauveSimu();
	}
	SimuModifiees = 0;
	for(simu=0; simu<SimuNb; simu++) {
		if(Simu[simu].nom[0] == '\0') {
			if(Simu[simu].nbvoies && (voie = Simu[simu].voie[0]) >= 0) strncpy(Simu[simu].nom,VoieManip[voie].nom,MODL_NOM);
			else sprintf(Simu[simu].nom,"Simu#%d",simu+1);
		}
		for(j=0; j<Simu[simu].nbvoies; j++) if((voie = Simu[simu].voie[j]) >= 0) VoieManip[voie].simu = simu;
		Simu[simu].periode = 1.4 / EnSecondes / Simu[simu].freq; // temporaire, Echantillonnage peut encore changer
		Simu[simu].mode_manu = Simu[simu].mode_auto = 0;
		Simu[simu].panneau.avant = 0;
		Simu[simu].panneau.message[0] = '\0';
	}
	Simu[simu].nom[0] = '\0';
	for(simu=0; simu<SimuNb; simu++) {
		printf("  | Generateur '%s': %d voie%s (",Simu[simu].nom,Accord1s(Simu[simu].nbvoies));
		if(Simu[simu].nbvoies > 5) printf("\n  | . ");
		for(j=0; j<Simu[simu].nbvoies; ) {
			printf("%s",VoieManip[Simu[simu].voie[j++]].nom);
			if(j < Simu[simu].nbvoies) printf(",%s",(j%5)?" ":"\n  | . ");
			else printf(")\n");
		}
	}
	return(nb_erreurs);
}
/* ========================================================================== */
char SambaSauveSimu() {
	FILE *f;
	
	RepertoireCreeRacine(FichierPrefSimu);
	if((f = fopen(FichierPrefSimu,"w"))) {
		printf("\n");
		SambaLogDef("= Ecriture de la simulation","dans",FichierPrefSimu);
		fprintf(f,"# Modes de simulation\n");
		ArgAppend(f,0,SimuDesc);
		fclose(f);
		SimuModifiees = 0;
	} else printf("! Fichier de simulation inaccessible: '%s' (%s)\n",FichierPrefSimu,strerror(errno));
	return((f != 0));
}
/* ========================================================================== */
static int SambaLitReseau() {
	int nb_erreurs;
	FILE *f; // char ligne[256],*r; char *nom;
	int ptn,sat,moi,rep,i,j; char c_est_moi,pas_trouve; char nom_mode[16];
	unsigned int code_ip_acquis;

	nb_erreurs = 0;
	if((f = fopen(FichierPrefReseau,"r"))) {
		SambaLogDef("= Lecture du reseau","dans",FichierPrefReseau);
		pas_trouve = 0;
		ArgFromFile(f,ReseauDesc,0);
		fclose(f);
		// ArgPrint("*",ReseauDesc);
	} else { /* Fichier du reseau inaccessible */
		printf("* Pas de reseau defini via       '%s' (%s)\n",FichierPrefReseau,strerror(errno));
		pas_trouve = 1;
		nb_erreurs++;
	}
	if(CompteRendu.reseau) printf("  . Cet ordinateur: %s (%08X)\n",AdresseIP,CodeIpLocal); moi = AcquisNb;
	for(sat=0; sat<AcquisNb; sat++) {
		code_ip_acquis = HostInt(Acquis[sat].adrs);
		c_est_moi = (code_ip_acquis == CodeIpLocal); if(c_est_moi) moi = sat;
		if(CompteRendu.reseau) {
			printf("  . Acquisition #%d: %s (%08X) = \"%s\"%s\n",sat+1,Acquis[sat].adrs,code_ip_acquis,AcquisListe[sat],
				   c_est_moi? " (cet ordinateur)": "");
			for(i=0; i<Acquis[sat].nbrep; i++) printf("    | lit le repartiteur %s\n",Repart[Acquis[sat].rep[i]].nom);
		}
	}
	if(CompteRendu.reseau) {
		for(ptn=0; ptn<PartitNb; ptn++) {
			printf("  . Partition #%d: maitre %s (%08X)",ptn+1,Partit[ptn].mtr,HostInt(Partit[ptn].mtr));
			for(sat=0; sat<Partit[ptn].nbsat; sat++) {
				printf(", satellite %s=%s (%08X)\n",AcquisListe[Partit[ptn].sat[sat]],Acquis[Partit[ptn].sat[sat]].adrs,HostInt(Acquis[Partit[ptn].sat[sat]].adrs));
			}
			printf("\n");
		}
		ArgKeyGetText(SAMBA_MODE_CLES,SambaMode,nom_mode,16);
		printf("  . Mode par defaut: %d (%s)\n",SambaMode,nom_mode);
	}

	for(ptn=0; ptn<PartitNb; ptn++) {
		if(CodeIpLocal == HostInt(Partit[ptn].mtr)) { if(SambaMode == SAMBA_AUTOGERE) SambaMode = SAMBA_MAITRE; break; }
		for(sat=0; sat<Partit[ptn].nbsat; sat++) if(CodeIpLocal == HostInt(Acquis[Partit[ptn].sat[sat]].adrs)) {
			if(SambaMode == SAMBA_AUTOGERE) SambaMode = SAMBA_SATELLITE; break;
		}
		if(sat < Partit[ptn].nbsat) break;
	}
	if(ptn < PartitNb) PartitLocale = ptn; else PartitLocale = -1;
	if(SambaMode == SAMBA_AUTOGERE) SambaMode = SAMBA_MONO;
	if(CompteRendu.reseau) {
		ArgKeyGetText(SAMBA_MODE_CLES,SambaMode,nom_mode,16);
		printf("  . Mode utilise: %d (%s)\n",SambaMode,nom_mode);
	}

#ifdef C_ETAIT_AVANT
	if((SambaMode == SAMBA_MONO) || (SambaMode == SAMBA_SATELLITE)) {
		if(!AcquisNb || ((AcquisNb == 1) && !strcmp(Acquis[0].adrs,"."))) sat = 0;
		else for(sat=0; sat<AcquisNb; sat++) if(CodeIpLocal == HostInt(Acquis[sat].adrs)) break;
		AcquisLocale = sat;
		if(AcquisLocale >= AcquisNb) {
			AcquisNb++;
			strcpy(Acquis[AcquisLocale].code,"local");
			strcpy(Acquis[AcquisLocale].adrs,NomExterne);
			Acquis[AcquisLocale].runs[0] = '_';
			Acquis[AcquisLocale].nbrep = 0;
			for(rep=0; rep<RepartNb; rep++) {
				for(sat=0; sat<AcquisNb; sat++) if(sat != AcquisLocale) {
					for(i=0; i<Acquis[sat].nbrep; i++) if(Acquis[sat].rep[i] == rep) break;
					if(i < Acquis[sat].nbrep) break;
				}
				if((sat >= AcquisNb) && (Acquis[AcquisLocale].nbrep < ACQUIS_MAXREP)) Acquis[AcquisLocale].rep[(Acquis[AcquisLocale].nbrep)++] = rep;
			}
		}
	} else {
		AcquisLocale = AcquisNb++;
		strcpy(Acquis[AcquisLocale].code,"maitre");
		strcpy(Acquis[AcquisLocale].adrs,NomExterne);
		Acquis[AcquisLocale].runs[0] = 'm';
		Acquis[AcquisLocale].nbrep = 0;
		for(rep=0; rep<RepartNb; rep++) if(Acquis[AcquisLocale].nbrep < ACQUIS_MAXREP) Acquis[AcquisLocale].rep[(Acquis[AcquisLocale].nbrep)++] = rep;
	}
#else  /* !C_ETAIT_AVANT */
	if(SambaMode != SAMBA_DISTANT) {
		if(!AcquisNb || ((AcquisNb == 1) && !strcmp(Acquis[0].adrs,"."))) AcquisLocale = 0;
		else AcquisLocale = moi; // for(sat=0; sat<AcquisNb; sat++) if(CodeIpLocal == HostInt(Acquis[sat].adrs)) break; AcquisLocale = sat;
		if(AcquisLocale >= AcquisNb) {
			AcquisNb++;
			strcpy(Acquis[AcquisLocale].code,(SambaMode == SAMBA_MAITRE)? "maitre": "local");
			strcpy(Acquis[AcquisLocale].adrs,NomExterne);
			Acquis[AcquisLocale].runs[0] = '_';
			Acquis[AcquisLocale].nbrep = 0;
			for(rep=0; rep<RepartNb; rep++) {
				for(sat=0; sat<AcquisNb; sat++) if(sat != AcquisLocale) {
					for(i=0; i<Acquis[sat].nbrep; i++) if(Acquis[sat].rep[i] == rep) break;
					if(i < Acquis[sat].nbrep) break;
				}
				if((sat >= AcquisNb) && (Acquis[AcquisLocale].nbrep < ACQUIS_MAXREP)) Acquis[AcquisLocale].rep[(Acquis[AcquisLocale].nbrep)++] = rep;
			}
			printf("  . Acquisition locale non declaree, ajoutee comme suit:\n");
			memcpy(&AcquisLue,&(Acquis[AcquisLocale]),sizeof(TypeAcquis));
			ArgPrint("*",AcquisDesc);
		}
	}
#endif /* !C_ETAIT_AVANT */

	if(AcquisLocale >= 0) printf("  . Acquisition locale: '%s' (code de run: '%c')\n",
		Acquis[AcquisLocale].code,Acquis[AcquisLocale].runs[0]);
	else printf("  * Pas d'acquisition locale\n");
	if((AcquisNb < 2) && !PartitNb) printf("  . Acquisition independante\n");
	else {
		if(AcquisNb > 1) {
			printf("  . Acquisition en reseau\n");
			printf("    | Ferme d'acquisition: %d noeuds (",AcquisNb);
			for(sat=0; sat<AcquisNb; sat++) printf("%s%s",sat?", ":"",Acquis[sat].code);
			printf(")\n");
		}
		if(PartitNb) {
			if(PartitNb == 1) printf("  . Partition controlee par %s: ",Partit[0].mtr);
			else printf("  . %d partitions: ",PartitNb);
			for(ptn=0; ptn<PartitNb; ptn++) {
				if(PartitNb > 1) printf("      | Controlee par %s: ",Partit[ptn].mtr);
				for(j=0; j<Partit[ptn].nbsat; j++) printf("%s%s",j?", ":"{ ",Acquis[Partit[ptn].sat[j]].code);
				printf(" }\n");
			}
		}
		if(PartitLocale >= 0) printf("  . Partition locale: %s\n",Partit[PartitLocale].mtr);
	}
	SambaMaitre = ((SambaMode == SAMBA_MAITRE) && (PartitLocale >= 0));
	SambaSat = ((SambaMode == SAMBA_SATELLITE) && (PartitLocale >= 0));

	/* Assignation du role de l'application en cours (central, satellite, distant) */
	switch(SambaMode) {
		case SAMBA_SATELLITE: strcpy(NomTache,NOM_SATELLITE); PasMaitre = 1; break;
		case SAMBA_MAITRE:    strcpy(NomTache,NOM_SUPERVISEUR); PasMaitre = 0; break;
		case SAMBA_DISTANT:   strcpy(NomTache,NOM_DISTANT); PasMaitre = 1; break;
		default:              strcpy(NomTache,"Systeme"); PasMaitre = 0; break;
	}
	ArgKeyGetText(SAMBA_MODE_CLES,SambaMode,nom_mode,16);
	printf("    . %s %s",NomTache,nom_mode);
	if(SambaMode != SAMBA_MONO) printf(" sur le port %d",PortEcoute);
	printf("\n");
#ifndef CODE_WARRIOR_VSN
	if(Starter[0] == '\0') strcpy(Starter,"neant");
	if(!strcmp(Starter,"neant")) { if(CompteRendu.reseau) printf("    . Autosynchronisation non parametree.\n"); }
	else {
		Demarreur = !strcmp(Acquis[AcquisLocale].code,Starter);
		if(CompteRendu.reseau) printf("    . Autosynchronisation possible sur %s (%s systeme).\n",Starter,Demarreur?"ce":"un autre");
	}
#else
	strcpy(Starter,"neant");
#endif

#ifdef ETAT_RESEAU
	Acquis[AcquisLocale].id = IdLocal;
#endif
	strcpy(SambaCodeHote,Acquis[AcquisLocale].code);
	strcpy(SambaAdrsHote,Acquis[AcquisLocale].adrs);
	sprintf(FichierCatalogue,"%sCatalogue_%s",CtlgPath,Acquis[AcquisLocale].code);
	SambaLogDef(". Catalogue des runs","dans",FichierCatalogue);

	if(pas_trouve && CreationFichiers) { if(!SambaSauveReseau()) nb_erreurs++; }

	for(i=0; i<Acquis[AcquisLocale].nbrep; i++) {
		for(sat=0; sat<AcquisLocale; sat++) {
			for(j=0; j<Acquis[sat].nbrep; j++) if(Acquis[sat].rep[j] == Acquis[AcquisLocale].rep[i]) Repart[Acquis[AcquisLocale].rep[i]].recopie = 1;
		}
	}

	return(nb_erreurs);
}
/* ========================================================================== */
char SambaSauveReseau() {
	FILE *f;

	if(SambaMode == SAMBA_MAITRE) --AcquisNb;
	RepertoireCreeRacine(FichierPrefReseau);
	if((f = fopen(FichierPrefReseau,"w"))) {
		printf("\n");
		SambaLogDef("= Ecriture du reseau","dans",FichierPrefReseau);
		fprintf(f,"# Reseau participant a l'acquisition\n");
		ArgAppend(f,0,ReseauDesc);
		fclose(f);
	} else printf("! Fichier du reseau inaccessible: '%s' (%s)\n",FichierPrefReseau,strerror(errno));
	return((f != 0));
}
/* ========================================================================== */
void SambaModifiePerimetre() {
	int rep,sat,i; char change,sauve; char *boutons[3];

	change = 0;
	for(rep=0; rep<RepartNb; rep++) {
#ifdef PERIMETRES_DISJOINTS
		char retire_local,ajoute_local;
		for(sat=0; sat<AcquisNb; sat++) {
			for(i=0; i<Acquis[sat].nbrep; i++) if(Acquis[sat].rep[i] == rep) break;
			if(i < Acquis[sat].nbrep) break;
		}
		if(sat < AcquisNb) {
			ajoute_local = (Repart[rep].local && (sat != AcquisLocale));
			retire_local = (!Repart[rep].local && (sat == AcquisLocale));
			if(ajoute_local || retire_local) {
				Acquis[sat].nbrep -= 1;
				for( ; i<Acquis[sat].nbrep; i++) Acquis[sat].rep[i] = Acquis[sat].rep[i+1];
			}
		} else ajoute_local = 1; /* trouve nulle part: mis en local a tout hasard */
		if(ajoute_local && (Acquis[AcquisLocale].nbrep < ACQUIS_MAXREP)) Acquis[AcquisLocale].rep[(Acquis[AcquisLocale].nbrep)++] = rep;
#else  /* PERIMETRES_DISJOINTS */
		for(i=0; i<Acquis[AcquisLocale].nbrep; i++) if(Acquis[AcquisLocale].rep[i] == rep) break;
		if(i < Acquis[AcquisLocale].nbrep) {
			if(!Repart[rep].local) {
				Acquis[AcquisLocale].nbrep -= 1; for( ; i<Acquis[AcquisLocale].nbrep; i++) Acquis[AcquisLocale].rep[i] = Acquis[AcquisLocale].rep[i+1];
				change = 1;
			}
		} else {
			if(Repart[rep].local) {
				if(Acquis[AcquisLocale].nbrep < ACQUIS_MAXREP) {
					Acquis[AcquisLocale].rep[(Acquis[AcquisLocale].nbrep)++] = rep; change = 1;
				} else {
					OpiumError("Trop de repartiteurs locaux (maxi: %d)",ACQUIS_MAXREP); break;
				}
			}
		}
#endif /* PERIMETRES_DISJOINTS */
	}
	for(sat=0; sat<AcquisNb; sat++) {
		for(i=0; i<Acquis[sat].nbrep; i++) Repart[Acquis[sat].rep[i]].sat = sat;
	}
	if(change) {
		boutons[0] = "temporaire"; boutons[1] = "a sauver"; boutons[2] = "\0"; /* ce dernier pour DicoListeLocale() */
		if(OpiumChoix(2,boutons,"Ce nouveau perimetre est",&sauve)) {
			ArgPrint(FichierPrefReseau,ReseauDesc);
			printf("* Nouveau perimetre sauve dans %s\n",FichierPrefReseau);
		}
	}
}
/* ========================================================================== */
void SambaEnvirModlLit() {
	RepertoireCreeRacine(FichierModlEnvir);
	SambaLogDef(". Lecture du modele d'environnement","dans",FichierModlEnvir);
	if(ArgScan(FichierModlEnvir,EnvirModlDesc) == 0) {
		printf("    . Environnement cree par defaut\n");
		EnvirVarNb = 0;
		strncpy(EnvirVar[EnvirVarNb].nom,"Type",ENVIR_NOM);
		EnvirVar[EnvirVarNb].type = ARG_TYPE_KEY;
		strncpy(EnvirVar[EnvirVarNb].mot_cles,RunCategCles,ENVIR_CLES);
		strncpy(EnvirVar[EnvirVarNb].explics,"Type de run",ENVIR_EXPLICS);
		EnvirVarNb++;
		strncpy(EnvirVar[EnvirVarNb].nom,"Source",ENVIR_NOM);
		EnvirVar[EnvirVarNb].type = ARG_TYPE_KEY;
		strncpy(EnvirVar[EnvirVarNb].mot_cles,"neant/Fe55",ENVIR_CLES);
		strncpy(EnvirVar[EnvirVarNb].explics,"Source de calibration en place",ENVIR_EXPLICS);
		EnvirVarNb++;
		strncpy(EnvirVar[EnvirVarNb].nom,"Blindage",ENVIR_NOM);
		EnvirVar[EnvirVarNb].type = ARG_TYPE_KEY;
		strncpy(EnvirVar[EnvirVarNb].mot_cles,"absent/ouvert/ferme",ENVIR_CLES);
		strncpy(EnvirVar[EnvirVarNb].explics,"Configuration du blindage",ENVIR_EXPLICS);
		EnvirVarNb++;
		//	strncpy(EnvirVar[EnvirVarNb].nom,"Environnement",ENVIR_NOM);
		//	EnvirVar[EnvirVarNb].type = ARG_TYPE_KEY;
		//	strncpy(EnvirVar[EnvirVarNb].mot_cles,RunEnvir,ENVIR_CLES);
		//	strncpy(EnvirVar[EnvirVarNb].explics,"Environnement du run",ENVIR_EXPLICS);
		//	EnvirVarNb++;
		ArgPrint(FichierModlEnvir,EnvirModlDesc);
		SambaEnvirCreeDesc(0);
	}
	if(ImprimeConfigs) {
		printf("#\n### Fichier: '%s' ###\n",FichierModlEnvir);
		ArgPrint("*",EnvirModlDesc);
	}
}
/* ========================================================================== */
static void SambaLitMateriel() {
	int sat,nb,i,j,k;
	// int bolo,cap,vm,vp,vl;
#ifdef MODULES_RESEAU
	char commande[80];
#else
#ifndef CODE_WARRIOR_VSN
	char nom_ip[HOTE_NOM];
#endif
#endif

#ifdef SETUP_RESEAU
	if((SambaMode == SAMBA_DISTANT)) {
		if(!CarlaDataRequest(AcqCible,IdBolos)) 
			printf("! Recuperation des detecteurs sur %s: %s\n",AcqCible,CarlaDataError());
		if(!CarlaDataRequest(AcqCible,IdVoies))
			printf("! Recuperation des voies sur %s: %s\n",AcqCible,CarlaDataError());
	}
#endif

	/* Modeles et chassis */
	SambaLogDef("= Lecture des modeles","dans",FichierPrefModeles);
	if(ArgScan(FichierPrefModeles,ModelesDirDesc) == 0) {
		if(CreationFichiers) ArgPrint(FichierPrefModeles,ModelesDirDesc);
		else OpiumError("Lecture de '%s' impossible",FichierPrefModeles);
	}
	SambaAjoutePrefPath(FichierModlChassis,NomModlChassis);
	SambaAjoutePrefPath(FichierModlNumeriseurs,NomModlNumeriseurs);
	SambaAjoutePrefPath(FichierModlDetecteurs,NomModlDetecteurs);
	SambaAjoutePrefPath(FichierModlCablage,NomModlCablage);
	SambaAjoutePrefPath(FichierModlRepartiteurs,NomModlRepartiteurs);
	SambaAjoutePrefPath(FichierModlEnvir,NomModlEnvir);
	
	OpiumTableFichier(OpiumTablePtr(SambaTableModeles,"Physique"),FichierModlDetecteurs,DetectionDesc);
	OpiumTableFichier(OpiumTablePtr(SambaTableModeles,"Voies"),FichierModlDetecteurs,DetectionDesc);
	OpiumTableFichier(OpiumTablePtr(SambaTableModeles,"Detecteurs"),FichierModlDetecteurs,DetectionDesc);
//	OpiumTableFichier(OpiumTablePtr(SambaTableModeles,"Cablages"),FichierModlCablage,CablageModlDesc);
	OpiumTableFichier(OpiumTablePtr(SambaTableModeles,"ADC"),FichierModlNumeriseurs,NumerModlDesc);
	OpiumTableFichier(OpiumTablePtr(SambaTableModeles,"Numeriseurs"),FichierModlNumeriseurs,NumerModlDesc);
	OpiumTableFichier(OpiumTablePtr(SambaTableModeles,"Repartiteurs"),FichierModlRepartiteurs,RepertModlDesc);
	OpiumTableFichier(OpiumTablePtr(SambaTableModeles,"Simulations"),FichierPrefSimu,SimuDesc);
	OpiumTableFichier(OpiumTablePtr(SambaTableModeles,"Environnement"),FichierModlEnvir,EnvirModlDesc);

	SambaEnvirModlLit();
	
	SambaLogDef(". Lecture des modeles de chassis","dans",FichierModlChassis);
	ArgScan(FichierModlChassis,ChassisDesc);
	if(ChassisNumerDim > 2) ChassisNumerDim = 2; /* maxi 2 positions prevues dans CablageEncodeConnecteur */
	if(ChassisDetecDim == 0) {
		ChassisDetec[0].nb = 4;  ChassisDetec[0].codage = CHASSIS_CODE_a; ChassisDetec[0].cles = 0;
		ChassisDetec[1].nb = 12; ChassisDetec[1].codage = CHASSIS_CODE_1; ChassisDetec[1].cles = 0;
		ChassisDetec[2].nb = 4;  ChassisDetec[2].codage = CHASSIS_CODE_z; ChassisDetec[2].cles = 0;
		ChassisDetecDim = 3;
		ArgPrint(FichierModlChassis,ChassisDesc);
	}
	for(i=0; i<ChassisDetecDim; i++) if(ChassisDetec[i].nb > 1) break;
	ChassisDetecUnique = (i >= ChassisDetecDim);
	// printf("    . Premier chassis detecteur multiple: #%d/%d (Chassis detecteurs %s)\n",i,ChassisDetecDim,ChassisDetecUnique? "en blanc": "etiquete");
	for(i=ChassisDetecDim; i<CHASSIS_DIMMAX; i++) {
		ChassisDetec[i].nb = 1; ChassisDetec[1].codage = CHASSIS_CODE_1; ChassisDetec[1].cles = 0;
	}
	if(ChassisNumerDim == 0) {
	//	ChassisNumer[0].nb = 4;  ChassisNumer[0].codage = CHASSIS_CODE_RANG; ChassisNumer[0].dim = 28;
		ChassisNumer[0].nb = 4;  ChassisNumer[0].codage = CHASSIS_CODE_NOMS; ChassisNumer[0].cles = "hg/bg/hd/bd";
		ChassisNumer[1].nb = 28; ChassisNumer[1].codage = CHASSIS_CODE_1; ChassisNumer[1].cles = 0;
		ChassisNumerDim = 2;
		ArgPrint(FichierModlChassis,ChassisDesc);
	}
	for(i=0; i<ChassisNumerDim; i++) if(ChassisNumer[i].nb > 1) break;
	ChassisNumerUnique = (i >= ChassisNumerDim);
	// printf("    . Premier chassis numeriseur multiple: #%d/%d (Chassis numeriseurs %s)\n",i,ChassisNumerDim,ChassisNumerUnique? "en blanc": "etiquete");
	for(i=ChassisNumerDim; i<CHASSIS_DIMMAX; i++) {
		ChassisNumer[i].nb = 1; ChassisNumer[1].codage = CHASSIS_CODE_1; ChassisNumer[1].cles = 0;
	}
	NumeriseurStructInit();   NumeriseursModlLit(0);
	DetecteursStructInit();   DetecteursModlLit(0,0);
	CablageStructInit();      CablageModlLit(0);
	RepartiteursStructInit(); RepartiteurModlLit(0);
	NumeriseursModlEpilogue();
	DetecteursModlEpilogue();
	if(CompteRendu.modeles) {
		printf("  . Examen des modeles de chassis\n");
		if(ChassisNumerUnique) printf("    . Un seul emplacement de numeriseur: codage inutile\n");
		else SambaImprimeSupport("Numeriseurs",ChassisNumer,ChassisNumerDim);
		if(ChassisDetecUnique) printf("    . Un seul emplacement de detecteur: codage inutile\n");
		else SambaImprimeSupport("Detecteurs",ChassisDetec,ChassisDetecDim);
	}
	printf("\n");

	/* Materiel d'acquisition */
	BoloNb = 0; MediaListeInit(SambaMedia,MEDIA_MAX);
	nb = 0;
	nb += NumeriseursLit(FichierPrefNumeriseurs);
	nb += RepartiteursLit(FichierPrefRepartiteurs);
	nb += SambaLitReseau();
	nb += CablageLit(FichierPrefCablage);
	nb += DetecteursLit(FichierPrefDetecteurs,LectSurFichier);
	nb += SambaLitSimu();
	if((nb > 0) && !InstalleSamba) {
		if(OpiumCheck(L_("%d erreur%s dans la definition du montage (voir journal)"),Accord1s(nb))) system("open -a Terminal");
	}

	if(SambaMode == SAMBA_SATELLITE) {
		SambaOuvreOreillesSat();
		OpiumPeriodicFctn(SambaEcouteMaitre);
	}
	if(PartitLocale >= 0) {
		for(j=0; j<Partit[PartitLocale].nbsat; j++) {
			sat = Partit[PartitLocale].sat[j];
			if((SambaMode == SAMBA_MAITRE) || (sat != AcquisLocale))
				Partit[PartitLocale].exter[j] = SambaConnecteSat(sat);
			else Partit[PartitLocale].exter[j] = 0;
			if(SambaMode == SAMBA_MAITRE) {
				// LectStampMini = (int64)0x123456789ABCDEF0; SambaEcritSatLong(sat,SMB_E,LectStampMini);
				SambaOuvreOreillesMaitre(sat);
			}
		}
		for(sat=0; sat<AcquisNb; sat++) if(!SambaMaitre || (sat != AcquisLocale)) {
			for(i=0; i<Acquis[sat].nbrep; i++) Repart[Acquis[sat].rep[i]].sat = sat;
		}
	}
	RepartiteursLocalise();
	LectStampMini = 0;
	if(CompteRendu.cablage.repart) RepartiteursImprimeNumeriseurs();

	SambaLogDef("Definition des exportations lue",0,FichierPrefExports);
	if(ExportLit()) for(k=0; k<ExportPackNb; k++)
		printf("  . Pack %s: %d variable%s exportable%s dans %s\n",ExportPack[k].nom,Accord2s(ExportPack[k].nbvars),ExportPack[k].info_nom);
	else printf("  . Pas d'exportation demandee\n");

	SambaLogDef("Lecture des media",0,FichierPrefMedia);
	if(!MediaLit(FichierPrefMedia,CreationFichiers)) {
		OpiumFail("Lecture de '%s' impossible",FichierPrefMedia);
	}
	MediaListeAjouteLus(SambaMedia);
	if(CompteRendu.media.liste) MediaListeDump(SambaMedia,"  * Liste complete des media");

	BancInit();   if(CompteRendu.initialisation) printf("  . Initialisation du banc de tests terminee\n");
	AutomInit();  if(CompteRendu.initialisation) printf("  . Initialisation du support terminee\n\n");
	BasicInit();  if(CompteRendu.initialisation) printf("  . Initialisation des acces basiques terminee\n");

	SambaLogDef("Recherche des procedures",0,FichierPrefProcedures);
	RepertoireListeCree(0,FichierPrefProcedures,SambaProcedure,SAMBA_PROC_MAX,&SambaProcsNb);
	printf("  . Trouve %d procedure%s\n",Accord1s(SambaProcsNb));
	if(SambaProcsNb) {
		pScriptChoix = PanelCreate(2);
		PanelList(pScriptChoix,"Script",SambaProcedure,&SambaProcsNum,24);
		PanelKeyB(pScriptChoix,"Selection",L_("inchangee/modifier"),&SambaNumerSelect,10);
		PanelSupport(pScriptChoix,WND_CREUX);
		PanelBoutonText(pScriptChoix,PNL_APPLY,"Executer");
		PanelOnApply(pScriptChoix,SambaScriptExec,0);
//		PRINT_OPIUM_ADRS(pScriptChoix);
//		PRINT_OPIUM_ADRS(pScriptEtat);
//		PRINT_OPIUM_ADRS(pScriptCntl);
		bScriptCntl = BoardCreate();
		BoardAddPanel(bScriptCntl,pScriptChoix,Dx,Dy,0);
		BoardAddPanel(bScriptCntl,pScriptEtat,OPIUM_EN_DESSOUS_DE pScriptChoix->cdr);
		BoardAddPanel(bScriptCntl,pScriptCntl,OPIUM_A_DROITE_DE pScriptEtat->cdr);
	} else bScriptCntl = 0;
	
	if(RepertoireAbsent("logs")) sprintf(SambaJournal,"%s%c.log",DateCourante,Acquis[AcquisLocale].runs[0]);
	else sprintf(SambaJournal,"logs/%s%c.log",DateCourante,Acquis[AcquisLocale].runs[0]);
	
#ifdef SETUP_RESEAU
	if(SambaMode == SAMBA_MAITRE) {
	#ifdef PARMS_RESEAU
		SambaSetup->BoloNb = BoloNb; SambaSetup->VoiesNb = VoiesNb;
	#endif
		CarlaDataUpdate(IdBolos); CarlaDataUpdate(IdVoies);
	}
#endif

	/* initialiser ceci APRES DetecteursLit() et, SURTOUT, apres ReseauLit */	
	CommandesNb = 0; // CmdeNum = 0; CmdeAdrs = 0; CmdesASauver = 0;
	
	LectSurFichier = (SrceType > SRC_REEL);
	LectArchFile = 0; LectArchPath = -1; LectSurFichierDejaVu = 0; LectArchData = 0;
	LectArchData = 0; LectTrancheRelue = 0;
#ifdef OBSOLETE
	LectArchPrepare(0);
#endif

#ifdef MODULES_RESEAU
	if(SambaMode == SAMBA_MAITRE) {
		int cols,i;
		printf("  . %d systeme%s d'acquisition declare%s\n",Accord2s(AcquisNb));
		if(AcquisNb > 1) {
			cols = 1;
			pAcquisEtat = PanelCreate(cols * AcquisNb);
			PanelColumns(pAcquisEtat,cols);
			PanelRemark(pAcquisEtat,"Systeme");
			for(sat=1; sat<AcquisNb; sat++) {
			#ifdef ETAT_RESEAU
				Acquis[sat].etat = (TypeAcquisEtat *)CarlaDataShareDyn(Acquis[sat].code,&(Acquis[sat].id),sizeof(TypeAcquisEtat),CARLA_NOINIT,CARLA_TEMPO);
			#endif
				i = PanelListB(pAcquisEtat,Acquis[sat].code,AcquisTexte,&(Acquis[sat].etat.status),16);
				PanelItemSelect(pAcquisEtat,i,0);
				PanelItemColors(pAcquisEtat,i,SambaBleuRougeOrangeJauneVert,5);
			#ifdef XCODE
				sprintf(commande,"ping -c 2 -noQq %s >/dev/null",Acquis[sat].adrs);
			#else
				sprintf(commande,"ping -c 2 -nq %s >/dev/null",Acquis[sat].adrs);
			#endif
				if(system(commande) == 0) Acquis[sat].etat.status = SATELLITE_ARRETE;
				else {
					printf("  ! %s (%s) ne repond pas: machine ignoree\n",Acquis[sat].code,Acquis[sat].adrs);
					Acquis[sat].etat.status = SATELLITE_IGNORE;
				}
			}
			/* on a alors cols = 2;
			 PanelRemark(pAcquisEtat,"KgJ");
			 for(sat=1; sat<AcquisNb; sat++) {
				 PanelItemSelect(pAcquisEtat,PanelFloat(pAcquisEtat,0,&(Acquis[sat].etat.KgJ),0);
			}
			*/
			PanelTitle(pAcquisEtat,"Etat acquisition");
		}
	#ifdef MSGS_RESEAU
		printf("  * Verification des communications (%d):\n",AcquisNb);
		for(sat=0; sat<AcquisNb; sat++) if(Acquis[sat].etat.status > SATELLITE_IGNORE) {
			if(!CarlaMsgLink(&(Acquis[sat].bal),NOM_SATELLITE,Acquis[sat].adrs)) {
				printf("    ! Connexion avec %s impossible\n",Acquis[sat].code);
				OpiumError(CarlaDataError());
				Acquis[sat].etat.status = SATELLITE_ABSENT;
			} else printf("    . Connexion avec %s etablie\n",Acquis[sat].code);
		}
	#endif
	}
#else  /* MODULES_RESEAU */
#ifndef CODE_WARRIOR_VSN
	if(strcmp(Starter,"neant")) {
		printf("= Etablissement des communications pour l'autosynchronisation\n");
		if(Demarreur) {
			for(sat=1; sat<AcquisNb; sat++) {
				printf("  . Socket de synchronisation en ecriture sur %s:%d\n",Acquis[sat].adrs,PortEcoute);
				if(SocketInit(Acquis[sat].adrs,PortEcoute,&(Acquis[sat].synchro.skt)) <= 0) {
					OpiumError("Adresse '%s' incorrecte, synchro impossible",Acquis[sat].adrs);
					printf("    ! Synchro sur %s impossible (%s)\n",Acquis[sat].code,strerror(errno));
					Acquis[sat].synchro.path = -1;
				} else {
					if((Acquis[sat].synchro.path = SocketConnectUDP(&(Acquis[sat].synchro.skt))) < 0) {
						OpiumError("Connexion UDP sur %s impossible, synchro abandonnee",Acquis[sat].adrs);
						printf("    ! Synchro sur %s impossible (%s)\n",Acquis[sat].code,strerror(errno));
					}
				}
			}
		} else {
			sprintf(nom_ip,"EDWACQ-%s.local",Starter);
			printf("  . Socket de synchronisation en lecture sur %s:%d\n",nom_ip,PortEcoute);
			if((Acquis[AcquisLocale].synchro.path = SocketUDP(PortEcoute)) < 0) {
				OpiumError("Ouverture UDP sur le port %d impossible, synchro abandonnee",PortEcoute);
				printf("    ! Synchro sur %s impossible (%s)\n",Starter,strerror(errno));
				strcpy(Starter,"neant");
			} else {
				if(fcntl(Acquis[AcquisLocale].synchro.path,F_SETFL,O_NONBLOCK) == -1) {
					printf("    ! Cette socket est bloquante (%s)\n",strerror(errno));
				}
			}
		}
	}
#endif
#endif /* MODULES_RESEAU */

	SambaConstruitMateriel();

#ifdef MENU_BARRE
	// WndMenuDebug(1);
	MenuBarreCreate(mSambaBarre);
#endif

}
/* ========================================================================== */
void SambaConstruitMateriel() {
	int vm,vp,vl,bolo,cap; int i,j;

	printf("\n= Determination du perimetre\n");
	RepartiteurAppliquePerimetre();

	printf("\n= Initialisations complementaires\n");
	for(vm=0; vm<ModeleVoieNb; vm++) ModeleVoie[vm].utilisee = 0;
	for(bolo=0; bolo<BoloNb; bolo++) if(Bolo[bolo].local) {
		for(cap=0; cap<Bolo[bolo].nbcapt; cap++) ModeleVoie[ModeleDet[Bolo[bolo].type].type[cap]].utilisee = 1;
	}
	pour_tout(i,NUMER_MAX) OrdreNumer[i].num = i;
	pour_tout(i,DETEC_MAX) OrdreDetec[i].num = i;
	if(CompteRendu.numer.classmt)   SambaClasseImprime(CLASSE_NUMER,CLASSE_NEANT);
	if(CompteRendu.detectr.classmt) SambaClasseImprime(CLASSE_DETEC,CLASSE_NEANT);
	pour_tout(i,CLASSE_NBOBJETS) SambaClasse(i,(CLASSE_INFO)(ClassementDemande[i]),0);
	if(CompteRendu.numer.classmt)   SambaClasseImprime(CLASSE_NUMER,ClassementCourant[CLASSE_NUMER]);
	if(CompteRendu.detectr.classmt) SambaClasseImprime(CLASSE_DETEC,ClassementCourant[CLASSE_DETEC]);

	for(i=0; i<ModelePhysNb; i++) TrmtPhysiqueListe[i] = ModelePhysListe[i];
	TrmtPhysiqueListe[i++] = "auto"; TrmtPhysiqueListe[i++] = "\0";
	for(SettingsCalageRef=0; SettingsCalageRef<ModelePhysNb; SettingsCalageRef++)
		if(!strcmp(SettingsCalageNom,TrmtPhysiqueListe[SettingsCalageRef])) break;
	for(SettingsCentrageRef=0; SettingsCentrageRef<ModelePhysNb; SettingsCentrageRef++)
		if(!strcmp(SettingsCentrageNom,TrmtPhysiqueListe[SettingsCentrageRef])) break;
	//	printf("* Calage des temps sur physique %d/%d\n",SettingsCalageRef,ModelePhysNb);
	//	i = 0; while(&(TrmtPhysiqueListe[i][0])) { printf("  . Calage possible #%d: %s\n",i,TrmtPhysiqueListe[i]); i++; }

	// necessaire pour LectCompacteBruit() appelee (eventuellement) dans LectSetup, donc avant CalcSetup
	for(bolo=0; bolo<BoloNb; bolo++) CalcAutoBolo[bolo] = 1;
	for(vp=0; vp<ModelePhysNb; vp++) {
		for(vl=0; vl<CalcPhysNb; vl++) if(!strcmp(ModelePhys[vp].nom,CalcSpectreAutoPrefs[vl].nom_phys)) break;
		if(vl < CalcPhysNb) {
			memcpy(CalcSpectreAuto+vp,CalcSpectreAutoPrefs+vl,sizeof(CalcSpectreAutoLu));
			CalcSpectreAuto[vp].lu = vl;
			CalcSpectreAutoPrefs[vl].lu = vp;
		} else {
			strcpy(CalcSpectreAuto[vp].nom_phys,ModelePhys[vp].nom);
			if(CalcPhysNb < PHYS_TYPES) { CalcSpectreAuto[vp].lu = vl = CalcPhysNb++; CalcSpectreAutoPrefs[vl].lu = vp; }
			else CalcSpectreAuto[vp].lu = -1;
		}
	}
	for(vl=0; vl<CalcPhysNb; vl++) if(CalcSpectreAutoPrefs[vl].lu < 0) {
		--CalcPhysNb;
		for(j=vl; j<CalcPhysNb; j++) memcpy(CalcSpectreAutoPrefs+j,CalcSpectreAutoPrefs+j+1,sizeof(CalcSpectreAutoLu));
		for(vp=0; vp<PHYS_TYPES; vp++) if(CalcSpectreAuto[vp].lu > vl) CalcSpectreAuto[vp].lu = CalcSpectreAuto[vp].lu - 1;
	}

	printf("= Optimisation des menus\n");
	if(RepartAvecInit || NumeriseurAcharger || NumeriseurIdentifiable || NumeriseurAdemarrer || (BoloNb >= 2)
	   || DetecAvecPolar || DetecAvecRaz || DetecAvecModulation || strcmp(NomRegenStart,"neant") || DetecAvecModulation) {
		mSambaProcs = MenuIntitule(iSambaProcsComplet,"Procedures");
		if(!RepartAvecInit) MenuItemHide(mSambaProcs,(IntAdrs)"Demarrage repartiteurs");
		if(!NumeriseurAcharger) MenuItemHide(mSambaProcs,(IntAdrs)"Chargement FPGA numeriseur");
		if(!NumeriseurIdentifiable) MenuItemHide(mSambaProcs,(IntAdrs)"Reconnaissance numeriseurs");
		if(!NumeriseurAdemarrer) MenuItemHide(mSambaProcs,(IntAdrs)"Demarrage numeriseurs");
		if(BoloNb < 2) MenuItemHide(mSambaProcs,(IntAdrs)"Parametrage detecteurs");
		if(!DetecAvecRaz) MenuItemHide(mSambaProcs,(IntAdrs)"RAZ FETs");
		if(!strcmp(NomRegenStart,"neant")) MenuItemDisable(mSambaProcs,(IntAdrs)"Regeneration");
		if(!DetecAvecModulation) {
			MenuItemHide(mSambaProcs,(IntAdrs)"Courbes V(I)");
			MenuItemHide(mSambaProcs,(IntAdrs)"Courbes R(T)");
		}
	} else {
		mSambaProcs = MenuIntitule(iSambaProcsRestreint,"Procedures");
	}
	if(!DetecAvecPolar) MenuItemHide(mSambaProcs,(IntAdrs)"Inversion polarisation");
	if(AutomLink == -2) MenuItemHide(mSambaReglages,(IntAdrs)"Automates");
	if(!DetecAvecReglages) MenuItemHide(mSambaReglages,(IntAdrs)"Reglage Voie");
	if(ConfigNb < 2) MenuItemHide(mLectRuns,(IntAdrs)"Configuration de run");
	if(!NumeriseurAvecStatus) {
		MenuItemHide(mMonitBarre,(IntAdrs)"Etat Numeriseurs");
		MenuItemHide(mMonitBarre,(IntAdrs)"Etat Detecteurs");
	}
	if(!SequenceNb) MenuItemHide(mLectRuns,(IntAdrs)"Sequences");

}
#pragma mark --- Liaison reseau cote satellites ---
/* ========================================================================== */
int SambaBufferiseShort(unsigned char *buffer, unsigned short valeur) {
	int i,k; unsigned short v;

	i = 0; v = valeur;
	for(k=0; k<sizeof(unsigned short); k++) {
		buffer[i++] = (byte)(v & 0xFF);
		v = (v >> 8);
	}
	return(i);
}
/* ========================================================================== */
int SambaBufferiseInt(unsigned char *buffer, int valeur) {
	int i,k; int v;
	
	i = 0; v = valeur;
	for(k=0; k<sizeof(int); k++) {
		buffer[i++] = (byte)(v & 0xFF);
		v = (v >> 8);
	}
	return(i);
}
/* ========================================================================== */
int SambaBufferiseInt64(unsigned char *buffer, int64 valeur) {
	int i,k; int64 v;
	
	i = 0; v = valeur;
	for(k=0; k<sizeof(int64); k++) {
		buffer[i++] = (byte)(v & 0xFF);
		v = (v >> 8);
	}
	return(i);
}
/* ========================================================================== */
int SambaBufferiseTexte(unsigned char *buffer, char *texte, int m) {
	int i,k;
	
	i = 0;
	k = 0; do buffer[i++] = texte[k]; while(texte[k++]);
	do buffer[i++] = '\0'; while(++k < m);
	return(i);
}
/* ========================================================================== */
static int SambaConnecteMaitre(TypeIpAdrs adrs, int port) {

	if(SocketInitNum(adrs.val,port,&SktSortieMaitre) <= 0) {
		printf("%s/ ! L'adresse %d.%d.%d.%d:%d est inaccessible",DateHeure(),IP_CHAMPS(adrs),port);
		return(0);
	};
	if(EcritureMaitre >= 0) SocketFermee(EcritureMaitre);
	EcritureMaitre = SocketConnectUDP(&SktSortieMaitre);
	printf("%s/ Envoi des donnees vers %d.%d.%d.%d:%d: %s\n",DateHeure(),IP_CHAMPS(adrs),port,
		(EcritureMaitre >= 0)? "operationnel": "impossible");

	return(1);
}
/* ========================================================================== */
static int SambaOuvreOreillesSat() {
	LectureMaitre = SocketUDP(PortEcoute);
	if(LectureMaitre >= 0) {
		if(fcntl(LectureMaitre,F_SETFL,O_NONBLOCK) == -1) {
			printf("  ! Socket pour lecture sur %d bloquante: %s\n",PortEcoute,strerror(errno));
		}
	} else {
		printf("!!! Connexion pour lecture sur %d impossible: %s\n",PortEcoute,strerror(errno));
		return(0);
	}
	printf("    . Acquisition a l'ecoute sur le port %d (via le chemin <%d>)\n",PortEcoute,LectureMaitre);
	return(1);
}
/* ========================================================================== */
static int SambaEcouteMaitre() {
	int k,n,nb; unsigned int lngr; short bb;
	struct {
		unsigned char cmde,mode;
		unsigned char c1,c2;
		union {
			unsigned short courte[3];
			int64 longue;
		} valeur;
	} message;
	TypeSocketIP skt; // SktEntreeMaitre
	TypeIpAdrs ipadrs;
	TypeBNModele *modele_bn; TypeBNModlRessource *ress; TypeNumeriseur *numeriseur;
	unsigned char ssadrs; unsigned short hval;

	if(LectureMaitre < 0) return(0);
	lngr = sizeof(TypeSocketIP);
	nb = recvfrom(LectureMaitre,(void *)&message,sizeof(message),0,(TypeSocket *)&skt,&lngr);
	if(nb > 0) {
		ipadrs.val = skt.sin_addr.s_addr;
		printf("%s/ Recu [%d/%ld] de %d.%d.%d.%d: %c=%02X.%02X/%02X.%02X",DateHeure(),nb,sizeof(message),
		   IP_CHAMPS(ipadrs),message.cmde,message.cmde,message.mode,message.c1,message.c2);
		if(nb == sizeof(message)) {
			if(BigEndianOrder) ByteSwappeLong(&message.valeur.longue);
			printf("/%016llX\n",message.valeur.longue);
		} else {
			n = (nb - 4) / 2;
			if(BigEndianOrder) ByteSwappeShortArray(message.valeur.courte,n);
			for(k=0; k<n; k++) printf(".%04X",message.valeur.courte[k]);
			printf("\n");
		}
	}
	if(nb >= 6) switch(message.cmde) {
	  case SMB_C:
//		ipadrs.val = ((TypeSocketIP *)&SktEntreeMaitre)->sin_addr.s_addr;
		ipadrs.val = skt.sin_addr.s_addr;
		printf("%s/ Maitrise demandee par %d.%d.%d.%d\n",DateHeure(),IP_CHAMPS(ipadrs));
		SambaConnecteMaitre(ipadrs,message.valeur.courte[0]);
		break;
	  case SMB_N:
		ssadrs = message.c2; hval = message.valeur.courte[0];
		bb = message.valeur.courte[1];
		numeriseur = &(Numeriseur[bb]);
		modele_bn = &(ModeleBN[numeriseur->def.type]);
		for(k=0; k<modele_bn->nbress; k++) if(modele_bn->ress[k].ssadrs == ssadrs) {
			ress = &(modele_bn->ress[k]);
			if(ress->bit0 < 0) numeriseur->ress[k].hval = hval;
			else numeriseur->ress[k].hval = (hval >> ress->bit0) & ress->masque;
			NumeriseurRessHexaChange(numeriseur,k);
			printf("%s/ %s.%s = %s %s\n",DateHeure(),numeriseur->nom,ress->nom,numeriseur->ress[k].cval,ress->unite);
		}
		switch(message.mode) {
		  case SMB_NUMER_AUTO_ADRS:
			NumeriseurChargeAdrsAuto(numeriseur,ssadrs,hval,"","");
			NumeriseurChargeRessourceFin("");
			break;
		  case SMB_NUMER_MANU_ADRS:
			/* reporter la valeur transmise dans la ressource en utilisant la sous-adresse */
			NumeriseurChargeValeurBB(numeriseur,ssadrs,hval);
			// short entree;
			// printf("%s/ Envoye a %s.%d: %s\n",DateHeure(),Repart[rep].nom,entree,RepartiteurValeurEnvoyee);
			printf("%s/ Envoye a %s: %s\n",DateHeure(),numeriseur->nom,RepartiteurValeurEnvoyee);
			break;
		}
		break;
	  case SMB_M:
		if(!Acquis[AcquisLocale].etat.active) {
			LectStampMini = message.valeur.longue;
			printf("%s/ Demarrage demande au timestamp %lld\n",DateHeure(),LectStampMini);
			SettingsPartition = SAMBA_PARTIT_STD;
			LectDemarre();
		}
		break; // sinon reseter le compteur
	  case SMB_A:
		printf("%s/ Arret demande via le reseau\n",DateHeure());
		LectArretExterieur = 1; LectStop(); break;
	  case SMB_E:
		printf("%s/ Recu trigger a %lld (0x%016llX)\n",DateHeure(),message.valeur.longue,message.valeur.longue);
		TrmtCandidatSignale(1,-1,message.valeur.longue,0,TRGR_EXTERNE);
		break;
	}
	return(nb);
}
#pragma mark --- Liaison reseau cote maitre ---
//+ SambaEcritSat1Court(sat,SMB_CONNECT,(unsigned short)port);
//+ SambaEcritSatLong(sat,SMB_M,LectStampMini);
//+ SambaEcritSatLong(sat,SMB_E,TimeStampTrgr);
//+ SambaEcritSat1Court(sat,SMB_ARRET);
/* ========================================================================== */
static char SambaConnecteSat(int sat) {
	if(SocketInit(Acquis[sat].adrs,PortEcoute,&(Acquis[sat].synchro.skt)) <= 0) {
		OpiumWarn("Adresse %s:%d incorrecte, secteur abandonne",Acquis[sat].adrs,PortEcoute);
		printf("    ! Connection sur %s:%d impossible (%s)\n",Acquis[sat].code,PortEcoute,strerror(errno));
		Acquis[sat].synchro.path = -1;
	} else {
		if((Acquis[sat].synchro.path = SocketConnectUDP(&(Acquis[sat].synchro.skt))) < 0) {
			OpiumWarn("Connexion UDP sur %s:%d impossible, secteur abandonne",Acquis[sat].adrs,PortEcoute);
			printf("    ! Connection sur %s:%d en erreur (%s)\n",Acquis[sat].code,PortEcoute,strerror(errno));
		}
	}
	if(Acquis[sat].synchro.path >= 0) {
		printf("  . Acces en ecriture sur %s:%d via le chemin <%d>\n",Acquis[sat].adrs,PortEcoute,Acquis[sat].synchro.path);
		return(1);
	} else return(0);
}
/* ========================================================================== */
int SambaEcritSat1Court(int sat, unsigned char cmde, unsigned char c1, unsigned char c2, unsigned short valeur) {
	unsigned char buffer[6]; int i,n;
	
	i = 0;
	buffer[i++] = cmde; buffer[i++] = 0;
	buffer[i++] = c1;   buffer[i++] = c2;
//	buffer[4] = (byte)(valeur & 0xFF); buffer[5] = (byte)((valeur >> 8) & 0xFF);
	i += SambaBufferiseShort(buffer+i,valeur);
	n = write(Acquis[sat].synchro.path,buffer,6);
	sprintf(AcquisMsgEnvoye,"%c.%02X.%02X.%02X.%02X%02X",buffer[0],buffer[1],buffer[2],buffer[3],buffer[5],buffer[4]);
	printf("%s/ Envoye a %s: %s [%d/6]\n",DateHeure(),Acquis[sat].code,AcquisMsgEnvoye,n);
	return(n);
}
/* ========================================================================== */
int SambaEcritSatCourt(int sat, unsigned char cmde, unsigned char cle, unsigned char c1, unsigned char c2, int nb, ...) {
	unsigned char buffer[6]; int i,n;
	va_list argptr; int k,l; unsigned short v;
	
	i = 0;
	buffer[i++] = cmde; buffer[i++] = cle;
	buffer[i++] = c1;   buffer[i++] = c2;
	va_start(argptr,nb);
	k = nb;
	while(k--) {
		v = va_arg(argptr,int);
		i += SambaBufferiseShort(buffer+i,v);
	}
	va_end(argptr);
	n = write(Acquis[sat].synchro.path,buffer,i);
	l = sprintf(AcquisMsgEnvoye,"%c.%02X/%02X.%02X",buffer[0],buffer[1],buffer[2],buffer[3]);
	for(k=0; k<nb; k++) l += sprintf(AcquisMsgEnvoye+l,".%02X%02X",buffer[(2*k)+5],buffer[(2*k)+4]);
	printf("%s/ Envoye a %s: %s [%d/%d]\n",DateHeure(),Acquis[sat].code,AcquisMsgEnvoye,n,i);
	return(n);
}
/* ========================================================================== */
int SambaEcritSatLong(int sat, unsigned char cmde, int64 valeur) {
	unsigned char buffer[12]; int i,n;

	i = 0;
	buffer[i++] = cmde; 
	// for(k=1; k<4; k++) buffer[i++] = 0;
	while(i < 4) buffer[i++] = 0;
	i += SambaBufferiseInt64(buffer+i,valeur);
	n = write(Acquis[sat].synchro.path,buffer,i);
	sprintf(AcquisMsgEnvoye,"%c.%02X/%02X%02X/%02X%02X%02X%02X.%02X%02X%02X%02X",
		buffer[0],buffer[1],buffer[2],buffer[3],buffer[11],buffer[10],buffer[9],buffer[8],buffer[7],buffer[6],buffer[5],buffer[4]);
	printf("%s/ Envoye a %s: %s [%d/%d]\n",DateHeure(),Acquis[sat].code,AcquisMsgEnvoye,n,i);
	return(n);
}
/* ========================================================================== */
static int SambaOuvreOreillesMaitre(int sat) {
	int port;
	
	port = PortLectSat + sat;
	Acquis[sat].ecoute.path = SocketUDP(port);
	if(Acquis[sat].ecoute.path >= 0) {
		if(fcntl(Acquis[sat].ecoute.path,F_SETFL,O_NONBLOCK) == -1) {
			printf("  ! Socket pour lecture sur %d bloquante: %s\n",port,strerror(errno));
		}
	} else {
		printf("!!! Connexion pour lecture sur %d impossible: %s\n",port,strerror(errno));
		return(0);
	}
	printf("    . Maitre a l'ecoute de %s sur le port %d (via le chemin <%d>)\n",Acquis[sat].code,port,Acquis[sat].ecoute.path);
	SambaEcritSat1Court(sat,SMB_CONNECT,(unsigned short)port);
	return(1);
}
#pragma mark --- Initialisation generale ---
/* ========================================================================== */
void SambaLitCalendrier(Menu menu, MenuItem *item) {
	int i,n;
	
	if(CompteRendu.initialisation) printf("  . Lecture du calendrier %s\n",CalendrierNom);
	if((SambaRunsNb = CaldrEvenements(CalendrierNum,SambaRunPrevu,MAXRUNS))) {
		n = 0;
		if(CompteRendu.initialisation) for(i=0; i<SambaRunsNb; i++) {
			char date[DATE_MAX];
			if(!strcmp(SambaRunPrevu[i].titre,CalendrierRun)) printf("  | Run #%3d:",++n); 
			else printf("  | %-9s",SambaRunPrevu[i].titre);
			DateLongPrint(date,SambaRunPrevu[i].date.debut); printf(" du %s [jour %3d]",date,DateIntToJours(DateLongToJour(SambaRunPrevu[i].date.debut)));
			DateLongPrint(date,SambaRunPrevu[i].date.fin)  ; printf(" au %s [jour %3d]\n",date,DateIntToJours(DateLongToJour(SambaRunPrevu[i].date.fin)));
		}
	} else printf("  ! Calendrier %s vide\n",CalendrierNom);
}
/* ========================================================================== */
static int SambaInit() {
	int i,n,rc,fmt; int voie,prio;
#ifdef MODULES_RESEAU
	TypeSvrBox cpulink;
#endif

	rc = 0; /* gcc */

#ifdef MODULES_RESEAU
	pAcquisEtat = 0;
	SambaProchainAffichage = 0;
	if(SambaMode == SAMBA_MONO) AcqCible[0] = '\0';
	else {
		if(SambaMode == SAMBA_MAITRE) strcpy(AcqCible,"dphmck.saclay.cea.fr");
		else if(SambaMode = SAMBA_SATELLITE) strcpy(AcqCible,"EDWACQ-m.local");
		if(!ModeBatch) {
			if(OpiumReadKeyB(L_("Mode d'execution"),L_(SAMBA_MODE_CLES),&SambaMode,12) == PNL_CANCEL) return(0);
		}
	}
#endif

	SambaLogTrait('=',0);
	printf("Demarrage de %s v%s le %s a %s.\n\n",SambaMoiMeme,SambaVersion,DateCivile(),DateHeure());
//+	if(setpriority(PRIO_PROCESS,0,-10) < 0) printf("! Changement de priorite: %s\n",strerror(errno));
	errno = 0;
	prio = getpriority(PRIO_PROCESS,0);
	if(prio == -1) printf("* Verification de la priorite: %s\n",strerror(errno));
	printf("  Execution en mode %s, priorite %d.\n",ModeBatch? "batch": "interactif",prio);
#ifdef SKTLIB
	if((AdresseUtilisee[0] != '\0') && strcmp(AdresseIP,AdresseUtilisee)) {
		strcpy(AdresseIP,AdresseUtilisee);
		HostName(AdresseIP,NomExterne);
		CodeIpLocal = HostAdrsToInt(AdresseIP);
	}
#endif /* SKTLIB */
	strcat(strcpy(BBstsPath,SauvVol),BBstsDir); RepertoireTermine(BBstsPath);
	for(fmt=0; fmt<ARCH_TYPEDATA; fmt++) {
		strcat(strcpy(ArchZone[fmt],SauvVol),ArchDir[fmt]); RepertoireTermine(ArchZone[fmt]);
	}

	SambaLogDef("L'executable a ete cree","dans",SambaAppli);
	printf("  Coordonnees IP pour les donnees          : '%s' (%s)\n",NomExterne,AdresseIP);
	// SambaLogDef("Coordonnees IP pour les donnees",":",AdresseIP);

	SambaLogTrait('.',"Emplacement de l'information");

	printf("Parametrage:\n");
	SambaLogDef("Racine des fichiers d'acquisition ($top)",":",FullTopDir);
	if(!RepertoireExiste(FullTopDir)) printf("  ! Le repertoire %s n'existe pas\n",FullTopDir);
	SambaLogDef("Les fichiers maitre sont dans $maitre","=",MaitreDir);
	if(!RepertoireExiste(MaitreDir)) {
		printf("  ! Le repertoire %s n'existe pas\n",MaitreDir);
		if(!strncmp(MaitreDir,"/Volumes/",9)) {
			char *c;
			c = MaitreDir + 9;
			strcpy(VolumeManquant,ItemJusquA('/',&c));
		}
	}
	SambaLogDef("Les fichiers de configuration sont","dans",PrefPath);
	printf("\n");
	SambaLogDef("Base de donnees 'detecteurs'","dans",DbazPath);
	SambaLogDef("Catalogue des sauvegardes","dans",CtlgPath);
	for(fmt=0; fmt<ARCH_TYPEDATA; fmt++) 
		printf("  | Repertoire pour les %-18s : '%s'\n",ArchFmtNom[fmt],ArchZone[fmt]);
	SambaLogDef("| Status des numeriseurs sauves","dans",BBstsPath);
	SambaLogDef("| Resultats stockes","dans",ResuPath);
	SambaLogDef("| Figures sauvees","dans",FigsPath);

	if((rc = RepertoireAbsent(FigsPath))) {
		printf("  ! La creation du repertoire %s retourne %d (%s/ errno=%d)\n",
			   FigsPath,rc,strerror(rc),errno);
		printf("  => Attention lors de la sauvegarde des graphiques\n");
		strcpy(FigsPath,FullTopDir);
	}
	if(VolumeManquant[0]) return(0);

	OpiumPShome(FigsPath);
	OpiumTxtHome(FigsPath);
	SambaNonOui = LL_(SambaBooleen);
	for(i=0; i<ARCH_NBMODES; i++) ArchModeDetec[i] = ArchModeNoms[i];
	ArchModeDetec[i++] = "standard"; ArchModeDetec[i++] = "\0";
	LectureActive = 0;
	Trigger.type = NEANT;
	Trigger.enservice = (Trigger.type == NEANT)? 0: 1;

	/* Valeurs par defaut de la configuration generale de la manip (non initialisees par SambaDefauts) */
	strcpy(NomModeles,"Modeles");
	strcpy(NomMedia,"Media");
	strcpy(NomSimu,"Simu");
	strcpy(NomModlDetecteurs,"../modeles_EDW/detecteurs");
	strcpy(NomModlNumeriseurs,"../modeles_EDW/numeriseurs");
	strcpy(NomModlRepartiteurs,"../modeles_EDW/repartiteurs");
	strcpy(NomModlCablage,"../modeles_EDW/cablages");
	strcpy(NomModlChassis,"../modeles_EDW/chassis");
	strcpy(NomModlEnvir,"../modeles_EDW/environnement");

	strcpy(NomCr,"CompteRenduInitial");
	strcpy(NomParms,"Parametres");
	strcpy(NomReseau,"Reseau");
	strcpy(NomSatellites,"Satellites");
	strcpy(NomDico,"Dictionnaires");
	strcpy(NomProcedures,"Procedures");
//	strcpy(NomScriptStart,"run_demarre");
//	strcpy(NomScriptStop,"run_stoppe");
	NomScriptStart[0] = '\0';
	NomScriptStop[0] = '\0';
	NomEntretienStart[0] = '\0';
	NomEntretienStop[0] = '\0';
	strcpy(NomRegenStart,"regen_demarre");
	strcpy(NomRegenStop,"regen_termine");
	strcpy(NomDetecteurs,"Detecteurs/liste");
	strcpy(NomCablage,"Cablage");
	strcpy(NomNumeriseurs,"Numeriseurs/liste");
	strcpy(NomRepartiteurs,"Repartiteurs");
	strcpy(NomFiltres,"Filtres");
	strcpy(NomSequences,"Sequences");
	strcpy(NomRegulEvt,"RegulEvt");
	strcpy(NomVi,"Vi");
	strcpy(NomEvtUnite,"EvtUnite");
	strcpy(NomLecture,"Lecture");
	strcpy(NomMonit,"Affichage");
	strcpy(NomAutom,"Automates");
	strcpy(NomCalcul,"Calculs");
	strcpy(NomFinesses,"Finesses");
	strcpy(NomBasic,"Basic");
	strcpy(NomVerif,"Verifications");
	strcpy(NomExports,"Exportations");
	strcpy(NomBanc,"Banc");

	strcpy(NomRuns,"RunEnregistre");
	strcpy(NomEtatElec,"EtatPolars");
	strcpy(NomEnvironnement,"RunConditions");
	strcpy(NomCaract,"RunCaracteristiques");

	strcpy(CalendrierRef,"Calendriers");
	strcpy(CalendrierNom,"Samba");
	strcpy(CalendrierRun,"run");
	
//	PRINT_OPIUM_ADRS(pScriptCntl);
	/* Reglages par fichier de configuration ("preferences") */
	/* Attention: pour les "PasMaitres", ne pas abuser (certaines valeurs 
	   doivent rester celles du superviseur) */
	SambaLogTrait('.',"Initialisation independante du materiel");

	SambaAjoutePrefPath(ConfigGeneral,NomGeneral);
	SambaLogDef("= Lecture de l'organisation generale","dans",ConfigGeneral);
	if(ArgScan(ConfigGeneral,Setup) == 0) {
		if(CreationFichiers) ArgPrint(ConfigGeneral,Setup);
		else OpiumError("Lecture sur '%s' impossible",ConfigGeneral);
	}
	if(ImprimeConfigs) {
		printf("#\n### Fichier: '%s' ###\n",ConfigGeneral);
		ArgPrint("*",Setup);
	}
	if(VoiesGerees < BoloGeres) VoiesGerees = BoloGeres * ModeleVoieNb;
//	PRINT_OPIUM_ADRS(pScriptCntl);
	SambaNomFichiers();
	SambaPrepareFolder();

	SambaLogDef("= Lecture des options de compte-rendu","dans",FichierCrParms);
	if(ArgScan(FichierCrParms,SambaCrDesc) <= 0) {
		if(CreationFichiers) {
			RepertoireCreeRacine(FichierCrParms);
			ArgPrint(FichierCrParms,SambaCrDesc);
		} else OpiumError("Lecture de '%s' impossible",FichierCrParms);
	}

	SambaLogDef("= Lecture de dictionnaires","dans",FichierPrefDico);
	if(SambaDicoLit() < 0) printf("  . Pas de dictionnaire (%s)\n",strerror(errno));

	SambaLogDef("= Lecture des calendriers","dans",FichierCaldrRef);
	CaldrInit(FichierCaldrRef); SambaRunsNb = 0;
	if((CalendrierNum = CaldrOuvre(CalendrierNom))) SambaLitCalendrier(0,0); 
	else printf("  . Pas de calendrier %s defini\n",CalendrierNom);

	/* Creation des structures dynamiques et initialisations dependant des preferences */
//	PRINT_OPIUM_ADRS(pScriptCntl);
	if(!SambaAlloc()) sortie("Memoire indisponible");
//	PRINT_OPIUM_ADRS(pScriptCntl);
	n = VoiesGerees * sizeof(HistoDeVoie);
	if(VoieHisto) free(VoieHisto); VoieHisto = (HistoDeVoie *)malloc(n);
	if(VoieHisto == 0) {
		printf("! Allocation de l'objet VoieHisto[%d] impossible\n",n);
		return(0);
	}
	for(voie=0; voie<VoiesGerees; voie++) VoieHisto[voie] = 0;
	n = VoiesGerees * sizeof(TypeVoieTampons);
	if(VoieTampon) free(VoieTampon); VoieTampon = (TypeVoieTampons *)malloc(n);
	if(VoieTampon == 0) {
		printf("! Allocation de l'objet VoieTampon[%d] impossible\n",n);
		return(0);
	}
    memset(VoieTampon,0,n);
	if(!FiltresInit(FLTR_MAX)) { printf("! %s\n",FiltreErreur); sortie("Memoire indisponible"); }
	FiltresDialogueCreate();
	
/*
 *  Fin de l'initialisation pour l'ensemble du logiciel
 *  (valeurs par defaut + interface utilisateur seulement si non dynamique)
 */
	SettingsInit();   if(CompteRendu.initialisation) printf("> Initialisation de la configuration terminee\n");
	DetecteursInit(); if(CompteRendu.initialisation) printf("> Initialisation des detecteurs terminee\n");
#ifdef BRANCHE_TESTS
	TestsInit();  if(CompteRendu.initialisation) printf("> Initialisation des tests terminee\n");
#endif
	LectInit();   if(CompteRendu.initialisation) printf("> Initialisation de la lecture terminee\n");
	MonitInit();  if(CompteRendu.initialisation) printf("> Initialisation du monitoring terminee\n");
	CalcInit();   if(CompteRendu.initialisation) printf("> Initialisation des calculs  terminee\n");
	DiagInit();   if(CompteRendu.initialisation) printf("> Initialisation des diagnostics terminee\n");
	TrmtInit();   if(CompteRendu.initialisation) printf("> Initialisation des traitements terminee\n");
	ExportInit(); if(CompteRendu.initialisation) printf("> Initialisation des exportations terminee\n");

	SambaLogTrait('-',"Materiel");
	SambaLitMateriel(); /* lit les detecteurs, et avant ca cablage et numeriseurs et tout et tout */
//	SambaLogTrait('-',"(materiel complet)");

/*
 * Etablissement des liens reseau
 */
	OpiumPeriodicFctn(SambaPeriodiqueServeur);
	cntlLecture = &LectCntl;
#ifdef MODULES_RESEAU
	switch(SambaMode) {
		/* Pas de lien reseau dans le mode mono */

	  case SAMBA_MAITRE:
		strcpy(AcqCible,".");
	#ifdef MSGS_RESEAU
		OpiumPeriodicFctn(SambaPeriodiqueAcquis); /* pour tester si on recu des messages */
	#endif
		break;

	  case SAMBA_SATELLITE:
		if(OpiumReadText("Acquisition a connecter",AcqCible,HOTE_NOM) == PNL_CANCEL)
			return(0);
		printf("  Connection sur l'acquisition geree par %s\n",AcqCible);
	#ifdef MSGS_RESEAU
		OpiumPeriodicFctn(SambaPeriodiqueAcquis); /* pour tester si on recu des messages */
		if(!CarlaMsgLink(&BalSuperviseur,NOM_SUPERVISEUR,AcqCible)) {
			OpiumError(CarlaDataError());
			return(0);
		}
	#endif
		break;

	  case SAMBA_DISTANT:
		if(OpiumReadText("Acquisition a connecter",AcqCible,HOTE_NOM) == PNL_CANCEL)
			return(0);
		printf("  Connection sur l'acquisition geree par %s\n",AcqCible);
	#ifdef MSGS_RESEAU
		if(!CarlaMsgLink(&BalSuperviseur,NOM_SUPERVISEUR,AcqCible)) {
			OpiumError(CarlaDataError());
			return(0);
		}
		OpiumPeriodicFctn(SambaPeriodiqueDistant); /* pour tester si on recu des messages */
	#endif
		break;
	}

/*
 * Allocation des structures dynamiques mais pas trop (liste SambaGeneral)
 */
	if(SambaMode == SAMBA_MONO) {
		/* SambaSetup n'apparait jamais en mode mono */
		// deja fait: cntlLecture = &LectCntl;
		/* EvtPartage n'apparait jamais en mode mono */
	} else {
		SvrOpenWait(10);
		// printf("%s/ Ouverture du lien avec %s\n",DateHeure(),NomExterne);
		rc = SvrOpen(&cpulink,NomExterne,PortSvr,TailleSvr);
		if(!rc) {
			OpiumError("Connexion avec %s impossible",NomExterne);
			printf("Connexion avec le serveur CARLA local impossible (%s)\n",strerror(errno));
			exit(errno);
		}
		printf("\n= Demarrage du service CARLA avec '%s' comme maitre\n",AcqCible);
		SvrSend(&cpulink,"start",AcqCible,strlen(AcqCible)+1);
		SvrRecv(&cpulink);
		SvrClose(&cpulink);
		// printf("%s/ Lien referme\n",DateHeure());
		/* Creation des objets partages */
		printf("  . Connexion avec le serveur local\n");
		// CarlaDebugLevel(2);
		if(!CarlaDataOpen(0,PortSvr,TailleSvr,NomTache)) {
			printf("Connexion avec le serveur CARLA local: %s\n",CarlaDataError());
			return(0);
		}
	#ifdef PARMS_RESEAU
		if(!(rc = CarlaListShare(SambaGeneral))) {
			printf("Partage memoire CARLA: %s\n",CarlaDataError());
			exit(errno);
		}
		printf("  . %d objet%s de la liste generale partage%s via CarlaListShare\n",Accord2s(rc));
	#endif
	#ifdef MSGS_RESEAU
		CarlaMsgCreate(NomTache,0);
	#endif
		CarlaStatus(CARLA_ACTIVE);
		CarlaDataClose();
		printf("  . Serveur CARLA local libere a %s\n",DateHeure());
	#ifdef ETAT_RESEAU
		/* SAUF QUE AcquisLocale n'est pas encore definie!! */
		Acquis[AcquisLocale].etat = (TypeAcquisEtat *)CarlaDataShareDyn(Acquis[AcquisLocale].code,&IdLocal,sizeof(TypeAcquisEtat),CARLA_NOINIT,CARLA_TEMPO);
		if((hexa)(Acquis[AcquisLocale].etat) == 0xFFFFFFFF) {
			printf("! Partage du status de cette tache en erreur: %s\n",CarlaDataError());
			printf("!!! Probable plantage dans le futur proche\n");
		}
	#endif
		CarlaDebugLevel(0);
	}
	/* Pour satellites et distants, recuperation des valeurs deja definies par le superviseur */
#ifdef PARMS_RESEAU
	if(PasMaitre) {
		if(!CarlaDataRequest(AcqCible,IdSetup)) {
			printf("Recuperation des parametres SAMBA sur %s: %s\n",AcqCible,CarlaDataError());
			return(errno);
		}
		strncpy(SambaIntitule,SambaSetup->SambaIntitule,256);
		PCIdemande = SambaSetup->PCIdemande;
		FIFOdim = SambaSetup->FIFOdim;
		strncpy(RunCategCles,SambaSetup->RunCategCles,80);
//		strncpy(RunEnvir,SambaSetup->RunEnvir,256);
		ModeleVoieNb = SambaSetup->ModeleVoieNb;
		BoloGeres = SambaSetup->BoloGeres;
		BoloNb = SambaSetup->BoloNb;
		VoiesGerees = SambaSetup->VoiesGerees;
		VoiesNb = SambaSetup->VoiesNb;
	#ifdef FILTRES_PARTAGES
		FiltreNb = SambaSetup->FiltreNb;
	#endif
		DmaDemande = SambaSetup->DmaDemande;
		strncpy(VoieStatus,SambaSetup->VoieStatus,MODL_NOM);
		ImprimeConfigs = SambaSetup->ImprimeConfigs;
		ExpertConnecte = SambaSetup->ExpertConnecte;
	}
	if(SambaMode == SAMBA_MAITRE) {
		strncpy(SambaSetup->SambaIntitule,SambaIntitule,256);
		SambaSetup->PCIdemande = PCIdemande;
		SambaSetup->FIFOdim = FIFOdim;
		strncpy(SambaSetup->RunCategCles,RunCategCles,80);
//		strncpy(SambaSetup->RunEnvir,RunEnvir,256);
		SambaSetup->ModeleVoieNb = ModeleVoieNb;
		SambaSetup->BoloGeres = BoloGeres;
		SambaSetup->BoloNb = BoloNb;
		SambaSetup->VoiesGerees = VoiesGerees;
		SambaSetup->VoiesNb = VoiesNb;
	#ifdef FILTRES_PARTAGES
		SambaSetup->FiltreNb = FiltreNb;
	#endif
		SambaSetup->DmaDemande = DmaDemande;
		strncpy(SambaSetup->VoieStatus,VoieStatus,MODL_NOM);
		SambaSetup->ImprimeConfigs = ImprimeConfigs;
		SambaSetup->ExpertConnecte = ExpertConnecte;
		/* CarlaDataUpdate(IdSetup); Inutile: pas encore de satellite en orbite ... */
	}
#endif /* PARMS_RESEAU */
#endif /* MODULES_RESEAU */

	ArgLngr(RepartDescEdit,"adresse",16,0);
	ArgLngr(RepartDescEdit,"etat",8,0);
	ArgLngr(RepartDescEdit,"donnees",7,0);

	OpiumTableCreate(&SambaTableMatos);
	DetecParmsCompletAS.table = (void *)Bolo;
	OpiumTableAdd(&SambaTableMatos,"Detecteurs",ARG_TYPE_STRUCT,(void *)&DetecParmsCompletAS,&BoloNb,0);
	OpiumTableAdd(&SambaTableMatos,"Numeriseurs",ARG_TYPE_STRUCT,(void *)&NumeriseurAS,&NumeriseurNb,0);
	// OpiumTableAdd(&SambaTableMatos,"Cablage",ARG_TYPE_STRUCT,(void *)&CablageAS,&CablageNb,0);
	OpiumTableAdd(&SambaTableMatos,"Repartiteurs",ARG_TYPE_STRUCT,(void *)&RepartiteurAS,&RepartNb,0);
	mSambaMatos = OpiumTableMenuCreate(SambaTableMatos);

/*	printf("%d cadres crees\n",OpiumNbCdr); */
/*
 *  Prise en compte des differents fichiers de preferences:
 *  parametres utilisateur et description manip depuis fichier;
 *  interface utilisateur si dynamique.
 */
#ifdef MODULES_RESEAU
	if((SambaMode == SAMBA_MAITRE) && pAcquisEtat) {
		printf(". Le panel pAcquisEtat %s affiche\n",MonitAffNet? "doit etre":"ne sera pas");
		if(MonitAffNet) OpiumDisplay(pAcquisEtat->cdr);
	} else printf(". Pas de panel pAcquisEtat\n");
#endif

	SambaLogTrait('-',"Initialisation dependante du materiel");
	if(CompteRendu.initialisation) printf("= Phase finale de l'initialisation ('setup'):\n");
	if(SambaMode != SAMBA_DISTANT) {
		if(!SettingsSetup()) OpiumError("Parametrisation de la configuration impossible");
		if(CompteRendu.initialisation) printf("  . Finalisation de la configuration terminee\n");
		if(!DetecteursSetup()) OpiumError("Finalisation des detecteurs impossible");
		if(CompteRendu.initialisation) printf("  . Finalisation des detecteurs terminee\n");

#ifdef BRANCHE_TESTS
		if(!TestsSetup()) OpiumError("Parametrisation des tests impossible");
		if(CompteRendu.initialisation) printf("  . Finalisation des tests terminee\n");
#endif
		if(!LectSetup()) OpiumError("Parametrisation de la lecture impossible");
		if(CompteRendu.initialisation) printf("  . Finalisation de la lecture terminee\n");
		if(!MonitSetup()) OpiumError("Parametrisation du monitoring impossible");
		if(CompteRendu.initialisation) printf("  . Finalisation du monitoring terminee\n");
		if(!AutomSetup()) OpiumError("Parametrisation des automates impossible");
		if(CompteRendu.initialisation) printf("  . Finalisation des automates terminee\n");
		if(!CalcSetup()) OpiumError("Parametrisation des calculs impossible");
		if(CompteRendu.initialisation) printf("  . Finalisation des calculs terminee\n");
		if(!BasicSetup()) OpiumError("Parametrisation des acces basiques impossible");
		if(CompteRendu.initialisation) printf("  . Finalisation des acces basiques terminee\n");
		if(!DiagSetup()) OpiumError("Parametrisation des diagnostics impossible");
		if(CompteRendu.initialisation) printf("  . Finalisation des diagnostics terminee\n");
		OrgaSetup();
		if(CompteRendu.initialisation) printf("  . Finalisation de l'organigramme terminee\n");
	}
	Trigger.actif = 0;

	printf("= Liste des runs sauvegardes:\n");
	EdbActive = (EdbServeur[0] && strcmp(EdbServeur,"neant"));
	if(EdbActive) printf("  . Base de donnees active sur %s\n",EdbServeur);
	else printf("  . Base de donnees inutilisee\n");
	if(!strcmp(LoggerAdrs,"neant")) LoggerAdrs[0] = '\0';
	if(LoggerAdrs[0] == '\0') printf("  . Pas de cahier de manip\n");
	else if(LoggerType == CAHIER_ELOG) {
		char adrs[MAXFILENAME]; char *r,*ip,*port;
		strcpy(adrs,LoggerAdrs);
		r = adrs; ip = ItemSuivant(&r,':'); port = ItemSuivant(&r,' ');
		if(*port == '\0') port = "8080";
		printf("  . Cahier de manip ELOG sur %s:%s\n",adrs,port);
		sprintf(LoggerCmde,"%s/executables/elog/elog -h %s -p %s %s",FullTopDir,ip,port,LoggerUser);
	} if(LoggerType == CAHIER_TEXTE) {
		printf("  . Cahier de manip au format TEXTE\n");
	}

	SambaMemId = SAMBA_MEM_CLE;
	SambaInfos = (TypeSambaShared *)SharedMemMake(SAMBA_MEM_NOM,sizeof(TypeSambaShared),&SambaMemId);
	printf("* Partage Memoire via %s @0x%08X (%d, %s)\n",SAMBA_MEM_NOM,(unsigned int)SambaInfos,errno,errno?strerror(errno):"successful");
	if(SambaInfos > 0) SambaInfos->lance_run = 0;

	printf("\n");
	sprintf(NomOrdiLocal,"DAQ %s",Acquis[AcquisLocale].code);
	SambaImprimeGeneral();
	if(CompteRendu.cablage.detectr) DetecteursImprime(1);
	// NumeriseursModlImprime();
	if(CompteRendu.voies.locales) SambaDumpVoies(0,CompteRendu.voies.composition);
	// ArgSuitAdresse(chaine(AutomRegenNom[1]),&(AutomRegenNom[1][0]));
	SettingsVerifParms();

	return(0);
}
//-	#include <GLUT/glut.h>
/* ========================================================================== */
int main(int argc, char *argv[]) {
//	int voie;
	char existe,on_recommence,affiche; char rep;
#ifdef GADGET
	char titre[80];
#endif

//-	glutInit(&argc, argv);
	gettimeofday(&SambaHeureDebut,0);
	// malloc_error_break();
    if((argc > 1) && !strcmp(argv[1],"-hw")) SambaConfigHwSw();
//	memset(&CompteRendu,1,sizeof(CompteRendu));
	bzero(&CompteRendu,sizeof(CompteRendu));
	SambaAideDemandee = (argc > 1)
		&& (!strcmp(argv[1],"-help") || !strcmp(argv[1],"--help") || !strcmp(argv[1],"--h") 
			|| !strcmp(argv[1],"-h") || !strcmp(argv[1],"-?"));
	if(!SambaAideDemandee) {
		SambaConfigHwSw();
		SambaInitBasique();
	}
	existe = SambaParms(argc,argv);
#ifdef WXWIDGETS
	return 1;
#endif
	affiche = SambaInitOpium();
	if(affiche && !existe && !InstalleSamba) {
		PanelBoutonText(pArgs,PNL_CANCEL,"No Future");
//		while(OpiumExec(pArgs->cdr) != PNL_OK)
//			OpiumError("On ne peut pas commencer tant que ces parametres sont incertains");
		if(OpiumExec(pArgs->cdr) == PNL_CANCEL) exit(0);
		ArgPrint("SambaArgs",SambaDesc);
	}
	if(!affiche) ModeBatch = 1;
	
	printf("^^^^^^^^^^^^^^^^^^^^^ HELLO\n");
	if(!InstalleSamba) SambaVerifieProcess();
	SambaPresentation = 0;
	if(!ModeBatch) {
#ifdef GADGET
		char date_compil[32],datation[80],*r,*mm,*jj; int aa;
		strcpy(date_compil,__DATE__);
		r = date_compil;
		mm = ItemJusquA(' ',&r);
		jj = ItemJusquA(' ',&r);
		sscanf(r,"%d",&aa); if(aa > 100) aa = aa % 100;
		sprintf(datation,"%s %s %d %s",jj,mm,aa,__TIME__);
		if(SambaPresentation) OpiumClear(SambaPresentation->cdr);
		else {
			SambaPresentation = InfoCreate(1,80);
			OpiumPosition(SambaPresentation->cdr,0,44);
			sprintf(titre,"%s version %s",SambaMoiMeme,SambaVersion);
			InfoTitle(SambaPresentation,titre);
		}
		InfoWrite(SambaPresentation,1,"SAMBA version %s, %s",SambaVersion,datation);
		OpiumDisplay(SambaPresentation->cdr);
	#ifdef MENU_BARRE
		OpiumMinimum(SambaPresentation->cdr);
	#else
		OpiumMinimum(mSambaBarre->cdr);
	#endif
#else
		OpiumMinimum(mSambaBarre->cdr);
#endif
	}

	LogBlocBegin();
	SambaInit();
/*	{
		struct timeval temps;
		struct timezone zone;
		time_t secs;
		LectT0Run = DateMicroSecs();
		gettimeofday(&temps,&zone);
		time(&secs);
		printf("DateMicroSecs = %20lld us\n",LectT0Run);
		printf("gettimeofday  = %14d%06d us\n",(int)temps.tv_sec,(int)temps.tv_usec);
		printf("time          = %14d s\n",(int)secs);
		printf("struct timeval: %d octets\n",sizeof(struct timeval));
		printf("time_t        : %d octets\n",sizeof(time_t));
	} */
//	printf("sizeof(double)=%d alors que sizeof(long double)=%d et que sizeof(float)=%d\n",sizeof(double),sizeof(long double),sizeof(float)); //	resultat: 8, 16 et 4
	printf("sizeof(void *)=%ld et sizeof(long)=%ld alors que sizeof(long long)=%ld et que sizeof(int)=%ld\n",sizeof(void *),sizeof(long),sizeof(long long),sizeof(int)); //	resultat: 4, 8 et 4
	LogBlocEnd();

	if(VolumeManquant[0]) {
		int x,y;
		x = (OpiumServerWidth(0) / 2) - (45 * Dx); y = (OpiumServerHeigth(0) / 4);
		OpiumPosition(OpiumCdrPrompt,x,y);
		OpiumFail("Le disque '%s' n'a pas ete monte",VolumeManquant);
		InfoDelete(SambaPresentation);
		SambaPresentation = InfoCreate(4,90);
		OpiumPosition(SambaPresentation->cdr,x,y);
		InfoTitle(SambaPresentation,"Procedure a suivre");
		InfoWrite(SambaPresentation,1,". Cliquer sur le fond d'ecran pour activer le Finder");
		InfoWrite(SambaPresentation,2,". Taper <cmd>K, selectionner l'adresse du serveur puis cliquer sur 'Se connecter'");
		InfoWrite(SambaPresentation,3,". Selectionner '%s', puis cliquer sur 'OK'",VolumeManquant);
		InfoWrite(SambaPresentation,4,". Relancer SAMBA (fenetre ci-dessous)");
		OpiumDisplay(SambaPresentation->cdr);
		OpiumMinimum(SambaPresentation->cdr);
		y += (8 * Dy);
		OpiumPosition(OpiumCdrPrompt,x,y);
		/* rend ENODEV (Operation not supported by device): {
			char montage[MAXFILENAME],lointain[MAXFILENAME];
			sprintf(montage,"/Volumes/%s",VolumeManquant);
			sprintf(lointain,"132.166.12.50:/Users/%s",VolumeManquant);
			RepertoireAbsent(montage);
			if(mount("afp",montage,MNT_SYNCHRONOUS,lointain) == -1) OpiumFail(strerror(errno));
		} */
		while(!OpiumCheck("Disque '%s' monte?",VolumeManquant)) sleep(1);
		remove("SambaPid"); errno = ENOENT; sortie("Disque maitre indisponible");
	}
	
	/* Dialogue */
	if(!InstalleSamba) {
		if(LoggerType == CAHIER_ELOG) {
			char commande[256];
			sprintf(commande,"%s -a \"Subject=Samba Execution\" \"Samba started on [B]%s[/B] at %s\" &",LoggerCmde,Acquis[AcquisLocale].code,DateHeure());
			printf("* Information ELOG:\n  [ %s ]\n",commande);
			system(commande);
		}
		TimerStart(LectTaskReadout,SettingsReadoutPeriod*1000);
		if(ModeBatch) LectAcqStd(); else do {
			if(!RepartNb) OrgaMagasinVisite(0,0);
			if(InitBolos) DetecteurChargeTous("*");
			if(LectCompacteUtilisee) { OpiumFork(bLecture); OpiumUserAction(); }
//		#ifdef GADGET
//			OpiumActive(SambaPresentation->cdr);
//		#endif
			on_recommence = 0;
		#ifdef MENU_BARRE
			MenuBarreExec();
		#else
			// OpiumDebug(OPIUM_DEBUG_OPIUM,1);
			OpiumExec(mSambaBarre->cdr);
		#endif

			if(SambaMode != SAMBA_DISTANT) {
				int mode;
				if(!SettingsExit()) { on_recommence = 1; continue; }
				if(!DetecteursExit()) { on_recommence = 1; continue; }
				if(!SambaSauve(&RepartiteursASauver,"systeme de repartition",0,0,&RepartiteursSauve)) { on_recommence = 1; continue; }
				if(!OrgaMagModifie[0]) for(mode=ORGA_MAG_REPART; mode <= ORGA_MAG_MAX; mode++) if(OrgaMagModifie[mode]) { OrgaMagModifie[0] = 1; break; }
				OrgaMagMode = 0;
				if(!SambaSauve(&OrgaMagModifie[0],"organigramme",-1,0,(TypeFctn)(&OrgaMagasinEnregistre))) { on_recommence = 1; continue; }
			#ifdef BRANCHE_TESTS
				if(!TestsExit()) { on_recommence = 1; continue; }
			#endif
				if(!LectExit())  { on_recommence = 1; continue; }
				if(!MonitExit()) { on_recommence = 1; continue; }
				if(!PlotExit())  { on_recommence = 1; continue; }
				if(!CalcExit())  { on_recommence = 1; continue; }
				if(!BasicExit()) { on_recommence = 1; continue; }
				if(!DiagExit())  { on_recommence = 1; continue; }
			}
		} while(on_recommence);
	}

	for(rep=0; rep<RepartNb; rep++) RepartiteurStoppeTransmissions(&(Repart[rep]),1);

	if(LoggerType == CAHIER_ELOG) {
		char commande[256];
		sprintf(commande,"%s -a \"Subject=Samba Execution\" \"Samba ended on [B]%s[/B] at %s\" &",LoggerCmde,Acquis[AcquisLocale].code,DateHeure());
		printf("* Information ELOG:\n  [ %s ]\n",commande);
		system(commande);
	}
	printf("%s/ Fin d'execution de SAMBA\n",DateHeure());
	/* Fin de l'execution */
	Acquis[AcquisLocale].etat.status = SATELLITE_ARRETE;
	printf("%s/ %d couleurs utilisees\n",DateHeure(),WndColorNb);
/*	int rep;
	for(rep=0; rep<RepartNb; rep++) {
		if(Repart[rep].actif) RepartiteurStoppeTransmissions(&(Repart[rep]),1);
		if(Repart[rep].famille == FAMILLE_NI) {
			printf("            . reset du driver %s (arret et suppression des taches associees)\n",Repart[rep].nom);
			DAQmxBaseResetDevice(Repart[rep].nom+3);
		}
	} */
	SambaLangueConclusion();
	
	remove("SambaPid");
	errno = 0; sortie(0);
	return(0);
}
