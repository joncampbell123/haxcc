
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <endian.h>

#include "cnode.h"

#include "cparsb.c.h"
#include "cparsl.c.h"

struct c_node*              last_translation_unit = NULL;

int yyparse();
const char *token2string(int tok);

struct c_node *c_node_alloc(void) {
    struct c_node *n;

    n = malloc(sizeof(*n));
    if (n == NULL) return NULL;
    memset(n,0,sizeof(*n));
    return n;
}

struct c_node *c_node_alloc_or_die(void) {
    struct c_node *n = c_node_alloc();
    if (n != NULL) return n;

    fprintf(stderr,"Out of memory (unable to allocate c_node)\n");
    abort();
}

void c_string_free(char **s) {
    assert(s != NULL);

    if (*s != NULL) {
        free(*s);
        *s = NULL;
    }
}

void c_node_delete_struct(struct c_node *n) {
    unsigned int chidx;

    if (n != NULL) {
        if (n->refcount != 0) {
            /* this means someone isn't tracking their resources properly */
            fprintf(stderr,"WARNING: node ptr=%p deleting with refcount=%u\n",
                (void*)n,n->refcount);
        }
        if (n->prev != NULL) {
            fprintf(stderr,"WARNING: node ptr=%p deleting while still linked in a list (prev != NULL)\n",
                (void*)n);
        }
        if (n->next != NULL) {
            fprintf(stderr,"WARNING: node ptr=%p deleting while still linked in a list (next != NULL)\n",
                (void*)n);
        }
        if (n->parent != NULL) {
            fprintf(stderr,"WARNING: node ptr=%p deleting while still linked to parent node (parent != NULL)\n",
                (void*)n);
        }
        for (chidx=0;chidx < c_node_MAX_CHILDREN;chidx++) {
            if (n->child[chidx] != NULL) {
                fprintf(stderr,"WARNING: node ptr=%p deleting while still linked to child nodes (child[%u] != NULL)\n",
                    (void*)n,chidx);
            }
        }

        if (n->token == IDENTIFIER) {
            c_string_free(&(n->value.value_IDENTIFIER.name));
        }

        free(n);
    }
}

void c_node_delete(struct c_node **n) {
    assert(n != NULL);

    c_node_delete_struct(*n);
    *n = NULL;
}

unsigned int c_node_addref(struct c_node **n) {
    assert(n != NULL); /* if n == NULL the caller is an idiot and this program deserves to blow up */
    assert(*n != NULL); /* if *n == NULL the caller is trying to addref something he freed or never allocated (idiot) */
    return ++((*n)->refcount);
}

void c_node_delete_if_no_refs(struct c_node **n) {
    assert(n != NULL);

    if (*n != NULL) {
        if ((*n)->refcount == 0)
            c_node_delete(n);
    }
}

unsigned int c_node_release_autodelete(struct c_node **n) {
    unsigned int refs;

    assert(n != NULL);

    refs = c_node_release(n);
    c_node_delete_if_no_refs(n);
    return refs;
}

unsigned int c_node_release(struct c_node **n) {
    unsigned int ref = 0; /* can't pull a refcount from a deleted node, unless you want use-after-free bugs in this compiler */

    assert(n != NULL); /* if n == NULL the caller is an idiot and this program deserves to blow up */
    if (*n == NULL) return 0; /* releasing a NULL pointer is OK */

    if ((*n)->refcount != 0) {
        ref = --((*n)->refcount);
    }
    else {
        /* not tracking resources properly?
         * most likely cause: allocating a node (result refcount == 0) then calling release() when you should call delete instead. */
        fprintf(stderr,"WARNING: node ptr=%p release called with refcount=0\n",(void*)(*n));
    }

    return ref;
}

void c_node_move_to(struct c_node **d,struct c_node **s) {
    assert(s != NULL);
    assert(d != NULL);

    if (*d != NULL)
        fprintf(stderr,"WARNING: node_move_to destination node *d != NULL. pointer will be overwritten. memory leakage may result.\n");

    *d = *s;
    *s = NULL;
}

void c_node_copy_lineno(struct c_node *d,struct c_node *s) {
    d->lineno = s->lineno;
}

void c_node_release_prev_link(struct c_node *node) {
    if (node->prev != NULL) {
        c_node_release(&(node->prev)); /* node is letting go of node->prev */
        assert(node->prev->next == node);
        node->prev->next = NULL;
        c_node_release(&node); /* and node->prev is letting go of node */
        node->prev = NULL;
    }
}

void c_node_addref_prev_link(struct c_node *node) {
    if (node->prev != NULL) {
        c_node_addref(&(node->prev)); /* node is grabbing onto node->prev */
        node->prev->next = node;
        c_node_addref(&node); /* and node->prev is grabbing onto node */
    }
}

