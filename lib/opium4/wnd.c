#include <environnement.h>

#define OPIUM_VSN "2.5"

// #define VERIFIE_WM

#include <stdio.h>
#include <errno.h>
#include <ctype.h>  /* pour toupper */
#include <stdlib.h> /* pour malloc, free et system */
#include <stdarg.h> /* pour va_start */
#include <fcntl.h>  /* pour open et al. */
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

extern char *__progname;
typedef unsigned long long UInt64,uint64;

#ifndef __MWERKS__
	#define STD_UNIX
#endif

#ifdef STD_UNIX
	#include <strings.h>  /* pour strlen qui rend size_t si intrinseque */
	#include <unistd.h>
#else
	#include <string.h>
#endif

#include <claps.h>
#include <decode.h>
#include <timeruser.h>
#include <opium.h>
#define WND_C
#include <wnd.h>

/* ------------------------------------------------------------------------ */
/* -------------------------- IMPLEMENTATION OpenGL ----------------------- */
/* ------------------------------------------------------------------------ */
#ifdef WXWIDGETS

	#include <opium_wx_interface.h>

	static void *WndFontPtr = NULL;
	// GLUT_BITMAP_8_BY_13; // GLUT_BITMAP_9_BY_15 // == &glutBitmap8By13 ou &glutBitmap9By15
	static char WndFontName[MAXFILENAME] = "Bitmap8By13";

#endif

/* ------------------------------------------------------------------------ */
/* -------------------------- IMPLEMENTATION OpenGL ----------------------- */
/* ------------------------------------------------------------------------ */
#ifdef OPENGL
	#ifdef VERIFIE_WM
		#pragma message("Application OPENGL")
	#endif
	static GLvoid *WndFontPtr = &glutBitmap8By13;
	// GLUT_BITMAP_8_BY_13; // GLUT_BITMAP_9_BY_15 // == &glutBitmap8By13 ou &glutBitmap9By15
	static char WndFontName[MAXFILENAME] = "Bitmap8By13";
	static WndCursor WndCursorDefaut;
	static useconds_t WndMgrTimeOut;
	static char WndShifted[127];

	typedef struct WndEventEltPile {
		WndEvent evt;
		struct WndEventEltPile *suivant;
	} StructWndEventEltPile;
	static StructWndEventEltPile *WndEventPileHaut,*WndEventPileBas;

	static void WndColbacKey(WndIdent w, int key, int scancode, int action, int mods);
//	static void WndColbacCurseur(WndIdent w, double x, double y);
	static void WndColbacSouris(WndIdent w, int button, int action, int mods);
	static void WndColbacEntre(WndIdent w, int entre);
	static void WndColbacPos(WndIdent w, int posx, int posy);
	static void WndColbacSize(WndIdent w, int sizx, int sizy);
	static void WndColbacRefresh(WndIdent w);
	static void WndColbacClose(WndIdent w);

/* fin implementation OpenGL */
#endif /* OPENGL */

/* ------------------------------------------------------------------------ */
/* -------------------------- IMPLEMENTATION X11 -------------------------- */
/* ------------------------------------------------------------------------ */
#ifdef X11
	#ifdef VERIFIE_WM
		#pragma message("Application X11")
	#endif
/* nom complets (avec choix entre 'medium' ou 'bold'):
 QD  Finder: static char WndFontName[MAXFILENAME] = "Monaco ou Courier";
 X11 darwin: static char WndFontName[MAXFILENAME] = "*courier*%s-r-*-%d-*";
 X11 ubuntu: static char WndFontName[MAXFILENAME] = "*courier*%d*%s-r-*";
 */
static char WndFontName[MAXFILENAME] = "*courier*medium-r-*-%d-*"; // ubuntu: "*courier*%d*medium-r-*" // "*courier*-r-*"
static unsigned long WndGCMask;
static long WndEventMask;

static short WndEssais;
static char *WndOpName[] = { "CreateWindow", "ChangeWindowAttributes", "GetWindowAttributes",
	"DestroyWindow", "DestroySubwindows", "ChangeSaveSet", "ReparentWindow",
	"MapWindow", "MapSubwindows", "UnmapWindow", "UnmapSubwindows",
	"ConfigureWindow ", "CirculateWindow", "GetGeometry", "QueryTree", "InternAtom",
	"GetAtomName", "ChangeProperty", "DeleteProperty", "GetProperty",
	"ListProperties", "SetSelectionOwner", "GetSelectionOwner", "ConvertSelection",
	"SendEvent", "GrabPointer", "UngrabPointer", "GrabButton", "UngrabButton",
	"ChangeActivePointerGrab", "GrabKeyboard", "UngrabKeyboard", "GrabKey",
	"UngrabKey", "AllowEvents", "GrabServer", "UngrabServer", "QueryPointer",
	"GetMotionEvents", "TranslateCoords", "WarpPointer", "SetInputFocus",
	"GetInputFocus", "QueryKeymap", "OpenFont", "CloseFont", "QueryFont",
	"QueryTextExtents", "ListFonts", "ListFontsWithInfo", "SetFontPath",
	"GetFontPath", "CreatePixmap", "FreePixmap", "CreateGC", "ChangeGC",
	"CopyGC", "SetDashes", "SetClipRectangles", "FreeGC", "ClearArea",
	"CopyArea", "CopyPlane", "PolyPoint", "PolyLine", "PolySegment",
	"PolyRectangle", "PolyArc", "FillPoly", "PolyFillRectangle", "PolyFillArc",
	"PutImage", "GetImage", "PolyText8", "PolyText16", "ImageText8", "ImageText16",
	"CreateColormap", "FreeColormap", "CopyColormapAndFree", "InstallColormap",
	"UninstallColormap", "ListInstalledColormaps", "AllocColor", "AllocNamedColor",
	"AllocColorCells", "AllocColorPlanes", "FreeColors", "StoreColors",
	"StoreNamedColor" , "QueryColors", "LookupColor", "CreateCursor",
	"CreateGlyphCursor", "FreeCursor", "RecolorCursor", "QueryBestSize",
	"QueryExtension", "ListExtensions", "ChangeKeyboardMapping", "GetKeyboardMapping",
	"ChangeKeyboardControl", "GetKeyboardControl", "Bell", "ChangePointerControl",
	"GetPointerControl", "SetScreenSaver", "GetScreenSaver", "ChangeHosts",
	"ListHosts", "SetAccessControl", "SetCloseDownMode", "KillClient",
	"RotateProperties", "ForceScreenSaver", "SetPointerMapping",
	"GetPointerMapping", "SetModifierMapping", "GetModifierMapping",
	"120", "121", "122", "123", "124", "125", "126", "NoOperation", "\0"
};
/* fin implementation X11 */
#endif /* X11 */

/* ------------------------------------------------------------------------ */
/* -------------------------- IMPLEMENTATION Windows ---------------------- */
/* ------------------------------------------------------------------------ */
#ifdef WIN32
	#ifdef VERIFIE_WM
		#pragma message("Application WIN32")
	#endif
	//#define MS_PRESSED_DISTANCE 7
	static CConsoleWin32 WndConsole;
	static bool WndConsoleOpen = false;

	LRESULT WINAPI WndProc(WndIdent hWnd, UINT mesage, WPARAM wParam, LPARAM lParam);

/* fin implementation Windows */
#endif /* WIN32 */

/* ------------------------------------------------------------------------ */
/* -------------------------- IMPLEMENTATION MAC -------------------------- */
/* ------------------------------------------------------------------------ */
#ifdef QUICKDRAW
	#ifdef VERIFIE_WM
		#pragma message("Application QUICKDRAW")
	#endif
	// #if (MAC_OS_X_VERSION_MAJOR != 1040)
	#ifndef OSX_10_4
		#include <AudioToolbox/AudioServices.h>
	#endif

	#ifdef CODE_WARRIOR_VSN
		#include <Windows.h>   /* Ajout pour G4MiG */
		#include <ToolUtils.h>
		#include <utsname.h>   /* existe sous UNIX dans <sys/utsname> */
		#include <Processes.h> /* pour LaunchApplication */
		#define AVEC_SIOUX
	#endif
	#ifdef AVEC_SIOUX
		#ifdef VERIFIE_WM
			#pragma message("Utilisation de SIOUX")
		#endif
		//#include <sioux.h>
		#include <SIOUX.h>
		#include <Processes.h>
		static char WndSiouxDone = 0;
	#else
		#ifdef CODE_WARRIOR_VSN
			#pragma message("Pas de SIOUX")
		#endif
	#endif
	#ifdef CARBON
		void GetQDGlobalsScreenBits(WndScreen *d);
		void GetPortBounds(CGrafPtr p, Rect *r);
		void InitCursor();
		void RGBForeColor(WndColor *c);
		void RGBBackColor(WndColor *c);
		void GetFNum(unsigned char *nom_fonte, WndFontId *id);
		void TextFont(short fonte);
		void TextSize(pixval size);
		short TextWidth(char *texte, int i, int nb);
		void GetFontInfo(WndFontInfo *fontinfo);
		void DrawText(char *texte, int k, pixval l);
		void LocalToGlobal(Point *origine);
		void PaintRect(Rect *r);
		void SetCPixel(int x, int y, WndColor *c);
		void Line(pixval x, pixval y);
		void MoveTo(pixval x, pixval y);
		void LineTo(pixval x, pixval y);
		void FrameRect(Rect *r);
		void FrameArc(Rect *r, int a0, int da);
		void FrameRoundRect(Rect *r, int rx, int ry);
		void EraseRect(Rect *r);
		void PaintArc(Rect *r, int a0, int da);
		//#define AQUA
		// WindowRef GetWindowFromPort(CGrafPtr port) defini dans HIToolbox.framework > MacWindows.h
		#define CARBON_DEPRECIE
		#ifdef CARBON_DEPRECIE
			#define WNDPORT(w) GetWindowPort(w)
			#define WND_UTILISE_FONTE(w,fonte) TextFont(fonte)  /* ou bien SetPortTextFont(w,fonte); */
			void SetPort(CGrafPtr w);
		#else
			#define CGrafPtr WindowRef
			#define WNDPORT(w) w
			#define SetPort(w) SetPortWindowPort(w);
			// pas mieux: #define WND_UTILISE_FONTE(w,fonte) SetPortTextFont(w,fonte) 
			#define WND_UTILISE_FONTE(w,fonte) TextFont(fonte)
		#endif
		// #define WND_UTILISE_FONTE(w,fonte) SetPortTextFont(w,fonte)
		// static struct OpaqueWindowPtr *wptr; // juste pour chercher le .h
	#else
		#define WNDPORT(w) w
		#define WND_UTILISE_FONTE(w,fonte) { w->txFont = fonte; PortChanged(w); }
	#endif
	#ifdef QUICKDRAW_ANCIEN
	static char *WndMgrBouton[] = {
		"InDesk",
		"inMenuBar",
		"inSysWindow",
		"inContent",
		"inDrag",
		"inGrow",
		"inGoAway",
		"inZoomIn",
		"inZoomOut"
	};
	#endif

	#ifdef MENU_BARRE_MAC
		#define WND_BASE_ID 1000
		Handle WndMenuStandard,WndMenuAppli;
		MenuHandle WndMenuLast; short WndMenuDim;
		int WndMenuId;
	#endif
	static char WndFontName[MAXFILENAME] = "Courier";
	static char WndDebugBarre = 0;
	static short WndEventMask;
	static UInt32 WndMgrTimeOut;
	/* Timer WndScheduler; */

/* fin implementation MAC */
#endif /* QUICKDRAW */

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

#ifdef VERIFIE_WM
	#ifdef macintosh   /* juste pour verifier */
		#ifdef CARBON
			#pragma message("Application macintosh au standard Carbon")
		#else
			#pragma message("Application macintosh sans Carbon")
		#endif
		#if CODE_WARRIOR_VSN == 5
			#pragma message("> Preparee pour CodeWarrior version 5")
		#else
			#if CODE_WARRIOR_VSN == 7
				#pragma message("> Preparee pour CodeWarrior version 7")
			#else
				#ifdef PROJECT_BUILDER
					#pragma message("> preparee pour Project Builder")
				#else
					#ifdef XCODE
						#pragma message("> preparee pour Xcode")
					#else
						#pragma message("! NON PREPAREE pour le compilateur en cours")
					#endif
				#endif
			#endif
		#endif
	/* Derniers resultats: Accessors calls ARE functions et Toolbox structs ARE opaque
		#ifdef CARBON
			#if ACCESSOR_CALLS_ARE_FUNCTIONS
				#pragma message("Accessors calls ARE functions")
			#else
				#pragma message("Accessors calls ARE NOT functions")
			#endif
			#if OPAQUE_TOOLBOX_STRUCTS
				#pragma message("Toolbox structs ARE opaque")
			#else
				#pragma message("Toolbox structs ARE NOT opaque")
			#endif
		#endif
	*/
	/* Derniers resultats: MAC_OS_X_BUILD = 0 et TARGET_API_MAC_CARBON = 1"
		#if MAC_OS_X_BUILD
			#pragma message("MAC_OS_X_BUILD = 1")
		#else
			#pragma message("MAC_OS_X_BUILD = 0")
		#endif
		#if TARGET_API_MAC_CARBON
			#pragma message("TARGET_API_MAC_CARBON = 1")
		#else
			#pragma message("TARGET_API_MAC_CARBON = 0")
		#endif
	*/
	#endif /* macintosh */
#endif /* VERIFIE_WM */

/* #define DEBUG */
/* #define DEBUG0 */

#ifdef DEBUG3
	#define DEBUG2
#endif
#ifdef DEBUG2
	#define DEBUG1
#endif
#ifdef DEBUG1
	#define DEBUG0
#endif

#define WND_APPLI 32
#define WND_COLOR_NB 2014

WndIdent WndRoot;
WndIdent WndIdRafraichie;
static WndFrame WndPremiere;
static int WndFontSize = 12;
static char WndQualDefaut = WND_Q_PAPIER;
static char WndSourisPressee;
static FILE *WndPS;
static int WndPSnum,WndPSx,WndPSy; char WndPSstroked;
static unsigned short WndPSred,WndPSgreen,WndPSblue;
#ifndef X11
static struct {
	WndColor c;
	char active;
} WndColorStock[WND_COLOR_NB];
#endif

static WndUserRequest *WndRequestNext;
static WndUserRequest *WndRequestLast;
static int WndWaitMin,WndWaitMax; /* attentes evts en millisec. */
static int WndNbWaits;            /* attentes evts en nombre de boucles */
#ifdef TIMER_UNIQUE
static Timer WndHorloge;
#endif
static int WndNum,WndOpened,WndActives,WndPassives;
static char WndPrevRoutine[40]; WndFrame WndPrevArg;
static WndIdent WndLastUpdated;
static void (*WndAProposFctn)();
static char WndAProposAppli[WND_APPLI];

// static int WndEventNum = 0;

static char *WndChoixEcriture[] = { "Abandonner l'ecriture", "Supprimer l'ancien", "\0" };

#define WND_MAXDASH 7 /* impose par la taille du champ associe dans graph->data->parm */
char *WndDashList[WND_MAXDASH] = {
#ifdef OS9
	"\x04\x04",
	"\x08\x04",
	"\x02\x02\x08\x02",
	"\x04\x08",
	"\x01\x02\x03",
	"\x04\x02\x02\x04",
	"\x04\x02"
#else
	"\4\4",
	"\10\4",
	"\2\2\10\2",
	"\4\10",
	"\1\2\3",
	"\4\2\2\4",
	"\4\2"
#endif
};

static void WndPScolorie(WndColor *c);
static char WndWriteOnlyDel(WndFrame f);
#ifdef DETAILLE
static void WndServerSingleColor(WndServer *s, int qual, int coul, WndColor *fond, WndColor *texte);
#endif
static void WndServerColor(WndServer *s, int coul, WndColor *fond[], WndColor *texte[]);
static void WndContextValInit(WndServer *s, WndContextVal *gcval, WndColor *fond, WndColor *texte);
static void WndContextValSetColors(WndContextVal *gcval, WndColor *fond, WndColor *text);
// static void WndServerColor(WndServer *s, int num, WndColor **fond, WndColor **text);

#ifdef WIN32
int OpiumRefresh(WndIdent wnd);
/* ========================================================================== */
LRESULT WINAPI WndProc(WndIdent hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if(hWnd == WndRoot)
		return DefWindowProc(hWnd, message, wParam, lParam);

	switch(message) {
	  case WM_CLOSE:
		PostMessage(hWnd, WND_ERASE, 0, 0);
		return 0;
	  case WM_ERASEBKGND:
		if(hWnd != GetFocus()) {
			//PostMessage(hWnd, WND_REDRAW, 0, 0);
			OpiumRefresh(hWnd);
		}
		return 0;
	  case WM_PAINT:
		return 0;
	  case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	  default:
		return(DefWindowProc(hWnd, message, wParam, lParam));
	}
}
#endif /* WIN32 */

/* ========================================================================== */
static void WndItemMajuscules(unsigned char *texte) {
	unsigned char *c,d;
	c = texte; while(*c) { d = toupper(*c); *c++ = d; };
}
/* ========================================================================== */
static void WndDrawLine(WndFrame f, WndContextPtr gc, int x0, int y0, int x1, int y1) {
	WndIdent w;
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
	if(gc == 0) gc = WND_TEXT;
	if(WndPS) {
		WndColor c;
	#ifdef X11
		WndScreen d; WndContextVal gcval;
		d = (f->s)->d;
		XGetGCValues(d,gc,GCForeground,&gcval);
		c.pixel = gcval.foreground;
		XQueryColor(d,DefaultColormap(d,DefaultScreen(d)),&c);
	#endif
	#ifdef OPENGL
		memcpy(&c,gc->foreground,sizeof(WndColor));
	#endif
	#ifdef QUICKDRAW
		memcpy(&c,gc->foreground,sizeof(WndColor));
	#endif
		WndPScolorie(&c);
		fprintf(WndPS,"%d %d moveto %d %d lineto\n",x0,-y0,x1,-y1);
		WndPSstroked = 0;
		return;
	}

	w = f->w;
#ifdef OPENGL
	int sizx,sizy; double xd,yd,xf,yf; char doit_terminer;

	glfwGetWindowSize(w,&sizx,&sizy);
	xd = WndXToDouble(x0,sizx);
	yd = WndYToDouble(y0,sizy);
	xf = WndXToDouble(x1,sizx);
	yf = WndYToDouble(y1,sizy);
	doit_terminer = WndRefreshBegin(f);
	if(gc) {
		if(gc->foreground) glColor3us((gc->foreground)->red,(gc->foreground)->green,(gc->foreground)->blue);
		glLineWidth(gc->line_width);
	}
	glBegin(GL_LINES);
	glVertex2d(xd,yd); glVertex2d(xf,yf);
	glEnd(); glFlush();
	if(doit_terminer) WndRefreshEnd(f);
#endif

#ifdef WIN32
	PAINTSTRUCT paintst; HDC hDC; HPEN hPen, hPenOld;

	hDC = BeginPaint(w, &paintst);
	hPenOld = (HPEN)SelectObject(hDC, hPen = CreatePen(gc->line_style, gc->line_width, (gc->foreground)));
	MoveToEx(hDC, x0, y0, (LPPOINT) NULL);
	LineTo(hDC, x1, y1);
	SelectObject(hDC, hPenOld);
	EndPaint(w, &paintst);
	InvalidateRect(w, &(paintst.rcPaint), false);
	DeleteObject(hPen);
#endif

#ifdef X11
	XDrawLine((f->s)->d,f->w,gc,x0,y0,x1,y1);
#endif

#ifdef QUICKDRAW
	SetPort(WNDPORT(w));
	if(gc->foreground) RGBForeColor(gc->foreground);
	MoveTo((pixval)x0,(pixval)y0); LineTo((pixval)x1,(pixval)y1);
#endif
}
/* ========================================================================== */
static void WndDrawString(WndFrame f, WndContextPtr gc, int x, int y, char *texte, size_t l) {
	WndIdent w; WndColor c;
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif

	if(!texte[0]) return;
	if(gc == 0) gc = WND_TEXT;
	if(WndPS) {
	#ifdef X11
		WndScreen d; WndContextVal gcval;
		d = (f->s)->d;
		XGetGCValues(d,gc,GCForeground,&gcval);
		c.pixel = gcval.foreground;
		XQueryColor(d,DefaultColormap(d,DefaultScreen(d)),&c);
	#endif
	#ifdef OPENGL
		memcpy(&c,gc->foreground,sizeof(WndColor));
	#endif
	#ifdef QUICKDRAW
		memcpy(&c,gc->foreground,sizeof(WndColor));
	#endif
		WndPScolorie(&c);
		fprintf(WndPS,"%d %d moveto (%s) show\n",x,-y,texte);
		WndPSstroked = 0;
		return;
	}

	w = f->w;
#ifdef WIN32
	PAINTSTRUCT paintst; HDC hDC; HFONT hFontOld;

	hDC = BeginPaint(w, &paintst);
	hFontOld = (HFONT) SelectObject(hDC, gc->font);
	SetTextColor(hDC, (gc->foreground));
	if(gc->background) SetBkColor(hDC, (gc->background));
	else SetBkMode(hDC, TRANSPARENT);
	//SetBkColor(hDC, *WndColorBgnd[f->qualite]);
	TextOut(hDC, x, y - WndCurSvr->lig, texte, l);
	SelectObject(hDC, hFontOld);
	EndPaint(w, &paintst);
	InvalidateRect(w, &(paintst.rcPaint), false);
#endif

#ifdef OPENGL
	int sizx,sizy; const char *t; double xd,yd,xf,y0,y1; char doit_terminer;

	// PRINT_GC(gc);
	glfwGetWindowSize(w,&sizx,&sizy);
	xd = WndXToDouble(x,sizx);
	yd = WndYToDouble(y,sizy);
	xf = WndXToDouble(x + (l * (f->s)->col),sizx);
	y0 = WndYToDouble(y - (f->s)->lig + 4,sizy); // 3?
	y1 = WndYToDouble(y + 2,sizy);
	doit_terminer = WndRefreshBegin(f);
	if(gc) {
		if(gc->background) {
			glColor3us((gc->background)->red,(gc->background)->green,(gc->background)->blue);
			glRectd(xd,y0,xf,y1);
		}
		if(gc->foreground) glColor3us((gc->foreground)->red,(gc->foreground)->green,(gc->foreground)->blue);
	} else glColor3us(0xFFFF,0xFFFF,0xFFFF);
	glRasterPos2d(xd,yd); //- glScalef(2.0,2.0,2.0);
	printf("OPENGL:  %d %d %s\n", x, y, texte);
	t = texte; while(*t) glutBitmapCharacter(WndFontPtr,*t++); // glutBitmapCharacter(WndFontPtr,*t++);
	if(doit_terminer) WndRefreshEnd(f);
#endif

#ifdef X11
	// XFillRectangle(d,w,gc,x,y,WndColToPix(l),WndLigToPix(1)); mauvais
	XDrawString((f->s)->d,w,gc,x,y,texte,l);
#endif

#ifdef QUICKDRAW
	SetPort(WNDPORT(w));
	// printf("(%s:%d) gc@%08X\n",__func__,__LINE__,(hexa)gc);
	WND_UTILISE_FONTE(w,gc->font);
	if(gc->background) {
	#ifdef FOND_MODIFIE
		GetBackColor(&c);
		if(gc->background != c) {
			RGBBackColor(gc->background);
			r.left = x; r.bottom = y + WndCurSvr->decal; /* + s->decal */
			r.right = r.left + WndColToPix(l); r.top = r.bottom - WndLigToPix(1);
			EraseRect(&r);
		}
	#else
		r.left = (pixval)x; r.bottom = (pixval)(y + WndCurSvr->decal); /* + s->decal */
		r.right = r.left + (pixval)WndColToPix(l); r.top = r.bottom - (pixval)WndLigToPix(1);
		RGBForeColor(gc->background);
		PaintRect(&r);
	#endif
	}
	//+ printf("(%s(%s)) gc->foreground=%08X\n",__func__,texte,(unsigned int)(gc->foreground));
	if(((unsigned long)gc->foreground > 0x10000) && (gc->foreground != (WndColor *)ERREUR)) RGBForeColor(gc->foreground);
	else printf("(%s(%s[%ld],%d,%d)) gc->foreground=%08X\n",__func__,texte,l,x,y,(unsigned int)(gc->foreground));
		printf("QUICKDRAW:  %d %d %s\n", x, y, texte);

	MoveTo((pixval)x,(pixval)y); DrawText(texte,0,(pixval)l);
#ifdef FOND_MODIFIE
	if(gc->background) { if(gc->background != c) RGBBackColor(c); }
#endif
#endif /* QUICKDRAW */
}

#ifdef WIN32
/* ======================================================================== */
int WndPrint(const char *szfmt, ...) {
	char s[300];
	va_list argptr;
	int cnt;

	if(!WndConsoleOpen) {
		WndConsole.StartDebugConsole(80,DEBUG_CONSOLE_SIZE, "stdout");
		WndConsoleOpen = true;
	}
	va_start(argptr, szfmt);
	cnt = vsprintf(s, szfmt, argptr);
	va_end(argptr);
	WndConsole.DebugPrintf(s);
	return(1);
}
#endif
/* ======================================================================== */
void WndStep(char *fmt, ...) {
	char texte[256];
	va_list argptr; int cnt;
	char reponse[256],*c;

	if(fmt) {
		va_start(argptr, fmt);
		cnt = vsprintf(texte, fmt, argptr);
		va_end(argptr);
		WndPrint("%s. ",texte);
	}
	WndPrint("<Return> pour continuer: "); fflush(stdout);
	c = reponse;
	do {
		*c = (char)getchar();
		if((*c == 0x0a) || (*c == EOF)) {
			*c = '\0'; return; // longueur: c - reponse
		}
		c++;
	} while(1);
}
/* ========================================================================== */
/* ========================== Definition du serveur ========================= */
/* ========================================================================== */
void WndLog(int x, int y, int ligs, int cols, char *nom) {
	WndLogX = x; WndLogY = y; WndLogLines = ligs; WndLogCols = cols;
#ifdef X11
	strncpy(WndLogName,nom,WND_LOGLNGR-1);
#endif
#ifdef QUICKDRAW
	strncpy(WndLogName+1,nom,WND_LOGLNGR-2);
	WndLogName[0] = (char)strlen(nom);
#endif
}
/* ========================================================================== */
void WndSetFontName(char *chaine) { strcpy(WndFontName,chaine); }
/* ========================================================================== */
void WndSetFontSize(int taille) { printf("FONT SIZE:    %d\n", taille); WndFontSize = taille; }
/* ========================================================================== */
void WndSetBgndBlack(char noir) { WndQualDefaut = noir? WND_Q_ECRAN: WND_Q_PAPIER; }
/* ========================================================================== */
char WndSetQualite(char qual) { char avant = WndQual; WndQual = qual; return(avant); }
/* ==========================================================================
void WndSetColors(char *bgnd, char *text, char *hilite, char *mods) {
	WndPrint("(WndSetColors) Cette routine doit etre supprimee!\n");
}
   ========================================================================== */
