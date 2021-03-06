#include <stdarg.h>

#ifndef WIN32
#ifndef __MWERKS__
#define STD_UNIX
#endif
#endif

#ifdef STD_UNIX
#include <strings.h>  /* pour strlen qui rend size_t si intrinseque */
#else
#include <string.h>
#endif

#include <opium.h>
#include <dateheure.h>

#ifdef WIN32
#ifdef WIN32_DEBUG
#using <System.dll>
#using <mscorlib.dll>
using namespace System;
using namespace System::Diagnostics;
#endif
#endif

#ifdef WXWIDGETS
void OpiumRefreshAllWindows();
#endif

static int OpiumPromptNb,OpiumPromptPremier;
static char **OpiumPromptReponses;
static char OpiumPromptNiveau;

#define PROGRESSION
#ifdef PROGRESSION

#define PROGRES_LARGEUR 225
static Cadre OpiumProgresPlanche;

static int OpiumProgresFini,OpiumProgresMaj,OpiumProgresTaux;
static float OpiumProgresReste,OpiumProgresTotal;
static int64 OpiumProgresDebut;
static Instrum OpiumProgresNiveau;
static TypeInstrumGlissiere OpiumProgresGlissiere = { PROGRES_LARGEUR, 8, 1, 0, 3, 3 };

static int  OpiumProgresStoppe();
static char OpiumProgresContinue;
static MenuItem OpiumProgresItem[] = { { "Stopper", MNU_FONCTION OpiumProgresStoppe }, MNU_END };
static Panel OpiumProgresPanel;

#endif

