
#include <stdint.h>

#include "cstring.h"

struct c_node {
    int /*enum yytokentype*/ token;
    union c_node_val {
        // for use with "int/long/long long" datatype
        // token == I_CONSTANT
        struct c_node_val_int {
            uint64_t        uint;
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
        c_stringref_t   val_string;
        // identifier storage, by name. again, by reference only.
        // token == TYPEDEF_NAME
        // token == ENUMERATION_CONSTANT
        // token == IDENTIFIER
        c_identref_t    val_identifier;
        // type spec
        // token == TYPE_SPECIFIER
        struct c_node_type_spec {
            int             main_type;
            signed char     bsign;      // -1=unspec  0=unsigned  1=signed
        } val_type_spec;
        // declaration spec
        // token == DECL_SPECIFIER
        struct c_node_decl_spec {
            struct c_node_type_spec     typespec;
        } val_decl_spec;
        // for anything else, this value is meaningless
    } value;
};

