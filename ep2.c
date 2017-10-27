#include "stellarnet_ep2.h"
#include <stdio.h> /* printf, FILE* */
#include <stdlib.h> /* strtof, calloc */
#include <errno.h> /* error checking */
#include <string.h> /* strerror */
#include <stdint.h> /* fixed-size integers */

// NOTE: this code assumes that your architecture has IEEE 32-bit floats and CHAR_BIT is 8

typedef union {
	uint8_t u8[2];
	uint16_t u16;
} endianness_test;

// byte swapping will occur for each `size`-sized item
static size_t fread_le(void * ptr, size_t size, size_t nmemb, FILE* stream) {
	{
		size_t ret = 0;
		if ((ret = fread(ptr, size, nmemb, stream)) != nmemb) return ret;
	}

	if (((endianness_test){.u16 = 0x0100}).u8[0]) { // host is big-endian => must byteswap
		char item[size]; // yes, this is C99 VLA

		for (size_t i = 0; i < nmemb; i++) {
			memcpy(item, (char*)ptr + i*size, size);
			for (size_t j = 0; j < size; j++)
				*((char*)ptr + i*size + j) = item[size-j-1];
		}
	}

	return nmemb;
}

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
	if (fread_le(header, sizeof(float), 9, infile) != 9) {
		ret = ep2_read_error;
		goto cleanup;
	}

	*num_spectra = (size_t)header[1];

	float * wl_c = wavelen_coefs; // use specified
	if (!wl_c) { // pointer was NULL
		wl_c = header + 6; // read from the file
		wl_c[3] = 0; // c4 isn't present in the file but we'd reserved space for it
	}

	*intensities = calloc(*num_spectra, sizeof(float*));
	if (!intensities) {
		ret = ep2_alloc_error;
		goto cleanup;
	}
	for (size_t i = 0; i < *num_spectra; i++) {
		(*intensities)[i] = calloc(ep2_num_points, sizeof(float));
		if (!(*intensities)[i]) {
			ret = ep2_alloc_error;
			goto cleanup;
		}
	}
	*wavelengths = calloc(ep2_num_points, sizeof(float));
	if (!*wavelengths) {
		ret = ep2_alloc_error;
		goto cleanup;
	}

	rewind(infile);
	for (size_t i = 0; i < *num_spectra; i++)
		if (fread_le((*intensities)[i], sizeof(float), ep2_num_points, infile) != ep2_num_points) {
			ret = ep2_read_error;
			goto cleanup;
		}
	for (size_t wl = 0; wl < ep2_num_points; wl++)
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