/* ======================================================================== */
static void OpiumPromptBoutons(char **reponses, int nb, int premier) {
	OpiumPromptReponses = reponses;
	OpiumPromptNb = nb;
	OpiumPromptPremier = premier;
}
/* ======================================================================== */
void OpiumPromptBrut(char brut) { OpiumPromptFmtBrut = brut; }
/* ======================================================================== */
int OpiumError(char *fmt, ...) {
	char texte[256]; char *utilise;
	va_list argptr; int cnt;

	utilise = OpiumPromptFmtBrut? fmt: L_(fmt);
	va_start(argptr, fmt);
	cnt = vsprintf(texte, utilise, argptr);
	va_end(argptr);
	
	if(DEBUG_PROMPT(1)) WndPrint("!!! %s: %s\n",OpiumPromptTitre[OPIUM_WARN],texte);
	OpiumPromptNiveau = OPIUM_NOTE; // OPIUM_ERROR;
	// OpiumTitle(OpiumCdrPrompt,"ATTENTION");
	OpiumTitle(OpiumCdrPrompt,OpiumPromptTitre[OPIUM_WARN]); // OPIUM_ERROR
	OpiumCdrPrompt->adrs = (void *)texte;
	OpiumPromptBoutons(OpiumReplyStandard,1,PNL_NBREPLIES - 1);
	OpiumPrioritaire = OpiumCdrPrompt;
	OpiumExec(OpiumCdrPrompt);
	return(1);
}
/* ======================================================================== */
static void OpiumMsgDisplay(char *texte, char niveau) {
	if(niveau < 0) niveau = 0; else if(niveau >= OPIUM_MSGNB) niveau = OPIUM_MSGNB - 1;
	if(DEBUG_PROMPT(1)) WndPrint("* %s: %s\n",OpiumPromptTitre[niveau],texte);
	OpiumPromptNiveau = niveau;
	OpiumTitle(OpiumCdrPrompt,OpiumPromptTitre[niveau]);
	OpiumCdrPrompt->adrs = (void *)texte;
	OpiumPromptBoutons(OpiumReplyStandard,1,PNL_NBREPLIES - 1);
	OpiumPrioritaire = OpiumCdrPrompt;
	OpiumExec(OpiumCdrPrompt);
}
/* ======================================================================== */
int OpiumMessage(char niveau, char *fmt, ...) {
	char texte[256]; char *utilise;
	va_list argptr; int cnt;
	
	utilise = OpiumPromptFmtBrut? fmt: L_(fmt);
	va_start(argptr, fmt);
	cnt = vsprintf(texte, utilise, argptr);
	va_end(argptr);
	OpiumMsgDisplay(texte,niveau);
	return(1);
}
/* ======================================================================== */
int OpiumNote(char *fmt, ...) {
	char texte[256]; char *utilise;
	va_list argptr; int cnt;
	
	utilise = OpiumPromptFmtBrut? fmt: L_(fmt);
	va_start(argptr, fmt);
	cnt = vsprintf(texte, utilise, argptr);
	va_end(argptr);
	OpiumMsgDisplay(texte,OPIUM_NOTE);
	return(1);
}
/* ======================================================================== */
int OpiumWarn(char *fmt, ...) {
	char texte[256]; char *utilise;
	va_list argptr; int cnt;
	
	utilise = OpiumPromptFmtBrut? fmt: L_(fmt);
	va_start(argptr, fmt);
	cnt = vsprintf(texte, utilise, argptr);
	va_end(argptr);
	OpiumMsgDisplay(texte,OPIUM_WARN);
	return(1);
}
/* ======================================================================== */
int OpiumFail(char *fmt, ...) {
	char texte[256]; char *utilise;
	va_list argptr; int cnt;
	
	utilise = OpiumPromptFmtBrut? fmt: L_(fmt);
	va_start(argptr, fmt);
	cnt = vsprintf(texte, utilise, argptr);
	va_end(argptr);
	OpiumMsgDisplay(texte,OPIUM_ERROR);
	return(1);
}
#ifdef WIN32_DEBUG
/* ========================================================================== */
void OpiumWin32Log(char *fmt) {
	Debug::Listeners->Add(new TextWriterTraceListener(Console::Out));
	Debug::AutoFlush = true;
	Debug::Indent();
	Debug::WriteLine(fmt);
	Debug::Unindent();
}
#endif
/* ========================================================================== */
int OpiumCheck(char *fmt, ...) {
	char texte[256]; char *utilise; //- char **traduite;
	va_list argptr; int cnt;

	utilise = OpiumPromptFmtBrut? fmt: L_(fmt);
	va_start(argptr, fmt);
	cnt = vsprintf(texte, utilise, argptr);
	va_end(argptr);

	if(DEBUG_PROMPT(1)) WndPrint("(OpiumCheck) Texte: '%s'\n",(char *)(OpiumCdrPrompt->adrs));
	OpiumPromptNiveau = OPIUM_CHECK;
	OpiumTitle(OpiumCdrPrompt,OpiumPromptTitre[OPIUM_CHECK]);
	OpiumCdrPrompt->adrs = (void *)texte;
	OpiumPromptBoutons(OpiumReplyStandard,2,PNL_NBREPLIES - 2);
	OpiumPrioritaire = OpiumCdrPrompt;
	return(OpiumExec(OpiumCdrPrompt));
}
/* ========================================================================== */
int OpiumChoix(int nb, char *boutons[], char *fmt, ...) {
	char texte[256]; char *utilise;
	va_list argptr; int cnt;
	
	utilise = OpiumPromptFmtBrut? fmt: L_(fmt);
	va_start(argptr, fmt);
	cnt = vsprintf(texte, utilise, argptr);
	va_end(argptr);
	if(DEBUG_PROMPT(1)) WndPrint("(OpiumChoix) Texte: '%s'\n",(char *)(OpiumCdrPrompt->adrs));
	
	OpiumPromptNiveau = OPIUM_CHECK;
	OpiumTitle(OpiumCdrPrompt,OpiumPromptTitre[OPIUM_CHECK]);
	OpiumCdrPrompt->adrs = (void *)texte;
	OpiumPromptBoutons(boutons,nb,0);
	OpiumPrioritaire = OpiumCdrPrompt;
	return(OpiumExec(OpiumCdrPrompt));
}

