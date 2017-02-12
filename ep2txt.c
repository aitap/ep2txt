#include <stdio.h> /* printf, FILE* */
#include <stdlib.h> /* strtof, calloc */
#include <errno.h> /* error checking */
#include <string.h> /* strerror */

/* assumption: same IEEE 32-bit float as used in the files */

const size_t num_points = 2051;

int main(int argc, char** argv) {
	float c[4] = {0};
	if (argc == 7) /* use supplied c1..c4 */
		for (size_t i = 0; i < 4; i++) {
			char *endptr = NULL;
			errno = 0;
			c[i] = strtof(argv[3+i], &endptr);
			if ((c[i] == 0.) && (endptr == argv[3+i]) || errno) {
				printf("%s isn't a floating point number\n", argv[3+i]);
				return 1;
			}
		} /* FIXME: there was something about dividing c1 and c2 by 2 and 4 ? */
	else if (argc != 3) {
		printf("Usage: %s infile.ep1 outfile.txt [c1 c2 c3 c4]\n", argv[0]);
		return 1;
	}
	
	FILE* infile = fopen(argv[1], "rb");
	if (!infile) {
		printf("%s: %s\n", argv[1], strerror(errno));
		return 2;
	}
	FILE* outfile = fopen(argv[2], "w");
	if (!outfile) {
		printf("%s: %s\n", argv[2], strerror(errno));
		return 2;
	}

	float header[9];
	if (fread(&header, sizeof(header), 1, infile) != 1) {
		printf("%s: short read of header; errno=%s\n", argv[1], strerror(errno));
		return 3;
	}
	if (argc != 7) { /* use c1..c3 from file */
		c[0] = header[6];
		c[1] = header[7];
		c[2] = header[8];
	}
	size_t count = (size_t)header[1];

	float *spectra = calloc(count, num_points*sizeof(float));
	if (!spectra) {
		printf("couldn't allocate memory for %d spectra\n", count);
		return 4;
	}
	rewind(infile);
	if (fread(spectra, num_points*sizeof(float), count, infile) != count) {
		printf("%s: short read of spectral data; errno=%s\n", argv[1], strerror(errno));
		return 3;
	}

	for (size_t wl = 0; wl < num_points; wl++) {
		if (fprintf(outfile, "%g", c[2] + wl*(c[0] + wl*(c[1] + c[3]*wl))) < 0) {
			printf("%s: write error %s\n", argv[2], strerror(errno));
			return 5;
		}
		for (size_t frame = 0; frame < count; frame++) {
			if (fprintf(outfile, "\t%g", spectra[frame*num_points+wl]) < 0) {
				printf("%s: write error %s\n", argv[2], strerror(errno));
				return 5;
			}
		}
		if (fprintf(outfile, "\n") < 0) {
			printf("%s: write error %s\n", argv[2], strerror(errno));
			return 5;
		}
	}

	free(spectra);
	if (fclose(outfile)) {
		printf("%s: couldn't close, %s", argv[2], strerror(errno));
		return 5;
	}
	if (fclose(infile)) {
		printf("%s: couldn't close, %s", argv[1], strerror(errno));
		return 3;
	}

	return 0;
}
