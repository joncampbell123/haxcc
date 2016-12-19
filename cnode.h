
#include <stdint.h>

#include "cstring.h"

struct c_node {
    int /*enum yytokentype*/ token;
    union c_node_val {
        // for use with "int/long/long long" datatype
        // token == I_CONSTANT
        uint64_t        val_uint;
        // for use with "float/double" datatype
        // token == F_CONSTANT
        double          val_double;
        // string storage. we don't put the pointer directly here, that's not sane.
        // instead we store a reference to string storage. that way we also keep track
        // whether ANSI or WIDE and we don't lose track of pointers.
        // token == STRING_LITERAL
        c_stringref_t   val_string;
        // for anything else, this value is meaningless
    } value;
};

