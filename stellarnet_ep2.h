#include <stddef.h>
char ep2_parse(
	/* in */ const char * /* filename */,
	/* in, opt */ float * /* wavelen_coefs */,
	/* out */ size_t * /* num_spectra */,
	/* out */ size_t * /* num_points */,
	/* out */ float ** /* wavelengths */,
	/* out */ float *** /* intensities */
);
