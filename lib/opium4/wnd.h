#ifndef WND_H
#define WND_H

#include <stdio.h>

#include <environnement.h>

#ifdef WND_C
	#define WND_VAR
#else
	#define WND_VAR extern
#endif

#ifdef TIMER_NIVEAU_WND
	#include <timeruser.h>
#endif /* TIMER_NIVEAU_WND */
// #define TIMER_PAR_EVT
#ifdef TIMER_PAR_EVT
	#define TIMER_UNIQUE
#endif /* TIMER_PAR_EVT */

#define WND_RELIEF

#ifdef OS9
	#define X11
	typedef enum { false = 0, true = 1 } BOOLEAN;
#endif /* OS9 */
#ifdef UNIX
	#ifndef cplusplus
		typedef enum { false = 0, true = 1 } BOOLEAN;
	#endif /* cplusplus */
#endif /* UNIX */

#ifdef ForOpiumX
	#define X11
#endif
#if !defined(X11) && !defined(WXWIDGETS)
	#ifdef ForOpiumGL
		#define OPENGL
	#else
		#ifdef darwin
			#define CARBON
			#define QUICKDRAW
		#else
			#define X11
		#endif
	#endif
#endif /* !X11 */

#ifdef macintosh
	#include <EnvironnementApple.h>
#endif /* CARBON */

#ifdef BATCH
	#ifdef X11
		#undef X11
	#endif /* X11 */
	#ifdef OPENGL
		#undef OPENGL
	#endif /* OPENGL */
	#ifdef QUICKDRAW
		#undef QUICKDRAW
	#endif /* QUICKDRAW */
#endif /* BATCH */

#ifdef QUICKDRAW
	#define MENU_BARRE
	#define MENU_BARRE_MAC
	#define QUICKDRAW_OR_OPENGL
#endif /* QUICKDRAW */
#ifdef OPENGL
	#define QUICKDRAW_OR_OPENGL
#endif /* OPENGL */

/*
#ifdef OPENGL
#pragma message("Option dans "__FILE__": OPENGL")
#else
#pragma message("Option dans "__FILE__": PAS OPENGL")
#endif
#ifdef QUICKDRAW
#pragma message("Option dans "__FILE__": QUICKDRAW")
#else
#pragma message("Option dans "__FILE__": PAS QUICKDRAW")
#endif
#ifdef X11
#pragma message("Option dans "__FILE__": X11")
#else
#pragma message("Option dans "__FILE__": PAS X11")
#endif
*/

#include <defineVAR.h>

typedef enum {
	ERREUR = -1, NUL = 0, BON = 1
} RETURN_CODE;
typedef char Bool;

#define WND_MAXCOULEUR 15
#define WND_HTML  (char *)(-3)
#define WND_VT100 (char *)(-2)
#define WND_NONE  (char *)(-1)
#define WND_REFRESH 0x7F

#define WND_A4 (WndServer *)-1
#define WND_PIXEL_CM (72.0 / 2.54)
/* 21 cm fois 72 dpi */
#define WND_A4_LARG 595
/* 29.7 cm fois 72 dpi */
#define WND_A4_HAUT 841

typedef enum {
	WND_PS_DERNIERE = 0,
	WND_PS_NOUVELLE,
	WND_PS_PREMIERE
} WND_PS_SAUTPAGE;

// #define WND_GC(f,index) f->gc[f->qualite].coul[index]

#define PRINT_GC(gc) \
	printf("(%s:%d) GC %s @%08X ->fgnd @%08X, ->bgnd@%08X\n",__func__,__LINE__,chaine_preprocesseur(gc),(unsigned int)gc,\
		gc? (unsigned int)(gc->foreground): 0xfeedc0de,gc? (unsigned int)(gc->background): 0xfeedc0de);
#define PRINT_FGC(f,index) \
	printf("(%s:%d) %08X.gc[%d] @%08X ->fgnd @%08X, ->bgnd@%08X\n",__func__,__LINE__,(unsigned int)f,index,(unsigned int)f->gc[WND_Q_ECRAN].coul[index],\
		f->gc[WND_Q_ECRAN].coul[index]? (unsigned int)(f->gc[WND_Q_ECRAN].coul[index]->foreground): 0xfeedc0de,\
		f->gc[WND_Q_ECRAN].coul[index]? (unsigned int)(f->gc[WND_Q_ECRAN].coul[index]->background): 0xfeedc0de);
#define PRINT_COLOR(couleur) \
	printf("(%s:%d) COUL %s @%08X -> (%04X,%04X,%04X)\n",__func__,__LINE__,chaine_preprocesseur(couleur),(unsigned int)couleur,\
		couleur? couleur->red: 0xcaca,couleur? couleur->green: 0xcaca,couleur? couleur->blue: 0xcaca)

//#ifdef WND_C
/* ------------------------------------------------------------------------ */
/* ------------------------- IMPLEMENTATION WXWIDGETS --------------------- */
/* ------------------------------------------------------------------------ */
#ifdef WXWIDGETS
typedef struct SambaWnd      *WndIdent;
typedef struct SambaApp      *WndScreen;
typedef struct wxImage   *WndSurface,WndTypeSurface;
typedef struct wxCursor      *WndCursor;
typedef int          WndFontId;
typedef int    		 WndFontInfo;
typedef struct {
	unsigned short  red;
	unsigned short  green;
	unsigned short  blue;
} WndColor;
typedef struct {
	short font;
	WndColor *foreground,*background;
	short line_width,line_style;
} WndContextVal,*WndContextPtr;
typedef struct  {
	WndIdent       w;
	UInt16         what;
	unsigned long  message;
	Ulong         when;
	struct {
		short      v;
		short      h;
	}              where;
	UInt16         modifiers;
} WndEvent;
typedef enum {
	WND_EXPOSE = 0,
	WND_MOVED,
	WND_RESIZE,
	WND_CONFIG,
	WND_IN_USE,
	WND_DELETE,
	WND_KEY,
	WND_NOKEY,
	WND_PRESS,
	WND_RELEASE,
	WND_DOUBLE,
	WND_BARRE,
	WND_NBEVENTS
} WND_EVENTTYPE;

