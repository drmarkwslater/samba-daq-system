#ifndef NANOPAW_H
#define NANOPAW_H

#ifdef NANOPAW_C
#define NANOPAW_VAR
#else
#define NANOPAW_VAR extern
#endif

#include <opium.h>
#include <histos.h>

#define MAXPLOTS 32
#define MAXESPACES 4
#define MAXNOMESPACE 16

#define MAXTITREPLOT 80

typedef struct {
	float somme;
	float xmax;
	float ymax;
	float median;
	float moyenne;
	float sigma;
	float bin;
} TypePlotStat;

NANOPAW_VAR TypePlotStat PlotStat;

typedef enum {
	PLOT_FONCTION = 0,
	PLOT_HISTO,
	PLOT_H2D,
	PLOT_PROFILX,
	PLOT_PROFILY,
	PLOT_BIPLOT,
	PLOT_NBTYPES
} PLOT_TYPE;

typedef enum {
	PLOT_SEP_BLANC = 0,
	PLOT_SEP_PTVIRG,
	PLOT_SEP_TAB,
	PLOT_SEP_NB
} PLOT_TYPE_SEP;

typedef enum {
	MARQUE_NORMAL = 0,
	MARQUE_TAGGE,
	MARQUE_CARRE,
	MARQUE_TRIANGLE,
	MARQUE_ETOILE,
	MARQUE_ROND,
	MARQUE_ABSENT,
	MARQUE_NB
} CODE_MARQUAGE;  /* PLOT_MARQUE */

typedef enum {
	FIT_NEANT = 0,
	FIT_LINEAIRE,
	FIT_PARABOLIQUE,
	FIT_GAUSSIEN,
	FIT_NB
} FIT_MODE;

typedef struct {
	char automatique;
	float min,max;
} TypeLimite;

typedef struct {
	char titre[MAXTITREPLOT];
	int ab6,ord;
	float xmin,xmax; int binx;
	float ymin,ymax; int biny;
	char type;
	Histo h; HistoData hd;
	Graph g;
	float xdroite[2],ydroite[2];
	int dim; float *xcourbe,*ycourbe;
	char vide;
	char avec_fit;
	TypeLimite x,y,z;
} TypePlot;

typedef struct {
	char nom[MAXNOMESPACE];
	int  num; /* numero (index dans les tables PlotEspacePtr et PlotEspaceList) */
	int *dim; /* nombre d'evenements en cours utilisables                       */
	int  max; /* nombre d'evenements maxi dans l'allocation actuelle            */
	TypeGraphIndiv *sel;
	Panel pNtuple;
} TypePlotEspace,*PlotEspace;

typedef struct PlotVarComposante {
	float coef;
	short srce;
	struct PlotVarComposante *svt;
} TypePlotVarComposante;

typedef struct {
	char *nom;
	char *unite;
	float **adrs;
	float min,max;
	PlotEspace espace;
	TypePlotVarComposante *composante;
	char sel;
} TypePlotVar;

#define MAXVARFITTEES 5
typedef union {
	float var[MAXVARFITTEES];
	struct {
		float base,pente;
		float err_base,err_pente;
		float khi2;
	} lin;
	struct {
		float base,x0,a,b,c;
	} par;
	struct {
		float base,x0,amplitude,sigma;
	} gau;
} TypeResultatFit;

NANOPAW_VAR TypeResultatFit ResultatFit;

NANOPAW_VAR PlotEspace PlotEspacePtr[MAXESPACES]; NANOPAW_VAR int PlotEspaceNb;
NANOPAW_VAR char *PlotEspacesList[MAXESPACES+1];
NANOPAW_VAR TypePlotVar *PlotVar; NANOPAW_VAR int PlotVarNb; NANOPAW_VAR char **PlotVarList;
NANOPAW_VAR int NumVar; NANOPAW_VAR float VarMini,VarMaxi;
NANOPAW_VAR TypePlot Plot[MAXPLOTS],PlotLu; NANOPAW_VAR int PlotsNb;
NANOPAW_VAR int PlotCourant;
NANOPAW_VAR char PlotFitMode;

