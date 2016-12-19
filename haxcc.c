
#include <stdio.h>
#include <stdint.h>

#include "cnode.h"

#include "cparsb.c.h"
#include "cparsl.c.h"

unsigned char char_width_b = 1;
unsigned char wchar_width_b = 2;
unsigned char int_width_b = 2;
unsigned char long_width_b = 4;
unsigned char longlong_width_b = 8;

unsigned char octtobin(const char c) {
    if (c >= '0' && c <= '7')
        return c - '0';

    return 0;
}

unsigned char hextobin(const char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return c + 10 - 'a';
    else if (c >= 'A' && c <= 'F')
        return c + 10 - 'A';

    return 0;
}

uint64_t strescp_o(char ** const s) {
    uint64_t v;

    // \nnn  where nnn is octal
    v  = (uint64_t)octtobin(*((*s)++)) << (uint64_t)6U;
    v += (uint64_t)octtobin(*((*s)++)) << (uint64_t)3U;
    v += (uint64_t)octtobin(*((*s)++));
    return v;
}

uint64_t strescp_x(char ** const s) {
    uint64_t v;

    // \xXX  where XX is hex
    v  = (uint64_t)hextobin(*((*s)++)) << (uint64_t)4U;
    v += (uint64_t)hextobin(*((*s)++));

    return v;
}

uint64_t strescp(char ** const s) {
    uint64_t val = 0;

    if (*(*s) == '\\') {
        (*s)++;

        switch (*((*s)++)) {
            case 'a':
                return 7;
            case 'b':
                return 8;
            case 't':
                return 9;
            case 'n':
                return 10;
            case 'v':
                return 11;
            case 'l':
                return 12;
            case 'r':
                return 13;
            case '\"':
                return 0x22;
            case '\'':
                return 0x27;
            case '\?':
                return 0x3F;
            case '\\':
                return 0x5C;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return strescp_o(s);//octal
            case 'x':
                return strescp_x(s);//hexadecimal
            default:
                return 0; // TODO
        };
    }
    else {
        val = *((*s)++); // deref pointer, postincrement, read char prior to p.i.
    }

    return val;
}

void iconst_parse(struct c_node_val_int *val,char *str,const unsigned int base) {
    // will begin with:
    // 0x.... (octal)
    // 0...  (octal)
    // (decimal)
    //
    // will never begin with minus sign
    //
    // may end in L, l, U, u, LL, ll, etc.
    val->uint = (uint64_t)strtoul(str,&str,0);
    val->bwidth = int_width_b; // default width by default
    val->bsign = 1; // signed by default

    // if followed by U or u then unsigned
    if (*str == 'u' || *str == 'U') {
        val->bsign = 0;
        str++;
    }

    if (*str == 'l' || *str == 'L') {
        val->bwidth = long_width_b;
        str++;
        if (*str == 'l' || *str == 'L') {
            val->bwidth = longlong_width_b;
            str++;
        }
    }

    fprintf(stderr,"Integer const 0x%llX w=%u s=%u\n",
        (unsigned long long)val->uint,
        val->bwidth,
        val->bsign);
}

void iconst_parse_char(struct c_node_val_int *val,char *str) {
    unsigned int shf = 0;
    unsigned int chw;
    uint64_t cval;

    // takes the form 'c'
    // can also take wide form L'c'
    // can also take unsigned form U'c' (really?)
    // can also take nonstanrd form 'FORM' i.e. for FOURCCs
    val->bwidth = char_width_b;
    val->bsign = 1;
    do {
        if (*str == 'u' || *str == 'U') {
            val->bsign = 0;
            str++;
        }
        else if (*str == 'l' || *str == 'L') {
            val->bwidth = wchar_width_b;
            str++;
        }
        else {
            break;
        }
    } while (1);

    if (*str++ != '\'') {
        fprintf(stderr,"BUG: parse char did not find starting '\n");
        return; // FIXME: what?
    }

    /* the rest is string const */
    // FIXME: I don't know what compilers are supposed to do with multi-char wide char const,
    //        but I do know that most compilers happen to allow something like 'RIFF' to
    //        assign a 32-bit const that becomes the char sequence 'R','I','F','F' in the
    //        native byte order.
    chw = val->bwidth;
    val->uint = 0;
    while (*str) {
        if (*str == '\'')
            break;

        if ((shf+chw) > sizeof(uint64_t)) {
            fprintf(stderr,"Const too big\n");
            break;
        }

        cval = strescp(&str);
        val->uint += (uint64_t)cval << ((uint64_t)shf * (uint64_t)8U);
        shf += chw;
    }

    fprintf(stderr,"Integer const 0x%llX w=%u s=%u\n",
        (unsigned long long)val->uint,
        val->bwidth,
        val->bsign);
}

int yyparse();

int main(int argc, char **argv) {
    // parse through the input until there is no more:
    yyin = stdin;
    do {
        yyparse();
    } while (!feof(yyin));
}