typedef enum {
	WND_MSELEFT   = 0,
	WND_MSEMIDDLE,
	WND_MSERIGHT,
	WND_MSEMONTE,
	WND_MSEDESC,
	WND_NBBUTTONS
} WND_MSEBUTTONS;

typedef enum {
	WND_ABS = 0,
	WND_REL = 1
} WND_COORDINATES;

#define WND_ASCINT_WIDZ 15

typedef enum {
	XK_ASCII = 0,
	XK_Alt_L,
	XK_Home,
	XK_KP_F4,
	XK_Left,
	XK_Right,
	XK_Up,
	XK_Down,
	XK_NBKEYS
} WND_KEYS;

typedef enum {
	WND_CURS_STD = 0,
	WND_CURS_FLECHE_DR,
	WND_CURS_FLECHE_GA,
	WND_CURS_CROIX,
	WND_CURS_VISEUR,
	WND_CURS_MAIN
} WND_CURSORS;

#endif

/* ------------------------------------------------------------------------ */
/* ------------------------- IMPLEMENTATION OPENGL ------------------------ */
/* ------------------------------------------------------------------------ */
#ifdef OPENGL
#include <GLUT/glut.h>
// contient glutBitmapHeight mais necessite libglut.dylib et ligGL.dylib juste avant libX11.dylib
//- #include <GL/freeglut.h>
// #include <GLFW/glfw3.h> ne fait pas partie du folder std/librairies/OpenGL
#include <glfw3.h>

#define WND_ASCINT_WIDZ 15
#define WndXToDouble(posx,sizx) (((double)(2 * (posx)) / (double)(sizx)) - 1.0)
#define WndYToDouble(posy,sizy) (1.0 - ((double)(2 * (posy)) / (double)(sizy)))

typedef GLFWmonitor *WndScreen;
typedef GLFWwindow  *WndIdent;
typedef GLFWcursor  *WndCursor;
typedef GLFWimage   *WndSurface,WndTypeSurface;
typedef int          WndFontId;
typedef int    		 WndFontInfo;
typedef struct {
	unsigned short  red;
	unsigned short  green;
	unsigned short  blue;
} WndColor;
typedef struct {
	short font;
	WndColor *foreground,*background;
	short line_width,line_style;
} WndContextVal,*WndContextPtr;
typedef struct  {
	WndIdent       w;
	UInt16         what;
	unsigned long  message;
	Ulong         when;
	struct {
		short      v;
		short      h;
	}              where;
	UInt16         modifiers;
} WndEvent;

// GLFW_ARROW_CURSOR, GLFW_IBEAM_CURSOR, GLFW_CROSSHAIR_CURSOR, GLFW_HAND_CURSOR, GLFW_HRESIZE_CURSOR et GLFW_VRESIZE_CURSOR
typedef enum {
	WND_CURS_STD = GLFW_HAND_CURSOR,
	WND_CURS_FLECHE_DR = GLFW_ARROW_CURSOR,
	WND_CURS_FLECHE_GA = GLFW_ARROW_CURSOR,
	WND_CURS_CROIX = GLFW_CROSSHAIR_CURSOR,
	WND_CURS_VISEUR = GLFW_IBEAM_CURSOR,
	WND_CURS_MAIN = GLFW_HAND_CURSOR
} WND_CURSORS;

typedef enum {
	WND_ABS = 0,
	WND_REL = 1
} WND_COORDINATES;

typedef enum {
	WND_DEV_KBD = 0x1000,
	WND_DEV_MSE = 0x2000,
	WND_DEV_MGR = 0x3000
} WND_DEVICE;
#define WND_DEV_MASK 0xF000
#define WND_DEV_ACTN 0x0FFF

typedef enum {
	WND_EXPOSE = 0,
	WND_MOVED,
	WND_RESIZE,
	WND_CONFIG,
	WND_IN_USE,
	WND_DELETE,
	WND_KEY,
	WND_NOKEY,
	WND_PRESS,
	WND_RELEASE,
	WND_DOUBLE,
	WND_BARRE,
	WND_NBEVENTS
} WND_EVENTTYPE;

typedef enum {
	WND_MSELEFT   = GLFW_MOUSE_BUTTON_LEFT,
	WND_MSEMIDDLE = GLFW_MOUSE_BUTTON_MIDDLE,
	WND_MSERIGHT  = GLFW_MOUSE_BUTTON_RIGHT,
	WND_MSEMONTE  = GLFW_MOUSE_BUTTON_4,
	WND_MSEDESC   = GLFW_MOUSE_BUTTON_5,
	WND_NBBUTTONS
} WND_MSEBUTTONS;
typedef enum {
	XK_ASCII = 0,
	XK_Alt_L = GLFW_MOD_SUPER, // GLFW_MOD_ALT,
	XK_Home,
	XK_KP_F4,
	XK_Left,
	XK_Right,
	XK_Up,
	XK_Down,
	XK_NBKEYS = XK_Down
} WND_KEYS;

/* fin implementation OPENGL */
#endif /* OPENGL */

/* ------------------------------------------------------------------------ */
/* -------------------------- IMPLEMENTATION X11 -------------------------- */
/* ------------------------------------------------------------------------ */
#ifdef X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#define WND_INTERLIGNE 2
#define WND_XMIN 7
#define WND_YMIN 27
#define WND_ASCINT_WIDZ 10

