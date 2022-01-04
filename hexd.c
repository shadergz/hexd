/*
 *  Source code written by Gabriel Correia
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>
#include <getopt.h>
#include <stdarg.h>
#include <assert.h>

#include <ctype.h>
#include <math.h>

static const char program_name[] = "hexd";
static const char program_version[] = "0.0.2";

static uint8_t conv;
static uint8_t coll = 16;
static uint8_t style = 0;

static uint32_t seek = 0;

static FILE *file = NULL;
static const char *filename = NULL;

static int32_t count = -1;

static int (*look_at_me[])(int) = {
    isalpha,
    isalnum,
    isprint,
    isupper,
    isdigit,
    isxdigit,
    NULL
};

/* Cleanup all the memory and close the FILE pointer */
void cleanup ()
{
    if (file)
        fclose (file);
    if (filename)
        free ((char*)filename);
}

/* Display a message and go away... */
__attribute__((noreturn)) void quit (const char *fmt, ...)
{
    va_list va;
    va_start (va, fmt);
    vfprintf (stderr, fmt, va);
    va_end (va);

    cleanup ();

    exit (1);
}

static void version (void)
{
    cleanup ();

    fprintf (stdout, 
        "Version: %s\n", 
        program_version
    );
    exit (0);

}
static void help (void)
{
    cleanup ();

    fprintf (stdout, "Usage (%s):\n"
        "\t--help\\-h\n"
        "\t\tDisplay this help menu\n"
        "\t--version\\-v\n"
        "\t\tDisplay the program version\n"
        "\t--octal\\-O\n"
        "\t\tDump the bytes data in the octal format\n"
        "\t--hexa\\-H\n"
        "\t\tDump the bytes data in the hexa format\n"
        "\t--seek\\-s\n"
        "\t\tSeek 'N' bytes from the input\n"
        "\t--coll\\-C\n"
        "\t\tDefine the size of the collum\n"
        "\t--style\\-l\n"
        "\t\tThe output ASCII table style, can be one of them:\n"
        "\t\t0: alpha, 1: alphanumeric, 2: printable, 3: uppercase\n"
        "\t\t4: digits, 5: hexa digits\n"
        "\t--count\\-c\\-n\n"
        "\t\tThe count of bytes to display\n"
        "\t--input\\-i\n"
        "\t\tThe input filename\n",
        program_name
    );

    exit (0);
}

int32_t main (int argc, char **argv)
{
    char *fmt = "%02x%*c";
    int32_t bread = 0, index, block, table_size;
    uint32_t current_offset = 0;

    int (*look)(int) = NULL;

    char *bytes_table = NULL,
        *ascii_table = NULL;

    const struct option options[] = {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {"octal", no_argument, NULL, 'O'},
        {"hexa", no_argument, NULL, 'H'},
        {"seek", required_argument, NULL, 's'},
        {"coll", required_argument, NULL, 'C'},
        {"style", required_argument, NULL, 'l'},
        {"count", required_argument, NULL, 'c'},
        {"input", required_argument, NULL, 'i'},
        {}
    };

    const char *short_options = "hOHs:C:l:n:c:i:";
    int c = 0;
    int optindex;

    while ((c = getopt_long (argc, argv, short_options, options, &optindex)) != -1)
        switch (c) {
        case 'h':
            help ();
        case 'v':
            version ();
        case 'O':
            conv = 1;
            break;
        case 'H':
            conv = 0;
            break;
        case 's':
            seek = (uint32_t) strtoul (optarg, NULL, 0);
            break;
        case 'C':
            coll = (uint16_t) strtoul (optarg, NULL, 0);
            break;
        case 'l':
            style = (uint8_t) strtoul (optarg, NULL, 0);
            break;
        case 'n':
        case 'c':
            count = (int32_t) strtoul (optarg, NULL, 0);
            break;
        case 'i':
            filename = strdup (optarg);
            break;
        case '?': default:
            quit ("Option not recognized\n");
            break;
        }

    if (!filename)
        if (optind)
            if (argc > optind)
                if (argv[optind])
                    filename = strdup (argv[optind]);

    if (coll < 10 || coll > 24 || (coll % 2))
        quit ("Invalid collum size (%u)\n", coll);
    
    if (style >= (sizeof (look_at_me) / sizeof (look_at_me[0]) - 1))
        quit ("Style not found (%u)\n", style);
    look = look_at_me[style];

    if (conv)
        fmt = "%03o%*c";

    if (filename) {
        if (!(file = fopen (filename, "rb")))
            quit ("Can't open the file \"%s\"\n", filename);
    } else {
        file = stdin;
    }

    if (seek)
        if ((fseek (file, seek, SEEK_SET)))
            quit ("Can't seek the file \"%s\" to %u position\n", filename, seek);
    
    table_size = coll * log10 (coll * 2);
    assert ((bytes_table = calloc (table_size, sizeof (char))));
    assert ((ascii_table = calloc (table_size, sizeof (char))));

    do {
        block = count != -1 ? (count > coll 
            ? (coll % count) ? coll : count : count) : coll;
        memset (bytes_table, 0, table_size);
        if (!(bread = fread (bytes_table, 1, 
            block, file)))
            break;

        current_offset += bread;
        for (index = 0; index < bread; index++) {
            if (!index)
                /* Display the offset */
                printf ("%08x: ", current_offset);
            printf (fmt, (uint8_t)bytes_table[index], ((index + 1 == coll / 2) ? 2 : 1), ' ');
            
            /* Saving the ascii character representation in the ascii table */
            ascii_table[index] = (look (bytes_table[index]) ? bytes_table[index] : '.');
        }
        for (; index < coll; index++) {
            printf ("%*c", ((index + 1 == coll / 2) ? 4 : 3), ' ');
            ascii_table[index] = ' ';
        }

        printf ("%s\n", ascii_table);

        if (count > -1) {
            count -= bread;
            if (count <= 0)
                break;
        }
    
    } while (bread >= 0);

    free (bytes_table);
    free (ascii_table);

    cleanup ();

    return 0;

}


