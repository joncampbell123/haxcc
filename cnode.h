
#include <stdint.h>

#include "cstring.h"

#define c_node_MAX_CHILDREN             3

/* NTS: In my previous attempt, memory allocation and linked list management
 *      became way too complex. I had one linked list struct for every type
 *      of token. In this revision, we just normalize the linked list
 *      structure in a generic fashion and have a struct in union for
 *      tokens that need it. Up to MAX_CHILDREN children are allowed so that
 *      we can link together expressions. Hopefull this code won't suck as
 *      much as my last attempt. */
struct c_node {
    int /*enum yytokentype*/            token;          /* lexer token */
    unsigned int                        refcount;
    struct c_node*                      prev;
    struct c_node*                      next;
    struct c_node*                      child[c_node_MAX_CHILDREN];
};

struct c_node *c_node_alloc(void);
void c_node_delete(struct c_node **n); /* <- will throw a warning if refcount != 0 */
unsigned int c_node_addref(struct c_node **n);
unsigned int c_node_release(struct c_node **n);
void c_node_move_to(struct c_node **d,struct c_node **s); /* <- *d = *s, *s = NULL */

void yyerror(const char *s);

