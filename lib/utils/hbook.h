/* Interface to the HBOOK package */#ifndef _HBOOK_H#define _HBOOK_Hstatic int HBK_tl,           HBK_id,           HBK_nx,           HBK_ny,           HBK_i,           HBK_ndim,           HBK_nprime;static float HBK_xmi,             HBK_xma,             HBK_ymi,             HBK_yma,             HBK_vmx,             HBK_x,             HBK_y,             HBK_w;#define HLIMIT(I)  \         HBK_i = I;  \         hlimit_(&HBK_i)#define HBOOK1(ID,CHTITL,NX,XMI,XMA,VMX) \         HBK_id = ID;      \         HBK_nx = NX;      \         HBK_xmi= XMI;     \         HBK_xma= XMA;     \         HBK_vmx= VMX;     \         hbook1_(&HBK_id,CHTITL,&HBK_nx,&HBK_xmi,&HBK_xma,  \                   &HBK_vmx,strlen(CHTITL))#define HBOOK2(ID,CHTITL,NX,XMI,XMA,NY,YMI,YMA,VMX) \         HBK_id = ID;      \         HBK_nx = NX;      \         HBK_xmi= XMI;     \         HBK_xma= XMA;     \         HBK_ny = NY;      \         HBK_ymi= YMI;     \         HBK_yma= YMA;     \         HBK_vmx= VMX;     \         hbook2_(&HBK_id,CHTITL,&HBK_nx,&HBK_xmi,&HBK_xma, \                  &HBK_ny,&HBK_ymi,&HBK_yma, \                   &HBK_vmx,strlen(CHTITL))#define HBOOKN(ID,CHTITL,NDIM,CHRZ,NPRIME,CHTAGS) \         HBK_id = ID;      \         HBK_ndim = NDIM;     \         HBK_nprime= NPRIME;     \         hbookn_(&HBK_id,CHTITL,&HBK_ndim,CHRZ,&HBK_nprime,CHTAGS, \            strlen(CHTITL),strlen(CHRZ),8)#define HBPROF(ID,CHTITL,NX,XMI,XMA,YMI,YMA,CHOPT) \         HBK_id = ID;      \         HBK_nx = NX;      \         HBK_xmi= XMI;     \         HBK_xma= XMA;     \         HBK_ymi= YMI;     \         HBK_yma= YMA;     \         hbprof_(&HBK_id,CHTITL,&HBK_nx,&HBK_xmi,&HBK_xma \            ,&HBK_ymi,&HBK_yma, CHOPT\            ,strlen(CHTITL), strlen(CHOPT))#define HBFUN1(ID,CHTITL,NX,XMI,XMA,FUN) \         HBK_id = ID;      \         HBK_nx = NX;      \         HBK_xmi= XMI;     \         HBK_xma= XMA;     \         HBK_vmx= VMX;     \         hbfun1_(&HBK_id,CHTITL,&HBK_nx,&HBK_xmi,&HBK_xma,  \                   &HBK_vmx,strlen(CHTITL))#define HFILL(ID,X,Y,W)  \         HBK_id = ID;      \         HBK_x  = X;    \         HBK_y  = Y;    \         HBK_w  = W;    \         hfill_(&HBK_id,&HBK_x,&HBK_y,&HBK_w)#define HF1(NH,X,W)   \         HBK_id = NH;      \         HBK_x  = X;    \         HBK_w  = W;    \         hf1_(&HBK_id,&HBK_x,&HBK_w)#define HF2(NH,X,Y,W) \         HBK_id = NH;      \         HBK_x  = X;    \         HBK_y  = Y;    \         HBK_w  = W;    \         hf2_(&HBK_id,&HBK_x,&HBK_y,&HBK_w)#define HFN(ID,X)  \         HBK_id = ID;      \         hfn_(&HBK_id,X)#define HFNT(ID)  \         HBK_id = ID;  \         hfnt_(&HBK_id) #define HBNT(ID,CHTITL,CHOPT) \         HBK_id = ID;      \         hbnt_(&HBK_id,CHTITL,CHOPT, \            strlen(CHTITL),strlen(CHOPT))#define HBNAME(ID,CHBLOK,VARI,CHFORM) \         HBK_id  = ID;      \         hbname_(&HBK_id,CHBLOK,VARI,CHFORM, \            strlen(CHBLOK),strlen(CHFORM))#define HPLOT(ID,CHOPT,CHCASE,NUM)   \         HBK_id = ID;      \         HBK_x  = NUM;     \         hplot_(&HBK_id,CHOPT,CHCASE,&HBK_x,strlen(CHOPT),strlen(CHCASE))#define HINDEX()   \         hindex_()#define HISTDO()   \         histdo_()#define HPRINT(I)  \         hprint_(&I)#define HRPUT(ID,A,B)    \         HBK_id = ID;      \                hrput_(&HBK_id,A,B,strlen(A),strlen(B))#define HROPEN(LUN,DIRNAME,FILENAME,NEWOLD,SIZE,FLAG) \         hropen_(&LUN,DIRNAME,FILENAME,NEWOLD,&SIZE,&FLAG\                       ,strlen(DIRNAME),strlen(FILENAME),strlen(NEWOLD))#define  HROUT(ID,CYCLE,MODE)  \         { HBK_id = ID; \           HBK_i = CYCLE; \           hrout_(&HBK_id,&HBK_i,MODE,strlen(MODE)); }#define  HCDIR(DIRNAME,CHOPT)  \         hcdir_(DIRNAME,CHOPT,strlen(DIRNAME),strlen(CHOPT))#define  HMDIR(DIRNAME,CHOPT)  \         hmdir_(DIRNAME,CHOPT,strlen(DIRNAME),strlen(CHOPT))#define  HREND(DIRNAME)  \         hrend_(DIRNAME,strlen(DIRNAME))#define HPLOPT(OPT,NUM)     \         hplopt_(OPT,&NUM,strlen(OPT))#define HGIVE(ID,TITLE,NX,XMI,XMA,NY,YMI,YMA,NWT,LOC,LEN) \         hgive_(&ID,TITLE,&NX,&XMI,&XMA,\                &NY,&YMI,&YMA,\                &NWT,&LOC,LEN)#define HUNPAK(ID,CONTENT,CHOICE,NUM)   \         hunpak_(&ID,CONTENT,CHOICE,&NUM,strlen(CHOICE))#define HPAK(ID,CONTENT) \         HBK_id = ID;      \         hpak_(&HBK_id,CONTENT)#define HPAKE(ID,CONTENT) \         HBK_id = ID;      \         hpake_(&HBK_id,CONTENT)#define HDELET(ID) \         HBK_id = ID;      \         hdelet_(&HBK_id)#define HRESET(ID,NAME)  \         HBK_id = ID;   \         hreset_(&HBK_id,NAME,strlen(NAME))/* Function prototypes for C++ */    void hidopt_  (int*, char*, int);    void hldir_  ( char *, char *, int, int);    void hlimit_ ( int * );    void hbook1_ ( int *, char *, int *, float *, float *, float *, int);    void hbook2_ ( int *, char *, int *, float *, float *, int *, float *, float *, float *, int);    void hbookn_ ( int *, char *, int *, char *, int *, char *, int, int, int);    void hbprof_ ( int *, char *, int *, float *, float * , float *, float *,  char * , int,  int);    void hbfun1_ ( int *, char *, int *, float *, float *, float *, int);    void hbname_ ( int *, char *, float*, char *, int, int);    void hfill_  ( int *, float *, float *, float *);    void hf1_    ( int *, float *, float *);    void hf2_    ( int *, float *, float *, float *);    void hfn_    ( int *, float * );    void hfnt_   ( int *);    void hbnt_   ( int *, char *, char *, int, int);    void hplot_  ( int *, char *, char *, float *, int, int);    void hindex_ ( void );    void histdo_ ( void );    void hprint_ ( int * );    void hrput_  ( int *, char *, char *, int, int );    void hropen_ ( int *, char *, char *, char *, int *, int *, int, int, int);    void hrout_  ( int *, int *, char *, int);    void hcdir_  ( char *, char *, int, int);    void hmdir_  ( char *, char *, int, int);    void hrend_  ( char *, int);    void hplopt_ ( char *, int *, int);    void hgive_  ( int *, char *, int *, float *, float *,                  int *, float *, float *,                  int *, int *, int);    void hunpak_ ( int *, float *, char *, int *, int);    void hpak_   ( int *, float *);    void hpake_  ( int *, float *);    void hdelet_ ( int *);    void hreset_ ( int *, char *, int);#endif /* HBOOK_H */