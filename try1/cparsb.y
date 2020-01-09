
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cnode.h"
#include "cstring.h"

// stuff from flex that bison needs to know about:
int yylex();
int yyparse();
FILE *yyin;

void yyerror(const char *s);
%}

%union {
    struct c_node*              node;
}


%token  IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL FUNC_NAME SIZEOF
%token  PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token  AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token  SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token  XOR_ASSIGN OR_ASSIGN
%token  TYPEDEF_NAME ENUMERATION_CONSTANT

%token  TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token  CONST RESTRICT VOLATILE
%token  BOOL CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token  COMPLEX IMAGINARY 
%token  STRUCT UNION ENUM ELLIPSIS

%token  CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token  ALIGNAS ALIGNOF ATOMIC GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL

%token  DESIGNATOR_ARRAY_INDEX
%token  DESIGNATOR_STRUCT_MEMBER

%token  NEGATE
%token  POSITIVE
%token  ADDRESS_OF
%token  EXPRESSION
%token  DECLARATION
%token  INIT_DECLARATOR
%token  INITIALIZER_LIST
%token  TYPECAST_INITIALIZER_LIST
%token  EXTERNAL_DECLARATION
%token  STORAGE_CLASS_SPECIFIER
%token  DECLARATOR_EXPRESSION
%token  FUNCTION_DEFINITION
%token  TYPE_SPECIFIER
%token  TYPE_QUALIFIER
%token  FUNCTION_SPECIFIER
%token  COMPOUND_STATEMENT
%token  ALIGNMENT_SPECIFIER
%token  STRUCT_DECLARATOR
%token  IDENTIFIER_LIST
%token  PARAMETER_LIST
%token  ARGUMENT_LIST
%token  POINTER_DEREF
%token  FUNCTION_REF
%token  DESIGNATION
%token  BLOCK_ITEM
%token  ARRAY_REF
%token  ANONYMOUS
%token  TYPECAST
%token  DO_WHILE
%token  POINTER
%token  TERNARY
%token  LABEL

%token-table

%start translation_unit
%%

primary_expression
    : IDENTIFIER {
        $<node>$ = $<node>1;
    }
    | constant {
        $<node>$ = $<node>1;
    }
    | string {
        $<node>$ = $<node>1;
    }
    | '(' expression ')' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = EXPRESSION; c_node_copy_lineno($<node>$,$<node>2);
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>1));
        c_node_release_autodelete(&($<node>3));
    }
    | generic_selection {
        $<node>$ = $<node>1;
    }
    ;

constant
    : I_CONSTANT        /* includes character_constant */
    | F_CONSTANT
    | ENUMERATION_CONSTANT  /* after it has been defined as such */
    ;

enumeration_constant        /* before it has been defined as such */
    : IDENTIFIER
    ;

string
    : STRING_LITERAL
    | FUNC_NAME
    ;

generic_selection
    : GENERIC '(' assignment_expression ',' generic_assoc_list ')'
    ;

generic_assoc_list
    : generic_association
    | generic_assoc_list ',' generic_association
    ;

generic_association
    : type_name ':' assignment_expression
    | DEFAULT ':' assignment_expression
    ;

postfix_expression
    : primary_expression
    | postfix_expression '[' expression ']' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = ARRAY_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>4));
    }
    | postfix_expression '(' ')' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = FUNCTION_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_release_autodelete(&($<node>3));
    }
    | postfix_expression '(' argument_expression_list ')' {
        struct c_node *idlist;

        idlist = c_node_alloc_or_die(); c_node_addref(&idlist); idlist->token = ARGUMENT_LIST; c_node_copy_lineno($<node>$,$<node>3);
        c_node_scan_to_head(&($<node>3));
        c_node_move_to_child_link(idlist,0,&($<node>3));

        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = FUNCTION_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,1,&idlist);
        c_node_release_autodelete(&($<node>4));
    }
    | postfix_expression '.' IDENTIFIER {
        $<node>$ = $<node>2;
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
    }
    | postfix_expression PTR_OP IDENTIFIER {
        $<node>$ = $<node>2;
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
    }
    | postfix_expression INC_OP {
        $<node>$ = $<node>2;
        $<node>$->value.value_INC_OP_direction = 1; /* post-increment */
        c_node_move_to_child_link($<node>$,0,&($<node>1));
    }
    | postfix_expression DEC_OP {
        $<node>$ = $<node>2;
        $<node>$->value.value_DEC_OP_direction = 1; /* post-decrement */
        c_node_move_to_child_link($<node>$,0,&($<node>1));
    }
    | '(' type_name ')' '{' initializer_list '}' {
        c_node_scan_to_head(&($<node>5));
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = TYPECAST_INITIALIZER_LIST; c_node_copy_lineno($<node>$,$<node>1);
        $<node>$->value.value_TYPECAST_INITIALIZER_LIST.extra_elem = 0;
        c_node_release_autodelete(&($<node>1));
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
        c_node_release_autodelete(&($<node>4));
        c_node_move_to_child_link($<node>$,1,&($<node>5));
        c_node_release_autodelete(&($<node>6));
    }
    | '(' type_name ')' '{' initializer_list ',' '}' {
        c_node_scan_to_head(&($<node>5));
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = TYPECAST_INITIALIZER_LIST; c_node_copy_lineno($<node>$,$<node>1);
        $<node>$->value.value_TYPECAST_INITIALIZER_LIST.extra_elem = 1;
        c_node_release_autodelete(&($<node>1));
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
        c_node_release_autodelete(&($<node>4));
        c_node_move_to_child_link($<node>$,1,&($<node>5));
        c_node_release_autodelete(&($<node>6));
        c_node_release_autodelete(&($<node>7));
    }
    ;

