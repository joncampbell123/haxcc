
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <endian.h>

#include "cnode.h"

#include "cparsb.c.h"
#include "cparsl.c.h"

void c_node_init(struct c_node *node) {
    memset(node,0,sizeof(*node));
}

unsigned char char_width_b = 1;
unsigned char wchar_width_b = 2;
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

struct identifier_t {
    char*               name;
};

#define MAX_IDENTS      32768

struct identifier_t     idents[MAX_IDENTS];
int                     idents_count=0;

c_identref_t idents_ptr_to_ref(struct identifier_t *id) {
    return (c_identref_t)(id - &idents[0]);
}

void idents_free_item(struct identifier_t *s) {
    if (s->name != NULL) {
        free(s->name);
        s->name = NULL;
    }
}

void idents_free_all(void) {
    while (idents_count > 0) {
        idents_count--;
        idents_free_item(&idents[idents_count]);
    }
}

struct identifier_t *idents_find(const char *str) {
    struct identifier_t *id;
    int i=0;

    while (i < idents_count) {
        id = &idents[i++];
        if (id->name && !strcmp(str,id->name))
            return id;
    }

    return NULL;
}

struct identifier_t *idents_alloc(void) {
    struct identifier_t *id;

    if (idents_count >= MAX_IDENTS) {
        fprintf(stderr,"Out of identifiers\n");
        return NULL;
    }

    id = &idents[idents_count++];
    memset(id,0,sizeof(*id));
    return id;
}

c_identref_t identifier_parse(const char *str) {
    struct identifier_t *id;

    id = idents_find(str);
    if (id == NULL) {
        id = idents_alloc();
        if (id == NULL) {
            fprintf(stderr,"Cannot alloc identifier\n");
            return 0; // FIXME
        }

        id->name = strdup(str);
        if (id->name == NULL) {
            fprintf(stderr,"Cannot strdup ident name\n");
            return 0; // FIXME
        }

        fprintf(stderr,"Identifier '%s' (new)\n",id->name);
    }
    else {
        fprintf(stderr,"Identifier '%s' (already)\n",id->name);
    }

    return idents_ptr_to_ref(id);
}

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

void fconst_parse(struct c_node_val_float *val,const char *str) {
    // FIXME: This does not support the hexadecimal form!
    val->bwidth = double_width_b;
    val->val = strtod(str,(char**)(&str));

    if (*str == 'f' || *str == 'F') {
        val->bwidth = float_width_b;
        str++;
    }
    else if (*str == 'l' || *str == 'L') {
        val->bwidth = longdouble_width_b;
        str++;
    }

    fprintf(stderr,"Float const: %.9f w=%u\n",
        val->val,
        val->bwidth);
}

void iconst_parse(struct c_node_val_int *val,const char *str,const unsigned int base) {
    // will begin with:
    // 0x.... (octal)
    // 0...  (octal)
    // (decimal)
    //
    // will never begin with minus sign
    //
    // may end in L, l, U, u, LL, ll, etc.
    val->uint = (uint64_t)strtoul(str,(char**)(&str),0);
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

void iconst_parse_char(struct c_node_val_int *val,const char *str) {
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
    // debug
    extern int yydebug;
    yydebug = 1;

    // parse through the input until there is no more:
    yyin = stdin;
    do {
        yyparse();
    } while (!feof(yyin));
    yylex_destroy();

    strings_free_all();
    idents_free_all();
}

