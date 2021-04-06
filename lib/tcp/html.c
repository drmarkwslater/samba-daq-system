/*
 *  html.c
 *  Librairies
 *
 *  Created by Michel GROS on 22.02.12.
 *  Copyright 2012 CEA/DSM/DAPNIA. All rights reserved.
 *
 */

#define HTML_C

#include <stdio.h>
#include <unistd.h> // pour getpid()
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#include "html.h"
#include <decode.h>
#define Accord2s(var) var,(var>1)?"s":"",(var>1)?"s":""

static char SvrPereUnique = 1;
// static int SvrTol = 8;
static char SvrPPC = 0;

/* ========================================================================= */
void HttpPageDebut(char *titre,int couleur) {
	int nb;

	//1 strcpy(HttpPageTotale,"Content-type: text/html;charset=ISO-8859-1\r\n\r\n");
	//2 strcpy(HttpPageTotale,"Content-type: text/html\n\n");
	//3 strcpy(HttpPageTotale,"<!DOCTYPE html PUBLIC>\n");
	//n strcat(HttpPageTotale,"<html>\n");
	// strcpy(HttpPageTotale,"Content-type: text/html\n<html>\n");
	strcpy(HttpReponse,"<html>\n");
	HttpRepDim = (int)strlen(HttpReponse);
	nb = sprintf(HttpReponse+HttpRepDim,"<head>");
	HttpRepDim += nb;
	if(titre && (titre[0] != '\0')) {
		nb = sprintf(HttpReponse+HttpRepDim,"<title>%s</title>",titre);
		HttpRepDim += nb;
	}
	nb = sprintf(HttpReponse+HttpRepDim,"</head>\n");
	HttpRepDim += nb;
	nb = sprintf(HttpReponse+HttpRepDim,"<body bgcolor='#%06X'>\n",couleur);
	HttpRepDim += nb;
	strcat(HttpReponse+HttpRepDim,"<font face='Courier' size=2>\n");
	HttpRepDim = (int)strlen(HttpReponse);
}
/* ======================================================================== */
int HttpEcrit(char *fmt, ...) {
	char texte[HTMLLIGNEMAX];
	va_list argptr; int nb;

	va_start(argptr, fmt);
	nb = vsnprintf(texte,HTMLLIGNEMAX,fmt,argptr);
	va_end(argptr);

	strcpy(HttpReponse+HttpRepDim,texte);
	HttpRepDim += nb;

	return(1);
}
/* ========================================================================= */
void HttpPageFin() {
	// strcat(HttpReponse,"\n</face>\n");
    strcat(HttpReponse,"\n</font>\n");
	strcat(HttpReponse,"</body>\n");
	strcat(HttpReponse,"</html>\n");
	HttpRepDim = (int)strlen(HttpReponse);
}
/* ========================================================================== */
static int HttpPageEnvoi(int skt, int rc) {
	int sup,nb;

	HttpPageDim = 0; nb = 0;
	sup = sprintf(HttpPageTotale,"HTTP/1.1 %03d \n",rc);
	HttpPageDim += sup;
	sup = sprintf(HttpPageTotale+HttpPageDim,"Content-Type: text/html\n");
	HttpPageDim += sup;
	sup = sprintf(HttpPageTotale+HttpPageDim,"Content-Length: %d\n",HttpRepDim);
	HttpPageDim += sup;
	sup = sprintf(HttpPageTotale+HttpPageDim,"Connection: %s\n",HttpTermine? "close": "keep-alive");
	HttpPageDim += sup;
	sup = sprintf(HttpPageTotale+HttpPageDim,"\n");
	HttpPageDim += sup;

	memcpy(HttpPageTotale+HttpPageDim,HttpReponse,HttpRepDim);
	HttpPageDim += HttpRepDim;
	HttpPageTotale[HttpPageDim] = '\0';
	//	printf("[%d]      ***** %s<%d> demande\n",getpid(),__func__,skt);
	// return(write(HttpSkt,HttpPageTotale,HttpPageDim));
	if(skt >= 0) nb = (int)send(skt,HttpPageTotale,HttpPageDim,0); // MSG_DONTROUTE
	// printf("[%d]      ***** %s<%d> retourne %d\n",getpid(),__func__,skt,rc);
	return(nb);
}
/* ========================================================================== */
int HttpServeur(int port) {
	char pas_fini;
	char nom[80]; char *r,*s,*d,*v;
	int base,rc,nb,l;
	TypeSocket rem_skt;

	printf("---------------------------------- [%d] Daemon lance sur le port #%d\n",getpid(),port);
	SvrPPC = EndianIsBig();
	base = SocketBase(port,&HttpSocket);
	if(base == -1) {
		printf("---------------------------------- [%d] Daemon plante (port #%d)\n",getpid(),port);
		printf("                                   (%d) %s\n",errno,strerror(errno));
		exit(errno);
	}
	SvrDebug = 0;
    SvrMulti = 1; SvrFils = 0;
    SvrPereUnique = 0;
	HttpSkt = -1;
	HttpTermine = 0;
	HttpClientType = HTTP_CLIENT_ORDI;
	SvrPrecedente = HttpSkt;

	while(!HttpTermine) {
		char type_connu;
		HttpSkt = SocketBranchee(base,&rem_skt);
		SvrPrecedente = HttpSkt;
		if(HttpSkt == -1) { sleep(1); continue; }
		SocketName(HttpSkt,HttpServeurNom);
		PeerName(HttpSkt,nom);
        if(SvrMulti) {
            if(SvrFils == 0) {
                SvrFils = SvrFork(base);
                /* SvrFils = 0:  je suis le fils;
				 SvrFils > 0:  je suis parent, ceci est le pid du fils;
				 SvrFils = -1: je suis egalement le parent, mais pas de fils cree (erreur <errno> */
                if(SvrFils > 0) {
                    printf("\n[%d] Socket #%d branchee sur le client %s, prochain serveur: [%d]\n",getpid(),HttpSkt,nom,SvrFils);
					printf("[%d] Reference serveur: %s:%d\n",getpid(),HttpServeurNom,port);
					HttpMajorLue = 0; HttpMinorLue = 9;
                    if(SvrPereUnique) continue;
                } else if(SvrFils < 0) {
                    printf("\n[%d] Socket #%d branchee sur le client %s, mais prochain serveur en erreur (retour %d -> %d: %s)\n",getpid(),HttpSkt,nom,SvrFils,errno,strerror(errno));
                    if(SvrPereUnique) continue;
                } else {
                    printf("\n---------------------------------- Nouveau serveur [%d] lance on %s:%d\n",getpid(),HttpServeurNom,port);
                    // SocketFermee(HttpSkt); // ce n'est pas au fils de la fermer tout de suite? non, a la fin!
                    if(!SvrPereUnique) continue;
                }
            }
        } else SvrFils = -1;
		type_connu = 0;
		pas_fini = 1;
		do {
			rc = (int)recv(HttpSkt,HttpPageRecue,HTMLPAGEMAX,0);
			if(rc < 0) printf("[%d] lecture de la requete en erreur (retour %d -> %d: %s)\n",getpid(),rc,errno,strerror(errno));
			else if(rc > 0) printf("[%d] %d octet%s lu%s\n",getpid(),Accord2s(rc));
			if(rc <= 0) { sleep(1); continue; }
			// else if(rc == 0) pas_fini = 1; // pas_fini = 0; <=> pas d'autre recv
			else {
				HttpPageRecue[HTMLPAGEMAX-1] = '\0';
				if(SvrDebug > 1) printf("[%d] <<<<<<<<<< recu (%d) via <%d>:\n%s\n[%d] >>>>>>>>>> fin de la page\n",getpid(),rc,HttpSkt,HttpPageRecue,getpid());
				r = HttpPageRecue;
				do {
					//- printf("{ %4d } %s\n",(int)(r-HttpPageRecue),r);
					if(!strncmp(r,"GET /",5)) {
						r += 5; d = r;
						s = r;
						while((*r != ' ') && (*r != '\0')) r++;
						if(*r != '\0') {
							if(s == r) s++; *r++ = '\0';
							if(!strcmp(r,"HTTP/")) {
								r += 5; v = r;
								while((*v != '.') && (*v != '\0')) v++;
								if(*v != '\0') *v++ = '\0';
								sscanf(r,"%d",&HttpMajorLue);
								sscanf(v,"%d",&HttpMinorLue);
								s = r;
							} else if(*r) s = r;
						}
						r = d; l = 0;
						while((*r != '?') && (*r != '\0') && (l < HTMLREQTMAX)) { r++; l++; }
						if(*r == '?') { if(s == r) s++; *r++ = '\0'; }
						strncpy(HttpUrlDemandee,d,HTMLREQTMAX); HttpUrlDemandee[HTMLREQTMAX-1] = '\0';
 						strncpy(HttpReqtRecue,r,HTMLREQTMAX); HttpReqtRecue[HTMLREQTMAX-1] = '\0';
                        /* if(SvrDebug > 1) */ printf("[%d] <<<<<<<<<< variables via <%d>:\n%s\n[%d] >>>>>>>>>>\n",getpid(),HttpSkt,HttpReqtRecue,getpid());
						r = s;
						// printf("> %4d < %s\n",(int)(r-HttpPageRecue),r);
						while((*r != '\n') && (*r != '\r') && (*r != '\0')) r++;
					} else if(!strncmp(r,"User-Agent: ",12)) {
						r += 12; v = r;
						while((*v != '\n') && (*v != '\0')) v++;
						if(*v == '\n') *v++ = '\0';
						strncpy(HttpDemandeur,r,HTMLREQTMAX); HttpUrlDemandee[HTMLREQTMAX-1] = '\0';
						if(SvrDebug > 1) printf("[%d] <<<<<<<<<< demandeur via <%d>:\n%s\n[%d] >>>>>>>>>>\n",getpid(),HttpSkt,HttpDemandeur,getpid());
						r = v;
					}
					if(*r) r++;
				} while(*r);
				printf("[%d] Fin de l'examen de la requete\n",getpid());
				if(!type_connu) { HttpClientType = strstr(HttpDemandeur,"iPhone")? HTTP_CLIENT_IPHONE: HTTP_CLIENT_ORDI; type_connu = 1; }
				rc = HttpTraiteRequete(HttpUrlDemandee,HttpDemandeur,HttpReqtRecue);
				nb = HttpPageEnvoi(HttpSkt,rc);
				if(SvrDebug > 1) printf("[%d] <<<<<<<<<< envoye (%d) via <%d>:\n%s\n[%d] >>>>>>>>>> fin de l'envoi (%d)\n",getpid(),HttpPageDim,HttpSkt,HttpPageTotale,getpid(),nb);
			}
		} while(pas_fini && !HttpTermine);
		SocketFermee(HttpSkt);
		printf("[%d] Socket #%d sur %s debranchee\n",getpid(),HttpSkt,nom);
		// if((SvrFils == 0) && SvrPereUnique) return(0);
		HttpSkt = -1; // ?
		// if(SvrFils > 0) HttpTermine = 1;
	};
	//	if(SvrPrecedente >= 0) SocketFermee(SvrPrecedente);
	printf("---------------------------------- [%d] Daemon stopped on port #%d\n",getpid(),port);
	fflush(stdout);
	close(base);
	if(SvrFils > 0) {
		printf("---------------------------------- [%d] Server stopped on port #%d\n",SvrFils,port);
		kill(SvrFils,SIGSTOP);
	}

	return(1);
}
