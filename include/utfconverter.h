#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/utsname.h>
#include <sys/times.h>
#include <time.h>
#include <sys/resource.h>
#include <math.h>

#include <getopt.h>

#include <string.h>

#include <stdbool.h>



#define MAX_BYTES 4
#define SURROGATE_SIZE 4
#define NON_SURROGATE_SIZE 2
#define NO_FD -1
#define OFFSET 2

#define FIRST   0
#define SECOND 1
#define THIRD  2
#define FOURTH 3

#ifdef __STDC__
#define P(x) x
#else
#define P(x) ()
#endif

/** The enum for endianness. */
typedef enum {LITTLE, BIG, EIGHT} endianness;

/** The struct for a codepoint glyph. */
typedef struct Glyph {
	unsigned char bytes[MAX_BYTES];
	endianness end;
	bool surrogate;
} Glyph;






/** The usage statement. */
const char *USAGE = { 

"./utf [-h|--help] [-v|-vv] -u OUT_ENC | --UTF=OUT_ENC IN_FILE [OUT_FILE]\n\n \
\tOption arguments:\n \
\t -h, --help\t Displays this usage.\n\n \
\t -v, -vv\t Toggles the verbosity of the program to level 1 or 2.\n\n \
\tMandatory argument:\n \
\t -u OUT_ENC, --UTF=OUT_ENC\tSets the output encoding.\n \
\t\t\t\t\tValid values for OUT_ENC: 16LE, 16BE\n\n \
\tPositional Arguments:\n \
\t IN_FILE\t The file to convert.\n \
\t [OUT_FILE]\t Output file name. If not present, defaults to stdout.\n"
};

/** Which endianness to convert to. */
extern endianness conversion;

/** Which endianness the source file is in. */
extern endianness source;

/**
 * A function that swaps the endianness of the bytes of an encoding from
 * LE to BE and vice versa.
 *
 * @param glyph The pointer to the glyph struct to swap.
 * @return Returns a pointer to the glyph that has been swapped.
 */
Glyph* swap_endianness(Glyph* glyph);

void print_verbose();


/**
 * Fills in a glyph with the given data in data[2], with the given endianness 
 * by end.
 *
 * @param glyph 	The pointer to the glyph struct to fill in with bytes.
 * @param data[2]	The array of data to fill the glyph struct with.
 * @param end	   	The endianness enum of the glyph.
 * @param fd 		The int pointer to the file descriptor of the input 
 * 			file.
 * @return Returns a pointer to the filled-in glyph.
 */
Glyph* fill_glyph (Glyph* glyph, unsigned char data[], endianness end, int fd);

Glyph* fill_utf8_glyph(Glyph* glyph, unsigned char data[], int fd);

/**
 * Writes the given glyph's contents to stdout.
 *
 * @param glyph The pointer to the glyph struct to write to stdout.
 */
void write_glyph (Glyph* glyph);

/**
 * Calls getopt() and parses arguments.
 *
 * @param argc The number of arguments.
 * @param argv The arguments as an array of string.
 */
void parse_args(int argc, char**argv);

/**
 * Prints the usage statement.
 */
void print_help ();

/**
 * Closes file descriptors and frees list and possibly does other
 * bookkeeping before exiting.
 *
 * @param The fd int of the file the program has opened. Can be given
 * the macro value NO_FD (-1) to signify that we have no open file
 * to close.
 */
int quit_converter (int fd, int exitStatus);

void start_clock(void);
float* end_clock();