argument_expression_list
    : assignment_expression
    | argument_expression_list ',' assignment_expression {
        $<node>$ = $<node>3;
        c_node_move_to_prev_link($<node>$,&($<node>1));
        c_node_release_autodelete(&($<node>2));
    }
    ;

unary_expression
    : postfix_expression {
        c_node_scan_to_parent_head(&($<node>1));
        $<node>$ = $<node>1;
    }
    | INC_OP unary_expression {
        $<node>$ = $<node>1;
        $<node>$->value.value_INC_OP_direction = -1; /* pre-increment */
        c_node_move_to_child_link($<node>$,0,&($<node>2));
    }
    | DEC_OP unary_expression {
        $<node>$ = $<node>1;
        $<node>$->value.value_DEC_OP_direction = -1; /* pre-increment */
        c_node_move_to_child_link($<node>$,0,&($<node>2));
    }
    | unary_operator cast_expression {
        $<node>$ = $<node>1;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
    }
    | SIZEOF unary_expression {
        $<node>$ = $<node>1;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
    }
    | SIZEOF '(' type_name ')' {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_release_autodelete(&($<node>4));
    }
    | ALIGNOF '(' type_name ')' {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_release_autodelete(&($<node>4));
    }
    ;

unary_operator
    : '&' {
        $<node>$ = $<node>1;
        $<node>$->token = ADDRESS_OF;
    }
    | '*' {
        $<node>$ = $<node>1;
        $<node>$->token = POINTER_DEREF;
    }
    | '+' {
        $<node>$ = $<node>1;
        $<node>$->token = POSITIVE;
    }
    | '-' {
        $<node>$ = $<node>1;
        $<node>$->token = NEGATE;
    }
    | '~'
    | '!'
    ;

cast_expression
    : unary_expression
    | '(' type_name ')' cast_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = TYPECAST; c_node_copy_lineno($<node>$,$<node>2);
        c_node_release_autodelete(&($<node>1));
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
        c_node_move_to_child_link($<node>$,1,&($<node>4));
    }
    ;

multiplicative_expression
    : cast_expression {
        $<node>$ = $<node>1;
    }
    | multiplicative_expression '*' cast_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    | multiplicative_expression '/' cast_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    | multiplicative_expression '%' cast_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    ;

additive_expression
    : multiplicative_expression {
        $<node>$ = $<node>1;
    }
    | additive_expression '+' multiplicative_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    | additive_expression '-' multiplicative_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    ;

shift_expression
    : additive_expression
    | shift_expression LEFT_OP additive_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    | shift_expression RIGHT_OP additive_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    ;

relational_expression
    : shift_expression
    | relational_expression '<' shift_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    | relational_expression '>' shift_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    | relational_expression LE_OP shift_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    | relational_expression GE_OP shift_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    ;

equality_expression
    : relational_expression
    | equality_expression EQ_OP relational_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    | equality_expression NE_OP relational_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    ;

and_expression
    : equality_expression
    | and_expression '&' equality_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    ;

exclusive_or_expression
    : and_expression
    | exclusive_or_expression '^' and_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    ;

inclusive_or_expression
    : exclusive_or_expression
    | inclusive_or_expression '|' exclusive_or_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    ;

logical_and_expression
    : inclusive_or_expression
    | logical_and_expression AND_OP inclusive_or_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    ;

logical_or_expression
    : logical_and_expression
    | logical_or_expression OR_OP logical_and_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    ;