// GC gere les informations via un element "GContext gid". Voir GraphDialogDonnee.

typedef Display      *WndScreen;
typedef XFontStruct  *WndFontInfo;
typedef Font          WndFontId;
typedef XColor        WndColor;
typedef Window        WndIdent;
typedef XGCValues     WndContextVal;
typedef GC            WndContextPtr;
typedef XEvent        WndEvent;
typedef XPoint        WndPoint; /* on veut simplement struct { short x,y } */
typedef XImage       *WndSurface,WndTypeSurface;
typedef Cursor        WndCursor;

typedef enum {
	WND_EXPOSE  = Expose, /* MapNotify, */
	WND_MOVED   = ConfigureNotify,
	WND_RESIZE  = ResizeRequest, /* ReparentNotify, */
	WND_CONFIG  = ConfigureNotify,
	WND_IN_USE  = FocusIn,
	WND_ERASE   = UnmapNotify,
	WND_DELETE  = DestroyNotify,
	WND_KEY     = KeyPress,
	WND_NOKEY   = KeyRelease,
	WND_PRESS   = ButtonPress,
	WND_RELEASE = ButtonRelease,
	WND_DOUBLE,
//	WND_SCROLLX,
//	WND_SCROLLY,
	WND_BARRE,
	WND_NBEVENTS = LASTEvent
} WND_EVENTTYPE;

typedef enum {
	WND_MSELEFT   = Button1,
	WND_MSEMIDDLE = Button2,
	WND_MSERIGHT  = Button3,
	WND_MSEMONTE  = Button4,
	WND_MSEDESC   = Button5,
	WND_NBBUTTONS = 6
} WND_MSEBUTTONS;
typedef enum {
	WND_ABS = CoordModeOrigin,
	WND_REL = CoordModePrevious
} WND_COORDINATES;
typedef enum {
	WND_CURS_STD = XC_X_cursor,
	WND_CURS_FLECHE_DR = XC_arrow,
	WND_CURS_FLECHE_GA = XC_left_ptr,
	WND_CURS_CROIX = XC_crosshair,
	WND_CURS_VISEUR = XC_cross,
	WND_CURS_MAIN = XC_hand2
} WND_CURSORS;

/* fin implementation X11 */
#endif /* X11 */

/* ------------------------------------------------------------------------ */
/* -------------------------- IMPLEMENTATION Windows ---------------------- */
/* ------------------------------------------------------------------------ */
#ifdef WIN32

#include <afxwin.h>
#include <afx.h>
#include <defineVAR.h>

#define WND_XMIN 7 /* ... */
#define WND_YMIN 27 /* ... */
#define WND_ASCINT_WIDZ 10 /* ... */
#define WND_INTERLIGNE 2

WND_VAR short WndTitleBar;
WND_VAR short WndBorderSize;

#define PI 3.14159265358979

typedef enum {
	WND_EXPOSE = WM_PAINT,
	WND_MOVED   = ConfigureNotify
	WND_RESIZE = WM_NCLBUTTONDOWN,
	WND_CONFIG = WM_NCLBUTTONDOWN,
	WND_IN_USE = WM_MOUSEACTIVATE,
	WND_ERASE = WM_PENWINLAST+1,
	WND_DELETE = WM_CLOSE,
	WND_KEY = WM_CHAR,
	WND_NOKEY = WM_KEYUP,
	WND_PRESS = WM_LBUTTONDOWN,
	WND_RELEASE = WM_LBUTTONUP,
	WND_DOUBLE = 514,
//	WND_SCROLLX = WM_HSCROLL,
//	WND_SCROLLY = WM_VSCROLL,
	WND_REDRAW = WM_PENWINLAST+2,
	WND_BARRE = WM_PENWINLAST+3,
	WND_NBEVENTS = 515
} WND_EVENTTYPE;

typedef enum {
	XK_ASCII = 0,
	XK_Alt_L = VK_MENU,
	XK_Home = VK_HOME,
	XK_KP_F4 = VK_F4,
	XK_Left = VK_LEFT,
	XK_Right = VK_RIGHT,
	XK_Up = VK_UP,
	XK_Down = VK_DOWN,
	XK_NBKEYS = 8
} WND_KEYS;

typedef enum {
	WND_MSELEFT = MK_LBUTTON,
	WND_MSEMIDDLE = MK_MBUTTON,
	WND_MSERIGHT = MK_RBUTTON,
	WND_MSEMONTE  = 4,
	WND_MSEDESC   = 5,
	WND_NBBUTTONS = 4
} WND_MSEBUTTONS;

typedef enum {
	WND_ABS = 0,
	WND_REL = 1
} WND_COORDINATES;

typedef HWND		WndIdent;
typedef POINT		WndPoint;
typedef COLORREF	WndColor;
typedef HFONT		WndFontId;
typedef HCURSOR		WndCursor;
typedef MSG		    WndEvent;
typedef INT		    WndScreen; /* ... */
typedef HFONT		WndFontInfo; /* ... */

typedef struct {
	HFONT		font;
	WndColor	foreground;
	WndColor	background;
	int		line_width;
	int		line_style;
} WndContextVal, *WndContextPtr;

typedef struct {
	HDC hDC;
	PAINTSTRUCT paintStruct;
	int width, height;
	unsigned char *pixels;
} *WndSurface,WndTypeSurface;

#define WND_CURS_MAIN IDC_HAND
#define WND_CURS_STD IDC_ARROW
#define WND_CURS_FLECHE_DR IDC_ARROW
#define WND_CURS_FLECHE_GA IDC_ARROW
#define WND_CURS_CROIX IDC_CROSS
#define WND_CURS_VISEUR IDC_CROSS