Bool WndInit(char *display) {
/* Initialise le serveur par defaut d'apres le serveur <display>
     et le defini comme serveur courant */
	WndTopFrame = WND_AT_END;
	WndLastWriteOnly = WND_AT_END;
	WndRequestNext = WND_NOREQUEST;
	WndRequestLast = WND_NOREQUEST;
	memset(&WndEventPrecedent,0,sizeof(WndUserRequest));
	ArgKeyFromList(WndSupportCles,WndSupportNom);
	WndModeVt100 = 0;
	WndModeNone = 0;
	WndQueryExit = 0;
	WndOpened = 0;
	WndActives = 0;
	WndPassives = 0;
	WndSourisPressee = 0;
	WndNum = 0;
	WndIdRafraichie = 0;
	WndColorNb = 0;
#ifdef TIMER_UNIQUE
	WndHorloge = 0;
#endif
	WndPremiere = 0;
	WndLastUpdated = 0;
	WndAProposFctn = 0;
	WndAProposAppli[0] = '\0';
	WndUserKeyNb = 0;
	WndPS = 0;
#ifdef MENU_BARRE
	WndMenuLast = 0;
	WndMenuId = 0;
	WndMenuAppli = 0;
	WndMenuAffiche = 0;
#endif
#ifdef OPENGL
	WndWaitMin =  15;  /* attente 2eme evt, min  15 ms */
	WndWaitMax = 150;  /* attente 2eme evt, max 150 ms */
	WndMgrTimeOut = 10000;  /* attente WM en microsecondes */
#endif

#ifdef WXWIDGETS
	WndWaitMin =  15;  /* attente 2eme evt, min  15 ms */
	WndWaitMax = 150;  /* attente 2eme evt, max 150 ms */
#endif

	WndNbWaits = WndWaitMax / WndWaitMin;

	return(WndOpen(&WndDefSvr,display));
}
#ifdef OPENGL
/* ========================================================================== */
static void WndErrorColbac(int error, const char *description) {
//	fputs(description, stderr);
	printf("!!! Erreur GLFW-%03d: %s\n",error,description);
	ImprimePileAppels;
}
#endif
/* ========================================================================== */
void WndJournalTitle(char *texte) {
#ifdef AVEC_SIOUX
	unsigned char titre[256];

	strncpy(titre+1,texte,255);
	titre[0] = (unsigned char)strlen(texte);
	SIOUXSetTitle(titre);
#endif
}
/* ========================================================================== */
Bool WndOpen(WndServer *s, char *display) {
/* Initialise la structure <s> d'apres le serveur <display> */
	char qual;
#ifdef MODE_VT100
	char *term,*termcap,*getenv();
	char descr[1024];
#endif

	/* retourne 0 si erreur */
	if(!s) return(NUL);
	// printf("(WndOpen) Serveur initialise @%08X\n",(hexa)s);

	WndCurSvr = s;
	if(display == WND_NONE) {
		WndModeNone = 1;
		s->haut = 24;
		s->larg = 80;
		return(BON);
	} else if(display == WND_VT100) {
		WndModeVt100 = 1;
#ifdef MODE_VT100
		term = getenv("TERM"); if(!term) term = "vt100";
		termcap = getenv("TERMCAP"); 
#ifdef OS9
		if(!termcap) termcap = "/h0/sys/termcap";
#else
		if(!termcap) termcap = "/usr/share/lib/termcap";
#endif
		if(tgetent(descr,term) == 1) {
			s->haut = tgetnum("li");
			s->larg = tgetnum("co");
		}
		if(s->haut < 1) s->haut = 24;
		if(s->larg < 1) s->larg = 80;
#endif
		s->haut = 24;
		s->larg = 80;
		return(BON);
	}
	WndCodeHtml = (display == WND_HTML);
	if(display && !WndCodeHtml) strncpy(WndDisplayInUse,display,MAXFILENAME);
	else WndDisplayInUse[0] = '\0';

#ifdef OPENGL
	const GLFWvidmode *vdo; int k;
// #define INIT_GLUT
#ifdef INIT_GLUT
	int argc; char **argv;
	argv = (char **)malloc(sizeof(char *));
	argc = 1; argv[0] = __progname; glutInit(&argc, argv);
#endif

	glfwSetErrorCallback(WndErrorColbac);
	if(glfwInit() == GL_FALSE) return(NUL);
	// choix entre GLFW_ARROW_CURSOR, GLFW_IBEAM_CURSOR, GLFW_CROSSHAIR_CURSOR, GLFW_HAND_CURSOR, GLFW_HRESIZE_CURSOR et GLFW_VRESIZE_CURSOR
	WndCursorDefaut = WndCreateStdCursor(GLFW_HAND_CURSOR);
	s->d = glfwGetPrimaryMonitor();
	/* GLFWvidmode *vdo_modes; int k,nb_vdo; int num,max;
	vdo_modes = glfwGetVideoModes(s->d,&nb_vdo);
	vdo = vdo_modes; max = 0; num = 0;
	for(k=0; k<nb_vdo; k++) {
		printf("(%s) mode #%d: %d+%d+%d %d x %d @%dHz\n",__func__,k+1,vdo->redBits,vdo->greenBits,vdo->blueBits,vdo->width,vdo->height,vdo->refreshRate);
		if(vdo->width > max) { max = vdo->width; num = k; }
		vdo++;
	}
	vdo = vdo_modes + num;
	printf("(%s) Ecran choisi: %d (%d x %d)\n",__func__,num+1,vdo->width,vdo->height);
	*/
	vdo = glfwGetVideoMode(s->d);
	printf("  Mode video courant: R%d+G%d+B%d %d x %d @%dHz\n",
		vdo->redBits,vdo->greenBits,vdo->blueBits,vdo->width,vdo->height,vdo->refreshRate);
	s->larg = vdo->width;  // 2880;
	s->haut = vdo->height; // 1800;
	//for(k=0; k<127; k++) WndShifted[k] = '�';
	WndShifted['1'] = '!'; WndShifted['2'] = '@'; WndShifted['3'] = '#'; WndShifted['4'] = '$'; WndShifted['5'] = '%';
	WndShifted['6'] = '^'; WndShifted['7'] = '&'; WndShifted['8'] = '*'; WndShifted['9'] = '('; WndShifted['0'] = ')';
	WndShifted['-'] = '_'; WndShifted['='] = '+'; WndShifted['['] = '{'; WndShifted[']'] = '}';
	WndShifted[','] = '<'; WndShifted['.'] = '>'; WndShifted['/'] = '?';
	WndShifted[';'] = ':'; WndShifted['\''] = '"'; WndShifted['\\'] = '|';

	// CGFontRef font_id; font_id = CGFontCreateWithFontName("Courier");
	(s->fonte).width = glutBitmapWidth(WndFontPtr,'M'); // Dy = glutBitmapHeight(WndFontPtr) - 1; // Dy = 13;
	(s->fonte).leading = 1;
	(s->fonte).ascent = (s->fonte).width + 2;
	(s->fonte).descent = (((s->fonte).width + 1) / 2) - (s->fonte).leading;
	printf("  Caracteres: %d x %d [%d+%d+%d]\n",(s->fonte).width,(s->fonte).ascent+(s->fonte).descent+(s->fonte).leading,(s->fonte).ascent,(s->fonte).descent,(s->fonte).leading);
//?	glfwSwapInterval(0); surtout pas
//	glfwWindowHint(); pas utile, a priori

	WndColorBlack  = WndColorGetFromRGB(0x0000,0x0000,0x0000);  /* noir  */
	WndColorWhite  = WndColorGetFromRGB(0xFFFF,0xFFFF,0xFFFF);  /* blanc */

	WndEventPileHaut = WndEventPileBas = 0;
#endif

#ifdef WXWIDGETS
	InitWxWidgetsApp(&(s->larg), &(s->haut));
	GetFontInfo(&(s->fonte.width));
	(s->fonte).leading = 1;
	(s->fonte).ascent = (s->fonte).width + 2;
	(s->fonte).descent = (((s->fonte).width + 1) / 2) - (s->fonte).leading;
	printf("  Caracteres: %d x %d [%d+%d+%d]\n",(s->fonte).width,(s->fonte).ascent+(s->fonte).descent+(s->fonte).leading,(s->fonte).ascent,(s->fonte).descent,(s->fonte).leading);
	WndColorBlack  = WndColorGetFromRGB(0x0000,0x0000,0x0000);  /* noir  */
	WndColorWhite  = WndColorGetFromRGB(0xFFFF,0xFFFF,0xFFFF);  /* blanc */
#endif
	
	strcpy(WndPrevRoutine,"le debut");
	WndPrevArg = WND_AT_END;

	WndLevelText[WND_Q_ECRAN]   = WND_COLOR_MAX;
	WndLevelBgnd[WND_Q_ECRAN]   = WND_COLOR_MIN;

	WndColorText[WND_Q_ECRAN]   = WndColorWhite;                    /* blanc */
	WndColorBgnd[WND_Q_ECRAN]   = WndColorBlack;                    /* noir  */
	WndColorLite[WND_Q_ECRAN]   = WndColorGetFromRGB(0xEFFF,0xEFFF,0xEFFF);  /* gris tres clair */
	WndColorDark[WND_Q_ECRAN]   = WndColorGetFromRGB(0x6000,0x6000,0x6000);  /* 8000 */
	WndColorHilite[WND_Q_ECRAN] = WndColorGetFromRGB(0xFFFF,0xFFFF,0x0000);  /* jaune */
	WndColorMods[WND_Q_ECRAN]   = WndColorGetFromRGB(0x0000,0xFFFF,0x0000);  /* vert  */
	WndColorAscr[WND_Q_ECRAN]   = WndColorGetFromRGB(0x8000,0x8000,0xC000);  /* bleu  */
	/*	WndColorAscr   = WndColorGetFromRGB(0xDFFF,0xDFFF,0xDFFF);     gris  */
	WndColorSub[WND_Q_ECRAN]    = WndColorGetFromRGB(0x8000,0x8000,0x8000);  /* gris fonce  */
	WndColorSuperdark[WND_Q_ECRAN] = WndColorGetFromRGB(0x1F00,0x1F00,0x1F00); /* presque noir, pour les grilles */
	WndColorGrey[WND_Q_ECRAN]   = WndColorGetFromRGB(0xA000,0xA000,0xA000);  /* gris assez clair */
	WndColorButn[WND_Q_ECRAN]   = WndColorGetFromRGB(0xB000,0xB000,0xB000);  /* gris plus clair */
	WndColorLumin[WND_Q_ECRAN]  = WndColorGetFromRGB(0xFFFF,0xFFFF,0x3FFF);  /* jaune */
	WndColorActif[WND_Q_ECRAN]  = WndColorGetFromRGB(0x0000,0x7FFF,0x0000);  /* vert sur gris */
	WndColorOrange[WND_Q_ECRAN] = WndColorGetFromRGB(0xFFFF,0x7FFF,0x0000);  /* orange */
	WndColorRouge[WND_Q_ECRAN]  = WndColorGetFromRGB(0xFFFF,0x0000,0x0000);  /* rouge */
	WndColorNoir[WND_Q_ECRAN]   = WndColorBlack;                             /* noir  */
	WndColorNote[WND_Q_ECRAN]   = WndColorMods[WND_Q_ECRAN]; /* WndColorGetFromRGB(0x7FFF,0xFFFF,0x7FFF);  vert clair */
	WndColorWarn[WND_Q_ECRAN]   = WndColorLumin[WND_Q_ECRAN]; /* WndColorGetFromRGB(0xFFFF,0xBFFF,0x3FFF);  jaune orange */
	WndColorErrr[WND_Q_ECRAN]   = WndColorGetFromRGB(0xFFFF,0x3F00,0x3F00);  /* rouge brique: 0xBFFF, 0x0000, 0x0000 */

	WndLevelText[WND_Q_PAPIER]   = WND_COLOR_MIN;
	WndLevelBgnd[WND_Q_PAPIER]    = WND_COLOR_MAX;

	WndColorText[WND_Q_PAPIER]   = WndColorBlack;                    /* noir  */
	WndColorBgnd[WND_Q_PAPIER]   = WndColorWhite;                    /* blanc */
	WndColorLite[WND_Q_PAPIER]   = WndColorGetFromRGB(0xEFFF,0xEFFF,0xEFFF);   /* gris tres clair */
	WndColorDark[WND_Q_PAPIER]   = WndColorGetFromRGB(0xB000,0xB000,0xB000);   /* 8000 puis 6000 mais B000 bon pour grilles papier */
	WndColorHilite[WND_Q_PAPIER] = WndColorGetFromRGB(0xBFFF,0xBFFF,0x0000);   /* jaune */
	WndColorMods[WND_Q_PAPIER]   = WndColorGetFromRGB(0x0000,0xBFFF,0x0000);   /* vert  */
	WndColorAscr[WND_Q_PAPIER]   = WndColorGetFromRGB(0x3FFF,0x3FFF,0x7FFF);   /* bleu clair */
	/*	WndColorAscr   = WndColorGetFromRGB(0xDFFF,0xDFFF,0xDFFF);     gris  */
	WndColorSub[WND_Q_PAPIER]    = WndColorGetFromRGB(0x8000,0x8000,0x8000);   /* gris fonce  */
	WndColorSuperdark[WND_Q_PAPIER] = WndColorGetFromRGB(0x1F00,0x1F00,0x1F00); /* presque noir, pour les grilles */
	WndColorGrey[WND_Q_PAPIER]   = WndColorGetFromRGB(0x7FFF,0x7FFF,0x7FFF);   /* gris tres clair */
	WndColorButn[WND_Q_PAPIER]   = WndColorGetFromRGB(0xB000,0xB000,0xB000);   /* gris plus clair */
	WndColorLumin[WND_Q_PAPIER]  = WndColorGetFromRGB(0x3FFF,0x3FFF,0x1000);   /* jaune */
	WndColorActif[WND_Q_PAPIER]  = WndColorGetFromRGB(0x0000,0x7FFF,0x0000);   /* vert sur gris */
	WndColorOrange[WND_Q_PAPIER] = WndColorGetFromRGB(0x7FFF,0x3FFF,0x0000);   /* orange */
	WndColorRouge[WND_Q_PAPIER]  = WndColorGetFromRGB(0x7FFF,0x0000,0x0000);   /* rouge */
	WndColorNoir[WND_Q_PAPIER]   = WndColorBlack;                              /* noir  */
	WndColorNote[WND_Q_PAPIER]   = WndColorGetFromRGB(0x3FFF,0x7FFF,0x3FFF);   /* vert clair */
	WndColorWarn[WND_Q_PAPIER]   = WndColorLumin[WND_Q_PAPIER]; /* WndColorGetFromRGB(0xFFFF,0xBFFF,0x3FFF);  jaune orange */
	WndColorErrr[WND_Q_PAPIER]   = WndColorGetFromRGB(0xFFFF,0x3F00,0x3F00);   /* rouge brique: 0xBFFF, 0x0000, 0x0000 */

	WndQual = WndQualDefaut;
	// PRINT_COLOR(WndColorText[WndQual]);
	s->decal = (s->fonte).descent + (s->fonte).leading;
	s->lig = (s->fonte).ascent + s->decal;
	s->col = (s->fonte).width;

	for(qual=0; qual<WND_NBQUAL; qual++) {
		if(WndColorLite[qual]   == (WndColor *)ERREUR) WndColorLite[qual]   = WndColorBgnd[qual];
		if(WndColorDark[qual]   == (WndColor *)ERREUR) WndColorDark[qual]   = WndColorLite[qual];
		if(WndColorHilite[qual] == (WndColor *)ERREUR) WndColorHilite[qual] = WndColorText[qual];
		if(WndColorMods[qual]   == (WndColor *)ERREUR) WndColorMods[qual]   = WndColorText[qual];
		if(WndColorAscr[qual]   == (WndColor *)ERREUR) WndColorAscr[qual]   = WndColorText[qual];
		if(WndColorSub[qual]    == (WndColor *)ERREUR) WndColorSub[qual]    = WndColorBgnd[qual];
		if(WndColorSuperdark[qual] == (WndColor *)ERREUR) WndColorSuperdark[qual] = WndColorBlack;
		if(WndColorGrey[qual]   == (WndColor *)ERREUR) WndColorGrey[qual]   = WndColorText[qual];
		if(WndColorButn[qual]   == (WndColor *)ERREUR) WndColorButn[qual]   = WndColorText[qual];
		if(WndColorLumin[qual]  == (WndColor *)ERREUR) WndColorLumin[qual]  = WndColorWhite;
		if(WndColorActif[qual]  == (WndColor *)ERREUR) WndColorActif[qual]  = WndColorText[qual];
		if(WndColorRouge[qual]  == (WndColor *)ERREUR) WndColorRouge[qual]  = WndColorText[qual];
		if(WndColorOrange[qual] == (WndColor *)ERREUR) WndColorOrange[qual] = WndColorText[qual];
		if(WndColorNote[qual]   == (WndColor *)ERREUR) WndColorNote[qual]   = WndColorText[qual];
		if(WndColorWarn[qual]   == (WndColor *)ERREUR) WndColorWarn[qual]   = WndColorText[qual];
		if(WndColorErrr[qual]   == (WndColor *)ERREUR) WndColorErrr[qual]   = WndColorRouge[qual];
		WndColorBoard[qual]  = WndColorGrey[qual];
#ifdef DETAILLE
		WndServerSingleColor(s,qual,WND_GC_STD,     WndColorBgnd[qual], WndColorText[qual]);
		WndServerSingleColor(s,qual,WND_GC_CURSOR,  WndColorBgnd[qual], WndColorLumin[qual]);
		WndServerSingleColor(s,qual,WND_GC_ASCR,    WndColorGrey[qual], WndColorAscr[qual]); // WndColorGrey
		WndServerSingleColor(s,qual,WND_GC_TEXT,    0,                  WndColorText[qual]);
		WndServerSingleColor(s,qual,WND_GC_CLEAR,   0,                  WndColorBgnd[qual]);
		WndServerSingleColor(s,qual,WND_GC_LITE,    0,                  WndColorLite[qual]); // WndColorGrey
		WndServerSingleColor(s,qual,WND_GC_DARK,    0,                  WndColorDark[qual]); // WndColorGrey
		WndServerSingleColor(s,qual,WND_GC_SELECTED,WndColorAscr[qual], WndColorMods[qual]);

		WndServerSingleColor(s,qual,WND_GC_HILITE,  WndColorBgnd[qual], WndColorHilite[qual]);
		WndServerSingleColor(s,qual,WND_GC_MODS,    WndColorBgnd[qual], WndColorMods[qual]);
		WndServerSingleColor(s,qual,WND_GC_REVERSE, WndColorText[qual], WndColorBgnd[qual]);
		WndServerSingleColor(s,qual,WND_GC_LUMIN,   WndColorBlack,      WndColorLumin[qual]);
		WndServerSingleColor(s,qual,WND_GC_GREY,    WndColorBlack,      WndColorGrey[qual]);
		WndServerSingleColor(s,qual,WND_GC_BUTN,    0,                  WndColorButn[qual]); // WndColorBlack
		WndServerSingleColor(s,qual,WND_GC_SDRK,    0,                  WndColorSuperdark[qual]); // WndColorGrey
		WndServerSingleColor(s,qual,WND_GC_NOIR,    0,                  WndColorBlack);
		WndServerSingleColor(s,qual,WND_GC_VERT,    0,                  WndColorActif[qual]);
		WndServerSingleColor(s,qual,WND_GC_ROUGE,   0,                  WndColorRouge[qual]);
		WndServerSingleColor(s,qual,WND_GC_ORANGE,  0,                  WndColorOrange[qual]);
#endif
	}
#ifndef DETAILLE
	WndServerColor(s,WND_GC_STD,     WndColorBgnd,  WndColorText);
	WndServerColor(s,WND_GC_CURSOR,  WndColorBgnd,  WndColorLumin);
	WndServerColor(s,WND_GC_ASCR,    WndColorGrey,  WndColorAscr);
	WndServerColor(s,WND_GC_TEXT,    0,             WndColorText);
	WndServerColor(s,WND_GC_CLEAR,   0,             WndColorBgnd);
	WndServerColor(s,WND_GC_LITE,    0,             WndColorLite);
	WndServerColor(s,WND_GC_DARK,    0,             WndColorDark);
	WndServerColor(s,WND_GC_SELECTED,WndColorAscr,  WndColorMods);

	WndServerColor(s,WND_GC_HILITE,  WndColorBgnd,  WndColorHilite);
	WndServerColor(s,WND_GC_MODS,    WndColorBgnd,  WndColorMods);
	WndServerColor(s,WND_GC_REVERSE, WndColorText,  WndColorBgnd);
	WndServerColor(s,WND_GC_LUMIN,   WndColorNoir,  WndColorLumin);
	WndServerColor(s,WND_GC_BOARD,   WndColorGrey,  WndColorNoir);
	WndServerColor(s,WND_GC_GREY,    WndColorNoir,  WndColorGrey);
	WndServerColor(s,WND_GC_BUTN,    0,             WndColorButn);
	WndServerColor(s,WND_GC_SDRK,    0,             WndColorSuperdark);
	WndServerColor(s,WND_GC_NOIR,    0,             WndColorNoir);
	WndServerColor(s,WND_GC_VERT,    0,             WndColorActif);
	WndServerColor(s,WND_GC_ROUGE,   0,             WndColorRouge);
	WndServerColor(s,WND_GC_ORANGE,  0,             WndColorOrange);
#endif

	WndContextValInit(s,&(WndPeintureGrise),0,WndColorAscr[WndQual]); /* WndColorGrey */

	return(BON);
}
#ifdef DETAILLE
/* ========================================================================== */
static void WndServerSingleColor(WndServer *s, int qual, int coul, WndColor *fond, WndColor *texte) {
	WndContextValInit(s,&(s->gc[qual].coul[coul]),fond,texte);
}
#endif
/* ========================================================================== */
static void WndServerColor(WndServer *s, int coul, WndColor *fond[], WndColor *text[]) {
	WndContextVal *gcval; int qual;

	for(qual=0; qual<WND_NBQUAL; qual++) {
		gcval = s->gc[qual].coul + coul;
		if(fond && text) WndContextValInit(s,gcval,fond[qual],text[qual]);
		else if(text) WndContextValInit(s,gcval,0,text[qual]);
		else if(fond) WndContextValInit(s,gcval,fond[qual],0);
	}
	/*
	if(fond && text) printf("(%s) Couleur #%d: %04X:%04X:%04X / %04X:%04X:%04X\n",__func__,coul,
			   text[WND_Q_ECRAN]->red,text[WND_Q_ECRAN]->green,text[WND_Q_ECRAN]->blue,
			   fond[WND_Q_ECRAN]->red,fond[WND_Q_ECRAN]->green,fond[WND_Q_ECRAN]->blue);
	else if(text) printf("(%s) Couleur #%d: %04X:%04X:%04X / ..............\n",__func__,coul,
			   text[WND_Q_ECRAN]->red,text[WND_Q_ECRAN]->green,text[WND_Q_ECRAN]->blue);
	else if(fond) printf("(%s) Couleur #%d: .............. / %04X:%04X:%04X\n",__func__,coul,
			   fond[WND_Q_ECRAN]->red,fond[WND_Q_ECRAN]->green,fond[WND_Q_ECRAN]->blue);
	printf("(%s) GC @%08X ->fgnd @%08X, ->bgnd@%08X\n",__func__,(unsigned int)gc,\
		   gc? (unsigned int)(gc->foreground): 0xfeedc0de,gc? (unsigned int)(gc->background): 0xfeedc0de);
	 */
}
/* ========================================================================== */
static void WndContextValInit(WndServer *s, WndContextVal *gcval, WndColor *fond, WndColor *texte) {
	gcval->font = (s->fonte).id;
	gcval->line_width = 1;
	gcval->line_style = 0 /* LineSolid */ ;
	WndContextValSetColors(gcval,fond,texte);
}
/* ========================================================================== */
static void WndContextValSetColors(WndContextVal *gcval, WndColor *fond, WndColor *text) {
#ifdef X11
	if(fond) gcval->background = fond->pixel;
	if(text) gcval->foreground = text->pixel;
#endif
#ifdef WIN32
	if(fond) gcval->background = *fond;
	if(text) gcval->foreground = *text;
#endif
#ifdef OPENGL
	gcval->background = fond;
	gcval->foreground = text;
#endif
#ifdef QUICKDRAW
	gcval->background = fond;
	gcval->foreground = text;
#endif
}
/* ========================================================================== */
void WndExit() {
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	glfwTerminate();
#endif
#ifdef MENU_BARRE_MAC
	MenuHandle menu;
#endif
	if(WndModeNone) return;
#ifdef TIMER_UNIQUE
	if(WndHorloge) {
		TimerStop(WndHorloge);
		sleep(2);
		TimerDelete(WndHorloge);
	}
#endif
#ifdef QUICKDRAW
#ifdef MENU_BARRE_MAC
	menu = GetMenuHandle(MENU_FICHIER);
#ifdef CW5
	EnableItem(menu,ITEM_QUITTER);
#else
	EnableMenuItem(menu,ITEM_QUITTER);
#endif
	SetMenuBar(WndMenuStandard);
	DrawMenuBar();
	WndMenuAffiche = 0;
#endif
#ifdef AVEC_SIOUX
	WndPrint("Faire <Pomme-Q><Return>\n");
#endif
#endif
}
/* ========================================================================== */
int WndTotalWidth(WndServer *svr) {
	if(svr == WND_A4) return(WND_A4_LARG);
	else return(svr->larg);
}
/* ========================================================================== */
int WndTotalHeigth(WndServer *svr) {
	if(svr == WND_A4) return(WND_A4_HAUT);
	else return(svr->haut);
}
/* ========================================================================== */
void WndAPropos(char *appli, void (*fctn)()) {
#ifdef QUICKDRAW
	char texte[80]; int l;
	MenuHandle menu;
#endif

	if(WndModeNone) return;
	strncpy(WndAProposAppli,appli,WND_APPLI);
	WndAProposFctn = fctn;
	if(WndAProposAppli[0]) {
#ifdef QUICKDRAW
		sprintf(texte,".A propos de %s",WndAProposAppli);
		l = strlen(texte) - 1;
		texte[0] = (unsigned char)l;
		menu = GetMenuHandle(MENU_POMME);
		InsertMenuItem(menu,(unsigned char *)texte,0);
#endif
	}
}
#ifdef MENU_BARRE
/* ========================================================================== */
void WndMenuDebug(char dbg) { WndDebugBarre = dbg; }
/* ========================================================================== */
int WndMenuCreate(char *texte) {
#ifdef QUICKDRAW
	char chaine[256]; int l;

	if(WndCodeHtml) return(1);
	l = strlen(texte); chaine[0] = l; strcpy(chaine+1,texte);
	WndMenuLast = NewMenu(WND_BASE_ID + WndMenuId,(unsigned char *)chaine);
	if(WndDebugBarre) printf("(%s) NewMenu(%d,\\%d'%s') rend %d\n",__func__,WND_BASE_ID + WndMenuId,chaine[0],(unsigned char *)(chaine+1),(int)WndMenuLast);
	WndMenuDim = 0;
	WndMenuId++;
	return(WndMenuLast != 0);
#else
	return(1);
#endif
}
/* ========================================================================== */
void WndMenuItem(char *texte, char suite, char sel) {
#ifdef QUICKDRAW
	char chaine[256]; int l;

	if(WndCodeHtml) return;
	l = strlen(texte);
	if(suite == 'm') l += 2;
	else if(suite == 'p') l += 4;
	chaine[0] = l; strcpy(chaine+1,texte);
	if(suite == 'm') strcat(chaine," >");
	else if(suite == 'p') strcat(chaine," ...");
/*	InsertMenuItem(WndMenuLast,chaine,9999); pb avec < / ( */
/*  autres metacaracteres: <B <I <U <O <S pour bold,italic,underline,outline,shadow
	/<char> : raccourci clavier
	/<esc> : a un sous-menu */
	if(WndDebugBarre) printf("(%s) InsertMenuItem(%d,'%s')\n",__func__,(int)WndMenuLast,(sel > 0)?"\px":((suite == 's')? "\p<U<B-": ((sel < 0)? "\px(": "\p<Ix")));
	InsertMenuItem(WndMenuLast,(sel > 0)?"\px":((suite == 's')? "\p<U<B-": ((sel < 0)? "\px(": "\p<Ix")),9999);
	if(WndDebugBarre) printf("(%s) SetMenuItemText(%d,%d,\\%d'%s')\n",__func__,(int)WndMenuLast,WndMenuDim+1,chaine[0],(unsigned char *)(chaine+1));
	SetMenuItemText(WndMenuLast,WndMenuDim+1,(unsigned char *)chaine);
	WndMenuDim++;
	if(WndDebugBarre) {
		int num; unsigned char item[80];
		printf("(%s) WndMenuLast[%d] nouveau:\n",__func__,WndMenuDim);
		for(num=0; num<WndMenuDim; num++) {
			GetMenuItemText(WndMenuLast,num+1,(unsigned char *)item);
			item[item[0]+1] = '\0';
			printf("(%s) | %2d: '%s'\n",__func__,num+1,item+1);
		}
	}
#endif
}
/* ========================================================================== */
void WndMenuSepare() { 
	if(WndCodeHtml) return;
#ifdef QUICKDRAW
	if(WndDebugBarre) printf("(%s) InsertMenuItem(%d,'%s')\n",__func__,(int)WndMenuLast,"\p-(");
	InsertMenuItem(WndMenuLast,"\p-(",9999);
	WndMenuDim++;
	if(WndDebugBarre) {
		int num; unsigned char item[80];
		printf("(%s) WndMenuLast[%d] nouveau:\n",__func__,WndMenuDim);
		for(num=0; num<WndMenuDim; num++) {
			GetMenuItemText(WndMenuLast,num+1,(unsigned char *)item);
			item[item[0]+1] = '\0';
			printf("(%s) | %2d: '%s'\n",__func__,num+1,item+1);
		}
	}
#endif
}
/* ========================================================================== */
void WndMenuInsert() { 
	if(WndCodeHtml) return;
#ifdef QUICKDRAW
	if(WndDebugBarre) printf("(%s) InsertMenu(%d,0)\n",__func__,(int)WndMenuLast);
	InsertMenu(WndMenuLast,0);
#endif
}
/* ========================================================================== */
void WndMenuClear(char *titre) {
#ifdef QUICKDRAW
	char texte[80]; short num,nb;

	if(WndCodeHtml) return;
#ifdef CW5
	nb = CountMItems(WndMenuLast);
#else
	nb = WndMenuDim;
#endif
	if(WndDebugBarre) {
		printf("(%s) Item a effacer: '%s'\n",__func__,titre);
		printf("(%s) WndMenuLast[%d] avant:\n",__func__,nb);
		for(num=0; num<nb; num++) {
			GetMenuItemText(WndMenuLast,num+1,(unsigned char *)texte);
			texte[texte[0]+1] = '\0';
			printf("(%s) | %2d: '%s'\n",__func__,num+1,texte+1);
		}
	}
	for(num=0; num<nb; num++) {
		GetMenuItemText(WndMenuLast,num+1,(unsigned char *)texte);
		texte[texte[0]+1] = '\0';
		if(!strcmp(texte+1,titre)) {
			if(WndDebugBarre) printf("(%s) Texte trouve en position %d\n",__func__,num+1);
			DeleteMenuItem(WndMenuLast,num+1);
			--WndMenuDim;
			break;
		}
	}
	if(WndDebugBarre) {
		printf("(%s) WndMenuLast[%d] apres:\n",__func__,nb);
		for(num=0; num<WndMenuDim; num++) {
			GetMenuItemText(WndMenuLast,num+1,(unsigned char *)texte);
			texte[texte[0]+1] = '\0';
			printf("(%s) | %2d: '%s'\n",__func__,num+1,texte+1);
		}
	}
#endif
}
/* ========================================================================== */
void WndMenuDisplay() {
	if(WndCodeHtml) return;
#ifdef QUICKDRAW
	DrawMenuBar(); 
#endif
	WndMenuAffiche = 1;
#ifdef QUICKDRAW
	WndMenuAppli = GetMenuBar();
#endif
}
#endif /* MENU_BARRE */
/* ========================================================================== */
int WndLigToPix(int lig) { return(lig * WndCurSvr->lig); }
/* ========================================================================== */
int WndColToPix(int col) { return(col * WndCurSvr->col); }
/* ========================================================================== */
int WndPixToLig(int pix) { return(pix / WndCurSvr->lig); }
/* ========================================================================== */
int WndPixToCol(int pix) { return(pix / WndCurSvr->col); }