conditional_expression
    : logical_or_expression
    | logical_or_expression '?' expression ':' conditional_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = TERNARY; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_move_to_child_link($<node>$,2,&($<node>5));
        c_node_release_autodelete(&($<node>2));
        c_node_release_autodelete(&($<node>4));
    }
    ;

assignment_expression
    : conditional_expression
    | unary_expression assignment_operator assignment_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>2);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    ;

assignment_operator
    : '='
    | MUL_ASSIGN
    | DIV_ASSIGN
    | MOD_ASSIGN
    | ADD_ASSIGN
    | SUB_ASSIGN
    | LEFT_ASSIGN
    | RIGHT_ASSIGN
    | AND_ASSIGN
    | XOR_ASSIGN
    | OR_ASSIGN
    ;

expression
    : assignment_expression
    | expression ',' assignment_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = $<node>2->token; c_node_copy_lineno($<node>$,$<node>3);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    ;

constant_expression
    : conditional_expression    /* with constraints */
    ;

declaration
    : declaration_specifiers ';' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = DECLARATION; c_node_copy_lineno($<node>$,$<node>1);
        c_node_declaration_specifiers_group_combine(&($<node>1));
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_autoregister_if_typedef($<node>$);
        if (c_node_init_declaration(&($<node>$)) != 0) YYABORT;
    }
    | declaration_specifiers init_declarator_list ';' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = DECLARATION; c_node_copy_lineno($<node>$,$<node>1);
        c_node_scan_to_head(&($<node>2));
        c_node_declaration_specifiers_group_combine(&($<node>1));
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>2));
        c_node_release_autodelete(&($<node>3));
        c_node_autoregister_if_typedef($<node>$);
        if (c_node_init_declaration(&($<node>$)) != 0) YYABORT;
    }
    | static_assert_declaration {
        $<node>$ = $<node>1;
    }
    ;

declaration_specifiers
    : storage_class_specifier declaration_specifiers {
        $<node>$ = $<node>1;
        $<node>$->groupcode = STORAGE_CLASS_SPECIFIER;
        c_node_move_to_next_link($<node>$,&($<node>2));
    }
    | storage_class_specifier {
        $<node>$ = $<node>1;
        $<node>$->groupcode = STORAGE_CLASS_SPECIFIER;
    }
    | type_specifier declaration_specifiers {
        $<node>$ = $<node>1;
        $<node>$->groupcode = TYPE_SPECIFIER;
        c_node_move_to_next_link($<node>$,&($<node>2));
    }
    | type_specifier {
        $<node>$ = $<node>1;
        $<node>$->groupcode = TYPE_SPECIFIER;
    }
    | type_qualifier declaration_specifiers {
        $<node>$ = $<node>1;
        $<node>$->groupcode = TYPE_QUALIFIER;
        c_node_move_to_next_link($<node>$,&($<node>2));
    }
    | type_qualifier {
        $<node>$ = $<node>1;
        $<node>$->groupcode = TYPE_QUALIFIER;
    }
    | function_specifier declaration_specifiers {
        $<node>$ = $<node>1;
        $<node>$->groupcode = FUNCTION_SPECIFIER;
        c_node_move_to_next_link($<node>$,&($<node>2));
    }
    | function_specifier {
        $<node>$ = $<node>1;
        $<node>$->groupcode = FUNCTION_SPECIFIER;
    }
    | alignment_specifier declaration_specifiers {
        $<node>$ = $<node>1;
        $<node>$->groupcode = ALIGNMENT_SPECIFIER;
        c_node_move_to_next_link($<node>$,&($<node>2));
    }
    | alignment_specifier {
        $<node>$ = $<node>1;
        $<node>$->groupcode = ALIGNMENT_SPECIFIER;
    }
    ;

init_declarator_list
    : init_declarator {
        $<node>$ = $<node>1;
    }
    | init_declarator_list ',' init_declarator {
        $<node>$ = $<node>3;
        c_node_move_to_prev_link($<node>$,&($<node>1));
        c_node_release_autodelete(&($<node>2));
    }
    ;