#ifdef WIN32_DEBUG
	void OpiumWin32Log(char *fmt);
#endif

/* fin implementation Windows */
#endif /* WIN32 */

/* ------------------------------------------------------------------------ */
/* ----------------------- IMPLEMENTATION QUICKDRAW ----------------------- */
/* ------------------------------------------------------------------------ */
#ifdef QUICKDRAW

#ifdef CARBON
	#ifdef PROJECT_BUILDER
		#include <Carbon/Carbon.h>
	#else  /* PROJECT_BUILDER */
		#ifdef XCODE
			//?? #define __CARBONSOUND__
/*
#undef  __OSX_AVAILABLE_STARTING
#define __OSX_AVAILABLE_STARTING(i,j)
#undef  __OSX_AVAILABLE_BUT_DEPRECATED_MSG
#define __OSX_AVAILABLE_BUT_DEPRECATED_MSG(i,j,k,l,m)
*/
			#include <Carbon/Carbon.h>
			// Ou est-il??? #include <CoreServices/CarbonCore/CarbonCore.h>
			#include <CoreServices/CoreServices.h>
			#include <CoreGraphics/CoreGraphics.h>
//			#include "/System/Library/Frameworks/ApplicationServices.framework/Frameworks/QD.framework/Headers/QD.h"
		#else  /* XCODE */
			#include <Carbon.h>
		#endif /* XCODE */
	#endif /* PROJECT_BUILDER */
#else /* CW5 */
	#include <Quickdraw.h>
	/* Ajout pour G4MiG et Samba/G3 (mais pas Tango/G3...): */
	#include <Events.h>
#endif /* CARBON */

#include <defineVAR.h>

//#ifdef CW5
typedef short pixval;
//#else
//typedef int pixval;
//#endif

#define WND_ASCINT_WIDZ 15
#define WND_INFRONT (WindowPtr)-1
#define WND_BEHIND  NIL

/* Version 1: typedef QDGlobals    *WndScreen; */
typedef BitMap        WndScreen;
typedef FontInfo      WndFontInfo;
typedef int           WndFontId;
typedef RGBColor      WndColor;
#ifdef CARBON
	typedef WindowRef     WndIdent;
#else
	typedef WindowPtr     WndIdent;
#endif
typedef struct {
	int width,height;
	unsigned char *pixels;
} *WndSurface,WndTypeSurface;

typedef struct {
	short font;
	WndColor *foreground,*background;
	short line_width,line_style;
} WndContextVal,*WndContextPtr;
typedef EventRecord   WndEvent;
typedef int WndCursor;

typedef enum {
	WND_EXPOSE = 0,
	WND_MOVED,
	WND_RESIZE,
	WND_CONFIG,
	WND_IN_USE,
	WND_DELETE,
	WND_KEY,
	WND_NOKEY,
	WND_PRESS,
	WND_RELEASE,
	WND_DOUBLE,
//	WND_SCROLLX,
//	WND_SCROLLY,
//	#ifdef MENU_BARRE
	WND_BARRE,
//	#endif
	WND_NBEVENTS = kHighLevelEvent+1
} WND_EVENTTYPE;
typedef enum {
	WND_MSELEFT = 1,
	WND_MSEMIDDLE,
	WND_MSERIGHT,
	WND_MSEMONTE,
	WND_MSEDESC,
	WND_NBBUTTONS = 4
} WND_MSEBUTTONS;
typedef enum {
	XK_ASCII = 0,
	XK_Alt_L,
	XK_Home,
	XK_KP_F4,
	XK_Left,
	XK_Right,
	XK_Up,
	XK_Down,
	XK_NBKEYS = XK_Down
} WND_KEYS;
typedef enum {
	WND_CURS_STD = 0,
	WND_CURS_FLECHE_DR,
	WND_CURS_FLECHE_GA,
	WND_CURS_CROIX,
	WND_CURS_VISEUR,
	WND_CURS_MAIN
} WND_CURSORS;
typedef enum {
	WND_ABS = 0,
	WND_REL = 1
} WND_COORDINATES;

/* Gestion du MenuBar */
#define MENU_POMME   32000
#define   ITEM_APROPOS   1
#define MENU_FICHIER 32001
#define   ITEM_NOUVEAU   1
#define   ITEM_OUVRIR    2
#define   ITEM_FERMER    3
#define   ITEM_SAUVER    4
#define   ITEM_PAGE      6
#define   ITEM_IMPRIMER  7
#define   ITEM_QUITTER   9
#define MENU_EDITER  32002
#define   ITEM_ANNULER   1
#define   ITEM_COUPER    3
#define   ITEM_COPIER    4
#define   ITEM_COLLER    5
#define   ITEM_EFFACER   6
#define   ITEM_SELECTALL 8

/* fin implementation MAC */
#endif /* QUICKDRAW */

//#endif /* WND_C */

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

#define WND_COLOR_MIN     0x0000
#define WND_COLOR_MAX     0xFFFF
#define WND_COLOR_DEMI    0x7FFF
#define WND_COLOR_QUART   0x3FFF
#define WND_COLOR_3QUARTS 0xBFFF

typedef enum {
	WND_CHECK = 0,
	WND_WAIT = 1
} WND_EVENT_MODE;

typedef enum {
	WND_FEN_STD = 0,
	WND_FEN_SUB,
	WND_FEN_BRD
} WND_TYPE;

