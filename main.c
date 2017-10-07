#include <stdio.h> // printf
#include <stdlib.h> // strtof
#include <errno.h> // error checking
#include "stellarnet_ep2.h"

int main (int argc, char** argv) {
	float coefs[4] = {};
	if (argc == 7) // use provided coeffs
		for (size_t i = 0; i < 4; i++) {
			char *endptr = NULL;
			errno = 0;
			coefs[i] = strtof(argv[3+i], &endptr);
			if ((coefs[i] == 0.) && (endptr == argv[3+i]) || errno) {
				printf("%s isn't a floating point number\n", argv[3+i]);
				return 1;
			}
		}
	else if (argc != 3) {
		printf("Usage: %s infile.ep1 outfile.txt [c1 c2 c3 c4]\n", argv[0]);
		return 1;
	}
	size_t num_spectra;
	float * wavelengths;
	float ** intensities;
	enum ep2_parse_result ret;
	if ((ret = ep2_parse(
		argv[1],
		argc == 7 ? coefs : NULL,
		&num_spectra,
		&wavelengths,
		&intensities
	))) {
		printf("Failed to parse %s, error code %d\n", argv[1], (int)ret);
		return 2;
	}

	FILE* outfile = fopen(argv[2], "w");
	if (!outfile) {
		printf("Failed to open %s\n", argv[2]);
		return 3;
	}

	for (size_t i = 0; i < ep2_num_points; i++) {
		if (fprintf(outfile, "%g", wavelengths[i]) < 0) {
			printf("%s: write error\n", argv[2]);
			return 3;
		}
		for (size_t frame = 0; frame < num_spectra; frame++) {
			if (fprintf(outfile, "\t%g", intensities[frame][i]) < 0) {
				printf("%s: write error\n", argv[2]);
				return 3;
			}
		}
		if (fprintf(outfile, "\n") < 0) {
			printf("%s: write error\n", argv[2]);
			return 3;
		}
	}
	
	free(wavelengths);
	for (size_t i = 0; i < num_spectra; i++)
		free(intensities[i]);
	free(intensities);
	return 0;
}
