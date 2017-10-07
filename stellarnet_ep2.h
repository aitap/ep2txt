#include <stddef.h>
enum {ep2_num_points = 2051};
enum ep2_parse_result {
	ep2_success = 0,
	ep2_open_error,
	ep2_read_error,
	ep2_alloc_error
};
enum ep2_parse_result ep2_parse(
	/* in */ const char * /* filename */,
	/* in, opt */ float /* wavelen_coefs */ [4],
	/* out */ size_t * /* num_spectra */,
	/* out */ float ** /* wavelengths */,
	/* out */ float *** /* intensities */
);