init_declarator
    : declarator '=' initializer {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = INIT_DECLARATOR; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    | declarator {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = INIT_DECLARATOR; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
    }
    ;

storage_class_specifier
    : TYPEDEF   /* identifiers must be flagged as TYPEDEF_NAME */
    | EXTERN
    | STATIC
    | THREAD_LOCAL
    | AUTO
    | REGISTER
    ;

type_specifier
    : VOID
    | CHAR
    | SHORT
    | INT
    | LONG
    | FLOAT
    | DOUBLE
    | SIGNED
    | UNSIGNED
    | BOOL
    | COMPLEX
    | IMAGINARY     /* non-mandated extension */
    | atomic_type_specifier
    | struct_or_union_specifier
    | enum_specifier
    | TYPEDEF_NAME      /* after it has been defined as such */
    ;

struct_or_union_specifier
    : struct_or_union '{' struct_declaration_list '}' {
        c_node_scan_to_head(&($<node>3));
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>4));
    }
    | struct_or_union IDENTIFIER '{' struct_declaration_list '}' {
        c_node_scan_to_head(&($<node>4));
        $<node>$ = $<node>1;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
        c_node_move_to_child_link($<node>$,1,&($<node>4));
        c_node_release_autodelete(&($<node>5));
    }
    | struct_or_union IDENTIFIER {
        $<node>$ = $<node>1;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
    }
    ;

struct_or_union
    : STRUCT
    | UNION
    ;

struct_declaration_list
    : struct_declaration {
        $<node>$ = $<node>1;
    }
    | struct_declaration_list struct_declaration {
        $<node>$ = $<node>2;
        c_node_move_to_prev_link($<node>$,&($<node>1));
    }
    ;

struct_declaration
    : specifier_qualifier_list ';'  /* for anonymous struct/union */ {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
    }
    | specifier_qualifier_list struct_declarator_list ';' {
        c_node_scan_to_head(&($<node>2));
        $<node>$ = $<node>1;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
    }
    | static_assert_declaration {
        $<node>$ = $<node>1;
    }
    ;

specifier_qualifier_list
    : type_specifier specifier_qualifier_list {
        $<node>$ = $<node>1;
        $<node>$->groupcode = TYPE_SPECIFIER;
        c_node_move_to_next_link($<node>$,&($<node>2));
    }
    | type_specifier {
        $<node>$ = $<node>1;
        $<node>$->groupcode = TYPE_SPECIFIER;
    }
    | type_qualifier specifier_qualifier_list {
        $<node>$ = $<node>1;
        $<node>$->groupcode = TYPE_QUALIFIER;
        c_node_move_to_next_link($<node>$,&($<node>2));
    }
    | type_qualifier {
        $<node>$ = $<node>1;
        $<node>$->groupcode = TYPE_QUALIFIER;
    }
    ;

struct_declarator_list
    : struct_declarator {
        $<node>$ = $<node>1;
    }
    | struct_declarator_list ',' struct_declarator {
        $<node>$ = $<node>3;
        c_node_move_to_prev_link($<node>$,&($<node>1));
        c_node_release_autodelete(&($<node>2));
    }
    ;

struct_declarator
    : ':' constant_expression {
        $<node>1->token = ANONYMOUS;
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = STRUCT_DECLARATOR; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>2));
    }
    | declarator ':' constant_expression {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = STRUCT_DECLARATOR; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
    }
    | declarator {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = STRUCT_DECLARATOR; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
    }
    ;

enum_specifier
    : ENUM '{' enumerator_list '}' {
        c_node_scan_to_head(&($<node>3));
        $<node>$ = $<node>1;
        $<node>$->value.value_ENUM.extra_elem = 0;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>4));
    }
    | ENUM '{' enumerator_list ',' '}' {
        c_node_scan_to_head(&($<node>3));
        $<node>$ = $<node>1;
        $<node>$->value.value_ENUM.extra_elem = 1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>4));
        c_node_release_autodelete(&($<node>5));
    }
    | ENUM IDENTIFIER '{' enumerator_list '}' {
        c_node_scan_to_head(&($<node>4));
        $<node>$ = $<node>1;
        $<node>$->value.value_ENUM.extra_elem = 0;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
        c_node_move_to_child_link($<node>$,1,&($<node>4));
        c_node_release_autodelete(&($<node>5));
    }
    | ENUM IDENTIFIER '{' enumerator_list ',' '}' {
        c_node_scan_to_head(&($<node>4));
        $<node>$ = $<node>1;
        $<node>$->value.value_ENUM.extra_elem = 1;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
        c_node_move_to_child_link($<node>$,1,&($<node>4));
        c_node_release_autodelete(&($<node>5));
        c_node_release_autodelete(&($<node>6));
    }
    | ENUM IDENTIFIER {
        $<node>$ = $<node>1;
        $<node>$->value.value_ENUM.extra_elem = 0;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
    }
    ;

enumerator_list
    : enumerator
    | enumerator_list ',' enumerator {
        $<node>$ = $<node>3;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_prev_link($<node>$,&($<node>1));
    }
    ;

