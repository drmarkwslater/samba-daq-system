#ifndef SNCF_H
#define SNCF_H

#ifdef SNCF_C
	#define SNCF_VAR
#else
	#define SNCF_VAR extern
#endif

#include <shared_memory.h>

#define SAMBA_MEM_NOM "SambaMem"
#define SAMBA_MEM_CLE 2018
SNCF_VAR int SambaMemCle;
SNCF_VAR int SambaMemId;
typedef struct {
	char lance_run,en_cours,trigger_demande,archive_demandee; short sessions;
	int evts_vus,evts_sauves;
	float taux_actuel,taux_global;
	char run[12],duree[12];
} TypeSambaShared;
SNCF_VAR TypeSambaShared *SambaInfos;

#endif /* SNCF_H */
