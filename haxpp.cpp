
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

using namespace std;

int main(int argc,char **argv) {
    for (int i=1;i < argc;i++) {
        haxpp_linesource ls;

        ls.setsource(argv[i]);
        if (!ls.open()) {
            fprintf(stderr,"Unable to open %s, error %s\n",ls.getsourcename().c_str(),strerror(errno));
            return 1;
        }

        while (!ls.eof()) {
            char *line = ls.readline();
            if (line == NULL) {
                if (!ls.error() && ls.eof()) {
                    break;
                }
                else {
                    fprintf(stderr,"Problem reading. error=%u eof=%u errno=%s\n",ls.error(),ls.eof(),strerror(errno));
                    return 1;
                }
            }

            printf("%s",line); /* often provides it's own newline */
        }

        if (ls.error()) {
            fprintf(stderr,"An error occurred while parsing %s\n",ls.getsourcename().c_str());
            return 1;
        }
    }

    return 0;
}

