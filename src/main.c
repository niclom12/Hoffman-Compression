#include <stdio.h>
#include <string.h>
#include "compress.h"
#include "huffsize.h"

static void usage(const char *exe) {
    fprintf(stderr,
        "Usage:\n"
        "  %s c <input> <output>   # compress\n"
        "  %s d <input> <output>   # decompress\n", exe, exe);
}

int main(int argc, char *argv[])
{
    if (argc!=4) { 
        usage(argv[0]); 
        return 1; 
    }

    if (argv[1][0]=='c'){
       return compress(argv[2], argv[3]);
    }

    else if (argv[1][0]=='d') {
        return decompress(argv[2], argv[3]);
    }
        
    else {
        usage(argv[0]); return 1;
    }
}