typedef enum {
	WND_GC_STD = 0,
	WND_GC_CURSOR,
	WND_GC_ASCR,
	WND_GC_TEXT,
	WND_GC_CLEAR,
	WND_GC_LITE,
	WND_GC_DARK,
	WND_GC_SELECTED,

	WND_GC_HILITE,
	WND_GC_MODS,
	WND_GC_REVERSE,
	WND_GC_LUMIN,
	WND_GC_BOARD,
	WND_GC_GREY,
	WND_GC_BUTN,
	WND_GC_SDRK,
	WND_GC_NOIR,
	WND_GC_VERT,
	WND_GC_ROUGE,
	WND_GC_ORANGE,
	WND_GC_NB
} WND_GC_COUL;

#define WND_GC_BASE WND_GC_SELECTED+1

typedef enum {
	WND_Q_PAPIER = 0,
	WND_Q_ECRAN,
	WND_NBQUAL
} WND_QUALITE;
WND_VAR char *WndSupportNom[WND_NBQUAL+1]
#ifdef WND_C
 = { "papier", "ecran", "\0" }
#endif
;
WND_VAR char WndSupportCles[16];

#define WND_GC(f,index) f->gc[f->qualite].coul[index]
// #define WndContext WND_GC

#define WND_STD      WND_GC(f,WND_GC_STD)
#define WND_CURSOR   WND_GC(f,WND_GC_CURSOR)
#define WND_ASCR     WND_GC(f,WND_GC_ASCR)
#define WND_TEXT     WND_GC(f,WND_GC_TEXT)
#define WND_CLEAR    WND_GC(f,WND_GC_CLEAR)
#define WND_LITE     WND_GC(f,WND_GC_LITE)
#define WND_DARK     WND_GC(f,WND_GC_DARK)
#define WND_SELECTED WND_GC(f,WND_GC_SELECTED)
/*
#define WND_HILITE   WND_GC(f,WND_GC_HILITE)
#define WND_MODS     WND_GC(f,WND_GC_MODS)
#define WND_REVERSE  WND_GC(f,WND_GC_REVERSE)
#define WND_LUMIN    WND_GC(f,WND_GC_LUMIN)
#define WND_GREY     WND_GC(f,WND_GC_GREY)
#define WND_BUTN     WND_GC(f,WND_GC_BUTN)
#define WND_SDRK     WND_GC(f,WND_GC_SDRK)
#define WND_VERT     WND_GC(f,WND_GC_VERT)
#define WND_ROUGE    WND_GC(f,WND_GC_ROUGE)
#define WND_ORANGE   WND_GC(f,WND_GC_ORANGE)
 */

#define WND_XY 0x01
#define WND_Z  0x02
#define WND_T  0x0C
#define WND_HORIZONTAL 0x00
#define WND_VERTICAL   0x01
#define WND_INTERIEUR  0x00
#define WND_EXTERIEUR  0x02
#define WND_RAINURE    0x00
#define WND_REGLETTE   0x04
#define WND_ECLAIRE    0x08
#define WND_OMBRE      0x0C
#define WND_BORD_HAUT  (WND_HORIZONTAL | WND_ECLAIRE | WND_EXTERIEUR)
#define WND_BORD_DRTE  (WND_VERTICAL   | WND_OMBRE   | WND_EXTERIEUR)
#define WND_BORD_BAS   (WND_HORIZONTAL | WND_OMBRE   | WND_EXTERIEUR)
#define WND_BORD_GCHE  (WND_VERTICAL   | WND_ECLAIRE | WND_EXTERIEUR)
#define WND_FOND_HAUT  (WND_HORIZONTAL | WND_OMBRE   | WND_INTERIEUR)
#define WND_FOND_DRTE  (WND_VERTICAL   | WND_ECLAIRE | WND_INTERIEUR)
#define WND_FOND_GCHE  (WND_VERTICAL   | WND_OMBRE   | WND_INTERIEUR)
#define WND_FOND_BAS   (WND_HORIZONTAL | WND_ECLAIRE | WND_INTERIEUR)

typedef enum {
	WND_RIEN = 0,
	WND_LIGNES,
	WND_PLAQUETTE,
	WND_CREUX,
	WND_RAINURES,
	WND_REGLETTES,
	WND_NBSUPPORTS
} WND_SUPPORTS;

typedef struct {
	short x,y;
} WndPoint;

typedef struct {
	WndFontId id;
	short width,ascent,descent,leading;
} WndFont;

typedef unsigned short WndColorLevel;
#define WND_COLORMAX ((1 << (sizeof(WndColorLevel) * 8)) - 1)
#define WND_COLOR100 ((float)WND_COLORMAX)
#define WND_COLORFLOAT (100.0 / WND_COLOR100)

typedef struct {
	WndScreen d;
	int larg,haut;
	WndFont fonte;
	struct { WndContextVal coul[WND_GC_NB]; } gc[WND_NBQUAL];
	int lig,col,decal;
} WndServer;

typedef struct {
	struct {
		unsigned short ident;
		unsigned int taille;
		unsigned int reserve;
		unsigned int offset;
	} fichier;
	struct {
		unsigned int taille;
		unsigned int larg,haut;
		unsigned short plans;
		unsigned short bits_par_couleur; // 1 (2 coul), 4 (16 coul), 8 (256 coul), 16, 32
		unsigned int codage;             // 1=8bits/pixel, 2=4bits/pixel, 3=reference de palette
		unsigned int total;
		unsigned int h_resol,v_resol;
		unsigned int dim_palette;
		unsigned int nb_ppales;
	} image;
} TypeWndBmpHeader;
typedef struct {
	unsigned char b,g,r,x;
} TypeWndBmpPalette,*WndBmpPalette;
typedef struct {
	unsigned char b,g,r;
} TypeWndBmpPixel24,*WndBmpPixel24;
//typedef struct {
//	unsigned short b,g,r;
//} TypeWndBmpPixel16,*WndBmpPixel16;
typedef struct {
	unsigned int x,b,g,r;
} TypeWndBmpPixel32,*WndBmpPixel32;

