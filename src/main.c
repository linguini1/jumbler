#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "helptext.h"
#include "jumble.h"

/* Output file path if an output file is selected. */
char *outfile = NULL;

/* Input file path if an input file is selected. */
char *infile = NULL;

int main(int argc, char **argv) {

    /* Parse command line options. */

    int c;
    while ((c = getopt(argc, argv, ":i:o:h")) != -1) {
        switch (c) {
        case 'h':
            puts(HELP_TEXT);
            exit(EXIT_SUCCESS);
            break;
        case 'o':
            outfile = optarg;
            break;
        case 'i':
            infile = optarg;
            break;
        case '?':
            fprintf(stderr, "Unknown option -%c\n", optopt);
            exit(EXIT_FAILURE);
            break;
        }
    }

    /* Select input stream */

    FILE *in = stdin;
    if (infile != NULL) {
        in = fopen(infile, "r");
        if (in == NULL) {
            fprintf(stderr, "Could not open file '%s' for reading with error: %s\n", infile, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    /* Select output stream */

    FILE *out = stdout;
    if (outfile != NULL) {
        out = fopen(outfile, "w");
        if (out == NULL) {
            fprintf(stderr, "Could not open file '%s' for writing with error: %s\n", outfile, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    /* Seed random library for more random jumbling. */

    srand(time(NULL));

    /* Begin processing. */

    jumble_fsm_t fsm;
    jumble_fsm_init(&fsm, in, out);
    jumble_fsm_run(&fsm);

    /* Close opened files. */

    fclose(in);
    fclose(out);

    return EXIT_SUCCESS;
}