enumerator  /* identifiers must be flagged as ENUMERATION_CONSTANT */
    : enumeration_constant '=' constant_expression {
        $<node>$ = $<node>1;
        $<node>$->token = ENUMERATION_CONSTANT;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_register_enum_constant($<node>$);
    }
    | enumeration_constant {
        $<node>$ = $<node>1;
        $<node>$->token = ENUMERATION_CONSTANT;
        c_node_register_enum_constant($<node>$);
    }
    ;

atomic_type_specifier
    : ATOMIC '(' type_name ')' {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_release_autodelete(&($<node>4));
    }
    ;

type_qualifier
    : CONST
    | RESTRICT
    | VOLATILE
    | ATOMIC
    ;

function_specifier
    : INLINE
    | NORETURN
    ;

alignment_specifier
    : ALIGNAS '(' type_name ')' {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_release_autodelete(&($<node>4));
    }
    | ALIGNAS '(' constant_expression ')' {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_release_autodelete(&($<node>4));
    }
    ;

declarator
    : pointer direct_declarator {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = POINTER; c_node_copy_lineno($<node>$,$<node>2);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>2));
    }
    | direct_declarator
    ;

direct_declarator
    : IDENTIFIER
    | '(' declarator ')' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = DECLARATOR_EXPRESSION; c_node_copy_lineno($<node>$,$<node>2);
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>1));
        c_node_release_autodelete(&($<node>3));
    }
    | direct_declarator '[' ']' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = ARRAY_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_release_autodelete(&($<node>3));
    }
    | direct_declarator '[' '*' ']' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = ARRAY_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>4));
    }
    | direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
    | direct_declarator '[' STATIC assignment_expression ']'
    | direct_declarator '[' type_qualifier_list '*' ']'
    | direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
    | direct_declarator '[' type_qualifier_list assignment_expression ']'
    | direct_declarator '[' type_qualifier_list ']'
    | direct_declarator '[' assignment_expression ']' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = ARRAY_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>4));
    }
    | direct_declarator '(' parameter_type_list ')' {
        struct c_node *idlist;

        idlist = c_node_alloc_or_die(); c_node_addref(&idlist); idlist->token = PARAMETER_LIST; c_node_copy_lineno($<node>$,$<node>3);
        c_node_scan_to_head(&($<node>3));
        c_node_move_to_child_link(idlist,0,&($<node>3));

        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = FUNCTION_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,1,&idlist);
        c_node_release_autodelete(&($<node>4));
    }
    | direct_declarator '(' ')' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = FUNCTION_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_release_autodelete(&($<node>3));
    }
    | direct_declarator '(' identifier_list ')' {
        struct c_node *idlist;

        idlist = c_node_alloc_or_die(); c_node_addref(&idlist); idlist->token = IDENTIFIER_LIST; c_node_copy_lineno($<node>$,$<node>3);
        c_node_scan_to_head(&($<node>3));
        c_node_move_to_child_link(idlist,0,&($<node>3));

        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = FUNCTION_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,1,&idlist);
        c_node_release_autodelete(&($<node>4));
    }
    ;

pointer
    : '*' type_qualifier_list pointer {
        $<node>$ = $<node>1;
        $<node>$->token = POINTER_DEREF;
        c_node_scan_to_head(&($<node>2));
        c_node_declaration_specifiers_group_combine(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
    }
    | '*' type_qualifier_list {
        $<node>$ = $<node>1;
        $<node>$->token = POINTER_DEREF;
        c_node_scan_to_head(&($<node>2));
        c_node_declaration_specifiers_group_combine(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>2));
    }
    | '*' pointer {
        $<node>$ = $<node>1;
        $<node>$->token = POINTER_DEREF;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
    }
    | '*' {
        $<node>$->token = POINTER_DEREF;
    }
    ;

type_qualifier_list
    : type_qualifier {
        $<node>$ = $<node>1;
        $<node>$->groupcode = TYPE_QUALIFIER;
    }
    | type_qualifier_list type_qualifier {
        $<node>$ = $<node>2;
        $<node>$->groupcode = TYPE_QUALIFIER;
        c_node_move_to_prev_link($<node>$,&($<node>1));
    }
    ;


parameter_type_list
    : parameter_list ',' ELLIPSIS {
        $<node>$ = $<node>1;
        c_node_move_to_next_link($<node>$,&($<node>3));
        c_node_release_autodelete(&($<node>2));
    }
    | parameter_list {
        c_node_scan_to_head(&($<node>1));
        $<node>$ = $<node>1;
    }
    ;

