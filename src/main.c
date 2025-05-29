#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compress.h"

/* --------------------------------------------------------- */
/*  argv                                                     */
/*  0  exe                                                   */
/*  1  c|d                                                   */
/*  2  input                                                 */
/*  3  output                                                */
/*  4  BWT  (optional, "0" or "1")                           */
/*  5  MTF  (optional, "0" or "1")                           */
/*  6  RLE  (optional, "0" or "1")                           */
/* --------------------------------------------------------- */

static void usage(const char *exe)
{
    fprintf(stderr,
        "Usage:\n"
        "  %s c <input> <output>              # full pipeline (BWT-MTF-RLE-Huff)\n"
        "  %s d <input> <output>\n"
        "  %s c <input> <output> <BWT> <MTF> <RLE>\n"
        "  %s d <input> <output> <BWT> <MTF> <RLE>\n"
        "      where each flag is 0 or 1\n",
        exe, exe, exe, exe);
}

/* parse "0"/"1", exit on anything else */
static int parse_flag(const char *s, const char *name)
{
    if (strcmp(s, "0") == 0) return 0;
    if (strcmp(s, "1") == 0) return 1;
    fprintf(stderr, "Invalid %s flag \"%s\" – must be 0 or 1\n", name, s);
    exit(1);
}

int main(int argc, char *argv[])
{
    /* ---------------- argument count ---------------- */
    if (argc != 4 && argc != 7) {
        usage(argv[0]);
        return 1;
    }

    /* ---------------- pipeline flags ---------------- */
    int use_bwt = 1, use_mtf = 1, use_rle = 1;       /* defaults */

    if (argc == 7) {
        use_bwt = parse_flag(argv[4], "BWT");
        use_mtf = parse_flag(argv[5], "MTF");
        use_rle = parse_flag(argv[6], "RLE");
    }

    /* ---------------- operation --------------------- */
    switch (argv[1][0]) {
    case 'c':
        return compress(argv[2], argv[3], use_bwt, use_mtf, use_rle);
    case 'd':
        return decompress(argv[2], argv[3], use_bwt, use_mtf, use_rle);
    default:
        usage(argv[0]);
        return 1;
    }
}