/* ========================================================================== */
/* ========================== Gestion des fenetres ========================== */
/* ========================================================================== */
#ifdef X11
void WndPrintAttributes(char *nom, WndFrame f) {
	WndScreen d;
	WndIdent w;
	XWindowAttributes attributs;
  
	d = (f->s)->d;
	w = f->w;
	XGetWindowAttributes(d, w, &attributs);
	#ifdef DEBUG_IMAGE
		WndPrint("(%s) La fenetre 0x%08X [%d x %d] @(%d,%d) est %s (%d) et de profondeur %d\n",
	       nom,w,
	       attributs.width,attributs.height,attributs.x,attributs.y,
	       (attributs.map_state == IsUnmapped)? "unmapped":
	       (attributs.map_state == IsUnviewable)? "unviewable": "viewable",
	       attributs.map_state,attributs.depth);
	#endif
}
#endif /* X11 */
/* ========================================================================== */
void WndNewRoot(char *title, int sizx, int sizy) {
	int posx,posy;
	WndServer *s;
#ifdef X11
	WndScreen d; WndIdent w;
#endif
#ifdef QUICKDRAW
	Rect r; WndIdent w;
#endif

	if(WndModeNone) return;
	s = WndCurSvr;
	if((sizx <= 0) || (sizx > s->larg)) sizx = s->larg; posx = (s->larg - sizx) / 2;
	if((sizy <= 0) || (sizy > s->haut)) sizy = s->haut; posy = (s->haut - sizy) / 2;
#ifdef X11
	d = s->d;
	w = XCreateSimpleWindow(d,WndRoot,posx,posy,sizx,sizy,2,BlackPixel(d,DefaultScreen(d)),WhitePixel(d,DefaultScreen(d)));
		// WndColorText[WndQual]->pixel,WndColorBgnd[WndQual]->pixel);
	XMapWindow(d,w);
	WndRoot = w;
//	XSelectInput(d,w,WndEventMask | SubstructureNotifyMask);
//	XSelectInput(d,w,SubstructureNotifyMask);
	XSelectInput(d,WndRoot,0x01FFFFFF);
#endif
#ifdef QUICKDRAW
	r.left = posx; r.top = posy; 
	r.right = r.left + sizx; r.bottom = r.top + sizy;
	w = NewCWindow(nil,&r,"\p\0",false,zoomDocProc,WND_INFRONT,true,WndNum++);
	SetPort(WNDPORT(w));
	TextSize((pixval)WndFontSize);
	WND_UTILISE_FONTE(w,(WndCurSvr->fonte).id);
	RGBForeColor(WndColorText[WndQual]);
	RGBBackColor(WndColorBgnd[WndQual]);
	ShowWindow(w);
#endif

}
/* ========================================================================== */
WndFrame WndDisplay(int qualite, int posx, int posy, int sizx, int sizy) {
	return(WndCreate(WND_FEN_STD,qualite,posx,posy,sizx,sizy));
}
/* ========================================================================== */
WndFrame WndAttach(int qualite, int posx, int posy, int sizx, int sizy) {
	return(WndCreate(WND_FEN_SUB,qualite,posx,posy,sizx,sizy));
}
/* ========================================================================== */
WndFrame WndCreate(int type, int qualite, int posx, int posy, int sizx, int sizy) {
	WndIdent w;
	WndFrame f;
	int qual,num;
	int lig,col,colmax;

	f = (WndFrame)malloc(sizeof(struct WndFrameStruct));
	if(!f) return(0);
	if(WndModeNone) w = 0;
	else if(WndCodeHtml) w = (WndIdent)((void *)(int64)WndOpened + 1);
	else if(WndModeVt100) {
		colmax = posx + sizx - 1;
		if(WndInitVt100) {
			WndPrint("\x1b(B\x1b)0\x0f"); WndInitVt100 = 0; 
		}
		WndPrint("\x0e");               /* jeu de caracteres: G1 (graphique) */
		WndPrint("\x1b[%d;%dHl",posy,posx);
		for(col=posx+1; col<colmax; col++) WndPrint("q");
		WndPrint("k");
		for(lig=1; lig<sizy-1; lig++) {
			WndPrint("\x1b[%d;%dHx\x1b[%dX",posy+lig,posx,sizx-2);
			WndPrint("\x1b[%d;%dHx",posy+lig,colmax);
		}
		WndPrint("\x1b[%d;%dHm",posy+sizy-1,posx);
		for(col=posx+1; col<colmax; col++) WndPrint("q");
		WndPrint("j");
		WndPrint("\x0f");               /* jeu de caracteres: G0 (ASCII) */
		w = 0;

	} else {
	#ifdef OPENGL
		w = glfwCreateWindow(sizx+WND_ASCINT_WIDZ,sizy+WND_ASCINT_WIDZ,"...",NULL,NULL);
		if(w) {
			glfwMakeContextCurrent(w);
			// int width, height; glfwGetFramebufferSize(w,&width,&height); glViewport(0,0,width,height);
			glfwSetWindowPos(w,posx,posy);
			glfwSetInputMode(w,GLFW_STICKY_KEYS,GL_TRUE);
			glfwSetInputMode(w,GLFW_STICKY_MOUSE_BUTTONS,GL_TRUE);
			glfwSetInputMode(w,GLFW_CURSOR,GLFW_CURSOR_NORMAL);
			glfwSetKeyCallback(w,WndColbacKey);
			//- glfwSetCursorPosCallback(w,WndColbacCurseur);
			glfwSetMouseButtonCallback(w,WndColbacSouris);
			glfwSetCursorEnterCallback(w,WndColbacEntre);
			glfwSetWindowPosCallback(w,WndColbacPos);
			glfwSetWindowSizeCallback(w,WndColbacSize);
			glfwSetWindowRefreshCallback(w,WndColbacRefresh);
			glfwSetWindowCloseCallback(w,WndColbacClose);
			glfwSetCursor(w,WndCursorDefaut);
			glfwSwapBuffers(w);
			WndNum++;
		} else {
			fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
			getchar();
			glfwTerminate();
		}
	#endif /* OPENGL */
	#ifdef WXWIDGETS
		w = WndCreateWx(posx, posy, sizx+WND_ASCINT_WIDZ,sizy+WND_ASCINT_WIDZ);
		if(w) {
			WndNum++;
		} else {
			fprintf( stderr, "Failed to open WxWidgets window.\n" );
			getchar();
		}
	#endif /* WXWIDGETS */
	}

	f->w = w;
	f->s = WndCurSvr;
	f->x = posx;
	f->y = posy;
	f->h = sizx;
	f->v = sizy;
	f->x0 = f->y0 = 0;
	f->xm = sizx; f->ym = sizy;
	f->lig0 = f->col0 = 0;
	f->ligm = WndPixToLig(sizy); f->colm = WndPixToCol(sizx);
	f->lig = f->col = -1;
	f->en_cours = 0;
	f->qualite = qualite;
	f->passive = 0;
	f->image.dx = f->image.dy = 0;
	f->image.lut = 0; f->image.nbcolors = 0;
	f->image.R = f->image.G = f->image.B = f->image.A = 0;
	f->image.surf = 0;
	f->extras = 0; f->extra_nb = 0;  f->extra_max = 0;
#ifdef TIMER_PAR_EVT
	#ifdef TIMER_UNIQUE
		f->delai = f->restant = 0;
	#else
		f->timer = 0;
	#endif
#endif
	f->dessous = WND_NOT_IN_STACK; /* valeur differente <=> mise dans la pile */
	f->nb_gc = WND_GC_BASE;
	for(qual=0; qual<WND_NBQUAL; qual++) {
		if(!(f->gc[qual].coul = Malloc(f->nb_gc,WndContextPtr))) return(0);
		for(num=0; num<f->nb_gc; num++) {
			f->gc[qual].coul[num] = WndContextCreateFromVal(f,&(WndCurSvr->gc[qual].coul[num]));
			// printf("(%s) GC[%d] ",__func__,num); PRINT_GC(f->gc[qual].coul[num]);
		}
	}
	if(WndPremiere == 0) WndPremiere = f;
	WndOpened++;
	return(f);
}
/* ========================================================================== */
void WndMinimale(WndFrame f) { WndPremiere = f; }
/* ========================================================================== */
void WndFenColorNb(WndFrame f, int nb) {
	char qual; int num,max;
	WndContextPtr *gc;

	if(!f) return;
	if(f->nb_gc == nb) return;
	max = nb; if(max > f->nb_gc) max = f->nb_gc;
	for(qual=0; qual<WND_NBQUAL; qual++) {
		if(!(gc = Malloc(nb,WndContextPtr))) return;
		for(num=0; num<max; num++) {
			gc[num] = WndContextCreate(f);
			WndContextCopy(f,f->gc[qual].coul[num],gc[num]);
			WndContextFree(f,f->gc[qual].coul[num]);
		}
		for( ; num<nb; num++) gc[num] = 0;
		if(f->gc[qual].coul) free(f->gc[qual].coul);
		f->gc[qual].coul = gc;
	}
	f->nb_gc = nb;
}
/* ========================================================================== */
void WndFenColorSet(WndFrame f, int index, int couleur) {
	char qual;
	WndServer *s;

	if(!f) return;
	if((index < 0) || (index >= f->nb_gc)) return;
	s = f->s;
	for(qual=0; qual<WND_NBQUAL; qual++) {
		WndContextFree(f,f->gc[qual].coul[index]);
		f->gc[qual].coul[index] = WndContextCreateFromVal(f,&(s->gc[qual].coul[couleur]));
	}
}
/* ========================================================================== */
void WndFenColorDump(WndFrame f, int qual) {

//	printf("(%s) F@%08X: %d couleurs. Qualite %s:\n",__func__,(hexa)f,f->nb_gc,WndSupportNom[qual]);
#ifndef X11
	int index;
	for(index=0; index<f->nb_gc; index++) {
		printf("(%s)   %-2d: gc@%08X ->fgnd @%08X, ->bgnd@%08X\n",__func__,index,(hexa)f->gc[qual].coul[index],
			f->gc[qual].coul[index]? (unsigned int)(f->gc[qual].coul[index]->foreground): 0xfeedc0de,
			f->gc[qual].coul[index]? (unsigned int)(f->gc[qual].coul[index]->background): 0xfeedc0de);
	}
#endif
}
/* ========================================================================== */
void WndRepere(WndFrame f, int x0, int y0, int xm, int ym) {
/* Les acces en (x,y) sont transformes en acces en (x0+x,y0+y)
   et supprimes si x<0 ou y<0 ou x>xm ou y>ym */
	if(!f) return;
	f->x0 = x0; f->y0 = y0; f->xm = xm; f->ym = ym;
	f->lig0 = WndPixToLig(y0); f->col0 = WndPixToCol(x0);
	f->ligm = WndPixToLig(ym); f->colm = WndPixToCol(xm);
}
/* ========================================================================== */
void WndTitle(WndFrame f, char *titre) {
	WndIdent w;

	if(f == WND_AT_END) return;
	if(WndModeNone) return;
	w = f->w;
#ifdef WXWIDGETS
	WndTitleWx(w, titre);
#endif
#ifdef OPENGL
	glfwSetWindowTitle(w,titre);
#endif
}
/* ========================================================================== */
void WndFenetreQualite(WndFrame f, char qual) { if(f) f->qualite = qual; }
/* ========================================================================== */
char WndBasicColors(WndFrame f, char *nom_fond, char *nom_text) {
	WndIdent w;
  	WndColor *fond,*text;
	WndContextVal gcval;
	WndServer *s;
#ifdef X11
	WndScreen d;
	XSetWindowAttributes attributs;
#endif

	if(WndModeNone) return(1);
	fond = WndColorGetFromName(nom_fond);
	if(fond == (WndColor *)ERREUR) return(0);
	text = WndColorGetFromName(nom_text);
	if(text == (WndColor *)ERREUR) return(0);

	w = f->w;
	s = f->s;
#ifdef X11
	d = s->d;
	/*	XGetWindowAttributes(d, w, &attributs); */
	attributs.background_pixel = fond->pixel;
	XChangeWindowAttributes(d,w,CWBackPixel,&attributs);
#endif

#ifdef WIN32
	//background
	HBRUSH hBrush;
	SetClassLong(w, GCL_HBRBACKGROUND, (long) (hBrush = CreateSolidBrush(*fond)));
	DeleteObject(hBrush);
	//foreground
	//pas de foreground associe
#endif

#ifdef QUICKDRAW
	if(!WndCodeHtml) {
		SetPort(WNDPORT(w));
		RGBForeColor(text);
		RGBBackColor(fond);
		//	PortChanged(w); a priori inutile
	}
#endif
	gcval.font = (s->fonte).id;
	WndContextValSetColors(&gcval,fond,text);
	WND_STD = WndContextCreateFromVal(f,&gcval);

	return(1);
}

#ifdef WIN32
/* ========================================================================== */
void WndDeleteUserCursor(WndCursor curs) { DestroyCursor(curs); }
#endif
/* ========================================================================== */
char WndBasicRGB(WndFrame f, int fr, int fg, int fb, int tr, int tg, int tb) {
	WndIdent w; WndServer *s;
  	WndColor *fond,*text;
	WndContextVal gcval;
#ifdef X11
    WndScreen d;
	XSetWindowAttributes attributs;
#endif

	if(WndModeNone) return(1);
	fond = WndColorGetFromRGB(fr,fg,fb);
	if(fond == (WndColor *)ERREUR) return(0);
	text = WndColorGetFromRGB(tr,tg,tb);
	if(text == (WndColor *)ERREUR) return(0);

	w = f->w; s = f->s;
#ifdef X11
	d = s->d;
	/*	XGetWindowAttributes(d, w, &attributs); */
	attributs.background_pixel = fond->pixel;
	/*	attributs.foreground_pixel = text->pixel; n'existe pas! */
	XChangeWindowAttributes(d,w,CWBackPixel /* | CWForePixel */ ,&attributs);
#endif

#ifdef WIN32
	// background
	HBRUSH hBrush;
	SetClassLong(w, GCL_HBRBACKGROUND, (long) (hBrush = CreateSolidBrush(*fond)));
	DeleteObject(hBrush);
	//foreground: pas de foreground associe
#endif

#ifdef QUICKDRAW
	if(!WndCodeHtml) {
		SetPort(WNDPORT(w));
		RGBBackColor(fond);
		RGBForeColor(text);
		//	PortChanged(w); a priori inutile
	}
#endif
	gcval.font = (s->fonte).id;
	WndContextValSetColors(&gcval,0,text);
	WND_TEXT = WndContextCreateFromVal(f,&gcval);

	return(1);
}
/* ========================================================================== */
char WndRefreshBegin(WndFrame f) {
	char devra_terminer;
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return 0;
#endif
#ifdef OPENGL
	devra_terminer = !(f->en_cours);
	if(devra_terminer) {
		// ImprimePileAppels;
		// printf("(%s) Debut de modification de F=%08llX\n",__func__,(uint64)f);
		glfwMakeContextCurrent(f->w);
		glfwSwapBuffers(f->w); f->en_cours = 1;
	}
#else
	devra_terminer = 0;
#endif
	return(devra_terminer);
}
/* ========================================================================== */
void WndRefreshEnd(WndFrame f) {
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	if((f->en_cours)) {
		// printf("(%s)     Fin de modification de F=%08llX\n",__func__,(uint64)f);
//?		glfwSwapInterval(1); surtout pas
		glfwSwapBuffers(f->w); f->en_cours = 0;
	}
#endif
}
/* ========================================================================== */
void WndControls(WndFrame f) {
	if(WndModeNone || (f == WND_AT_END) || WndCodeHtml) return;
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	int sizx,sizy;
	glfwGetWindowSize(f->w,&sizx,&sizy);
	WndPaint(f,sizx-WND_ASCINT_WIDZ,sizy-WND_ASCINT_WIDZ,WND_ASCINT_WIDZ,WND_ASCINT_WIDZ,WndColorAscr[WND_Q_ECRAN],0);
#endif
#ifdef QUICKDRAW
	DrawGrowIcon(f->w);
#endif
}
/* ========================================================================== */
void WndBorders(WndFrame f) {
	int sizx,sizy;

	if(WndModeNone || (f == WND_AT_END) || WndCodeHtml) return;
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	char doit_terminer;
	doit_terminer = WndRefreshBegin(f);
	glfwGetWindowSize(f->w,&sizx,&sizy);
#else
	sizx = f->h + WND_ASCINT_WIDZ; sizy = f->v + WND_ASCINT_WIDZ;
#endif
	WndFillBgnd(f,WND_GC(f,WND_GC_ASCR),sizx-WND_ASCINT_WIDZ+1,0,WND_ASCINT_WIDZ-1,sizy-WND_ASCINT_WIDZ);
	WndFillBgnd(f,WND_GC(f,WND_GC_ASCR),0,sizy-WND_ASCINT_WIDZ+1,sizx-WND_ASCINT_WIDZ,WND_ASCINT_WIDZ-1);
	WndFillBgnd(f,WND_GC(f,WND_GC_SELECTED),sizx-WND_ASCINT_WIDZ+1,sizy-WND_ASCINT_WIDZ+1,WND_ASCINT_WIDZ-1,WND_ASCINT_WIDZ-1);
#ifdef OPENGL
	if(doit_terminer) WndRefreshEnd(f);
#endif
}
/* ========================================================================== */
WndCursor WndCreateStdCursor(int num) {
	WndCursor curseur;
#ifdef WXWIDGETS
	return WndCreateStdCursorWx();
#endif
#ifdef OPENGL
	return(glfwCreateStandardCursor(num));
#endif
	return(curseur);
}
/* ========================================================================== */
WndCursor WndCreateUserCursor(int larg, int haut, unsigned char *map) {
	WndCursor curseur;
	
	if(WndModeNone) return((WndCursor)0);
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return NULL;
#endif
#ifdef X11
	Pixmap source,masque;
	WndScreen d; WndColor *fgnd,*bgnd;

	d = WndCurSvr->d;
	source = XCreateBitmapFromData(d,WndRoot,(const char *)map,larg,haut);
	masque = source;
	fgnd = WndColorHilite[WndQual];
	bgnd = WndColorBlack;
	curseur = XCreatePixmapCursor(d,source,masque,fgnd,bgnd,larg/2,haut/2);
	XFreePixmap(d,masque);
#endif
#ifdef WIN32
	curseur = CreateCursor(NULL, larg/2, haut/2, larg, haut, (const void*)map, (const void*)map);
#endif
#ifdef QUICKDRAW_OR_OPENGL
	curseur = 0;
#endif
	return(curseur);
}
/* ========================================================================== */
void WndAssignCursor(WndFrame f, WndCursor curseur) {
	WndIdent w;

	if(WndModeNone) return;
	w = f->w;
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	glfwSwapBuffers(w);
	glfwSetCursor(w,curseur);
	glfwSwapBuffers(w);
#endif
#ifdef X11
	XDefineCursor((f->s)->d,w,curseur);
#endif
#ifdef WIN32
	SetClassLong(w, GCL_HCURSOR, (long)curseur);
#endif
	return;
}
/* ========================================================================== */
void WndExtraInit(WndFrame f, int max) {
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	if(max >= 0) {
		if(f->extras) {
			if((f->extra_max < max) || (max == 0)) {
				free(f->extras); f->extras = 0;
				f->extra_max = 0;
			}
		}
		if(!(f->extras) && (max > 0)) {
			f->extras = (WndTypeExtraActn *)malloc(max * sizeof(WndTypeExtraActn));
			if(f->extras) f->extra_max = max;
		}
	}
#endif
	f->extra_nb = 0;
}
/* ========================================================================== */
static void WndExtraExec(WndFrame f, char action, WndContextPtr gc, int i, int j, int k, int l, void *info) {
	switch(action) {
	  case WND_ACTN_PAINT:  WndPaint(f,i,j,k,l,(WndColor *)info,3); break;
	  case WND_ACTN_WRITE:  WndWrite(f,gc,i,j,(char *)info); break;
	  case WND_ACTN_STRING: WndString(f,gc,i,j,(char *)info); break;
	  case WND_ACTN_LINE:   WndLine(f,gc,i,j,k,l); break;
	}
}
/* ========================================================================== */
void WndExtraAdd(WndFrame f, char action, WndContextPtr gc, int i, int j, int k, int l, void *info) {
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	if(!(f->extras)) return;
	if(f->extra_nb >= f->extra_max) return;
	f->extras[f->extra_nb].gc = gc;
	f->extras[f->extra_nb].i = i;
	f->extras[f->extra_nb].j = j;
	f->extras[f->extra_nb].k = k;
	f->extras[f->extra_nb].l = l;
	f->extras[f->extra_nb].info = info;
	f->extras[f->extra_nb].action = action;
	(f->extra_nb)++;
#else  /* OPENGL */
	WndExtraExec(f,action,gc,i,j,k,l,info);
#endif /* OPENGL */
}
/* ========================================================================== */
short WndExtraNb(WndFrame f) { return(f->extra_nb); }
/* ========================================================================== */
void WndExtraDisplay(WndFrame f) {
	int n;
	WndContextPtr gc; char action; int i,j,k,l; void *info;

	if(!(f->extras)) return;
	for(n=0; n<f->extra_nb; n++) {
		action = f->extras[n].action;
		gc = f->extras[n].gc;
		i = f->extras[n].i;
		j = f->extras[n].j;
		k = f->extras[n].k;
		l = f->extras[n].l;
		info = f->extras[n].info;
		WndExtraExec(f,action,gc,i,j,k,l,info);
	}
}
/* ========================================================================== */
void WndResize(WndFrame f) {
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	// glfwSetWindowSize(f->w,f->h + WND_ASCINT_WIDZ,f->v + WND_ASCINT_WIDZ); provoque un rebouclage
#endif
#ifdef QUICKDRAW
	if(!WndCodeHtml) SizeWindow(f->w,f->h + WND_ASCINT_WIDZ,f->v + WND_ASCINT_WIDZ,0);
	/* quoique.. h et v devraient etre short */
#endif
}
// #define DEBUG_STACK
/* ========================================================================== */
void WndStackPrePrint(char *routine, WndFrame f) {
	int i,j;
	WndFrame cur,prec,derniere;

/*	return; */
	if(WndTopFrame == WND_AT_END) { /* pas de pile */
		WndPrint("Apres %s, %s(%08llX) trouve la pile vide\n",WndPrevRoutine,routine,(UInt64)f);
		return;
	}
	if(WndModeNone) return;
	cur = WndTopFrame; i = 0;
	WndPrint("Apres %s, %s(%08llX) trouve la pile[%d]:\n   ",WndPrevRoutine,routine,(UInt64)f,WndActives);
	do {
		WndPrint("%08llX ",(UInt64)cur);
		derniere = cur;
		cur = cur->dessous; i++;
		if(!(i%8)) WndPrint("\n");
		if(cur == WND_AT_END) break;
		prec = WndTopFrame; j = 0;
		while(j < i) {
			if(cur == prec) break;
			prec = prec->dessous; j++;
		}
		if(j < i) {
			WndPrint("%08llX=wnd #%d !!!",(UInt64)cur,j);
			derniere->dessous = WND_AT_END;
			break;
		}
	} while(i <= (2 * WndActives));
	if(i%8) WndPrint("\n");
}
/* ========================================================================== */
static void WndStackPrint(char *routine, WndFrame f) {
	int i,j;
	WndFrame cur,prec,derniere;

/*	return; */
	if(WndTopFrame == WND_AT_END) { /* pas de pile */
		WndPrint("Apres %s(%08x), %s(%08x) laisse la pile vide\n",
			WndPrevRoutine,(hexa)WndPrevArg,routine,(hexa)f);
		return;
	}
	cur = WndTopFrame; i = 0;
	WndPrint("Apres %s(%08x), %s(%08x -> %08x) laisse la pile[%d]:\n   ",
		WndPrevRoutine,(hexa)WndPrevArg,routine,(hexa)f,(f == WND_AT_END)? 0: (hexa)f->dessous,WndActives);
	do {
		WndPrint("%08x ",(hexa)cur);
		if((cur != WND_AT_END) && ((hexa)cur < 0x10000)) break;
		derniere = cur;
		cur = cur->dessous; i++;
		if(!(i%8)) WndPrint("\n   ");
		if(cur == WND_AT_END) break;
		prec = WndTopFrame; j = 0;
		while(j < i) {
			if(cur == prec) break;
			prec = prec->dessous; j++;
		}
		if(j < i) {
			WndPrint("%08x=wnd #%d !!!",(hexa)cur,j);
			derniere->dessous = WND_AT_END;
			break;
		}
	} while(i <= ( 2 * WndActives));
	if(i%8) WndPrint("\n");
}
/* ========================================================================== */
static void WndStackVerif(char toujours, char *routine, WndFrame f) {
	WndFrame cur; int stacksize;

	if(toujours) WndStackPrint(routine,f);
	else {
		stacksize = 0;
		cur = WndTopFrame;
		while(cur != WND_AT_END) {
			cur = cur->dessous;
			if(++stacksize > WndActives) {
				WndStackPrint(routine,f);
				break;
			}
			if((cur != WND_AT_END) && ((hexa)cur < 0x10000)) {
				stacksize = -1; break;
			}
		}
		if(stacksize < WndActives) WndStackPrint(routine,f);
	}
	strcpy(WndPrevRoutine,routine);
	WndPrevArg = f;
}
/* ========================================================================== */
static char WndUnstack(WndFrame f) {
	WndFrame courante,precedente;
	int nbloop;
	nbloop = 0;

	if(WndTopFrame == WND_AT_END) return(0); /* pile vide */
	if(f->dessous == WND_NOT_IN_STACK) return(1); /* pas encore mise dans la pile */
	if(f->passive) { WndWriteOnlyDel(f); return(1); }
	precedente = 0;
	courante = WndTopFrame;
	while(courante != f) {
		precedente = courante;
		courante = precedente->dessous;
		if(courante == WND_AT_END) break;
		if(++nbloop > WndActives) {
			WndPrint("(WndUnstack) %d/%d fenetres examinees!!? ca fait trop...\n",nbloop,WndActives);
			courante = WND_AT_END;
			break;
		}
		if((courante != WND_AT_END) && ((hexa)courante < 0x10000)) {
			courante = WND_AT_END;
			break;
		}
	}
	if(courante != WND_AT_END) {
		if(precedente) precedente->dessous = courante->dessous;
		else WndTopFrame = courante->dessous;
		WndActives--;
	}
#ifdef DEBUG
	  else {
	  WndPrint("(WndUnstack) Fenetre non depilee: %08llX\n",(UInt64)f);
	  WndStackPrint("WndUnstack",f);
	}
#endif
#ifdef DEBUG_STACK
	WndPrint("(WndUnstack) Retire F=%08llX => TopFrame F=%08llX\n",(UInt64)f,(UInt64)WndTopFrame);
#endif
	WndStackVerif(0,"WndUnstack",f);
	return(1);
}
/* ========================================================================== */
void WndRaise(WndFrame f) {
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef X11
	WndScreen d;
#endif
	WndIdent w;

	if(WndModeNone) return;
#ifdef DEBUG_STACK
	WndPrint("(WndRaise)      TopFrame: F=%08llX\n",(UInt64)f);
#endif
	if(f == WND_AT_END) return;
#ifdef DEBUG_STACK
	WndPrint("(WndRaise)      TopIdent: W=%08llX\n",(UInt64)(f->w));
#endif
	if(!(w = f->w)) return;
#ifdef OPENGL
	if(!WndCodeHtml) glfwShowWindow(w);
#endif
#ifdef X11
	d = (f->s)->d;
	XRaiseWindow(d,w);
#endif
#ifdef WIN32
	BringWindowToTop(w);
#endif
#ifdef QUICKDRAW
	if(!WndCodeHtml) { SelectWindow(w); ShowWindow(w); }
#endif

	if(f == WndTopFrame) return;
	WndUnstack(f);
	f->dessous = WndTopFrame; WndTopFrame = f;
#ifdef DEBUG_STACK
	WndPrint("(WndRaise) La fenetre du dessus (%08llX) devient Top Frame\n",(UInt64)f);
#endif
	WndActives++;
	if(f->passive) {
		WndPrint("(WndRaise) La fenetre du dessus (%08llX) devient 'active'\n",(UInt64)f);
		WndWriteOnlyDel(f);
	}
	WndStackVerif(0,"WndRaise",f);
}
/* ========================================================================== */
void WndPutAtTop(WndFrame f) {
#ifdef DEBUG_STACK
	WndPrint("(WndPutAtTop)      Frame: F=%08llX\n",(UInt64)f);
#endif
	if(WndModeNone) return;
	if(f == WND_AT_END) return;
	if(f == WndTopFrame) return;
	WndUnstack(f);
	f->dessous = WndTopFrame; WndTopFrame = f;
#ifdef DEBUG_STACK
	WndPrint("(WndPutAtTop) La fenetre du dessus (%08llX) devient Top Frame\n",(UInt64)f);
#endif
	WndActives++;
	if(f->passive) {
		WndPrint("(WndPutAtTop) La fenetre du dessus (%08llX) devient 'active'\n",(UInt64)f);
		WndWriteOnlyDel(f);
	}
	WndStackVerif(0,"WndPutAtTop",f);
}
/* ========================================================================== */
void WndShowTheTop() {
	WndFrame f; WndIdent w;
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef X11
	WndScreen d; XWindowAttributes etat;
	short essai_tente;
#endif

	if(WndModeNone) return;
#ifdef DEBUG_STACK
	WndPrint("(%s) TopFrame: F=%08llX\n",__func__,(uint64)WndTopFrame);
#endif
	if((f = WndTopFrame) == WND_AT_END) return;
#ifdef DEBUG_STACK
	WndPrint("(%s) TopIdent: W=%08llX\n",__func__,(uint64)(f->w));
#endif
	if(!(w = f->w)) return;
#ifdef OPENGL
	if(!WndCodeHtml) glfwShowWindow(w);
#endif
#ifdef X11
	d = (f->s)->d;
	XRaiseWindow(d,w);
//	WndEssais = 3;
//	while(WndEssais) {
//		essai_tente = WndEssais;
//		XSetInputFocus(d,w,RevertToNone,CurrentTime);  /* pour la remettre active? */
//		if(WndEssais == essai_tente) break; /* erreur BadMatch si w pas viewable */
//		else WndPrint("X11: sur XSetInputFocus(w=%08X) avec f=%08X\n",(hexa)w,(hexa)f);
//	}
	essai_tente = 8;
	while(essai_tente--) {
		XGetWindowAttributes(d,w,&etat);
		if(etat.map_state == IsViewable) {
			XSetInputFocus(d,w,RevertToParent,CurrentTime); break;
		} else usleep(10000);
	}

#endif
#ifdef WIN32
	BringWindowToTop(w);
#endif
#ifdef QUICKDRAW
	if(!WndCodeHtml) { SelectWindow(w); ShowWindow(w); }
#endif

}
/* ========================================================================== */
// void WndSendToTop(WndFrame f) { if(f) { WndPutAtTop(f); WndShowTheTop(); } }
void WndSendToTop(WndFrame f) {
	if(WndModeNone) return;
#ifdef DEBUG_STACK
	WndPrint("(WndSendToTop)     Frame: F=%08llX\n",(UInt64)WndTopFrame);
#endif
	if(f) { WndPutAtTop(f); WndShowTheTop(); }
}
#ifndef X11
/* ========================================================================== */
void WndUpdateBegin(WndIdent w) {
	if(WndModeNone) return;
#ifdef QUICKDRAW
	if(WndCodeHtml) return;
	if(w) BeginUpdate(w);
#endif
}
/* ========================================================================== */
void WndUpdateEnd(WndIdent w) {
	if(WndModeNone) return;
#ifdef QUICKDRAW
	if(WndCodeHtml) return;
	if(w) {
/*		DrawControls(w); */
		EndUpdate(w);
	}
#endif
}
#endif
/* ========================================================================== */
void WndWriteOnlySet(WndFrame f) {
	if(WndModeNone) return;
	f->dessous = WndLastWriteOnly; WndLastWriteOnly = f;
	f->passive = 1;
	WndPassives++;
}
#ifdef TIMER_NIVEAU_WND
#ifdef TIMER_PAR_EVT
#ifdef TIMER_UNIQUE
/* ========================================================================== */
void WndWriteOnlyDump() {
	WndFrame f;

	if(WndModeNone) return;
	f = WndLastWriteOnly;
	while(f != WND_AT_END) {
		WndPrint("Frame @%08X: delai=%d, restant=%d\n",(hexa)f,f->delai,f->restant);
		f = f->dessous;
	}
}
/* ========================================================================== */
static void WndWriteOnlyUpdate(void *bidon) {
/* ATTENTION: appelee dans une interruption => pas de WndPrint !!! */
	WndFrame f;

#ifdef DEBUG2
	WndPrint("(WndWriteOnlyUpdate) Update sur @%08X ?\n",WndLastWriteOnly);
#endif
	f = WndLastWriteOnly;
	while(f != WND_AT_END) {
		if(f->delai) {
			f->restant -= 1;
			if(!f->restant) {
				f->restant = f->delai;
				WndEventQueue(WND_REFRESH,f->w);
			}
		}
		f = f->dessous;
	}
	TimerContinue(WndHorloge);
}
/* ========================================================================== */
void WndWriteOnlyRefresh(WndFrame f, unsigned char delai) {
	if(WndModeNone) return;
	f->delai = f->restant = delai;
	if(!WndHorloge) {
		WndHorloge = TimerCreate(WndWriteOnlyUpdate,0);
#ifdef DEBUG2
		WndPrint("(WndWriteOnlyRefresh) Demarrage du timer @%08X\n",WndHorloge);
#endif
		TimerStart(WndHorloge,TIMER_SECONDE);
	}
}
#else /* TIMER_UNIQUE */
/* ========================================================================== */
static void WndRefreshPost(void *f) {
/*	WndRefreshed = ((WndFrame)f)->w;
	WndTimerUsed = ((WndFrame)f)->timer; */
	WndEventQueue(WND_REFRESH,((WndFrame)f)->w);
	TimerContinue(((WndFrame)f)->timer);
}
/* ========================================================================== */
void WndRefreshAuto(WndFrame f, unsigned char delai) {
	if(WndModeNone) return;
	f->timer = TimerCreate(WndRefreshPost,(void *)f);
	TimerStart(f->timer,TIMER_SECONDE,delai);
}
/* ========================================================================== */
void WndRefreshStop(WndFrame f) {
	if(WndModeNone) return;
	if(f->timer) {
		TimerStop(f->timer);
		sleep(2);
		TimerDelete(f->timer);
	}
	f->timer = 0;
}
#endif /* TIMER_UNIQUE */
#endif /* TIMER_PAR_EVT */
#endif /* TIMER_NIVEAU_WND */
/* ========================================================================== */
static char WndWriteOnlyDel(WndFrame f) {
	WndFrame courante,precedente;

	if(WndLastWriteOnly == WND_AT_END) return(0);
	if(f->dessous == WND_NOT_IN_STACK) return(1);
	precedente = 0;
	courante = WndLastWriteOnly;
	while(courante != f) {
		precedente = courante;
		courante = precedente->dessous;
		if(courante == WND_AT_END) break;
	}
	if(courante != WND_AT_END) {
		if(precedente) precedente->dessous = courante->dessous;
		else WndLastWriteOnly = courante->dessous;
		f->passive = 0;
		WndPassives--;
	}
#ifdef DEBUG
	WndPrint("(WndDelDisplay) LastDisplay @%08X\n",WndLastWriteOnly);
#endif
/*	WndStackVerif(0,"WndWriteOnlyDel",f); pas la meme pile..! */
	return(1);
}
/* ========================================================================== */
void WndMove(WndFrame f, int x, int y) {
	if(WndModeNone || WndCodeHtml) return;
#ifdef WXWIDGETS
	WndMoveWx(f->w,x,y);
#endif
#ifdef OPENGL
	glfwSetWindowPos(f->w,x,y);
#endif
}
/* ========================================================================== */
void WndClear(WndFrame f) {
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef DEBUG2
	WndPrint("Suppression frame @%08X: serveur @%08X, window @%08X\n",f,f->s,f->w);
#endif

	if(WndModeNone) return;
	if((f == WndPremiere) && (WndOpened > 1) && !WndQueryExit) { WndRaise(WndPremiere); return; }
	if(WndModeVt100) {
		int lig;
		for(lig=0; lig<f->v; lig++) 
		WndPrint("\x1b[%d;%dH\x1b[%dX",f->y+lig,f->x,f->h);
		fflush(stdout);
	} else if(!WndCodeHtml) {
		int qual,num;
		if(f == WND_AT_END) return;
		for(qual=0; qual<WND_NBQUAL; qual++) {
			for(num=0; num<f->nb_gc; num++) WndContextFree(f,f->gc[qual].coul[num]);
			if(f->gc[qual].coul) free(f->gc[qual].coul);
		}
	#if !defined(X11) && !defined(WXWIDGETS)
		if(f->image.surf) {
			if((f->image.surf)->pixels) free((f->image.surf)->pixels);
			free(f->image.surf);
		}
	#endif
		if(f->extras) free(f->extras);
	#ifdef OPENGL
		glfwDestroyWindow(f->w);
	#endif
	#ifdef X11
		if(f->image.surf) XDestroyImage(f->image.surf);
		XDestroyWindow((f->s)->d,f->w);
	#endif
	#ifdef WIN32
		DestroyWindow(f->w);
	#endif
	#ifdef QUICKDRAW
		DisposeWindow(f->w);
	#endif
	}

	WndUnstack(f);
	WndStackVerif(0,"WndClear",f);
	WndShowTheTop();

	WndOpened--;
	free(f);
	f =  WND_AT_END;
}
/* ========================================================================== */
int WndUserKeyAdd(int code, char *texte, int (*fctn)()) {
	if(WndUserKeyNb >= WND_MAXUSERKEYS) return(0);
	WndUserKey[WndUserKeyNb].code = code;
	strncpy(WndUserKey[WndUserKeyNb].texte,texte,WND_MAXREQUEST);
	WndUserKey[WndUserKeyNb].fctn = fctn;
	WndUserKeyNb++;
	return(1);
}
#ifdef OPENGL
/* ========================================================================== */
void WndEventEmpile(WndIdent w, UInt16 type, unsigned long info, Ulong when, short x, short y, UInt16 mods) {
	StructWndEventEltPile *elt; WndEvent *evt;

	elt = ( StructWndEventEltPile *)malloc(sizeof(StructWndEventEltPile));
	if(elt) {
		evt = &(elt->evt);
		evt->w = w;
		evt->what = type;
		evt->message = info;
		evt->when = when;
		evt->where.h = x;
		evt->where.v = y;
		evt->modifiers = mods;
		elt->suivant = 0;
		if(!WndEventPileHaut) WndEventPileHaut = elt;
		if(WndEventPileBas) WndEventPileBas->suivant = elt;
		WndEventPileBas = elt;
	}
}
/* ========================================================================== */
// void WndColbacCurseur(WndIdent w, double x, double y) {
// 	printf("(%s) F=%08lX (%g, %g)\n",__func__,(unsigned long)w,x,y); // correct
// }
/* ========================================================================== */
static void WndColbacKey(WndIdent w, int key, int scancode, int action, int mods) {
	if(key >= GLFW_KEY_LEFT_SHIFT) return;
	WndEventEmpile(w,(UInt16)(WND_DEV_KBD|action),(unsigned long)key,0,0,0,(UInt16)mods);
}
/* ========================================================================== */
static void WndColbacSouris(WndIdent w, int button, int action, int mods) {
	double px,py; short x,y;

	if(glfwWindowShouldClose(w)) {
		WndEventEmpile(w,(UInt16)WND_DELETE,0,0,0,0,0);
	} else {
		glfwGetCursorPos(w,&px,&py);
		x = (short)px; y = (short)py;
		// printf("(%s) Recu: %g x %g, transmis: %d x %d\n",__func__,px,py,x,y);
		//+ printf("(%s) Souris %s en %d x %d\n",__func__,(action == GLFW_PRESS)?"pressee":"relachee",x,y);
		WndEventEmpile(w,(UInt16)(WND_DEV_MSE|action),(unsigned long)button,0,x,y,(UInt16)mods);
	}
}
/* ========================================================================== */
static void WndColbacEntre(WndIdent w, int entre) {
//	void _glfwPlatformUnhideWindow(WndIdent window);
	if(entre == GL_TRUE) {
		// _glfwPlatformUnhideWindow(w);
		WndFrame courante;
		courante = WndTopFrame;
		while(courante->w != w) {
			courante = courante->dessous;
			if(courante == WND_AT_END) break;
		}
		if(courante->w == w) WndSendToTop(courante);
	}
}
/* ========================================================================== */
static void WndColbacPos(WndIdent w, int posx, int posy) {
	WndEventEmpile(w,(UInt16)(WND_DEV_MGR|WND_MOVED),0,0,posx,posy,0);
}
/* ========================================================================== */
static void WndColbacSize(WndIdent w, int sizx, int sizy) {
	WndEventEmpile(w,(UInt16)(WND_DEV_MGR|WND_RESIZE),0,0,sizx-WND_ASCINT_WIDZ,sizy-WND_ASCINT_WIDZ,0);
}
/* ========================================================================== */
static void WndColbacRefresh(WndIdent w) {
//-	if(w = WndIdRafraichie) return;
	WndEventEmpile(w,(UInt16)(WND_DEV_MGR|WND_EXPOSE),0,0,0,0,0);
//-	WndIdRafraichie = w;
}
/* ========================================================================== */
static void WndColbacClose(WndIdent w) {
	WndEventEmpile(w,(UInt16)(WND_DEV_MGR|WND_DELETE),0,0,0,0,0);
}