parameter_list
    : parameter_declaration {
        $<node>$ = $<node>1;
    }
    | parameter_list ',' parameter_declaration {
        $<node>$ = $<node>3;
        c_node_move_to_prev_link($<node>$,&($<node>1));
        c_node_release_autodelete(&($<node>2));
    }
    ;

parameter_declaration
    : declaration_specifiers declarator {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = DECLARATION; c_node_copy_lineno($<node>$,$<node>1);
        c_node_declaration_specifiers_group_combine(&($<node>1));
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>2));
        c_node_init_declaration(&($<node>$));
    }
    | declaration_specifiers abstract_declarator {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = DECLARATION; c_node_copy_lineno($<node>$,$<node>1);
        c_node_declaration_specifiers_group_combine(&($<node>1));
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>2));
        c_node_init_declaration(&($<node>$));
    }
    | declaration_specifiers
    ;

identifier_list
    : IDENTIFIER
    | identifier_list ',' IDENTIFIER {
        $<node>$ = $<node>3;
        c_node_move_to_prev_link($<node>$,&($<node>1));
        c_node_release_autodelete(&($<node>2));
    }
    ;

type_name
    : specifier_qualifier_list abstract_declarator {
        $<node>$ = $<node>1;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
    }
    | specifier_qualifier_list {
        $<node>$ = $<node>1;
    }
    ;

abstract_declarator
    : pointer direct_abstract_declarator {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = POINTER; c_node_copy_lineno($<node>$,$<node>2);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>2));
    }
    | pointer {
        $<node>$ = $<node>1;
    }
    | direct_abstract_declarator {
        $<node>$ = $<node>1;
    }
    ;

direct_abstract_declarator
    : '(' abstract_declarator ')' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = DECLARATOR_EXPRESSION; c_node_copy_lineno($<node>$,$<node>2);
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>1));
        c_node_release_autodelete(&($<node>3));
    }
    | '[' ']' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = ARRAY_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_release_autodelete(&($<node>1));
        c_node_release_autodelete(&($<node>2));
    }
    | '[' '*' ']' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = ARRAY_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_release_autodelete(&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>2));
        c_node_release_autodelete(&($<node>3));
    }
    | '[' STATIC type_qualifier_list assignment_expression ']'
    | '[' STATIC assignment_expression ']'
    | '[' type_qualifier_list STATIC assignment_expression ']'
    | '[' type_qualifier_list assignment_expression ']'
    | '[' type_qualifier_list ']'
    | '[' assignment_expression ']' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = ARRAY_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_release_autodelete(&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>2));
        c_node_release_autodelete(&($<node>3));
    }
    | direct_abstract_declarator '[' ']' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = ARRAY_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_release_autodelete(&($<node>3));
    }
    | direct_abstract_declarator '[' '*' ']' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = ARRAY_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>4));
    }
    | direct_abstract_declarator '[' STATIC type_qualifier_list assignment_expression ']'
    | direct_abstract_declarator '[' STATIC assignment_expression ']'
    | direct_abstract_declarator '[' type_qualifier_list assignment_expression ']'
    | direct_abstract_declarator '[' type_qualifier_list STATIC assignment_expression ']'
    | direct_abstract_declarator '[' type_qualifier_list ']'
    | direct_abstract_declarator '[' assignment_expression ']' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = ARRAY_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
        c_node_release_autodelete(&($<node>4));
    }
    | '(' ')' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = FUNCTION_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_release_autodelete(&($<node>1));
        c_node_release_autodelete(&($<node>2));
    }
    | '(' parameter_type_list ')' {
        struct c_node *idlist;

        idlist = c_node_alloc_or_die(); c_node_addref(&idlist); idlist->token = PARAMETER_LIST; c_node_copy_lineno($<node>$,$<node>1);
        c_node_scan_to_head(&($<node>2));
        c_node_move_to_child_link(idlist,0,&($<node>2));

        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = FUNCTION_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_release_autodelete(&($<node>1));
        c_node_move_to_child_link($<node>$,1,&idlist);
        c_node_release_autodelete(&($<node>3));
    }
    | direct_abstract_declarator '(' ')' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = FUNCTION_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_release_autodelete(&($<node>3));
    }
    | direct_abstract_declarator '(' parameter_type_list ')' {
        struct c_node *idlist;

        idlist = c_node_alloc_or_die(); c_node_addref(&idlist); idlist->token = PARAMETER_LIST; c_node_copy_lineno($<node>$,$<node>3);
        c_node_scan_to_head(&($<node>3));
        c_node_move_to_child_link(idlist,0,&($<node>3));

        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = FUNCTION_REF; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,1,&idlist);
        c_node_release_autodelete(&($<node>4));
    }
    ;

