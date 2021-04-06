#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <math.h>
#include <execinfo.h>

extern char *__progname;

#define OPIUM_C
#include <opium.h>
#include <decode.h>
#include <dico.h>
#include <dateheure.h>
#include <html.h>
typedef unsigned long long UInt64;

#define Accord1s(var) var,(var>1)?"s":""
#define Accord2s(var) var,(var>1)?"s":"",(var>1)?"s":""
#define Accord1S(var) var,(var>1)?"S":""
#define Accord2S(var) var,(var>1)?"S":"",(var>1)?"S":""

#define CHANGE_CURSEUR

typedef enum {
	MACRO_STORE = -1,
	MACRO_NONE  = 0,
	MACRO_RECALL = 1
} MACRO_PHASES;

static char OpiumHome[OPIUM_MAXFILE];

static int OpiumNbExec;
#ifdef POSITION_PAR_TYPE
static int OpiumPosPrevueX[OPIUM_NBTYPES],OpiumPosPrevueY[OPIUM_NBTYPES];
#else
static int OpiumProchainX,OpiumProchainY;
#endif
static int OpiumDernierClicX,OpiumDernierClicY;
static WndUserRequest OpiumLastEvent;
static TypeOpiumFctn OpiumOption[OPIUM_NBTYPES];
static char OpiumAscenseurEnCours;
// static char OpiumZoomEnCours;
// static int OpiumZoom0[0],OpiumZoom0[1];

static char  OpiumMacroCntrl[4] = { '\0', '\0', '\0', '\0' };   /* car. d'appel des macros   */
static char  OpiumMacroName[40];       /* nom de la macro courante  */
static int   OpiumMacroPhase;          /* -1: ecr. / 1: lec. macro  */
static FILE *OpiumMacroFile;           /* FILE de la macro courante */
static Info  OpiumMacroInfo;

static char  OpiumTxtPath[OPIUM_MAXFILE];
static char  OpiumTxtNom[OPIUM_MAXFILE];

static Menu  OpiumPSmenu;
static Panel OpiumPSpanel;
static char  OpiumPSpath[OPIUM_MAXFILE];
static char  OpiumPSnom[OPIUM_MAXFILE];
//- static char  OpiumPSajout,OpiumPSfin;
static char  OpiumPStitre[OPIUM_MAXTITRE];
static int   OpiumPSx,OpiumPSy,OpiumPSytot,OpiumPSpage;
static float OpiumPSutilise;

static Dico OpiumDicoEnCours;
static Menu OpiumDicoMdial;
static long OpiumEvtNum;

#define OPIUM_HTML_NOM 16
static char OpiumHtmlVarName[OPIUM_MAXCDR][OPIUM_HTML_NOM];
static void OpiumHtmlPageDebut();
static void OpiumHtmlPageFin();