#endif /* OPENGL */
/* ========================================================================== */
void WndEventQueue(char type, WndIdent w) {
/* ATTENTION: peut etre appelee dans une interruption => pas de WndPrint !!! */
	WndUserRequest *u;
#ifdef QUICKDRAW
#ifdef CARBON
	CGrafPtr p;
#endif
	Rect r;
	Point origine;
#endif

	if(WndModeNone) return;
	u = (WndUserRequest *)malloc(sizeof(WndUserRequest));
	if(!u) return;
	u->type = type;
	u->w = w;
#ifdef QUICKDRAW
	if(!WndCodeHtml) {
	#ifdef CARBON
		p = WNDPORT(w);
		SetPort(p);
		GetPortBounds(p,&r);
	#else
		SetPort(WNDPORT(w));
		r = w->portRect;
	#endif
	}
	origine.h = origine.v = 0;
	LocalToGlobal(&origine);
	u->x = origine.h;
	u->y = origine.v;
	u->h = r.right - r.left;
	u->v = r.bottom - r.top;
#endif
	u->code = 0; u->texte[0] = '\0';
	if(WndRequestNext == WND_NOREQUEST) WndRequestNext = u;
	if(WndRequestLast != WND_NOREQUEST) WndRequestLast->suivant = u;
	WndRequestLast = u;
	WndRequestLast->suivant = WND_NOREQUEST;
#ifdef DEBUG2
	WndPrint("Evt ajoute type %d sur @%08X: @%08X (premier @%08X)\n",
		type,w,u,WndRequestNext);
#endif
}
/* ========================================================================== */
static short WndEventUse(WndUserRequest *u) {
#ifdef DEBUG2
	WndPrint("(WndEventUse) Evt poste @%08X type %d\n",WndRequestNext,WndRequestNext->type);
#endif
	memcpy(u,WndRequestNext,sizeof(WndUserRequest));
	free(WndRequestNext);
	WndRequestNext = u->suivant;
	if(u == WndRequestLast) WndRequestLast = WND_NOREQUEST;
#ifdef DEBUG2
	WndPrint("(WndEventUse) Evt delivre type %d\n",u->type);
#endif
	return(u->type);
}
/* ========================================================================== */
static short WndEventBuild(WndEvent *e, WndUserRequest *u) {
	int l;
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return 0;
#endif
#ifdef OPENGL
	char souris; unsigned long c;
#endif
#ifdef QUICKDRAW
#ifdef CARBON
	CGrafPtr p;
#endif
	WndIdent wevt;
	Point origine;
	Rect limites,r;
	long dimensions,selection;
	int v,h,macmenu,macitem;
	MenuHandle touche;
	Str255 string;
	short ou_ca;
	char c;
#endif

#ifdef OPENGL
	u->w = e->w;
	l = 0;
	if((e->what & WND_DEV_MASK) == WND_DEV_MGR) {
		u->type = e->what & WND_DEV_ACTN;
		switch(u->type) {
		  case WND_MOVED:
			u->x = e->where.h;
			u->y = e->where.v;
			u->h = u->v = 0;
			break;
		  case WND_RESIZE:
			u->x = u->y = 0;
			u->h = e->where.h;
			u->v = e->where.v;
			break;
		  default:
			u->x = u->y = u->h = u->v = 0;
		}
		u->code = 0;
	} else {
		souris = ((e->what & WND_DEV_MASK) == WND_DEV_MSE);
		e->what = e->what & WND_DEV_ACTN;
		switch(e->what) {
		//! Definitions des cles dans GLFW/glfw3.h
		  case GLFW_PRESS:
			if(souris) {
				u->type = WND_PRESS;
				u->x = e->where.h;
				u->y = e->where.v;
				//+ printf("(%s) Souris pressee en %d x %d\n",__func__,u->x,u->y);
				u->h = u->v = 0;
				u->code = (unsigned int)e->message;
				if(e->modifiers == GLFW_MOD_CONTROL) {
					if(u->code == WND_MSELEFT) u->code = WND_MSERIGHT;
					else if(u->code == WND_MSERIGHT) u->code = WND_DOUBLE;
				}
			} else /* clavier */ {
				u->type = WND_KEY;
				u->x = u->y = u->h = u->v = 0;
				c = e->message;
				switch(c) {
				  case GLFW_KEY_HOME:      u->code = XK_Home;  break;
				  case GLFW_KEY_ESCAPE:    u->code = XK_KP_F4; break;
				  case GLFW_KEY_LEFT:      u->code = XK_Left;  break;
				  case GLFW_KEY_RIGHT:     u->code = XK_Right; break;
				  case GLFW_KEY_UP:        u->code = XK_Up;    break;
				  case GLFW_KEY_DOWN:      u->code = XK_Down;  break;
				  case GLFW_KEY_BACKSPACE: u->code = XK_ASCII; u->texte[0] = 0x08; l = 1; break;
				  case GLFW_KEY_TAB:       u->code = XK_ASCII; u->texte[0] = 0x09; l = 1; break;
				  case GLFW_KEY_ENTER:     u->code = XK_ASCII; u->texte[0] = 0x0D; l = 1; break;
				  case GLFW_KEY_DELETE:    u->code = XK_ASCII; u->texte[0] = 0x7F; l = 1; break;
				  default:
					/* e->modifiers contient l'etat de Cntl, Option, Command, CapsLock et Shift */
					if((c >= GLFW_KEY_KP_0) && (c <= GLFW_KEY_KP_9)) c = c - GLFW_KEY_KP_0 + GLFW_KEY_0;
					if((c >= GLFW_KEY_A) && (c <= GLFW_KEY_Z)) {
						if(e->modifiers == GLFW_MOD_CONTROL) c -= 0x40;
						else if(e->modifiers != GLFW_MOD_SHIFT) c += 0x20;
					} else if((c < 127) && (e->modifiers == GLFW_MOD_SHIFT)) c = WndShifted[c];
					if(e->modifiers == GLFW_MOD_SUPER) {
						u->code = XK_Alt_L;
						if(c == 'q') { WndQueryExit = 1; break; }
					} else u->code = XK_ASCII;
					u->texte[0] = c; l = 1;
					break;
				}
			}
			break;
		  case GLFW_RELEASE:
			u->type = souris? WND_RELEASE: WND_NOKEY;
			u->x = e->where.h;
			u->y = e->where.v;
			//+ printf("(%s) Souris relachee en %d x %d\n",__func__,u->x,u->y);
			u->h = u->v = 0;
			u->code = e->message;
			break;
		}
	}
	u->texte[l] = '\0';
#endif

#ifdef X11
	u->type = e->type;
  	// WndPrint("(%s) Evt recu: %s [type %d] en (%d, %d) \n",__func__,WndEventName[e->type],e->type,e->xbutton.x,e->xbutton.y);
  	// if((e->type == WND_PRESS) || (e->type == WND_RELEASE)) WndPrint("(%s) Evt recu: %s #%d en (%d, %d)\n",__func__,WndEventName[e->type],e->xbutton.button,e->xbutton.x,e->xbutton.y);
	switch(e->type) {
	  case WND_EXPOSE:
		if(e->type == DestroyNotify) u->type = WND_DELETE;
		u->w = e->xexpose.window;
		u->x = e->xexpose.x;
		u->y = e->xexpose.y;
		u->h = e->xexpose.width - WND_ASCINT_WIDZ;
		u->v = e->xexpose.height - WND_ASCINT_WIDZ;
		u->code = 0; u->texte[0] = '\0';
		if(u->x < WND_XMIN) u->x = WND_XMIN;
		if(u->y < WND_YMIN) u->y = WND_YMIN;
		break;
	  case WND_CONFIG:
		if(e->type == DestroyNotify) u->type = WND_DELETE;
		u->w = e->xconfigure.window;
		u->x = e->xconfigure.x;
		u->y = e->xconfigure.y;
		u->h = e->xconfigure.width - WND_ASCINT_WIDZ;
		u->v = e->xconfigure.height - WND_ASCINT_WIDZ;
		u->code = 0; u->texte[0] = '\0';
		if(u->x < WND_XMIN) u->x = WND_XMIN;
		if(u->y < WND_YMIN) u->y = WND_YMIN;
		break;
	  case WND_IN_USE:
		u->w = e->xfocus.window;
		break;
	  case WND_ERASE:
		// WndPrint("(WndEventBuild) Evt recu: %s (type %d), transforme en DELETE\n",WndEventName[e->type],e->type);
		// evenement recu lors de la fermeture propre ('quitter' ou 'fermer')
		u->type = WND_DELETE;
		u->w = e->xunmap.window;
		u->x = u->y = u->h = u->v = 0;
		break;
	  case WND_DELETE:
		// WndPrint("(WndEventBuild) Evt recu: %s (type %d), declare DELETE\n",WndEventName[e->type],e->type);
		u->w = e->xdestroywindow.window;
		u->x = u->y = u->h = u->v = 0;
		break;
	  case WND_KEY:
		u->w = e->xkey.window;
		/* u->x = e->xkey.x; u->y = e->xkey.y; */
		u->x = u->y = 0;
		u->h = u->v = 0;
		l = XLookupString((XKeyEvent *)e,u->texte,WND_MAXREQUEST,(KeySym *)&(u->code),NULL);
		u->texte[l] = '\0';
		break;
	  case WND_NOKEY:
		u->w = e->xkey.window;
		u->x = u->y = 0;
		u->h = u->v = 0;
		break;
	  case WND_PRESS:
	  case WND_RELEASE:
		  u->w = e->xbutton.window;
		  u->x = e->xbutton.x;
		  u->y = e->xbutton.y;
		  u->h = u->v = 0;
		  u->code = e->xbutton.button;
		  u->texte[0] = '\0';
		break;
	  case MotionNotify:
		u->type = WND_PRESS;
		u->w = e->xmotion.window;
		u->x = e->xmotion.x;
		u->y = e->xmotion.y;
		u->h = u->v = 0;
		u->code = e->xmotion.state;
		u->texte[0] = '\0';
		break;
	  default:
/* 		WndPrint("(WndEventBuild) Evt recu: %s (type %d), NON TRAITE\n",WndEventName[e->type],e->type); */
		u->type = WND_NOKEY;
		break;
	}
	// if(u->type == WND_DELETE) WndPrint("(%s) Evt recu: %s (type %d), declare DELETE\n",__func__,WndEventName[e->type],e->type);
/*	u->lig = WndPixToLig(u->y);
	u->col = WndPixToCol(u->x);
	u->vert = WndPixToLig(u->v);
	u->hori = WndPixToCol(u->h);
*/
#endif

#ifdef WIN32
	RECT windowPos;
	u->type = e->message;

	switch(u->type) {
	  case WND_EXPOSE:
		u->w = e->hwnd;
		GetWindowRect(e->hwnd, &windowPos);
		u->x = windowPos.left;
		u->y = windowPos.top;
		u->h = windowPos.right - windowPos.left - WND_ASCINT_WIDZ;
		u->v = windowPos.bottom - windowPos.top - WND_ASCINT_WIDZ;
		u->code = 0;
		u->texte[0] = '\0';
		if(u->x < WND_XMIN) u->x = WND_XMIN;
		if(u->y < WND_YMIN) u->y = WND_YMIN;
		break;
	  case WND_CONFIG:
		u->w = e->hwnd;
		GetWindowRect(e->hwnd, &windowPos);
		u->x = windowPos.left;
		u->y = windowPos.top;
		u->h = windowPos.right - windowPos.left - WND_ASCINT_WIDZ;
		u->v = windowPos.bottom - windowPos.top - WND_ASCINT_WIDZ - WndTitleBar;
		u->code = 0; 
		u->texte[0] = '\0';
		if(u->x < WND_XMIN) u->x = WND_XMIN;
		if(u->y < WND_YMIN) u->y = WND_YMIN;
		break;
	  case WND_REDRAW:
	  case WND_IN_USE:
		u->w = e->hwnd;
		break;
	  case WND_ERASE:
		u->w = e->hwnd;
		u->x = u->y = u->h = u->v = 0;
		u->type = WND_DELETE;
		break;
	  case WND_DELETE:
		u->w = e->hwnd;
		u->x = u->y = u->h = u->v = 0;
		break;
	  case WM_KEYDOWN:
		u->w = e->hwnd;
		u->x = 0;
		u->y = 0;
		u->h = 0;
		u->v = 0;
		switch((int)e->wParam) {
		  case XK_Home:
		  case XK_KP_F4:
		  case XK_Left:
		  case XK_Right:
		  case XK_Up:
		  case XK_Down:
		  case XK_Alt_L:
			  u->type = WND_KEY;
			  u->texte[0] = '\0';
			  u->code = (unsigned int)e->wParam;
			  break;
		  default:
			  u->type = WND_NOKEY;	
		  }
		  break;
	  case WND_KEY:
		  u->w = e->hwnd;
		  u->x = 0;
		  u->y = 0;
		  u->h = 0;
		  u->v = 0;
		  u->texte[0] = (char)e->wParam;
		  u->texte[1] = '\0';
		  u->code = XK_ASCII;	
		  break;
	  case WND_NOKEY:
		u->w = e->hwnd;
		u->x = u->y = 0;
		u->h = u->v = 0;
		break;
	  case WND_PRESS:
	  case WND_RELEASE:
		  u->w = e->hwnd;
		  u->x = (int)(short)LOWORD(e->lParam); 
		  u->y = (int)(short)HIWORD(e->lParam);
		  u->h = u->v = 0;
		  u->code = (unsigned int)WND_MSELEFT;
		  u->texte[0] = '\0';
		  break;
	  default:
		u->type = (UINT)WND_NOKEY;
		break;
	}

	return(u->type);
#endif

#ifdef QUICKDRAW
#ifdef DEBUG0
	WndPrint("*** (WndEventBuild) Evenement '%s' [code %d] a %d secondes\n",WndEventName[e->what],e->what,e->when/60);
	WndPrint("                    avec u->type=%d\n",u->type);
#endif
	u->type = -1;
	u->w = (WndTopFrame != WND_AT_END)? WndTopFrame->w: 0;
#ifdef DEBUG2
	WndPrint("TopFrame @%08X (fenetre #%08X)\n",WndTopFrame,u->w);
#endif
	switch(e->what) {
	  case mouseDown:
		WndLastUpdated = 0;
		ou_ca = FindWindow(e->where,&wevt);
	#ifdef DEBUG0
		WndPrint("mouseDown en (%d, %d)= zone '%s' de %08X\n",e->where.h,e->where.v,WndMgrBouton[ou_ca],wevt);
	#endif
		u->w = wevt;
		if(ou_ca == inMenuBar) {
		#ifdef DEBUG2
			WndPrint("Choix d'un menu general: ");
		#endif
			selection = MenuSelect(e->where);
		#ifdef DEBUG2
			WndPrint(" %08X\n",selection);
		#endif
		#ifndef CW5
			if(selection == 0xB10B0009 /* kAEQuitApplication */) { WndQueryExit = 1; break; }
		#endif
			macmenu = HiWord(selection); /* menu ID */
			if(!macmenu) break;          /* on a clique a cote d'un item */
			touche = GetMenuHandle(macmenu);
			macitem = LoWord(selection); /* item # */
			GetMenuItemText(touche,macitem,string);
			string[string[0]+1] = '\0';
		#ifdef DEBUG0
			WndPrint("Menu selectionne: %d, item %d (%s)\n",macmenu,macitem,
				string[0]? (string[1]? (char *)(string+1): (string[2]? (char *)(string+2): (char *)(string+3))): "texte indisponible");
			#endif
			switch(macmenu) {
			  case MENU_POMME:
				switch(macitem) {
				  case ITEM_APROPOS:
					if(WndAProposFctn) (*WndAProposFctn)();
					break;
				  default:
				#ifdef DEBUG3
					WndPrint("Menu selectionne: %d (POMME), item #%d[%d] (%s)\n",macmenu,macitem,
						string[0],string[0]? (char *)(string+3): "texte indisponible");
					/* WndPrint("["); for(l=0; l<=string[0]; l++) {
						if(isprint(string[l])) WndPrint("%c",string[l]); else WndPrint("<%02x>",string[l]);
					}; WndPrint("]\n"); */
				#endif
				#ifndef CARBON
					OpenDeskAcc(string);
				#endif
					break;
				}
				break;
			  case MENU_FICHIER:
			#ifdef DEBUG3
				WndPrint("Menu selectionne: %d (FILE), item: %d\n",macmenu,macitem);
			#endif
				switch(macitem) {
				  case ITEM_FERMER:
				  case ITEM_QUITTER:   /* requete JM  */
					WndQueryExit = 1;
					break;
			#ifdef MENU_BARRE
				#ifndef CW5
				  case ITEM_QUITTER+2:
					WndQueryExit = 1;
					break;
				#endif
				  case ITEM_QUITTER+1:
					if(WndMenuAffiche) SetMenuBar(WndMenuStandard);
					else SetMenuBar(WndMenuAppli);
					WndMenuAffiche = 1 - WndMenuAffiche;
					DrawMenuBar();
					u->type =  WND_BARRE;
					u->x = -1;
					u->y = u->h = u->v = 0;
					u->code = 0;
					u->texte[0] = '\0';
					return(u->type);
					break;
			#endif /* MENU_BARRE */
				  default:
					break;
				}
				break;
			  case MENU_EDITER:
			#ifdef DEBUG3
				WndPrint("Menu selectionne: %d (EDIT), item: %d\n",macmenu,macitem);
			#endif
				break;
			  default:
		#ifdef MENU_BARRE
			#ifdef DEBUG3
				WndPrint("Menu selectionne: %d (default), item: %d\n",macmenu,macitem);
			#endif
				/* menu: item (macmenu-WND_BASE_ID) du menu principal (argument de MenuBarre)
				   item: item (macitem-1) */
				u->type =  WND_BARRE;
				u->y = (touche == WndMenuLast)? -1: macmenu - WND_BASE_ID;
			#ifdef DEBUG3
				WndPrint("Menuhandle=%08X/%08X, soit colonne %d (%08X-%08X)\n",
					(hexa)touche,(hexa)WndMenuLast,u->x,(hexa)macmenu,(hexa)WND_BASE_ID);
			#endif
				u->x = u->h = u->v = 0;
				u->code = macitem - 1;
				if(!string[0]) strcpy(u->texte,"(indefini)");
				else {
					if(string[0] >= WND_MAXREQUEST) string[0] = WND_MAXREQUEST - 1;
					strncpy(u->texte,(char *)(string+1),string[0]);
					u->texte[string[0]] = '\0';
				}
				HiliteMenu(0);
				return(u->type);
		#endif /* MENU_BARRE */
				break;
			}
		#ifndef CARBON
			SystemTask();
		#endif
/*			sleep(1); */
			HiliteMenu(0);
			break;
		} else if(wevt == nil) break;
	#ifdef CARBON
		p = WNDPORT(wevt); SetPort(p); GetPortBounds(p,&r);
	#else
		SetPort(WNDPORT(w)); r = wevt->portRect;
	#endif /* CARBON */
		c = ((e->modifiers & keyCodeMask) >> 8) & 0xFF;
	#ifdef DEBUG2
		WndPrint("what: %08X, msg: %08X, when: %08X, where: %08X, mods: %08X soit c=%02X\n",e->what,e->message,e->when,e->where,e->modifiers,c);
	#endif
		switch(c) {
			case 0x00: u->code = WND_MSELEFT;   break;
			case 0x01: u->code = WND_MSEMIDDLE; break; /* avec Pomme */
			case 0x02: u->code = WND_MSEMIDDLE; break; /* avec Shift */
			case 0x04: u->code = WND_MSEMIDDLE; break; /* avec Lock  */
			case 0x08: u->code = WND_MSEMIDDLE; break; /* avec Alt   */
			case 0x10: u->code = WND_MSERIGHT;  break; /* avec Ctrl  */
			default: u->code = WND_MSELEFT; break;
		}
		switch(ou_ca) {
		  case inContent:
			origine.v = origine.h = 0;
			LocalToGlobal(&origine);
			u->type = WND_PRESS;
			u->x = e->where.h - origine.h;
			u->y = e->where.v - origine.v;
			u->h = u->v = 0;
		#ifdef DEBUG2
			WndPrint("Construit evt '%s' bouton #%d\n",WndTypeName[u->type],u->code);
		#endif
			u->texte[0] = '\0';
			break;
		  case inDrag:
		#ifdef DEBUG1
			WndPrint("Deplacement de la fenetre #%08X ...",wevt); fflush(stdout);
		#endif
			/* Version 1: limites = qd.screenBits.bounds; */
			limites = (WndCurSvr->d).bounds;
			DragWindow(wevt,e->where,&limites);
		#ifdef DEBUG1
			WndPrint(" termine (bouton souris %s)\n",StillDown()? "enfonce": "relache");
		#endif
			origine.h = origine.v = 0;
			LocalToGlobal(&origine);
/*			MoveWindow(wevt,origine.h,origine.v,false); appele par DragWindow */
			u->type = WND_CONFIG;
			u->x = origine.h;
			u->y = origine.v;
			u->h = r.right - r.left - WND_ASCINT_WIDZ;
			u->v = r.bottom - r.top - WND_ASCINT_WIDZ;
			u->texte[0] = '\0';
			break;
		  case inGrow:
			/* Version 1: limites = qd.screenBits.bounds; */
			limites = (WndCurSvr->d).bounds;
			dimensions = GrowWindow(wevt,e->where,&limites);
			v = HiWord(dimensions); h = LoWord(dimensions);
			SizeWindow(wevt,h,v,false);
			origine.h = origine.v = 0;
			LocalToGlobal(&origine);
			u->type = WND_RESIZE /* WND_CONFIG */;
			u->x = origine.h;
			u->y = origine.v;
			u->h = h - WND_ASCINT_WIDZ;
			u->v = v - WND_ASCINT_WIDZ;
			u->texte[0] = '\0';
			break;
		  case inGoAway:
			if(TrackGoAway(wevt,e->where)) u->type = WND_DELETE;
			break;
		  case inZoomIn: case inZoomOut:
			if(TrackBox(wevt,e->where,ou_ca)) {
				ZoomWindow(wevt,ou_ca,true);
				origine.h = origine.v = 0;
				LocalToGlobal(&origine);
				u->type = WND_CONFIG;
				u->x = origine.h;
				u->y = origine.v;
				u->h = r.right - r.left - WND_ASCINT_WIDZ;
				u->v = r.bottom - r.top - WND_ASCINT_WIDZ;
				u->texte[0] = '\0';
			} else {
				u->type = -1;
			}
			break;
//		  case inMenuBar: deja traite hors switch (wevt == nil)
		#ifndef CARBON
		  case inSysWindow:
			SystemClick(e,wevt);
			break;
		#endif
		}
		break;
	  case mouseUp:
		WndLastUpdated = 0;
		ou_ca = FindWindow(e->where,&wevt);
	#ifdef DEBUG2
		WndPrint("Fin clic en (%d, %d)= zone '%s' de %08X\n",e->where.h,e->where.v,WndMgrBouton[ou_ca],wevt);
	#endif
		u->w = wevt;
		if(wevt == nil) break;
		SetPort(WNDPORT(wevt));
		c = ((e->modifiers & keyCodeMask) >> 8) & 0xFF;
		switch(c) {
		  case 0x00: u->code = WND_MSELEFT;   break;
		  case 0x01: u->code = WND_MSEMIDDLE; break; /* avec Pomme */
		  case 0x02: u->code = WND_MSEMIDDLE; break; /* avec Shift */
		  case 0x04: u->code = WND_MSEMIDDLE; break; /* avec Lock  */
		  case 0x08: u->code = WND_MSEMIDDLE; break; /* avec Alt   */
		  case 0x10: u->code = WND_MSERIGHT;  break; /* avec Ctrl  */
		  default: u->code = WND_MSELEFT; break;
		}
		switch(ou_ca) {
		  case inContent:
			origine.v = origine.h = 0;
			LocalToGlobal(&origine);
			u->type =  WND_RELEASE;
			u->x = e->where.h - origine.h;
			u->y = e->where.v - origine.v;
			u->h = u->v = 0;
			u->texte[0] = '\0';
		#ifdef DEBUG2
			WndPrint("Construit evt '%s' bouton #%d\n",WndTypeName[u->type],u->code);
		#endif
			break;
		}
		break;
	  case keyUp:   u->type = WND_NOKEY; u->x = u->y = u->h = u->v = 0; break;
	  case autoKey: u->type = WND_KEY; break;
	  case keyDown:
		WndLastUpdated = 0;
	#ifdef DEBUG2
		WndPrint("Cle: %08X",e->message);
	#endif
		u->type = WND_KEY;
		u->x = u->y = u->h = u->v = 0;
		c = e->message & charCodeMask;
		l = 0;
		switch(c) {
		  case kFunctionKeyCharCode:   u->code = XK_Home;  break;
		  case kClearCharCode:         u->code = XK_KP_F4; break;
		  case kLeftArrowCharCode:     u->code = XK_Left;  break;
		  case kRightArrowCharCode:    u->code = XK_Right; break;
		  case kUpArrowCharCode:       u->code = XK_Up;    break;
		  case kDownArrowCharCode:     u->code = XK_Down;  break;
		  default:
			u->texte[0] = c;
			l = 1;
		/* e->modifiers contient l'etat de Cntl, Option, Command, CapsLock et Shift */
			  if(e->modifiers & cmdKey) {
				  u->code = XK_Alt_L;
				  if(c == 'q') { WndQueryExit = 1; break; }
			  } else u->code = XK_ASCII;
			break;
		}
		u->texte[l] = '\0';
	#ifdef DEBUG2
		if(isprint(c)) WndPrint(" '%c'\n",c); else WndPrint(" \"%02X\"\n",c);
	#endif
		break;
	  case updateEvt:
	#ifdef DEBUG2
		WndPrint("Mise a jour fenetre #%08X\n",e->message);
	#endif
		if((wevt = (WindowPtr)e->message) == WndLastUpdated) {
	#ifdef DEBUG1
			WndPrint("Fenetre deja mise a jour\n");
	#endif
			return(-1);
		}
		u->w = WndLastUpdated = wevt;
	#ifdef CARBON
		p = WNDPORT(wevt);
		SetPort(p);
		GetPortBounds(p,&r);
	#else
		SetPort(WNDPORT(wevt));
		r = wevt->portRect;
	#endif
		origine.h = origine.v = 0;
		LocalToGlobal(&origine);
		u->type = WND_EXPOSE;
		u->x = origine.h;
		u->y = origine.v;
		u->h = r.right - r.left - WND_ASCINT_WIDZ;
		u->v = r.bottom - r.top - WND_ASCINT_WIDZ;
		u->code = 0; u->texte[0] = '\0';
		break;
	#ifdef DEBUG0
	  default:
	  	WndPrint("(WndEventBuild) Event type %d ('%s') non traite\n",e->what,WndEventName[e->what]);
	#endif
	}
#ifdef DEBUG2
	WndPrint("Construit evt sur fenetre #%08X (%08X)\n",u->w,wevt);
#endif
#endif

	if(u->type == WND_KEY) {
		for(l=0; l<WndUserKeyNb; l++) {
			if((WndUserKey[l].code == u->code) && !strcmp(WndUserKey[l].texte,u->texte)) break;
		}
		if(l < WndUserKeyNb) {
		#ifdef DEBUG1
			WndPrint("Fonction executee directement via <Pomme-%s>\n",u->texte);
		#endif
			(*WndUserKey[l].fctn)();
			return(-1);
		}
	}
#ifdef DEBUG1
	if((u->type >= 0) && (u->type < 12)) WndPrint("Retourne evt '%s' code %d\n",WndTypeName[u->type],u->code);
	//else if(u->type == 1) WndPrint("Non-evenement!\n");
#endif
	return(u->type);
}
/* ========================================================================== */
static int WndEventNext(WndEvent *e) {
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return 0;
#endif
#ifdef OPENGL
	StructWndEventEltPile *elt;

	if(!WndEventPileHaut) glfwPollEvents();
	if(!WndEventPileHaut) return(0);
	elt = WndEventPileHaut;
	memcpy(e,&(elt->evt),sizeof(WndEvent));
	WndEventPileHaut = elt->suivant;
	if(!WndEventPileHaut) WndEventPileBas = 0;
	free(elt);
	return(1);
#endif

#ifdef X11
	return(XCheckMaskEvent(WndCurSvr->d,WndEventMask,e));
#endif

#ifdef WIN32
	int rc = PeekMessage(e, 0, 0, 0, PM_REMOVE);
	if(rc) {
		TranslateMessage(e);
		DispatchMessage(e);
	}
	return(rc);
#endif

#ifdef QUICKDRAW
	return(GetNextEvent(WndEventMask,e));
#endif
}
/* ========================================================================== */
static char WndEventMouse(WndUserRequest *u, int *posx, int *posy, int *evt_recu, WndEvent *e) {
	char souris_pressee = 0;
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return 0;
#endif
#ifdef OPENGL
	souris_pressee = 1;
	if((*evt_recu = WndEventNext(e))) {
		souris_pressee = (((e->what & WND_DEV_MASK) != WND_DEV_MSE) || ((e->what & WND_DEV_ACTN) != GLFW_RELEASE));
		if(souris_pressee) {
			*posx = e->where.h;
			*posy = e->where.v;
		}
	}
#endif

#ifdef X11
	if((*evt_recu = XCheckMaskEvent(WndCurSvr->d,WndEventMask,e))) {
		if(e->type == MotionNotify) {
			*posx = e->xmotion.x;
			*posy = e->xmotion.y;
			souris_pressee = 1;
		}
	}
#endif

#ifdef WIN32
	if(*evt_recu = PeekMessage(e, 0, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) {
		TranslateMessage(e);
		DispatchMessage(e);
		if(e->message == WM_MOUSEMOVE) {
			*posx = (int)(short)LOWORD(e->lParam);
			*posy = (int)(short)HIWORD(e->lParam);
			souris_pressee = 1;
		}
	}
#endif

#ifdef QUICKDRAW
	Point pos;

	if(StillDown()) {
		GetMouse(&pos);
		*posx = pos.h;
		*posy = pos.v;
		souris_pressee = 1;
	} else {
		*evt_recu = GetNextEvent(WndEventMask,e);
		if(*evt_recu) souris_pressee = 0;
		else /* coup fourre chez Apple, StillDown() rend faux si clic-droit!!! */ {
			GetMouse(&pos);
			*posx = pos.h;
			*posy = pos.v;
			souris_pressee = 1;
			*evt_recu = 1; /* en fait aurait du rester inchange */
		}
	}
#endif
	return(souris_pressee);
}
/*-- #define REPEAT_KEY */
/* ========================================================================== */
int WndEventNew(WndUserRequest *u, WND_EVENT_MODE mode) {
	WndEvent e; int type,x,y,posx,posy;
	int nbwaits,rc;
	char souris_pressee;

	rc = 0; /* gcc */
	posx = posy = 0; /* gcc */
	if(WndModeNone) { u->type = WND_EXPOSE; u->w = 0; return(1); }
	if(WndRequestNext != WND_NOREQUEST) { WndEventUse(u); return(1); }
	memcpy(&WndEventPrecedent,u,sizeof(WndUserRequest));

	souris_pressee = 0;
	nbwaits = 0; /* gcc */
	type = -1;
#ifdef DEBUG1
	printf("----------------------------------------- %03d\n",WndEventNum++);
#endif
	do {
	#ifdef QUICKDRAW
	#ifndef CARBON
		SystemTask();
	#endif
	#endif /* QUICKDRAW */
		if((u->type != WND_PRESS)
		#ifdef WND_SCROLLX
		   && (u->type != WND_SCROLLX) 
		#endif
		#ifdef WND_SCROLLY
		   && (u->type != WND_SCROLLY)
		#endif
		   ) {

		#ifdef REPEAT_KEY
			if(u->type == WND_KEY) {
				WndPrint("Avant, une touche etait enfoncee\n");
				nbwaits = WndNbWaits;
				do {
					rc = WndEventNext(&e);
					if(rc) break; else if(nbwaits > 1) {
						#ifdef QUICKDRAW
						#ifndef CARBON
						SystemTask();
						#endif
						#endif /* QUICKDRAW */
						TimerMilliSleep(WndWaitMin);
					}
				} while(--nbwaits);
				WndPrint("On termine avec nbwaits=%d et rc=%d\n",nbwaits,rc);
				if(!rc) return; /* genere le meme evenement si la touche reste enfoncee */
			} else
		#endif /* REPEAT_KEY */

				rc = WndEventNext(&e);
			// WndPrint("(WndEventNew) WndEventNext rend %d et type=%d\n",rc,e.type);
			if(rc) {
			#ifdef AVEC_SIOUX
				char peut_etre_sioux; WndEvent wevt;
				peut_etre_sioux = 1;
				if(e.what == mouseDown) {
					if(FindWindow(e.where,&wevt) == inMenuBar) peut_etre_sioux = 0;
				}
				if(peut_etre_sioux) {
					if(SIOUXHandleOneEvent(&e)) {
						if(mode == WND_WAIT) continue; else return(0);
					}
				}
			#endif /* AVEC_SIOUX */
				type = WndEventBuild(&e,u);
			#ifdef DEBUG1
				if((u->type >= 0) && (u->type < 12)) WndPrint("Recu 1er  evt '%s' code %d en mode %s\n",WndTypeName[u->type],u->code,mode?"attente":"verif");
			#endif
			#ifdef DEBUG0
				if(u->type != -1) WndPrint("(WndEventNew) Recu '%s'=%d @(%d, %d) + (%d, %d)\n",
				#ifdef QUICKDRAW
					WndTypeName[u->type],u->type,u->x,u->y,u->h,u->v);
				#else
					WndEventName[u->type],u->type,u->x,u->y,u->h,u->v);
				#endif /* QUICKDRAW */
			#endif /* DEBUG0 */
				if(mode == WND_WAIT) {
					if(type == WND_NOKEY) continue;
					if(WndQueryExit) break;
				}
			} else if(WndRequestNext != WND_NOREQUEST) {
				type = WndEventUse(u); if(mode == WND_CHECK) rc = 1;
			}
			if((type == WND_NOKEY) && (mode == WND_CHECK)) rc = 0;
			else if(type == WND_PRESS) {
				souris_pressee = 1;
				nbwaits = 2 * WndNbWaits; // nbwaits = 1 deja essaye pour acq sous It, sans succes
			} else souris_pressee = 0;
		} else { u->x += u->h; u->y += u->v; souris_pressee = 1; nbwaits = 1; }
		if(souris_pressee) {
//#define DEBUG1
			x = u->x; y = u->y;
			do {
				souris_pressee = WndEventMouse(u,&posx,&posy,&rc,&e);
			#ifdef DEBUG1
				WndPrint("Plus que %d tests a faire, souris %s avec rc=%d\n",nbwaits,souris_pressee?"pressee":"lachee",rc);
			#endif
				if(souris_pressee) {
					if(nbwaits > 1) {
						#ifdef QUICKDRAW
						#ifndef CARBON
						SystemTask();
						#endif
						#endif
						TimerMilliSleep(WndWaitMin);
					} else {
						u->h = posx - x;
						u->v = posy - y;
					#ifdef DEBUG1
						WndPrint("Dernier test souris: rc=%d +(%d, %d)\n",rc,u->h,u->v);
					#endif
						if(mode == WND_CHECK) return(1);
						/* if((u->h != 0) || (u->v != 0)) return(1); */
					}
				} else if(rc) {
					WndEventBuild(&e,u);
				#ifdef DEBUG1
					if((u->type >= 0) && (u->type < 12)) WndPrint("Recu 2eme evt '%s' code %d\n",WndTypeName[u->type],u->code);
				#endif
				#ifdef DEBUG0
					if(u->type == -1) WndPrint("(WndEventNew) rc=%d (non-evenement)\n",rc);
					else WndPrint("(WndEventNew) rc=%d -> Recu aussi '%s'=%d @(%d, %d) + (%d, %d)\n",
					#ifdef QUICKDRAW
						rc,WndTypeName[u->type],u->type,u->x,u->y,u->h,u->v);
					#else
						rc,WndEventName[u->type],u->type,u->x,u->y,u->h,u->v);
					#endif
				#endif
					u->h = u->x - x;
					u->v = u->y - y;
					/* --	u->x = x; u->y = y; */
					break;
				} else if(mode == WND_CHECK) break;
			#ifdef DEBUG1
				WndPrint("Encore %d attentes\n",nbwaits);
			#endif
			} while(--nbwaits);
			break;
//#undef DEBUG1
		} else if(type != -1) break;
		if(mode == WND_WAIT) TimerMilliSleep(WndWaitMin);
	} while(mode == WND_WAIT);
	WndSourisPressee = souris_pressee;