int PlotRedraw();
int PlotLimite();
int PlotRefresh();
int PlotSauve();
int PlotRestore();
int PlotErase();
NANOPAW_VAR Menu mPlot,mPlotExtract,mPlotFits,mPlotCourbes,mPlotInfos,mPlotTypes;
NANOPAW_VAR MenuItem iPlotExamine[]
#ifdef NANOPAW_C
= {
	{ "Retracer",           MNU_FONCTION   PlotRedraw },
//	{ "Extractions",        MNU_MENU    &mPlotExtract },
	#ifdef LIMITES_AXES
	{ "Limites axes",       MNU_FONCTION   PlotLimite },
	#endif
	{ "Fitter",             MNU_MENU     &mPlotFits },
	{ "Supplements",        MNU_MENU     &mPlotCourbes },
	{ "Infos",              MNU_MENU     &mPlotInfos },
	{ "Rafraichir",         MNU_FONCTION   PlotRefresh },
	{ "Sauver",             MNU_FONCTION   PlotSauve },
	{ "Relit",              MNU_FONCTION   PlotRestore },
	{ "Effacer",            MNU_FONCTION   PlotErase },
	MNU_END
}
#endif
;
NANOPAW_VAR Panel pPlotVarSel;
NANOPAW_VAR unsigned char CodeMarquage[]  /* --> PlotMarqueCode */
#ifdef NANOPAW_C
 = {
	GRF_CODE_PNT | GRF_CODE_POINT,
	GRF_CODE_PNT | GRF_CODE_CROSS,
	GRF_CODE_PNT | GRF_CODE_CARRE,
	GRF_CODE_PNT | GRF_CODE_TRGLE,
	GRF_CODE_PNT | GRF_CODE_STAR,
	GRF_CODE_PNT | GRF_CODE_ROND,
	GRF_NODISPLAY
}
#endif
;

NANOPAW_VAR char **PlotMarqueTraduit;
NANOPAW_VAR char *TexteMarquage[MARQUE_NB+1]  /* --> PlotMarqueTexte */
#ifdef NANOPAW_C
 = { "normal", "croix", "carre", "triangle", "etoile", "cercle", "absent", "\0" }
#endif
;
NANOPAW_VAR char **PlotTypeTraduit;
NANOPAW_VAR char *PlotTypeTexte[PLOT_NBTYPES+1]
#ifdef NANOPAW_C
 = { "fonction", "histo", "2D-histo", "profil X", "profil Y", "biplot", "\0" }
#endif
;
NANOPAW_VAR char PlotTypeCles[256];

NANOPAW_VAR int PlotCoordIndex[2]
#ifdef NANOPAW_C
 = { 0, 1 }
#endif
;

/* PAS POSSIBLE AVEC PlotVarList DYNAMIQUE:
 
NANOPAW_VAR ArgDesc PlotFctnDesc[]
#ifdef NANOPAW_C
 = { { "variable", DESC_LISTE PlotVarList, &PlotLu.ord,  0 }, DESC_END }
#endif
;
NANOPAW_VAR ArgDesc PlotHistoDesc[]
#ifdef NANOPAW_C
 = {
	{ "variable", DESC_LISTE PlotVarList, &PlotLu.ab6,  0 },
	{ "minimum",  DESC_FLOAT              &PlotLu.xmin, 0 },
	{ "maximum",  DESC_FLOAT              &PlotLu.xmax, 0 },
	{ "nb_bins",  DESC_INT                &PlotLu.binx, 0 },
	DESC_END
}
#endif
;
NANOPAW_VAR ArgDesc Plot2DDesc[]
#ifdef NANOPAW_C
 = {
	{ "x",         DESC_LISTE PlotVarList, &PlotLu.ab6,  0 },
	{ "x.min",     DESC_FLOAT              &PlotLu.xmin, 0 },
	{ "x.max",     DESC_FLOAT              &PlotLu.xmax, 0 },
	{ "x.nb_bins", DESC_INT                &PlotLu.binx, 0 },
	{ "y",         DESC_LISTE PlotVarList, &PlotLu.ord,  0 },
	{ "y.min",     DESC_FLOAT              &PlotLu.ymin, 0 },
	{ "y.max",     DESC_FLOAT              &PlotLu.ymax, 0 },
	{ "y.nb_bins", DESC_INT                &PlotLu.biny, 0 },
	DESC_END
}
#endif
;
NANOPAW_VAR ArgDesc PlotBiDesc[]
#ifdef NANOPAW_C
 = {
	{ "x", DESC_LISTE PlotVarList, &PlotLu.ab6,  0 },
	{ "y", DESC_LISTE PlotVarList, &PlotLu.ord,  0 },
	DESC_END
}
#endif
;
NANOPAW_VAR ArgDesc *PlotTypeDesc[]
#ifdef NANOPAW_C
 = { PlotFctnDesc, PlotHistoDesc, Plot2DDesc, PlotBiDesc, 0 }
#endif
;
 */

NANOPAW_VAR ArgDesc *PlotFctnDesc,*PlotHistoDesc,*Plot2DDesc,*PlotBiDesc;
NANOPAW_VAR ArgDesc *PlotTypeDesc[4];

NANOPAW_VAR ArgDesc PlotLimAutoDesc[]
#ifdef NANOPAW_C
 = { DESC_END }