/* ========================================================================== */
int OpiumPSDemarre(),OpiumPSAjoute(),OpiumPSPage(),OpiumPSTermine();
static MenuItem OpiumPSitems[] = {
	{ "Creer et fermer", MNU_FONCTION OpiumPSComplet },
	{ "Premier dessin",  MNU_FONCTION OpiumPSDemarre },
	{ "Dessin suivant",  MNU_FONCTION OpiumPSAjoute },
	{ "Saut de page",    MNU_FONCTION OpiumPSPage },
	{ "Fin du fichier",  MNU_FONCTION OpiumPSTermine },
	{ "Quitter",         MNU_RETOUR },
	MNU_END
};
/* ========================================================================== */
static int OpiumDicoCherche(Menu menu, MenuItem *item);
static int OpiumDicoListe(Menu menu, MenuItem *item);
static MenuItem OpiumDicoIdial[] = {
	{ "Chercher expression", MNU_FONCTION OpiumDicoCherche },
	{ "Lister non traduits", MNU_FONCTION OpiumDicoListe },
	{ "Quitter",             MNU_RETOUR },
	MNU_END
};
/* ========================================================================== */
int PasImplementee() { OpiumError("Fonction pas encore implementee"); return(0); }
/* ========================================================================== */
int EnConstruction() { OpiumError("Chemin en travaux. Passage interdit!"); return(0); }
/* ========================================================================== */
#ifdef DEBUG_OPIUM4
int OpiumDebug(int opt, int level) {
	if((opt >= 0) && (opt < OPIUM_DEBUG_NBOPTS)) OpiumDebugLevel[opt] = level;
	return(1);
}
#else
int OpiumDebug(int opt, int level) { return(0); }
#endif
/* ========================================================================== */
int OpiumInit(char *display) {
/* Initialisation generale */
	int rc;
	int i;

	rc = WndInit(display);
	// printf("(OpiumInit) WndInit retourne %d\n",rc);
	RepertoireInit(OpiumHome); //WIN32: GetCurrentDirectory(MAXFILENAME,OpiumHome);
	bzero(OpiumHttpTitre,OPIUM_APP_NOM);

	OpiumMargeX = 0;
#ifdef MENU_BARRE
	OpiumMargeY = 44;
#else
	OpiumMargeY = WndLigToPix(1);
#endif
	// printf("(%s) Marges initiales: (%d, %d)\n",__func__,OpiumMargeX,OpiumMargeY);
#ifdef POSITION_PAR_TYPE
	OpiumPosPrevueY[OPIUM_MENU]  = OpiumMargeY;  OpiumPosPrevueX[OPIUM_MENU]  = OpiumMargeX;
	OpiumPosPrevueY[OPIUM_PANEL] = OpiumMargeY;  OpiumPosPrevueX[OPIUM_PANEL] = WndColToPix(40);
	OpiumPosPrevueY[OPIUM_TERM]  = WndLigToPix(15); OpiumPosPrevueX[OPIUM_TERM]  = WndColToPix(40);
	OpiumPosPrevueY[OPIUM_GRAPH] = WndLigToPix(20); OpiumPosPrevueX[OPIUM_GRAPH] = OpiumMargeX;
	OpiumPosPrevueY[OPIUM_INFO]  = OpiumServerHeigth(0) / 2;
	OpiumPosPrevueX[OPIUM_INFO]  = OpiumServerWidth(0) / 2 - WndColToPix(20);
	OpiumPosPrevueY[OPIUM_PROMPT] = WndLigToPix(20);
	OpiumPosPrevueX[OPIUM_PROMPT]  = OpiumServerWidth(0) / 2 - WndColToPix(20);
	OpiumPosPrevueY[OPIUM_BOARD] = OpiumMargeY;  OpiumPosPrevueX[OPIUM_BOARD] = OpiumMargeX;
	OpiumPosPrevueY[OPIUM_INSTRUM] = OpiumServerHeigth(0) * 3 / 4;
	OpiumPosPrevueX[OPIUM_INSTRUM] = OpiumMargeX;
	OpiumPosPrevueY[OPIUM_FIGURE] = OpiumServerHeigth(0) * 3 / 4;
	OpiumPosPrevueX[OPIUM_FIGURE] = OpiumMargeX;
	OpiumPosPrevueY[OPIUM_ONGLET] = OpiumServerHeigth(0) * 3 / 4;
	OpiumPosPrevueX[OPIUM_ONGLET] = OpiumMargeX;
#endif
	OpiumDernierClicX = OpiumDernierClicY = 0;
	for(i=0; i<OPIUM_NBTYPES; i++) OpiumOption[i] = 0;
	for(i=0; i<PNL_NBTYPES; i++) OpiumPanelResizable[i] = 0;
	if(!WndCodeHtml) {
		OpiumPanelResizable[PNL_PSWD] = 1;
		OpiumPanelResizable[PNL_FILE] = 1;
		OpiumPanelResizable[PNL_TEXTE] = 1;
		OpiumPanelResizable[PNL_LISTE] = 1;
		OpiumPanelResizable[PNL_LISTL] = 1;
		OpiumPanelResizable[PNL_LISTS] = 1;
		OpiumPanelResizable[PNL_LISTB] = 1;
		OpiumPanelResizable[PNL_KEY]  = 1;
		OpiumPanelResizable[PNL_KEYL] = 1;
		OpiumPanelResizable[PNL_KEYS] = 1;
		OpiumPanelResizable[PNL_KEYB] = 1;
	}

	OpiumNbCdr = 0;
	OpiumNbExec = 0;
	OpiumCdrMinimal = 0;
	OpiumHttpPort = 1953;
	OpiumHttpDesc = 0; OpiumHttpVars = 0;
	OpiumHtmlPageBandeau = 0;
	OpiumTableChaineGlobale = 0;
	OpiumNbRefresh = 0;
	OpiumLastRefreshed = 0;
	GraphUserSaisi = 0;
	OpiumPrioritaire = 0;
	OpiumMacroPhase = MACRO_NONE;
	OpiumUserPeriodic = 0;
	OpiumEntryPre = 0;
	OpiumEntryPost = 0;
	OpiumUseMouse = 1;
	OpiumNextTitle = 0;
//	OpiumReplyChoix = 0;
	OpiumLastEvent.type = -1; /* rapport au cas WND_PRESS teste dans WndEventNew */
	OpiumZoomAutorise = 0;
	BoardZoneNb = 0;
	MenuEnBarre = 0;
	OpiumPSzoome = 0;
	OpiumPSenCours = 0;
	OpiumDernierMessage[0] = '\0';

	OpiumOui = "Oui"; OpiumNon = "Non"; OpiumChoixBool = "non/oui";
	OpiumListeVide = "(vide)";
	OpiumListeInvl = "INVALIDE!";
	strcpy(GraphErrorText,"jusque la, ca va...");
	OpiumReplyStandard = OpiumPanelReply;
	OpiumPromptTitre = OpiumPromptNom;
	OpiumPromptFmtBrut = 0;

	strcpy(OpiumJpegEspace,"~/Desktop"); RepertoireTermine(OpiumJpegEspace);
	// printf("(%s) Photos sauvees dans %s\n",__func__,OpiumJpegEspace);
	OpiumTxtPath[0] = OpiumTxtNom[0] = '\0';
	OpiumPSpath[0] = OpiumPSnom[0] = '\0'; //- OpiumPSajout = OpiumPSfin = 0;
	OpiumPStitre[0] = '\0';

	OpiumAscenseurEnCours = 0;
	OpiumZoomEnCours = 0;  OpiumZoom0[0] = OpiumZoom0[1] = 0;
	DicoInit(&OpiumDico,"OpiumDico",0);
	OpiumDicoAllonge = 0;
	OpiumDicoEnCours = 0;
	OpiumDicoMdial = MenuLocalise(OpiumDicoIdial);
	OpiumEvtNum = 0;

#ifdef DEBUG_OPIUM4
	for(i=0; i<WND_NBEVENTS; i++) sprintf(OpiumEventType[i],"EventType#%d",i);
	strcpy(OpiumEventType[WND_EXPOSE],"Expose");
	strcpy(OpiumEventType[WND_MOVED],"Moved");
	strcpy(OpiumEventType[WND_RESIZE],"Resize");
	strcpy(OpiumEventType[WND_CONFIG],"Config");
	strcpy(OpiumEventType[WND_IN_USE],"FocusIn");
	strcpy(OpiumEventType[WND_DELETE],"Delete");
	strcpy(OpiumEventType[WND_KEY],"KeyPress");
	strcpy(OpiumEventType[WND_NOKEY],"KeyRelease");
	strcpy(OpiumEventType[WND_PRESS],"MousePress");
	strcpy(OpiumEventType[WND_RELEASE],"MouseRelease");
#ifdef WND_SCROLLX
	strcpy(OpiumEventType[WND_SCROLLX],"ScrollX");
#endif
#ifdef WND_SCROLLY
	strcpy(OpiumEventType[WND_SCROLLY],"ScrollY");
#endif
#ifdef WND_BARRE
	strcpy(OpiumEventType[WND_BARRE],"Barre");
#endif

	for(i=0; i<WND_NBBUTTONS; i++) strcpy(OpiumMseButton[i],"Unk'nButton");
	strcpy(OpiumMseButton[WND_MSELEFT],"MouseLeft");
	strcpy(OpiumMseButton[WND_MSEMIDDLE],"MouseMiddle");
	strcpy(OpiumMseButton[WND_MSERIGHT],"MouseRight");

	for(i=0; i<OPIUM_DEBUG_NBOPTS; i++) OpiumDebugLevel[i] = 0;
#endif

/*  a tracer avec WndPoly */
	OpiumMark[GRF_POINT][1].x =  GRF_DIM_POINT; OpiumMark[GRF_POINT][1].y =  0;
	OpiumMark[GRF_POINT][2].x =  0;             OpiumMark[GRF_POINT][2].y =  GRF_DIM_POINT;
	OpiumMark[GRF_POINT][3].x = -GRF_DIM_POINT; OpiumMark[GRF_POINT][3].y =  0;
	OpiumMark[GRF_POINT][4].x =  0;             OpiumMark[GRF_POINT][4].y = -GRF_DIM_POINT;

/*  a tracer avec GraphMarkDraw */
	OpiumMark[GRF_CROSS][1].x =  0;                 OpiumMark[GRF_CROSS][1].y =  GRF_DIM_CROSS;
	OpiumMark[GRF_CROSS][2].x = -GRF_DIM_CROSS / 2; OpiumMark[GRF_CROSS][2].y = -GRF_DIM_CROSS / 2;
	OpiumMark[GRF_CROSS][3].x =  GRF_DIM_CROSS;     OpiumMark[GRF_CROSS][3].y =  0;

/*  a tracer avec GraphMarkDraw */
	OpiumMark[GRF_STAR][1].x =  0;                OpiumMark[GRF_STAR][1].y =  GRF_DIM_STAR;
	OpiumMark[GRF_STAR][2].x =  GRF_DIM_STAR / 2; OpiumMark[GRF_STAR][2].y =  0;
	OpiumMark[GRF_STAR][3].x = -GRF_DIM_STAR;     OpiumMark[GRF_STAR][3].y = -GRF_DIM_STAR;
	OpiumMark[GRF_STAR][4].x =  0;                OpiumMark[GRF_STAR][4].y =  GRF_DIM_STAR / 2;
	OpiumMark[GRF_STAR][5].x =  GRF_DIM_STAR;     OpiumMark[GRF_STAR][5].y =  0;
	OpiumMark[GRF_STAR][6].x =  0;                OpiumMark[GRF_STAR][6].y = -GRF_DIM_STAR / 2;
	OpiumMark[GRF_STAR][7].x = -GRF_DIM_STAR;     OpiumMark[GRF_STAR][7].y =  GRF_DIM_STAR;

/*  a tracer avec WndPoly */
	OpiumMark[GRF_TRGLE][1].x =  GRF_DIM_TRGLE / 2; OpiumMark[GRF_TRGLE][1].y =  GRF_DIM_TRGLE;
	OpiumMark[GRF_TRGLE][2].x = -GRF_DIM_TRGLE;     OpiumMark[GRF_TRGLE][2].y =  0;
	OpiumMark[GRF_TRGLE][3].x =  GRF_DIM_TRGLE / 2; OpiumMark[GRF_TRGLE][3].y = -GRF_DIM_TRGLE;

/*  a tracer avec WndPoly */
	OpiumMark[GRF_CARRE][1].x =  GRF_DIM_CARRE; OpiumMark[GRF_CARRE][1].y =  0;
	OpiumMark[GRF_CARRE][2].x =  0;             OpiumMark[GRF_CARRE][2].y =  GRF_DIM_CARRE;
	OpiumMark[GRF_CARRE][3].x = -GRF_DIM_CARRE; OpiumMark[GRF_CARRE][3].y =  0;
	OpiumMark[GRF_CARRE][4].x =  0;             OpiumMark[GRF_CARRE][4].y = -GRF_DIM_CARRE;

/*  a tracer avec WndPoly */
	OpiumMark[GRF_ROND][1].x =  GRF_DIM_ROND / 2;  OpiumMark[GRF_ROND][1].y =  0;
	OpiumMark[GRF_ROND][2].x =  GRF_DIM_ROND / 4;  OpiumMark[GRF_ROND][2].y =  GRF_DIM_ROND / 4;
	OpiumMark[GRF_ROND][3].x =  0;                 OpiumMark[GRF_ROND][3].y =  GRF_DIM_ROND / 2;
	OpiumMark[GRF_ROND][4].x = -GRF_DIM_ROND / 4;  OpiumMark[GRF_ROND][4].y =  GRF_DIM_ROND / 4;
	OpiumMark[GRF_ROND][5].x = -GRF_DIM_ROND / 2;  OpiumMark[GRF_ROND][5].y =  0;
	OpiumMark[GRF_ROND][6].x = -GRF_DIM_ROND / 4;  OpiumMark[GRF_ROND][6].y = -GRF_DIM_ROND / 4;
	OpiumMark[GRF_ROND][7].x =  0;                 OpiumMark[GRF_ROND][7].y = -GRF_DIM_ROND / 2;
	OpiumMark[GRF_ROND][8].x =  GRF_DIM_ROND / 4;  OpiumMark[GRF_ROND][8].y = -GRF_DIM_ROND / 4;

/* Attention a la declaration d'OpiumMark[][9] => pas plus que 8 segments, ou alors augmenter [9] */

/*  a tracer avec WndPoly */
	OpiumPointPapier[1].x =  2 * GRF_DIM_POINT; OpiumPointPapier[1].y =  0;
	OpiumPointPapier[2].x =  0;                 OpiumPointPapier[2].y =  2 * GRF_DIM_POINT;
	OpiumPointPapier[3].x = -2 * GRF_DIM_POINT; OpiumPointPapier[3].y =  0;
	OpiumPointPapier[4].x =  0;                 OpiumPointPapier[4].y = -2 * GRF_DIM_POINT;

	if(rc) {
		// printf("  Creation d'un fenetre racine nommee '%s'\n",__progname);
		// WndNewRoot(__progname,9999,9999);

#ifdef CHANGE_CURSEUR
		OpiumCurseurMain = WndCreateStdCursor((int)WND_CURS_FLECHE_GA);
		OpiumCurseurViseur = WndCreateStdCursor((int)WND_CURS_CROIX);
#endif
		
		OpiumRouge  = WndColorGetFromRGB(GRF_RGB_RED);
		OpiumOrange = WndColorGetFromRGB(GRF_RGB_ORANGE);
		OpiumJaune  = WndColorGetFromRGB(GRF_RGB_YELLOW);
		OpiumVert   = WndColorGetFromRGB(GRF_RGB_GREEN);
		
		OpiumCdrPrompt = OpiumCreate(OPIUM_PROMPT,0);
		if(!OpiumCdrPrompt) {
			WndPrint("Ca demarre tres mal!!! OpiumCdrPrompt n'est meme pas cree...\n");
			return(0);
		}
		OpiumMacroInfo = InfoCreate(1,70);
		OpiumPanelQuick = PanelCreate(1);
		GraphDialogCreate();

		OpiumEditSurligne[0] = '\0';
		OpiumEditCar0 = -1;
		OpiumGlissante = 0;
		GraphQualite = WND_Q_PAPIER;
		GraphWidzDefaut[WND_Q_PAPIER] = 3; GraphWidzDefaut[WND_Q_ECRAN] = 1;
		GraphArrayLarg = 400; GraphArrayHaut = 300; GraphArrayEnCours = 0;

		OpiumTxtfile  = 0;

		OpiumPSmenu = MenuCreate(OpiumPSitems);
		OpiumPSpanel = PanelCreate(2);
		PanelFile(OpiumPSpanel,"Sur le fichier",OpiumPSnom,OPIUM_MAXFILE);
		PanelText(OpiumPSpanel,"Titre general",OpiumPStitre,OPIUM_MAXTITRE);
	}

	{
		int j,k,l,n,s; int dim;

		dim = OPIUM_COLORS_AUTO;
		k = 0; j = 0;
		n = dim / 5; l = n; s = WND_COLOR_MAX / n;
		for(; k<l; k++, j+=s) { OpiumColorVal[k].r = 0; OpiumColorVal[k].g = j; OpiumColorVal[k].b = WND_COLOR_MAX; }
		l += n;
		for(; k<l; k++, j-=s) { OpiumColorVal[k].r = 0; OpiumColorVal[k].g = WND_COLOR_MAX; OpiumColorVal[k].b = j; }
		l += n;
		for(; k<l; k++, j+=s) { OpiumColorVal[k].r = j; OpiumColorVal[k].g = WND_COLOR_MAX; OpiumColorVal[k].b = 0; }
		l += n;
		for(; k<l; k++, j-=s) { OpiumColorVal[k].r = WND_COLOR_MAX; OpiumColorVal[k].g = j; OpiumColorVal[k].b = 0; }
		l += n;
		for(; k<l; k++, j+=s) { OpiumColorVal[k].r = WND_COLOR_MAX; OpiumColorVal[k].g = 0; OpiumColorVal[k].b = j; }
		for(; k<dim; k++)     { OpiumColorVal[k].r = WND_COLOR_MAX; OpiumColorVal[k].g = WND_COLOR_MAX; OpiumColorVal[k].b = WND_COLOR_MAX; }
		OpiumColorVal[0].r= 0; OpiumColorVal[0].g = 0; OpiumColorVal[0].b = 0;
	}
	/* {
		char *pomme,*c;
		pomme = "ð";
		printf("(%s) Symbole pomme: ",__func__);
		c = pomme; while(*c) printf("%02X",(unsigned char)*c++);
		printf("\n");
	} */
	return(rc);
}
/* ========================================================================== */
OpiumServer OpiumOpen(char *display) {
/* Ouvre un nouveau serveur (devient courant). Rend un pointeur sur ce serveur */
	OpiumServer svr;

	svr = (OpiumServer)malloc(sizeof(WndServer));
	if(WndOpen(svr,display) == BON) return(svr); else return(0);
}
/* ========================================================================== */
void OpiumAppCode(char *nom) { strncpy(OpiumHttpTitre,nom,OPIUM_APP_NOM-1); }
/* ========================================================================== */
void OpiumPort(int port) { OpiumHttpPort = port; }
/* ========================================================================== */
void OpiumClose(OpiumServer svr) { free(svr); }
/* ========================================================================== */
void OpiumServerCurrent(OpiumServer svr) {
/* Defini <svr> comme serveur courant, ou bien:
   defini le serveur par defaut comme serveur courant si <svr> = 0 */
	if(svr) WndCurSvr = svr; else WndCurSvr = &WndDefSvr;
}
/* ========================================================================== */
void OpiumServerDefault(OpiumServer svr) {
/* Defini <svr> comme serveur par defaut */
	memcpy(&WndDefSvr,svr,sizeof(WndServer));
}
/* ========================================================================== */
int OpiumServerWidth(OpiumServer svr) {
/* Renvoie la largeur en pixels de l'ecran du serveur <svr>, ou bien
   renvoie la largeur en pixels de l'ecran du serveur courant si <svr> = 0 */
	if(svr) return(WndTotalWidth(svr)); else return(WndTotalWidth(WndCurSvr));
}
/* ========================================================================== */
int OpiumServerHeigth(OpiumServer svr) {
/* Renvoie la hauteur en pixels de l'ecran du serveur <svr>, ou bien
   renvoie la hauteur en pixels de l'ecran du serveur courant si <svr> = 0 */
	if(svr) return(WndTotalHeigth(svr)-OpiumMargeY); else return(WndTotalHeigth(WndCurSvr)-OpiumMargeY);
}
/* ========================================================================== */
void OpiumExit() {
	int i;

	for(i=0; i<OpiumNbCdr; i++ ) OpiumDelete(OpiumCdr[i]);
	OpiumNbCdr = 0;
	WndExit();
}
/* ========================================================================== */
char OpiumDicoUtilise(char *fichier, DICO_MODE mode) {
	// DicoLocaleListeLibere OpiumReplyStandard si >1 appel de OpiumDicoUtilise
	int rc;

	if(OpiumDico.nbtermes && (fichier[0] == '\0')) {
		DicoReset(&OpiumDico,0,0); return(1);
	} else {
		DicoRaz(&OpiumDico); rc = DicoLit(&OpiumDico,fichier);
		if(rc) {
			DicoEnrichi(&OpiumDico,mode);
			
			/* !!! TOUTES CES VARIABLES DOIVENT ETRE PRE_INITIALISEES DANS OpiumInit() */
			OpiumOui = L_("Oui");
			OpiumNon = L_("Non");
			OpiumChoixBool = L_("non/oui");
			strcpy(GraphErrorText,L_("jusque la, ca va..."));
			OpiumListeVide = L_("(vide)");
			OpiumListeInvl = L_("INVALIDE!");
			OpiumReplyStandard = LL_(OpiumPanelReply);
			OpiumPromptTitre = LL_(OpiumPromptNom);
			
			DicoInsere(&OpiumDico,"chemin","");
			DicoInsere(&OpiumDico,"fichier","");
			DicoInsere(&OpiumDico,"Fermer","");
			DicoInsere(&OpiumDico,"Rep","");
			DicoInsere(&OpiumDico,"Repert","");
			DicoInsere(&OpiumDico,"Repertoires","");
			DicoInsere(&OpiumDico,"Fichiers","");
			// printf("* %d terme%s OPIUM defini%s\n",Accord2s(OpiumDico.nbtermes));
		}
		return(rc);
	}
}
/* ========================================================================== */
char OpiumDicoSauve(char *fichier) {
	return(DicoSauve(&OpiumDico,fichier));
}
/* ========================================================================== */
void OpiumSetMouse(char flag) {
	OpiumUseMouse = flag; 
}
/* ========================================================================== */
static void OpiumAfficheMessage() {
	if(OpiumDernierMessage[0] == '!') OpiumWarn(OpiumDernierMessage);
	else if(OpiumDernierMessage[0] == '*') OpiumNote(OpiumDernierMessage+2);
}
/* ========================================================================== */
Cadre OpiumCreate(int type, void *adrs) {
/* Creation d'un Cadre */
	Cadre cdr;
	int i,k;

	cdr = (Cadre)malloc(sizeof(TypeCadre));
	if(!cdr) {
		WndPrint("(OpiumCreate) manque %ld octets pour le cadre #%d\n",sizeof(TypeCadre),OpiumNbCdr);
		return(0);
	}

	if(OpiumNbCdr >= OPIUM_MAXCDR) {
		WndPrint("(OpiumCreate) Deja %d cadres crees, cadre @%08X abandonne\n",OpiumNbCdr,(hexa)cdr);
		return(0);
	}
	OpiumCdr[OpiumNbCdr++] = cdr;
	if(DEBUG_OPIUM(2)) WndPrint("Creation cadre #%d @%08X (= %s @%08X)\n",
		OpiumNbCdr,(hexa)cdr,OpiumCdrType[type],(hexa)adrs);

	cdr->f = 0;
	cdr->displayed = 0;
	cdr->planche = 0;
	cdr->actif = 0;
	cdr->bouton = 0;
	cdr->nb_boutons = 0;
	cdr->suivant = 0;
	cdr->support = WND_RIEN;
	cdr->debranche = 0;
	cdr->var_modifiees = 0;
	cdr->type = (char)type;
	cdr->adrs = adrs;
	cdr->titre[0] = '\0';
	cdr->nom[0] = '\0';
	k = 0; for(i=0; i<OpiumNbCdr; i++) if(OpiumCdr[i]->type == type) k++;
	cdr->num = k;
	cdr->enter = 0;          /* enter(cdr) (!= 0 si cdr abandonne)  */
	cdr->exit = 0;           /* exit(cdr,arg) (=nouveau retour)     */
	cdr->clear = 0;          /* clear(cdr,arg) (=nouveau retour)    */
	cdr->a_vide = 0;         /* a_vide(cdr,e,arg) (=nouveau retour) */
	cdr->code_rendu = 0;
	for(i=0; i<OPIUM_MAXKEYS; i++) {
		cdr->key[i].code = 0;
		cdr->key[i].fctn = 0;
	}
	cdr->defaultx = cdr->defaulty = cdr->defaulth = cdr->defaultv = OPIUM_AUTOPOS;
	cdr->pos_std_x = cdr->pos_std_y = 0;
	cdr->ref_pos = 0;
	cdr->haut = cdr->larg = WND_ASCINT_WIDZ;
	cdr->x0 = cdr->y0 = cdr->x = cdr->y = 0;
	cdr->dx = cdr->dy = 0; cdr->dh = cdr->dv = WND_ASCINT_WIDZ;
	cdr->modexec = OPIUM_DISPLAY;
	cdr->log = 0;
#ifdef TIMER_NIVEAU_OPIUM
#ifdef TIMER_PAR_EVT
	cdr->timer = 0;
#endif
#endif
	cdr->delai = cdr->restant = 0;
	cdr->fond = 0;
	return(cdr);
}
#ifdef A_FINIR
/* ========================================================================== */
OpiumCopy(Cadre cdr) {
	Menu m; Panel p; Term t; Graph g; Info j; Prompt q; Board b; Instrum i; 
	Figure f; Onglet o;
	void *adrs;
	Cadre copie;

	switch(cdr->type) {
	  case OPIUM_MENU:
		m = (Menu)cdr->adrs;
		adrs = (void *)MenuCreate(m->items);
		copie = ((Menu)adrs)->cdr;
		memcpy(adrs,m,sizeof(TypeMenu));
		((Menu)adrs)->cdr = copie;
		break;
	  case OPIUM_PANEL:
		p = (Panel)cdr->adrs;
		adrs = (void *)PanelCreate(p->max);
		copie = ((Panel)adrs)->cdr;
		memcpy(adrs,p,sizeof(TypePanel));
		((Panel)adrs)->cdr = copie;
		break;
	  case OPIUM_TERM:
	  	t = (Term)cdr->adrs;
	  	adrs = (void *)TermCreate(t->haut,t->larg,t->taille);
	  	copie = ((Term)adrs)->cdr;
		memcpy(adrs,t,sizeof(TypeTerm));
		((Term)adrs)->cdr = copie;
		break;
	  case OPIUM_GRAPH:
		g = (Graph)cdr->adrs;
		adrs = (void *)GraphCreate(g->larg,g->haut,g->max);
		copie = ((Graph)adrs)->cdr;
		memcpy(adrs,m,sizeof(TypeMenu));
		((Graph)adrs)->cdr = copie;
		break;
	  case OPIUM_INFO:
		j = (Info)cdr->adrs;
		adrs = (void *)InfoCreate(j->haut,j->larg);
		copie = ((Info)adrs)->cdr;
		memcpy(adrs,m,sizeof(TypeMenu));
		((Info)adrs)->cdr = copie;
		break;
	  case OPIUM_PROMPT:
		return(OpiumCdrPrompt);
		break;
	  case OPIUM_BOARD:
		copie = BoardCreate();
		adrs = copier->adrs;??
		memcpy(adrs,m,sizeof(...));
		((...)adrs)->cdr = copie;
		break;
	  case OPIUM_INSTRUM:
		i = (Instrum)cdr->adrs;
		adrs = (void *)InstrumAdd(i->forme,i->parms);
		copie = ((Instrum)adrs)->cdr;
		memcpy(adrs,m,sizeof(TypeMenu));
		((Menu)adrs)->cdr = copie;
		break;
	  case OPIUM_FIGURE:
		f = (Figure)cdr->adrs;
		adrs = (void *)FigureCreate(...);
		copie = ((Figure)adrs)->cdr;
		memcpy(adrs,f,sizeof(TypeFigure));
		((Figure)adrs)->cdr = copie;
		break;
	  case OPIUM_ONGLET:
		o = (Onglet)cdr->adrs;
		adrs = (void *)OngletDefini(...);
		copie = ((Onglet)adrs)->cdr;
		memcpy(adrs,o,sizeof(TypeOnglet));
		((Onglet)adrs)->cdr = copie;
		break;
	}
	memcpy(copie,cdr,sizeof(TypeCadre));
	copie->adrs = adrs;
	cdr->f = 0;
	cdr->displayed = 0;
	cdr->planche = 0;
	cdr->actif = 0;
	cdr->suivant = 0;
	return(copie);
}
#endif
/* ========================================================================== */
int OpiumDelete(Cadre cdr) {
	int i;

	if(!cdr) return(0);
	if(DEBUG_OPIUM(1)) WndPrint("Destruction cadre @%08X (= %s @%08X)\n",
		(hexa)cdr,cdr->nom,(hexa)cdr->adrs);
	if(cdr->f) OpiumClear(cdr);
	for(i=0; i<OpiumNbCdr; i++) if(OpiumCdr[i] == cdr) break;
	if(i == OpiumNbCdr) {
		WndPrint("(%s) ERREUR SYSTEME, %s '%s' non repertorie @%08X!!\n",
			__func__,OpiumCdrType[(int)cdr->type],cdr->nom,(hexa)cdr);
	} else {
		OpiumNbCdr--; for( ; i<OpiumNbCdr; i++) OpiumCdr[i] = OpiumCdr[i+1];
	}
	free(cdr);
	return(1);
}
/* ========================================================================== */
void OpiumName(Cadre cdr, char *nom) {
	if(!cdr) return;
	if(!nom) { cdr->nom[0] = '\0';return; }
	strncpy(cdr->nom,nom,OPIUM_MAXNOM);
	cdr->nom[OPIUM_MAXNOM - 1] = '\0';
}
/* ========================================================================== */
void OpiumTitle(Cadre cdr, char *titre) {
	if(!cdr) return;
	if(titre) {
		strncpy(cdr->titre,titre,OPIUM_MAXTITRE);
		cdr->titre[OPIUM_MAXTITRE - 1] = '\0';
	} else cdr->titre[0] = '\0';
	OpiumName(cdr,titre);
}
/* ========================================================================== */
void OpiumGetTitle(Cadre cdr, char *titre) {
	if(cdr) strncpy(titre,cdr->titre,OPIUM_MAXTITRE);
}
/* ========================================================================== */
void OpiumOptions(int type, TypeOpiumFctn fctn) {
	OpiumOption[type] = fctn; 
}
/* ========================================================================== */
void OpiumPeriodicFctn(TypeFctn fctn) {
	OpiumUserPeriodic = fctn;
}
/* ========================================================================== */
void OpiumEnterFctn(Cadre cdr, TypeOpiumFctnArg fctn, void *arg) {
	if(cdr) { cdr->enter = fctn; cdr->enter_arg = arg; }
}
/* ========================================================================== */
void OpiumExitFctn(Cadre cdr, TypeOpiumFctnArg fctn, void *arg) {
	if(cdr) { cdr->exit = fctn;  cdr->exit_arg = arg; }
}
/* ========================================================================== */
void OpiumClearFctn(Cadre cdr, TypeOpiumFctnArg fctn, void *arg) {
	if(cdr) { cdr->clear = fctn;  cdr->clear_arg = arg; }
}
/* ========================================================================== */
void *OpiumEnterArg(Cadre cdr) { return(cdr? cdr->enter_arg: 0); }
/* ========================================================================== */
void *OpiumExitArg(Cadre cdr) { return(cdr? cdr->exit_arg: 0); }
/* ========================================================================== */
void *OpiumClearArg(Cadre cdr) { return(cdr? cdr->clear_arg: 0); }
/* ========================================================================== */
void OpiumMinimum(Cadre cdr) { OpiumCdrMinimal = cdr; if(cdr) WndMinimale(cdr->f); }
/* ========================================================================== */
void OpiumBranche(Cadre cdr, char branche) { if(cdr) cdr->debranche = !branche; }
/* ========================================================================== */
int OpiumKeyEnable(Cadre cdr, int code, TypeOpiumFctnInt fctn) {
	int i,j;

	if(!code || !cdr) return(0);
	for(i=0; i<OPIUM_MAXKEYS; i++) {
		if(cdr->key[i].code == code) break;
		if(!cdr->key[i].code) break;
	}
	if(i == OPIUM_MAXKEYS) return(0);
	if(fctn) {
		cdr->key[i].code = code;
		cdr->key[i].fctn = fctn;
	} else {
		if(!cdr->key[i].code) return(0);
		for(j=i ; j<OPIUM_MAXKEYS; ) {
			j++;
			cdr->key[i].code = cdr->key[j].code;
			cdr->key[i].fctn = cdr->key[j].fctn;
			if(!cdr->key[i].code) break;
			i = j;
		}
	}
	return(1);
}
/* ========================================================================== */
void OpiumPosition(Cadre cdr, int x, int y) {
	if(!cdr) return;
	cdr->defaultx = cdr->x = x;
	cdr->defaulty = cdr->y = y;
}
/* ========================================================================== */
void OpiumSize(Cadre cdr, int h, int v) {
	if(!cdr) return;
	cdr->defaulth = h;
	cdr->defaultv = v;
}
/* ========================================================================== */
void OpiumGetPosition(Cadre cdr, int *x, int *y) {
	if(!cdr) return;
	if(cdr->f) { *x = cdr->x; *y = cdr->y; } 
	else { *x = cdr->defaultx; *y = cdr->defaulty; }
}
/* ========================================================================== */
void OpiumGetSize(Cadre cdr, int *h, int *v) {
	if(!cdr) return;
	*h = cdr->dh;
	*v = cdr->dv;
}
/* ========================================================================== */
void OpiumSizeChanged(Cadre cdr) {
	if(!cdr) return;
	if(DEBUG_OPIUM(1)) WndPrint("(%s) Evaluation de la taille de %08X/%-12s\n",__func__,(hexa)cdr,cdr->nom);
	switch(cdr->type) {
		case OPIUM_MENU:    OpiumSizeMenu(cdr,0);    break;
		case OPIUM_PANEL:   OpiumSizePanel(cdr,0);   break;
		case OPIUM_TERM:    OpiumSizeTerm(cdr,0);    break;
		case OPIUM_GRAPH:   OpiumSizeGraph(cdr,0);   break;
		case OPIUM_INFO:    OpiumSizeInfo(cdr,0);    break;
		case OPIUM_PROMPT:  OpiumSizePrompt(cdr,0);  break;
		case OPIUM_BOARD:   OpiumSizeBoard(cdr,0);   break;
		case OPIUM_INSTRUM: OpiumSizeInstrum(cdr,0); break;
		case OPIUM_FIGURE:  OpiumSizeFigure(cdr,0);  break;
		case OPIUM_ONGLET:  OpiumSizeOnglet(cdr,0);  break;
	}
	
	if(cdr->defaulth != OPIUM_AUTOPOS) cdr->dh = cdr->defaulth;
	if(cdr->defaultv != OPIUM_AUTOPOS) cdr->dv = cdr->defaultv;
	if(cdr->dx >= cdr->larg) cdr->dx = cdr->larg - cdr->dh; if(cdr->dx < 0) cdr->dx = 0;
	if(cdr->dy >= cdr->haut) cdr->dy = cdr->haut - cdr->dv; if(cdr->dy < 0) cdr->dy = 0;
	if(DEBUG_OPIUM(1)) WndPrint("(%s) Dims definitives: [%d x %d]\n",__func__,cdr->dh,cdr->dv);
}
/* ========================================================================== */
void OpiumDrawingColor(Cadre cdr, WndContextPtr *gc, char *nom) {
	WndContextPtr local;

	if(!cdr) return;
	local = *gc;
	if(!local) {
		local = WndContextCreate(cdr->f);
		if(local) {
			if(!WndContextFgndName(cdr->f,local,nom)) {
				WndContextFree(cdr->f,local); local = 0;
			}
		} 
		*gc = local;
	}
}
/* ========================================================================== */
void OpiumDrawingRGB(Cadre cdr, WndContextPtr *gc,
	WndColorLevel r, WndColorLevel g, WndColorLevel b) {
	WndContextPtr local;

	if(!cdr) return;
	local = *gc;
	if(!local) {
		local = WndContextCreate(cdr->f);
		if(local) {
			if(!WndContextFgndRGB(cdr->f,local,r,g,b)) {
				WndContextFree(cdr->f,local); local = 0;
			}
		} 
		*gc = local;
	}
}
/* ========================================================================== */
void OpiumDrawingBgndRGB(Cadre cdr, WndContextPtr *gc,
	WndColorLevel r, WndColorLevel g, WndColorLevel b) {
	WndContextPtr local;
	
	if(!cdr) return;
	local = *gc;
	if(!local) {
		local = WndContextCreate(cdr->f);
		if(local) {
			if(!WndContextBgndRGB(cdr->f,local,r,g,b)) {
				WndContextFree(cdr->f,local); local = 0;
			}
		} 
		*gc = local;
	}
}
/* ========================================================================== */
void OpiumColorInit(OpiumColorState *s) { s->max = 2; s->num = 1; }
/* ========================================================================== */
int OpiumColorNext(OpiumColorState *s) {
	int k;

	if(!s) return(0);
	k = (OPIUM_COLORS_AUTO / s->max) * s->num;
	s->num += 2;
	if(s->num > s->max) { if(s->max < OPIUM_COLORS_AUTO) s->max *= 2; s->num = 1; }
	return(k);
}
/* ========================================================================== */
void OpiumColorAssign(OpiumColor *c, WndColorLevel r, WndColorLevel g, WndColorLevel b) {
	c->r = r; c->g = g; c->b = b;
}
/* ========================================================================== */
void OpiumColorArcEnCiel(OpiumColor *lut, int dim) {
	/* Jeu de <dim> couleurs */
	int i,j,l,n,s; OpiumColor *elt;
	
	n = dim / 5; s = WND_COLOR_MAX / n;
	i = 0; j = 0; elt = lut; l = n;
	for(; i<l; i++, j+=s) OpiumColorAssign(elt++,0,j,WND_COLOR_MAX); l += n;
	for(; i<l; i++, j-=s) OpiumColorAssign(elt++,0,WND_COLOR_MAX,j); l += n;
	for(; i<l; i++, j+=s) OpiumColorAssign(elt++,j,WND_COLOR_MAX,0); l += n;
	for(; i<l; i++, j-=s) OpiumColorAssign(elt++,WND_COLOR_MAX,j,0); l += n;
	for(; i<l; i++, j+=s) OpiumColorAssign(elt++,WND_COLOR_MAX,0,j);
	for(; i<dim; i++) OpiumColorAssign(elt++,WND_COLOR_MAX,WND_COLOR_MAX,WND_COLOR_MAX);
	OpiumColorAssign(lut,0,0,0);
}
/* ========================================================================== */
void OpiumBouton(Cadre cdr, WndFrame f, char *texte) {
	if(!cdr) return;
	WndButton(f,WND_TEXT,WndPixToLig(cdr->dv) - 1,0,WndPixToCol(cdr->dh),texte);
}
/* ========================================================================== */
void OpiumBoutonsScrolles(Cadre cdr, WndFrame f, int premier, int nb, char *texte[]) {
	int x,y,h,k,i;
	
	if(!cdr) return;
//	cdr->ligmin = (cdr->dy)? WndPixToLig(cdr->dy - 1) + 1: 0;
//	cdr->colmin = (cdr->dx)? WndPixToCol(cdr->dx - 1) + 1: 0;
	x = -cdr->colmin;
	y = WndPixToLig(cdr->haut) - 1 - cdr->ligmin;
	h = WndPixToCol(cdr->larg) / nb;
	i = premier;
	for(k=0; k<nb; k++) {
		if(x >= 0) WndButton(f,WND_TEXT,y,x,h,texte[i]);
		i++; x += h;
	}
}
/* ========================================================================== */
void OpiumBoutonsTjrs(Cadre cdr, WndFrame f, int premier, int nb, char *texte[]) {
	int x,y,h,k,i;
	
	if(!cdr) return;
	x = 0;
	y = WndPixToLig(cdr->dv) - 1;
	h = WndPixToCol(cdr->dh) / nb;
	i = premier;
	for(k=0; k<nb; k++) {
		WndButton(f,WND_TEXT,y,x,h,texte[i]);
		i++; x += h;
	}
}
/* ========================================================================== */
void OpiumSupport(Cadre cdr, char support) { if(cdr) cdr->support = support; }
/* ========================================================================== */
void OpiumMode(Cadre cdr, char modexec) { if(cdr) cdr->modexec = modexec; }
/* ========================================================================== */
void OpiumLog(Cadre cdr, char log) { if(cdr) cdr->log = log; }
/* ========================================================================== */
void OpiumEntryBefore(int (*fctn)()) { OpiumEntryPre = fctn; }
/* ========================================================================== */
void OpiumEntryAfter(int (*fctn)(int)) { OpiumEntryPost = fctn; }
/* ========================================================================== */
/* ..................... fonctions a usage interne .......................... */
/* .......................................................................... */
double OpiumGradsLineaires(double echelle) {
	double mag,nbchiffres,approx,nbgrad,grad;
	char negatif;

	negatif = 0;
	if(echelle < 0.0) { negatif = 1; echelle = -echelle; }
	else if(echelle == 0.0) echelle = 1.0;
	mag = log10(echelle);
	modf(mag,&nbchiffres); if(mag < 0.0) nbchiffres -= 1.0;
	approx = pow(10.0,nbchiffres);
	grad = approx;
	modf(echelle/grad,&nbgrad);
	if(nbgrad < 5.0) {
		grad /= 2.0;
		modf(echelle/grad,&nbgrad); 
		if(nbgrad < 5.0) {
			grad /= 2.5; /* modf(echelle/grad,&nbgrad); */
		}
	}
	return(negatif? -grad: grad);
}
/* .......................................................................... */
void OpiumSetOptions(Cadre cdr) {
	TypeOpiumFctn fctn;
	if((fctn = OpiumOption[(int)cdr->type])) (*fctn)(cdr);
}
/* .......................................................................... */
static int OpiumPosPrepareX(Cadre cdr) {
/* prochaine position en x */
#ifdef POSITION_PAR_TYPE
	return(OpiumPosPrevueX[(int)cdr->type]);
#else
	int courant;
	courant = OpiumProchainX;
	if(courant < 10) courant = 10;
	// OpiumProchainX += WndColToPix(2);
	if(OpiumProchainX >  OpiumServerWidth(0)) OpiumProchainX = 10;
	return(courant);
#endif
}
/* .......................................................................... */
static int OpiumPosPrepareY(Cadre cdr) {
	int courant;
/* prochaine position en y */

#ifdef POSITION_PAR_TYPE
	courant = OpiumPosPrevueY[(int)cdr->type];
	switch(cdr->type) {
	  case OPIUM_MENU:
	  case OPIUM_TERM:
	  case OPIUM_BOARD:
		OpiumPosPrevueY[(int)cdr->type] += WndLigToPix(2);
		break;
	  case OPIUM_PANEL:
		OpiumPosPrevueY[(int)cdr->type] += cdr->haut + OpiumMargeY + WND_ASCINT_WIDZ;
		break;
	  case OPIUM_INFO:
	  case OPIUM_PROMPT:
	  case OPIUM_GRAPH:
	  case OPIUM_INSTRUM:
	  case OPIUM_FIGURE:
	  case OPIUM_ONGLET:
		OpiumPosPrevueY[(int)cdr->type] += cdr->haut + WndLigToPix(2) + WND_ASCINT_WIDZ;
		break;
	}
	if(OpiumPosPrevueY[(int)cdr->type] >  OpiumServerHeigth(0)) {
		OpiumPosPrevueY[(int)cdr->type] = 25;
		OpiumPosPrevueX[(int)cdr->type] += cdr->larg;
	}
	if(courant < 25) courant = 25;
#else
	courant = OpiumProchainY;
	if(courant < 25) courant = 25;
	OpiumProchainY += WndLigToPix(2);
	if(OpiumProchainY >  OpiumServerHeigth(0)) {
		OpiumProchainX = 10;
		OpiumProchainY = 25;
	}
#endif
	return(courant);
}
/* .......................................................................... */
static Cadre OpiumCdrPointe(Cadre cdr, WndUserRequest *u) {
	Cadre board;

	if(DEBUG_OPIUM(1)) WndPrint("(%s) On recherche le cadre pointe par l'evenement (%d, %d)\n",__func__,u->x,u->y);
	board = cdr; /* pour gcc */
	while(cdr->type == OPIUM_BOARD) {
		board = cdr;
		// printf("(%s) Element actif avant: %s %s ->cdr@%08X ->f@%08X\n",__func__,OpiumCdrType[(int)(board->actif)->type],(board->actif)->nom,(board->actif),(hexa)((board->actif)->f));
		if(u->type == WND_KEY) {
			if(!(cdr = board->actif)) cdr = (Cadre)board->adrs;
			if(DEBUG_OPIUM(1)) WndPrint("(%s) Boitier retenu: %08X/%-12s (etant actif)\n",__func__,(hexa)cdr,cdr->nom);
			if(!cdr) break;
		} else {
			cdr = BoardComposant(board,u->x + board->dx,u->y + board->dy);
			if(DEBUG_OPIUM(1)) WndPrint("(%s) Boitier retenu: %08X/%-12s (devient actif)\n",__func__,(hexa)cdr,cdr?cdr->nom:"aucun");
			if(!cdr) break;
			board->actif = cdr;
			// printf("(%s) Element actif apres: %s %s ->cdr@%08X ->f@%08X\n",__func__,OpiumCdrType[(int)(board->actif)->type],(board->actif)->nom,(board->actif),(hexa)((board->actif)->f));
		}
		u->x += (board->dx - cdr->x); u->y += (board->dy - cdr->y);
		if(DEBUG_OPIUM(1)) WndPrint("(%s) soit lieu evt: (%d, %d)\n",__func__,u->x,u->y);
	}
	if(cdr) return(cdr); else { board->actif = 0; return(board); }
}
/* .......................................................................... */
static Cadre OpiumCdrUtilise(WndIdent wid) {
/* Identification de l'cadre pointe */
	WndFrame f;
	int i;

	if(DEBUG_OPIUM(1)) WndPrint("(%s) On recherche le cadre qui est dans la fenetre Id=%08X\n",__func__,(hexa)wid);
	if(!wid) return(0);
	f = 0;
	for(i=0; i<OpiumNbCdr; i++) {
		if(!(f = OpiumCdr[i]->f)) continue;
		if(OpiumCdr[i]->planche) continue;
		if(f->w == wid) break;
	}
	if(DEBUG_OPIUM(1)) WndPrint("(%s) Cadre #%d/%d @%08X\n",__func__,i,OpiumNbCdr,(i == OpiumNbCdr)? 0: (hexa)(OpiumCdr[i]));
	if(i == OpiumNbCdr) return(0);
	else return(OpiumCdr[i]);
}
/* .......................................................................... */
char OpiumEstContenuDans(Cadre cdr, Cadre planche) {
	Cadre contenant,dessus; short iter;

	iter = 10;
	contenant = cdr;
	while((dessus = contenant->planche) && iter--) {
		if(dessus == planche) return(1);
		if(dessus == cdr) break;
		contenant = dessus;
	}
	return(0);
}
/* .......................................................................... */
void OpiumPlaceLocale(Cadre cdr) {
/* Defini l'emplacement occupe par un cadre, localement a sa planche/fenetre */
	int x,y; int hmax,vmax;

#ifdef POSITION_PAR_TYPE
	cdr->pos_std_x = OpiumPosPrevueX[(int)cdr->type];
	cdr->pos_std_y = OpiumPosPrevueY[(int)cdr->type];
#else
	cdr->pos_std_x = OpiumProchainX;
	cdr->pos_std_y = OpiumProchainY;
#endif
	if(cdr->defaultx != OPIUM_AUTOPOS) x = cdr->defaultx; else x = OpiumPosPrepareX(cdr);
	if(cdr->defaulty != OPIUM_AUTOPOS) y = cdr->defaulty; else y = OpiumPosPrepareY(cdr);
	cdr->x = x; cdr->y = y;
	if(DEBUG_OPIUM(1)) WndPrint("(%s) Cadre %08X/%s prevu en (%d, %d)\n",__func__,(hexa)cdr,cdr->nom,cdr->x,cdr->y);
	if(cdr->planche) {
		if((cdr->type == OPIUM_MENU) || (cdr->type == OPIUM_PANEL) || (cdr->type == OPIUM_INFO)) {
			x = ((cdr->x - 1) / WndColToPix(1)) + 1; // x = (cdr->x + (WndColToPix(1) / 2)) / WndColToPix(1); 
			cdr->x = x * WndColToPix(1);
			y = ((cdr->y - 1) / WndLigToPix(1)) + 1; // y = (cdr->y + (WndLigToPix(1) / 2)) / WndLigToPix(1);
			cdr->y = y * WndLigToPix(1);
		}
	} else {
		hmax = OpiumServerWidth(0) - WndColToPix(2) - OpiumMargeX;
		if((cdr->x + cdr->dh) > hmax) cdr->x = hmax - cdr->dh;
		if(cdr->x < OpiumMargeX) {
			cdr->x = OpiumMargeX;
			// if((cdr->x + cdr->dh) > hmax) cdr->dx = hmax - cdr->x;
		}
		vmax = OpiumServerHeigth(0) - WndLigToPix(2) - OpiumMargeY;
		if((cdr->y + cdr->dv) > vmax) cdr->y = vmax - cdr->dv;
		if(cdr->y < OpiumMargeY) {
			cdr->y = OpiumMargeY;
			// if((cdr->y + cdr->dv) > vmax) cdr->dv = vmax - cdr->y;
		}
	}
	if(DEBUG_OPIUM(1)) WndPrint("(%s) Cadre %08X/%s place en (%d, %d)\n",__func__,(hexa)cdr,cdr->nom,cdr->x,cdr->y);
}
/* .......................................................................... */
char OpiumCoordFenetre(Cadre cdr, WndFrame *f) {
/* Calcule les coordonnees utilisables, cad relativement a la fenetre utilisee */
	Cadre contenant,dessus; WndFrame fx;
	int x0,y0; int iter;

	*f = 0;
	if(DEBUG_OPIUM(1)) printf("(%s) Position   (%d, %d) dans F%08X pour C%08X='%s', repere (%d, %d)\n",__func__,cdr->x,cdr->y,(hexa)(cdr->f),(hexa)cdr,cdr->nom,cdr->x0,cdr->y0);
//#ifdef OPIUM_V1
//	if(!cdr->f) {
//		x0 = y0 = 0; contenant = cdr;
//		while((dessus = contenant->planche)) {
//			x0 += contenant->x; y0 += contenant->y;
//			contenant = dessus;
//		}
//		fx = contenant->f; if(!fx) { *f = 0; return(0); }
//		cdr->f = fx; cdr->x0 = x0; cdr->y0 = y0; 
//	} else { fx = cdr->f; x0 = cdr->x0; y0 = cdr->y0; }
//#else
//#ifdef OPIUM_V2
//	if(!cdr->f) {
//		x0 = y0 = 0; contenant = cdr;
//		while((dessus = contenant->planche)) {
//			printf("(%s) Ajout position propre de '%s' (%d, %d), retrait decalage planche (%d, %d)\n",__func__,
//				   contenant->nom,contenant->x,contenant->y,dessus->dx,dessus->dy);
//			x0 += (contenant->x - dessus->dx); y0 += (contenant->y - dessus->dy);
//			contenant = dessus;
//		}
//		fx = contenant->f; if(!fx) { *f = 0; return(0); }
//		cdr->f = fx; cdr->x0 = x0; cdr->y0 = y0; 
//	} else {
//		fx = cdr->f; x0 = cdr->x0; y0 = cdr->y0;
//		contenant = cdr;
//		while((dessus = contenant->planche)) {
//			printf("(%s) Retrait decalage planche (%d, %d)\n",__func__,
//				   dessus->dx,dessus->dy);
//			x0 -= dessus->dx; y0 -= dessus->dy;
//			contenant = dessus;
//		}
//	}
//#else
	if(!cdr->f) {
		x0 = y0 = 0; contenant = cdr;
		if(DEBUG_OPIUM(1)) printf("(%s) | Recherche une fenetre utilisable\n",__func__);
		iter = 10;
		while((dessus = contenant->planche) && iter--) {
			if(DEBUG_OPIUM(1)) {
				printf("(%s) | '%s' est contenu dans la planche C%08X='%s'\n",__func__,contenant->nom,(hexa)dessus,dessus->nom);
				printf("(%s) | Ajout position propre de C%08X='%s' (%d, %d)\n",__func__,(hexa)contenant,contenant->nom,contenant->x,contenant->y);
			}
			if(dessus == cdr) { /* *f = 0; */ return(0); }
			x0 += contenant->x; y0 += contenant->y;
			contenant = dessus;
		}
		if(iter < 0) { /* *f = 0; */ return(0); }
		fx = contenant->f;
		if(DEBUG_OPIUM(1)) printf("(%s) | Fenetre utilisable: F%08X\n",__func__,(hexa)fx);
		if(!fx) { /* *f = 0; */ return(0); }
		cdr->f = fx; cdr->x0 = x0; cdr->y0 = y0; 
	} else { fx = cdr->f; x0 = cdr->x0; y0 = cdr->y0; }
	contenant = cdr;
	if(DEBUG_OPIUM(1)) printf("(%s) | Calcule les coordonnees locales\n",__func__);
	iter = 10;
	while((dessus = contenant->planche) && iter--) {
		if(DEBUG_OPIUM(1)) {
			printf("(%s) | '%s' est contenu dans la planche C%08X='%s'\n",__func__,contenant->nom,(hexa)dessus,dessus->nom);
			printf("(%s) | Retrait decalage planche C%08X='%s' (%d, %d)\n",__func__,(hexa)dessus,dessus->nom,dessus->dx,dessus->dy);
		}
		if(dessus == cdr) { /* *f = 0; */ return(0); }
		x0 -= dessus->dx; y0 -= dessus->dy;
		contenant = dessus;
	}
	if(iter < 0) { /* *f = 0; */ return(0); }
//#endif
//#endif
	WndRepere(fx,x0,y0,cdr->dh,cdr->dv);
	if(DEBUG_OPIUM(1)) printf("(%s) | Repere   (%d, %d) dans F%08X pour '%s'\n",__func__,x0,y0,(hexa)fx,cdr->nom);

	*f = fx; return(1);
}
/* .......................................................................... */
int OpiumUse(Cadre cdr, int modexec) {
/* Definition et affichage de la fenetre supportant le cadre */
	TypeOpiumFctnArg fctn; int rc;
	int larg,haut; int ic;
	Cadre boitier; WndFrame f; int qual;

	f = cdr->f;
	//- OpiumDebugLevel[OPIUM_DEBUG_OPIUM] = 1;
	if(DEBUG_OPIUM(9)) WndPrint("(%s) %s %s '%s'\n",__func__,
		OpiumExecName[modexec],OpiumCdrType[(int)cdr->type],cdr->titre);
	if(DEBUG_OPIUM(2)) WndPrint("(%s) C=%08X, F=%08X\n",__func__,(hexa)cdr,(hexa)f);
	if(f) {
		if(DEBUG_OPIUM(1)) WndPrint("(%s) F=%08X existe (non-nul), mise au sommet (Old)\n",__func__,(hexa)f);
		if((modexec == OPIUM_DISPLAY) && (cdr->modexec != OPIUM_DISPLAY)) --OpiumNbExec;
		else if((modexec != OPIUM_DISPLAY) && (cdr->modexec == OPIUM_DISPLAY)) OpiumNbExec++;
		if(/* f->passive && ( */ modexec == OPIUM_DISPLAY /*)*/) {
			if(DEBUG_OPIUM(1)) {
				WndPrint("(%s) Reafficherait en haut la fenetre passive F=%08X\n",__func__,(hexa)f);
				WndPrint("           qui contient le %s '%s' en mode %s\n",OpiumCdrType[(int)cdr->type],cdr->nom,OpiumExecName[modexec]);
			}
		} else {
			if(modexec >= 0) cdr->modexec = (char)modexec;
			WndSendToTop(f);
		}
		return(0);
	}

	if(DEBUG_OPIUM(1)) WndPrint("(%s) Creation de la fenetre en mode %s\n",__func__,OpiumExecName[modexec]);
	while(cdr->planche) cdr = cdr->planche;
	cdr->modexec = (char)modexec;
	cdr->var_modifiees = 0;
	if(cdr->type == OPIUM_BOARD) {
		boitier = (Cadre)cdr->adrs;
		while(boitier) {
			if((boitier->type != OPIUM_GRAPH) && (boitier->type != OPIUM_TERM)) boitier->modexec = (char)modexec;
			boitier = boitier->suivant;
		}
	}
	if((fctn = cdr->enter)) { if((rc = (*fctn)(cdr,cdr->enter_arg))) return(rc); }
	OpiumSizeChanged(cdr);
	larg = WndCurSvr->larg - OpiumMargeX;
	if(cdr->dh >= larg) {
		cdr->dh = larg; cdr->dx = 0;
		cdr->defaultx = cdr->x = OpiumMargeX;
	}
	haut = WndCurSvr->haut - OpiumMargeY;
	if(cdr->dv >= haut) {
		cdr->dv = haut; cdr->dy = 0;
		cdr->defaulty = cdr->y = OpiumMargeY;
	}
	OpiumPlaceLocale(cdr);
	if(cdr->type == OPIUM_GRAPH) {
		qual = GraphQualite;
		if(DEBUG_OPIUM(1)) printf("(%s) Evaluation du mode\n",__func__);
		((Graph)(cdr->adrs))->mode |= (qual << 5);
		if(DEBUG_OPIUM(1)) printf("(%s) Mode fixe a 0x%04X\n",__func__,(hexa)((Graph)(cdr->adrs))->mode);
	} else qual = WndQual;
	if(cdr->type == OPIUM_BOARD)
		cdr->f = f = WndCreate(WND_FEN_BRD,qual,cdr->x,cdr->y,cdr->dh,cdr->dv);
	else if(modexec == OPIUM_SUBWND) {
		cdr->f = f = WndAttach(qual,cdr->x,cdr->y,cdr->dh,cdr->dv);
		OpiumPrioritaire = cdr;
	} else {
		if(DEBUG_OPIUM(1)) printf("(%s) Affichage de la fenetre\n",__func__);
		cdr->f = f = WndDisplay(qual,cdr->x,cdr->y,cdr->dh,cdr->dv);
		if(DEBUG_OPIUM(1)) printf("(%s) Fenetre affichee\n",__func__);
/*		if(cdr->type == OPIUM_BOARD) WndBasicRGB(f,0xC000,0xC000,0xC000,0,0,0); */
	}

	if(DEBUG_OPIUM(1)) printf("(%s) Affichage du %s %s C:%08X dans F:%08X\n",__func__,OpiumCdrType[cdr->type],cdr->nom,(hexa)cdr,(hexa)(cdr->f));
	if(!f) return(0);
	ic = WND_GC_BASE;
	switch(cdr->type) {
	  case OPIUM_MENU:
		OpiumMenuColorNb(cdr,0,&ic);
		WndFenColorNb(f,ic);
		OpiumMenuColorSet(cdr,f);
		break;
	  case OPIUM_PANEL:
		OpiumPanelColorNb(cdr,0,&ic);
		WndFenColorNb(f,ic);
		OpiumPanelColorSet(cdr,f);
		break;
	  case OPIUM_GRAPH:
		OpiumGraphColorNb(cdr,0,&ic);
		WndFenColorNb(f,ic);
		OpiumGraphColorSet(cdr,f);
		break;
	  case OPIUM_INSTRUM:
		OpiumInstrumColorNb(cdr,0,&ic);
		WndFenColorNb(f,ic);
		OpiumInstrumColorSet(cdr,f);
		break;
	  case OPIUM_FIGURE:
		OpiumFigureColorNb(cdr,0,&ic);
		WndFenColorNb(f,ic);
		OpiumFigureColorSet(cdr,f);
		break;
	  case OPIUM_ONGLET:
		OpiumOngletColorNb(cdr,0,&ic);
		WndFenColorNb(f,ic);
		OpiumOngletColorSet(cdr,f);
		break;
	  case OPIUM_BOARD:
		OpiumBoardColorNb(cdr,0,&ic);
		WndFenColorNb(f,ic);
		OpiumBoardColorSet(cdr,f);
		// WndFenColorDump(f,WND_Q_ECRAN);
		break;
	}

	WndMove(f,f->x,f->y);
	if(DEBUG_OPIUM(1)) WndPrint("(%s) Fenetre placee en (%d, %d)\n",__func__,f->x,f->y);
	if(!(cdr->nom[0])) {
		if(OpiumNextTitle) OpiumTitle(cdr,OpiumNextTitle);
		else sprintf(cdr->nom,"%s%d",OpiumCdrType[cdr->type],cdr->num); /* on pourrait etre plus astucieux */
	}
	// if(DEBUG_OPIUM(1)) WndPrint("(%s) Nom du cadre: '%s'\n",__func__,cdr->nom);
	if(cdr->nom[0]) {
		WndTitle(f,cdr->nom);
	#ifdef MENU_BARRE
		WndMenuItem(cdr->nom,0,1);
	#endif
	}
	if(DEBUG_OPIUM(1)) WndPrint("(%s) Utilisation de la fenetre @%08X\n",__func__,(hexa)f);
	//? if(modexec > OPIUM_DISPLAY) { OpiumNbExec++; WndPutAtTop(f); } //-- else WndWriteOnlySet(f);
	if(modexec > OPIUM_DISPLAY) OpiumNbExec++; 
	WndPutAtTop(f);
	if(DEBUG_OPIUM(9)) WndPrint("(%s) Ouverture %s %s '%s', mode %d (%d executable%s)\n",__func__,
		OpiumCdrType[(int)cdr->type],(cdr->modexec == OPIUM_DISPLAY)?"affiche":"executable",
		cdr->titre,modexec,Accord1s(OpiumNbExec));
#ifdef CHANGE_CURSEUR
	if(cdr->type == OPIUM_GRAPH) WndAssignCursor(f,OpiumCurseurViseur);
	else WndAssignCursor(f,OpiumCurseurMain);
#endif
	/* WndRaise(f); a ce niveau entraine erreur dans XNextEvent */
	if(DEBUG_OPIUM(9)) {
		WndPrint("(%s) Frame @%08X affichee (fenetre #%08X), TopFrame 0x%08X\n",__func__,
			(hexa)cdr->f,(hexa)(cdr->f)->w,(hexa)WndTopFrame);
	}
#ifdef WIN32
	OpiumRefresh(cdr);
#endif
	//- OpiumDebugLevel[OPIUM_DEBUG_OPIUM] = 0;
	if(WndCodeHtml) {
		ArgDesc *elt; int i;
		OpiumHtmlPageDebut();
		OpiumHttpVars = 1; for(i=0; i<OpiumNbCdr; i++) if(OpiumCdr[i]->f) OpiumHttpVars++;
		printf("(%s) %d Variable%s HTML\n",__func__,Accord1s(OpiumHttpVars));
		OpiumHttpDesc = ArgCreate(OpiumHttpVars);
		elt = OpiumHttpDesc;
		ArgAdd(elt++,"mBarre",DESC_STR(OPIUM_ITEM_NOM) OpiumHttpAction,0);
		printf("(%s) | Variable '%s' pour le %s '%s'\n",__func__,"mBarre","menu","Barre");
		HttpEcrit("<table width=100%%><tr>\n");
		for(i=0; i<OpiumNbCdr; i++) if(OpiumCdr[i]->f) {
			char *var; Menu menu; MenuItem *item; Panel panel; PanelItem *pvar;
			int k,l,n;
			// OpiumRefresh(OpiumCdr[i]);
			switch(OpiumCdr[i]->type) {
				case OPIUM_MENU:
					menu = (Menu)OpiumCdr[i]->adrs; if(!menu) break;
					item = menu->items; if(!item) break;
					var = &(OpiumHtmlVarName[i][0]);
					sprintf(var,"c%d",i);
					printf("(%s) | Variable '%s' pour le %s '%s'\n",__func__,var,"menu",OpiumCdr[i]->titre);
					HttpEcrit("    <td><fieldset style=\"background: #FFDFBF; border-radius:10px\"><legend><b>%s</b></legend><table>\n",OpiumCdr[i]->titre);
					l = 0;
					while(item->texte) {
						if(item->sel == MNU_ITEM_CACHE) { item->aff = -1; goto suivant; }
						HttpEcrit("        <tr><td><input style=\"background: #9FFF9F\" type=\"submit\" name=\"%s\" value=\"%s\"></td></tr>\n",var,item->texte);
						//+ printf("(%s) | . bouton '%s'\n",__func__,item->texte);
						item->aff = l++;
					suivant:
						item++;
					}
					HttpEcrit("    </table></fieldset></td>\n");
					ArgAdd(elt++,var,DESC_STR(OPIUM_ITEM_NOM) OpiumHttpAction,0);
					break;
				case OPIUM_PANEL:
					panel = (Panel)OpiumCdr[i]->adrs; if(!panel) break;
					pvar = panel->items; if(!pvar) break;
					n = PanelItemNb(panel);
					var = &(OpiumHtmlVarName[i][0]);
					sprintf(var,"c%d",i);
					printf("(%s) | Variable '%s' pour le %s[%d] '%s'\n",__func__,var,"panel",n,OpiumCdr[i]->titre);
					HttpEcrit("    <td><fieldset style=\"background: #FFDFBF; border-radius:10px\"><legend><b>%s</b></legend><table>\n",OpiumCdr[i]->titre);
					for(k=0; k<n; k++) {
						pvar = panel->items + k;
						// if(pvar->type < 0) continue;
						HttpEcrit("        <tr><td bgcolor=\"#9F9FFF\" align=right><label>%s</label></td>",pvar->traduit);
						PanelCode(pvar,PNL_WRITE,0,0);
						// HttpEcrit("<td bgcolor=\"FFFFFF\" align=left>%s</td></tr>\n",pvar->ptrval);
						HttpEcrit("<td bgcolor=\"FFFFFF\" align=left><input type=\"text\" name=\"%s\" value=\"%s\"></td></tr>\n",pvar->traduit,pvar->ptrval);
						//+ printf("(%s) | . variable '%s' = '%s'\n",__func__,pvar->traduit,pvar->ptrval);
					}
					HttpEcrit("    </table></fieldset></td>\n");
					ArgAdd(elt++,var,DESC_STR(OPIUM_ITEM_NOM) OpiumHttpAction,0);
					break;
			}
		}
		HttpEcrit("</tr></table><br>\n");
		OpiumHtmlPageFin();
		printf("(%s) Desc ",__func__); ArgPrint("*",OpiumHttpDesc);
//		while(!OpiumHttpRequetteArrivee) sleep(1);
	}
	return(0);
}
/* ========================================================================== */
int OpiumDisplay(Cadre cdr) {
/* affichage simple d'un Cadre */
	int erreur;

	if(!cdr) return(0);
	if(DEBUG_OPIUM(1)) WndPrint("OpiumDisplay(%s '%s' @%08X, %s affiche)\n",
		OpiumCdrType[(int)cdr->type],cdr->nom,(hexa)cdr,
		(cdr->displayed)? "deja": "pas encore");
	if(cdr->type == OPIUM_GRAPH) GraphParmsSave(cdr->adrs);
	if(cdr->displayed) return(OpiumRefreshIf(OPIUM_TOUJOURS,cdr));
	else {
		erreur = OpiumUse(cdr,OPIUM_DISPLAY);
		/*	if(!erreur) OpiumRefreshIf(OPIUM_NECESSAIRE,cdr); fait via evt Expose? */
		return(erreur);
	}
}
/* ========================================================================== */
char OpiumRefreshBegin(Cadre cdr) {
	WndFrame f;
	if(cdr && OpiumCoordFenetre(cdr,&f)) return(WndRefreshBegin(f));
	else return(0);
}
/* ========================================================================== */
void OpiumRefreshEnd(Cadre cdr) {
	WndFrame f;
	if(cdr && OpiumCoordFenetre(cdr,&f)) {
		WndRefreshEnd(f);
//		OpiumRefreshIf(OPIUM_TOUJOURS,cdr);
	}
}
/* ========================================================================== */
int OpiumRefreshAgain(Cadre cdr) {
	OpiumLastRefreshed = 0;
	return(OpiumRefreshIf(OPIUM_TOUJOURS,cdr));
}
/* ========================================================================== */
int OpiumRefresh(Cadre cdr) { return(OpiumRefreshIf(OPIUM_TOUJOURS,cdr)); }

