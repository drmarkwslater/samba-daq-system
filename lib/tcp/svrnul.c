#include <environnement.h>

#include "html.h"
extern TypeSvrBox SvrLink;

/* ========================================================================== */
int HttpRepond(char *requete) { return(1); }
/* ========================================================================== */
// int HttpRepondClient(TypeHttpClient *client) { return(1); }
/* ========================================================================== */
int SvrActive() { return(1); }
/* ========================================================================== */
int SvrAborted() { return(1); }
/* ========================================================================== */
int SvrQuited() { return(1); }
/* ========================================================================== */
int SvrExited() { return(1); }
/* ========================================================================== */
int SvrUser() { SvrError(&SvrLink,"Serveur vide",13); return(1); }
