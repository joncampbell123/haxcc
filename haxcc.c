
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <endian.h>

#include "cnode.h"

#include "cparsb.c.h"
#include "cparsl.c.h"

int yyparse();

unsigned char char_width_b = 1;
unsigned char wchar_width_b = 2;
unsigned char short_width_b = 2;
unsigned char int_width_b = 2;
unsigned char long_width_b = 4;
unsigned char longlong_width_b = 8;
unsigned char double_width_b = 8;
unsigned char float_width_b = 4;
unsigned char longdouble_width_b = 10;

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

uint64_t strescp_o(const char ** const s) {
    uint64_t v;

    // \nnn  where nnn is octal
    v  = (uint64_t)octtobin(*((*s)++)) << (uint64_t)6U;
    v += (uint64_t)octtobin(*((*s)++)) << (uint64_t)3U;
    v += (uint64_t)octtobin(*((*s)++));
    return v;
}

uint64_t strescp_x(const char ** const s) {
    uint64_t v;

    // \xXX  where XX is hex
    v  = (uint64_t)hextobin(*((*s)++)) << (uint64_t)4U;
    v += (uint64_t)hextobin(*((*s)++));

    return v;
}

uint64_t strescp(const char ** const s) {
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

/* TODO: when ready, retrieve I_CONST, F_CONST, and char const parsing from git tag "this-try-sucks-20170305-1256-but-can-do-some-neato-global-alignment" */

struct string_t {
    unsigned char       wchar;      // L prefix
    unsigned char       utf8;       // u8 prefix
    unsigned char       bsign;      // signed char array (FIXME: so what does an unsigned string do?)
    unsigned char*      bytes;      // byte array of string
    size_t              bytelen;    // length of string
};

#define MAX_STRINGS     16384

struct string_t         strings[MAX_STRINGS];
int                     strings_count=0;

uint64_t width2mask(unsigned char w) {
    if (w == 0) return 0;
    return ((uint64_t)1ULL << ((uint64_t)w * (uint64_t)8ULL)) - (uint64_t)1ULL; /* x,8,16,24,32,40,48,56,64 */
}

uint64_t width2mask_signbit(unsigned char w) {
    if (w == 0) return 0;
    return ((uint64_t)1ULL << (((uint64_t)w * (uint64_t)8ULL) - (uint64_t)1ULL)); /* x,7,15,23,31,39,47,55,63 */
}

void fprintf_indent(FILE *fp,int indent) {
    while (indent-- > 0) fprintf(fp,"    ");
}

void strings_free_item(struct string_t *s) {
    if (s->bytes != NULL) {
        free(s->bytes);
        s->bytes = NULL;
    }
    s->bytelen = 0;
}

void strings_free_all(void) {
    while (strings_count > 0) {
        strings_count--;
        strings_free_item(&strings[strings_count]);
    }
}

struct string_t *strings_alloc(void) {
    struct string_t *r;

    if (strings_count >= MAX_STRINGS)
        return NULL;

    r = &strings[strings_count++];
    memset(r,0,sizeof(*r));
    r->bsign = 1;
    return r;
}

c_stringref_t string_t_to_ref(struct string_t *r) {
    return (c_stringref_t)(r - &strings[0]);
}

c_stringref_t sconst_parse(const char *str) {
    struct string_t *r = strings_alloc();
    unsigned char *tmp;
    size_t tmp_alloc;
    size_t tmp_len;
    uint64_t cval;

    if (r == NULL) {
        fprintf(stderr,"Unable to alloc string ref\n");
        return 0; // FIXME
    }

    do {
        if (*str == 'u' || *str == 'U') {
            str++;
            if (*str == '8') {
                r->utf8 = 1;
                str++;
            }
            else {
                r->bsign = 0;
            }
        }
        else if (*str == 'l' || *str == 'L') {
            r->wchar = 1;
            str++;
        }
        else {
            break;
        }
    } while(1);

    if (*str++ != '\"') {
        fprintf(stderr,"String did not begin with \"\n");
        return 0; // FIXME
    }

    tmp_alloc = 128;
    tmp_len = 0;
    tmp = malloc(tmp_alloc);
    if (tmp == NULL) {
        fprintf(stderr,"Unable to alloc str\n");
        return 0; // FIXME
    }

    while (*str) {
        if (*str == '\"')
            break;

        if ((tmp_len+10) >= tmp_alloc) {
            size_t nalloc = tmp_alloc * 2U;
            unsigned char *np;

            np = (unsigned char*)realloc((void*)tmp,nalloc);
            if (np == NULL) {
                fprintf(stderr,"Out of string space\n");
                return 0; // FIXME
            }

            tmp = np;
            tmp_alloc = nalloc;
        }

        cval = strescp(&str);
        if (r->utf8) {
            // TODO
        }
        else if (r->wchar) {
            if (wchar_width_b == 2) {
                *((uint16_t*)(tmp+tmp_len)) = htole16((uint16_t)cval);
                tmp_len += 2;
            }
            else if (wchar_width_b == 4) {
                *((uint32_t*)(tmp+tmp_len)) = htole32((uint32_t)cval);
                tmp_len += 4;
            }
            else {
                abort();
            }
        }
        else {
            // FIXME: Obviously, this doesn't handle multibyte character encodings.
            tmp[tmp_len++] = (char)cval;
        }
    }

    assert((tmp_len+1) <= tmp_alloc);
    tmp[tmp_len] = 0;

    r->bytes = tmp;
    r->bytelen = tmp_len;

    fprintf(stderr,"String const ref=%llu \"%s\" len=%zu w=%u u=%u s=%u\n",
        (unsigned long long)string_t_to_ref(r),
        (r->bytes!=NULL?(char*)r->bytes:""),
        r->bytelen,
        r->wchar,
        r->utf8,
        r->bsign);

    return string_t_to_ref(r);
}

int main(int argc, char **argv) {
    int res;

    // debug
    extern int yydebug;
    yydebug = 1;

    // parse through the input until there is no more:
    res = 0;
    yyin = stdin;
    do {
        if ((res=yyparse()) != 0) break;
    } while (!feof(yyin));
    yylex_destroy();

    strings_free_all();
    return (res != 0) ? 1 : 0;
}

