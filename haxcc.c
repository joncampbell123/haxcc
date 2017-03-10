
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

        if (n->token == IDENTIFIER || n->token == ENUMERATION_CONSTANT || n->token == TYPEDEF_NAME) {
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

void c_node_scan_to_parent_head(struct c_node **n) {
    assert(n != NULL);

    while (*n && (*n)->parent != NULL)
        *n = (*n)->parent;
}

void c_node_scan_to_head(struct c_node **n) {
    assert(n != NULL);

    while (*n && (*n)->prev != NULL)
        *n = (*n)->prev;
}

void c_node_scan_to_end(struct c_node **n) {
    assert(n != NULL);

    while (*n && (*n)->next != NULL)
        *n = (*n)->next;
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

void c_node_move_to_parent_link(struct c_node *node,struct c_node **next) {
    unsigned int chidx = 0;
    struct c_node *p;

    assert(next != NULL);
    p = (*next);

    while (chidx < c_node_MAX_CHILDREN) {
        if (p->child[chidx] == NULL) {
            c_node_move_to(&(p->child[chidx]),&node);
            c_node_addref_child_link(p,chidx);
            *next = NULL;
            return;
        }

        chidx++;
    }

    fprintf(stderr,"move_to_parent_link no child nodes in parent\n");
}

/* FIXME: test this code more */
void c_node_insert_right_before_in_list(struct c_node **tn,struct c_node *sn) {
    struct c_node *prev;

    assert(tn != NULL);
    if (*tn == NULL || sn == NULL) return;

    /* node must not already be in list */
    assert(sn->prev == NULL);
    assert(sn->next == NULL);

    /* store prev link */
    prev = (*tn)->prev;

    /* update tn->prev to point to sn */
    c_node_release_prev_link(*tn);
    (*tn)->prev = sn;
    c_node_addref_prev_link(*tn);
    assert(sn->next == (*tn));

    /* update sn->prev to point to old (*tn)->prev. sn->next == NULL already, we checked */
    sn->prev = prev;
    c_node_addref_prev_link(sn);
    if (prev != NULL) { assert(prev->next == sn); };

    *tn = sn;
}

void c_node_insert_after_in_list(struct c_node *tn,struct c_node *sn) {
    struct c_node *next;

    if (tn == NULL || sn == NULL) return;

    /* node must not already be in list */
    assert(sn->prev == NULL);
    assert(sn->next == NULL);

    /* store next link */
    next = tn->next;

    /* update tn->next to point to sn */
    c_node_release_next_link(tn);
    tn->next = sn;
    c_node_addref_next_link(tn);
    assert(sn->prev == tn);

    /* update sn->next to point to old tn->next. remember sn->next == NULL already, we assert()'d on that. */
    sn->next = next;
    c_node_addref_next_link(sn);
}

void c_node_remove_from_list(struct c_node *n) {
    struct c_node *next,*prev;

    if (n != NULL) {
        prev = n->prev;
        next = n->next;

        c_node_release_next_link(n);
        c_node_release_prev_link(n);
        assert(n->prev == NULL);
        assert(n->next == NULL);

        /* NTS: remember the addref_*_link function updates the partner's opposite pointer automatically.
         *      call one or the other only, not both. */
        if (prev != NULL) {
            assert(prev->next == NULL);
            prev->next = next;
            c_node_addref_next_link(prev);
        }
        else if (next != NULL) {
            assert(next->prev == NULL);
            next->prev = prev;
            c_node_addref_prev_link(next);
        }
    }
}

void c_node_dumptree(struct c_node *n,int indent);

void c_node_declaration_specifiers_group_combine(struct c_node **n) {
    struct c_node *scan,*s;

    scan = *n;
    while (scan != NULL && (s=(scan->next)) != NULL) {
        while (s && (scan->groupcode != s->groupcode || (scan->groupcode == s->groupcode && scan->token != s->token)))
            s = s->next;

        if (s != NULL) {
            c_node_remove_from_list(s);
            c_node_insert_after_in_list(scan,s);
        }

        scan = scan->next;
    }
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

struct identifier_t {
    int                 token;      // IDENTIFIER, ENUM_CONSTANT, TYPEDEF_NAME
    unsigned int        defined:1;
    unsigned int        deleted:1;
    struct c_node*      ident;      // identifier node
    struct c_node*      node;
};

#define MAX_IDENTIFIERS 16384

struct identifier_t     idents[MAX_IDENTIFIERS];
c_identref_t            idents_scope_boundary = 0; /* ref >= boundary means in scope */
c_identref_t            idents_count=0;

struct identifier_t *idents_get(c_identref_t idr) {
    if (idr < (c_identref_t)MAX_IDENTIFIERS)
        return &idents[idr];

    return NULL;
}

void idents_free_struct(struct identifier_t *id) {
    if (id->ident != NULL) {
        c_node_release_autodelete(&(id->ident));
        id->ident = NULL;
    }
    if (id->node != NULL) {
        c_node_release_autodelete(&(id->node));
        id->node = NULL;
    }
}

void idents_free(c_identref_t idr) {
    idents_free_struct(idents_get(idr));
}

void idents_free_all(void) {
    while (idents_count > 0) {
        idents_count--;
        idents_free(idents_count);
    }
}

void idents_mark_deleted(c_identref_t idr) {
    struct identifier_t *id = idents_get(idr);
    if (id != NULL) id->deleted = 1;
}

void idents_mark_defined(c_identref_t idr) {
    struct identifier_t *id = idents_get(idr);
    if (id != NULL) id->defined = 1;
}

/* NTS: This scans BACKWARDS in the allocation array for the first non-deleted identifier of the same name.
 *      This allows variables in an inner scope to shadow global scope while in scope. */
struct identifier_t *idents_name_lookup(const char *name,c_identref_t min_stop_at) {
    c_identref_t scan = idents_count;

    if ((scan--) != min_stop_at) {
        do {
            struct identifier_t *id = idents_get(scan);
            if (id != NULL) {
                if (!id->deleted && id->ident != NULL) {
                    struct c_node_IDENTIFIER *ident = &(id->ident->value.value_IDENTIFIER);

                    if (ident->name != NULL) {
                        if (!strcmp(name,ident->name))
                            return id;
                    }
                }
            }
        } while ((scan--) != min_stop_at);
    }

    return NULL;
}

struct identifier_t *idents_new(void) {
    struct identifier_t *id;

    if (idents_count >= (c_identref_t)MAX_IDENTIFIERS)
        return NULL;

    id = &idents[idents_count++];
    memset(id,0,sizeof(*id));
    return id;
}

void idents_set_name(c_identref_t idr,struct c_node *identnode) {
    struct identifier_t *id;

    id = idents_get(idr);
    if (id == NULL) return;

    if (id->ident != NULL) {
        c_node_release_autodelete(&(id->ident));
        id->ident = NULL;
    }

    id->ident = identnode;

    if (id->ident != NULL)
        c_node_addref(&(id->ident));
}

void idents_set_node(c_identref_t idr,struct c_node *node) {
    struct identifier_t *id;

    id = idents_get(idr);
    if (id == NULL) return;

    if (id->node != NULL) {
        c_node_release_autodelete(&(id->node));
        id->node = NULL;
    }

    id->node = node;

    if (id->node != NULL)
        c_node_addref(&(id->node));
}

c_identref_t idents_ptr_to_ref(struct identifier_t *id) {
    return (c_identref_t)(id - &idents[0]);
}

void c_node_autoregister_if_typedef(struct c_node *n) {
    unsigned char is_typedef = 0;
    struct c_node *sn,*idn;

    assert(n->token == DECLARATION);

    /* first child is declaration */
    /* second child is init declarator */
    if (n->child[0] == NULL || n->child[1] == NULL)
        return;

    for (sn=n->child[0];sn != NULL;sn=sn->next) {
        if (sn->token == TYPEDEF)
            is_typedef = 1;
    }

    if (!is_typedef)
        return;

    for (sn=n->child[1];sn != NULL;sn=sn->next) {
        if (sn->token == INIT_DECLARATOR) {
            if ((idn=sn->child[0]) != NULL) {
                /* find the identifier */
                while (idn->token != IDENTIFIER) {
                    if (idn->token == ARRAY_REF ||
                        idn->token == FUNCTION_REF ||
                        idn->token == DECLARATOR_EXPRESSION) {
                        idn = idn->child[0];
                    }
                    else if (idn->token == POINTER) {
                        idn = idn->child[1];
                    }
                    else {
                        break;
                    }
                }

                if (idn && idn->token == IDENTIFIER) {
                    struct identifier_t *id;

                    assert(idn->value.value_IDENTIFIER.name != NULL);

                    id = idents_new();
                    if (id == NULL) return; /* TODO: error */

                    id->defined = 1;
                    id->token = TYPEDEF_NAME;
                    idents_set_name(idents_ptr_to_ref(id),idn);
                }
                else {
                    fprintf(stderr,"Warning: unable to register typedef\n");
                }
            }
        }
    }
}

void c_node_register_enum_constant(struct c_node *n) {
    struct identifier_t *id;

    assert(n->value.value_IDENTIFIER.name != NULL);

    id = idents_new();
    if (id == NULL) return; /* TODO: error */

    id->defined = 1;
    id->token = ENUMERATION_CONSTANT;
    idents_set_name(idents_ptr_to_ref(id),n);
}

int sym_type(const char *name) {
    struct identifier_t *id = idents_name_lookup(name,0);

    if (id == NULL) return IDENTIFIER;
    return id->token;
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

struct string_t *strings_get(c_stringref_t x) {
    if (x >= (c_stringref_t)strings_count)
        return NULL;

    return &strings[x];
}

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
        return c_stringref_t_NONE;
    }

    do {
        if (*str == 'u' || *str == 'U') {
            /* FIXME */
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
        return c_stringref_t_NONE;
    }

    tmp_alloc = 128;
    tmp_len = 0;
    tmp = malloc(tmp_alloc);
    if (tmp == NULL) {
        fprintf(stderr,"Unable to alloc str\n");
        return c_stringref_t_NONE;
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
                return c_stringref_t_NONE;
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
    struct c_node *pn = NULL;
    unsigned int chidx;

    for (;n!=NULL;n=(pn=n)->next) {
        fprintf_indent_node(stderr,indent);
        fprintf(stderr,"node %p: token(%u)='%s' lineno=%u refcount=%u",
            (void*)n,n->token,token2string(n->token),n->lineno,n->refcount);

        if (n->groupcode != 0)
            fprintf(stderr," groupcode=%d",n->groupcode);

        if (n->token == INC_OP)
            fprintf(stderr," %s-increment",n->value.value_INC_OP_direction>0?"post":"pre");
        else if (n->token == DEC_OP)
            fprintf(stderr," %s-decrement",n->value.value_INC_OP_direction>0?"post":"pre");
        else if (n->token == ENUM)
            fprintf(stderr," extra-elem=%u",n->value.value_ENUM.extra_elem);
        else if (n->token == INITIALIZER_LIST)
            fprintf(stderr," extra-elem=%u",n->value.value_INITIALIZER_LIST.extra_elem);
        else if (n->token == TYPECAST_INITIALIZER_LIST)
            fprintf(stderr," extra-elem=%u",n->value.value_TYPECAST_INITIALIZER_LIST.extra_elem);

        fprintf(stderr,"\n");

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
        else if (n->token == ENUMERATION_CONSTANT) {
            fprintf_indent_node(stderr,indent+1);
            fprintf(stderr,"ENUMERATION_CONSTANT id=%ld name='%s' value=0x%llx(%llu)\n",
                (long)n->value.value_IDENTIFIER.id,
                n->value.value_IDENTIFIER.name,
                (unsigned long long)n->value.value_IDENTIFIER.enum_constant,
                (unsigned long long)n->value.value_IDENTIFIER.enum_constant);
        }
        else if (n->token == TYPEDEF_NAME) {
            fprintf_indent_node(stderr,indent+1);
            fprintf(stderr,"TYPEDEF_NAME id=%ld name='%s'\n",
                    (long)n->value.value_IDENTIFIER.id,
                    n->value.value_IDENTIFIER.name);
        }
        else if (n->token == STRING_LITERAL) {
            c_stringref_t ref = n->value.value_STRING_LITERAL;
            struct string_t *str = strings_get(ref);

            fprintf_indent_node(stderr,indent+1);
            fprintf(stderr,"STRING_LITERAL id=%ld ",
                (long)ref);
            if (str != NULL) {
                if (str->wchar || str->utf8) {
                }
                else {
                    fprintf(stderr,"str='%s' ",str->bytes?((char*)str->bytes):"(null)");
                }
            }
            fprintf(stderr,"\n");
        }

        if (n->next != NULL && n->next->prev != n) {
            fprintf_indent_node(stderr,indent+1);
            fprintf(stderr,"WARNING: node->next->prev != node\n");
        }
        if (n->prev != NULL && pn == NULL) {
            fprintf_indent_node(stderr,indent+1);
            fprintf(stderr,"WARNING: node->prev != NULL and first node of this level\n");
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

#define REGISTER_IDENTIFIER_ERROR_IF_EXISTS             (1U << 0U)
#define REGISTER_IDENTIFIER_MUST_EXIST                  (1U << 1U)

struct identifier_t *register_identifier(struct c_node *nid,unsigned int regflags) {
    struct identifier_t *id = NULL;

    assert(nid != NULL);

    if (nid->value.value_IDENTIFIER.id != c_identref_t_NONE)
        return idents_get(nid->value.value_IDENTIFIER.id);

    if (nid->value.value_IDENTIFIER.name == NULL) {
        fprintf(stderr,"register_identifier(): no name\n");
        return NULL;
    }

    id = idents_name_lookup(nid->value.value_IDENTIFIER.name,0);
    if (id != NULL) {
        if (regflags & REGISTER_IDENTIFIER_ERROR_IF_EXISTS) {
            fprintf(stderr,"register_identifier(): '%s' already defined\n",nid->value.value_IDENTIFIER.name);
            return NULL;
        }

        nid->value.value_IDENTIFIER.id = idents_ptr_to_ref(id);
        return id;
    }

    if (regflags & REGISTER_IDENTIFIER_MUST_EXIST) {
        fprintf(stderr,"register_identifier(): '%s' does not exist\n",nid->value.value_IDENTIFIER.name);
        return NULL;
    }

    id = idents_new();
    if (id == NULL) {
        fprintf(stderr,"register_identifier(): cannot alloc identifier\n");
        return NULL;
    }

    nid->value.value_IDENTIFIER.id = idents_ptr_to_ref(id);
    assert(id->ident == NULL);
    id->ident = nid;
    c_node_addref(&(id->ident));
    idents_set_name(nid->value.value_IDENTIFIER.id,nid);
    return id;
}

int enum_const_eval(struct c_node *idn) {
    struct identifier_t *eid;
    struct c_node *en;

    assert(idn->token == ENUMERATION_CONSTANT);

    if ((eid=register_identifier(idn,REGISTER_IDENTIFIER_MUST_EXIST)) == NULL)
        return -1;
    if ((en=eid->node) == NULL) {
        fprintf(stderr,"enum identifier no node\n");
        return -1;
    }
    assert(en->token == ENUMERATION_CONSTANT);
    c_string_free(&(idn->value.value_IDENTIFIER.name));
    idn->value.value_I_CONSTANT.v.uint = en->value.value_IDENTIFIER.enum_constant;
    idn->value.value_I_CONSTANT.bsign = 0;
    idn->value.value_I_CONSTANT.bwidth = int_width_b;
    idn->token = I_CONSTANT;
    return 0;
}

int expression_eval(struct c_node *idn) {
    struct c_node *nullnode = NULL;
    struct c_node *sn;
    int r;

    assert(idn->token == EXPRESSION);

    /* we do not concern ourself with prev/next/parent */
    /* we do expect the node to only have one child */
    while (idn->token == EXPRESSION) {
        if ((sn=idn->child[0]) == NULL)
            break;

        if (sn->token == I_CONSTANT) {
            /* pull it up, replace EXPRESSION node */
            idn->token = sn->token;
            idn->value = sn->value;
            /* dispose of child contents */
            memset(&(sn->value),0,sizeof(sn->value));
            c_node_move_to_child_link(idn,0,&nullnode);
            c_node_release_autodelete(&(sn));
        }
        else if (sn->token == ENUMERATION_CONSTANT) {
            if ((r=enum_const_eval(sn)) != 0)
                return r;

            /* pull it up, replace EXPRESSION node */
            idn->token = sn->token;
            idn->value = sn->value;
            /* dispose of child contents */
            memset(&(sn->value),0,sizeof(sn->value));
            c_node_move_to_child_link(idn,0,&nullnode);
            c_node_release_autodelete(&(sn));
        }
        else {
            break;
        }
    }

    return 0;
}

int enum_expr_eval(struct c_node **idn) {
    assert(idn != NULL);

    if ((*idn)->token == ENUMERATION_CONSTANT)
        return enum_const_eval(*idn);
    else if ((*idn)->token == EXPRESSION)
        return expression_eval(*idn);

    return 0;
}

/* ENUM
 *   child[0] = identifier (if present, NULL if not)
 *   child[1] = first node of ENUMERATION_CONSTANT linked list (follow ->next list) */
int register_enum(struct c_node *node) {
    struct c_node *identifier,*enumconsts;
    unsigned long long enum_it_val = 0;
    struct identifier_t *id;
    struct c_node *idn;

    assert(node != NULL);

    identifier = node->child[0];
    enumconsts = node->child[1];

    if (identifier != NULL) {
        if ((id=register_identifier(identifier,0)) == NULL)
            return -1;

        /* make sure it's an enum */
        if (id->token != 0 && id->token != ENUM) {
            fprintf(stderr,"identifier already exists but NOT as enum\n");
            return -1;
        }
        id->token = ENUM;

        /* we treat it like a declaration if there are no enum constants provided */
        if (enumconsts != NULL) {
            if (id->defined) {
                fprintf(stderr,"enum already defined.\n");
                return -1;
            }

            id->defined = 1;
        }
    }

    /* and then register enumeration identifiers */
    for (;enumconsts != NULL;enumconsts=enumconsts->next) {
        if (enumconsts->token == ENUMERATION_CONSTANT) {
            if ((id=register_identifier(enumconsts,0)) == NULL)
                return -1;

            /* make sure it's an enum */
            if (id->token != 0 && id->token != ENUMERATION_CONSTANT) {
                fprintf(stderr,"identifier already exists but NOT as enum const\n");
                return -1;
            }

            if (id->defined) {
                fprintf(stderr,"enum const already defined\n");
                return -1;
            }

            id->token = ENUMERATION_CONSTANT;
            id->defined = 1;

            /* if the ENUMERATION_CONSTANT has an I_CONST child node,
             * that integer constant becomes the new value instead */
            if ((idn=enumconsts->child[0]) != NULL) {
                if (idn->token != I_CONSTANT) {
                    if (enum_expr_eval(&idn)) { /* will eval expressions upward then change token to I_CONSTANT */
                        fprintf(stderr,"enum expression evaluation failure\n");
                        return -1;
                    }
                }

                if (idn->token == I_CONSTANT)
                    enum_it_val = idn->value.value_I_CONSTANT.v.uint;
                else {
                    fprintf(stderr,"enum const not an integer constant\n");
                    return -1;
                }
            }

            /* register node and value */
            idents_set_node(idents_ptr_to_ref(id),enumconsts);
            enumconsts->value.value_IDENTIFIER.enum_constant = enum_it_val;

            /* increment const */
            enum_it_val++;
        }
    }

    return 0;
}

int enumerator_pass(struct c_node *node) {
    struct c_node *sc;
    unsigned int i;
    int r;

    for (sc=node;sc!=NULL;sc=sc->next) {
        if (sc->token == ENUM) {
            if ((r=register_enum(sc)) != 0)
                return r;
        }
        else {
            for (i=0;i < c_node_MAX_CHILDREN;i++) {
                if ((r=enumerator_pass(sc->child[i])) != 0)
                    return r;
            }
        }
    }

    return 0;
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
    idents_free_all();

    /* first pass: enumerator marking */
    if (res == 0 && last_translation_unit != NULL)
        res = enumerator_pass(last_translation_unit);

    /* finish parsing final tree */
    if (last_translation_unit != NULL) {
        fprintf(stderr,"Resulting tree:\n");
        c_node_dumptree(last_translation_unit,1);
    }

    /* delete the tree, we're done */
    if (last_translation_unit != NULL) {
        c_node_delete_tree(&last_translation_unit);
        c_node_release(&last_translation_unit);
        c_node_delete(&last_translation_unit);
    }

    idents_free_all();
    strings_free_all();
    return (res != 0) ? 1 : 0;
}

