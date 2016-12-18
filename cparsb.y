%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// stuff from flex that bison needs to know about:
int yylex();
int yyparse();
FILE *yyin;
 
void yyerror(const char *s);
%}

// Bison fundamentally works by asking flex to get the next token, which it
// returns as an object of type "yystype".  But tokens could be of any
// arbitrary data type!  So we deal with that in Bison by defining a C union
// holding each of the types of tokens that Flex could return, and have Bison
// use that union instead of "int" for the definition of "yystype":
%union {
    int ival;
    float fval;
    char *sval;
}

// define the constant-string tokens:
%token SNAZZLE TYPE
%token END ENDL

// define the "terminal symbol" token types I'm going to use (in CAPS
// by convention), and associate each with a field of the union:
%token <ival> INT
%token <fval> FLOAT
%token <sval> STRING

%%
// this is the actual grammar that bison will parse, but for right now it's just
// something silly to echo to the screen what bison gets from flex.  We'll
// make a real one shortly:
snazzle:
      snazzle INT    { fprintf(stderr,"int: %d\n",$2); }
    | snazzle FLOAT  { fprintf(stderr,"float: %.3f\n",$2); }
    | snazzle STRING { fprintf(stderr,"string: %s\n",$2); }
    | INT            { fprintf(stderr,"int: %d\n",$1); }
    | FLOAT          { fprintf(stderr,"float: %.3f\n",$1); }
    | STRING         { fprintf(stderr,"string: %s\n",$1); }
    ;
%%

int main(int argc, char **argv) {
    // parse through the input until there is no more:
    do {
        yyparse();
    } while (!feof(yyin));
}

void yyerror(const char *s) {
    fprintf(stderr,"Error: %s\n",s);
    exit(-1);
}