#ifdef DEBUG0
	WndPrint("(WndEventNew) rc=%d -> Rendu '%s'=%d @(%d, %d) + (%d, %d)\n",
	#ifdef QUICKDRAW
		rc,(u->type==-1)?"neant":WndTypeName[u->type],u->type,u->x,u->y,u->h,u->v);
	#else
		rc,(u->type==-1)?"neant":WndEventName[u->type],u->type,u->x,u->y,u->h,u->v);
	#endif
#endif
#ifdef DEBUG1
	if(rc) {
		if((u->type >= 0) && (u->type < 12)) WndPrint("Signale evt '%s' code %d\n",WndTypeName[u->type],u->code);
		// else WndPrint("Signale evt de type #%d, code %d\n",u->type,u->code);
	}
#endif
	return(rc);
}
#ifdef QUICKDRAW
/* ========================================================================== */
char WndUpdateFlushed(WndUserRequest *u) {
	WndEvent e;

	if(WndModeNone) return(1);
	if(CheckUpdate(&e)) { WndEventBuild(&e,u); return(0); }
	else return(1);
}
#endif /* QUICKDRAW */
/* ========================================================================== */
int WndEventReady(WndUserRequest *u) {
	WndEvent e;
	int rc;
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return 0;
#endif
	if(WndModeNone) return(0);
	if(WndRequestNext != WND_NOREQUEST) { WndEventUse(u); rc = 1; }
	else {
		rc = 1;
	#ifdef OPENGL
		while(WndEventNext(&e)) usleep(WndMgrTimeOut);
	#endif
	#ifdef X11
		rc = XCheckMaskEvent(WndCurSvr->d,WndEventMask,&e);
	#endif
	#ifdef WIN32
		rc = PeekMessage(&e, 0, 0, 0, PM_REMOVE);
		if(rc) {
			TranslateMessage(&e);
			DispatchMessage(&e);
		}
	#endif
	#ifdef QUICKDRAW
		rc = WaitNextEvent(0 /* WndEventMask */ ,&e,WndMgrTimeOut,nil);
		rc = GetNextEvent(WndEventMask,&e);
	#endif
		if(rc) {
			WndEventBuild(&e,u);
			if(u->type == -1) rc = 0;
		}
	}
#ifdef DEBUG0
	WndPrint("(%s) rc=%d -> Rendu '%s'=%d @(%d, %d) + (%d, %d)\n",__func__,
	#ifdef QUICKDRAW
		rc,(u->type==-1)?"neant":WndTypeName[u->type],u->type,u->x,u->y,u->h,u->v);
	#else
		rc,(u->type==-1)?"neant":WndEventName[u->type],u->type,u->x,u->y,u->h,u->v);
	#endif /* QUICKDRAW */