void c_node_move_to_prev_link(struct c_node *node,struct c_node **prev) {
    if (node->prev == *prev) return;

    c_node_release_prev_link(node);
    c_node_move_to(&(node->prev),prev); /* copies to node->prev, sets *prev = NULL */
    c_node_addref_prev_link(node);
}

void c_node_release_next_link(struct c_node *node) {
    if (node->next != NULL) {
        c_node_release(&(node->next)); /* node is letting go of node->next */
        assert(node->next->prev == node);
        node->next->prev = NULL;
        c_node_release(&node); /* and node->next is letting go of node */
        node->next = NULL;
    }
}

void c_node_addref_next_link(struct c_node *node) {
    if (node->next != NULL) {
        c_node_addref(&(node->next)); /* node is grabbing onto node->next */
        node->next->prev = node;
        c_node_addref(&node); /* and node->next is grabbing onto node */
    }
}

void c_node_move_to_next_link(struct c_node *node,struct c_node **next) {
    if (node->next == *next) return;

    c_node_release_next_link(node);
    c_node_move_to(&(node->next),next); /* copies to node->next, sets *next = NULL */
    c_node_addref_next_link(node);
}

void c_node_release_child_link(struct c_node *node,unsigned int chidx) {
    assert(chidx < c_node_MAX_CHILDREN);

    if (node->child[chidx] != NULL) {
        c_node_release(&(node->child[chidx])); /* node is letting go of child */
        assert(node->child[chidx]->parent == node);
        node->child[chidx]->parent = NULL;
        c_node_release(&node); /* and child is letting go of node */
        node->child[chidx] = NULL;
    }
}

void c_node_addref_child_link(struct c_node *node,unsigned int chidx) {
    assert(chidx < c_node_MAX_CHILDREN);

    if (node->child[chidx] != NULL) {
        c_node_addref(&(node->child[chidx])); /* node is grabbing onto child */
        node->child[chidx]->parent = node;
        c_node_addref(&node); /* and child is grabbing onto node */
    }
}

void c_node_move_to_child_link(struct c_node *node,unsigned int chidx,struct c_node **child) {
    assert(chidx < c_node_MAX_CHILDREN);

    if (node->child[chidx] == *child) return;

    c_node_release_child_link(node,chidx);
    c_node_move_to(&(node->child[chidx]),child); /* copies to node->child[chidx], sets *child = NULL */
    c_node_addref_child_link(node,chidx);
}

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

void c_node_f_constant_parse(struct c_node *d,char *s) {
    d->value.value_F_CONSTANT.bwidth = double_width_b; /* default double */
    d->value.value_F_CONSTANT.val = strtof(s,&s);

    if (*s == 'f' || *s == 'F')
        d->value.value_F_CONSTANT.bwidth = float_width_b;
    else if (*s == 'l' || *s == 'L')
        d->value.value_F_CONSTANT.bwidth = longdouble_width_b;
}

void c_node_i_constant_parse(struct c_node *d,char *s,int base) {
    unsigned char u=0,l=0;

    d->value.value_I_CONSTANT.bsign = 1; /* signed by default */
    d->value.value_I_CONSTANT.bwidth = int_width_b;
    d->value.value_I_CONSTANT.v.uint = (uint64_t)strtoull(s,&s,base);

    /* and then suffixes like l/L, u/U, ll/LL */
    while (*s == 'l' || *s == 'L' || *s == 'u' || *s == 'U') {
        if (*s == 'l' || *s == 'L') l++;
        else if (*s == 'u' || *s == 'U') u++;
        s++;
    }

    if (u) d->value.value_I_CONSTANT.bsign = 0;
    if (l >= 2) d->value.value_I_CONSTANT.bwidth = longlong_width_b;
    else if (l == 1) d->value.value_I_CONSTANT.bwidth = long_width_b;
}

void c_node_i_constant_char_parse(struct c_node *d,char *s) {
    unsigned int fw = 0;

    d->value.value_I_CONSTANT.v.uint = 0;
    if (*s == 'u') {
        d->value.value_I_CONSTANT.bsign = 1;
        d->value.value_I_CONSTANT.bwidth = 2;
        s++;
    }
    else if (*s == 'U') {
        d->value.value_I_CONSTANT.bsign = 1;
        d->value.value_I_CONSTANT.bwidth = 4;
        s++;
    }
    else if (*s == 'L') {
        d->value.value_I_CONSTANT.bsign = 1;
        d->value.value_I_CONSTANT.bwidth = wchar_width_b;
        s++;
    }
    else if (*s == '\'') {
        d->value.value_I_CONSTANT.bsign = 1;
        d->value.value_I_CONSTANT.bwidth = 1;
    }
    else {
        yyerror("I_CONSTANT char constant unknown char");
    }

    if (*s++ != '\'') return;

    while (*s != '\'') {
        /* TODO: Unicode handling */
        d->value.value_I_CONSTANT.v.uint <<= ((unsigned long long)d->value.value_I_CONSTANT.bwidth << 3ULL);
        d->value.value_I_CONSTANT.v.uint += strescp(&s);
        fw += d->value.value_I_CONSTANT.bwidth;
    }

    if (d->value.value_I_CONSTANT.bwidth < fw)
        d->value.value_I_CONSTANT.bwidth = fw;
}

