
#include <stdio.h>

#include "cparsl.c.h"

int main(int argc, char **argv) {
    // parse through the input until there is no more:
    yyin = stdin;
    do {
        yyparse();
    } while (!feof(yyin));
}

