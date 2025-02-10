#ifndef include_rand48_h
#define include_rand48_h
#include <stdlib.h>
#if !(_SVID_SOURCE || _XOPEN_SOURCE)
double drand48(void) {
    return rand() / (RAND_MAX + 1.0);
}

long int lrand48(void) {
    return rand();
}

long int mrand48(void) {
    return rand() > RAND_MAX / 2 ? rand() : -rand();
}

void srand48(long int seedval) {
    srand(seedval);
}
unsigned short *seed48(unsigned short seed16v[3]) {
	long int seedval = 0;
	int i;
	for(i=0; i<3; i++) {
		seedval = (seedval<<16)+seed16v[i];
	}
	srand48(seedval);
	return 0;
}
#endif

#endif  /* include_rand48_h */