initializer
    : '{' initializer_list '}' {
        c_node_scan_to_head(&($<node>2));
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = INITIALIZER_LIST; c_node_copy_lineno($<node>$,$<node>1);
        $<node>$->value.value_INITIALIZER_LIST.extra_elem = 0;
        c_node_release_autodelete(&($<node>1));
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
    }
    | '{' initializer_list ',' '}' {
        c_node_scan_to_head(&($<node>2));
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = INITIALIZER_LIST; c_node_copy_lineno($<node>$,$<node>1);
        $<node>$->value.value_INITIALIZER_LIST.extra_elem = 1;
        c_node_release_autodelete(&($<node>1));
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
        c_node_release_autodelete(&($<node>4));
    }
    | assignment_expression
    ;

initializer_list
    : designation initializer {
        $<node>$ = $<node>2;
        c_node_move_to_child_link($<node>$,0,&($<node>1));
    }
    | initializer {
        $<node>$ = $<node>1;
    }
    | initializer_list ',' designation initializer {
        $<node>$ = $<node>4;
        c_node_move_to_prev_link($<node>$,&($<node>1));
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
    }
    | initializer_list ',' initializer {
        $<node>$ = $<node>3;
        c_node_move_to_prev_link($<node>$,&($<node>1));
        c_node_release_autodelete(&($<node>2));
    }
    ;

designation
    : designator_list '=' {
        c_node_scan_to_head(&($<node>1));
        $<node>$ = $<node>2;
        $<node>$->token = DESIGNATION;
        c_node_move_to_child_link($<node>$,0,&($<node>1));
    }
    ;

designator_list
    : designator {
        $<node>$ = $<node>1;
    }
    | designator_list designator {
        $<node>$ = $<node>2;
        c_node_move_to_prev_link($<node>$,&($<node>1));
    }
    ;

designator
    : '[' constant_expression ']' {
        $<node>$ = $<node>1;
        $<node>$->token = DESIGNATOR_ARRAY_INDEX;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
    }
    | '.' IDENTIFIER {
        $<node>$ = $<node>1;
        $<node>$->token = DESIGNATOR_STRUCT_MEMBER;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
    }
    ;

static_assert_declaration
    : STATIC_ASSERT '(' constant_expression ',' STRING_LITERAL ')' ';' {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_release_autodelete(&($<node>4));
        c_node_move_to_child_link($<node>$,1,&($<node>5));
        c_node_release_autodelete(&($<node>6));
        c_node_release_autodelete(&($<node>7));
    }
    ;

statement
    : labeled_statement
    | compound_statement
    | expression_statement
    | selection_statement
    | iteration_statement
    | jump_statement
    ;

labeled_statement
    : IDENTIFIER ':' statement {
        $<node>$ = $<node>2;
        $<node>$->token = LABEL;
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
    }
    | CASE constant_expression ':' statement {
        $<node>$ = $<node>1;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
        c_node_move_to_child_link($<node>$,1,&($<node>4));
    }
    | DEFAULT ':' statement {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,1,&($<node>3));
    }
    ;

compound_statement
    : '{' '}' {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = COMPOUND_STATEMENT; c_node_copy_lineno($<node>$,$<node>1);
        c_node_release_autodelete(&($<node>1));
        c_node_release_autodelete(&($<node>2));
    }
    | '{'  block_item_list '}' {
        c_node_scan_to_head(&($<node>2));
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = COMPOUND_STATEMENT; c_node_copy_lineno($<node>$,$<node>1);
        c_node_release_autodelete(&($<node>1));
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
    }
    ;

block_item_list
    : block_item {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = BLOCK_ITEM; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
    }
    | block_item_list block_item {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = BLOCK_ITEM; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_move_to_prev_link($<node>$,&($<node>1));
    }
    ;

block_item
    : declaration
    | statement
    ;

expression_statement
    : ';' {
        $<node>$->token = EXPRESSION;
    }
    | expression ';' {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
    }
    ;

selection_statement
    : IF '(' expression ')' statement ELSE statement {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_release_autodelete(&($<node>4));
        c_node_move_to_child_link($<node>$,1,&($<node>5));
        c_node_release_autodelete(&($<node>6));
        c_node_move_to_child_link($<node>$,2,&($<node>7));
    }
    | IF '(' expression ')' statement {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_release_autodelete(&($<node>4));
        c_node_move_to_child_link($<node>$,1,&($<node>5));
    }
    | SWITCH '(' expression ')' statement {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_release_autodelete(&($<node>4));
        c_node_move_to_child_link($<node>$,1,&($<node>5));
    }
    ;