#endif
	return(rc);
}
/* ========================================================================== */
int WndEventWait(int evt, WndUserRequest *u) {
	WndEvent e;
	int rc;
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return 0;
#endif
	if(WndModeNone) return(0);
#ifdef OPENGL
	rc = 0;
#endif

#ifdef X11
	rc = XCheckTypedEvent(WndCurSvr->d,evt,&e);
#endif

#ifdef WIN32
	rc = PeekMessage(&e, 0, evt, evt, PM_REMOVE);

	if(rc)
	{
		TranslateMessage(&e);
		DispatchMessage(&e);
	}
#endif

#ifdef QUICKDRAW
	rc = WaitNextEvent(WndEventMask,&e,WndMgrTimeOut,nil);
	rc = (e.what == evt);
#endif

	if(rc) WndEventBuild(&e,u);
	return(rc);
}
/* ========================================================================== */
void WndSend(WndFrame f, int type, int x, int y, int h, int v) {
#ifdef X11
	WndEvent e;

	if(WndModeNone) return;
	switch(type) {
		case WND_EXPOSE:
		e.xexpose.type = type;
		e.xexpose.window = f->w;
		e.xexpose.x = x;
		e.xexpose.y = y;
		e.xexpose.width = h;
		e.xexpose.height = v;
		break;
		case WND_CONFIG:
		e.xconfigure.type = type;
		e.xconfigure.window = f->w;
		e.xconfigure.x = x;
		e.xconfigure.y = y;
		e.xconfigure.width = h;
		e.xconfigure.height = v;
		break;
		case WND_DELETE:
		e.xdestroywindow.type = type;
		e.xdestroywindow.window = f->w;
		break;
		case WND_KEY:
		e.xkey.type = type;
		e.xkey.window = f->w;
		e.xkey.x = x;
		e.xkey.y = y;
		e.xkey.state = h;
		e.xkey.keycode = v;
		break;
		case WND_PRESS:
		case WND_RELEASE:
		e.xbutton.type = type;
		e.xbutton.window = f->w;
		e.xbutton.x = x;
		e.xbutton.y = y;
		e.xbutton.state = h;
		e.xbutton.button = v;
		break;
	}
	XSendEvent((f->s)->d,f->w,0,0,&e);
#endif

#ifdef WIN32
	if(WndModeNone) return;
	SendMessage(f->w, type, NULL, NULL);
#endif

#ifdef QUICKDRAW
	if(WndModeNone) return;
	switch(type) {
		case WND_EXPOSE: PostEvent(updateEvt,(long)(f->w)); break;
	}
#endif
}
/* ============== Fonctions speciales (necessitent X ToolKit) =============== */
#ifdef _XtIntrinsic_h
void WndTimeOutFctn(WndFrame f, void (*fctn)(), void *arg, 
	unsigned long duree) {
	XtAppAddTimeOut(XtDisplayToApplicationContext((f->s)->d),
		duree,fctn,(XtPointer)arg); 
}
/* ........................................................................ */
char WndEventCheck(WndUserRequest *u) {
	if(XtAppPending(XtDisplayToApplicationContext((f->s)->d)) & XtIMXEvent) {
		WndEventNew(&e,WND_WAIT); return(1);
	} else return(0);
}
#else
/* =========================================================================== */
char WndEventCheck(WndUserRequest *u) {
	char rc;
	WndEvent e;

	if(WndModeNone) return(0);
	rc = 0;
#ifdef X11
	rc = XCheckMaskEvent(WndCurSvr->d,WndEventMask,&e);
#endif

#ifdef WIN32
	rc = PeekMessage(&e, 0, 0, 0, PM_REMOVE);
	TranslateMessage(&e);
	DispatchMessage(&e);
#endif

#ifdef QUICKDRAW
	rc = WaitNextEvent(WndEventMask,&e,WndMgrTimeOut,nil);
#endif
	if(rc) WndEventBuild(&e,u);
	return(rc);
}
#endif
/* ========================================================================== */
/* ========================= Amenagements de trace ========================== */
/* ========================================================================== */
WndColor *WndColorGetFromName(char *nom) {
#ifdef X11
	WndColor *c; WndScreen d; WndColor q;
#else
	char code[5]; WndColorLevel r,g,b;
#endif

	if(WndModeNone) return((WndColor *)ERREUR);
#ifdef X11
	c = (WndColor *)malloc(sizeof(WndColor));
	if(!c) return((WndColor *)ERREUR);
	d = WndCurSvr->d;
/*	{	int rc;
		rc = XLookupColor(d,DefaultColormap(d,DefaultScreen(d)),nom,&q,c);
		if(rc) {
			rc = XAllocColor(d,DefaultColormap(d,DefaultScreen(d)),c);
			if(rc) return(c);
		}
		return((WndColor *)ERREUR);
	} */
	if(XAllocNamedColor(d,DefaultColormap(d,DefaultScreen(d)),nom,c,&q))
		return(c);
	else { free(c); return((WndColor *)ERREUR); }
#else
	code[4] = '\0';
	strncpy(code,nom,4);   sscanf(code,"%hx",&r);
	strncpy(code,nom+4,4); sscanf(code,"%hx",&g);
	strncpy(code,nom+8,4); sscanf(code,"%hx",&b);
	return(WndColorGetFromRGB(r,g,b));
#endif
}
/* ========================================================================== */
WndColor *WndColorGetFromRGB(WndColorLevel r, WndColorLevel g, WndColorLevel b) {
	WndColor *c;

	WndColor nouvelle; int k,l;
	if(WndModeNone) return((WndColor *)ERREUR);
#ifdef WXWIDGETS
	nouvelle.red = r; nouvelle.green = g; nouvelle.blue = b;
#endif
#ifdef OPENGL
	nouvelle.red = r; nouvelle.green = g; nouvelle.blue = b;
#endif
	l = -1;
	for(k=0; k<WndColorNb; k++) {
		if(WndColorStock[k].active) {
			if(!bcmp(&(WndColorStock[k].c),&nouvelle,sizeof(WndColor))) break;
		} else if(l < 0) l = k;
	}
	if(k == WndColorNb) {
		if(l >= 0) k = l;
		else {
			if(WndColorNb >= WND_COLOR_NB) return(0);
			WndColorNb++;
		}
		memcpy(&(WndColorStock[k].c),&nouvelle,sizeof(WndColor));
		WndColorStock[k].active = 1;
	}
	c = &(WndColorStock[k].c);
	return(c);
}
/* ========================================================================== */
void WndColorFree(WndColor *c) {
#ifndef X11
	int k;
#endif

	if(WndModeNone) return;
#ifdef X11
	if(c) free(c); return;
#else
	for(k=0; k<WndColorNb; k++) {
		if(!bcmp(&(WndColorStock[k]),c,sizeof(WndColor))) break;
	}
	if(k < WndColorNb) WndColorStock[k].active = 0;
#endif
}
/* ========================================================================== */
char WndColorSetByName(WndColor *c, char *nom) {
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return 0;
#endif
#ifdef OPENGL
	char code[5];

	code[4] = '\0';
	strncpy(code,nom,4);   sscanf(code,"%hx",&(c->red));
	strncpy(code,nom+4,4); sscanf(code,"%hx",&(c->green));
	strncpy(code,nom+8,4); sscanf(code,"%hx",&(c->blue));
	return(1);
#endif
#ifdef X11
	WndScreen d; WndColor q;

	if(WndModeNone) return(1);
	d = WndCurSvr->d;
	if(XAllocNamedColor(d,DefaultColormap(d,DefaultScreen(d)),nom,c,&q))
		return(1);
	else return(0);
#endif

#ifdef WIN32
	char code[5];
	long r, g ,b;

	code[4] = '\0';
	strncpy(code,nom,4);   sscanf(code,"%hx",&r);
	strncpy(code,nom+4,4); sscanf(code,"%hx",&g);
	strncpy(code,nom+8,4); sscanf(code,"%hx",&b);
	*c = RGB(r>>8, g>>8, b>>8);
	return (1);
#endif

#ifdef QUICKDRAW
	char code[5];

	code[4] = '\0';
	strncpy(code,nom,4);   sscanf(code,"%hx",&(c->red));
	strncpy(code,nom+4,4); sscanf(code,"%hx",&(c->green));
	strncpy(code,nom+8,4); sscanf(code,"%hx",&(c->blue));
	return(1);
#endif
}
/* ========================================================================== */
char WndColorSetByRGB(WndColor *c, WndColorLevel r, WndColorLevel g, WndColorLevel b) {
#ifdef WXWIDGETS
	c->red = r; c->green = g; c->blue = b;
#endif
#ifdef OPENGL
	c->red = r; c->green = g; c->blue = b;
#endif
	return (1);
}
/* ========================================================================== */
WndContextPtr WndContextCreate(WndFrame f) {
/* Cree ab nihilo un contexte graphique utilisable par <f> */
	if(f) return(WndContextSupportCreate(f,f->qualite));
	else return(WndContextSupportCreate(0,WndQual));
}
/* ========================================================================== */
WndContextPtr WndContextSupportCreate(WndFrame f, int qual) {
	/* Cree ab nihilo un contexte graphique utilisable par <f> */
	WndServer *s; WndContextVal *gcval; WndContextPtr gc;
	if(WndModeNone) return((WndContextPtr)1);
#ifdef OPENGL
	gcval = gc = (WndContextPtr)malloc(sizeof(WndContextVal));
#endif
#ifdef WXWIDGETS
	gcval = gc = (WndContextPtr)malloc(sizeof(WndContextVal));
#endif

	if(!gcval) return(0);

	if(f) { s = f->s; gcval->font = (s->fonte).id; }
	else { s = 0; gcval->font = (WndCurSvr->fonte).id; } // s=0 pour le compilo
	gcval->line_width = (qual == WND_Q_PAPIER)? 3: 1;

#ifdef OPENGL
	gcval->foreground = WndColorText[qual];
	gcval->background = WndColorBgnd[qual];
	gcval->line_style = 0 /* LineSolid */ ;
#endif

#ifdef WXWIDGETS
	gcval->foreground = WndColorText[qual];
	gcval->background = WndColorBgnd[qual];
	gcval->line_style = 0 /* LineSolid */ ;
#endif
	return(gc);
}
/* ========================================================================== */
WndContextPtr WndContextCreateFromVal(WndFrame f, WndContextVal *gcval) {
/* Cree un contexte graphique a partir de valeurs deja memorisees */
	WndContextPtr gc;
	if(WndModeNone) return((WndContextPtr)1);
#ifdef OPENGL
	gc = (WndContextPtr)malloc(sizeof(WndContextVal));
	if(gc) memcpy(gc,gcval,sizeof(WndContextVal));
#endif
#ifdef WXWIDGETS
	gc = (WndContextPtr)malloc(sizeof(WndContextVal));
	if(gc) memcpy(gc,gcval,sizeof(WndContextVal));
#endif
	return(gc);
}
/* ========================================================================== */
void WndContextCopy(WndFrame f, WndContextPtr gc_src, WndContextPtr gc_dest) {
	if(WndModeNone) return;
#ifdef WXWIDGETS
	memcpy(gc_dest,gc_src,sizeof(WndContextVal));
#endif
#ifdef OPENGL
	memcpy(gc_dest,gc_src,sizeof(WndContextVal));
#endif
	return;
}
/* ========================================================================== */
void WndContextFree(WndFrame f, WndContextPtr gc) {
	if(WndModeNone) return;
#ifdef OPENGL
	if(gc) free(gc);
#endif
#ifdef WXWIDGETS
	if(gc) free(gc);
#endif

}
/* ========================================================================== */
char WndContextFgndName(WndFrame f, WndContextPtr gc, char *nom) {
	/* WndColorText[f->qualite] = attribution par defaut. Sinon, couleur reellement demandee */
	/* ATTENTION: si la couleur est allouee, a desallouer par la suite */
	WndColor *c;

	if(WndModeNone) return(1);
	if(nom[0] == '\0') c = WndColorText[f->qualite]; else c = WndColorGetFromName(nom);
	if(c == (WndColor *)ERREUR) return(0);
	if(gc == 0) gc = WND_STD;
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return 0;
#endif
#ifdef OPENGL
	gc->foreground = c;
#endif
#ifdef X11
	{
		WndScreen d;
		if(f) d = (f->s)->d; else d = WndCurSvr->d;
		XSetForeground(d,gc,c->pixel);
		if(nom[0] != '\0') WndColorFree(c); /* ???? */
	}
#endif
#ifdef WIN32
//	if(gc->foreground) {
//		if(gc->foreground != *WndColorText[f->qualite]) WndColorFree(&gc->foreground);
//	}
	gc->foreground = *c;
#endif
#ifdef QUICKDRAW
//	if(gc->foreground) {
//		if(gc->foreground != WndColorText[f->qualite]) WndColorFree(gc->foreground);
//	}
	gc->foreground = c;
#endif

	return(1);
}
/* ========================================================================== */
char WndContextFgndRGB(WndFrame f, WndContextPtr gc, 
	WndColorLevel r, WndColorLevel g, WndColorLevel b) {
	/* ATTENTION: la couleur est allouee, donc a desallouer par la suite */
	WndColor *c;

	if(WndModeNone) return(1);
	if((c = WndColorGetFromRGB(r,g,b)) == (WndColor *)ERREUR) return(0);
	if(gc == 0) gc = WND_STD;

#ifdef WXWIDGETS
	gc->foreground = c;
#endif
#ifdef OPENGL
	gc->foreground = c;
#endif

	return(1);
}
/* ========================================================================== */
char WndContextBgndRGB(WndFrame f, WndContextPtr gc, 
	WndColorLevel r, WndColorLevel g, WndColorLevel b) {
	WndColor *c; // int qual;

	if(WndModeNone) return(1);
	if((c = WndColorGetFromRGB(r,g,b)) == (WndColor *)ERREUR) return(0);
	if(gc == 0) gc = WND_STD;
#ifdef WXWIDGETS
	gc->background = c;
#endif

#ifdef OPENGL
	gc->background = c;
#endif
	return(1);
}
/* ========================================================================== */
char WndContextFgndColor(WndFrame f, WndContextPtr gc, WndColor *c) {
	/* ATTENTION: la couleur n'est PAS desallouee ni desallouable */
	if(WndModeNone) return(1);
	if(gc == 0) gc = WND_STD;
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return 0;
#endif
#ifdef OPENGL
	gc->foreground = c;
#endif
#ifdef X11
	{	WndScreen d;
		if(f) d = (f->s)->d; else d = WndCurSvr->d;
		XSetForeground(d,gc,c->pixel);
	}
#endif
#ifdef WIN32
	gc->foreground = *c;
#endif
#ifdef QUICKDRAW
	gc->foreground = c;
#endif
	
	return(1);
}
/* ========================================================================== */
char WndContextBgndColor(WndFrame f, WndContextPtr gc, WndColor *c) {
	/* ATTENTION: la couleur n'est PAS desallouee ni desallouable */
	if(WndModeNone) return(1);
	if(gc == 0) gc = WND_STD;
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return 0;
#endif
#ifdef OPENGL
	gc->background = c;
#endif
#ifdef X11
	WndScreen d;
	if(f) d = (f->s)->d; else d = WndCurSvr->d;
	XSetBackground(d,gc,c->pixel);
#endif
#ifdef WIN32
	gc->background = *c;
#endif
#ifdef QUICKDRAW
	gc->background = c;
#endif

	return(1);
}
/* ========================================================================== */
void WndContextSetColors(WndFrame f, WndContextPtr gc, WndColor *fond, WndColor *text) {
	WndContextFgndColor(f,gc,text);
	WndContextBgndColor(f,gc,fond);
}
/* ========================================================================== */
void WndContextLine(WndFrame f, WndContextPtr gc, unsigned int type, unsigned int width) {
#ifdef X11
	int trace; int l; WndScreen d;
#endif
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
	if(WndModeNone) return;
	if(WndPS) {
		fprintf(WndPS,"%d setlinewidth\n",width);
	/*	fprintf(WndPS,"<tableau> %d setdash\n",xxx,decalage); */
		return;
	}
	if(gc == 0) gc = WND_STD;
#ifdef X11
	if(f) d = (f->s)->d; else d = WndCurSvr->d;
	if(type == 0) trace = LineSolid; else trace = LineOnOffDash;
	XSetLineAttributes(d,gc,width,trace,CapRound,JoinRound);
	type--;
	if(type < WND_MAXDASH) {
		l = strlen(WndDashList[type]);
		XSetDashes(d,gc,0,WndDashList[type],l);
	}
#endif
#ifdef QUICKDRAW_OR_OPENGL
	gc->line_style = (short)type;
	gc->line_width = (short)width;
#endif
}
/* ========================================================================== */
/* ======================== Acces direct aux pixels ========================= */
#ifdef OPENGL
/* ========================================================================== */
char *WndGLerreur(GLenum rc) {
	if(rc == GL_NO_ERROR) return("pas d'erreur");
	else switch(rc) {
	  case GL_INVALID_ENUM:      return("mot-cle invalide");
	  case GL_INVALID_VALUE:     return("valeur invalide");
	  case GL_INVALID_OPERATION: return("operation invalide");
	  case GL_STACK_OVERFLOW:    return("pile debordee");
	  case GL_STACK_UNDERFLOW:   return("pile vide");
	  case GL_OUT_OF_MEMORY:     return("hors d'acces memoire");
	  case GL_TABLE_TOO_LARGE:   return ("table trop grande");
	  case GL_INVALID_FRAMEBUFFER_OPERATION: return("operation framebuffer invalide ");
//  	  case GL_TEXTURE_TOO_LARGE: return("texture too large");
	  default: return("erreur inconnue");
	}
}
#endif
#ifdef X11
/* ========================================================================== */
void WndImageDump(WndSurface surf) {
	  WndPrint("(XImage *)0x%08X.width=%d, .height=%d\n",(hexa)surf,surf->width,surf->height);
	  WndPrint("                    .xoffset=%d\n",surf->xoffset);
	  WndPrint("                    .format=%s (%d)\n",
		 (surf->format == XYBitmap)? "XYBitmap":
		 (surf->format == XYPixmap)? "XYPixmap": "ZPixmap",surf->format);
	  WndPrint("                    .data=0x%08X\n",(hexa)surf->data);
	  WndPrint("                    .byte_order=%s (%d)\n",
		(surf->byte_order == LSBFirst)? "LSBFirst": "MSBFirst",surf->byte_order);
	  WndPrint("                    .bitmap_unit=%d\n",surf->bitmap_unit);
	  WndPrint("                    .bitmap_bit_order=%s (%d)\n",
		(surf->bitmap_bit_order == LSBFirst)? "LSBFirst": "MSBFirst",surf->bitmap_bit_order);
	  WndPrint("                    .bitmap_pad=%d\n",surf->bitmap_pad);
	  WndPrint("                    .depth=%d\n",surf->depth);
	  WndPrint("                    .bytes_per_line=%d\n",surf->bytes_per_line);
	  WndPrint("                    .bits_per_pixel=%d\n",surf->bits_per_pixel);
}
#endif
/* ========================================================================== */
void WndImageOffset(WndFrame f, int dx, int dy) {
	if(WndModeNone) return;
	f->image.dx = dx; f->image.dy = dy;
}
/* ========================================================================== */
static void WndImageCree(WndFrame f, int larg, int haut, WndColor *lut, int dim) {
	int surface; unsigned char *p; int i,nb;

	// f->image.dx = f->image.dy = 0; par defaut a la creation de f. Peut avoir ete modifie par WndImageOffset
	surface = larg * haut;
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef WXWIDGETS

#elif defined X11
	WndIdent w; WndServer *s; WndScreen d;
	w = f->w; s = f->s; d = s->d;
	if(f->image.surf) XDestroyImage(f->image.surf);
	// WndPrintAttributes(__func__,f);
	// WndPrint("(%s) XGetImage(0x%08x,0x%08x,0,0,%d,%d,0x%08X,%d);\n",__func__,d,w,f->h,f->v,0xFFFFFF,ZPixmap);
	f->image.surf = XGetImage(s->d,w,f->x0,f->y0,f->xm,f->ym,0xFFFFFF,ZPixmap);
	// WndStep("(%s) WndImageDump",__func__); WndImageDump(f->image.surf);
#else /* X11 */
	#ifdef OPENGL
	{	unsigned short *t; WndColor *c;
		if(f->image.lut != lut) {
			if(f->image.nbcolors != dim) {
				if((f->image.R)) { free(f->image.R); f->image.R = 0; }
				if((f->image.G)) { free(f->image.G); f->image.G = 0; }
				if((f->image.B)) { free(f->image.B); f->image.B = 0; }
				if((f->image.A)) { free(f->image.A); f->image.A = 0; }
				f->image.nbcolors = 0;
			}
			if(lut) {
				if(!(f->image.R)) f->image.R = (unsigned short *)malloc(dim * sizeof(unsigned short));
				if(!(f->image.G)) f->image.G = (unsigned short *)malloc(dim * sizeof(unsigned short));
				if(!(f->image.B)) f->image.B = (unsigned short *)malloc(dim * sizeof(unsigned short));
				if(!(f->image.A)) f->image.A = (unsigned short *)malloc(dim * sizeof(unsigned short));
				if((t = f->image.R)) { c = lut; for(i=0; i<dim; i++,c++) *t++ = c->red; }
				if((t = f->image.G)) { c = lut; for(i=0; i<dim; i++,c++) *t++ = c->green; }
				if((t = f->image.B)) { c = lut; for(i=0; i<dim; i++,c++) *t++ = c->blue; }
				if((t = f->image.A)) { *t++ = 0; for(i=1; i<dim; i++) *t++ = 0x000F; }
			}
		}
		if(!WndPS) {
			larg = (((larg - f->image.dx - 1) / 4) + 1) * 4;
			haut -= f->image.dy;
			surface = larg * haut;
		}
	}
	#endif /* !OPENGL */
	if(WndPS) {
		larg -= f->image.dx;
		haut -= f->image.dy;
		surface = larg * haut;
	}
	f->image.lut = lut; f->image.nbcolors = dim;
	nb = (f->image.lut)? surface: 3 * surface;
	//- printf("(%s) Surface %d x %d %s LUT, soit %d octets\n",__func__,larg,haut,(f->image.lut)? "avec":"sans",nb);
	if(!(f->image.surf)) {
		f->image.surf = (WndSurface)malloc(sizeof(WndTypeSurface));
		if(f->image.surf) {
			(f->image.surf)->width = (f->image.surf)->height = 0;
			(f->image.surf)->pixels = 0;
		}
	}
	if(f->image.surf) {
		if(((f->image.surf)->width * (f->image.surf)->height) != surface) {
			if((f->image.surf)->pixels) free((f->image.surf)->pixels); (f->image.surf)->pixels = 0;
		}
		if(!(f->image.surf)->pixels) (f->image.surf)->pixels = (unsigned char *)malloc(nb * sizeof(unsigned char));
		if((f->image.surf)->pixels) {
			(f->image.surf)->width = larg;
			(f->image.surf)->height = haut;
		} else (f->image.surf)->width = (f->image.surf)->height = 0;
	#ifdef WIN32
		(f->image.surf)->hDC = BeginPaint(f->w,&((f->image.surf)->paintStruct));
	#endif
	}
#endif /* !X11 */

#ifndef WXWIDGETS
	// printf("(%s) Surface @%08lX[%d] %d x %d creee\n",__func__,(lhex)((f->image.surf)->pixels),surface,(f->image.surf)->width,(f->image.surf)->height);
	if(f->image.surf) {
		if((p = (f->image.surf)->pixels)) { i = nb; while(i--) *p++ = 0; }
	}

#endif
}
/* ========================================================================== */
void WndImageInit(WndFrame f, int larg, int haut, WndColor *lut, int dim) {

	if(WndModeNone) return;
	if(WndPS) {
	#ifdef QUICKDRAW
		WndImageCree(f,larg,haut,lut,dim);
	#endif
		fprintf(WndPS,"gsave\n");
		if(lut) {
			int r,g,b; WndColor *c; int i;
			fprintf(WndPS,"/pixel 1 string def\n");
			fprintf(WndPS,"/lut <");
			c = lut;
			for(i=0; i<dim; i++,c++) {
				if(!(i%8)) fprintf(WndPS,"\n");
			#ifndef WIN32
				r = (c->red   >> 8) & 0xFF;
				g = (c->green >> 8) & 0xFF;
				b = (c->blue  >> 8) & 0xFF;
			#else
				r = GetRValue(*c);
				g = GetGValue(*c);
				b = GetBValue(*c);
			#endif
				if((r + g + b) == 0) r = g = b = 0xFF;
				else if((r + g + b) == 0x2FD) r = g = b = 0;
				fprintf(WndPS,"%02X%02X%02X ",r,g,b);
			}
			fprintf(WndPS,"> def\n");
			fprintf(WndPS,"{lut currentfile pixel readhexstring pop 0 get 3 mul 3 getinterval} bind\n");
		}
		fprintf(WndPS," false 3 colorimage");
		fprintf(WndPS,"%d %d 8 [1 0 0 -1 0 0]\n",f->xm-f->x0,f->ym-f->y0);
		fprintf(WndPS,"%d %d moveto\n",f->image.dx,f->image.dy + WND_ASCINT_WIDZ);
		WndPSstroked = 0;
	}

#ifdef QUICKDRAW
	f->image.lut = lut; f->image.nbcolors = dim;
	SetPort(WNDPORT(f->w));
#else
	WndImageCree(f,larg,haut,lut,dim);
#endif
}
/* ========================================================================== */
static void WndImageFillVal(WndFrame f, int x, int y, int val) {
	int larg,haut; unsigned char *pixels; int k;

	if(!(f->image.surf)) return;
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifndef WXWIDGETS
	if((pixels = (f->image.surf)->pixels) == 0) return;
	larg = (f->image.surf)->width; if((x < 0) || (x >= larg)) return;
	haut = (f->image.surf)->height; if((y < 0) || (y >= haut)) return;
#ifdef OPENGL
	k = (larg * (haut - y - 1)) + x;
#else
	k = (larg * y) + x;
#endif
	if(k < (larg * haut)) *(pixels + k) = (val & 0XFF);
	else printf("(%s) pixel (%d, %d) @%d/%d\n",__func__,x,y,k,larg * haut);
#endif
}
/* ========================================================================== */
static void WndImageFill8bits(WndFrame f, int x, int y, WndCol8bits *c) {
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	int larg,haut; unsigned char *pixels; int k,n;
	if(!(f->image.surf)) return;
	if((pixels = (f->image.surf)->pixels) == 0) return;
	larg = (f->image.surf)->width; if((x < 0) || (x >= larg)) return;
	haut = (f->image.surf)->height; if((y < 0) || (y >= haut)) return;
	k = (larg * (haut - y - 1)) + x;
	if(k < (larg * haut)) {
		n = 3 * k;
		*(pixels + n++) = c->r;
		*(pixels + n++) = c->g;
		*(pixels + n++) = c->b;
	}
#endif
}
#ifndef OPENGL
/* ========================================================================== */
static void WndImageDrawColor(WndFrame f, int x, int y, WndColor *c) {
#ifdef X11
	if((f->image.surf)) XPutPixel(f->image.surf,x,y,c->pixel);
#endif
#ifdef WIN32
	if((f->image.surf)) SetPixel((f->image.surf)->hDC, x, y, *c);
#endif
#ifdef QUICKDRAW
	#ifdef CARBON
		Rect r; GetWindowPortBounds(f->w,&r);
		SetCPixel(f->x0 + x + r.left,f->y0 + y + r.top,c);
	#else
		SetCPixel(f->x0 + x + ((f->w)->portRect).left,f->y0 + y + ((f->w)->portRect).top,c);
	#endif
#endif
}
#endif
/* ========================================================================== */
void WndImagePixel(WndFrame f, int x, int y, int val) {
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
	if(WndModeNone) return;
	if(val < 0) val = 0; else if(val >= f->image.nbcolors) val = f->image.nbcolors - 1;
	if(WndPS) WndImageFillVal(f,x - f->image.dx,y - f->image.dy,val);
	else {
	#ifdef OPENGL
		WndImageFillVal(f,x - f->image.dx,y - f->image.dy,val);
	#else
		if(f->image.lut) {
			WndColor *c;
			c = f->image.lut + val;
			WndImageDrawColor(f,x,y,c);
		}
	#endif
	}
}
/* ========================================================================== */
void WndIconeInit(WndFrame f,WndIcone icone) {
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	WndImageInit(f,(int)icone->larg,(int)icone->haut,0,0);
	if(f->image.surf) {
		unsigned char *p;
		if((p = (f->image.surf)->pixels)) {
			int i,nb; WndColor *c;
			nb = 3 * (f->image.surf)->width * (f->image.surf)->height;
			c = WndColorGrey[WND_Q_ECRAN];
			i = nb; while(i--) {
				*p++ = (unsigned char)(c->red >> 8); i--;
				*p++ = (unsigned char)(c->green >> 8); i--;
				*p++ = (unsigned char)(c->blue >> 8);
			}
		}
	}
#endif
#ifdef X11
	WndIdent w; WndServer *s; WndScreen d;
	w = f->w; s = f->s; d = s->d;
	if(f->image.surf) XDestroyImage(f->image.surf);
	f->image.surf = XGetImage(s->d,w,f->x0,f->y0,f->xm,f->ym,0xFFFFFF,ZPixmap);
#endif
}
/* ========================================================================== */
void WndIconePixel(WndFrame f, int x, int y, WndIcnPxl pixel) {
	WndCol8bits c8; WndColor *c;
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
	if(WndModeNone) return;
	if(WndPS) {
		if((pixel->m)) { c8.r = pixel->r; c8.g = pixel->g; c8.b = pixel->b; }
		else {
			c = WndColorGrey[WND_Q_ECRAN];
			c8.r = (unsigned char)(c->red >> 8);
			c8.g = (unsigned char)(c->green >> 8);
			c8.b = (unsigned char)(c->blue >> 8);
		}
		WndImageFill8bits(f,x - f->image.dx,y - f->image.dy,&c8);
		return;
	}
#ifdef OPENGL
	if((pixel->m)) { c8.r = pixel->r; c8.g = pixel->g; c8.b = pixel->b; }
	else {
		c = WndColorGrey[WND_Q_ECRAN];
		c8.r = (unsigned char)(c->red >> 8);
		c8.g = (unsigned char)(c->green >> 8);
		c8.b = (unsigned char)(c->blue >> 8);
	}
	WndImageFill8bits(f,x - f->image.dx,y - f->image.dy,&c8);
#else  /* !OPENGL */
	WndColor p;
	if(!(pixel->m)) return;
	#ifdef WIN32
		p = RGB(pixel->r, pixel->g, pixel->b);
	#else
		WndColorSetByRGB(&p,pixel->r<<8,pixel->g<<8,pixel->b<<8);
	#endif
	WndImageDrawColor(f,x,y,&p);
#endif /* !OPENGL */
}
/* ========================================================================== */
void WndImageShow(WndFrame f) {
	unsigned char *pixels;
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifndef WXWIDGETS
	if(WndModeNone) return;
	if(!(f->image.surf)) return;
	if(WndPS) {
		int larg,haut; int i,nb,val;
		if((pixels = (f->image.surf)->pixels) == 0) return;
		larg = (f->image.surf)->width;
		haut = (f->image.surf)->height;
		nb = larg * haut;
		for(i=0; i<nb; i++) {
			if(!(i % 40)) fprintf(WndPS,"\n");
			val = (*pixels++ & 0xFF);
			fprintf(WndPS,"%02X",val);
		}
		fprintf(WndPS,"\nstroke\n");
		WndPSstroked = 1;
	#ifdef QUICKDRAW
		free((f->image.surf)->pixels); (f->image.surf)->pixels = 0;
	#endif
		return;
	}

#ifdef OPENGL
	char doit_terminer; int dim,sizx,sizy; double x,y; // GLenum rc;

	if(!(pixels = (f->image.surf)->pixels)) return;
	doit_terminer = WndRefreshBegin(f);
	glfwGetWindowSize(f->w,&sizx,&sizy);
	//- glPixelStorei(GL_PACK_ALIGNMENT,4);
	if(f->image.lut) {
		x = ((double)(2 * (f->x0 + f->image.dx)) / (double)sizx) - 1.0;
		y = ((double)(2 * (f->y0 + f->image.dy + WND_ASCINT_WIDZ)) / (double)sizy) - 1.0;
		glRasterPos2d(x,y);
		//- printf("(%s) image avec LUT\n",__func__);
		glPixelTransferi(GL_MAP_COLOR,1); glPixelTransferi(GL_INDEX_SHIFT,0); glPixelTransferi(GL_INDEX_OFFSET,0); // par defaut
		dim = f->image.nbcolors;
		if((f->image.R)) glPixelMapusv(GL_PIXEL_MAP_I_TO_R,dim,f->image.R); // rc = glGetError(); printf("R rend %04X (%s)\n",rc,WndGLerreur(rc));
		if((f->image.G)) glPixelMapusv(GL_PIXEL_MAP_I_TO_G,dim,f->image.G); // rc = glGetError(); printf("G rend %04X (%s)\n",rc,WndGLerreur(rc));
		if((f->image.B)) glPixelMapusv(GL_PIXEL_MAP_I_TO_B,dim,f->image.B); // rc = glGetError(); printf("B rend %04X (%s)\n",rc,WndGLerreur(rc));
		if((f->image.A)) glPixelMapusv(GL_PIXEL_MAP_I_TO_A,dim,f->image.A); // rc = glGetError(); printf("A rend %04X (%s)\n",rc,WndGLerreur(rc));
		glDrawPixels((f->image.surf)->width,(f->image.surf)->height,GL_COLOR_INDEX,GL_UNSIGNED_BYTE,pixels);
	} else /* !f->image.lut */ {
		/* int i,k; unsigned char *pixel;
		printf("(%s) Icone:\n0000| ",__func__);
		k = (f->image.surf)->width * (f->image.surf)->height;
		pixel = pixels; i = 0;
		while(k--) {
			printf("%02X",*pixel++); i++;
			if(!(i%16)) printf("\n%04d| ",i); else if(!(i%3)) printf(" ");
		}
		printf("\n-----------------------------\n"); */
		x = ((double)(2 * (f->x0 + f->image.dx)) / (double)sizx) - 1.0;
		y = 1.0 - ((double)(2 * (f->y0 + f->image.dy + WND_ASCINT_WIDZ)) / (double)sizy);
		glRasterPos2d(x,y);
		glDrawPixels((f->image.surf)->width,(f->image.surf)->height,GL_RGB,GL_UNSIGNED_BYTE,pixels);
	}
	if(doit_terminer) WndRefreshEnd(f);
#endif

#ifdef X11
	WndIdent w; WndServer *s;
 	w = f->w; s = f->s;
	/* Valeur de GC?? */
	XPutImage(s->d,w,f->gc[0].coul[WND_GC_TEXT],f->image.surf,0,0,f->x0,f->y0,f->xm,f->ym);
	XDestroyImage(f->image.surf); f->image.surf = 0;
#endif

#ifdef WIN32
	EndPaint(f->w, &((f->image.surf)->paintStruct));
	InvalidateRect(f->w, &(((f->image.surf)->paintStruct).rcPaint), false);
#endif
    
#ifdef QUICKDRAW
	#ifdef CARBON
	//	QDFlushPortBuffer(GetQDGlobalsThePort(),NULL);
	#endif
	//	PortChanged(f->w); a priori inutile
#endif
#endif
}
/* ========================================================================== */
WndIcone WndIconeFromBmpFile(char *nom_complet, char *explics) {
	int p; ssize_t lu; unsigned short bloc[27];
	TypeWndBmpHeader entete;
	WndBmpPixel24 ligne24; /* WndBmpPixel16 ligne16; */ WndBmpPixel32 ligne32;
	char *ligne_lue;
	int i,j,bits,taille_pixel,dim_image,dim_ligne; char xdirect,ydirect;
	WndIcone icone; WndIcnPxl pixel;
	char dbg;
	
	dbg = 0;
	ligne24 = 0; ligne32 = 0; pixel = 0; // gcc..
    ligne_lue = 0;
	icone = 0;
	if((p = open(nom_complet,O_RDONLY)) < 0) { sprintf(explics,"%s: illisible (%s)",nom_complet,strerror(errno)); return(0); }
	//	lu = read(p,(void *)entete,sizeof(TypeBmpHeader));
	lu = read(p,(void *)bloc,54);
	if(lu < 0) { sprintf(explics,"%s: illisible (%s)",nom_complet,strerror(errno)); close(p); return(0); }
	else if(!lu) { sprintf(explics,"%s vide",nom_complet); close(p); return(0); }
	
	if(dbg) {
		printf("(%s) Entete du fichier %s:\n",__func__,nom_complet);
		for(i=0; i<27; i++) {
			printf(" %04x",bloc[i? ((i%2)?i+1:i-1): 0]);
			if((i == 6) || (i == 14) || (i == 22) || (i == 26)) printf("\n");
		}
	}
	entete.fichier.ident   =    bloc[0];
	entete.fichier.taille  =   (bloc[ 2] << 16) | bloc[ 1];
	entete.fichier.reserve =   (bloc[ 4] << 16) | bloc[ 3];
	entete.fichier.offset  =   (bloc[ 6] << 16) | bloc[ 5];
	entete.image.taille    =   (bloc[ 8] << 16) | bloc[ 7];
	entete.image.larg      =   (bloc[10] << 16) | bloc[ 9];
	entete.image.haut      =   (bloc[12] << 16) | bloc[11];
	entete.image.plans     =    bloc[13];
	entete.image.bits_par_couleur = bloc[14];
	entete.image.codage    =   (bloc[16] << 16) | bloc[15];
	entete.image.total     =   (bloc[18] << 16) | bloc[17];
	entete.image.h_resol   =   (bloc[20] << 16) | bloc[19];
	entete.image.v_resol   =   (bloc[22] << 16) | bloc[21];
	entete.image.dim_palette = (bloc[24] << 16) | bloc[23];
	entete.image.nb_ppales =   (bloc[26] << 16) | bloc[25];
	xdirect = ydirect = 1;
	if(entete.image.larg & 0x80000000) { entete.image.larg = (0xFFFFFFFF - entete.image.larg) + 1; xdirect = 0; }
	if(entete.image.haut & 0x80000000) { entete.image.haut = (0xFFFFFFFF - entete.image.haut) + 1; ydirect = 0; xdirect = 1 - xdirect; }
	if(!entete.image.codage) entete.image.codage = 1;
	if(dbg)  {
		printf("(%s) Decodage de l'entete BMP:\n",__func__);
		printf("   ID       total      nul    offset\n");
		printf("  %04X    %08X  %08X  %08X\n",entete.fichier.ident,entete.fichier.taille,entete.fichier.reserve,entete.fichier.offset);
		printf(" taille    largeur   hauteur  plan bits\n");
		printf("%08X %c%08X %c%08X  %04X %04X\n",entete.image.taille,xdirect?' ':'-',entete.image.larg,ydirect?' ':'-',entete.image.haut,entete.image.plans,entete.image.bits_par_couleur);
		printf(" codage     total    h_resol   v_resol   LutDim    ppales\n");
		printf("%08X  %08X  %08X  %08X  %08X  %08X\n",entete.image.codage,entete.image.total,entete.image.h_resol,entete.image.v_resol,
			   entete.image.dim_palette,entete.image.nb_ppales);
	}
	bits = entete.image.bits_par_couleur & 0x7F;
	if(bits == 24) taille_pixel = sizeof(TypeWndBmpPixel24);
//	else if(bits == 16) taille_pixel = sizeof(TypeWndBmpPixel16);
	else if(bits == 32) taille_pixel = sizeof(TypeWndBmpPixel32);
	else { sprintf(explics,"%s: %d bits/couleur non gere",nom_complet,bits); goto hev; }
	dim_image = entete.image.larg * entete.image.haut;
	dim_ligne = (((entete.image.larg * taille_pixel - 1) / 4) + 1) * 4;
	ligne_lue = (char *)malloc(dim_ligne);
	if(ligne_lue == 0) { sprintf(explics,"%s: manque %d octets pour lire une ligne",nom_complet,dim_ligne); goto hev; }
	if(bits == 24) ligne24 = (WndBmpPixel24)ligne_lue;
//	else if(bits == 16) ligne16 = (WndBmpPixel16)ligne_lue;
	else if(bits == 32) ligne32 = (WndBmpPixel32)ligne_lue;
	pixel = (WndIcnPxl)malloc(dim_image * sizeof(TypeWndIcnPxl));
	if(pixel == 0) { sprintf(explics,"%s: manque %ld octets pour stocker les pixels",nom_complet,dim_image * sizeof(TypeWndIcnPxl)); goto hev; }
	icone = (WndIcone)malloc(sizeof(TypeWndIcone));
	if(icone == 0) { sprintf(explics,"%s: manque %ld octets pour creer l'icone",nom_complet,sizeof(TypeWndIcone)); goto hev; }
	icone->larg = entete.image.larg;
	icone->haut = entete.image.haut;
	icone->pixel = pixel;
	if(dbg) printf("(%s) Icone[%d x %d]@%08X, pixel[%d]@%08X\n",__func__,icone->larg,icone->haut,(hexa)icone,dim_image,(hexa)icone->pixel);
	if(entete.image.codage == 1) {
		int ligne;
		if(dbg) ligne = 1;
		for(i=0; i<dim_image; i++) icone->pixel[i].m = 1;
		j = ydirect? 0: (dim_image - 1);
		do {
			lu = read(p,ligne_lue,dim_ligne);
			if(lu <= 0) break;
			if(dbg> 1) printf("(%s) ligne %d -----------------------------------------\n",__func__,ligne++);
			for(i=0; i<entete.image.larg; i++) {
				if(xdirect) {
					if(bits == 24) {
						icone->pixel[j].r = ligne24[i].r;
						icone->pixel[j].g = ligne24[i].g;
						icone->pixel[j].b = ligne24[i].b;
//					} else if(bits == 16) {
//						icone->pixel[j].r = (unsigned char)(ligne16[i].r >> 8);
//						icone->pixel[j].g = (unsigned char)(ligne16[i].g >> 8);
//						icone->pixel[j].b = (unsigned char)(ligne16[i].b >> 8);
					} else if(bits == 32) {
						icone->pixel[j].r = (unsigned char)(ligne32[i].r >> 24);
						icone->pixel[j].g = (unsigned char)(ligne32[i].g >> 24);
						icone->pixel[j].b = (unsigned char)(ligne32[i].b >> 24);
					}
				} else {
					if(bits == 24) {
						icone->pixel[j].r = ligne24[entete.image.larg - i - 1].r;
						icone->pixel[j].g = ligne24[entete.image.larg - i - 1].g;
						icone->pixel[j].b = ligne24[entete.image.larg - i - 1].b;
//					} else if(bits == 16) {
//						icone->pixel[j].r = (unsigned char)(ligne16[entete.image.larg - i - 1].r >> 8);
//						icone->pixel[j].g = (unsigned char)(ligne16[entete.image.larg - i - 1].g >> 8);
//						icone->pixel[j].b = (unsigned char)(ligne16[entete.image.larg - i - 1].b >> 8);
					} else if(bits == 32) {
						icone->pixel[j].r = (unsigned char)(ligne32[entete.image.larg - i - 1].r >> 24);
						icone->pixel[j].g = (unsigned char)(ligne32[entete.image.larg - i - 1].g >> 24);
						icone->pixel[j].b = (unsigned char)(ligne32[entete.image.larg - i - 1].b >> 24);
					}
				}
				if(ydirect) j++; else --j;
			}
		} while(1);
	} else if(entete.image.codage == 3) /* utilise palette */ {
		sprintf(explics,"%s: mode palette non implemente",nom_complet);
		free(icone); icone = 0;
	}
	
hev:
	close(p);
	if(ligne_lue) free(ligne_lue);
	if(!icone && pixel) free(pixel);
	return(icone);
}
/* ========================================================================== */
char WndIconeToBmpFile(char *nom_complet, WndIcone icone, char image, char *explics) {
	int p; ssize_t lu; unsigned short bloc[27];
	TypeWndBmpHeader entete;
	WndBmpPixel24 ligne24; /* WndBmpPixel16 ligne16; */ WndBmpPixel32 ligne32;
	char *ligne_ecrite;
	int i,j,k,n,b,bits,hdr,taille_pixel,dim_ligne;
	WndIcnPxl pixel;
	char dbg;
	
	dbg = 0;
    ligne_ecrite = 0;
	if((p = open(nom_complet,O_WRONLY|O_CREAT|O_EXCL)) < 0) {
		if(errno == EEXIST) {
			if(OpiumChoix(2,WndChoixEcriture,"Fichier deja cree. Que faire donc?")) {
				if((p = open(nom_complet,O_WRONLY|O_CREAT)) >= 0) goto go;
				sprintf(explics,"%s inutilisable (%s)",nom_complet,strerror(errno));
			} else sprintf(explics,"%s deja existant. Changer de nom..",nom_complet);
		} else sprintf(explics,"%s inaccessible (%s)",nom_complet,strerror(errno));
		return(0);
	}
go:	if(fchmod(p,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) < 0) {
		sprintf(explics,"chmod %s: %s",nom_complet,strerror(errno));
	}
	lu = -1;
	if(image) bits = 24; else bits = 24; // 8;
	if(bits == 24) taille_pixel = sizeof(TypeWndBmpPixel24);
	//	else if(bits == 16) taille_pixel = sizeof(TypeWndBmpPixel16);
	else if(bits == 32) taille_pixel = sizeof(TypeWndBmpPixel32);
	else if(bits == 8) taille_pixel = 1;
	else { sprintf(explics,"%s: %d bits/couleur non gere",nom_complet,bits); goto hev; }
	if(bits == 1) dim_ligne = (((icone->larg  - 1) / 32) + 1) * 4;
	else dim_ligne = ((((icone->larg * taille_pixel) - 1) / 4) + 1) * 4;
	
	ligne_ecrite = (char *)malloc(dim_ligne);
	if(ligne_ecrite == 0) { sprintf(explics,"%s: manque %d octets pour lire une ligne",nom_complet,dim_ligne); goto hev; }
	if(bits == 24) ligne24 = (WndBmpPixel24)ligne_ecrite;
	//	else if(bits == 16) ligne16 = (WndBmpPixel16)ligne_ecrite;
	else if(bits == 32) ligne32 = (WndBmpPixel32)ligne_ecrite;
	
	hdr = 54; // sizeof(TypeWndBmpHeader) rend 56 (14 * 4)
	
	entete.fichier.ident = 0x4D42;
	entete.fichier.taille = hdr + (icone->haut * dim_ligne);
	entete.fichier.reserve = 0;
	entete.fichier.offset = hdr;
	entete.image.taille = 40; // taille de entete.image
	entete.image.larg = icone->larg;
	entete.image.haut = icone->haut;
	entete.image.plans = 1;
	entete.image.bits_par_couleur = bits;
	entete.image.codage = 0;
	entete.image.total = 0;
	entete.image.h_resol = 0;
	entete.image.v_resol = 0;
	entete.image.dim_palette = 0;
	entete.image.nb_ppales = 0;

	if(dbg)  {
		printf("(%s) Encodage de l'entete BMP:\n",__func__);
		printf("   ID       total      nul    offset\n");
		printf("  %04X    %08X  %08X  %08X\n",entete.fichier.ident,entete.fichier.taille,entete.fichier.reserve,entete.fichier.offset);
		printf(" taille    largeur   hauteur  plan bits\n");
		printf("%08X  %08X  %08X  %04X %04X\n",entete.image.taille,entete.image.larg,entete.image.haut,entete.image.plans,entete.image.bits_par_couleur);
		printf(" codage     total    h_resol   v_resol   LutDim    ppales\n");
		printf("%08X  %08X  %08X  %08X  %08X  %08X\n",entete.image.codage,entete.image.total,entete.image.h_resol,entete.image.v_resol,
			   entete.image.dim_palette,entete.image.nb_ppales);
	}

	bloc[ 0] = entete.fichier.ident;
	bloc[ 1] = entete.fichier.taille & 0xFFFF;  bloc[ 2] = (entete.fichier.taille >> 16) & 0xFFFF;  // entete.fichier.taille  =   (bloc[ 2] << 16) | bloc[ 1];
	bloc[ 3] = entete.fichier.reserve & 0xFFFF; bloc[ 4] = (entete.fichier.reserve >> 16) & 0xFFFF; // entete.fichier.reserve =   (bloc[ 4] << 16) | bloc[ 3];
	bloc[ 5] = entete.fichier.offset & 0xFFFF;  bloc[ 6] = (entete.fichier.offset >> 16) & 0xFFFF;  // entete.fichier.offset  =   (bloc[ 6] << 16) | bloc[ 5];
	bloc[ 7] = entete.image.taille & 0xFFFF;    bloc[ 8] = (entete.image.taille >> 16) & 0xFFFF;    // entete.image.taille    =   (bloc[ 8] << 16) | bloc[ 7];
	bloc[ 9] = entete.image.larg & 0xFFFF;      bloc[10] = (entete.image.larg >> 16) & 0xFFFF;      // entete.image.larg      =   (bloc[10] << 16) | bloc[ 9];
	bloc[11] = entete.image.haut & 0xFFFF;      bloc[12] = (entete.image.haut >> 16) & 0xFFFF;      // entete.image.haut      =   (bloc[12] << 16) | bloc[11];
	bloc[13] = entete.image.plans;
	bloc[14] = entete.image.bits_par_couleur;
	bloc[15] = entete.image.codage & 0xFFFF;    bloc[16] = (entete.image.codage >> 16) & 0xFFFF;    // entete.image.codage    =   (bloc[16] << 16) | bloc[15];
	bloc[17] = entete.image.total & 0xFFFF;     bloc[18] = (entete.image.total >> 16) & 0xFFFF;     // entete.image.total     =   (bloc[18] << 16) | bloc[17];
	bloc[19] = entete.image.h_resol & 0xFFFF;   bloc[20] = (entete.image.h_resol >> 16) & 0xFFFF;   // entete.image.h_resol   =   (bloc[20] << 16) | bloc[19];
	bloc[21] = entete.image.v_resol & 0xFFFF;   bloc[22] = (entete.image.v_resol >> 16) & 0xFFFF;   // entete.image.v_resol   =   (bloc[22] << 16) | bloc[21];
	bloc[23] = entete.image.dim_palette & 0xFFFF; bloc[24] = (entete.image.dim_palette >> 16) & 0xFFFF; // entete.image.dim_palette = (bloc[24] << 16) | bloc[23];
	bloc[25] = entete.image.nb_ppales & 0xFFFF; bloc[26] = (entete.image.nb_ppales >> 16) & 0xFFFF;  // entete.image.nb_ppales =   (bloc[26] << 16) | bloc[25];
	if(dbg) {
		printf("(%s) Entete du fichier %s:\n",__func__,nom_complet);
		for(i=0; i<27; i++) {
			printf(" %04x",bloc[i? ((i%2)?i+1:i-1): 0]);
			if((i == 6) || (i == 14) || (i == 22) || (i == 26)) printf("\n");
		}
	}
	lu = write(p,bloc,54);
	if(lu < 0) { sprintf(explics,"%s: inutilisable (%s)",nom_complet,strerror(errno)); goto hev; }
	else if(!lu) { sprintf(explics,"%s vide",nom_complet); goto hev; }
		
	pixel = icone->pixel;
	if(dbg) printf("(%s) Icone[%d x %d]@%08X, pixel[%d]@%08X\n",__func__,icone->larg,icone->haut,(hexa)icone,icone->larg * icone->haut,(hexa)icone->pixel);
	if((entete.image.codage == 0) || (entete.image.codage == 1)) {
		int ligne;
		if(dbg) ligne = 1;
		if(bits == 1) for(i=0; i<dim_ligne; i++) ligne_ecrite[i] = 0;
		else for(i=(icone->larg * taille_pixel); i<dim_ligne; i++) ligne_ecrite[i] = 0;
		j = 0;
		for(k=0; k<entete.image.haut; k++) {
			if(dbg> 1) printf("(%s) ligne %d -----------------------------------------\n",__func__,ligne++);
			if(bits == 1) { b = 0; n = 0; }
			for(i=0; i<entete.image.larg; i++) {
				if(bits == 24) {
					ligne24[i].r = image? icone->pixel[j].r: ((icone->pixel[j].m)? 0xFF: 0x00); // icone->pixel[j].r = ligne24[i].r;
					ligne24[i].g = image? icone->pixel[j].g: ((icone->pixel[j].m)? 0xFF: 0x00); // icone->pixel[j].g = ligne24[i].g;
					ligne24[i].b = image? icone->pixel[j].b: ((icone->pixel[j].m)? 0xFF: 0x00); // icone->pixel[j].b = ligne24[i].b;
					// } else if(bits == 16) {
					//	icone->pixel[j].r = (unsigned char)(ligne16[i].r >> 8);
					//	icone->pixel[j].g = (unsigned char)(ligne16[i].g >> 8);
					//	icone->pixel[j].b = (unsigned char)(ligne16[i].b >> 8);
				} else if(bits == 32) {
					ligne32[i].r = image? (icone->pixel[j].r << 24) & 0xFF000000: ((icone->pixel[j].m)? 0xFFFFFFFF: 0); // icone->pixel[j].r = (unsigned char)(ligne32[i].r >> 24);
					ligne32[i].g = image? (icone->pixel[j].g << 24) & 0xFF000000: ((icone->pixel[j].m)? 0xFFFFFFFF: 0); // icone->pixel[j].g = (unsigned char)(ligne32[i].g >> 24);
					ligne32[i].b = image? (icone->pixel[j].b << 24) & 0xFF000000: ((icone->pixel[j].m)? 0xFFFFFFFF: 0); // icone->pixel[j].b = (unsigned char)(ligne32[i].b >> 24);
					ligne32[i].x =       ((icone->pixel[j].m)? 0xFFFFFFFF: 0);  // icone->pixel[j].r = (unsigned char)(ligne32[i].r >> 24);
				} else if(bits == 8) {
					ligne_ecrite[i] = ((icone->pixel[j].m)? 0xFF: 0);
				} else if(bits == 1) {
					if(icone->pixel[j].m) ligne_ecrite[n] = ligne_ecrite[n] | (1 << b);
					if(++b > 31) { b = 0; if(++n >= dim_ligne) break; }
				}
				j++;
			}
			if((lu = write(p,ligne_ecrite,dim_ligne)) <= 0) break;
		};
	} else if(entete.image.codage == 3) /* utilise palette */ {
		sprintf(explics,"%s: mode palette non implemente",nom_complet);
	}
	
hev:
	close(p);
	if(ligne_ecrite) free(ligne_ecrite);
	return((lu > 0));
}
/* ========================================================================== */
WndIcone WndIconeRead(char *nom_complet, char *explics) {
	unsigned short larg,haut; int dim_image;
	int p; ssize_t lu;
	WndIcone icone; WndIcnPxl pixel;
	char dbg;
	
	dbg = 0;
	icone = 0;
	if((p = open(nom_complet,O_RDONLY)) < 0) { sprintf(explics,"illisible (%s)",strerror(errno)); return(0); }
	lu = read(p,&larg,sizeof(unsigned short));
	if(lu < 0) { sprintf(explics,"illisible (%s)",strerror(errno)); return(0); }
	else if(!lu) { sprintf(explics,"vide"); return(0); }
	lu = read(p,&haut,sizeof(unsigned short));
	if(lu < 0) { sprintf(explics,"illisible (%s)",strerror(errno)); return(0); }
	else if(!lu) { sprintf(explics,"incomplet"); return(0); }
	
	dim_image = larg * haut;
	pixel = (WndIcnPxl)malloc(dim_image * sizeof(TypeWndIcnPxl));
	if(pixel == 0) { sprintf(explics,"manque %ld octets pour stocker les pixels",dim_image * sizeof(TypeWndIcnPxl)); goto hev; }
	icone = (WndIcone)malloc(sizeof(TypeWndIcone));
	if(icone == 0) { sprintf(explics,"manque %ld octets pour creer l'icone",sizeof(TypeWndIcone)); goto hev; }
	icone->larg = larg;
	icone->haut = haut;
	icone->pixel = pixel;
	if(dbg) printf("(%s) Icone[%d x %d]@%08X, pixel[%d]@%08X\n",__func__,icone->larg,icone->haut,(hexa)icone,dim_image,(hexa)icone->pixel);
	
	lu = read(p,pixel,dim_image * sizeof(TypeWndIcnPxl));
	if(lu < 0) { sprintf(explics,"illisible (%s)",strerror(errno)); return(0); }
	else if(!lu) { sprintf(explics,"incomplet"); return(0); }
	
hev:
	close(p);
	if(!icone && pixel) free(pixel);
	return(icone);
}
/* ========================================================================= */
char WndIconeSave(WndIcone icone, char type, char *nom, char *chemin, char *explics) {
	char nom_complet[MAXFILENAME],masque[MAXFILENAME]; FILE *f; int p; ssize_t lu; int dim_image;
	char defvar[256]; int i,ligne;
	
	dim_image = icone->larg * icone->haut;
	if(chemin) {
		if(*chemin) {
			strcpy(nom_complet,chemin);
			if(chemin[strlen(chemin) - 1] != '/') strcat(nom_complet,"/");
			strcat(nom_complet,nom);
		} else strcpy(nom_complet,nom);
	} else strcpy(nom_complet,nom);
	switch(type) {
		case ICONE_H:
			strcat(nom_complet,".h");
			if(!(f = fopen(nom_complet,"w"))) {
				sprintf(explics,"%s inutilisable (%s)",nom_complet,strerror(errno)); return(0);
			}
			strncpy(defvar,nom,255); defvar[255] = '\0'; WndItemMajuscules((unsigned char *)defvar);
			fprintf(f,"#ifndef %s\n",defvar);
			fprintf(f,"#define %s\n\n",defvar);
			fprintf(f,"TypeWndIcnPxl %sPixels[] = {",nom);
			if(icone->larg < 21) ligne = icone->larg; else ligne = 20;
			for(i=0; i<dim_image; i++) {
				if(!(i % ligne)) fprintf(f,"\n\t"); else fprintf(f," ");
				fprintf(f,"{ %d, 0x%02X, 0x%02X, 0x%02X }%c",icone->pixel[i].m,
						icone->pixel[i].r,icone->pixel[i].g,icone->pixel[i].b,
						(i < (dim_image - 1))? ',': ' ');
			}
			fprintf(f,"\n};\n");
			fprintf(f,"TypeWndIcone %s = { %d, %d, %sPixels };\n\n",nom,icone->larg,icone->haut,nom);
			fprintf(f,"#endif /* %s */\n",defvar);
			fclose(f);
			break;
		case ICONE_FIC:
			strcat(nom_complet,".icone");
			if((p = open(nom_complet,O_WRONLY|O_CREAT|O_EXCL)) < 0) {
//				sprintf(explics,"%s inutilisable (%s)",nom_complet,strerror(errno)); return(0);
				if(errno == EEXIST) {
					if(OpiumChoix(2,WndChoixEcriture,"Fichier deja cree. Que faire donc?")) {
						if((p = open(nom_complet,O_WRONLY|O_CREAT)) >= 0) goto go;
						sprintf(explics,"%s inutilisable (%s)",nom_complet,strerror(errno));
					} else sprintf(explics,"%s deja existant. Changer de nom..",nom_complet);
				} else sprintf(explics,"%s inutilisable (%s)",nom_complet,strerror(errno));
				return(0);
			}
		go:	if(fchmod(p,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) < 0) {
				sprintf(explics,"chmod %s: %s",nom_complet,strerror(errno));
			}
			lu = write(p,&icone->larg,sizeof(unsigned short));
			if(lu < 0) { sprintf(explics,"illisible (%s)",strerror(errno)); return(0); }
			else if(!lu) { sprintf(explics,"vide"); return(0); }
			lu = write(p,&icone->haut,sizeof(unsigned short));
			if(lu < 0) { sprintf(explics,"illisible (%s)",strerror(errno)); return(0); }
			else if(!lu) { sprintf(explics,"vide"); return(0); }
			lu = write(p,icone->pixel,dim_image * sizeof(TypeWndIcnPxl));
			if(lu < 0) { sprintf(explics,"illisible (%s)",strerror(errno)); return(0); }
			else if(!lu) { sprintf(explics,"incomplet"); return(0); }
			close(p);
			break;
		case ICONE_BMP:
 #define PLUS_MASQUE
		#ifdef PLUS_MASQUE
			strcat(strcpy(masque,nom_complet),"_msk.bmp");
			if(!( WndIconeToBmpFile(masque,icone,0,explics))) return(0);
			strcat(nom_complet,"_img.bmp");
		#else
			strcat(nom_complet,".bmp");
		#endif
			if(!(WndIconeToBmpFile(nom_complet,icone,1,explics))) return(0);
			break;
		default: sprintf(explics,"ERREUR SYSTEME SUR LE TYPE D'ICONE (%d)",type); return(0);
	}
	sprintf(explics,"Icone sauvee sur %s",nom_complet);
	return(1);
}
/* ========================================================================== */
WndIcone WndIconeCompactee(WndIcone srce, int facteur) {
	int larg,haut,x,y,j;
	WndIcone icone; WndIcnPxl pixel;
	struct { int m,r,g,b,n; } *tempo;
	int hori,vert,dim,h,h0,hp;

	if(facteur <= 0) facteur = 1;
	larg = srce->larg; hori = ((larg - 1) / facteur) + 1;
	haut = srce->haut; vert = ((haut - 1) / facteur) + 1;
	dim = hori * vert;
	pixel = (WndIcnPxl)malloc(dim * sizeof(TypeWndIcnPxl));
	if(pixel == 0) return(0);
	icone = (WndIcone)malloc(sizeof(TypeWndIcone));
	if(icone == 0) { free(pixel); return(0); }
	icone->larg = hori;
	icone->haut = vert;
	icone->pixel = pixel;
	if(facteur == 1) {
		j = 0;
		for(y=0; y<haut; y++) {
			for(x=0; x<larg; x++) {
				icone->pixel[j].m = srce->pixel[j].m;
				icone->pixel[j].r = srce->pixel[j].r;
				icone->pixel[j].g = srce->pixel[j].g;
				icone->pixel[j].b = srce->pixel[j].b;
				j++;
			}
		}
	} else {
		tempo = (void *)malloc(hori * 5 * sizeof(int));
		if(tempo == 0) { free(pixel); free(icone); return(0); }
		j = 0; hp = -1;
		for(y=0; y<=haut; y++) {
			h0 = (y / facteur) * hori;
			if((h0 > hp) || (y == haut)){
				for(h=0; h<hori; h++) {
					if(hp >= 0) {
						icone->pixel[hp+h].m = tempo[h].m;
						if(tempo[h].n) {
							icone->pixel[hp+h].r = tempo[h].r / tempo[h].n;
							icone->pixel[hp+h].g = tempo[h].g / tempo[h].n;
							icone->pixel[hp+h].b = tempo[h].b / tempo[h].n;
						} else {
							icone->pixel[hp+h].r = icone->pixel[hp+h].g = icone->pixel[hp+h].b = 0;
						}
					}
					tempo[h].m = 0;
					tempo[h].r = tempo[h].g = tempo[h].b = 0;
					tempo[h].n = 0;
				}
				if(y == haut) break;
				hp = h0;
			}
			for(x=0; x<larg; x++) {
				h = x / facteur;
				if(srce->pixel[j].m) tempo[h].m = 1;
				tempo[h].r += srce->pixel[j].r;
				tempo[h].g += srce->pixel[j].g;
				tempo[h].b += srce->pixel[j].b;
				tempo[h].n += 1;
				j++;
			}
		}
		free(tempo);
	}
	return(icone);
}
/* ========================================================================== */
void WndIconeDelete(WndIcone icone) {
	if(!icone) return;
	if(icone->pixel) free(icone->pixel);
	free(icone);
}
/* ========================================================================== */
/* ======================== Dessin en mode graphique ======================== */
/* ========================================================================== */
void WndString(WndFrame f, WndContextPtr gc, int x, int y, char *texte) {
	size_t l;

	if(WndModeNone) return;
	l = strlen(texte);
	// PRINT_GC(gc);
	WndDrawString(f,gc,f->x0 + x,f->y0 + y - (f->s)->decal,texte,l);
}
/* ========================================================================== */
void WndLine(WndFrame f, WndContextPtr gc, int x, int y, int l, int h) {
	WndDrawLine(f,gc,f->x0+x,f->y0+y,f->x0+x+l,f->y0+y+h);
}
/* ========================================================================== */
void WndPoly(WndFrame f, WndContextPtr gc, WndPoint *p, int nb, int type) {
	WndIdent w;
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef X11
	WndScreen d; WndServer *s;
	int x1,y1,x2,y2;
#endif
	int k,e,dy,signe,line_width;
	int i; WndPoint *q;

	if(WndModeNone) return;
	if(nb <= 0) return;
	if(gc == 0) gc = WND_TEXT;
#ifdef X11
	s = f->s;
	d = s->d;
#endif
	if(WndPS) {
		WndColor c;
#ifdef X11
		WndContextVal gcval;
		XGetGCValues(d,gc,GCForeground,&gcval);
		line_width = gcval.line_width;
		c.pixel = gcval.foreground;
		XQueryColor(d,DefaultColormap(d,DefaultScreen(d)),&c);
#endif
#ifdef WIN32
		line_width = gc->line_width;
		memcpy(&c,gc->foreground,sizeof(WndColor));
#endif
#ifdef OPENGL
		line_width = gc->line_width;
		memcpy(&c,gc->foreground,sizeof(WndColor));
#endif
#ifdef QUICKDRAW
		line_width = gc->line_width;
		memcpy(&c,gc->foreground,sizeof(WndColor));
#endif
		WndPScolorie(&c);
		e = 1; signe = -1;
		for(k=0; k<line_width; k++) {
			dy = signe * (e++ >> 1); signe = -signe;
			fprintf(WndPS,"%d %d moveto\n",f->x0 + p->x,-(f->y0 + p->y + dy));
			q = p; i = nb;
			while(--i) {
				q++;
				if(type == WND_ABS)
					fprintf(WndPS," %4d %4d lineto",f->x0 + q->x,-(f->y0 + q->y + dy));
				else fprintf(WndPS," %4d %4d rlineto",q->x,-q->y);
				if(!((nb - i) % 4)) fprintf(WndPS,"\n");
			}
			if((nb - 1) % 4) fprintf(WndPS,"\n");
		}
		WndPSstroked = 0;
		return;
	}
	w = f->w;
#ifdef X11
	s = f->s;
	if((f->x0 == 0) && (f->y0 == 0)) XDrawLines(d,w,gc,p,nb,type); /* !!! probleme du chgt de repere!!! */
	else {
		q = p; i = nb;
		x1 = f->x0 + p->x;
		y1 = f->y0 + p->y;
		while(--i) {
			q++;
			if(type == WND_ABS) {
				x2 = f->x0 + q->x;
				y2 = f->y0 + q->y;
			} else {
				x2 = x1 + q->x;
				y2 = y1 + q->y;
			}
			WndDrawLine(f,gc,x1,y1,x2,y2);
			x1 = x2;
			y1 = y2;
		}
	}
#endif

#ifdef WIN32
	PAINTSTRUCT paintst; HDC hDC; HPEN hPen, hPenOld;

	hDC = BeginPaint(f->w, &paintst);
	if(gc->foreground) hPenOld = (HPEN)SelectObject(hDC, hPen = CreatePen(gc->line_style, gc->line_width, (gc->foreground) ));
	e = 1; signe = -1;
	for(k=0; k<gc->line_width; k++) {
		dy = signe * (e++ >> 1); signe = -signe;
		if(type == WND_ABS) MoveToEx(hDC, f->x0 + p->x,f->y0 + p->y + dy, NULL);
		else MoveToEx(hDC, p->x, p->y + dy, NULL);
		q = p; i = nb;
		while(--i) {
			q++;
			if(type == WND_ABS) LineTo(hDC, f->x0 + q->x, f->y0 + q->y + dy);
			else LineTo(hDC, q->x + p->x, q->y + p->y);
		}
	}
	//PolylineTo(hDC, p, nb);
	if(gc->foreground) SelectObject(hDC, hPenOld);
	EndPaint(f->w, &paintst);
	InvalidateRect(f->w, &(paintst.rcPaint), false);
	if(gc->foreground) DeleteObject(hPen);
#endif

#ifdef OPENGL
	int sizx,sizy; double xd,yd; char doit_terminer;
	short xc,yc;

	doit_terminer = WndRefreshBegin(f);
	if(gc) {
		if(gc->foreground) glColor3us((gc->foreground)->red,(gc->foreground)->green,(gc->foreground)->blue);
		glLineWidth(gc->line_width);
	}
	glfwGetWindowSize(w,&sizx,&sizy);
	glBegin(GL_LINE_STRIP);
	q = p; i = nb;
	xd = WndXToDouble(f->x0 + q->x,sizx);
	yd = WndYToDouble(f->y0 + q->y,sizy);
	glVertex2d(xd,yd);
	if(type == WND_REL) { xc = q->x; yc = q->y; }
	while(--i) {
		q++;
		if(type == WND_ABS) {
			xd = WndXToDouble(f->x0 + q->x,sizx);
			yd = WndYToDouble(f->y0 + q->y,sizy);
			glVertex2d(xd,yd);
		} else {
			xc += q->x; yc += q->y;
			xd = WndXToDouble(xc,sizx);
			yd = WndYToDouble(yc,sizy);
			glVertex2d(xd,yd);
		}
	}
	glEnd(); glFlush();
	if(doit_terminer) WndRefreshEnd(f);
#endif

#ifdef QUICKDRAW
	SetPort(WNDPORT(w));
	RGBForeColor(gc->foreground);
	// printf("(%s) @%08X.line_width = %d\n",__func__,(hexa)gc,gc->line_width);
	e = 1; signe = -1;
	for(k=0; k<gc->line_width; k++) {
		dy = signe * (e++ >> 1); signe = -signe;
		MoveTo(f->x0 + p->x,f->y0 + p->y + dy);
		q = p; i = nb;
		while(--i) {
			q++;
			if(type == WND_ABS) LineTo(f->x0 + q->x,f->y0 + q->y + dy);
			else Line(q->x,q->y);
		}
	}
//	RGBForeColor(WndColorText[f->qualite]);
#endif
}
/* ========================================================================== */
/* ========================== Decors pour le texte ========================== */
/* ========================================================================== */
void WndRectangle(WndFrame f, WndContextPtr gc, int x, int y, int l, int h) {
	WndIdent w;
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
	if(WndModeNone) return;
	w = f->w;
	if(gc == 0) gc = WND_TEXT;
#ifdef OPENGL
	int sizx,sizy; double xd,yd,xf,yf; char doit_terminer;

//	y -= (3 * (f->s)->lig / 4) + 1;
	glfwGetWindowSize(w,&sizx,&sizy);
	xd = WndXToDouble(f->x0 + x,sizx);
	yd = WndYToDouble(f->y0 + y,sizy);
	xf = WndXToDouble(f->x0 + x + l,sizx);
	yf = WndYToDouble(f->y0 + y + h,sizy);
	doit_terminer = WndRefreshBegin(f);
	if(gc) {
		if(gc->foreground) glColor3us((gc->foreground)->red,(gc->foreground)->green,(gc->foreground)->blue);
	}
	glRectd(xd,yd,xf,yf);
	if(doit_terminer) WndRefreshEnd(f);
#endif

#ifdef X11
	XDrawRectangle((f->s)->d,w,gc,f->x0 + x,f->y0 + y,l,h);
#endif

#ifdef WIN32
	PAINTSTRUCT paintst; HDC hDC; HPEN hPen, hPenOld;

	hDC = BeginPaint(f->w, &paintst);
	hPenOld = (HPEN) SelectObject(hDC, hPen = CreatePen(gc->line_style, gc->line_width, (gc->foreground)));
	if(gc->background) SetBkColor(hDC, (gc->background));
	else SetBkColor(hDC, *WndColorBgnd[f->qualite]);
	Rectangle(hDC, f->x0 + x, f->y0 + y, f->x0 + x+l, f->y0 + y+h);
	SelectObject(hDC, hPenOld);
	EndPaint(f->w, &paintst);
	InvalidateRect(f->w, &(paintst.rcPaint), false);
	DeleteObject(hPen);
#endif

#ifdef QUICKDRAW
	Rect r;
	SetPort(WNDPORT(w));
	RGBForeColor(gc->foreground);
	r.left = f->x0 + x; r.top = f->y0 + y; 
	r.right = r.left + l; r.bottom = r.top + h;
	FrameRect(&r);
//	RGBForeColor(WndColorText[f->qualite]);
#endif
}
/* ========================================================================== */
void WndArc(WndFrame f, WndContextPtr gc, int x, int y, int l, int h, 
	int a0, int da) {
	WndIdent w; WndServer *s;

	if(WndModeNone) return;
	w = f->w; s = f->s;
	if(gc == 0) gc = WND_TEXT;
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	int sizx,sizy; char doit_terminer;
	int i,pts_tot,pts_nb; double alpha,delta,fin;
	double x0,y0,demiX,demiY,echX,echY,px,py;

	glfwGetWindowSize(w,&sizx,&sizy);
	doit_terminer = WndRefreshBegin(f);
	if(gc) {
		if(gc->foreground) glColor3us((gc->foreground)->red,(gc->foreground)->green,(gc->foreground)->blue);
		glLineWidth(1);
	}
	x0 = (double)(f->x0 + x); y0 = (double)(f->y0 + y);
	demiX = (double)l / 2.0; demiY = (double)h / 2.0;
	echX = 2.0 / (double)sizx; echY = 2.0 / (double)sizy;
	pts_tot = 12;
	delta = PI / (double)pts_tot;
	pts_nb = (int)((double)da / delta) + 1;
	glBegin(GL_LINE_STRIP); // defini dans gl.h (reference dans glut.h)
	alpha = (double)a0 / RADTODEG; fin = (double)(a0 + da) / RADTODEG;
	for(i=0; i<pts_tot; i++) {
		px = x0 + (1.0 + sin(alpha)) * demiX;
		py = y0 + (1.0 - cos(alpha)) * demiY;
		glVertex2d((echX * px) - 1.0,1.0 - (echY * py));
		if(alpha < fin) { alpha += delta; if(alpha > fin) alpha = fin; }
		else break;
	}
	glEnd(); glFlush();
	if(doit_terminer) WndRefreshEnd(f);
#endif

#ifdef X11
	int b0,db;
	b0 = (90 - a0) * 64;
	db = -da * 64;
	XDrawArc(s->d,f->w,gc,f->x0 + x,f->y0 + y,l,h,b0,db);
#endif

#ifdef WIN32
	using namespace std;
	PAINTSTRUCT paintst; HDC hDC; HPEN hPen, hPenOld;

	hDC = BeginPaint(w, &paintst);
	hPenOld = (HPEN) SelectObject(hDC, hPen = CreatePen(gc->line_style, gc->line_width, (gc->foreground)));
	a0 += 90;
	da += a0;
	int ca0 = l * (float)cos( (double)(a0) * (PI/180) );
	int sa0 = h * (float)sin( (double)(a0) * (PI/180) );
	int cda = l * (float)cos( (double)(da) * (PI/180) );
	int sda = h * (float)sin( (double)(da) * (PI/180) );
	SetArcDirection(hDC, AD_COUNTERCLOCKWISE);
	Arc(hDC,f->x0 + x, 
			f->y0 + y, 
			f->x0 + x + l, 
			f->y0 + y + h,
			f->x0 + x + l/2 + ca0,
			f->y0 + y + l/2 + sa0,
			f->x0 + x + l/2 + cda,
			f->y0 + y + l/2 + sda
			);
	SelectObject(hDC, hPenOld);
	EndPaint(w, &paintst);
	InvalidateRect(w, &(paintst.rcPaint), false);
	DeleteObject(hPen);
#endif

#ifdef QUICKDRAW
	Rect r;
	SetPort(WNDPORT(w));
	RGBForeColor(gc->foreground);
	r.left = f->x0 + x; r.top = f->y0 + y; 
	r.right = r.left + l; r.bottom = r.top + h;
	FrameArc(&r,a0,da);
//	RGBForeColor(WndColorText[f->qualite]);
#endif
}
/* ========================================================================== */
void WndOvale(WndFrame f, WndContextPtr gc, int x, int y, int l, int h) {
	WndIdent w;
	WndServer *s;
#ifdef QUICKDRAW
	Rect r;
#endif
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
	if(WndModeNone) return;
	w = f->w;
	s = f->s;
	if(gc == 0) gc = WND_TEXT;
#ifdef OPENGL
	int sizx,sizy; char doit_terminer;

	glfwGetWindowSize(w,&sizx,&sizy);
	doit_terminer = WndRefreshBegin(f);
	if(gc) {
		if(gc->foreground) glColor3us((gc->foreground)->red,(gc->foreground)->green,(gc->foreground)->blue);
		glLineWidth(1);
	}
	glBegin(GL_LINE_LOOP); // defini dans gl.h (reference dans glut.h)
	#ifdef OVALE_CARRE
		double xd,yd,xf,yf;
		xd = WndXToDouble(f->x0 + x,sizx);
		yd = WndYToDouble(f->y0 + y,sizy);
		xf = WndXToDouble(f->x0 + x + l,sizx);
		yf = WndYToDouble(f->y0 + y + h,sizy);
		glVertex2d(xd,yd); glVertex2d(xf,yd); glVertex2d(xf,yf); glVertex2d(xd,yf); // glVertex2d(xd,yd);
	#else
		glVertex2d(WndXToDouble(f->x0+x+3,sizx),WndYToDouble(f->y0+y,sizy));

		glVertex2d(WndXToDouble(f->x0+x+l-3,sizx),WndYToDouble(f->y0+y,sizy));
//		glVertex2d(WndXToDouble(f->x0+x+l-2,sizx),WndYToDouble(f->y0+y+1,sizy));
//		glVertex2d(WndXToDouble(f->x0+x+l-1,sizx),WndYToDouble(f->y0+y+2,sizy));
		glVertex2d(WndXToDouble(f->x0+x+l,sizx),  WndYToDouble(f->y0+y+3,sizy));

		glVertex2d(WndXToDouble(f->x0+x+l,sizx),  WndYToDouble(f->y0+y+h-3,sizy));
//		glVertex2d(WndXToDouble(f->x0+x+l-1,sizx),WndYToDouble(f->y0+y+h-2,sizy));
//		glVertex2d(WndXToDouble(f->x0+x+l-2,sizx),WndYToDouble(f->y0+y+h-1,sizy));
		glVertex2d(WndXToDouble(f->x0+x+l-3,sizx),WndYToDouble(f->y0+y+h,sizy));

		glVertex2d(WndXToDouble(f->x0+x+3,sizx),WndYToDouble(f->y0+y+h,sizy));
//		glVertex2d(WndXToDouble(f->x0+x+2,sizx),WndYToDouble(f->y0+y+h-1,sizy));
//		glVertex2d(WndXToDouble(f->x0+x+1,sizx),WndYToDouble(f->y0+y+h-2,sizy));
		glVertex2d(WndXToDouble(f->x0+x,sizx),  WndYToDouble(f->y0+y+h-3,sizy));

		glVertex2d(WndXToDouble(f->x0+x,sizx),  WndYToDouble(f->y0+y+3,sizy));
//		glVertex2d(WndXToDouble(f->x0+x+1,sizx),WndYToDouble(f->y0+y+2,sizy));
//		glVertex2d(WndXToDouble(f->x0+x+2,sizx),WndYToDouble(f->y0+y+1,sizy));
		// glVertex2d(WndXToDouble(f->x0+x+3,sizx),WndYToDouble(f->y0+y,sizy));
	#endif

	glEnd(); glFlush();
	if(doit_terminer) WndRefreshEnd(f);
#endif

#ifdef X11
/*	XDrawArc(s->d,f->w,gc,x,y,l,h,0,64*360); */
	XDrawRectangle(s->d,w,gc,f->x0 + x,f->y0 + y,l,h);
#endif

#ifdef WIN32
	PAINTSTRUCT paintst; HDC hDC; HPEN hPen, hPenOld; HBRUSH hBrush, hBrushOld;

	hDC = BeginPaint(f->w, &paintst);
	hPenOld = (HPEN)SelectObject(hDC, hPen = CreatePen(gc->line_style, gc->line_width, (gc->foreground)));
	if(gc->foreground) hBrushOld = (HBRUSH) SelectObject(hDC, hBrush = CreateSolidBrush(gc->foreground));
	else hBrushOld = (HBRUSH) SelectObject(hDC, hBrush = CreateSolidBrush(*WndColorBgnd[f->qualite]));
	RoundRect(hDC, f->x0 + x,f->y0 + y, f->x0 + x+l, f->y0 + y+h, l/2, h/2);
	SelectObject(hDC, hPenOld);
	SelectObject(hDC, hBrushOld);
	EndPaint(f->w, &paintst);
	InvalidateRect(f->w, &(paintst.rcPaint), false);
	DeleteObject(hPen);
	DeleteObject(hBrush);
#endif

#ifdef QUICKDRAW
	SetPort(WNDPORT(w));
	RGBForeColor(gc->foreground);
	r.left = f->x0 + x; r.top = f->y0 + y; 
	r.right = r.left + l; r.bottom = r.top + h;
/*	FrameOval(&r); */
	FrameRoundRect(&r,10,10);
/*	FillRoundRect(&r,10,10,&((s->d)->gray)); trouver un pattern homogene */
//	RGBForeColor(WndColorText[f->qualite]);
#endif
}
/* ========================================================================== */
void WndFillFgnd(WndFrame f, WndContextPtr gc, int x, int y, int l, int h) {

	if(WndModeNone) return;
	if(!f) return;
	if(gc == 0) gc = WND_TEXT;
	#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	WndPaint(f,x,y,l,h,gc->foreground,0);
#endif

#ifdef X11
	XFillRectangle((f->s)->d,f->w,gc,f->x0+x,f->y0+y,l,h);
#endif

#ifdef WIN32
	WndPaint(f,x,y,l,h,&(gc->foreground),0);
#endif

#ifdef QUICKDRAW
/*	Rect r; SetPort(WNDPORT(f->w));
	r.left = f->x0 + x; r.top = f->y0 + y;
	r.right = r.left + l; r.bottom = r.top + h;
	RGBForeColor(gc->foreground);
	PaintRect(&r); */
//	RGBForeColor(WndColorText[f->qualite]);
	WndPaint(f,x,y,l,h,gc->foreground,0);
#endif
}
/* ========================================================================== */
void WndFillBgnd(WndFrame f, WndContextPtr gc, int x, int y, int l, int h) {

	if(WndModeNone) return;
	if(!f) return;

	if(gc == 0) gc = WND_STD;
#ifdef X11
	XFillRectangle((f->s)->d,f->w,gc,f->x0 + x,f->y0 + y,l,h);
#else
	if(!(gc->background)) return;
#endif
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	int sizx,sizy; double xd,yd,xf,yf; char doit_terminer;
	WndIdent w; w = f->w;

	if(gc) {
		glfwGetWindowSize(w,&sizx,&sizy);
		xd = WndXToDouble(f->x0 + x,sizx);
		yd = WndYToDouble(f->y0 + y,sizy);
		xf = WndXToDouble(f->x0 + x + l,sizx);
		yf = WndYToDouble(f->y0 + y + h,sizy);
		doit_terminer = WndRefreshBegin(f);
		if(gc->background) glColor3us((gc->background)->red,(gc->background)->green,(gc->background)->blue);
		glRectd(xd,yd,xf,yf);
		if(doit_terminer) WndRefreshEnd(f);
	}
#endif

#ifdef WIN32
	WndPaint(f, x, y, l, h, &(gc->background),0);
#endif
	
#ifdef QUICKDRAW
	Rect r;
	SetPort(WNDPORT(f->w));
	RGBForeColor(gc->background);
	r.left = f->x0 + x; r.top = f->y0 + y; 
	r.right = r.left + l; r.bottom = r.top + h;
	PaintRect(&r);
	RGBForeColor(WndColorText[f->qualite]);
#endif
}
/* ========================================================================== */
void WndPaint(WndFrame f, int x, int y, int l, int h, WndColor *c, int org) {

	if(WndModeNone) return;

#ifdef X11
	/* trouver a peindre selon couleur definie par c
	WndContextPtr gc_rect; WndContextVal gcval; WndScreen d;
	d = (f->s)->d;
	c->flags = DoRed | DoGreen | DoBlue;
	if(XAllocColor(d,DefaultColormap(d,DefaultScreen(d)),c)) {
		gcval->foreground = c->pixel;
		XGetGCValues(d,gc,WndGCMask,&gcval);
		gc_rect = XCreateGC(d,w,WndGCMask,&gcval);
		XSetBackground(d,gc_rect,c->pixel);
		XFillRectangle(d,f->w,gc_rect,f->x0 + x,f->y0 + y,l,h);
		XFreeGC(s->d,gc_rect);
	} else */
		XClearArea(d,f->w,f->x0 + x,f->y0 + y,l,h,0); // utilise le bgnd de <f>
#endif
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	int sizx,sizy; double xd,yd,xf,yf; char doit_terminer;
	WndIdent w; w = f->w;

	glfwGetWindowSize(w,&sizx,&sizy);
	xd = WndXToDouble(f->x0 + x,sizx);
	yd = WndYToDouble(f->y0 + y,sizy);
	xf = WndXToDouble(f->x0 + x + l,sizx);
	yf = WndYToDouble(f->y0 + y + h,sizy);
	doit_terminer = WndRefreshBegin(f);
	glColor3us(c->red,c->green,c->blue);
	glRectd(xd,yd,xf,yf);
	if(doit_terminer) WndRefreshEnd(f);
#endif

#ifdef WIN32
	PAINTSTRUCT paintst; HDC hDC; HPEN hPen, hPenOld; HBRUSH hBrush, hBrushOld;
	hDC = BeginPaint(f->w, &paintst);
	if(WndModeNone) return;
	hPenOld = (HPEN) SelectObject(hDC, hPen = CreatePen(PS_NULL, 0, 0));
	hBrushOld = (HBRUSH) SelectObject(hDC, hBrush = CreateSolidBrush(*c));
	//SelectObject(hDC, CreateSolidBrush(*WndColorBgnd[f->qualite]));
	Rectangle(hDC, f->x0 + x, f->y0 + y, f->x0 + x + l, f->y0 + y + h);
	SelectObject(hDC, hPenOld);
	SelectObject(hDC, hBrushOld);
	EndPaint(f->w, &paintst);
	InvalidateRect(f->w, &(paintst.rcPaint), false);
	DeleteObject(hPen);
	DeleteObject(hBrush);
#endif

#ifdef QUICKDRAW
	Rect r;
	SetPort(WNDPORT(f->w));
	r.left = f->x0 + x; r.top = f->y0 + y;
	r.right = r.left + l; r.bottom = r.top + h;
	RGBForeColor(c);
	PaintRect(&r);
#endif
}
/* ========================================================================== */
void WndBlank(WndFrame f) {

	if(WndModeNone) return;
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	char doit_terminer;
	doit_terminer = WndRefreshBegin(f);
	// a tester: glClearColor(1.0, 1.0, 1.0, 1.0); glClear(GL_COLOR_BUFFER_BIT);
	glColor3us(0x0000,0x0000,0x0000);
	glRectd(-1.0,-1.0,1.0,1.0);
	if(doit_terminer) WndRefreshEnd(f);
#endif

#ifdef X11
	XClearWindow((f->s)->d,f->w);
#endif

#ifdef WIN32
	WndErase(f, 0, 0, f->h + WndBorderSize, f->v+WndBorderSize);
#endif

#ifdef QUICKDRAW
	Rect r; CGrafPtr p;
	p = WNDPORT(f->w); SetPort(p); GetPortBounds(p,&r);
	EraseRect(&r);
#endif
}
/* ========================================================================== */
void WndErase(WndFrame f, int x, int y, int l, int h) {

	if(WndModeNone) return;
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
//	WndRectangle(f,WND_CLEAR,x,y,l,h);
	WndPaint(f,x,y,l,h,WndColorGrey[WND_Q_ECRAN],0);
#endif

#ifdef X11
	XClearArea((f->s)->d,f->w,f->x0 + x,f->y0 + y,l,h,0);
#endif

#ifdef WIN32
	PAINTSTRUCT paintst; HDC hDC; HPEN hPen, hPenOld; HBRUSH hBrush, hBrushOld;

	hDC = BeginPaint(f->w, &paintst);
	//WndColor c = GetBkColor(hDC);
	hPenOld = (HPEN) SelectObject(hDC, hPen = CreatePen(PS_NULL, 0, 0));
	hBrushOld = (HBRUSH) SelectObject(hDC, hBrush = CreateSolidBrush(*WndColorBgnd[f->qualite]));
	Rectangle(hDC, f->x0 + x, f->y0 + y, f->x0 + x+l, f->y0 + y+h);
	SelectObject(hDC, hPenOld);
	SelectObject(hDC, hBrushOld);
	EndPaint(f->w, &paintst);
	InvalidateRect(f->w, &(paintst.rcPaint), false);
	DeleteObject(hPen);
	DeleteObject(hBrush);
#endif

#ifdef QUICKDRAW
	Rect r;
	SetPort(WNDPORT(f->w));
	r.left = f->x0 + x; r.top = f->y0 + y;
	r.right = r.left + l; r.bottom = r.top + h;
	EraseRect(&r);
#endif
}
/* ========================================================================== */
void WndSecteur(WndFrame f, WndContextPtr gc, int x, int y, int l, int h, 
	int a0, int da) {
	WndIdent w; WndServer *s;

	if(WndModeNone) return;
	w = f->w; s = f->s;
	if(gc == 0) gc = WND_TEXT;

#ifdef X11
	int b0,db;
	b0 = (90 - a0) * 64;
	db = -da * 64;
	XFillArc(s->d,f->w,gc,f->x0 + x,f->y0 + y,l,h,b0,db);
#endif

#ifdef WIN32
	using namespace std;
	PAINTSTRUCT paintst; HDC hDC; HPEN hPen, hPenOld; HBRUSH hBrush, hBrushOld;

	hDC = BeginPaint(w, &paintst);
	hPenOld = (HPEN)SelectObject(hDC, hPen = CreatePen(gc->line_style, gc->line_width, (gc->foreground)));
	if(gc->foreground) hBrushOld = (HBRUSH)SelectObject(hDC, hBrush = CreateSolidBrush((gc->foreground)));
	else hBrushOld = (HBRUSH)SelectObject(hDC, hBrush = CreateSolidBrush(*WndColorBgnd[f->qualite]));
	a0 += 90;
	da += a0;
	int ca0 = l * (float)cos( (double)(a0) * (PI/180) );
	int sa0 = h * (float)sin( (double)(a0) * (PI/180) );
	int cda = l * (float)cos( (double)(da) * (PI/180) );
	int sda = h * (float)sin( (double)(da) * (PI/180) );

	SetArcDirection(hDC, AD_COUNTERCLOCKWISE);
	Pie(hDC,f->x0 + x, 
			f->y0 + y, 
			f->x0 + x + l, 
			f->y0 + y + h,
			f->x0 + x + l/2 + ca0,
			f->y0 + y + l/2 + sa0,
			f->x0 + x + l/2 + cda,
			f->y0 + y + l/2 + sda
	);

	SelectObject(hDC, hPenOld);
	SelectObject(hDC, hBrushOld);
	EndPaint(w, &paintst);
	InvalidateRect(w, &(paintst.rcPaint), false);
	DeleteObject(hPen);
	DeleteObject(hBrush);
#endif

#ifdef QUICKDRAW
	Rect r;
	SetPort(WNDPORT(w));
	RGBForeColor(gc->foreground);
	r.left = f->x0 + x; r.top = f->y0 + y; 
	r.right = r.left + l; r.bottom = r.top + h;
	PaintArc(&r,a0,da);
//	RGBForeColor(WndColorText[f->qualite]);
#endif
}
/* ========================================================================== */
void WndRelief(WndFrame f, int epaisseur, int type, int x, int y, int l) {
	WndServer *s;
	int e,d,effet,t1,t2; char exterieur,doit_terminer;

	if(WndModeNone) return;
	s = f->s; effet = type & WND_T; t1 = t2 = -1;
	switch(effet) {
	  case WND_RAINURE:  t1 = WND_GC_DARK; t2 = WND_GC_LITE; break;
	  case WND_REGLETTE: t1 = WND_GC_LITE; t2 = WND_GC_DARK; break;
	  case WND_ECLAIRE:  t1 = WND_GC_LITE; t2 = -1; break;
	  case WND_OMBRE:    t1 = WND_GC_DARK; t2 = -1; break;
	}
	doit_terminer = WndRefreshBegin(f);
	switch(type & WND_XY) {
	  case WND_HORIZONTAL:
		if(t2 >= 0) {
			d = 0;
			e = epaisseur / 2; while(e--) {
				WndLine(f,WND_GC(f,t1),x,y+d,l,0);
				d++;
			}
			e = epaisseur / 2; while(e--) {
				WndLine(f,WND_GC(f,t2),x,y+d,l,0);
				d++;
			}
		} else {
			exterieur = type & WND_EXTERIEUR;
			e = epaisseur;
			if(((effet == WND_ECLAIRE) && exterieur) ||
				((effet == WND_OMBRE) && !exterieur)) {
				d = e;
				while(e--) {
					d--;
					WndLine(f,WND_GC(f,t1),x-d,y-d-1,l+(2*d),0);
				}
			} else {
				d = 0;
				while(e--) {
					WndLine(f,WND_GC(f,t1),x-d,y+d,l+(2*d),0);
					d++;
				}
			}
		}
		break;
	  case WND_VERTICAL:
		if(t2 >= 0) {
			d = 0;
			e = epaisseur / 2; while(e--) {
				WndLine(f,WND_GC(f,t1),x+d,y,0,l);
				d++;
			}
			e = epaisseur / 2; while(e--) {
				WndLine(f,WND_GC(f,t2),x+d,y,0,l);
				d++;
			}
		} else {
			exterieur = type & WND_EXTERIEUR;
			e = epaisseur;
			if(((effet == WND_ECLAIRE) && exterieur) ||
				((effet == WND_OMBRE) && !exterieur)) {
				d = e;
				while(e--) {
					d--;
					WndLine(f,WND_GC(f,t1),x-d-1,y-d,0,l+(2*d));
				}
			} else {
				d = 0;
				while(e--) {
					WndLine(f,WND_GC(f,t1),x+d,y-d,0,l+(2*d));
					d++;
				}
			}
			break;
		}
		break;
	}
	if(doit_terminer) WndRefreshEnd(f);
}
/* ========================================================================== */
void WndEntoure(WndFrame f, char support, int x, int y, int h, int v) {
	char doit_terminer;

	if(WndModeNone) return;
	doit_terminer = WndRefreshBegin(f);
	switch(support) {
	  case WND_LIGNES:
		x = x - 1; y = y - 1;
		h = h + 1; v = v + 1;
		WndLine(f,WND_DARK,x,y,h,0);
		WndLine(f,WND_DARK,x,y,0,v);
		WndLine(f,WND_DARK,x+h,y,0,v);
		WndLine(f,WND_DARK,x,y+v,h,0);
		break;
	  case WND_PLAQUETTE:
		x = x - 2; y = y - 2;
		h = h + 2; v = v + 2;
		//	printf("(%s) Entoure plaquette @(%d, %d) + (%d, %d)\n",__func__,x,y,h,v);
		WndRelief(f,2,WND_BORD_HAUT,x,y,h);
		WndRelief(f,2,WND_BORD_DRTE,x+h,y,v);
		WndRelief(f,2,WND_BORD_BAS, x,y+v,h);
		WndRelief(f,2,WND_BORD_GCHE,x,y,v);
		break;
	  case WND_CREUX:
		// x = x - 2; y = y - 2;
		// h = h + 2; v = v + 2;
		WndRelief(f,2,WND_FOND_HAUT,x,y,h);
		WndRelief(f,2,WND_FOND_DRTE,x+h,y,v);
		WndRelief(f,2,WND_FOND_BAS, x,y+v,h);
		WndRelief(f,2,WND_FOND_GCHE,x,y,v);
		break;
	  case WND_RAINURES:
		x = x - 2; y = y - 2;
		h = h + 2; v = v + 2;
		//	printf("(%s) Entoure rainures @(%d, %d) + (%d, %d)\n",__func__,x,y,h,v);
		WndRelief(f,2,WND_RAINURE | WND_HORIZONTAL,x,y,h);
		WndRelief(f,2,WND_RAINURE | WND_VERTICAL,  x+h,y,v);
		WndRelief(f,2,WND_RAINURE | WND_HORIZONTAL,x,y+v,h);
		WndRelief(f,2,WND_RAINURE | WND_VERTICAL,  x,y,v);
		break;
	  case WND_REGLETTES:
		x = x - 2; y = y - 2;
		h = h + 2; v = v + 2;
		WndRelief(f,2,WND_REGLETTE | WND_HORIZONTAL,x,y,h);
		WndRelief(f,2,WND_REGLETTE | WND_VERTICAL,  x+h,y,v);
		WndRelief(f,2,WND_REGLETTE | WND_HORIZONTAL,x,y+v,h);
		WndRelief(f,2,WND_REGLETTE | WND_VERTICAL,  x,y,v);
		break;
	}
	if(doit_terminer) WndRefreshEnd(f);
}
/* ========================================================================== */
/* ========================= Ecriture en mode texte ========================= */
/* ========================================================================== */
static void WndCursorDraw(WndFrame f, WndContextPtr gc) {
	WndIdent w; WndServer *s; int x,y;

	if(WndModeNone) return;
	if(gc == 0) return;
	w = f->w; s = f->s;
	x = f->col * s->col;
	y = (f->lig + 1) * s->lig;

#ifdef X11
/*	WndDrawLine(f,gc,x,y-(s->lig),x,y); */
	WndDrawLine(f,gc,x,y-1,x+(s->col),y-1);
	WndDrawLine(f,gc,x,y,x+(s->col),y);
#endif
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	char doit_terminer;
	
	doit_terminer = WndRefreshBegin(f);
	WndDrawLine(f,gc,x,y-1,x+(s->col),y-1);
	WndDrawLine(f,gc,x,y,x+(s->col),y);
	if(doit_terminer) WndRefreshEnd(f);
#endif

#ifdef WIN32
	PAINTSTRUCT paintst; HDC hDC; HPEN hPen, hPenOld;

	hDC = BeginPaint(w, &paintst);
	hPenOld = (HPEN) SelectObject(hDC, hPen = CreatePen(gc->line_style, gc->line_width, (gc->foreground)));
	MoveToEx(hDC, x, y-1, 0); LineTo(hDC, x+(s->col), y-1);
	MoveToEx(hDC, x, y, 0); LineTo(hDC, x+(s->col), y);
	SelectObject (hDC, hPenOld);
	EndPaint(w, &paintst);
	InvalidateRect(w, &(paintst.rcPaint), false);
	DeleteObject(hPen);
#endif

#ifdef QUICKDRAW
	SetPort(WNDPORT(f->w));
	RGBForeColor(gc->foreground);
	MoveTo(x,y-1); Line(s->col,0);
	MoveTo(x,y); Line(s->col,0);
//	RGBForeColor(WndColorText[f->qualite]);
#endif
}
/* ========================================================================== */
void WndCursorSet(WndFrame f, int lig, int col) {
	char doit_terminer;

	if(WndModeNone) return;
	if(!f) return;
	doit_terminer = WndRefreshBegin(f);
	if(f->lig >= 0) WndCursorDraw(f,WND_CLEAR);
	f->lig = f->lig0 + lig; f->col = f->col0 + col;
	WndCursorDraw(f,WND_GC(f,WND_GC_CURSOR));
	if(doit_terminer) WndRefreshEnd(f);
}
/* ========================================================================== */
void WndWrite(WndFrame f, WndContextPtr gc, int lig, int col, char *texte) {
	int x,y; size_t l; WndServer *s;

	if(WndModeNone) return;
	if(!f) return;
	s = f->s;
	x = (f->col0 + col) * s->col;
	y = (f->lig0 + lig + 1) * s->lig - s->decal;
	l = strlen(texte);
	// if(gc == 0) gc = WND_TEXT; PRINT_GC(gc);
	WndDrawString(f,gc,x,y,texte,l);
}
/* ========================================================================== */
void WndHBar(WndFrame f, WndContextPtr gc, int lig, int col, int lngr) {
	int x,y,l;
	WndServer *s;

	if(WndModeNone) return;
	if(!f) return;
	s = f->s;
	x = (f->col0 + col) * s->col;
	y = (f->lig0 + lig + 1) * s->lig - (s->lig / 2) - (s->fonte).leading;
	l = lngr * s->col;
	// if(gc == 0) gc = WND_TEXT;
	WndDrawLine(f,gc,x,y,x+l,y);
}
/* ========================================================================== */
void WndVBar(WndFrame f, WndContextPtr gc, int lig, int col, int haut) {
	int x,y,h;
	WndServer *s;

	if(WndModeNone) return;
	if(!f) return;
	s = f->s;
	x = (f->col0 + col) * s->col + (s->col / 2);
	y = (f->lig0 + lig) * s->lig;
	h = haut * s->lig;
	// if(gc == 0) gc = WND_TEXT;
	WndDrawLine(f,gc,x,y,x,y+h);
}
/* ========================================================================== */
void WndButton(WndFrame f, WndContextPtr gc, int lig, int col, int dim, char *texte) {
	WndServer *s; int x,y; char doit_terminer;
#ifdef AQUA
	ControlRef b; Rect r; char titre[80];
#else
	size_t l; int dx;
#endif
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
	if(WndModeNone) return;
	if(!f) return;
	s = f->s;
	x = col * s->col;
	y = lig * s->lig;
#ifdef OPENGL
//	y -= 2;
#endif
	if(gc == 0) gc = WND_TEXT;
#ifdef AQUA
	r.left = f->x0 + x; r.right = r.left + (dim * s->col) - 1;
	r.top = f->y0 + y + 1; r.bottom = r.top + s->lig - 1;
	strcpy(titre+1,texte);
	titre[0] = strlen(texte);
	b = NewControl(f->w,&r,titre,true,0,0,0,0,0);
/*	HiliteControl(b,0x7F); ne sert pas a colorer le bouton */
/*	SetUpControlBackground(b,0xFF,true);  ne marche pas */
#else
	doit_terminer = WndRefreshBegin(f);
	WndOvale(f,gc,x,y+1,(dim * s->col) - 1,s->lig + 1);
	WndOvale(f,WND_DARK,x+1,y+2,(dim * s->col) - 1,s->lig + 1);
	l = strlen(texte); if(l >= dim) l = dim - 1;
	dx = ((dim - (int)l) / 2) * s->col; if(dx < 1) dx = 1;
	x += f->x0 + dx;
	y += f->y0 + s->lig - s->decal + 2;
	WndDrawString(f,gc,x,y,texte,l);
	if(doit_terminer) WndRefreshEnd(f);
#endif
}
/* ========================================================================== */
// return(gc);
/* ========================================================================== */
void WndClearText(WndFrame f, WndContextPtr gc, int lig, int col, int lngr) {
	int x,y,l; WndIdent w; WndServer *s;

	if(WndModeNone) return;
	if(!f) return;
	w = f->w;
	s = f->s;
	x = (f->col0 + col) * s->col;
	y = (f->lig0 + lig) * s->lig;
	l = lngr * s->col;
	if(gc == 0) gc = WND_STD;
#ifdef DEBUG3
	WndPrint("Fenetre #%08X: effacement a (%d, %d) + (%d, %d)\n",f->w,x,y,l,s->lig);
#endif
#ifdef WXWIDGETS
	printf("WND FUNCTION:   %d\n", __LINE__);
	return;
#endif
#ifdef OPENGL
	int sizx,sizy; double xd,yd,xf,yf; char doit_terminer;

//	y -= (3 * (f->s)->lig / 4) + 1;
	if(gc->background) {
		glfwGetWindowSize(w,&sizx,&sizy);
		xd = WndXToDouble(x,sizx);
		yd = WndYToDouble(y,sizy);
		xf = WndXToDouble(x + l,sizx);
		yf = WndYToDouble(y + s->lig,sizy);
		doit_terminer = WndRefreshBegin(f);
		glColor3us((gc->background)->red,(gc->background)->green,(gc->background)->blue);
		// printf("(%s) Colore (%d, %d) + (%d, %d) en %04X/%04X/%04X\n",__func__,x,y,l,s->lig,(gc->background)->red,(gc->background)->green,(gc->background)->blue);
		glRectd(xd,yd,xf,yf);
		if(doit_terminer) WndRefreshEnd(f);
	}
#endif
#ifdef X11
	WndContextPtr gc_reverse; WndContextVal gcval,*ptr; unsigned long tempo;
	// XClearArea(s->d,f->w,x,y,l,s->lig,0);
	ptr = &gcval; /* pour recopier le code de WndContextCreate */
	XGetGCValues(s->d,gc,WndGCMask,ptr);
	tempo = ptr->background; ptr->background = ptr->foreground; ptr->foreground = tempo;
	gc_reverse = XCreateGC(s->d,w,WndGCMask,ptr);
	XFillRectangle(s->d,w,gc_reverse,x,y,l,s->lig);
	XFreeGC(s->d,gc_reverse);
#endif
#ifdef WIN32
	WndErase(f, x, y, l, s->lig);
#endif
#ifdef QUICKDRAW
	Rect r;
	SetPort(WNDPORT(w));
	r.left = x; r.top = y;
	r.right = r.left + l; r.bottom = r.top + s->lig;
	if(gc->background) {
	#ifdef FOND_MODIFIE
		RGBColor c;
		GetBackColor(&c);
		if(gc->background != c) RGBBackColor(gc->background);
		EraseRect(&r);
		if(gc->background != c) RGBBackColor(c);
	#else
		RGBForeColor(gc->background);
		#ifdef DEBUG3
			WndPrint("(WndClearText) Fenetre (%d, %d) -> (%d, %d) peinte\n",
				r.left,r.top,r.right,r.bottom);
		#endif
		PaintRect(&r);
	#endif
	} else EraseRect(&r);
#endif
}
/* ========================================================================== */
void WndSouligne(WndFrame f, WndContextPtr gc, int lig, int col, int lngr) {
	int x,y,l;
	WndServer *s;
	
	if(WndModeNone) return;
	if(!f) return;
	s = f->s;
	x = (f->col0 + col) * s->col;
	y = ((f->lig0 + lig + 1) * s->lig) - 1;
	l = lngr * s->col;
	// if(gc == 0) gc = WND_TEXT; PRINT_GC(gc);
	WndDrawLine(f,gc,x,y,x+l,y);
}
/* ========================================================================== */
void WndBarreDroite(WndFrame f, WndContextPtr gc, int lig, int col, int lngr) {
	int x,y,l;
	WndServer *s;
	
	if(WndModeNone) return;
	if(!f) return;
	s = f->s;
	x = (f->col0 + col) * s->col;
	y = ((f->lig0 + lig + 1) * s->lig) - 1;
	l = lngr * s->col;
	// gc = WND_DARK;
	WndDrawLine(f,gc,x+l-1,y,x+l-1,y-s->lig);
}
/* ========================================================================== */
void WndBarreGauche(WndFrame f, WndContextPtr gc, int lig, int col, int lngr) {
	int x,y,l;
	WndServer *s;
	
	if(WndModeNone) return;
	if(!f) return;
	s = f->s;
	x = (f->col0 + col) * s->col;
	y = ((f->lig0 + lig + 1) * s->lig) - 1;
	l = lngr * s->col;
	// gc = WND_DARK;
	WndDrawLine(f,gc,x,y,x,y-s->lig);
}
/* ========================================================================== */
void WndBell(WndFrame f, int force) {
#ifdef X11
	WndServer *s;

	if(WndModeNone) return;
	if(!f) return;
	s = f->s;
	if(force < -100) force = 100;
	else if(force > 100) force = 100;
	XBell(s->d,force);
#endif

#ifdef WIN32
	MessageBeep(MB_ICONEXCLAMATION);
#endif

#ifdef QUICKDRAW
//	SetSysBeepVolume(force);
//	SysBeep(30);
#if defined(__AudioServices_h__)
	AudioServicesPlayAlertSound(kUserPreferredAlert);
#endif
#endif
}
/* ========================================================================== */
static void WndPShautPage() {
	if(WndPS) {
		fprintf(WndPS,"%%%%Page: %d %d\n",WndPSnum,WndPSnum);
		fprintf(WndPS,"[1 0 0 1 0 %d] concat\n",WND_A4_HAUT);
	}
	WndPSx = WndPSy = 0; WndPSstroked = 1;
	WndPSred = WndPSgreen = WndPSblue = 0xFFFF;
}
/* ========================================================================== */
FILE *WndPSopen(char *nom, char *suite) {
	if(WndModeNone) return((FILE *)1);
	if(WndPS) return((FILE *)1);
	if(!(WndPS = fopen(nom,suite))) return(0);
	if(*suite == 'w') {
		fprintf(WndPS,"%%!PS-Adobe-1.0\n");
		/* Pas obligatoire mais ca fait plus joli... */
		fprintf(WndPS,"%%%%Creator: GROS Michel\n");
		fprintf(WndPS,"%%%%based on OPIUM4 version %s\n",OPIUM_VSN);
		fprintf(WndPS,"%%%%BoundingBox: 0 0 %d %d\n",WND_A4_LARG,WND_A4_HAUT);
		fprintf(WndPS,"%%%%DocumentFonts: Courier\n");
		fprintf(WndPS,"%%%%EndComments\n\n");

		fprintf(WndPS,"/Courier findfont %d scalefont setfont\n",WndFontSize);
		/* Ca non plus [...aussi] */
		fprintf(WndPS,"%%%%EndProlog\n\n");
		WndPSnum = 1;
	}
	WndPShautPage();

	return(WndPS);
}
/* ========================================================================== */
char WndPSopened() { return(WndPS != 0); }
/* ========================================================================== */
static void WndPScolorie(WndColor *c) {
#ifndef WIN32
	if((c->red != WndPSred) || (c->green != WndPSgreen) || (c->blue != WndPSblue)) {
		if(!WndPSstroked) { fprintf(WndPS,"stroke\n"); WndPSstroked = 1; }
		if((c->red == 0xFFFF) && (c->green == 0xFFFF) && (c->blue == 0xFFFF))
			fprintf(WndPS,"0 0 0 setrgbcolor ");
		else fprintf(WndPS,"%g %g %g setrgbcolor ",(float)c->red / 65535.0,
			(float)c->green / 65535.0,(float)c->blue / 65535.0);
		WndPSred = c->red; WndPSgreen = c->green; WndPSblue = c->blue;
	}
#endif
}
/* ========================================================================== */
void WndPSstring(int x, int y, char *texte) {
#ifndef WIN32
	if(WndModeNone) return;
	if(WndPS) {
		WndColor c;
		c.red = c.green = c.blue = 0xFFFF;
		WndPScolorie(&c);
		fprintf(WndPS,"%d %d moveto (%s) show\n",x,-y,texte);
		WndPSstroked = 0;
	}
#endif
}
/* ========================================================================== */
void WndPSposition(int posx, int posy) {
	int dx,dy;

	if(WndModeNone) return;
	if(WndPS) {
		dx = posx - WndPSx; dy = posy - WndPSy;
		if(dx || dy) {
			fprintf(WndPS,"[1 0 0 1 %d %d] concat\n",dx,-dy);
			WndPSx = posx; WndPSy = posy;
		}
	/*	fprintf(WndPS,"%d %d scale\n",echx,echy);  a calculer */
	}
}
/* ========================================================================== */
void WndPSfintrace() { if(WndPS) { fprintf(WndPS,"stroke\n"); WndPSstroked = 1; }}
/* ========================================================================== */
void WndPSpage(WND_PS_SAUTPAGE nouvelle) {
	if(WndModeNone) return;
	if(WndPS) {
		if(!WndPSstroked) { fprintf(WndPS,"stroke\n"); WndPSstroked = 1; }
		if(nouvelle != WND_PS_PREMIERE) { fprintf(WndPS,"showpage\n"); WndPSnum++; }
		if(nouvelle != WND_PS_DERNIERE) WndPShautPage();
	}
}
/* ========================================================================== */
int WndPSpageNb() { return(WndPSnum); }
/* ========================================================================== */
void WndPSclose() { if(WndPS) { fclose(WndPS); 	WndPS = 0; } }
/* ========================================================================== */
#include <errno.h>
//#define LAUNCH
#ifdef DATA_FORK
#include <Files.h>
#endif
#ifdef RESSOURCE_FORK
#include <Resources.h>
#endif
/* ========================================================================== */
int WndLaunch(char *commande) {
#ifdef X11
	return(system(commande));
#endif
#ifdef QUICKDRAW
#ifdef CARBON
	return(system(commande));
#endif
#ifdef DESK_ACC
	int l;
#endif
#ifdef DATA_FORK
	FSSpec fichier;
	char *parms;
	int rc; short ref;

	parms = commande + 1; while(*parms) { if(*parms == ' ') break; parms++; }
	commande[0] = parms - commande - 1;
	while(*parms == ' ') *parms++ = '\0';
	FSMakeFSSpec(0,0,commande,&fichier);
	rc = FSpOpenDF(&fichier,fsCurPerm,&ref);
	WndPrint("(WndLaunch) %s rend %d et refNum=%d\n",commande+1,rc,ref);
	return(rc);
#endif
#ifdef RESSOURCE_FORK
	char *parms;
	int rc;

	parms = commande + 1; while(*parms) { if(*parms == ' ') break; parms++; }
	commande[0] = parms - commande - 1;
	while(*parms == ' ') *parms++ = '\0';
	rc = HOpenResFile(0,0,commande,fsCurPerm);
	WndPrint("(WndLaunch) %s rend %d, en fait %d\n",commande+1,rc,ResError());
	return(rc);
#endif
#ifdef LAUNCH
	LaunchParamBlockRec def;
	FSSpec appli;
#ifdef AVEC_EVENT
	AppParameters evt;
#endif
	char *parms; // char *index();
	int rc;

//	parms = index(commande+1,' ');
//	if(parms) while(*parms == ' ') *parms++ = '\0';
	parms = commande + 1; while(*parms) { if(*parms == ' ') break; parms++; }
	commande[0] = parms - commande - 1;
	while(*parms == ' ') *parms++ = '\0';
	FSMakeFSSpec(0,0,commande,&appli);
#ifdef AVEC_EVENT
	evt.theMsgEvent.what = kHighLevelEvent;
	evt.theMsgEvent.message = 0; /* ?? */
	evt.messageLength = strlen(parms);
	strcpy(evt.messageBuffer,parms);
#endif

	def.launchBlockID = extendedBlock;
	def.launchEPBLength = extendedBlockLen;
	def.launchFileFlags = 0;
	def.launchControlFlags = launchContinue | launchNoFileFlags;
	def.launchAppSpec = &appli;
#ifdef AVEC_EVENT
	def.launchAppParameters = &evt;
#else
	def.launchAppParameters = 0;
#endif

//	return((int)LaunchApplication(&def));
	rc = (int)LaunchApplication(&def);
	WndPrint("(WndLaunch) %s rend %d\n",commande+1,rc);
//	WndPrint("(%s)\n",strerror(errno)); rend no Error!! et (rc) rend unknown error...
	return(rc);
#endif
#ifdef DESK_ACC
	WndPrint("(WndLaunch) Lance '%s'",
				(commande[1]? (char *)(commande+1):
				(commande[2]? (char *)(commande+2):
				(char *)(commande+3))));
	WndPrint(" [");
	for(l=0; l<=commande[0]; l++) {
		if(isprint(commande[l])) WndPrint("%c",commande[l]);
		else WndPrint("<%02x>",commande[l]);
	}
	WndPrint("]\n");
	OpenDeskAcc(commande);
	return(0);
#endif
#endif
	return(0);
}