#ifdef PROGRESSION
/* ========================================================================== */
int OpiumProgresStoppe() { OpiumProgresContinue = 0; return(0); }
#define Accord1s(var) var,(var>1)?"s":""
/* ========================================================================== */
void OpiumProgresCreate() {
	Menu m; Panel p;
	int dx,dy;

	OpiumProgresNiveau = InstrumInt(INS_COLONNE,&OpiumProgresGlissiere,&OpiumProgresTaux,0,100);
	InstrumSupport(OpiumProgresNiveau,WND_CREUX);
	OpiumProgresPanel = p = PanelCreate(2);
	PanelItemSelect(p,PanelFloat(p,L_("Reste"),&OpiumProgresReste),0);
	PanelItemSelect(p,PanelFloat(p,L_("secs sur"),&OpiumProgresTotal),0);
	PanelFormat(p,1,"%.1f"); PanelLngr(p,1,6);
	PanelFormat(p,2,"%.1f"); PanelLngr(p,2,6);
	PanelColumns(p,2); PanelMode(p,PNL_DIRECT); PanelSupport(p,WND_CREUX);
	dx = WndColToPix(1); dy = WndLigToPix(1);
	OpiumProgresPlanche = BoardCreate();
	BoardAddInstrum(OpiumProgresPlanche,OpiumProgresNiveau,dx,0,0);
	BoardAddPanel(OpiumProgresPlanche,p,dx,4*dy,0);
	BoardAddMenu(OpiumProgresPlanche,m=MenuCreate(OpiumProgresItem),(PROGRES_LARGEUR-(7*dx))/2,6*dy,0);
	OpiumSupport(m->cdr,WND_PLAQUETTE);
	// OpiumPosition(OpiumProgresPlanche,300,200);
	OpiumProgresReste = OpiumProgresTotal = 0.0;
}
/* ========================================================================== */
void OpiumProgresTitle(char *titre) { OpiumTitle(OpiumProgresPlanche,titre); }
/* ========================================================================== */
void OpiumProgresInit(int total) {
	OpiumProgresFini = total;
	OpiumProgresMaj = OpiumProgresFini / 100;
	if(OpiumProgresMaj == 0) OpiumProgresMaj = 1;
	OpiumProgresContinue = 1;
	OpiumProgresTaux = 0;
	OpiumProgresReste = OpiumProgresTotal = 0.0;
	OpiumProgresDebut = DateMicroSecs();
	OpiumFork(OpiumProgresPlanche);

#ifndef WXWIDGETS
	OpiumRefresh(OpiumProgresPlanche);
#else
	// Only tell the windows to refresh rather than actually calling graphics commands
	OpiumRefreshAllWindows();
#endif

	OpiumUserAction();
}
/* ========================================================================== */
char OpiumProgresRefresh(int actuel) {
	int64 deja_passe;
	
	if(!(actuel % OpiumProgresMaj)) {
		if(OpiumProgresFini) OpiumProgresTaux = (actuel * 100) / OpiumProgresFini;
		else OpiumProgresTaux = 100;
		if(actuel) {
			deja_passe = DateMicroSecs() - OpiumProgresDebut;
			OpiumProgresTotal = (float)deja_passe * (float)OpiumProgresFini / (float)actuel / 1000000.0;
			OpiumProgresReste = OpiumProgresTotal - ((float)deja_passe / 1000000.0);
		}
#ifndef WXWIDGETS
		OpiumRefresh(OpiumProgresNiveau->cdr);
#else
		// Only tell the windows to refresh rather than actually calling graphics commands
		OpiumRefreshAllWindows();
#endif

		PanelRefreshVars(OpiumProgresPanel);
		OpiumUserAction();
	}
	return(OpiumProgresContinue);
}
/* ========================================================================== */
void OpiumProgresClose() { OpiumClear(OpiumProgresPlanche); }
#endif
/* ========================================================================== */
int OpiumGetChar() {
  WndUserRequest e;
  int evt,key;

  /* entree utilisateur au clavier uniquement */
  if(DEBUG_PROMPT(3)) WndPrint("OpiumGetChar()\n");

  /* gestion des evenements */
  if(OpiumEntryPre) {
    key = (*OpiumEntryPre)();
    if(DEBUG_PROMPT(3)) WndPrint("EntryBefore: cle rendue=%08X\n",key);
  } else key = 0;
  if(key) {
    e.type = WND_KEY;
    e.code = 0; /* ca serait bien de coder les cles */
    e.texte[0] = (char)key; e.texte[1] = '\0';
  } else {
	e.type = -1; /* rapport au cas WND_PRESS */
  	WndEventNew(&e,WND_WAIT);
  }
  evt = e.type;
  if(DEBUG_PROMPT(1)) WndPrint("Evt '%s'\n",OpiumEventType[evt]);
  if(e.texte[0]) key = (int)(e.texte[0]); else key = (int)(e.code);
  if(OpiumEntryPost) (*OpiumEntryPost)(key);
  if(DEBUG_PROMPT(1)) WndPrint("Caractere entre: %02X\n",key);
  return(key);
}
/* ========================================================================== */
int OpiumCheckChar() {
  WndUserRequest e;
  int evt,key,rc;

  /* entree utilisateur au clavier uniquement */
  if(DEBUG_PROMPT(3)) WndPrint("OpiumGetChar()\n");

  /* gestion des evenements */
  if(OpiumEntryPre) {
    key = (*OpiumEntryPre)();
    if(DEBUG_PROMPT(3)) WndPrint("EntryBefore: cle rendue=%08X\n",key);
  } else key = 0;
  if(key) {
    e.type = WND_KEY;
    e.code = 0; /* ca serait bien de coder les cles */
    e.texte[0] = (char)key; e.texte[1] = '\0';
    rc = 1;
  } else rc = WndEventWait(WND_KEY,&e);
  if(rc) {
    evt = e.type;
    if(DEBUG_PROMPT(1)) WndPrint("Evt '%s'\n",OpiumEventType[evt]);
    if(e.texte[0]) key = (int)(e.texte[0]); else key = (int)(e.code);
    if(OpiumEntryPost) (*OpiumEntryPost)(key);
    if(DEBUG_PROMPT(1)) WndPrint("Caractere entre: %02X\n",key);
  }
  return(key);
}
/* ========================================================================== */
int OpiumSizePrompt(Cadre cdr, char from_wm) {
  char *prompt;
	size_t lt,l,max; int i,k;
#ifdef OPIUM_AJUSTE_WND
  int haut,larg;
#endif

	if(from_wm) {
#ifdef OPIUM_AJUSTE_WND
		larg = WndPixToCol(cdr->dh);
		cdr->dh = WndColToPix(larg);
		haut = WndPixToLig(cdr->dv);
		cdr->dv = WndLigToPix(haut);
#endif
		return(1);
	}
	if(DEBUG_OPIUM(1) && (cdr->type != OPIUM_PROMPT)) {
		WndPrint("+++ OpiumSizePrompt sur le %s @%08X\n",cdr->nom,(hexa)cdr->adrs);
		return(0);
	}
	prompt = (char *)cdr->adrs;
	lt = strlen(prompt); if(lt < 4) lt = 4; /* pour pouvoir placer le bouton OK */
	if(DEBUG_PROMPT(1)) WndPrint("(OpiumSizePrompt) Texte: '%s' (%ld cars)\n",prompt,lt);
	max = 0; i = OpiumPromptPremier;
	for(k=0; k<OpiumPromptNb; k++) {
 		l = strlen(OpiumPromptReponses[i]);
		if(l < 4) l = 4;
		if(max < l) max = l;
		i++;
	}
	max *= OpiumPromptNb;
	if(max < lt) max = lt;
	
	cdr->larg = WndColToPix((int)max);
	cdr->haut = OpiumUseMouse? WndLigToPix(2): WndLigToPix(1);
	cdr->dh = cdr->larg; cdr->dv = cdr->haut;
	if(DEBUG_PROMPT(1)) WndPrint("(OpiumSizePrompt) Fenetre %d x %d\n",cdr->larg,cdr->haut);
	return(1);
}
/* ========================================================================== */
int OpiumRefreshPrompt(Cadre cdr) {
	char *prompt; char std;
	WndFrame f; WndContextPtr gc;
	
	if(DEBUG_OPIUM(1) && (cdr->type != OPIUM_PROMPT)) {
		WndPrint("+++ OpiumRefreshPrompt sur le %s @%08X\n",cdr->nom,(hexa)cdr->adrs);
		return(0);
	}
	prompt = (char *)cdr->adrs;
	if(WndModeNone) {
		if(OpiumPromptNb == 1) printf("* %s: %s\n",OpiumPromptTitre[OpiumPromptNiveau],prompt);
		else printf("> %s: OK\n",prompt);
		return(1);
	}
	if(!OpiumCoordFenetre(cdr,&f)) return(0);
	gc = WND_TEXT; std = 1;
	if(OpiumPromptNiveau > OPIUM_CHECK) {
		gc = WndContextCreate(f);
		if(gc) {
			std = 0;
			cas_ou(OpiumPromptNiveau) {
			  vaut OPIUM_NOTE:  WndContextSetColors(f,gc,WndColorNote[WndQual],WndColorBgnd[WndQual]); break;
			  vaut OPIUM_WARN:  WndContextSetColors(f,gc,WndColorWarn[WndQual],WndColorBgnd[WndQual]); break;
			  vaut OPIUM_ERROR: WndContextSetColors(f,gc,WndColorErrr[WndQual],WndColorBgnd[WndQual]); break;
			  au_pire:          WndContextSetColors(f,gc,WndColorBgnd[WndQual],WndColorText[WndQual]); break;
			}
			WndFillBgnd(f,gc,0,0,f->xm+WND_ASCINT_WIDZ,f->ym+WndLigToPix(1)+WND_ASCINT_WIDZ);
			cas_ou(OpiumPromptNiveau) {
			  vaut OPIUM_NOTE:  WndContextSetColors(f,gc,0,WndColorBgnd[WndQual]); break;
			  vaut OPIUM_WARN:  WndContextSetColors(f,gc,0,WndColorBgnd[WndQual]); break;
			  vaut OPIUM_ERROR: WndContextSetColors(f,gc,0,WndColorBgnd[WndQual]); break;
			  au_pire:          WndContextSetColors(f,gc,0,WndColorText[WndQual]); break;
			}
		} else gc = WND_TEXT;
	}
	WndWrite(f,gc,0,0,prompt);
	if(!OpiumUseMouse) return(1);
	// OpiumBoutonsScrolles(cdr,f,OpiumPromptPremier,OpiumPromptNb,OpiumPromptReponses);
	// gc pas standard..: OpiumBoutonsTjrs(cdr,f,OpiumPromptPremier,OpiumPromptNb,OpiumPromptReponses);
	{
		int x,y,h,k,i;
		x = 0;
		y = WndPixToLig(cdr->dv) - 1;
		h = WndPixToCol(cdr->dh) / OpiumPromptNb;
		i = OpiumPromptPremier;
		for(k=0; k<OpiumPromptNb; k++) {
			WndButton(f,gc,y,x,h,OpiumPromptReponses[i]);
			i++; x += h;
		}
	}
	unless(std) WndContextFree(cdr->f,gc);
	return(1);
}
/* ========================================================================== */
int OpiumRunPrompt(Cadre cdr, WndUserRequest *e) {
	char *prompt;
	int lig;
	int code_rendu;
	char return_done;
	size_t l,m; int h;
	
	if(DEBUG_OPIUM(1) && (cdr->type != OPIUM_PROMPT)) {
		WndPrint("+++ OpiumRunPrompt sur le %s @%08X\n",cdr->nom,(hexa)cdr->adrs);
		return(0);
	}
	prompt = (char *)cdr->adrs;
	lig = WndPixToLig(e->y);
	if(DEBUG_PROMPT(1)) WndPrint("Prompt @%08X, ligne %d, bouton %d\n",
		(hexa)prompt,lig,e->x/(cdr->larg/2));
	code_rendu = -1;
	if(e->type == WND_KEY) {
		l = strlen(e->texte);
		if(DEBUG_PROMPT(2)) {
			WndPrint("%04lX ( ",e->code);
			for(m=0; m<l; m++) WndPrint("%02X ",e->texte[m]); WndPrint(")\n");
		}
		if(l <= 0) {
			if(e->code == XK_KP_F4) code_rendu = 0;
		} else {
			return_done = (e->texte[l - 1] == 0x0D);
			if(return_done) {
				if(OpiumPromptNb == 1) code_rendu = 1;
				else code_rendu = OpiumPromptNb - 1;
			}
		}
	} else if(e->type == WND_RELEASE) switch(e->code) {
	  case WND_MSELEFT:
#ifdef WXWIDGETS
		// for wx widgets, the buttons are placed on line 2
		if(lig == 2) {
#else
		if(lig == 1) {
#endif
			if(OpiumPromptNb == 1) code_rendu = 1;
			else { h = cdr->larg / OpiumPromptNb; code_rendu = e->x / h; }
		};
		break;
	  default:
		/* autres entrees souris dans un prompt: pas d'action */
		break;
	}
		
	if(DEBUG_PROMPT(1)) WndPrint("Code rendu: %d\n",code_rendu);
	return(code_rendu);
}