#ifdef WIN32
/* ========================================================================== */
int OpiumRefresh(WndIdent wnd) { 
	return(OpiumRefreshIf(OPIUM_TOUJOURS,OpiumCdrUtilise(wnd))); 
}
#endif
/* ========================================================================== */
int OpiumRefreshIf(OPIUM_REFRESH_CONDITION condition, Cadre cdr) {
/* rafraichissement d'un Cadre */
	WndFrame f; Cadre board;
	int x,y,h,v; int haut,larg,dx,dy; int64 dispo;
	char redimensionne,sans_fond,type,doit_terminer;

	if(DEBUG_OPIUM(1)) WndPrint("(%s) Commence avec %s '%s' %d x %d, affichage %d x %d\n",__func__,OpiumCdrType[(int)cdr->type],cdr->nom,cdr->larg,cdr->haut,cdr->dh,cdr->dv);
	if(DEBUG_OPIUM(1)) WndPrint("(%s) Cadre %08llX a rafraichir\n",__func__,(UInt64)cdr);
	if(!cdr) return(0);
	if(condition == OPIUM_NECESSAIRE) {
// 		if(cdr == OpiumLastRefreshed) WndPrint("(OpiumRefreshIf) Deja rafraichi!\n");
		if(cdr == OpiumLastRefreshed) return(0);
	}
	if(!OpiumCoordFenetre(cdr,&f)) {
		WndPrint("(%s) Pas trouve la fenetre associee a %s '%s'\n",__func__,OpiumCdrType[(int)cdr->type],cdr->nom);
		ImprimePileAppels;
		return(0);
	}
#ifdef A_TESTER
	if(condition == OPIUM_NECESSAIRE) { if(f != WndTopFrame) return(0); }
#endif
	if(DEBUG_OPIUM(1)) {
		WndPrint("(%s) %s '%s' @%08llX sur la fenetre F=@%08llX [%08llX]\n",__func__,
			OpiumCdrType[(int)cdr->type],cdr->nom,(UInt64)cdr,(UInt64)f,(UInt64)cdr->f);
	}

	redimensionne = 0;
	larg = (f->s)->larg - OpiumMargeX;
	if(cdr->dh >= larg) {
		cdr->dh = larg;
		cdr->defaultx = cdr->x = f->x = OpiumMargeX;
		redimensionne = 1;
	}
	haut = (f->s)->haut - OpiumMargeY;
	if(cdr->dv >= haut) {
		cdr->dv = haut;
		cdr->defaulty = cdr->y = f->y = OpiumMargeY;
		redimensionne = 1;
	}
	if(redimensionne) {
		switch(cdr->type) {
			case OPIUM_MENU:    OpiumSizeMenu(cdr,1);   break;
			case OPIUM_PANEL:   OpiumSizePanel(cdr,1);  break;
			case OPIUM_TERM:    OpiumSizeTerm(cdr,1);   break;
			case OPIUM_GRAPH:   OpiumSizeGraph(cdr,1);  break;
			case OPIUM_INFO:    OpiumSizeInfo(cdr,1);   break;
			case OPIUM_PROMPT:  OpiumSizePrompt(cdr,1); break;
			case OPIUM_BOARD:   OpiumSizeBoard(cdr,1);  break;
			case OPIUM_INSTRUM: OpiumSizeInstrum(cdr,1);  break;
			case OPIUM_FIGURE:  OpiumSizeFigure(cdr,1);  break;
			case OPIUM_ONGLET:  OpiumSizeOnglet(cdr,1);  break;
		}
		f->h = cdr->dh;
		f->v = cdr->dv;
		WndResize(f);
	}

	doit_terminer = WndRefreshBegin(f);
	sans_fond = (cdr->type == OPIUM_FIGURE);
	if(sans_fond) {
		type = ((Figure)(cdr->adrs))->type;
		sans_fond = ((type == FIG_DROITE) || (type == FIG_LIEN) || (type == FIG_ICONE) || (type == FIG_CADRE));
	}
	if((board = cdr->planche)) {
		if(cdr->type == OPIUM_PANEL) {
			Panel p;
			p = (Panel)cdr->adrs;
			if(p->support) {
				x = 2; y = (p->mode & PNL_DIRECT)? 2: WndLigToPix(1) + 1;
			} else x = y = 0;
			WndPaint(f,-x,-x,cdr->dh+x,cdr->dv+y,WndColorBoard[f->qualite],1); /* -x,-x et non pas -x,-y */
		} else if(!sans_fond /* && (cdr->type != OPIUM_TERM) */) WndPaint(f,0,0,cdr->dh,cdr->dv,WndColorBoard[f->qualite],2);
	} else {
		if(!sans_fond) WndBlank(f);
		// WndEntoure(f,WND_CREUX,0,0,cdr->dh,cdr->dv);
	}
	cdr->ligmin = (cdr->dy)? WndPixToLig(cdr->dy - 1) + 1: 0;
	cdr->colmin = (cdr->dx)? WndPixToCol(cdr->dx - 1) + 1: 0;
	cdr->lignb  = WndPixToLig(cdr->dv);
	cdr->colnb  = WndPixToCol(cdr->dh);
	// if(DEBUG_OPIUM(2)) WndPrint("(%s specific) Cadre %s %d x %d, affichage %d x %d\n",__func__,cdr->nom,cdr->larg,cdr->haut,cdr->dh,cdr->dv);
	switch(cdr->type) {
	  case OPIUM_MENU:    OpiumRefreshMenu(cdr);   break;
	  case OPIUM_PANEL:   OpiumRefreshPanel(cdr);  break;
	  case OPIUM_TERM:    OpiumRefreshTerm(cdr);   break;
	  case OPIUM_GRAPH:   OpiumRefreshGraph(cdr); break;
	  case OPIUM_INFO:    OpiumRefreshInfo(cdr);   break;
	  case OPIUM_PROMPT:  OpiumRefreshPrompt(cdr); break;
	  case OPIUM_BOARD:   OpiumRefreshBoard(cdr);  break;
	  case OPIUM_INSTRUM: OpiumRefreshInstrum(cdr);  break;
	  case OPIUM_FIGURE:  OpiumRefreshFigure(cdr);  break;
	  case OPIUM_ONGLET:  OpiumRefreshOnglet(cdr);  break;
	}
	WndExtraDisplay(f);
	// if(DEBUG_OPIUM(2)) WndPrint("(%s continue) Cadre %s %d x %d, affichage %d x %d\n",__func__,cdr->nom,cdr->larg,cdr->haut,cdr->dh,cdr->dv);

	if(!cdr->planche) WndControls(f);
	dx = (WndColToPix(1) * 3 / 2) - 2;
	dy = WndLigToPix(1);
	if(cdr->dh < cdr->larg) {
		y = cdr->dv;
		// WndLine(f,WND_GC(f,WND_GC_ASCR),0,y,cdr->dh,0);
		y++;
		WndRelief(f,2,WND_BORD_HAUT,0,y,cdr->dh+WND_ASCINT_WIDZ);
		WndFillBgnd(f,WND_GC(f,WND_GC_ASCR),0,y,cdr->dh,WND_ASCINT_WIDZ-1);

		dispo = (int64)(cdr->dh) - (2 * WND_ASCINT_WIDZ);
		x = (int)(((int64)(cdr->dx) * dispo) / (int64)(cdr->larg)) + WND_ASCINT_WIDZ;
		h = (int)((int64)(cdr->dh) * dispo) / (int64)(cdr->larg); if(h < 2) h = 2;
		if((x + h) > cdr->dh) x = cdr->dh - h;
		// WndFillFgnd(f,WND_GC(f,WND_GC_ASCR),x,y,h,cdr->dv + WND_ASCINT_WIDZ - y - 1);
		WndFillFgnd(f,WND_GC(f,WND_GC_ASCR),x,y,h,WND_ASCINT_WIDZ-2);
		WndRelief(f,2,WND_BORD_GCHE,x,y,WND_ASCINT_WIDZ-2);
		WndRelief(f,2,WND_BORD_DRTE,x+h,y,WND_ASCINT_WIDZ-2);
		
		// WndFillFgnd(f,f->text[WND_GC_DARK],0,y,WND_ASCINT_WIDZ,cdr->dv + WND_ASCINT_WIDZ - y - 1);
		WndString(f,WND_CLEAR,4,y+WndLigToPix(1)+1,"<");
		WndEntoure(f,WND_PLAQUETTE,4,y+4,dx-2,WND_ASCINT_WIDZ-6);
		
		// WndFillFgnd(f,f->text[WND_GC_DARK],cdr->dh-WND_ASCINT_WIDZ,y,WND_ASCINT_WIDZ,cdr->dv + WND_ASCINT_WIDZ - y - 1);
		WndString(f,WND_CLEAR,cdr->dh-dx+1,y+WndLigToPix(1)+1,">");
		WndEntoure(f,WND_PLAQUETTE,cdr->dh-dx,y+4,dx,WND_ASCINT_WIDZ-6);
	}
	if(cdr->dv < cdr->haut) {
		x = cdr->dh;
		// WndLine(f,f->ascenseur,x,0,0,cdr->dv);
		x++;
		WndRelief(f,2,WND_BORD_GCHE,x,0,cdr->dv);
		WndFillBgnd(f,WND_GC(f,WND_GC_ASCR),x,0,WND_ASCINT_WIDZ-1,cdr->dv);

		dispo = (int64)cdr->dv - (2 * WND_ASCINT_WIDZ);
		y = (int)(((int64)(cdr->dy) * dispo) / (int64)(cdr->haut)) + WND_ASCINT_WIDZ;
		v = (int)((int64)(cdr->dv) * dispo) / (int64)(cdr->haut); if(v < 2) v = 2;
		if((y + v) > cdr->dv) y = cdr->dv - v;
		
//		y+v = ((cdr->dy + cdr->dv) * cdr->dv) / cdr->haut;
//		ymax =  cdr->dv - ((cdr->dv * cdr->dv) / cdr->haut);
//		dymax = ymax * cdr->haut / cdr->dv;
//		dymax = (cdr->dv  * cdr->haut / cdr->dv) - (((cdr->dv * cdr->dv) / cdr->haut) * cdr->haut / cdr->dv);
//		dymax = cdr->haut - cdr->dv;
		
		// WndFillFgnd(f,f->ascenseur,x,y,cdr->dh + WND_ASCINT_WIDZ - x- 1,v);
		WndFillFgnd(f,WND_GC(f,WND_GC_ASCR),x,y,WND_ASCINT_WIDZ-2,v);
		WndRelief(f,2,WND_BORD_HAUT,x,y,WND_ASCINT_WIDZ-2);
		WndRelief(f,2,WND_BORD_BAS,x,y+v,WND_ASCINT_WIDZ-2);
		
		// WndFillFgnd(f,f->text[WND_GC_DARK],x,0,cdr->dh + WND_ASCINT_WIDZ - x- 1,WND_ASCINT_WIDZ);
		WndString(f,WND_CLEAR,x+3,WndLigToPix(1),"^");
		WndEntoure(f,WND_PLAQUETTE,x+2,2,dx,dy-5);

		// WndFillFgnd(f,f->text[WND_GC_DARK],x,cdr->dv-WND_ASCINT_WIDZ,cdr->dh + WND_ASCINT_WIDZ - x- 1,WND_ASCINT_WIDZ);
		WndString(f,WND_CLEAR,x+3,cdr->dv,"v");
		WndEntoure(f,WND_PLAQUETTE,x+2,cdr->dv-dy+3,dx,dy-3);
	}
	if(doit_terminer) WndRefreshEnd(f);

	cdr->displayed = 1;
	OpiumLastRefreshed = cdr;
	if(DEBUG_OPIUM(1)) WndPrint("(%s) Termine avec %s '%s' %d x %d, affichage %d x %d\n",__func__,OpiumCdrType[(int)cdr->type],cdr->nom,cdr->larg,cdr->haut,cdr->dh,cdr->dv);
	if(DEBUG_OPIUM(1)) WndPrint("(%s) Fenetre %08X rafraichie (%s %s @%08X)\n",__func__,(hexa)f,OpiumCdrType[(int)cdr->type],cdr->nom,(hexa)cdr);
	return(1);
}
/* ========================================================================== */
void OpiumWndResize(Cadre cdr) {
	WndFrame f;
	
	if(!cdr) return;
	switch(cdr->type) {
		case OPIUM_MENU:    OpiumSizeMenu(cdr,1);   break;
		case OPIUM_PANEL:   OpiumSizePanel(cdr,1);  break;
		case OPIUM_TERM:    OpiumSizeTerm(cdr,1);   break;
		case OPIUM_GRAPH:   OpiumSizeGraph(cdr,1);  break;
		case OPIUM_INFO:    OpiumSizeInfo(cdr,1);   break;
		case OPIUM_PROMPT:  OpiumSizePrompt(cdr,1); break;
		case OPIUM_BOARD:   OpiumSizeBoard(cdr,1);  break;
		case OPIUM_INSTRUM: OpiumSizeInstrum(cdr,1);  break;
		case OPIUM_FIGURE:  OpiumSizeFigure(cdr,1);  break;
		case OPIUM_ONGLET:  OpiumSizeOnglet(cdr,1);  break;
	}
	if(!OpiumCoordFenetre(cdr,&f)) {
		WndPrint("(%s) Pas trouve la fenetre associee a %s '%s'\n",__func__,OpiumCdrType[(int)cdr->type],cdr->nom);
		return;
	}
	f->h = cdr->dh;
	f->v = cdr->dv;
	WndResize(f);
}
// #ifdef OPIUM_HTTP
#include <html.h>
/* ========================================================================== */
void OpiumHtmlBandeauDefini(TypeFctn fctn) {
	OpiumHtmlPageBandeau = fctn;
}
/* ========================================================================== */
static void OpiumHtmlPageDebut() {
	MenuItem *item;
	//- Menu sousmenu; MenuItem *sousitem;

	if(OpiumHttpDesc) free(OpiumHttpDesc); OpiumHttpVars = 0;
	HttpPageDebut(OpiumHttpTitre,0xFFFFDF);
	if(HttpClientType == HTTP_CLIENT_IPHONE) {
		HttpEcrit("<style type=\"text/css\">\n");
		HttpEcrit("legend { font-size: 60%; }\n");
		HttpEcrit("p { font-size: 100%; }\n");
		HttpEcrit("br { font-size: 100%; }\n");
		HttpEcrit("table { font-size: 100%; }\n");
		HttpEcrit("td { font-size: 70%; }\n");
		HttpEcrit("input { font-size: 60%; }\n");
		HttpEcrit("</style>\n");
	}
	if(OpiumHtmlPageBandeau) (*OpiumHtmlPageBandeau)();
	HttpEcrit("<br><i>Serveur #%d</i>\n",getpid());
	HttpEcrit("<form action=\"http://%s:%d\">\n",HttpServeurNom,OpiumHttpPort);

	if(!MenuEnBarre) return;
	item = MenuEnBarre->items; if(!item) return;
	HttpEcrit("<table width=100%% cellpadding=1><tr bgcolor=\"#3F3FFF\">\n");
	while(item->texte) {
		if(item->sel == MNU_ITEM_CACHE) goto item_svt;
//		HttpEcrit("<td align=center><input style=\"background: #3F3FFF\" type=\"submit\" name=\"mBarre\" value=\"%s\"></td>",item->texte);
		HttpEcrit("    <td align=center><input type=\"submit\" name=\"mBarre\" value=\"%s\"></td>\n",item->texte);
	item_svt:
		item++;
	}
	HttpEcrit("</tr></table><br>\n");
}
/* ========================================================================== */
static void OpiumHtmlPageFin() {
	HttpEcrit("</form>\n");
	HttpPageFin();
	// attendre une requete http
	OpiumHttpRequetteArrivee = 0;
}
/* ========================================================================== */
int OpiumTraiteRequete(char *url, char *demandeur, char *parms) {
	/* appele par HttpTraiteRequete() qui est appele dans le main(), *si* l'appli doit etre un serveur */
	WndUserRequest u;
	Cadre cdr; Menu menu; MenuItem *item;
	int i,k,n; char *var; int cdr_ouverts;

	printf("(%s) Requete recue: [%s] analysee @%08X\n",__func__,parms,(hexa)OpiumHttpDesc);
	if((strlen(parms) == 0) || !OpiumHttpDesc) {
		ArgDesc *elt;
		OpiumHtmlPageDebut();
		OpiumHtmlPageFin();
		OpiumHttpDesc = ArgCreate(1);
		elt = OpiumHttpDesc;
		ArgAdd(elt++,"mBarre",DESC_STR(OPIUM_ITEM_NOM) OpiumHttpAction,0);
		return(1);
	}
	ArgSplit(parms,'&',OpiumHttpDesc);
	printf("(%s) Analyse ",__func__); ArgPrint("*",OpiumHttpDesc);
	/* Transformer les variables recues en evenement opium, puis appeler OpiumManage(),
	 lequel appellera OpiumHtmlPagexxx() via OpiumRun() */
	k = ArgGetVarLue(OpiumHttpDesc);
	//+ printf("(%s) Var lue: #%d\n",__func__,k);
	var = ArgItemGetName(OpiumHttpDesc,k);
	//+ printf("(%s) = \"%s\"\n",__func__,var);
	if(k > 0) {
		sscanf(var+1,"%d",&i); cdr = OpiumCdr[i];
		u.type = WND_RELEASE; u.w = (cdr->f)->w;
		printf("(%s) Cadre utilise: #%d = %s \"%s\"\n",__func__,i,OpiumCdrType[cdr->type],cdr->nom);
	} else {
		cdr = MenuEnBarre->cdr;
		u.type = WND_BARRE; u.w = 0;
		printf("(%s) Cadre utilise: MenuBarre\n",__func__);
	}
	printf("(%s) Action demandee: [%s:\"%s\"]\n",__func__,var,OpiumHttpAction);
	switch(cdr->type) {
	  case OPIUM_MENU:
		menu = (Menu)cdr->adrs; if(!menu) break;
		item = menu->items; if(!item) break;
		n = 0;
		while(item->texte) {
			if(item->sel == MNU_ITEM_CACHE) { item->aff = -1; goto suivant; }
			if(!strcmp(OpiumHttpAction,item->texte)) break;
		suivant:
			item++; n++;
		}
		if(item->texte) { u.y = (k > 0)? WndLigToPix(n): n; u.x = 0; }
		u.h = u.v = 0; u.code = (k > 0)? WND_MSELEFT: 999; u.texte[0] = '\0';
		break;
	}
	u.suivant = 0;
	cdr_ouverts = 0;
	OpiumManage(0,&u,&cdr_ouverts);

	OpiumHttpRequetteArrivee = 1;
	return(1);
}
// #endif /* OPIUM_HTTP */
/* ========================================================================== */
void OpiumActive(Cadre cdr) { if(cdr && cdr->displayed) WndRaise(cdr->f); }
/* ========================================================================== */
int OpiumFork(Cadre cdr) { return(OpiumUse(cdr,OPIUM_FORK)); }
/* ========================================================================== */
int OpiumExec(Cadre cdr_initial) { return(OpiumRun(cdr_initial,OPIUM_EXEC)); }
/* ========================================================================== */
int OpiumSub(Cadre cdr_initial) { return(OpiumRun(cdr_initial,OPIUM_SUBWND)); }
/* ========================================================================== */
int OpiumRun(Cadre cdr_initial, int mode) {
	int cdr_ouverts,code_a_rendre,key;
	WndUserRequest u;
	int nbloop,precedent;
	
	WndQueryExit = 0; OpiumLastRefreshed = 0;
/* affichage d'un cadre avec entree utilisateur */
	if(DEBUG_OPIUM(1)) WndPrint("OpiumRun(%s %08X): deja %d executable%s\n",
		OpiumExecName[mode],(hexa)cdr_initial,Accord1s(OpiumNbExec));
	if(cdr_initial) {
		if(cdr_initial == OpiumPanelQuick->cdr) (OpiumPanelQuick->cdr)->titre[0] = '\0';
		if((code_a_rendre = OpiumUse(cdr_initial,mode))) return(code_a_rendre);
		cdr_ouverts = 1; /* nombre de cadres "localement" en exec */
	}
	  else return(0); /* avait ete supprimee mais alors mais cdr_ouverts pas defini... */
	if(OpiumPrioritaire) WndBell(cdr_initial->f,100);

/* gestion des evenements */
	u.type = -1; /* rapport au cas WND_PRESS */
	nbloop = 0; precedent = u.type;
	code_a_rendre = 0;
	while(OpiumNbExec && cdr_ouverts) {
		if(u.type == precedent) nbloop++; else nbloop = 1;
		precedent = u.type;
		if(DEBUG_OPIUM(1)) {
			WndPrint("(%s) Attente evt avec Wnd%s #%d\n",__func__,(u.type == -1)? "none": WndTypeName[(int)u.type],nbloop);
			WndPrint("          sur %d fenetre%s et %d cadre%s dans cette boucle ---------------- #%03ld -------------\n",
				Accord1s(OpiumNbExec),Accord1s(cdr_ouverts),OpiumEvtNum);
		}
		if(OpiumEntryPre) {
			key = (*OpiumEntryPre)();
			if(DEBUG_OPIUM(3)) WndPrint("EntryBefore: cle rendue=%08X\n",key);
		} else key = 0;
		if(key) {
			if(WndTopFrame == WND_AT_END) continue;
			u.type = WND_KEY;
			u.code = 0; /* ca serait bien de coder les cles */
			u.w = WndTopFrame->w;
			u.x = u.y = u.h = u.v = 0;
			u.texte[0] = (char)key; u.texte[1] = '\0';
		} else WndEventNew(&u,WND_WAIT);
		memcpy(&OpiumLastEvent,&u,sizeof(WndUserRequest));
		if(DEBUG_OPIUM(1)) WndPrint("(%s) Recupere evt '%s %s' en %d x %d\n",__func__,
			(u.type==-1)?"neant":OpiumEventType[u.type],(u.type==-1)?"de chez neant":((u.type == WND_KEY)?u.texte:OpiumMseButton[u.code]),u.x,u.y);
		if(WndQueryExit) {
			if(cdr_initial) switch(cdr_initial->type) {
			  case OPIUM_MENU: case OPIUM_TERM: case OPIUM_GRAPH:
				code_a_rendre = 1; break;
			  case OPIUM_PANEL:
				code_a_rendre = PNL_CANCEL; break;
			  default:
				code_a_rendre = 0; break;
			} else code_a_rendre = 0;
			break;
		}
		OpiumEvtNum++; if(OpiumEvtNum >= 0x7FFFFFFE) OpiumEvtNum = 1;
		OpiumDernierMessage[0] = '\0';
/*		OpiumDebugLevel[OPIUM_DEBUG_OPIUM] = 1; */
		code_a_rendre = OpiumManage(cdr_initial,&u,&cdr_ouverts);
/*		OpiumDebugLevel[OPIUM_DEBUG_OPIUM] = 0; */
		if(DEBUG_OPIUM(1)) WndPrint("(%s) Encore %d cadre%s ouvert%s, dont %d executable%s\n",__func__,Accord2s(cdr_ouverts),Accord1s(OpiumNbExec));
		/* deja prevu dans OpiumManage: if(OpiumCdrMinimal) { int i;
			for(i=0; i<OpiumNbCdr; i++) if(OpiumCdr[i]->displayed) break;
			if(i >= OpiumNbCdr) OpiumFork(OpiumCdrMinimal);
		} */
		OpiumAfficheMessage();
	}
	return(code_a_rendre);
}
/* ========================================================================== */
void OpiumPhotoDir(char *repert) {
    if(repert) strncpy(OpiumJpegEspace,repert,MAXFILENAME);
    else strcpy(OpiumJpegEspace,"~/Desktop");
    RepertoireTermine(OpiumJpegEspace);
	// printf("(%s) Photos sauvees dans %s\n",__func__,OpiumJpegEspace);
}
/* ========================================================================== */
void OpiumPhotoSauve() {
	char commande[256]; char nom[32]; int rc;

	// printf("(%s) Photos sauvees dans %s\n",__func__,OpiumJpegEspace);
	snprintf(nom,32,"Fig_%s.jpg",DateCompacte());
	sprintf(commande,"mkdir -p %s; screencapture -iWo %s%s",OpiumJpegEspace,OpiumJpegEspace,nom); // OSX_10_4: -iW
	rc = system(commande);
	OpiumDernierMessage[0] = '\0';
	if((rc >= 0) && (rc < 127)) sprintf(OpiumDernierMessage,"* [%d] Fenetre '%s' sauvee dans %s",rc,nom,OpiumJpegEspace);
	else sprintf(OpiumDernierMessage,"! [%d] Pas de sauvegarde de la fenetre",rc);
	OpiumAfficheMessage();
}
/* ========================================================================== */
int OpiumManage(Cadre cdr_initial, WndUserRequest *u, int *cdr_ouverts) {
	Cadre cdr,pointe; WndFrame f; WndIdent wid;
	int evt,key,zoom,x,y,i;
	int xevt,yevt; char evt_recadre;
	int code_a_rendre,code_rendu;
	char a_effacer,cle_interpretee;
	int rc;

	evt = u->type; i = 0;
	if(DEBUG_OPIUM(1)) WndPrint("=== OpiumManage === Evt '%s %s' @(%d x %d) + (%d x %d) pixels sur W=%08llX\n",
		(evt==-1)?"neant":OpiumEventType[evt],
		(evt==-1)?"de chez neant":((evt == WND_KEY)?u->texte:(((evt == WND_PRESS)||(evt == WND_RELEASE))?OpiumMseButton[u->code]:"")),
		u->x,u->y,u->h,u->v,(UInt64)u->w);
	if(evt == -1) return(0);
	evt_recadre = 0; wid = 0;

	if(evt == WND_KEY) {
		if(WndTopFrame == WND_AT_END) return(0); else wid = WndTopFrame->w;
		if((u->code == XK_Alt_L) && !strcmp(u->texte,"w")) evt = WND_DELETE;
#ifdef MENU_BARRE
	} else if(evt == WND_BARRE) {
		if(u->y == -1) {
			if(DEBUG_OPIUM(2)) WndPrint("(%s) Fenetre '%s' demandee\n",__func__,u->texte);
			if(u->code == 0) OpiumPhotoSauve();
			else for(i=0; i<OpiumNbCdr; i++ ) {
				cdr = OpiumCdr[i];
				if(cdr->displayed && !strcmp(cdr->nom,u->texte)) {
					if(DEBUG_OPIUM(2)) WndPrint("(%s) Fenetre '%s' recuperee\n",__func__,cdr->titre);
					WndSendToTop(cdr->f);
					break;
				}
			}
			if(DEBUG_OPIUM(2)) { if(i == OpiumNbCdr) WndPrint("(%s) Pas de fenetre a ce nom\n",__func__); }
			return(0);
		} else return(MenuBarreManage(u->y,u->code));
#endif
	} else {
		if(((evt == WND_PRESS) || (evt == WND_RELEASE)) && (OpiumMacroPhase ==  MACRO_STORE)) {
			u->type = -1;
			return(0);
		}
		if(DEBUG_OPIUM(3)) WndPrint("(%s) Evenement acceptable\n",__func__);
		wid = u->w;
	}
	cdr = OpiumCdrUtilise(wid);

	if(!cdr) {
		if(DEBUG_OPIUM(1)) WndPrint("(%s) Fenetre W%08llX hors application\n",__func__,(UInt64)wid);
		if(DEBUG_OPIUM(3)) {
			WndPrint("(%s) Fenetres de l'application:\n",__func__);
			for(i=0; i<OpiumNbCdr; i++) {
				WndPrint("%s %s, ",OpiumCdrType[(int)OpiumCdr[i]->type],OpiumCdr[i]->nom);
				f = OpiumCdr[i]->f;
				if(!f) WndPrint("pas de fenetre associee\n");
				else WndPrint("fenetre #%08llX %dx%d +(%d,%d)\n",(UInt64)f->w,f->x,f->y,f->h,f->v);
			}
		}
		return(0);
	}
#ifdef WIN32
	if(evt != WND_EXPOSE) OpiumLastRefreshed = 0;
#else
  #ifdef OPENGL
	if(evt != WND_EXPOSE) OpiumLastRefreshed = 0;
  #else
	if(evt != WND_CONFIG) OpiumLastRefreshed = 0;
  #endif
#endif
	pointe = cdr; /* pointe peut etre modifie selon evt (p.exe WND_KEY) */
	code_a_rendre = 0; /* ca, c'est encore a verifier! */
	xevt = yevt = 0; /* pour gcc */
	code_rendu = 0;  /* pour gcc */
	f = cdr->f;
	if(DEBUG_OPIUM(1)) WndPrint("(%s) Fenetre utilisee: C=%08llX F=%08llX W=%08llX wid=%08llX\n",__func__,
		(UInt64)cdr,(UInt64)f,(UInt64)f->w,(UInt64)wid);
/* 	OpiumDebugLevel[OPIUM_DEBUG_OPIUM] = 1; */
	switch(evt) {
	  case WND_EXPOSE:
		if(DEBUG_OPIUM(1)) WndPrint("(%s) -> %s %s '%s': C=%08llX F=%08llX\n",__func__,
			OpiumEventType[evt],OpiumCdrType[(int)cdr->type],cdr->nom,(UInt64)cdr,(UInt64)f);
		if(!f) break;
		WndUpdateBegin(f->w);
#ifdef WIN32
		if(!OpiumZoomEnCours)
#endif
			rc = OpiumRefreshIf(OPIUM_NECESSAIRE,cdr);
		WndUpdateEnd(f->w);
		if(DEBUG_OPIUM(1)) WndPrint("(%s) Fenetre du dessus: F=%08llX\n",__func__,(UInt64)WndTopFrame);
		if(DEBUG_OPIUM(1)) WndPrint("(%s) [WND_EXPOSE] Dims definitives: [%d x %d]\n",__func__,cdr->dh,cdr->dv);
		if(rc) WndShowTheTop();
		if(DEBUG_OPIUM(1)) WndPrint("(%s) -> %s %s '%s' termine\n",__func__,
			OpiumEventType[evt],OpiumCdrType[(int)cdr->type],cdr->nom);
		break;
#ifdef WIN32
	  case WND_REDRAW:
		  OpiumRefresh(cdr);
		  break;
#endif
	  case WND_REFRESH:
		if(DEBUG_OPIUM(1)) WndPrint("(%s) -> WndRefresh %s: C=%08llX F=%08llX\n",__func__,
			cdr->nom,(UInt64)cdr,(UInt64)f);
		if(!f) break;
		if(cdr->type == OPIUM_PANEL) {
			if(cdr != OpiumLastRefreshed) PanelRefreshVars((Panel)cdr->adrs);
		} else OpiumRefreshIf(OPIUM_NECESSAIRE,cdr);
		break;
	  case WND_CONFIG:
		if(DEBUG_OPIUM(1)) WndPrint("(%s) -> %s %s '%s': C=%08llX F=%08llX\n",__func__,
			OpiumEventType[evt],OpiumCdrType[(int)cdr->type],cdr->nom,(UInt64)cdr,(UInt64)f);
		if(!OpiumUseMouse) break;
		if(!f) break;
		cdr->defaultx = cdr->x = f->x = u->x;
		cdr->defaulty = cdr->y = f->y = u->y;
		cdr->dh = u->h;
		cdr->dv = u->v;
		if(cdr->dh >= cdr->larg) cdr->dx = 0;
		if(cdr->dv >= cdr->haut) cdr->dy = 0;
		switch(cdr->type) {
		  case OPIUM_MENU:    OpiumSizeMenu(cdr,1);   break;
		  case OPIUM_PANEL:   OpiumSizePanel(cdr,1);  break;
		  case OPIUM_TERM:    OpiumSizeTerm(cdr,1);   break;
		  case OPIUM_GRAPH:   OpiumSizeGraph(cdr,1);  break;
		  case OPIUM_INFO:    OpiumSizeInfo(cdr,1);   break;
		  case OPIUM_PROMPT:  OpiumSizePrompt(cdr,1); break;
		  case OPIUM_BOARD:   OpiumSizeBoard(cdr,1);  break;
		  case OPIUM_INSTRUM: OpiumSizeInstrum(cdr,1);  break;
		  case OPIUM_FIGURE:  OpiumSizeFigure(cdr,1);  break;
		  case OPIUM_ONGLET:  OpiumSizeOnglet(cdr,1);  break;
		}
		if(DEBUG_OPIUM(1)) WndPrint("(WND_CONFIG) Dims definitives: [%d x %d]\n",cdr->dh,cdr->dv);
		if(!cdr->displayed) break;
		f->h = cdr->dh; f->v = cdr->dv;
		WndResize(f);
#ifndef X11
		WndSendToTop(f);     //pas avec mwm
#endif
//-- au lieu de:		OpiumRefreshIf(OPIUM_NECESSAIRE,cdr);
		break;
	  case WND_MOVED:
		if(DEBUG_OPIUM(1)) WndPrint("-> %s %s '%s': C=%08X F=%08X\n",
									OpiumEventType[evt],OpiumCdrType[(int)cdr->type],cdr->nom,(hexa)cdr,(hexa)f);
		if(!OpiumUseMouse) break;
		if(!f) break;
		cdr->defaultx = cdr->x = f->x = u->x;
		cdr->defaulty = cdr->y = f->y = u->y;
		if(!cdr->displayed) break;
	#ifndef X11
		WndSendToTop(f);     //pas avec mwm
	#endif
		break;
	  case WND_RESIZE:  /* ca arrive ??? -> genere avec QuickDraw */
		if(DEBUG_OPIUM(1)) WndPrint("-> %s %s '%s': C=%08X F=%08X\n",
			OpiumEventType[evt],OpiumCdrType[(int)cdr->type],cdr->nom,(hexa)cdr,(hexa)f);
		if(!OpiumUseMouse) break;
		if(!cdr->displayed) break;
		if(!f) break;
		cdr->dh = u->h; cdr->dv = u->v;
		switch(cdr->type) {
		  case OPIUM_MENU:    OpiumSizeMenu(cdr,1); break;
		  case OPIUM_PANEL:   OpiumSizePanel(cdr,1); break;
		  case OPIUM_TERM:    OpiumSizeTerm(cdr,1); break;
		  case OPIUM_GRAPH:   OpiumSizeGraph(cdr,1); break;
		  case OPIUM_INFO:    OpiumSizeInfo(cdr,1); break;
		  case OPIUM_PROMPT:  OpiumSizePrompt(cdr,1); break;
		  case OPIUM_BOARD:   OpiumSizeBoard(cdr,1);  break;
		  case OPIUM_INSTRUM: OpiumSizeInstrum(cdr,1);  break;
		  case OPIUM_FIGURE:  OpiumSizeFigure(cdr,1);  break;
		  case OPIUM_ONGLET:  OpiumSizeOnglet(cdr,1);  break;
		}
		if(cdr->dh >= cdr->larg) cdr->dx = 0;
		if(cdr->dv >= cdr->haut) cdr->dy = 0;
		cdr->defaulth = cdr->dh;
		cdr->defaultv = cdr->dv;
		if(DEBUG_OPIUM(1)) WndPrint("(WND_RESIZE) Dims definitives: [%d x %d]\n",cdr->dh,cdr->dv);
		if(!cdr->displayed) break; /* 2eme fois... */
		f->h = cdr->dh; f->v = cdr->dv;
		WndResize(f);
		OpiumRefreshIf(OPIUM_TOUJOURS,cdr);
		break;
	  case WND_DELETE:
		if(DEBUG_OPIUM(1)) WndPrint("-> %s %s '%s': C=%08X F=%08X\n",
			OpiumEventType[evt],OpiumCdrType[(int)cdr->type],cdr->nom,(hexa)cdr,(hexa)f);
		OpiumClear(cdr);
		if((cdr->modexec == OPIUM_EXEC) || (cdr->modexec == OPIUM_SUBWND))
			*cdr_ouverts = *cdr_ouverts - 1;
		if(DEBUG_OPIUM(2)) WndPrint("Mode initial #%d, %d cadres encore ouverts\n",
			cdr->modexec,*cdr_ouverts);
#ifdef POSITION_PAR_TYPE
		OpiumPosPrevueX[(int)cdr->type] = cdr->pos_std_x;
		OpiumPosPrevueY[(int)cdr->type] = cdr->pos_std_y;
#else
		OpiumProchainX = cdr->pos_std_x;
		OpiumProchainY = cdr->pos_std_y;
#endif
		cdr->displayed = 0;
		if(pointe == cdr_initial) {
			if(pointe->type == OPIUM_PANEL) code_a_rendre = PNL_CANCEL;
			else code_a_rendre = 1;
			OpiumPrioritaire = 0;
		}
		if(OpiumCdrMinimal) {
			for(i=0; i<OpiumNbCdr; i++) if(OpiumCdr[i]->displayed) break;
			if(i >= OpiumNbCdr) OpiumRun(OpiumCdrMinimal,OpiumCdrMinimal->modexec);
		}
		break;
	#ifdef SANS_DOUTE_MAUVAIS
	  case WND_IN_USE:
		if(DEBUG_OPIUM(1)) WndPrint("-> %s %s '%s': C=%08X F=%08X\n",
			OpiumEventType[evt],OpiumCdrType[(int)cdr->type],cdr->nom,(hexa)cdr,(hexa)f);
		if(cdr->modexec == OPIUM_DISPLAY) break;
		/* if(!cdr->modexec) f = WndTopFrame; */
		if(DEBUG_OPIUM(3)) WndPrint("(OpiumManage) Frame @%08X au sommet (Evt)\n",(hexa)f);
//		if(f != WndTopFrame) WndRaise(f);
		break;
	#endif
	#ifdef WND_SCROLLX
	  case WND_SCROLLX:
		xevt = u->x; yevt = u->y; evt_recadre = 1;
		pointe = OpiumCdrPointe(cdr,u);
		if(DEBUG_OPIUM(1))
			WndPrint("-> %s %s '%s': C=%08X F=%08X\n",
				OpiumEventType[evt],OpiumCdrType[(int)pointe->type],pointe->nom,(hexa)pointe,(hexa)f);
		if(pointe->dh < pointe->larg) {
			zoom = pointe->larg / pointe->dh;
			x = u->x * zoom;
			if(x < pointe->dx) pointe->dx -= ((pointe->dh * 50) / 100);
			else if(x >= (pointe->dx + pointe->dh)) pointe->dx += ((pointe->dh * 50) / 100);
			else if(u->h == 0) break;
			else pointe->dx += (u->h * zoom);
			if(pointe->dx < 0) pointe->dx = 0;
			else if((pointe->dx + pointe->dh) > pointe->larg) {
				pointe->dx = pointe->larg - pointe->dh;
			}
			if(DEBUG_OPIUM(1)) WndPrint("Cadre simplement rafaichi en X\n");
			if(pointe->type == OPIUM_GRAPH) ((Graph)(pointe->adrs))->axe[0].pret = 0;
			OpiumRefreshIf(OPIUM_NECESSAIRE,pointe);
		}
		break;
	#endif
	#ifdef WND_SCROLLY
	  case WND_SCROLLY:
		xevt = u->x; yevt = u->y; evt_recadre = 1;
		pointe = OpiumCdrPointe(cdr,u);
		if(DEBUG_OPIUM(1))
			WndPrint("-> %s %s '%s': C=%08X F=%08X\n",
				OpiumEventType[evt],OpiumCdrType[(int)pointe->type],pointe->nom,(hexa)pointe,(hexa)f);
		if(DEBUG_OPIUM(2)) {
		  WndPrint("Largeur reelle: %d, cadre: %d\n",pointe->dh,pointe->larg);
		  WndPrint("Evt a %d, limite a %d\n",u->y,pointe->haut - WndLigToPix(1));
		  WndPrint("Deplacement: h=%d (v=%d)\n",u->h,u->v);
		}
		if(pointe->dv < pointe->haut) {
			zoom = pointe->haut / pointe->dv;
			y = u->y * zoom;
			if(y < pointe->dy) pointe->dy -= ((pointe->dv * 80) / 100);
			else if(y >= (pointe->dy + pointe->dv)) pointe->dy += ((pointe->dv * 80) / 100);
			else if(u->v == 0) break;
			else pointe->dy += (u->v * zoom);
			if(pointe->dy < 0) pointe->dy = 0;
			else if((pointe->dy + pointe->dv) > pointe->haut) {
				pointe->dy = pointe->haut - pointe->dv;
			}
			if(DEBUG_OPIUM(1)) WndPrint("Cadre simplement rafaichi en Y\n");
			if(pointe->type == OPIUM_GRAPH) ((Graph)(pointe->adrs))->axe[1].pret = 0;
			OpiumRefreshIf(OPIUM_NECESSAIRE,pointe);
		}
		break;
	#endif
	  case WND_KEY:
		xevt = u->x; yevt = u->y; evt_recadre = 1;
		pointe = OpiumCdrPointe(cdr,u); /* u->x,y calcules d'apres curseur */
		if(DEBUG_OPIUM(1)) WndPrint("-> %s %s '%s': C=%08X F=%08X\n",
			OpiumEventType[evt],OpiumCdrType[(int)pointe->type],pointe->nom,(hexa)pointe,(hexa)f);
		if(u->texte[0]) key = (int)(u->code << 16) | (int)(u->texte[0]); else key = (int)(u->code);
		if(pointe->log) {
			if(u->code == XK_Alt_L) WndPrint("Alt-");
			if(isprint((int)(u->texte[0]))) LogBook("%c",(int)(u->texte[0]));
			else if((key > 0) && (key < 27)) LogBook("<^%c>",key+(int)'A'-1);
			else LogBook("<%02X>",(int)(u->texte[0]));
		}
		if(OpiumEntryPost) (*OpiumEntryPost)(key);
		if(DEBUG_OPIUM(3)) WndPrint("(OpiumManage) Caractere entre: %02X\n",key);
		cle_interpretee = 0;
		for(i=0; i<OPIUM_MAXKEYS; i++) {
			if(DEBUG_OPIUM(3)) WndPrint("(OpiumManage) Cle testee: %02X\n",pointe->key[i].code);
			if(!pointe->key[i].code) break;
			else if(pointe->key[i].code == key) {
				TypeOpiumFctnInt fctn;
				if((fctn = (TypeOpiumFctnInt)pointe->key[i].fctn)) code_rendu = (*fctn)(pointe,key);
				a_effacer = (code_rendu != 0);
				if(DEBUG_OPIUM(3)) WndPrint("(OpiumManage) Cle utilisee: rendu %d (fenetre a %s)\n",
				code_rendu,a_effacer?"effacer":"garder");
				if(pointe == cdr_initial) code_a_rendre = code_rendu;
				if(a_effacer) {
					OpiumPrioritaire = 0;
					OpiumClear(pointe);
					if((pointe->modexec == OPIUM_EXEC) || (pointe->modexec == OPIUM_SUBWND))
						*cdr_ouverts = *cdr_ouverts - 1;
					if(DEBUG_OPIUM(1)) WndPrint("(OpiumManage) %s termine, %d cadre(s) encore ouvert(s)\n",
						OpiumExecName[(int)pointe->modexec],*cdr_ouverts);
				}
				cle_interpretee = 1;
				break;
			}
		}
		if(cle_interpretee) break;
	  case WND_PRESS:
	  case WND_RELEASE:

#ifdef WIN32_DEBUG	
		{
			char tmp[80];
			sprintf(tmp, "Mouse Pressed x:%d y:%d dh:%d dv:%d dx:%d dy:%d larg:%d haut:%d type:%d modexec:%d", 
				u->x, u->y, pointe->dh, pointe->dv, pointe->dx, pointe->dy, pointe->larg, pointe->haut, pointe->type, pointe->modexec);
			OpiumWin32Log(tmp);
		}
#endif

		if(evt != WND_KEY) {
			OpiumDernierClicX = u->x; OpiumProchainX = u->x + cdr->x;
			OpiumDernierClicY = u->y; OpiumProchainY = u->y + cdr->y;
			xevt = u->x; yevt = u->y; evt_recadre = 1;
			pointe = OpiumCdrPointe(cdr,u);  /* change u->x et u->y pour les rendre locaux */
			/* if(pointe->type == OPIUM_MENU) {
				Menu menu;
				menu = (Menu)pointe->adrs;
				printf("(%s.1) @%08X: %s.type=%d -> %08X\n",__func__,(hexa)menu->items,menu->items[0].texte,menu->items[0].type,(hexa)(menu->items[0].adrs));
			} */
			if(DEBUG_OPIUM(1)) WndPrint("-> %s %s '%s': C=%08X F=%08X\n",
				OpiumEventType[evt],pointe?OpiumCdrType[(int)pointe->type]:"(?)",pointe?pointe->nom:"(nul)",(hexa)pointe,(hexa)f);
			if((pointe->dh < pointe->larg) && ((u->y > pointe->dv) || OpiumAscenseurEnCours)) /* (pointe->dv - WND_ASCINT_WIDZ) */ {
				OpiumAscenseurEnCours = (evt == WND_PRESS);
				//- zoom = (pointe->larg + pointe->dh) / pointe->dh; x = u->x * zoom;
				if(pointe->dh > (2 * WND_ASCINT_WIDZ)) zoom = pointe->larg / (pointe->dh - (2 * WND_ASCINT_WIDZ)); else zoom = pointe->larg / 2;
				x = (u->x - WND_ASCINT_WIDZ) * zoom;
				if(x < pointe->dx) {
					if(u->x <= WND_ASCINT_WIDZ) pointe->dx -= (u->x + 1) / 2; // ((WND_ASCINT_WIDZ - u->x + 1) / 2);
					else pointe->dx -= (pointe->dh / 2);
				} else if(x >= (pointe->dx + pointe->dh)) {
					if(u->x >= (pointe->dh - WND_ASCINT_WIDZ)) pointe->dx += (pointe->dh - u->x + 1) / 2; // ((u->x - (pointe->dh - WND_ASCINT_WIDZ) + 1) / 2);
					else pointe->dx += (pointe->dh / 2);
				} else if(u->h != 0) pointe->dx += (u->h * zoom);
				else break;
				if(pointe->dx < 0) pointe->dx = 0;
				else if((pointe->dx + pointe->dh) > pointe->larg) pointe->dx = pointe->larg - pointe->dh;
				if(DEBUG_OPIUM(1)) WndPrint("(OpiumManage) Cadre simplement rafaichi en X\n");
				if(pointe->type == OPIUM_GRAPH) ((Graph)(pointe->adrs))->axe[0].pret = 0;
				OpiumRefreshIf(OPIUM_NECESSAIRE,pointe);
				break;
			}
			if(((pointe->dv < pointe->haut) && ((u->x > pointe->dh) || OpiumAscenseurEnCours)) || (u->code == WND_MSEMONTE) || (u->code == WND_MSEDESC)) /* (pointe->dh - WND_ASCINT_WIDZ) */ {
				OpiumAscenseurEnCours = (evt == WND_PRESS);
				if(u->code == WND_MSEMONTE) {
					pointe->dy -= WndLigToPix(1); // (pointe->dv / 2);
				} else if(u->code == WND_MSEDESC) {
					pointe->dy += WndLigToPix(1); // (pointe->dv / 2);
				} else {
					//- zoom = (pointe->haut + pointe->dv) / pointe->dv; y = u->y * zoom;
					if(pointe->dv > (2 * WND_ASCINT_WIDZ)) zoom = pointe->haut / (pointe->dv - (2 * WND_ASCINT_WIDZ)); else zoom = pointe->haut / 2;
					y = (u->y - WND_ASCINT_WIDZ) * zoom;
					if(y < pointe->dy) {
						if(u->y <= WND_ASCINT_WIDZ) pointe->dy -= (u->y + 1) / 2;
						else pointe->dy -= (pointe->dv / 2);
					} else if(y >= (pointe->dy + pointe->dv)) {
						if(u->y >= (pointe->dv - WND_ASCINT_WIDZ)) pointe->dy += (pointe->dv - u->y + 1) / 2;
						else pointe->dy += (pointe->dv / 2);
					} else if(u->v != 0) pointe->dy += (u->v * zoom);
					else break;
				}
				if(pointe->dy < 0) pointe->dy = 0;
				else if((pointe->dy + pointe->dv) > pointe->haut) pointe->dy = pointe->haut - pointe->dv;
				if(DEBUG_OPIUM(1)) WndPrint("(OpiumManage) Cadre simplement rafaichi en Y\n");
				if(pointe->type == OPIUM_GRAPH) ((Graph)(pointe->adrs))->axe[1].pret = 0;
				OpiumRefreshIf(OPIUM_NECESSAIRE,pointe);
				break;
			}
		}
		if(OpiumPrioritaire) {
			if(pointe != OpiumPrioritaire) {
				/* if(OpiumPrioritaire->modexec == OPIUM_SUBWND) {
					OpiumClear(OpiumPrioritaire); *cdr_ouverts = *cdr_ouverts - 1;
					OpiumPrioritaire = 0; code_a_rendre = 0;
				} else */ WndSendToTop(OpiumPrioritaire->f);
				break;
			}
		}
		if(pointe->debranche) break;
		if(pointe->log && ((evt == WND_PRESS) || (evt == WND_RELEASE))) LogBook("<M%d>",u->code);
		a_effacer = 0;
			//- OpiumDebugLevel[OPIUM_DEBUG_OPIUM] = 3;
		if(DEBUG_OPIUM(1)) WndPrint("(OpiumManage) Fenetre active de type %s\n",pointe?OpiumCdrType[(int)pointe->type]:"inconnu");
		switch(pointe->type) {
		  case OPIUM_MENU:
		    if(DEBUG_OPIUM(3)) WndPrint("(OpiumManage) Traitement par OpiumRunMenu\n");
			// printf("avant: pointe @%08X, code_rendu @%08X=%x\n",(hexa)pointe,(hexa)(pointe->code_rendu),pointe->code_rendu);
			code_rendu = OpiumRunMenu(pointe,u,cdr_ouverts);
			// printf("apres: pointe @%08X, code_rendu @%08X=%x\n",(hexa)pointe,(hexa)(pointe->code_rendu),pointe->code_rendu);
			a_effacer = (code_rendu != 0);
			break;
		  case OPIUM_PANEL:
		    if(DEBUG_OPIUM(3)) WndPrint("(OpiumManage) Traitement par OpiumRunPanel\n");
			code_rendu = OpiumRunPanel(pointe,u);
			if(code_rendu == PNL_NBREPLIES) return(0);
			a_effacer = (code_rendu > PNL_APPLY);
			break;
		  case OPIUM_TERM:
		    if(DEBUG_OPIUM(3)) WndPrint("(OpiumManage) Traitement par OpiumRunTerm\n");
			code_rendu = OpiumRunTerm(pointe,u);
			a_effacer = (code_rendu > 0);
			break;
		  case OPIUM_GRAPH:
		    if(DEBUG_OPIUM(3)) WndPrint("(OpiumManage) Traitement par OpiumRunGraph\n");
			code_rendu = OpiumRunGraph(pointe,u);
			a_effacer = (code_rendu > 0);
			break;
		  case OPIUM_INFO:
		    if(DEBUG_OPIUM(3)) WndPrint("(OpiumManage) Traitement par OpiumRunInfo\n");
			code_rendu = OpiumRunInfo(pointe,u);
			a_effacer = (code_rendu >= 0);
			break;
		  case OPIUM_PROMPT:
		    if(DEBUG_OPIUM(3)) WndPrint("(OpiumManage) Traitement par OpiumRunPrompt\n");
			code_rendu = OpiumRunPrompt(pointe,u);
			a_effacer = (code_rendu >= 0);
			break;
		  case OPIUM_BOARD:
		    if(DEBUG_OPIUM(3)) WndPrint("(OpiumManage) Traitement par OpiumRunBoard\n");
			code_rendu = OpiumRunBoard(pointe,u);
			a_effacer = (code_rendu > 0);
			break;
		  case OPIUM_INSTRUM:
		    if(DEBUG_OPIUM(3)) WndPrint("(OpiumManage) Traitement par OpiumRunInstrum\n");
			code_rendu = OpiumRunInstrum(pointe,u);
			a_effacer = (code_rendu >= 0);
			break;
		  case OPIUM_FIGURE:
		    if(DEBUG_OPIUM(1)) WndPrint("(OpiumManage) Traitement par OpiumRunFigure\n");
			code_rendu = OpiumRunFigure(pointe,u);
			a_effacer = (code_rendu > 0);
			break;
		  case OPIUM_ONGLET:
		    if(DEBUG_OPIUM(1)) WndPrint("(OpiumManage) Traitement par OpiumRunOnglet\n");
			code_rendu = OpiumRunOnglet(pointe,u);
			a_effacer = (code_rendu > 0);
			break;
		}
		if(DEBUG_OPIUM(1)) {
//			WndPrint("(OpiumManage) Reference @%08X: %s %s, pointe @%08X: %s %s\n",
//					 (hexa)cdr,cdr?OpiumCdrType[(int)cdr->type]:"(?)",cdr?cdr->nom:"(nul)",(hexa)pointe,pointe?OpiumCdrType[(int)pointe->type]:"(?)",pointe?pointe->nom:"(nul)");
			WndPrint("(OpiumManage) Reference @%08X, pointe @%08X\n",(hexa)cdr,(hexa)pointe);
			WndPrint("(OpiumManage) Code rendu=%d, fenetre %s\n",code_rendu,a_effacer?"a effacer":"a garder");
			WndPrint("(OpiumManage) Jusque la, %d cadres ouverts, dont %d executables\n",
				*cdr_ouverts,OpiumNbExec);
		}
		pointe->code_rendu = code_rendu;
		if(pointe == cdr_initial) code_a_rendre = code_rendu;
		if(a_effacer) {
			code_a_rendre = code_rendu;
			OpiumPrioritaire = 0;
			OpiumClear(pointe);
			if((pointe->modexec == OPIUM_EXEC) || (pointe->modexec == OPIUM_SUBWND))
				*cdr_ouverts = *cdr_ouverts - 1;
			if(DEBUG_OPIUM(1)) WndPrint("(OpiumManage) %s termine, %d cadre(s) encore ouvert(s)\n",
				OpiumExecName[(int)pointe->modexec],*cdr_ouverts);
		}
			//- OpiumDebugLevel[OPIUM_DEBUG_OPIUM] = 0;
		break;
	  default:
		if(DEBUG_OPIUM(2))
			WndPrint("(OpiumManage) Evt '%s' (#%d) non traite\n",WndEventName[evt],evt);
		cdr = OpiumCdr[0]; /* histoire de ne pas declencher d'autre erreur */
		break;
	}
/* 	OpiumDebugLevel[OPIUM_DEBUG_OPIUM] = 0; */
	if(DEBUG_OPIUM(9)) WndPrint("(OpiumManage) %s termine, %d cadre(s) encore ouvert(s)\n",OpiumExecName[(int)pointe->modexec],*cdr_ouverts);
	if(!cdr && DEBUG_OPIUM(1))
		WndPrint("%s W=%08X: pas de cadre associe!\n",OpiumEventType[evt],(hexa)u->w);
	if(evt_recadre) { u->x = xevt; u->y = yevt; }
	return(code_a_rendre);
}
/* ========================================================================== */
void OpiumDernierClic(int *x, int *y) { *x = OpiumDernierClicX; *y = OpiumDernierClicY; }
/* ========================================================================== */
void OpiumUpdateFlush() {
	int cdr_ouverts;
	Cadre cdr; WndUserRequest u;

	cdr = 0;
	cdr_ouverts = 0;
	do {
		if(WndUpdateFlushed(&u)) break;
		else OpiumManage(cdr,&u,&cdr_ouverts);
	} while(1);
}
/* ========================================================================== */
INLINE int OpiumUserAction() {
	int cdr_ouverts;
	Cadre cdr;

	cdr = 0;
	cdr_ouverts = 0;
	if(WndEventNew(&OpiumLastEvent,WND_CHECK)) {
		OpiumDernierMessage[0] = '\0';
		OpiumManage(cdr,&OpiumLastEvent,&cdr_ouverts);
		OpiumAfficheMessage();
		return(1);
	} else {
		if(OpiumUserPeriodic) (*OpiumUserPeriodic)();
		return(0);
	}
}
/* ========================================================================== */
void OpiumScrollX(Cadre cdr, int dx) {
	if(!cdr) return;
	cdr->dx += dx;
	if(cdr->dx < 0) cdr->dx = 0;
	else if((cdr->dx + cdr->dh) > cdr->larg) cdr->dx = cdr->larg - cdr->dh;
}
/* ========================================================================== */
void OpiumScrollY(Cadre cdr, int dy) {
	if(!cdr) return;
	cdr->dy += dy;
	if(cdr->dy < 0) cdr->dy = 0;
	else if((cdr->dy + cdr->dv) > cdr->haut) cdr->dy = cdr->haut - cdr->dv;
}
/* ========================================================================== */
void OpiumScrollXset(Cadre cdr, int dx) {
	if(!cdr) return;
	cdr->dx = dx;
	if(cdr->dx < 0) cdr->dx = 0;
	else if((cdr->dx + cdr->dh) > cdr->larg) cdr->dx = cdr->larg - cdr->dh;
}
/* ========================================================================== */
void OpiumScrollYset(Cadre cdr, int dy) {
	if(!cdr) return;
	cdr->dy = dy;
	if(cdr->dy < 0) cdr->dy = 0;
	else if((cdr->dy + cdr->dv) > cdr->haut) cdr->dy = cdr->haut - cdr->dv;
}
/* ========================================================================== */
void OpiumScrollXmin(Cadre cdr) { cdr->dx = 0; }
/* ========================================================================== */
void OpiumScrollYmin(Cadre cdr) { cdr->dy = 0; }
/* ========================================================================== */
void OpiumScrollXmax(Cadre cdr) { cdr->dx = cdr->larg - cdr->dh; }
/* ========================================================================== */
void OpiumScrollYmax(Cadre cdr) { cdr->dy = cdr->haut - cdr->dv; }
#ifdef TIMER_NIVEAU_OPIUM
#ifdef _XtIntrinsic_h
/* ========================================================================== */
static void OpiumTimerRestart(Cadre cdr) {
	if(cdr->timer) {
		WndTimeOutFctn(cdr->f,OpiumTimerRestart,cdr,cdr->timer);
		if(cdr->type == OPIUM_PANEL) PanelRefreshVars((Panel)(cdr->adrs));
		else OpiumRefresh(cdr);
	}
}
/* .......................................................................... */
OpiumTimer(Cadre cdr, unsigned long timer) {
/* rafraichissement d'un cadre toutes les <timer> millisecondes */
	if(!cdr) return(0);
	cdr->timer = timer;
	OpiumTimerRestart(cdr);
	return(1);
}
#else  /* _XtIntrinsic_h */
#ifdef TIMER_PAR_EVT
#ifdef TIMER_UNIQUE
/* ========================================================================== */
int OpiumTimer(Cadre cdr, int timer) {
	unsigned char delai;
/* rafraichissement d'un cadre toutes les <timer> secondes */
	if(!cdr) return(0);
	delai = timer;
	WndWriteOnlyRefresh(cdr->f,delai);
	return(1);
}
#else  /* TIMER_UNIQUE */
/* ========================================================================== */
int OpiumTimer(Cadre cdr, int delai) {
	WndFrame f;
/* rafraichissement d'un cadre toutes les <timer> secondes */
	if(!cdr) return(0);
	if(!(f = cdr->f)) return(0);
#ifdef TIMER_V0
	if(delai) {
		if(!f->timer) f->timer = TimerCreate(WndRefreshPost,(void *)f);
		if(DEBUG_OPIUM(2)) WndPrint("(OpiumTimer) Demarrage du timer @%08X\n",(hexa)f->timer);
		TimerStart(f->timer,delai*TIMER_SECONDE);
	} else TimerStop(f->timer);
#else
	if(delai) WndRefreshStart(f,delai); else WndRefreshStop(f);
#endif
	return(1);
}
#endif  /* TIMER_UNIQUE */
#else   /* TIMER_PAR_EVT (donc ici, timer gere au niveau OPIUM et non WND) */
/* ========================================================================== */
int OpiumTimer(Cadre cdr, int delai) {
	/* rafraichissement d'un cadre toutes les <delai> millisecondes */
	int i;

	if(!cdr) return(0);
	cdr->delai = cdr->restant = delai;
	for(i=0; i<OpiumNbRefresh; i++) {
		if(OpiumToRefresh[i] == cdr) {
			if(delai == 0) {
				--OpiumNbRefresh;
				for( ; i<OpiumNbRefresh; i++) OpiumToRefresh[i] = OpiumToRefresh[i+1];
			}
			return(1);
		}
	}
	if(OpiumNbRefresh >= OPIUM_MAXCDR) return(0);
	OpiumToRefresh[OpiumNbRefresh] = cdr;
	OpiumNbRefresh++;
	return(1);
}
#endif  /* TIMER_PAR_EVT */
#endif  /* _XtIntrinsic_h */
#endif  /* TIMER_NIVEAU_OPIUM */
/* ========================================================================== */
static int OpiumFermeComposant(Cadre cdr) {
	TypeOpiumFctnArg fctn; int rc;
	WndFrame f;
	Menu menu; MenuItem *im;
	Panel panel; PanelItem *ip; int nb,i,j;

	if((fctn = cdr->exit)) rc = (*fctn)(cdr,cdr->exit_arg); else rc = 0;
	f = cdr->f;
	if(DEBUG_OPIUM(2)) WndPrint("(%s) Fonction de sortie appelee, rc=%d\n",__func__,rc);
	switch(cdr->type) {
	  case OPIUM_MENU:
		menu = (Menu)cdr->adrs; if(!menu) return(0);
		im = menu->items; if(!im) return(0);
		while(im->texte) {
//			printf("Bouton '%s' @%08X ..\n",im->texte,im->gc);
			if(im->gc && f) { WndContextFree(f,im->gc); im->gc = 0; }
//			printf(".. eteint\n");
			im++;
		}
		break;
	  case OPIUM_PANEL:
		panel = (Panel)cdr->adrs; if(!panel) return(0);
		ip = panel->items; if(!ip) return(0);
		nb = panel->cur;
		for(i=0; i<nb; i++,ip++) {
			if(ip->gc) for(j=0; j<ip->nbfonds; j++) if(ip->gc[j] && f) {
				WndContextFree(f,ip->gc[j]); ip->gc[j] = 0;
			}
		}
		break;
	}
	if((cdr->modexec > OPIUM_DISPLAY) && (!cdr->planche)) OpiumNbExec--;
	if(DEBUG_OPIUM(1)) WndPrint("(%s) Fermeture %s %s '%s' (%d executable%s)\n",__func__,
		OpiumCdrType[(int)cdr->type],(cdr->modexec == OPIUM_DISPLAY)?"affiche":"executable",
		cdr->titre,Accord1s(OpiumNbExec));
	return(rc);
}
/* ========================================================================== */
static void OpiumFermePlanche(Cadre contenant) {
	Cadre boitier;

	boitier = (Cadre)contenant->adrs;
	while(boitier) {
		if(boitier->type == OPIUM_BOARD) OpiumFermePlanche(boitier);
		if(boitier->var_modifiees) contenant->var_modifiees = 1;
		OpiumFermeComposant(boitier);
		boitier->f = 0;
		boitier->displayed = 0;
		boitier = boitier->suivant;
	}
}
/* ========================================================================== */
int OpiumClear(Cadre cdr) {
/* effacement d'un cadre de l'ecran */
	int rc; Cadre contenant; TypeOpiumFctnArg fctn;

	if(!cdr) return(0);
	contenant = cdr;
	while(contenant->planche) contenant = contenant->planche;
	if(contenant->type == OPIUM_BOARD) OpiumFermePlanche(contenant);
	rc = OpiumFermeComposant(contenant);
	WndClear(contenant->f);
#ifdef MENU_BARRE
	WndMenuClear(contenant->nom);
#endif
	if(DEBUG_OPIUM(1)) WndPrint("(%s) Fenetre effacee: F=%08X\n",__func__,(hexa)contenant->f);
	contenant->f = 0;
	contenant->displayed = 0;
#ifdef POSITION_PAR_TYPE
	OpiumPosPrevueX[(int)contenant->type] = contenant->pos_std_x;
	OpiumPosPrevueY[(int)contenant->type] = contenant->pos_std_y;
#else
	OpiumProchainX = contenant->pos_std_x;
	OpiumProchainY = contenant->pos_std_y;
#endif
	if(DEBUG_OPIUM(1)) WndPrint("(%s) Reste %d frame(s) executable(s)\n",__func__,OpiumNbExec);
	if((fctn = cdr->clear)) rc = (*fctn)(cdr,cdr->clear_arg);
	return(rc);
}
/* ======================================================================== */
/* ............................... Macros ..................................*/
/* ======================================================================== */
void OpiumMacroDelim(char *chaine) {
	if(chaine) strncpy(OpiumMacroCntrl,chaine,3);
	else {
		OpiumMacroCntrl[0] = '\0';
		OpiumMacroCntrl[1] = '\0';
		OpiumMacroCntrl[2] = '\0';
	}
}
/* ======================================================================== */
int OpiumMacroStore(char *nomlu)  {
	char *nom,action[4];

	if(*nomlu == '+') {
		nom = nomlu + 1; strcpy(action,"a+"); 
	} else if(*nomlu == '-') {
		nom = nomlu + 1; strcpy(action,"w+"); 
	} else {
		nom = nomlu; strcpy(action,"w"); 
	}
	if(!(OpiumMacroFile = fopen(nom,action))) return(0);
	OpiumMacroPhase =  MACRO_STORE;
/*
	OpiumPanelInfo = PanelCreate(1);
	PanelText(OpiumPanelInfo,"Enregistrement de",nom,strlen(nom));
	OpiumDisplay(OpiumPanelInfo->cdr);
	*/
	if(DEBUG_OPIUM(2)) WndPrint("OpiumMacroInfo @%08X\n",(hexa)OpiumMacroInfo);
	if(OpiumMacroInfo) {
		InfoErase(OpiumMacroInfo);
		InfoWrite(OpiumMacroInfo,0,"Enregistrement de %s",nom);
		OpiumDisplay(OpiumMacroInfo->cdr);
	}
	return(1);
}
/* ======================================================================== */
void OpiumMacroClose() {
	if(OpiumMacroFile) fclose(OpiumMacroFile);
	OpiumMacroPhase =  MACRO_NONE;
/*
	OpiumClear(OpiumPanelInfo->cdr);
	PanelDelete(OpiumPanelInfo);
	*/
	OpiumClear(OpiumMacroInfo->cdr);
}
/* ======================================================================== */
int OpiumMacroRecall(char *nom) {
	if(!(OpiumMacroFile = fopen(nom,"r"))) return(0);
	OpiumMacroPhase =  MACRO_RECALL;
/*
	OpiumPanelInfo = PanelCreate(1);
	PanelText(OpiumPanelInfo,"Execution de",nom,strlen(nom));
	OpiumDisplay(OpiumPanelInfo->cdr);
	*/
	if(OpiumMacroInfo) {
		InfoErase(OpiumMacroInfo);
		InfoWrite(OpiumMacroInfo,0,"Execution de %s",nom);
		OpiumDisplay(OpiumMacroInfo->cdr);
	}
	return(1);
}
/* ======================================================================== */
int OpiumMacroBefore() {
	int rep;

	if(OpiumMacroPhase ==  MACRO_RECALL) {
		rep = getc(OpiumMacroFile);
		if(feof(OpiumMacroFile)) {
			OpiumMacroClose(); rep = 0; 
		}
		if(DEBUG_OPIUM(2)) WndPrint("(OpiumMacroBefore) cle recuperee=%02x\n",rep);
	} else rep = 0;
	return(rep);
}
/* ======================================================================== */
int OpiumMacroAfter(int key) {
	Panel p;
	char rep;
	int rc;

	rep = (char)key;
	if(DEBUG_OPIUM(2)) WndPrint("(OpiumMacroAfter) cle entree=%02x\n",rep);
	if(rep == OpiumMacroCntrl[1]) {
		if(OpiumMacroPhase !=  MACRO_STORE) OpiumError("Pas de Macro en cours");
		else OpiumMacroClose();
	} else if(rep == OpiumMacroCntrl[2]) {
		if(!OpiumMacroBase())
		OpiumError("Debut Macro seulement depuis le menu principal");
		else if(OpiumMacroPhase !=  MACRO_NONE) 
		OpiumError("Macro precedente non terminee");
		else {
			p = PanelCreate(1);
			PanelText(p,"Nom du fichier a executer",OpiumMacroName,40);
			rc = OpiumExec(p->cdr);
			PanelDelete(p);
			if(rc == PNL_CANCEL) return(0);
			if(!OpiumMacroRecall(OpiumMacroName)) OpiumError("Lecture impossible");
		}
	}
	if(OpiumMacroPhase == MACRO_STORE) putc(rep,OpiumMacroFile);
	if(rep == OpiumMacroCntrl[0]) {
		if(!OpiumMacroBase())
			OpiumError("Debut Macro seulement depuis le menu principal");
		else if(OpiumMacroPhase !=  MACRO_NONE)
			OpiumError("Macro precedente non terminee");
		else {
			p = PanelCreate(1);
			PanelText(p,"Nom du fichier de commande",OpiumMacroName,40);
			rc = OpiumExec(p->cdr);
			PanelDelete(p);
			if(rc == PNL_CANCEL) return(0);
			if(!OpiumMacroStore(OpiumMacroName)) OpiumError("Ecriture impossible");
		}
	}
	return(0);
}
/* ========================================================================== */
int OpiumMacroBase() { return(OpiumNbExec == 1); }
/* ========================================================================== */
void OpiumEditDeselecte(WndFrame f) {
//2	printf("(%s) suppression selection depuis 0x%08X\n",__func__,(hexa)f);
	if(OpiumEditSurligne[0]) {
		WndClearText(OpiumEditFen,WND_GC(OpiumEditFen,OpiumEditType),OpiumEditLig,OpiumEditCol,(int)strlen(OpiumEditSurligne));
		WndWrite(OpiumEditFen,WND_GC(OpiumEditFen,OpiumEditType),OpiumEditLig,OpiumEditCol,OpiumEditSurligne);
//2		printf("(%s) texte '%s' reaffiche dans 0x%08X(%d,%d) en mode %d\n",__func__,OpiumEditSurligne,
//2			   (hexa)OpiumEditFen,OpiumEditLig,OpiumEditCol,OpiumEditType);
		OpiumUserAction();
	}
	OpiumEditSurligne[0] = '\0';
	OpiumEditCar0 = -1;
}
/* ========================================================================== */
int OpiumEditCopier(Cadre cdr, int cle) {
	if(OpiumEditSurligne[0]) strcpy(OpiumEditReserve,OpiumEditSurligne);
	//+ printf("(%s) Clipboard: '%s'\n",__func__,OpiumEditReserve);
	return(0);
}
/* ========================================================================== */
int OpiumEditColler(Cadre cdr, int cle) {
	char valeur[256]; int nb,n; int poub;

	if(!OpiumEditReserve[0]) return(0);
	strncpy(valeur,OpiumEditItem->ptrval,OpiumEditCurs); valeur[OpiumEditCurs] = '\0';
	strcat(valeur,OpiumEditReserve);
	n = (int)strlen(OpiumEditItem->ptrval);
	/* if(panel->overstrike) nb = (int)strlen(OpiumEditReserve); else */
    if(OpiumEditSurligne[0]) nb = (int)strlen(OpiumEditSurligne);
	else nb = 0;
	if((OpiumEditCurs + nb) < n) strcat(valeur,OpiumEditItem->ptrval+OpiumEditCurs+nb);
	PanelCode(OpiumEditItem,PNL_VERIFY,valeur,&poub);
	//+ printf("(%s) %s = '%s'\n",__func__,OpiumEditItem->texte,OpiumEditItem->ptrval);
	WndClearText(OpiumEditFen,WND_GC(OpiumEditFen,OpiumEditType),OpiumEditLig,OpiumEditCol,OpiumEditItem->maxdisp);
	WndWrite(OpiumEditFen,WND_GC(OpiumEditFen,OpiumEditType),OpiumEditLig,OpiumEditCol,OpiumEditItem->ptrval);
	//+ printf("(%s) texte '%s' reaffiche dans 0x%08X(%d,%d) en mode %d\n",__func__,OpiumEditItem->ptrval,(hexa)OpiumEditFen,OpiumEditLig,OpiumEditCol,OpiumEditType);
	
	return(0);
}
/* ========================================================================== */
int OpiumEditEffacer(Cadre cdr, int cle) {
	char valeur[256]; int nb,n; int poub;
	
	if(!OpiumEditReserve[0]) return(0);
	strcpy(OpiumEditReserve,OpiumEditSurligne);
	//+ printf("(%s) Clipboard: '%s'\n",__func__,OpiumEditReserve);
	strncpy(valeur,OpiumEditItem->ptrval,OpiumEditCurs); valeur[OpiumEditCurs] = '\0';
	n = (int)strlen(OpiumEditItem->ptrval);
	nb = (int)strlen(OpiumEditSurligne);
	if((OpiumEditCurs + nb) < n) strcat(valeur,OpiumEditItem->ptrval+OpiumEditCurs+nb);
	PanelCode(OpiumEditItem,PNL_VERIFY,valeur,&poub);
	//+ printf("(%s) %s = '%s'\n",__func__,OpiumEditItem->texte,OpiumEditItem->ptrval);
	WndClearText(OpiumEditFen,WND_GC(OpiumEditFen,OpiumEditType),OpiumEditLig,OpiumEditCol,OpiumEditItem->maxdisp);
	WndWrite(OpiumEditFen,WND_GC(OpiumEditFen,OpiumEditType),OpiumEditLig,OpiumEditCol,OpiumEditItem->ptrval);
	//+ printf("(%s) texte '%s' reaffiche dans 0x%08X(%d,%d) en mode %d\n",__func__,OpiumEditItem->ptrval,(hexa)OpiumEditFen,OpiumEditLig,OpiumEditCol,OpiumEditType);
	return(0);
}
/* ========================================================================== */
void OpiumTxtHome(char *path) {
	if(path) strcpy(OpiumTxtPath,path); else OpiumTxtPath[0] = '\0';
}
/* ========================================================================== */
int OpiumTxtCreer() {
	if(OpiumTxtNom[0] == '\0') {
		if(OpiumTxtPath[0] == '\0') strcpy(OpiumTxtNom,"opium.txt");
		else strcat(strcpy(OpiumTxtNom,OpiumTxtPath),"opium.txt");
	}
	if(OpiumReadFile("Dans le fichier",OpiumTxtNom,OPIUM_MAXFILE) == PNL_CANCEL) return(0);
	if(!(OpiumTxtfile = fopen(OpiumTxtNom,"w"))) {
		OpiumError("Impossible d'ecrire sur %s",OpiumTxtNom);
		return(0);
	}
	return(1);
}
/* ========================================================================== */
void OpiumPSformat(OPIUM_PS_FORMAT choix) {
	OpiumPSzoome = choix; if(OpiumPSzoome) OpiumPSutilise = (float)OpiumServerWidth(WND_A4);
}
/* ========================================================================== */
void OpiumPSlargeur(float cm) {
	OpiumPSzoome = (cm > 0.0); if(OpiumPSzoome) OpiumPSutilise = cm * WND_PIXEL_CM;
}
/* ========================================================================== */
void OpiumPSentete(char *texte) { strncpy(OpiumPStitre,texte,OPIUM_MAXTITRE); }
/* ========================================================================== */
int OpiumPSInit(char *nom, int page) {
	if(nom) strncpy(OpiumPSnom,nom,OPIUM_MAXFILE);
	if(WndPSopen(OpiumPSnom,(page > 0)? "a": "w")) {
		OpiumPSpage = page; OpiumPSenCours = 1; 
		OpiumPSx = OPIUM_PSMARGE_X;
		OpiumPSy = OpiumPSytot = OPIUM_PSMARGE_Y;
		return(1);
	} else {
		OpiumError("Impossible d'ecrire sur %s",OpiumPSnom); return(0);
	}
}
/* ========================================================================== */
void OpiumPSPosition(int x, int y) {
	OpiumPSx = x;
	OpiumPSy = OpiumPSytot = y;
}
/* ========================================================================== */
void OpiumPSNouvellePage() {
	// WndPSfintrace();
	if(OpiumPSpage) WndPSpage(WND_PS_NOUVELLE);
	OpiumPSpage++;
	OpiumPSx = OPIUM_PSMARGE_X;
	OpiumPSy = OpiumPSytot = OPIUM_PSMARGE_Y;
	if(OpiumPStitre[0]) {
		int x,y;
		x = (OpiumServerWidth(WND_A4) - WndColToPix((int)strlen(OpiumPStitre))) / 2;
		y = OpiumPSy + WndLigToPix(1);
		WndPSstring(x,y,OpiumPStitre);
		OpiumPSy = y + WndLigToPix(1);
	}
}
/* ========================================================================== */
void OpiumPSRecopie(Cadre cdr) {
	int xsiz,ysiz,xzoom,yzoom; float facteur;
	Graph graph; // float xech,yech,xpixel,ypixel;
	WndFrame f; int qual;

	if(!cdr) return;
	graph = 0; xsiz = ysiz = 0; xzoom = yzoom = 1; // GCC
	if((cdr->type == OPIUM_GRAPH) && OpiumPSzoome) {
		graph = (Graph)cdr->adrs; if(!graph) return;
		facteur = OpiumPSutilise / (float)cdr->dh;
		// printf("(%s) facteur[%s] = %d / %d = %g\n",__func__,cdr->titre,OpiumServerWidth(WND_A4),cdr->larg,facteur);
		xsiz = cdr->dh; xzoom = graph->axe[0].zoom;
		ysiz = cdr->dv; yzoom = graph->axe[1].zoom;
		cdr->dh = (int)(facteur * (float)cdr->dh); /* soit OpiumPSutilise */
		cdr->dv = (int)(facteur * (float)cdr->dv); /* soit OpiumPSutilise x rapport initial hauteur/largeur */
		graph->axe[0].zoom = graph->axe[1].zoom = 1;
		OpiumSizeGraph(cdr,1);
	}
	
	if(!OpiumPSpage) OpiumPSNouvellePage();
	else {
		if((OpiumPSx > OPIUM_PSMARGE_X) && ((OpiumPSx + cdr->larg) > OpiumServerWidth(WND_A4))) {
			OpiumPSx = OPIUM_PSMARGE_X; OpiumPSy = OpiumPSytot + 5;
		}
		if((OpiumPSy > OPIUM_PSMARGE_Y) && ((OpiumPSy + cdr->haut) > OpiumServerHeigth(WND_A4))) 
			OpiumPSNouvellePage();
	}
	
	WndPSposition(OpiumPSx,OpiumPSy);
	// OpiumDisplay(cdr); devrait etre plus universel
	qual = WND_Q_ECRAN; // gcc
	if((f = cdr->f)) {
		qual = f->qualite;
		f->qualite = WND_Q_PAPIER;
	}
	OpiumRefresh(cdr);
	if(f) f->qualite = qual;
	WndPSfintrace();
	OpiumPSx += (cdr->larg /* + OPIUM_PSMARGE_X */);
	if((OpiumPSy + cdr->haut) > OpiumPSytot) OpiumPSytot = OpiumPSy + cdr->haut;

	if((cdr->type == OPIUM_GRAPH) && OpiumPSzoome) {
		cdr->dh = xsiz; graph->axe[0].zoom = xzoom;
		cdr->dv = ysiz; graph->axe[1].zoom = yzoom;
		OpiumSizeGraph(cdr,1);
	}
}
/* ========================================================================== */
void OpiumPSPause() { WndPSpage(WND_PS_DERNIERE); OpiumPSenCours = 0; }
/* ========================================================================== */
void OpiumPSReprend() { OpiumPSenCours = 1; WndPSpage(WND_PS_PREMIERE); }
/* ========================================================================== */
void OpiumPSClose() { WndPSpage(WND_PS_DERNIERE); WndPSclose(); OpiumPSenCours = 0; }
/* ========================================================================== */
int OpiumPSprint(Cadre cdr, int cle) {
	OpiumPScadre = cdr;
	OpiumExec(OpiumPSmenu->cdr);
	OpiumRefresh(cdr);
	return(0);
}
/* ========================================================================== */
void OpiumPShome(char *path) {
	if(path) {
		strncpy(OpiumPSpath,path,OPIUM_MAXFILE);
		RepertoireTermine(OpiumPSpath);
	} else OpiumPSpath[0] = '\0';
}
/* ========================================================================== */
int OpiumPSDemarre() {
	char *nom;
	
	if(OpiumPSpath[0] != '\0') nom = OpiumPSpath;
	else nom = OpiumJpegEspace;
	snprintf(OpiumPSnom,OPIUM_MAXFILE,"%sFig_%s.ps",nom,DateCompacte());
	OpiumPSnom[OPIUM_MAXFILE-1] = '\0';

	if(OpiumPScadre->titre[0]) nom = OpiumPScadre->titre;
	else if(OpiumPScadre->nom[0]) nom = OpiumPScadre->nom;
	else nom = OpiumCdrType[OpiumPScadre->type];
	strncpy(OpiumPStitre,nom,OPIUM_MAXTITRE);
	OpiumPStitre[OPIUM_MAXTITRE-1] = '\0';

	if(OpiumExec(OpiumPSpanel->cdr) == PNL_CANCEL) return(1);
	if(OpiumPSInit(0,0)) { OpiumPSRecopie(OpiumPScadre); WndPSclose(); OpiumPSenCours = 0; }

	return(0);
}
/* ========================================================================== */
int OpiumPSAjoute() {
	if(!WndPSopen(OpiumPSnom,"a")) {
		OpiumError("Impossible de re-ecrire sur %s",OpiumPSnom);
		return(0);
	}
	OpiumPSenCours = 1;
	OpiumPSRecopie(OpiumPScadre);
	WndPSclose(); OpiumPSenCours = 0;
	return(0);
}
/* ========================================================================== */
int OpiumPSPage() {
	if(OpiumReadText("Titre general",OpiumPStitre,32) == PNL_CANCEL) return(0);
	/* Affichage du panel avant WndPSopen car (WndPS!=0) => refresh sur fichier!!! */
	if(!WndPSopen(OpiumPSnom,"a")) {
		OpiumError("Impossible de re-ecrire sur %s",OpiumPSnom);
		return(0);
	}
	OpiumPSenCours = 1;
	OpiumPSNouvellePage();
	WndPSclose(); OpiumPSenCours = 0;
	return(0);
}
/* ========================================================================== */
int OpiumPSTermine() {
	if(!WndPSopen(OpiumPSnom,"a")) {
		OpiumError("Impossible de re-ecrire sur %s",OpiumPSnom);
		return(0);
	}
	OpiumPSenCours = 1;
	OpiumPSClose();
	return(1);
}
/* ========================================================================== */
int OpiumPSComplet() { OpiumPSDemarre(); OpiumPSTermine(); return(1); }