iteration_statement
    : WHILE '(' expression ')' statement {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_release_autodelete(&($<node>4));
        c_node_move_to_child_link($<node>$,1,&($<node>5));
    }
    | DO statement WHILE '(' expression ')' ';' {
        $<node>$ = $<node>1;
        $<node>$->token = DO_WHILE;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
        c_node_release_autodelete(&($<node>4));
        c_node_move_to_child_link($<node>$,1,&($<node>5));
        c_node_release_autodelete(&($<node>6));
        c_node_release_autodelete(&($<node>7));
    }
    | FOR '(' expression_statement expression_statement ')' statement {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_move_to_child_link($<node>$,1,&($<node>4));
        c_node_release_autodelete(&($<node>5));
        c_node_move_to_child_link($<node>$,3,&($<node>6));
    }
    | FOR '(' expression_statement expression_statement expression ')' statement {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_move_to_child_link($<node>$,1,&($<node>4));
        c_node_move_to_child_link($<node>$,2,&($<node>5));
        c_node_release_autodelete(&($<node>6));
        c_node_move_to_child_link($<node>$,3,&($<node>7));
    }
    | FOR '(' declaration expression_statement ')' statement {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_move_to_child_link($<node>$,1,&($<node>4));
        c_node_release_autodelete(&($<node>5));
        c_node_move_to_child_link($<node>$,3,&($<node>6));
    }
    | FOR '(' declaration expression_statement expression ')' statement {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
        c_node_move_to_child_link($<node>$,0,&($<node>3));
        c_node_move_to_child_link($<node>$,1,&($<node>4));
        c_node_move_to_child_link($<node>$,2,&($<node>5));
        c_node_release_autodelete(&($<node>6));
        c_node_move_to_child_link($<node>$,3,&($<node>7));
    }
    ;

jump_statement
    : GOTO IDENTIFIER ';' {
        $<node>$ = $<node>1;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
    }
    | CONTINUE ';' {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
    }
    | BREAK ';' {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
    }
    | RETURN ';' {
        $<node>$ = $<node>1;
        c_node_release_autodelete(&($<node>2));
    }
    | RETURN expression ';' {
        $<node>$ = $<node>1;
        c_node_move_to_child_link($<node>$,0,&($<node>2));
        c_node_release_autodelete(&($<node>3));
    }
    ;

translation_unit
    : external_declaration {
        $<node>$ = $<node>1;
        last_translation_unit = $<node>$;
    }
    | translation_unit external_declaration {
        $<node>$ = $<node>2;
        c_node_move_to_prev_link($<node>$,&($<node>1));
        /* do not update translation unit, because the new node is at the END of the linked list */
    }
    ;

external_declaration
    : function_definition {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = EXTERNAL_DECLARATION; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
    }
    | declaration {
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = EXTERNAL_DECLARATION; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
    }
    ;

function_definition
    : declaration_specifiers declarator declaration_list compound_statement {
        c_node_scan_to_head(&($<node>3));
        c_node_declaration_specifiers_group_combine(&($<node>1));
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = FUNCTION_DEFINITION; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>2));
        c_node_move_to_child_link($<node>$,2,&($<node>3));
        c_node_move_to_child_link($<node>$,3,&($<node>4));
    }
    | declaration_specifiers declarator compound_statement {
        c_node_declaration_specifiers_group_combine(&($<node>1));
        $<node>$ = c_node_alloc_or_die(); c_node_addref(&($<node>$)); $<node>$->token = FUNCTION_DEFINITION; c_node_copy_lineno($<node>$,$<node>1);
        c_node_move_to_child_link($<node>$,0,&($<node>1));
        c_node_move_to_child_link($<node>$,1,&($<node>2));
        c_node_move_to_child_link($<node>$,3,&($<node>3));
    }
    ;

declaration_list
    : declaration {
        $<node>$ = $<node>1;
    }
    | declaration_list declaration {
        $<node>$ = $<node>2;
        c_node_move_to_prev_link($<node>$,&($<node>1));
    }
    ;

%%
#include <stdio.h>

void yyerror(const char *s)
{
    fflush(stdout);
    fprintf(stderr, "*** %s\n", s);
}

const char *token2string(int tok) {
    if (tok < 0)
        return "?";

    return yytname[YYTRANSLATE(tok)];
}