void c_node_identifier_parse(struct c_node *d,char *s) {
    d->value.value_IDENTIFIER.id = c_identref_t_NONE;
    d->value.value_IDENTIFIER.name = strdup(s);
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

void fprintf_indent_node(FILE *fp,int indent) {
    int i = indent;
    while (i-- > 0) fprintf(fp,"  ");
    fprintf(fp,"[%d] ",indent);
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

c_stringref_t sconst_parse(char *str) {
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

void c_node_dumptree(struct c_node *n,int indent) {
    unsigned int chidx;

    for (;n!=NULL;n=n->next) {
        fprintf_indent_node(stderr,indent);
        fprintf(stderr,"node %p: token(%u)='%s' lineno=%u refcount=%u\n",
            (void*)n,n->token,token2string(n->token),n->lineno,n->refcount);

        if (n->token == F_CONSTANT) {
            fprintf_indent_node(stderr,indent+1);
            fprintf(stderr,"F_CONSTANT value=%.20f width=%u\n",
                n->value.value_F_CONSTANT.val,
                n->value.value_F_CONSTANT.bwidth);
        }
        else if (n->token == I_CONSTANT) {
            fprintf_indent_node(stderr,indent+1);
            fprintf(stderr,"I_CONSTANT value=0x%llx width=%u sign=%u\n",
                (unsigned long long)n->value.value_I_CONSTANT.v.uint,
                n->value.value_I_CONSTANT.bwidth,
                n->value.value_I_CONSTANT.bsign);
        }
        else if (n->token == IDENTIFIER) {
            fprintf_indent_node(stderr,indent+1);
            fprintf(stderr,"IDENTIFIER id=%ld name='%s'\n",
                (long)n->value.value_IDENTIFIER.id,
                n->value.value_IDENTIFIER.name);
        }

        if (n->next != NULL && n->next->prev != n) {
            fprintf_indent_node(stderr,indent+1);
            fprintf(stderr,"WARNING: node->next->prev != node\n");
        }
        if (n->prev != NULL && n->prev->next != n) {
            fprintf_indent_node(stderr,indent+1);
            fprintf(stderr,"WARNING: node->prev->next != node\n");
        }
        for (chidx=0;chidx < c_node_MAX_CHILDREN;chidx++) {
            if (n->child[chidx] != NULL && n->child[chidx]->parent != n) {
                fprintf_indent_node(stderr,indent+1);
                fprintf(stderr,"WARNING: node->child[%u]->parent != node\n",chidx);
            }
            if (n->child[chidx] != NULL) {
                fprintf_indent_node(stderr,indent+1);
                fprintf(stderr,"child node %u:\n",chidx);
                c_node_dumptree(n->child[chidx],indent+2);
            }
        }
    }
}

void c_node_delete_tree(struct c_node **node) {
    struct c_node *childnode = NULL;
    struct c_node *nextnode = NULL;
    unsigned int chidx;

    while (*node != NULL) {
        /* depth first */
        for (chidx=0;chidx < c_node_MAX_CHILDREN;chidx++) {
            childnode = (*node)->child[chidx]; /* save ptr. disconnect will zero array entry */
            c_node_release_child_link(*node,chidx); /* disconnect child from node */
            assert((*node)->child[chidx] == NULL); /* you did your job, RIIIIGHT? */
            c_node_delete_tree(&childnode); /* then recurse */
        }

        /* "prev" should be NULL, since we only follow "next" */
        assert((*node)->prev == NULL);

        /* disconnect from next ptr */
        nextnode = (*node)->next;
        c_node_release_next_link(*node);

        /* autodelete node */
        c_node_release_autodelete(node);

        /* next */
        *node = nextnode;
    }
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

    /* finish parsing final tree */
    if (last_translation_unit != NULL) {
        fprintf(stderr,"Resulting tree:\n");
        c_node_dumptree(last_translation_unit,1);
        c_node_delete_tree(&last_translation_unit);
        c_node_release(&last_translation_unit);
        c_node_delete(&last_translation_unit);
    }

    strings_free_all();
    return (res != 0) ? 1 : 0;
}

