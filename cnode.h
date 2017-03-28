
#include <stdint.h>

#include "cstring.h"

#define c_node_MAX_CHILDREN             4

/* NTS: In my previous attempt, memory allocation and linked list management
 *      became way too complex. I had one linked list struct for every type
 *      of token. In this revision, we just normalize the linked list
 *      structure in a generic fashion and have a struct in union for
 *      tokens that need it. Up to MAX_CHILDREN children are allowed so that
 *      we can link together expressions. Hopefull this code won't suck as
 *      much as my last attempt. */
struct c_node {
    int /*enum yytokentype*/            token;          /* lexer token */
    int                                 lineno;         /* line number seen */
    unsigned int                        refcount;
    int                                 groupcode;      /* used for grouping during parsing */
    struct c_node*                      prev;
    struct c_node*                      next;
    struct c_node*                      parent;
    struct c_node*                      child[c_node_MAX_CHILDREN];
    union c_node_val {
        struct c_node_I_CONSTANT {
            union {
                uint64_t    uint;
                int64_t     sint;
            } v;
            unsigned char   bwidth;     // width, in bytes
            unsigned char   bsign;      // 1=signed 0=unsigned
        } value_I_CONSTANT;
        struct c_node_F_CONSTANT {
            long double     val;        // FIXME: On some compilers long double == double
            unsigned char   bwidth;     // width, in bytes
        } value_F_CONSTANT;
        struct c_node_IDENTIFIER {      // token == IDENTIFIER or token == ENUMERATION_CONSTANT or TYPEDEF_NAME
            c_identref_t                id;
            char*                       name;
            uint64_t                    enum_constant; // if token == ENUMERATION_CONSTANT
        } value_IDENTIFIER;
        struct c_node_ENUM {
            unsigned char               extra_elem; // at the end
        } value_ENUM;
        struct c_node_INITIALIZER_LIST {
            unsigned char               extra_elem; // at the end
        } value_INITIALIZER_LIST;
        struct c_node_TYPECAST_INITIALIZER_LIST {
            unsigned char               extra_elem; // at the end
        } value_TYPECAST_INITIALIZER_LIST;
        struct c_node_DECLARATION {
            unsigned int                is_restrict:1;
            unsigned int                is_atomic:1;
            unsigned int                is_static:1;
            unsigned int                is_extern:1;
            unsigned int                is_const:1;
            int                         main_type_token;    // AUTO, VOID, BOOL, INT, FLOAT, TYPEDEF_NAME, ENUM, or 0 if not specified
            union {
                c_identref_t            ident;              // TYPEDEF_NAME, ENUM, the identifier in question, or c_identref_t_NONE
                struct c_node_I_CONSTANT ival;              // INT
                struct c_node_F_CONSTANT fval;              // FLOAT
            } v;
        } value_DECLARATION;
        c_stringref_t       value_STRING_LITERAL;
        char                value_INC_OP_direction;     /* -1 = pre-increment  1 = post-increment */
        char                value_DEC_OP_direction;     /* -1 = pre-decrement  1 = post-decrement */
    } value;
};

extern int yylineno;

extern struct c_node*                   last_translation_unit;

struct c_node *c_node_alloc(void);
struct c_node *c_node_alloc_or_die(void);
void c_node_delete(struct c_node **n); /* <- will throw a warning if refcount != 0 */
unsigned int c_node_addref(struct c_node **n);
unsigned int c_node_release(struct c_node **n);
unsigned int c_node_release_autodelete(struct c_node **n);
void c_node_move_to(struct c_node **d,struct c_node **s); /* <- *d = *s, *s = NULL */
void c_node_move_to_prev_link(struct c_node *node,struct c_node **next);
void c_node_move_to_next_link(struct c_node *node,struct c_node **next);
void c_node_move_to_child_link(struct c_node *node,unsigned int chidx,struct c_node **next);
void c_node_move_to_parent_link(struct c_node *node,struct c_node **next);
void c_node_copy_lineno(struct c_node *d,struct c_node *s);
void c_node_scan_to_parent_head(struct c_node **n);
void c_node_scan_to_head(struct c_node **n);
void c_node_scan_to_end(struct c_node **n);

void c_node_register_enum_constant(struct c_node *n);

void c_node_declaration_specifiers_group_combine(struct c_node **n);
void c_node_autoregister_if_typedef(struct c_node *n);

void c_node_i_constant_parse(struct c_node *d,char *s,int base);
void c_node_i_constant_char_parse(struct c_node *d,char *s);
void c_node_identifier_parse(struct c_node *d,char *s);
void c_node_f_constant_parse(struct c_node *d,char *s);

int c_node_init_declaration(struct c_node **n);

void yyerror(const char *s);

