
#include <stdint.h>

#include "cstring.h"

#define c_identref_t_NONE       ((c_identref_t)(~0ULL))

struct c_node;
struct c_block_item_node;
struct c_node_initializer;

struct c_init_decl_node {
    struct c_node_initializer*          initializer;
    c_identref_t                        identifier;
    struct c_init_decl_node*            next;
};

struct c_node {
    int /*enum yytokentype*/ token;
    union c_node_val {
        // for use with "int/long/long long" datatype
        // token == I_CONSTANT
        struct c_node_val_int {
            union {
                uint64_t    uint;
                int64_t     sint;
            } v;
            unsigned char   bwidth;     // width, in bytes
            unsigned char   bsign;      // 1=signed 0=unsigned
        } val_uint;
        // for use with "float/double" datatype
        // token == F_CONSTANT
        struct c_node_val_float {
            unsigned char   bwidth;     // width, in bytes
            double          val;        // FIXME: This won't allow "long double"
        } val_float;
        // string storage. we don't put the pointer directly here, that's not sane.
        // instead we store a reference to string storage. that way we also keep track
        // whether ANSI or WIDE and we don't lose track of pointers.
        // token == STRING_LITERAL
        c_stringref_t       val_string;
        // identifier storage, by name. again, by reference only.
        // token == TYPEDEF_NAME
        // token == ENUMERATION_CONSTANT
        // token == IDENTIFIER
        c_identref_t        val_identifier;
        // type spec
        // token == TYPE_SPECIFIER
        struct c_node_type_spec {
            int             main_type;
            signed char     bsign;      // -1=unspec  0=unsigned  1=signed
        } val_type_spec;
        // type qualifier
        // token == TYPE_QUALIFIER
        struct c_node_type_qual {
            unsigned int    is_const:1;
            unsigned int    is_restrict:1;
            unsigned int    is_volatile:1;
            unsigned int    is_atomic:1;
        } val_type_qual;
        // storage class specifier
        // token == TYPE_QUALIFIER
        struct c_node_storage_class {
            unsigned int    is_typedef:1;
            unsigned int    is_extern:1;
            unsigned int    is_static:1;
            unsigned int    is_thread_local:1;
            unsigned int    is_auto:1;
            unsigned int    is_register:1;
        } val_storage_class;
        // function specifier
        // token == FUNC_SPECIFIER
        struct c_node_func_spec {
            unsigned int    is_inline:1;
            unsigned int    is_noreturn:1;
        } val_func_spec;
        // declaration spec
        // token == DECL_SPECIFIER
        struct c_node_decl_spec {
            struct c_node_type_spec     typespec;
            struct c_node_type_qual     typequal;
            struct c_node_func_spec     funcspec;
            struct c_node_storage_class storageclass;
            struct c_init_decl_node*    init_decl_list;
        } val_decl_spec;
        // init decl
        // token == INIT_DECL_LIST
        struct c_init_decl_node*        init_decl_list;
        // initializer
        // token == INITIALIZER
        struct c_node_initializer*      initializer;
        // typecast
        // token == TYPECAST
        struct c_node_typecast_node {
            struct c_node_decl_spec     decl_spec;
            struct c_node*              typecast_node;
        } val_typecast_node;
        // block item
        // token == BLOCK_ITEM
        struct c_block_item_node*       block_item_list;
        // compount statement root (directed at block item)
        // token == COMPOUND_STATEMENT
        struct c_node*                  compound_statement_root;
        // for anything else, this value is meaningless
    } value;
};

struct c_node_initializer {
    struct c_node                       node;
    struct c_node_initializer*          next;
};

struct c_block_item_node {
    struct c_node                       node;
    struct c_block_item_node*           next;
};