typedef enum { ICONE_H = 0, ICONE_FIC, ICONE_BMP } ICONE_TYPE;
typedef struct {
	char m;
	unsigned char r,g,b;
} TypeWndIcnPxl,*WndIcnPxl;
typedef struct {
	short larg,haut;
	WndIcnPxl pixel;
} TypeWndIcone,*WndIcone;

typedef struct {
	unsigned char  r;
	unsigned char  g;
	unsigned char  b;
} WndCol8bits;

typedef struct {
	int dx,dy;
	WndColor *lut; int nbcolors;
	unsigned short *R,*G,*B,*A;
	WndSurface surf;
} WndImage;

typedef enum {
	WND_ACTN_PAINT = 0,
	WND_ACTN_WRITE,
	WND_ACTN_STRING,
	WND_ACTN_LINE,
	WND_ACTN_NB
} WND_ACTION;
typedef struct {
	WndContextPtr gc;
	int i,j,k,l;
	void *info;
	char action;
} WndTypeExtraActn;

typedef struct WndFrameStruct {
	WndServer *s;
	WndIdent w;
	int nb_gc;
	struct { WndContextPtr *coul; } gc[WND_NBQUAL];
	int x,y,h,v;               /* taille                                             */
	int x0,y0,xm,ym;           /* limites temporaires (sous-fenetre)                 */
	int lig0,col0,ligm,colm;   /* idem, en termes de lignes/colonnes                 */
	short lig,col;             /* position du curseur (-1 si pas de curseur affiche) */
	char en_cours;
	char passive;
	char qualite;
	WndImage image;
	WndTypeExtraActn *extras;
	short extra_nb,extra_max;
#ifdef TIMER_NIVEAU_WND
	#ifdef TIMER_PAR_EVT
		#ifdef TIMER_UNIQUE
			unsigned char delai,restant;
		#else
			Timer timer;
		#endif
	#endif
#endif
	struct WndFrameStruct *dessous;
} *WndFrame;
#define WND_NOT_IN_STACK (WndFrame)-1
#define WND_AT_END (WndFrame)0

#define WND_MAXREQUEST 80
typedef struct WndUserRequestStruct {
	short  type;
	WndIdent w;
	int   x,y,h,v;
	unsigned long code;
	char  texte[WND_MAXREQUEST];
	struct WndUserRequestStruct *suivant;
} WndUserRequest;
#define WND_NOREQUEST (WndUserRequest *)0

WND_VAR WndUserRequest WndEventPrecedent;

#define WND_MAXUSERKEYS 32
typedef struct {
	int   code;
	char  texte[WND_MAXREQUEST];
	int (*fctn)(); /* un retour int peut etre demande par un menu opium */
} WndUserKeyType;

WND_VAR int WndLogX
#ifdef WND_C
 = 500
#endif
;
WND_VAR int WndLogY
#ifdef WND_C
 = 45
#endif
;
WND_VAR int WndLogLines
#ifdef WND_C
 = 50
#endif
;
WND_VAR int WndLogCols
#ifdef WND_C
 = 80
#endif
;
WND_VAR char WndLogSave
#ifdef WND_C
 = 1
#endif
;
#define WND_LOGLNGR 80
WND_VAR char WndLogName[WND_LOGLNGR]
#ifdef WND_C
 = "\07Journal"
#endif
;
WND_VAR char WndCodeHtml;
WND_VAR char WndQual;
WND_VAR char WndModeNone;
WND_VAR char WndModeVt100;
WND_VAR char WndInitVt100
#ifdef WND_C
= 1
#endif
;
WND_VAR WndContextVal WndPeintureGrise;
WND_VAR char WndQueryExit;
WND_VAR WndIdent  WndRefreshed;
#ifdef TIMER_PAR_EVT
#ifndef TIMER_UNIQUE
WND_VAR Timer WndTimerUsed;
#endif
#endif
WND_VAR WndServer WndDefSvr,*WndCurSvr;
WND_VAR char WndDisplayInUse[MAXFILENAME];
WND_VAR WndFrame WndTopFrame,WndLastWriteOnly;
WND_VAR WndUserKeyType WndUserKey[WND_MAXUSERKEYS];

WND_VAR WndColorLevel WndLevelBgnd[WND_NBQUAL],WndLevelText[WND_NBQUAL];

WND_VAR WndColor *WndColorBlack,*WndColorWhite;
WND_VAR WndColor *WndColorBgnd[WND_NBQUAL],*WndColorText[WND_NBQUAL];
WND_VAR WndColor *WndColorMods[WND_NBQUAL],*WndColorHilite[WND_NBQUAL];
WND_VAR WndColor *WndColorAscr[WND_NBQUAL],*WndColorActif[WND_NBQUAL],*WndColorSub[WND_NBQUAL],*WndColorBoard[WND_NBQUAL];
WND_VAR WndColor *WndColorButn[WND_NBQUAL],*WndColorLumin[WND_NBQUAL];
WND_VAR WndColor *WndColorLite[WND_NBQUAL],*WndColorDark[WND_NBQUAL],*WndColorSuperdark[WND_NBQUAL],*WndColorGrey[WND_NBQUAL];
WND_VAR WndColor *WndColorNoir[WND_NBQUAL],*WndColorRouge[WND_NBQUAL],*WndColorOrange[WND_NBQUAL];
WND_VAR WndColor *WndColorNote[WND_NBQUAL],*WndColorWarn[WND_NBQUAL],*WndColorErrr[WND_NBQUAL];
WND_VAR int WndColorNb;

