#include "stellarnet_ep2.h"
#include <stdio.h> /* printf, FILE* */
#include <stdlib.h> /* strtof, calloc */
#include <errno.h> /* error checking */
#include <string.h> /* strerror */

/* assumption: same IEEE 32-bit float as used in the files */

const size_t num_points = 2051;

enum ep2_parse_result ep2_parse(
	const char * filename, float wavelen_coefs[4], size_t * num_spectra,
	float ** wavelengths, float *** intensities
) {
	enum ep2_parse_result ret = ep2_success;
	// so that cleanup may determine if pointer is allocated
	*wavelengths = NULL;
	*intensities = NULL;

	FILE* infile = fopen(filename, "rb");
	if (!infile) { ret = ep2_open_error; goto cleanup; }

	float header[10]; // header is 9 bytes, but we use the 10th below
	if (fread(header, sizeof(header), 1, infile) != 1) {
		ret = ep2_read_error;
		goto cleanup;
	}

	*num_spectra = (size_t)header[1];

	float * wl_c = wavelen_coefs; // use specified
	if (!wl_c) { // pointer was NULL
		wl_c = header + 6; // read from the file
		wl_c[4] = 0; // c4 isn't present in the file
	}

	*intensities = calloc(*num_spectra, sizeof(float*));
	if (!intensities) {
		ret = ep2_alloc_error;
		goto cleanup;
	}
	for (size_t i = 0; i < *num_spectra; i++) {
		(*intensities)[i] = calloc(num_points, sizeof(float));
		if (!(*intensities)[i]) {
			ret = ep2_alloc_error;
			goto cleanup;
		}
	}
	*wavelengths = calloc(num_points, sizeof(float));
	if (!*wavelengths) {
		ret = ep2_alloc_error;
		goto cleanup;
	}

	rewind(infile);
	for (size_t i = 0; i < *num_spectra; i++)
		if (fread((*intensities)[i], sizeof(float), num_points, infile) != num_points) {
			ret = ep2_read_error;
			goto cleanup;
		}
	for (size_t wl = 0; wl < num_points; wl++)
		(*wavelengths)[wl] = wl_c[2] + wl*(wl_c[0] + wl*(wl_c[1] + wl_c[3]*wl));

cleanup:
	if (infile) fclose(infile);
	if (ret) {
		if (*wavelengths) free(*wavelengths);
		if (*intensities) {
			for (size_t i = 0; i < *num_spectra; i++)
				free((*intensities)[i]);
			free(*intensities);
		}
	}

	return ret;
}
