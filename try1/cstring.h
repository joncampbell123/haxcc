
#ifndef HAX_CSTRING_H
#define HAX_CSTRING_H

typedef size_t      c_stringref_t;
typedef size_t      c_identref_t;

#define c_stringref_t_NONE      ((size_t)(~0ULL))
#define c_identref_t_NONE       ((size_t)(~0ULL))

c_stringref_t sconst_parse(char *str);

#endif //HAX_CSTRING_H

