
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>

#include "util.h"
#include "linesrc.h"
#include "linesink.h"

using namespace std;

static string                   in_file = "-";
static string                   out_file = "-";

static haxpp_linesink           out_ls;
static haxpp_linesource         in_ls;

static void help() {
    fprintf(stderr,"haxpp infile outfile\n");
}

static int parse_argv(int argc,char **argv) {
    int nwac=0;
    char *a;
    int i=1;

    while (i < argc) {
        a = argv[i++];

        if (*a == '-' && strcmp(a,"-") != 0/*not "-"*/) {
            do { a++; } while (*a == '-');

            if (!strcmp(a,"h") || !strcmp(a,"help")) {
                help();
                return 1;
            }
            else {
                fprintf(stderr,"Unknown switch %s\n",a);
                return 1;
            }
        }
        else {
            switch (nwac++) {
                case 0:
                    in_file = a;
                    break;
                case 1:
                    out_file = a;
                    break;
                default:
                    fprintf(stderr,"Unexpected arg %s\n",a);
                    return 1;
            }
        }
    }

    return 0;
}

int main(int argc,char **argv) {
    if (parse_argv(argc,argv))
        return 1;

    if (in_file == "-")
        in_ls.setsource(stdin);
    else
        in_ls.setsource(in_file);

    if (!in_ls.open()) {
        fprintf(stderr,"Unable to open infile, %s\n",strerror(errno));
        return 1;
    }

    if (out_file == "-")
        out_ls.setsink(stdout);
    else
        out_ls.setsink(out_file);

    if (!out_ls.open()) {
        fprintf(stderr,"Unable to open outfile, %s\n",strerror(errno));
        return 1;
    }

    while (!in_ls.eof()) {
        char *line = in_ls.readline();
        if (line == nullptr) {
            if (!in_ls.error() && in_ls.eof()) {
                break;
            }
            else {
                fprintf(stderr,"Problem reading. error=%u eof=%u errno=%s\n",in_ls.error(),in_ls.eof(),strerror(errno));
                return 1;
            }
        }

        if (!out_ls.write(line)) {
            fprintf(stderr,"Error writing output\n");
            return 1;
        }
    }

    if (in_ls.error()) {
        fprintf(stderr,"An error occurred while parsing %s\n",in_ls.getsourcename().c_str());
        return 1;
    }

    if (out_ls.error()) {
        fprintf(stderr,"An error occurred while parsing %s\n",out_ls.getsinkname().c_str());
        return 1;
    }

    out_ls.close();
    in_ls.close();
    return 0;
}