WND_VAR int WndUserKeyNb;
WND_VAR char WndMenuAffiche;

#ifdef WIN32
	int WndPrint(const char *szfmt, ...);
	#define BYTESWAP
	#define WIN32_BACKGROUND
	#define DEBUG_CONSOLE_SIZE 500
#else
	#define WndPrint printf
#endif

void WndStep(char *fmt, ...);
void WndLog(int x, int y, int ligs, int cols, char *nom);
void WndSetFontName(char *chaine);
void WndSetFontSize(int taille);
void WndSetBgndBlack(char noir);
char WndSetQualite(char qual);
void WndSetColors(char *bgnd, char *text, char *hilite, char *mods);
Bool WndOpen(WndServer *s, char *display);
Bool WndInit(char *display);
void WndFenColorNb(WndFrame f, int nb);
void WndFenColorSet(WndFrame f, int num, int couleur);
void WndFenColorDump(WndFrame f, int qual);
void WndExit();
void WndJournalTitle(char *texte);
int WndTotalWidth(WndServer *svr);
int WndTotalHeigth(WndServer *svr);
void WndAPropos(char *appli, void (*fctn)());
void WndContextSetColors(WndFrame f, WndContextPtr gc, WndColor *fond, WndColor *text);
#ifdef MENU_BARRE
	void WndMenuDebug(char dbg);
	int  WndMenuCreate(char *texte);
	void WndMenuItem(char *texte, char suite, char sel);
	void WndMenuSepare();
	void WndMenuInsert();
	void WndMenuClear(char *titre);
	void WndMenuDisplay();
#endif
int WndLigToPix(int lig);
int WndColToPix(int col);
int WndPixToLig(int pix);
int WndPixToCol(int pix);
WndFrame WndDisplay(int qual, int posx, int posy, int sizx, int sizy);
WndFrame WndAttach(int qual, int posx, int posy, int sizx, int sizy);
WndFrame WndCreate(int type, int qual, int posx, int posy, int sizx, int sizy);
void WndMinimale(WndFrame f);
void WndRepere(WndFrame f, int x0, int y0, int xm, int ym);
void WndMask(WndFrame f, int x0, int y0, int xm, int ym);
void WndTitle(WndFrame f, char *titre);
void WndStackPrePrint(char *routine, WndFrame f);
char WndBasicRGB(WndFrame f, int fr, int fg, int fb, int tr, int tg, int tb);
char WndRefreshBegin(WndFrame f);
void WndRefreshEnd(WndFrame f);
void WndControls(WndFrame f);
void WndBorders(WndFrame f);
WndCursor WndCreateStdCursor(int num);
WndCursor WndCreateUserCursor(int larg, int haut, unsigned char *map);
void WndAssignCursor(WndFrame f, WndCursor curseur);
void WndExtraInit(WndFrame f, int max);
void WndExtraAdd(WndFrame f, char action, WndContextPtr gc, int i, int j, int k, int l, void *info);
short WndExtraNb(WndFrame f);
void WndExtraDisplay(WndFrame f);
void WndStackPrePrint(char *routine, WndFrame f);
void WndResize(WndFrame f);
void WndRaise(WndFrame f);
void WndPutAtTop(WndFrame f);
void WndShowTheTop();
void WndSendToTop(WndFrame f);
#ifdef X11
	#define WndUpdateBegin(w)
	#define WndUpdateEnd(w)
#else
	void WndUpdateBegin(WndIdent w);
	void WndUpdateEnd(WndIdent w);
#endif
void WndWriteOnlySet(WndFrame f);
#ifdef TIMER_NIVEAU_WND
#ifdef TIMER_PAR_EVT
#ifdef TIMER_UNIQUE
	void WndWriteOnlyDump();
	void WndWriteOnlyRefresh(WndFrame f, unsigned char delai) ;
#else
	void WndRefreshAuto(WndFrame f, unsigned char delai);
	void WndRefreshStop(WndFrame f);
#endif
#endif
#endif
void WndMove(WndFrame f, int x, int y);
void WndBlank(WndFrame f);
void WndClear(WndFrame f);
int WndUserKeyAdd(int code, char *texte, int (*fctn)());
void WndEventQueue(char type, WndIdent w);
int WndEventNew(WndUserRequest *u, WND_EVENT_MODE mode);
#ifdef QUICKDRAW
	char WndUpdateFlushed(WndUserRequest *u);
#else
	#define WndUpdateFlushed(u) 1