#endif
;
NANOPAW_VAR ArgDesc PlotXlimManuDesc[]
#ifdef NANOPAW_C
 = {
	{ "x.min", DESC_FLOAT &PlotLu.x.min, 0 },
	{ "x.max", DESC_FLOAT &PlotLu.x.max, 0 },
	DESC_END
}
#endif
;
NANOPAW_VAR ArgDesc PlotYlimManuDesc[]
#ifdef NANOPAW_C
 = {
	{ "y.min", DESC_FLOAT &PlotLu.y.min, 0 },
	{ "y.max", DESC_FLOAT &PlotLu.y.max, 0 },
	DESC_END
}
#endif
;
NANOPAW_VAR ArgDesc *PlotXlimDesc[]
#ifdef NANOPAW_C
 = { PlotXlimManuDesc, PlotLimAutoDesc, 0 }
#endif
;
NANOPAW_VAR ArgDesc *PlotYlimDesc[]
#ifdef NANOPAW_C
 = { PlotYlimManuDesc, PlotLimAutoDesc, 0 }
#endif
;
NANOPAW_VAR ArgDesc PlotDesc[]
#ifdef NANOPAW_C
 = {
	{ 0, DESC_KEY PlotTypeCles,    &PlotLu.type,  0 },
	{ 0, DESC_VARIANTE(PlotLu.type) PlotTypeDesc, 0 },
	DESC_END
}
#endif
;
NANOPAW_VAR ArgStruct PlotAS
#ifdef NANOPAW_C
 = { (void *)Plot, (void *)&PlotLu, sizeof(TypePlot), PlotDesc }
#endif
;
NANOPAW_VAR ArgDesc PlotListeDesc[]
#ifdef NANOPAW_C
 = { { "Figures", DESC_STRUCT_SIZE(PlotsNb,MAXPLOTS) &PlotAS,  0 }, DESC_END }
#endif
;

PlotEspace PlotEspaceCree(char *nom, int *dim);
void PlotEspaceSupprime(PlotEspace espace);
char PlotVarCree(TypePlotVar *var, char *nom, char *unite, float **adrs, PlotEspace espace);
void PlotVarMinMax(TypePlotVar *var, float min, float max);
void PlotVarSelecte(TypePlotVar *var, char sel);
int PlotVarsDeclare(TypePlotVar *var);
void PlotVarsInterface();
char PlotNtupleCree(int max, float init);
char PlotNtupleAjoute(int i, int max, float init);
void PlotNtupleSupprime();
int PlotIndexVar(char *nom);
void PlotNtupleSeparateur(char sep);
int PlotNtupleEcrit(char *nom, char sel_var, char sel_evt, char ajoute);
int PlotNtupleRelit(char *nom, char ajoute);
void PlotNtupleAffiche(PlotEspace espace, int num, int x, int y, int h, int v);
/*void PlotNtupleTitre(int x, int y, char *texte);
void PlotNtuplePosition(int x, int y);
void PlotNtupleGetPos(int *x, int *y);
void PlotNtupleGetSize(int *h, int *v);
void PlotNtupleRefresh(); */
int PlotNtuplePointe(PlotEspace *espace);
int PlotNtupleRect();
void PlotNtupleColore(PlotEspace espace, int num, unsigned char r, unsigned char g, unsigned char b);
void PlotNtupleMarque(PlotEspace espace, int num, unsigned char marque);
char PlotNtupleInclu(PlotEspace espace, int num);
char PlotNtupleExclu(PlotEspace espace, int num);
char PlotNtuplePresent(PlotEspace espace, int num);
char PlotNtupleAbsent(PlotEspace espace, int num);
void PlotNtupleMasque();
int PlotChoix();
int PlotSelect(char *titre);
int PlotChoixFig1D();
int PlotChoixFig2D();
int PlotNouveau(int type);
int PlotRedraw();
int PlotRefresh();
void PlotRefreshSiUniqueBiplot();
void PlotRempliHisto(int p, int mode, int numvar, float mini, float maxi, int bins);
HistoData PlotGetData(int p);
int PlotTrace(int p, char nouveau);
int PlotLineaire();
int PlotFitNonlin(int p, char mode);
int PlotParabolique();
int PlotGaussien();
int PlotLimite();
int PlotStatsCalcule(int p, float mini, float maxi);
int PlotStatsDemande();
// void PlotResolImprimeListe(Cadre *liste);
int PlotResolImprime();
int PlotResolCalcule();
int PlotAnnule(int p);
int PlotRepertoire(char *nom);
int PlotErase();
int PlotCourbeEdite(char creation);
int PlotCourbeCree();
int PlotCourbeModifie();
char PlotCourbeSupprime();
int PlotCourbeAffiche();
char PlotCourbeAjoute();
char PlotCourbeEfface();
int PlotInit();
void PlotBuildUI();
void PlotColors(int qual, char *fig, char *fit);
int PlotExit();

#endif


