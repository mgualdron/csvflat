// -------------------------------------------------------------------------
// Program Name:    csvflat.c
//
// Purpose:         To remove embedded newlines from CSV files.
//
// -------------------------------------------------------------------------
#define _GNU_SOURCE //cause stdio.h to include vasprintf
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "util/dbg.h"
#include <csv.h>
#define NUL_REPLACEMENT_CHARACTER 63   // This is a '?'
#define NL_REPLACEMENT_CHARACTER 32   // This is a Space

#define Sasprintf(write_to, ...) {           \
    char *tmp_string_to_extend = (write_to); \
    asprintf(&(write_to), __VA_ARGS__);      \
    free(tmp_string_to_extend);              \
}

static const char *program_name = "csvflat";
static char *delim_arg = "\t";
static char delim_csv = CSV_COMMA;
static char *quote_arg = NULL;
static char quote = CSV_QUOTE;
static int ignore_this = 0;

typedef struct { unsigned int rcount; unsigned int fcount; char *record; } CSV_status;

static void try_help (int status) {
    printf("Try '%s --help' for more information.\n", program_name);
    exit(status);
}

static void usage (int status) {
    if (status != 0) {
        try_help(status);
    }
    else {
      printf ("\
Usage: %s [OPTION]... [FILE]...\n\
", program_name);

      printf ("\
Output records from a CSV file after replacing any embedded newlines with spaces.\n\
More than one FILE can be specified.\n\
");

      printf ("\
\n\
  -d, --delimiter=DELIM  the delimiting character for the input FILE(s)\n\
  -Q, --csv-quote        CSV quoting character (double-quote is the default)\n\
  -h, --help             This help\n\
");
    }

    exit (status);
}


static struct option long_options[] = {
    {"delimiter",   required_argument, 0, 'd'},
    {"csv-quote",   required_argument, 0, 'Q'},
    {"help",        no_argument      , 0, 'h'},
    {0, 0, 0, 0}
};


static void replace_newlines(char *line, ssize_t bytes_read)
{
    for (ssize_t i = 0; i < bytes_read; i++) {
        if ( line[i] == 10 || line[i] == 13 ) {
            line[i] = NL_REPLACEMENT_CHARACTER;
        }
    }
}

// Callback 1 for CSV support, called whenever a field is processed:
void cb1 (void *s, size_t len, void *data)
{
    size_t fld_size;
    char *fld = (char *)s;
    CSV_status *csv_track = (CSV_status *)data;

    replace_newlines(fld, (ssize_t)len);

    csv_track->fcount++;
    fld_size = csv_write2(NULL, 0, len ? fld : "", len, quote);
    char *out_temp = (char *)malloc((fld_size + 1) * sizeof(char));
    csv_write2(out_temp, fld_size, len ? fld : "", len, quote);
    out_temp[fld_size] = '\0';  // NUL-terminate the written field
    if ( csv_track->record == NULL ) {
        Sasprintf(csv_track->record, "%s", out_temp);
    }
    else {
        Sasprintf(csv_track->record, "%s%c%s", csv_track->record, delim_csv, out_temp);
    }
    free(out_temp);
}

// A function pointer to one of the cb2 functions below:
void (*cb2) (int, void *);

// Callback 2 for CSV support, called whenever a record is processed:
void cb2_none (int c, void *data)
{
    CSV_status *csv_track = (CSV_status *)data;

    csv_track->rcount++;
    printf("%s\n", csv_track->record);

    csv_track->fcount = 0;
    free(csv_track->record);
    csv_track->record = NULL;
    ignore_this = c;
}

int csvflat(char *filename)
{
    struct csv_parser p;
    char buf[1024];
    FILE *fp = NULL;
    size_t bytes_read = 0; // num of chars read
    CSV_status *csv_track = (CSV_status *)malloc(sizeof(CSV_status));

    csv_track->rcount = 0;
    csv_track->fcount = 0;
    csv_track->record = NULL;

    if (filename[0] == '-') {
        fp = stdin;
    }
    else {
        fp = fopen(filename, "rb");
    }

    check(fp != NULL, "Error opening file: %s.", filename);

    check(csv_init(&p, CSV_APPEND_NULL) == 0, "Error initializing CSV parser.");

    // Set some parsing params:
    csv_set_delim(&p, delim_csv);
    csv_set_quote(&p, quote);

    while ((bytes_read=fread(buf, 1, 1024, fp)) > 0) {
        check(csv_parse(&p, buf, bytes_read, cb1, cb2, csv_track) == bytes_read, "Error while parsing file: %s", csv_strerror(csv_error(&p)));
    }

    check(csv_fini(&p, cb1, cb2, csv_track) == 0, "Error finishing CSV processing.");

    csv_free(&p);
    free(csv_track);

    fclose(fp);

    return 0;

error:
    return -1;
}


/* The main function */
int main (int argc, char *argv[])
{
    int c;
    int delim_arg_flag = 0;
    int csv_mode = 1;

    while (1) {

        // getopt_long stores the option index here.
        int option_index = 0;

        c = getopt_long (argc, argv, "hlCcNd:n:", long_options, &option_index);

        // Detect the end of the options.
        if (c == -1) break;

        switch (c) {
            case 0:
                // If this option set a flag, do nothing else now.
                break;

            case 'd':
                debug("option -d with value `%s'", optarg);
                delim_arg = optarg;
                delim_arg_flag = 1;
                break;

            case 'Q':
                debug("option -Q");
                quote_arg = optarg;
                check(strlen(quote_arg) == 1, "ERROR: CSV quoting character must be exactly one byte long");
                quote = quote_arg[0];
                break;

            case 'h':
                debug("option -h");
                usage(0);
                break;

            case ':':   /* missing option argument */
                fprintf(stderr, "%s: option '-%c' requires an argument\n",
                        argv[0], optopt);
                break;

            // getopt_long already printed an error message.
            case '?':
                usage(1);
                break;

            default:
                usage(1);
        }
    }

    if (csv_mode && delim_arg_flag) {
        check(strlen(delim_arg) == 1, "ERROR: CSV delimiter must be exactly one byte long");
        delim_csv = delim_arg[0];
    }

    int j = optind;  // A copy of optind (the number of options at the command-line),
                     // which is not the same as argc, as that counts ALL
                     // arguments.  (optind <= argc).

    // Process any remaining command line arguments (input files).
    do {

        char *filename = NULL;

        // Assume STDIN if no additional arguments, else loop through them:
        if (optind == argc) {
            filename = "-";
        }
        else if (optind < argc) {
            filename = argv[j];
        }
        else if (optind > argc) {
            break;
        }

        debug("The filename is %s", filename);

        // Process the file:
        cb2 = cb2_none;
        check(csvflat(filename) == 0, "Error in CSV-mode processing of file: %s", filename);

        j++;

    } while (j < argc);

    return 0;

error:
    return -1;
}
