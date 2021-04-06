#include <environnement.h>

#include <errno.h>
#ifndef ERROR
#define ERROR -1
#endif

// pour printf
#include <stdio.h>

#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>

#include <shared_memory.h>

#ifdef OS9
#include <module.h>
struct datamodule {
	struct modhcom header;
	int data_offset;
} *modstart;
#endif

/* ========================================================================= */
void *SharedMemMake(char *nom, int taille, int *id) {  
#ifdef OS9
	if(((int)modstart=_mkdata_module(nom,taille,0x8001,0x333)) == (int)ERROR)
		return(ERROR);
	else return( (void *)modstart + modstart->data_offset );
#else
	int p,v; key_t cle; void *adrs; struct shmid_ds conf;

	p = open(nom, O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
//-	if((adrs = malloc(taille))) { bzero(adrs,taille); write(p,adrs,taille); free(adrs); }
	close(p);
	errno = 0;
	cle = ftok(nom,*id);
//	printf("  | ftok: cle 0x%08X (%d, %s)\n",(unsigned int)cle,errno,strerror(errno));
	v = shmget(cle,taille,IPC_CREAT); // (IPC_PRIVATE,taille,0644|IPC_CREAT|IPC_EXCL)
//	printf("  | shmget: Id  0x%08X (%d, %s)\n",(unsigned int)v,errno,strerror(errno));
	errno = 0;
	p = shmctl(v,IPC_STAT,&conf);
//	if(p != -1) printf("  > shmctl: %ld/%d octets\n",conf.shm_segsz,taille);
//	else printf("  | shmctl: %d, %s\n",errno,strerror(errno));
	conf.shm_perm.mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
//	printf("  | shmctl: %d, %s\n",errno,strerror(errno));
	errno = 0;
	shmctl(v,IPC_SET,&conf);
//	printf("  | shmctl: %d, %s\n",errno,strerror(errno));
	adrs = shmat(v,0,0); // SHM_RND
//	printf("  | shmat: adresse 0x%08X (%d, %s)\n",(unsigned int)adrs,errno,strerror(errno));
//-	if(adrs) bzero(adrs,taille);
	*id = v;
	return(adrs);

#endif
}
/* ========================================================================= */
void *SharedMemLink(char *nom, int id) {  
#ifdef OS9
	if(((int)modstart=modlink(nom,0x400)) == (int)ERROR) return(ERROR);
	else return( (void *)modstart + modstart->data_offset );
#else
	return(shmat(id,0,0)); // SHM_RND
#endif
}
/* ========================================================================= */
void *SharedMemAdrs(char *nom, int taille, int *id) {  
#ifdef OS9
	if(((int)modstart=modlink(nom,0x400)) == (int)ERROR) {
		if(((int)modstart=_mkdata_module(nom,taille,0x8001,0x333)) == (int)ERROR)
			return(ERROR);
	}
	return( (void *)modstart + modstart->data_offset );
#else
	void *adrs;
	if((adrs = shmat(*id,0,0)) == (void *)-1) adrs = SharedMemMake(nom,taille,id); // SHM_RND
	return(adrs);
#endif
}
/* ========================================================================= */
int SharedMemUnlk(char *nom, void *adrs) {  
#ifdef OS9
	if(munload(nom,0) == ERROR) return(ERROR);
	else return(0);
#else
	return(shmdt(adrs));
#endif
}
/* ========================================================================= */
int SharedMemKill(char *nom, void *adrs) {  
	register int i;

	i = 256;
#ifdef OS9
	while((munload(nom,0) != ERROR) && i) i-- ;
	return(i);
#else
	while((shmdt(adrs) != -1) && i) i-- ;
	return(i);
#endif
}
/* ========================================================================= */
void SharedMemRaz(char *nom, int id) {
#ifdef OS9
	register int i;
	i = 256; while((munload(nom,0) != ERROR) && i) i-- ;
#else
	struct shmid_ds buffer;
	shmctl(id,IPC_RMID,&buffer);
#endif
}
/* ========================================================================= */
void SharedMemRazAll(int depart, int arrivee) {
	int i;
	struct shmid_ds buffer;
	
	for(i=depart; i<arrivee; i++) shmctl(i,IPC_RMID,&buffer);
}
/* ========================================================================= */
void *SharedMemLoad(char *nom, int *id, char *path) {
	void *adrs;
#ifdef OS9
	char commande[80];
#endif
	
	if((adrs = SharedMemLink(nom,*id)) == (void *)ERROR) {
		if(path) {
#ifdef OS9
			strcpy(commande,"load -d ");
			strcat(commande,path);
			strcat(commande,"/");
			strcat(commande,nom);
			strcat(commande," <>>>/nil");
			system(commande);                /*   load module from disk  */   
#else
			/* pour UNIX: creer la zone (d'ou pointeur <adrs> et ident <id>),
			   lire le fichier avec read(n,<adrs>,..)
			*/
#endif
			if((adrs = SharedMemLink(nom,*id)) == (void *)ERROR) {
#ifdef DEBUG
				printf("(SharedMemload) commande %s en erreur\n",commande);
#endif
				/*
				return(SharedMemMake(nom,taille));
				*/
			} else {
			        SharedMemUnlk(nom,adrs);
#ifdef DEBUG
				printf("(SharedMemload) %s lu du fichier\n",nom);
#endif
			}
		}
	}
#ifdef DEBUG
	  else printf("(SharedMemLoad) %s deja charge\n",nom);
#endif
	return(adrs);
}
