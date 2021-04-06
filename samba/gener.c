#include <environnement.h>
//#ifdef CODE_WARRIOR_VSN
//#pragma message("Compilation de "__FILE__)
//#endif

#include <stdlib.h>
#include <time.h>
#include <math.h>

//#define TEST_GENER
#ifdef CODE_WARRIOR_VSN
#define srandom srand
#define random rand
#endif


#define NB_TIRAGES 12

static float Moyenne,Sigma;

/* ========================================================================== */
/* Gaussienne de moyenne 0.0 et de largeur sigma                         */
static float smear(sigma) float sigma; {
    int i; float fluctuation;

	fluctuation = 0.0;
	/* random generator between 0 - 1 */
    for(i=0; i<NB_TIRAGES; i++) fluctuation += (float)(random() % 10000) / 10000.0;
//    fluctuation = sigma * (fluctuation - ((float)NB_TIRAGES / 2.0));
    fluctuation = sigma * ((fluctuation / (float)NB_TIRAGES) - 0.5);
    return(fluctuation);
}
/* ========================================================================== */
/* initialize random generator                                           */
void mc_init(float energie, float resolution) {
	srandom((unsigned)time(NULL));
	Moyenne = energie;
	Sigma = resolution;
	return;
}
/* ========================================================================== */
float mc_amp(int *delai) {
	float amplitude;

	if(Moyenne > 0.0) amplitude = Moyenne;
	/* Moyenne=0 => pas de raie => fond plat => random generator between 0 - 200 */
    else amplitude = (float)(random() % 200000) / 1000.0 ;
    /* return amplitude at E=Moyenne+smearing(Sigma) */
	return(amplitude + smear(Sigma));
}
#ifdef TEST_GENER
#define MAX 65536
/* ========================================================================== */
int main(int argc, char *argv[]) {
	float amp,moyenne,sigma;
	int i,delai;
	
	mc_init(122.0,5.0);
	moyenne = sigma = 0.0;
	for(i=0; i<MAX; i++) {
		amp = mc_amp(&delai);
//		printf("%3d/ amp=%10g\n",i,amp);
		moyenne += amp;
		sigma += (amp * amp);
	}
	moyenne /= (float)MAX;
	sigma = (float)sqrt((double)(sigma / (float)MAX - (moyenne * moyenne)));
	printf("=> moyenne=%g, sigma=%g\n",moyenne,sigma);
	return(0);
}
#endif



