#ifndef OPIUM_WX_INTERFACE_H
#define OPIUM_WX_INTERFACE_H

enum SambaEventWx {
    SMBWX_PAINT = 0,
    SMBWX_CONFIG,
    SMBWX_MOUSE_LEFT_DOWN,
    SMBWX_MOUSE_LEFT_UP,
    SMBWX_FOCUS,
    SMBWX_KEY,
    SMBWX_MOUSE_RIGHT_DOWN,
    SMBWX_MOUSE_RIGHT_UP,
    SMBWX_DELETE
};

#ifdef __cplusplus
#define EXTERNC extern "C"
EXTERNC int OpiumManageWx(struct Cadre *cdr_initial, struct SambaWnd* w, SambaEventWx type, int x, int y, int h, int v);
#else
#define EXTERNC
#endif

EXTERNC void InitWxWidgetsApp(int *scr_width, int *scr_height);
EXTERNC void GetFontInfo(short *width, short *ascent, short *descent, short *leading);
EXTERNC int OpiumExecWx(struct Cadre *cdr, struct SambaWnd *w);
EXTERNC void OpiumRefreshAllWindows();
EXTERNC void OpiumCheckThreadRefreshCall();
EXTERNC struct SambaWnd *WndCreateWx(int x, int y, unsigned int width, unsigned int height);
EXTERNC void WndTitleWx(struct SambaWnd *w, char *title);
EXTERNC void WndDrawStringWx(struct SambaWnd *w, int x, int y, char *text, unsigned short fr, unsigned short fg, unsigned short fb, 
                            unsigned short br, unsigned short bg, unsigned short bb, char draw_bg );
EXTERNC void WndDrawRectWx(struct SambaWnd *w, int x, int y, int width, int height, unsigned short r, unsigned short g, unsigned short b);
EXTERNC void WndDrawLineWx(struct SambaWnd *w, int x0, int y0, int x1, int y1, unsigned short r, unsigned short g, unsigned short b);
EXTERNC void WndDrawPolyWx(struct SambaWnd *w, int *x, int *y, int num, unsigned short r, unsigned short g, unsigned short b);
EXTERNC void WndDrawArcWx(struct SambaWnd *w, int x, int y, int width, int height, int start, int stop, unsigned short r, unsigned short g, unsigned short b);
EXTERNC void WndMoveWx(struct SambaWnd *w, int x, int y);
EXTERNC void WndResizeWx(struct SambaWnd *w, int h, int v);
EXTERNC void WndClearAllWx();
EXTERNC void WndClearWx(struct SambaWnd *w);
EXTERNC void WndShowTheTopWx(struct SambaWnd *w);
EXTERNC void WndAssignCursorWx(struct SambaWnd *w, struct wxCursor *c);
EXTERNC struct wxCursor *WndCreateStdCursorWx(int cur_type);
EXTERNC void WndGetWindowSizeWx(struct SambaWnd *w, int *width, int *height);
EXTERNC void LockPaintEvents();
EXTERNC void UnlockPaintEvents();
EXTERNC void WndDestroyImageWx(struct wxImage *img);
EXTERNC struct wxImage *WndCreateImageWx(int width, int height);
EXTERNC void WndSetPixelWx(struct wxImage *img, int x, int y, unsigned short r, unsigned short g, unsigned short b);
EXTERNC void WndImageShowWx(struct SambaWnd *w, struct wxImage *img, int x, int y);
EXTERNC void SetWndExitFuncWX(int (*func)());
EXTERNC int CheckMainWindowCloseWx(struct Cadre *cdr);
#endif