#define OPIUM_MAX_TERME 80
#define OPIUM_DICO_PAGE 24
/* ========================================================================== */
static int OpiumDicoCherche(Menu menu, MenuItem *item) {
	Dico dico; DicoLexique terme; char accepte[OPIUM_MAX_TERME],officiel[OPIUM_MAX_TERME];
	Panel p; char ajout; int i; size_t l;
	
	if(!(dico = OpiumDicoEnCours)) {
		OpiumError("Le dictionnaire a editer n'a pas ete designe");
		return(0);
	}
	if(OpiumReadText("Expression",accepte,OPIUM_MAX_TERME) == PNL_CANCEL) return(0);
	officiel[0] = '\0'; i = l = 0;
	if((terme = DicoTerme(dico,accepte,&i))) {
		ajout = -1;
		if(terme->officiel) { strncpy(officiel,terme->officiel,OPIUM_MAX_TERME); l = strlen(officiel); } 
	} else ajout = OpiumCheck("Expression absente du dictionnaire %s. L'ajouter?",dico->nom);
	if(ajout) {
		if(officiel[0] == '\0') { strncpy(officiel,accepte,OPIUM_MAX_TERME); l = 0; }
		p = PanelCreate(2);
		PanelItemSelect(p,PanelText(p,"Expression",accepte,OPIUM_MAX_TERME),0);
		PanelText(p,"Traduction",officiel,OPIUM_MAX_TERME);
		if(OpiumExec(p->cdr) == PNL_OK) {
			if(ajout > 0) DicoAjouteLigne(dico,i,accepte,officiel);
			else {
				if(l < strlen(officiel)) {
					if(terme->officiel) free(terme->officiel);
					l = (((strlen(officiel)) / 4) + 1) * 4;
					terme->officiel = (char *)malloc(l);
				}
				if(terme->officiel) strcpy(terme->officiel,officiel);
			}
		}
		PanelDelete(p);
	}
	
	return(0);
}
/* ========================================================================== */
typedef struct {
	char officiel[OPIUM_MAX_TERME];
	int ligne;
} TypeOpiumDicoLigne,*OpiumDicoLigne;
/* ========================================================================== */
static int OpiumDicoListe(Menu menu, MenuItem *item) {
	Dico dico; OpiumDicoLigne liste;
	int i,j,k,l,m,n,nb; Panel p;
	
	if(!(dico = OpiumDicoEnCours)) {
		OpiumError("Le dictionnaire a editer n'a pas ete designe");
		return(0);
	}
	nb = 0;
	for(i=0; i<dico->nbtermes; i++) if(dico->terme[i].accepte) {
		if(!(dico->terme[i].officiel) || (dico->terme[i].officiel[0] == '\0')) nb++;
	}
	if(!nb) {
		OpiumNote("Le dictionnaire '%s' n'a pas de terme sans traduction",dico->nom);
		return(0);
	}
	liste = Malloc(nb,TypeOpiumDicoLigne);
	j = 0;
	for(i=0; i<dico->nbtermes; i++) if(dico->terme[i].accepte) {
		if(!(dico->terme[i].officiel) || (dico->terme[i].officiel[0] == '\0')) {
			liste[j].officiel[0] = '\0';
			liste[j].ligne = i;
			j++;
		}
	}
	
	p = PanelCreate(2 * OPIUM_DICO_PAGE);
	j = 0;
	do {
		k = 0; m = j;
		while((j < nb) && (k < OPIUM_DICO_PAGE)) {
			i = liste[j].ligne;
			PanelItemSelect(p,PanelText(p,"Expression",dico->terme[i].accepte,OPIUM_MAX_TERME),0);
			PanelText(p,"Traduction",liste[j].officiel,OPIUM_MAX_TERME);
			j++; k++;
		}
		if(OpiumExec(p->cdr) == PNL_CANCEL) break;
		for(n=m; n<j; n++) if(liste[n].officiel[0]) {
			i = liste[n].ligne;
			l = (int)strlen(liste[n].officiel) + 1;
			if(dico->terme[i].officiel && (l >= (int)strlen(dico->terme[i].officiel))) {
				free(dico->terme[i].officiel); dico->terme[i].officiel = 0;
			}
			if(!(dico->terme[i].officiel)) {
				l = ((((int)strlen(liste[n].officiel)) / 4) + 1) * 4;
				dico->terme[i].officiel = (char *)malloc(l);
			}
			if(dico->terme[i].officiel) strcpy(dico->terme[i].officiel,liste[n].officiel);
		}
		PanelErase(p);
	} forever;
	PanelDelete(p);
	free(liste);
	
	return(0);
}
/* ========================================================================== */
void OpiumDicoEdite(Dico dico) {
	OpiumDicoEnCours = dico;
	OpiumExec(OpiumDicoMdial->cdr);
	OpiumDicoEnCours = 0;
}
/* ========================================================================== */
#ifdef macintosh
/* ========================================================================== */
void OpiumEcritReel(char *chaine, char *fmt, float reel) {
	sprintf(chaine,fmt,reel);
}
/* ========================================================================== */
void OpiumEcritDble(char *chaine, char *fmt, double dble) {
	sprintf(chaine,fmt,dble);
}
#endif

