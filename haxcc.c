
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <endian.h>

#include "cnode.h"

#include "cparsb.c.h"
#include "cparsl.c.h"

void yyerror(const char *s);

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

/* TODO: to allow scoping to work, scan backwards from last entry */
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

const char *idents_get_name(const c_identref_t x) {
    struct identifier_t *id;

    if (x >= idents_count)
        return NULL;

    id = &idents[x];
    return id->name;
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

    /* TODO: this should be written to use backwards scan of find.
     *       if the identifier found is below a "scope" boundary,
     *       then allow allocating another identifier. the duplicate
     *       value will be removed when the scope is closed. */
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

/* res = p1 * p2 */
int c_node_multiply(struct c_node *res,struct c_node *p1,struct c_node *p2) {
    if (p1->token == I_CONSTANT && p2->token == I_CONSTANT) {
        *res = *p1;

        /* promote the result to the larger of the two sizes */
        if (res->value.val_uint.bwidth < p2->value.val_uint.bwidth)
            res->value.val_uint.bwidth = p2->value.val_uint.bwidth;

        /* if the second one is signed, then the result of the first one is signed */
        if (p2->value.val_uint.bsign) res->value.val_uint.bsign = 1;

        /* do the multiply */
        res->value.val_uint.uint *= p2->value.val_uint.uint;

        /* done */
        return 1;
    }

    yyerror("Unsupported multiply");
    return 0;
}

/* res = <operator>p1 */
int c_node_unaryop(struct c_node *res,struct c_node *op,struct c_node *p1) {
    if (op->token == '+') {
        /* does nothing */
        *res = *p1;
        return 1;
    }
    else if (op->token == '-') { /* negation */
        if (p1->token == I_CONSTANT) {
            *res = *p1;
            res->value.val_uint.bsign = 1; /* negation makes it signed */
            res->value.val_uint.sint = -(res->value.val_uint.sint);
            return 1;
        }

        yyerror("Unsupported negate");
        return 0;
    }

    yyerror("Unsupported unary operator");
    return 0;
}

/* res = p1 / p2 */
int c_node_divide(struct c_node *res,struct c_node *p1,struct c_node *p2) {
    if (p1->token == I_CONSTANT && p2->token == I_CONSTANT) {
        *res = *p1;

        /* promote the result to the larger of the two sizes */
        if (res->value.val_uint.bwidth < p2->value.val_uint.bwidth)
            res->value.val_uint.bwidth = p2->value.val_uint.bwidth;

        /* if the second one is signed, then the result of the first one is signed */
        if (p2->value.val_uint.bsign) res->value.val_uint.bsign = 1;

        /* do the divide. watch out for divide by zero */
        if (p2->value.val_uint.uint == 0) {
            yyerror("Constant divide by zero, compile time expression eval error");
            return 0;
        }
        res->value.val_uint.uint /= p2->value.val_uint.uint;

        /* done */
        return 1;
    }

    yyerror("Unsupported divide");
    return 0;
}

/* res = p1 % p2 */
int c_node_modulus(struct c_node *res,struct c_node *p1,struct c_node *p2) {
    if (p1->token == I_CONSTANT && p2->token == I_CONSTANT) {
        *res = *p1;

        /* promote the result to the larger of the two sizes */
        if (res->value.val_uint.bwidth < p2->value.val_uint.bwidth)
            res->value.val_uint.bwidth = p2->value.val_uint.bwidth;

        /* if the second one is signed, then the result of the first one is signed */
        if (p2->value.val_uint.bsign) res->value.val_uint.bsign = 1;

        /* do the divide. watch out for divide by zero */
        if (p2->value.val_uint.uint == 0) {
            yyerror("Constant divide by zero (modulus), compile time expression eval error");
            return 0;
        }
        res->value.val_uint.uint %= p2->value.val_uint.uint;

        /* done */
        return 1;
    }

    yyerror("Unsupported modulus");
    return 0;
}

/* res = p1 + p2 */
int c_node_add(struct c_node *res,struct c_node *p1,struct c_node *p2) {
    if (p1->token == I_CONSTANT && p2->token == I_CONSTANT) {
        *res = *p1;

        /* promote the result to the larger of the two sizes */
        if (res->value.val_uint.bwidth < p2->value.val_uint.bwidth)
            res->value.val_uint.bwidth = p2->value.val_uint.bwidth;

        /* if the second one is signed, then the result of the first one is signed */
        if (p2->value.val_uint.bsign) res->value.val_uint.bsign = 1;

        /* do the addition */
        res->value.val_uint.uint += p2->value.val_uint.uint;

        /* done */
        return 1;
    }

    yyerror("Unsupported addition");
    return 0;
}

/* res = p1 - p2 */
int c_node_sub(struct c_node *res,struct c_node *p1,struct c_node *p2) {
    if (p1->token == I_CONSTANT && p2->token == I_CONSTANT) {
        *res = *p1;

        /* promote the result to the larger of the two sizes */
        if (res->value.val_uint.bwidth < p2->value.val_uint.bwidth)
            res->value.val_uint.bwidth = p2->value.val_uint.bwidth;

        /* if the second one is signed, then the result of the first one is signed */
        if (p2->value.val_uint.bsign) res->value.val_uint.bsign = 1;

        /* do the subtraction */
        res->value.val_uint.uint -= p2->value.val_uint.uint;

        /* done */
        return 1;
    }

    yyerror("Unsupported subtraction");
    return 0;
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

void c_node_init_decl(struct c_node_decl_spec *ds) {
    memset(ds,0,sizeof(*ds));
}

int c_node_funcspec_add(struct c_node_func_spec *fs,int token) {
    if (token == INLINE) {
        if (fs->is_inline) {
            yyerror("cannot specify inline more than once");
            return 0;
        }
        fs->is_inline = 1;
    }
    if (token == NORETURN) {
        if (fs->is_noreturn) {
            yyerror("cannot specify noreturn more than once");
            return 0;
        }
        fs->is_noreturn = 1;
    }
    return 1;
}

int c_node_funcspec_add_node(struct c_node_func_spec *fs,const struct c_node_func_spec *afs) {
    if (afs->is_inline && !c_node_funcspec_add(fs,INLINE))
        return 0;
    if (afs->is_noreturn && !c_node_funcspec_add(fs,NORETURN))
        return 0;

    return 1;
}

int c_node_funcspec_init(struct c_node_func_spec *fs,int token) {
    fs->is_inline = 0;
    fs->is_noreturn = 0;
    return c_node_funcspec_add(fs,token);
}

int c_node_typespec_init(struct c_node_type_spec *ts,int token) {
    /* should be SIGNED, UNSIGNED, INT, LONG, etc. */
    if (token == UNSIGNED || token == SIGNED) {
        ts->main_type = 0; // unspecified
        ts->bsign = (token == SIGNED) ? 1 : 0;
    }
    else {
        ts->main_type = token;
        ts->bsign = -1; // unspecified
    }

    return 1;
}

int c_node_typequal_add(struct c_node_type_qual *ts,int token) {
    if (token == CONST) {
        if (ts->is_const) {
            yyerror("type qualifier const specified more than once");
            return 0;
        }
        ts->is_const = 1;
    }
    else if (token == RESTRICT) {
        if (ts->is_restrict) {
            yyerror("type qualifier restrict specified more than once");
            return 0;
        }
        ts->is_restrict = 1;
    }
    else if (token == VOLATILE) {
        if (ts->is_volatile) {
            yyerror("type qualifier volatile specified more than once");
            return 0;
        }
        ts->is_volatile = 1;
    }
    else if (token == ATOMIC) {
        if (ts->is_atomic) {
            yyerror("type qualifier atomic specified more than once");
            return 0;
        }
        ts->is_atomic = 1;
    }
    else {
        yyerror("unexpected token in type qualifier stage");
        return 0;
    }

    return 1;
}

int c_node_typequal_add_node(struct c_node_type_qual *ts,const struct c_node_type_qual *ats) {
    if (ats->is_const && !c_node_typequal_add(ts,CONST))
        return 0;
    if (ats->is_restrict && !c_node_typequal_add(ts,RESTRICT))
        return 0;
    if (ats->is_volatile && !c_node_typequal_add(ts,VOLATILE))
        return 0;
    if (ats->is_atomic && !c_node_typequal_add(ts,ATOMIC))
        return 0;

    return 1;
}

int c_node_storageclass_add(struct c_node_storage_class *sc,int token) {
    if (token == TYPEDEF) {
        if (sc->is_typedef) {
            yyerror("typedef specified more than once");
            return 0;
        }
        sc->is_typedef = 1;
    }
    if (token == EXTERN) {
        if (sc->is_extern) {
            yyerror("extern specified more than once");
            return 0;
        }
        sc->is_extern = 1;
    }
    if (token == STATIC) {
        if (sc->is_static) {
            yyerror("static specified more than once");
            return 0;
        }
        sc->is_static = 1;
    }
    if (token == THREAD_LOCAL) {
        if (sc->is_thread_local) {
            yyerror("thread_local specified more than once");
            return 0;
        }
        sc->is_thread_local = 1;
    }
    if (token == AUTO) {
        if (sc->is_auto) {
            yyerror("auto specified more than once");
            return 0;
        }
        sc->is_auto = 1;
    }
    if (token == REGISTER) {
        if (sc->is_register) {
            yyerror("register specified more than once");
            return 0;
        }
        sc->is_register = 1;
    }

    if (sc->is_extern && sc->is_static) {
        yyerror("cannot specify static and extern at the same time");
        return 0;
    }

    return 1;
}

int c_node_storageclass_add_node(struct c_node_storage_class *ts,const struct c_node_storage_class *ats) {
    if (ats->is_typedef && !c_node_storageclass_add(ts,TYPEDEF))
        return 0;
    if (ats->is_extern && !c_node_storageclass_add(ts,EXTERN))
        return 0;
    if (ats->is_static && !c_node_storageclass_add(ts,STATIC))
        return 0;
    if (ats->is_thread_local && !c_node_storageclass_add(ts,THREAD_LOCAL))
        return 0;
    if (ats->is_auto && !c_node_storageclass_add(ts,AUTO))
        return 0;
    if (ats->is_register && !c_node_storageclass_add(ts,REGISTER))
        return 0;

    return 1;
}

int c_node_storageclass_init(struct c_node_storage_class *sc,int token) {
    sc->is_typedef = 0;
    sc->is_extern = 0;
    sc->is_static = 0;
    sc->is_thread_local = 0;
    sc->is_auto = 0;
    sc->is_register = 0;
    return c_node_storageclass_add(sc,token);
}

int c_node_typequal_init(struct c_node_type_qual *ts,int token) {
    ts->is_restrict = 0;
    ts->is_volatile = 0;
    ts->is_atomic = 0;
    ts->is_const = 0;
    return c_node_typequal_add(ts,token);
}

const char *typespec_token_to_str(const int tok) {
    switch (tok) {
        case 0:
            return "(unspecified)";
        case BOOL:
            return "BOOL";
        case CHAR:
            return "CHAR";
        case SHORT:
            return "SHORT";
        case INT:
            return "INT";
        case LONG:
            return "LONG";
        case SIGNED:
            return "SIGNED";
        case UNSIGNED:
            return "UNSIGNED";
        case FLOAT:
            return "FLOAT";
        case DOUBLE:
            return "DOUBLE";
        case VOID:
            return "VOID";
        case LONG_LONG:
            return "LONG-LONG";
        case LONG_DOUBLE:
            return "LONG-DOUBLE";
        default:
            break;
    }

    return "?";
}

void c_node_storageclass_dump(const struct c_node_storage_class * const sc) {
    if (sc->is_typedef)
        fprintf(stderr,"typedef ");
    if (sc->is_extern)
        fprintf(stderr,"extern ");
    if (sc->is_static)
        fprintf(stderr,"static ");
    if (sc->is_thread_local)
        fprintf(stderr,"thread_local ");
    if (sc->is_auto)
        fprintf(stderr,"auto ");
    if (sc->is_register)
        fprintf(stderr,"register ");

    fprintf(stderr,"\n");
}

void c_node_funcspec_dump(const struct c_node_func_spec * const fs) {
    if (fs->is_inline)
        fprintf(stderr,"inline ");
    if (fs->is_noreturn)
        fprintf(stderr,"noreturn ");

    fprintf(stderr,"\n");
}

void c_node_typequal_dump(const struct c_node_type_qual * const tq) {
    if (tq->is_const)
        fprintf(stderr,"const ");
    if (tq->is_restrict)
        fprintf(stderr,"restrict ");
    if (tq->is_volatile)
        fprintf(stderr,"volatile ");
    if (tq->is_atomic)
        fprintf(stderr,"atomic ");

    fprintf(stderr,"\n");
}

void c_node_typespec_dump(const struct c_node_type_spec * const ts) {
    if (ts->bsign > 0)
        fprintf(stderr,"signed");
    else if (ts->bsign == 0)
        fprintf(stderr,"unsigned");
    else
        fprintf(stderr,"(sign unspecified)");
    fprintf(stderr," ");

    fprintf(stderr,"type=%s",typespec_token_to_str(ts->main_type));

    fprintf(stderr,"\n");
}

int c_node_typespec_add(struct c_node_type_spec *ts,int token) {
    fprintf(stderr,"typespec comb %u and %u\n",ts->main_type,token);

    /* should be SIGNED, UNSIGNED, INT, LONG, etc. */
    if (token == UNSIGNED || token == SIGNED) {
        signed char ns = (token == SIGNED) ? 1 : 0;

        if (ts->bsign == -1)
            ts->bsign = ns;
        else if (ts->bsign == ns)
            fprintf(stderr,"warning: unnecessary duplicate signed/unsigned typespec\n");
        else {
            yyerror("signed/unsigned duplicate definition");
            return 0;
        }
    }
    else if (ts->main_type == 0) {
        /* has not yet been specified */
        ts->main_type = token;
    }
    else if (ts->main_type == LONG && token == LONG) {
        ts->main_type = LONG_LONG; /* C99 long long */
    }
    else if ((ts->main_type == INT && token == LONG) || (ts->main_type == LONG && token == INT)) {
        /* long int / int long becomes long */
        ts->main_type = LONG;
    }
    else if ((ts->main_type == INT && token == SHORT) || (ts->main_type == SHORT && token == INT)) {
        /* short int / int short becomes short */
        ts->main_type = SHORT;
    }
    else if ((ts->main_type == DOUBLE && token == LONG) || (ts->main_type == LONG && token == DOUBLE)) {
        /* long double / double long becomes LONG_DOUBLE token */
        ts->main_type = LONG_DOUBLE;
    }
    else {
        /* duplicate type specifiers, which is invalid */
        yyerror("duplicate/multiple type specifiers");
        return 0;
    }

    return 1;
}

int c_node_on_type_spec(struct c_node *typ) { /* convert to TYPE_SPECIFIER */
    if (typ->token == TYPE_SPECIFIER)
        return 1;

    if (!c_node_typespec_init(&typ->value.val_type_spec,typ->token))
        return 0;

    typ->token = TYPE_SPECIFIER;
    return 1;
}

int c_node_on_type_qual(struct c_node *typ) { /* convert to TYPE_QUALIFIER */
    if (typ->token == TYPE_QUALIFIER)
        return 1;

    if (!c_node_typequal_init(&typ->value.val_type_qual,typ->token))
        return 0;

    typ->token = TYPE_QUALIFIER;
    return 1;
}

int c_node_on_storage_class_spec(struct c_node *stc) {
    if (stc->token == STORAGE_CLASS_SPECIFIER)
        return 1;

    if (!c_node_storageclass_init(&stc->value.val_storage_class,stc->token))
        return 0;

    stc->token = STORAGE_CLASS_SPECIFIER;
    return 1;
}

int c_node_on_func_spec(struct c_node *typ) { /* convert to FUNC_SPECIFIER */
    if (typ->token == FUNC_SPECIFIER)
        return 1;

    if (!c_node_funcspec_init(&typ->value.val_func_spec,typ->token))
        return 0;

    typ->token = FUNC_SPECIFIER;
    return 1;
}

int c_node_type_to_decl(struct c_node *typ) { /* convert TYPE_SPECIFIER to DECL_SPECIFIER */
    if (typ->token == DECL_SPECIFIER)
        return 1;

    /* I'm concerned since in the union the members overlap, that's why the extra copying.
     * Yes, this design stinks. We'll improve it later. */
    if (typ->token == TYPE_SPECIFIER) {
        struct c_node_type_spec ts;

        ts = typ->value.val_type_spec;
        c_node_init_decl(&typ->value.val_decl_spec);
        typ->value.val_decl_spec.typespec = ts;
        typ->token = DECL_SPECIFIER;
    }
    else if (typ->token == TYPE_QUALIFIER) {
        struct c_node_type_qual tq;

        tq = typ->value.val_type_qual;
        c_node_init_decl(&typ->value.val_decl_spec);
        typ->value.val_decl_spec.typequal = tq;
        typ->token = DECL_SPECIFIER;
    }
    else if (typ->token == STORAGE_CLASS_SPECIFIER) {
        struct c_node_storage_class sc;

        sc = typ->value.val_storage_class;
        c_node_init_decl(&typ->value.val_decl_spec);
        typ->value.val_decl_spec.storageclass = sc;
        typ->token = DECL_SPECIFIER;
    }
    else if (typ->token == FUNC_SPECIFIER) {
        struct c_node_func_spec fs;

        fs = typ->value.val_func_spec;
        c_node_init_decl(&typ->value.val_decl_spec);
        typ->value.val_decl_spec.funcspec = fs;
        typ->token = DECL_SPECIFIER;
    }
    else {
        yyerror("Unexpected type node");
        return 0;
    }

    return 1;
}

void c_init_decl_node_init(struct c_init_decl_node *i) {
    memset(i,0,sizeof(*i));
    i->identifier = c_identref_t_NONE;

}

struct c_init_decl_node *alloc_init_decl_node(void) {
    struct c_init_decl_node *n = (struct c_init_decl_node*)malloc(sizeof(struct c_init_decl_node));
    if (n == NULL) return NULL;
    c_init_decl_node_init(n);
    return n;
}

int c_node_add_declaration_init_decl(struct c_node *decl,struct c_node *initdecl) {
    assert(decl->token == DECL_SPECIFIER);
    assert(initdecl->token == INIT_DECL_LIST);

    if (decl->value.val_decl_spec.init_decl_list != NULL) {
        yyerror("declaration init decl list already initialized");
        return 0;
    }

    if (initdecl->value.init_decl_list != NULL) {
        /* you just move the list from one to the other, done */
        decl->value.val_decl_spec.init_decl_list =
            initdecl->value.init_decl_list;
        initdecl->value.init_decl_list =
            NULL;
    }

    return 1;
}

void init_c_node_initializer(struct c_node_initializer *n) {
    memset(n,0,sizeof(*n));
}

struct c_node_initializer *alloc_c_node_initializer(void) {
    struct c_node_initializer *n = (struct c_node_initializer*)malloc(sizeof(struct c_node_initializer));
    if (n == NULL) return NULL;
    init_c_node_initializer(n);
    return n;
}

int c_node_init_decl_attach_initializer(struct c_node *decl,struct c_node *init) {
    assert(decl->token == INIT_DECL_LIST);
    assert(init->token == INITIALIZER);

    if (init->value.initializer == NULL)
        return 1;

    if (decl->value.init_decl_list == NULL) {
        yyerror("init decl list == NULL nothing to attach initializer to");
        return 0;
    }
    if (decl->value.init_decl_list->next != NULL) {
        yyerror("init decl list has more than one element, cannot attach initializer to");
        return 0;
    }
    decl->value.init_decl_list->initializer = init->value.initializer;
    init->value.initializer = NULL;
    return 1;
}

int c_node_convert_to_initializer(struct c_node *decl) {
    struct c_node_initializer *cinit;

    if (decl->token == INITIALIZER)
        return 1;

    if (decl->token == IDENTIFIER ||
        decl->token == I_CONSTANT ||
        decl->token == F_CONSTANT ||
        decl->token == STRING_LITERAL ||
        decl->token == ENUMERATION_CONSTANT) {
        cinit = alloc_c_node_initializer();
        if (cinit == NULL) {
            yyerror("cannot alloc initializer");
            return 0;
        }

        cinit->node = *decl;
        decl->value.initializer = cinit;
    }
    else {
        fprintf(stderr,"init tok=%u\n",decl->token);
        yyerror("initializer not supported");
        return 0;
    }

    decl->token = INITIALIZER;
    return 1;
}

void c_init_decl_append(struct c_node *d,struct c_node *s) {
    if (s->value.init_decl_list == NULL)
        return; /* nothing to append */

    if (d->value.init_decl_list != NULL) {
        struct c_init_decl_node *n = d->value.init_decl_list;
        while (n->next != NULL) n=n->next;
        n->next = s->value.init_decl_list;
        s->value.init_decl_list = NULL;
    }
    else {
        d->value.init_decl_list = s->value.init_decl_list;
        s->value.init_decl_list = NULL;
    }
}

int c_node_add_init_decl(struct c_node *decl,struct c_node *initdecl) {
    assert(initdecl->token == INIT_DECL_LIST);
    assert(decl->token == INIT_DECL_LIST);
    c_init_decl_append(decl,initdecl);
    return 1;
}

int c_node_on_init_decl(struct c_node *typ) { /* convert to INIT_DECL_LIST */
    if (typ->token == INIT_DECL_LIST)
        return 1;

    if (typ->token == IDENTIFIER) {
        struct c_init_decl_node *idn = alloc_init_decl_node();
        if (idn == NULL) {
            yyerror("unable to alloc init decl node");
            return 0;
        }

        idn->identifier = typ->value.val_identifier;
        typ->value.init_decl_list = idn;
    }
    else {
        yyerror("init decl unexpected node");
        return 0;
    }

    typ->token = INIT_DECL_LIST;
    return 1;
}

int c_node_add_type_to_decl(struct c_node *decl,struct c_node *typ) {
    assert(decl->token == DECL_SPECIFIER);

    if (typ->token == TYPE_SPECIFIER) {
        int add_type;

        add_type = typ->value.val_type_spec.main_type;
        if (add_type == 0) {
            if (typ->value.val_type_spec.bsign < 0)
                return 1;

            add_type = typ->value.val_type_spec.bsign > 0 ? SIGNED : UNSIGNED;
        }

        if (!c_node_typespec_add(&decl->value.val_decl_spec.typespec,add_type))
            return 0;
    }
    else if (typ->token == TYPE_QUALIFIER) {
        if (!c_node_typequal_add_node(&decl->value.val_decl_spec.typequal,&typ->value.val_type_qual))
            return 0;
    }
    else if (typ->token == STORAGE_CLASS_SPECIFIER) {
        if (!c_node_storageclass_add_node(&decl->value.val_decl_spec.storageclass,&typ->value.val_storage_class))
            return 0;
    }
    else if (typ->token == FUNC_SPECIFIER) {
        if (!c_node_funcspec_add_node(&decl->value.val_decl_spec.funcspec,&typ->value.val_func_spec))
            return 0;
    }
    else {
        yyerror("unexpected node type, adding to decl");
        fprintf(stderr,"Typ token=%u\n",typ->token);
        return 0;
    }

    return 1;
}

void c_init_decl_node_dump(struct c_init_decl_node *n) {
    fprintf(stderr,"init decl entry:");
    if (n->identifier != c_identref_t_NONE) {
        const char *name = idents_get_name(n->identifier);
        if (name != NULL) fprintf(stderr," identifier=\"%s\"",name);
    }
    if (n->initializer != NULL) {
        struct c_node_initializer *in = n->initializer;

        fprintf(stderr," { ");
        for (;in != NULL;in=in->next) {
            if (in->node.token == I_CONSTANT) {
                fprintf(stderr,"iconst=0x%llx ",(unsigned long long)in->node.value.val_uint.uint);
            }
            else if (in->node.token == IDENTIFIER) {
                const char *name = idents_get_name(in->node.value.val_identifier);
                fprintf(stderr,"ident=\"%s\" ",name?name:"(null)");
            }
            else {
                fprintf(stderr,"tok=%u ",in->node.token);
            }
        }
        fprintf(stderr,"}");
    }
    fprintf(stderr,"\n");
}

int c_node_finish_declaration(struct c_node *decl) {
    assert(decl->token == DECL_SPECIFIER);

    fprintf(stderr,"Finished declaration:\n");
    fprintf(stderr,"  typespec: ");
    c_node_typespec_dump(&decl->value.val_decl_spec.typespec);
    fprintf(stderr,"  typequal: ");
    c_node_typequal_dump(&decl->value.val_decl_spec.typequal);
    fprintf(stderr,"  storageclass: ");
    c_node_storageclass_dump(&decl->value.val_decl_spec.storageclass);
    fprintf(stderr,"  funcspec: ");
    c_node_funcspec_dump(&decl->value.val_decl_spec.funcspec);
    fprintf(stderr,"  init decl:\n");

    {
        struct c_init_decl_node *n = decl->value.val_decl_spec.init_decl_list;
        for (;n != NULL;n=n->next) {
            fprintf(stderr,"    ");
            c_init_decl_node_dump(n);
        }
    }

    return 1;
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