#endif
int WndEventReady(WndUserRequest *u);
//int WndReady(int evt, WndUserRequest *u);
int WndEventWait(int evt, WndUserRequest *u);
void WndSend(WndFrame f, int type, int x, int y, int h, int v);
WndColor *WndColorGetFromName(char *nom);
WndColor *WndColorGetFromRGB(WndColorLevel r, WndColorLevel g, WndColorLevel b);
char WndColorSetByName(WndColor *c, char *nom);
char WndColorSetByRGB(WndColor *c, WndColorLevel r, WndColorLevel g, WndColorLevel b);
void WndColorFree(WndColor *c);
WndContextPtr WndContextCreate(WndFrame f);
WndContextPtr WndContextSupportCreate(WndFrame f, int qual);
WndContextPtr WndContextCreateFromVal(WndFrame f, WndContextVal *gcval);
void WndContextCopy(WndFrame f, WndContextPtr gc_src, WndContextPtr gc_dest);
void WndContextFree(WndFrame f, WndContextPtr gc);
char WndContextFgndName(WndFrame f, WndContextPtr gc, char *nom);
char WndContextFgndRGB(WndFrame f, WndContextPtr gc,WndColorLevel r, WndColorLevel g, WndColorLevel b);
char WndContextBgndRGB(WndFrame f, WndContextPtr gc,WndColorLevel r, WndColorLevel g, WndColorLevel b);
char WndContextFgndColor(WndFrame f, WndContextPtr gc, WndColor *c);
char WndContextBgndColor(WndFrame f, WndContextPtr gc, WndColor *c);
void WndContextLine(WndFrame f, WndContextPtr gc, unsigned int type, unsigned int width);
void WndImageInit(WndFrame f, int larg, int haut, WndColor *lut, int dim);
void WndImageOffset(WndFrame f, int dx, int dy);
void WndImagePixel(WndFrame f, int x, int y, int val);
void WndImageShow(WndFrame f);
WndIcone WndIconeFromBmpFile(char *nom_complet, char *explics);
WndIcone WndIconeRead(char *nom_complet, char *explics);
char WndIconeSave(WndIcone icone, char type, char *nom, char *chemin, char *explics);
WndIcone WndIconeCompactee(WndIcone srce, int facteur);
void WndIconeDelete(WndIcone icone);
void WndIconeInit(WndFrame f, WndIcone icone);
void WndIconePixel(WndFrame f, int x, int y, WndIcnPxl pixel);
void WndString(WndFrame f, WndContextPtr gc, int x, int y, char *texte);
void WndLine(WndFrame f, WndContextPtr gc, int x, int y, int l, int h);
void WndPoly(WndFrame f, WndContextPtr gc, WndPoint *p, int nb, int type);
void WndRectangle(WndFrame f, WndContextPtr c, int x, int y, int l, int h);
void WndRelief(WndFrame f, int epaisseur, int type, int x, int y, int l);
void WndPaint(WndFrame f, int x, int y, int l, int h, WndColor *c, int org);
void WndFillFgnd(WndFrame f, WndContextPtr gc, int x, int y, int l, int h);
void WndFillBgnd(WndFrame f, WndContextPtr gc, int x, int y, int l, int h);
void WndOvale(WndFrame f, WndContextPtr c, int x, int y, int l, int h);
void WndSecteur(WndFrame f, WndContextPtr gc, int x, int y, int l, int h,
	int a0, int da);
void WndArc(WndFrame f, WndContextPtr gc, int x, int y, int l, int h,
	int a0, int da);
void WndEntoure(WndFrame f, char support, int x, int y, int h, int v);
void WndErase(WndFrame f, int x, int y, int l, int h);
void WndCursorSet(WndFrame f, int lig, int col);
void WndWrite(WndFrame f, WndContextPtr gc, int lig, int col, char *texte);
void WndHBar(WndFrame f, WndContextPtr gc, int lig, int col, int lngr);
void WndVBar(WndFrame f, WndContextPtr gc, int lig, int col, int haut);
void WndButton(WndFrame f, WndContextPtr gc, int lig, int col, int dim, char *texte);
void WndClearText(WndFrame f, WndContextPtr gc, int lig, int col, int lngr);
void WndSouligne(WndFrame f, WndContextPtr gc, int lig, int col, int lngr);
void WndBarreDroite(WndFrame f, WndContextPtr gc, int lig, int col, int lngr);
void WndBarreGauche(WndFrame f, WndContextPtr gc, int lig, int col, int lngr);
void WndBell(WndFrame f, int force);
int WndLaunch(char *commande);
FILE *WndPSopen(char *nom, char *suite);
char WndPSopened();
void WndPSstring(int x, int y, char *texte);
void WndPSposition(int posx, int posy);
void WndPSfintrace();
void WndPSpage(WND_PS_SAUTPAGE nouvelle);
int WndPSpageNb();
void WndPSclose();

/* ............................. wnddebug.h ................................. */

WND_VAR char *WndEventName[WND_NBEVENTS]
#ifdef WND_C
= {
#ifdef X11
	"None",
	"Undefined",
	"KeyPress",
	"KeyRelease",
	"ButtonPress",
	"ButtonRelease",
	"MotionNotify",
	"EnterNotify",
	"LeaveNotify",
	"FocusIn",
	"FocusOut",
	"KeymapNotify",
	"Expose",
	"GraphicsExpose",
	"NoExpose",
	"VisibilityNotify",
	"CreateNotify",
	"DestroyNotify",
	"UnmapNotify",
	"MapNotify",
	"MapRequest",
	"ReparentNotify",
	"ConfigureNotify",
	"ConfigureRequest",
	"GravityNotify",
	"ResizeRequest",
	"CirculateNotify",
	"CirculateRequest",
	"PropertyNotify",
	"SelectionClear",
	"SelectionRequest",
	"SelectionNotify",
	"ColormapNotify",
	"ClientMessage",
	"MappingNotify"
#endif /* X11 */
#ifdef QUICKDRAW
	"nullEvent",
	"mouseDown",
	"mouseUp",
	"keyDown",
	"keyUp",
	"autoKey",
	"updateEvt",
	"diskEvt",
	"activateEvt",
	"type9Evt",
	"type10Evt",
	"type11Evt",
	"type12Evt",
	"type13Evt",
	"type14Evt",
	"osEvt",
	"type16Evt",
	"type17Evt",
	"type18Evt",
	"type19Evt",
	"type20Evt",
	"type21Evt",
	"type22Evt",
	"kHighLevelEvt"
#endif /* QUICKDRAW */
}
#endif /* WND_C */
;

/* Correct seulement pour MAC */
WND_VAR char *WndTypeName[]
#ifdef WND_C
 = {
	"Expose", "Resize", "Config", "InUse", "Delete", "Key", "NoKey", "Press", "Release", "Double",
	/* "ScrollX", "ScrollY", */ "Barre",
}
#endif
;

#endif
