
#include <stdio.h>
#include <stdint.h>

#include "cnode.h"

#include "cparsb.c.h"
#include "cparsl.c.h"

int yyparse();

int main(int argc, char **argv) {
    // parse through the input until there is no more:
    yyin = stdin;
    do {
        yyparse();
    } while (!feof(yyin));
}

