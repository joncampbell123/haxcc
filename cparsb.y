
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

int c_node_add(struct c_node *res,struct c_node *p1,struct c_node *p2);
int c_node_sub(struct c_node *res,struct c_node *p1,struct c_node *p2);
int c_node_divide(struct c_node *res,struct c_node *p1,struct c_node *p2);
int c_node_modulus(struct c_node *res,struct c_node *p1,struct c_node *p2);
int c_node_multiply(struct c_node *res,struct c_node *p1,struct c_node *p2);
int c_node_add_declaration_init_decl(struct c_node *decl,struct c_node *initdecl);
int c_node_init_decl_attach_initializer(struct c_node *decl,struct c_node *init);
int c_node_add_init_decl(struct c_node *decl,struct c_node *initdecl);
int c_node_convert_to_initializer(struct c_node *decl);
int c_node_on_init_decl(struct c_node *typ); /* convert to INIT_DECL_LIST */
int c_node_on_func_spec(struct c_node *typ); /* convert to FUNC_SPECIFIER */
int c_node_on_type_qual(struct c_node *typ); /* convert to TYPE_QUALIFIER */
int c_node_on_type_spec(struct c_node *typ); /* convert to TYPE_SPECIFIER */
int c_node_type_to_decl(struct c_node *typ); /* convert TYPE_SPECIFIER to DECL_SPECIFIER */
int c_node_finish_declaration(struct c_node *decl);
int c_node_add_type_to_decl(struct c_node *decl,struct c_node *typ);
int c_node_on_storage_class_spec(struct c_node *stc); /* convert to STORAGE_CLASS_SPECIFIER */

%}

%union {
    struct c_node   node;
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

%token  LONG_LONG
%token  LONG_DOUBLE

%token  STORAGE_CLASS_SPECIFIER
%token  TYPE_SPECIFIER
%token  TYPE_QUALIFIER
%token  DECL_SPECIFIER
%token  FUNC_SPECIFIER
%token  INIT_DECL_LIST
%token  INITIALIZER

%start translation_unit
%%

primary_expression
    : IDENTIFIER
    | constant
    | string
    | '(' expression ')'
    | generic_selection
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
    | postfix_expression '[' expression ']'
    | postfix_expression '(' ')'
    | postfix_expression '(' argument_expression_list ')'
    | postfix_expression '.' IDENTIFIER
    | postfix_expression PTR_OP IDENTIFIER
    | postfix_expression INC_OP
    | postfix_expression DEC_OP
    | '(' type_name ')' '{' initializer_list '}'
    | '(' type_name ')' '{' initializer_list ',' '}'
    ;

argument_expression_list
    : assignment_expression
    | argument_expression_list ',' assignment_expression
    ;

unary_expression
    : postfix_expression
    | INC_OP unary_expression
    | DEC_OP unary_expression
    | unary_operator cast_expression
    | SIZEOF unary_expression
    | SIZEOF '(' type_name ')'
    | ALIGNOF '(' type_name ')'
    ;

unary_operator
    : '&'
    | '*'
    | '+'
    | '-'
    | '~'
    | '!'
    ;

cast_expression
    : unary_expression
    | '(' type_name ')' cast_expression
    ;

multiplicative_expression
    : cast_expression
    | multiplicative_expression '*' cast_expression {
        if (!c_node_multiply(&($<node>$),&($<node>1),&($<node>3))) YYABORT;
    }
    | multiplicative_expression '/' cast_expression {
        if (!c_node_divide(&($<node>$),&($<node>1),&($<node>3))) YYABORT;
    }
    | multiplicative_expression '%' cast_expression {
        if (!c_node_modulus(&($<node>$),&($<node>1),&($<node>3))) YYABORT;
    }
    ;

additive_expression
    : multiplicative_expression
    | additive_expression '+' multiplicative_expression {
        if (!c_node_add(&($<node>$),&($<node>1),&($<node>3))) YYABORT;
    }
    | additive_expression '-' multiplicative_expression {
        if (!c_node_sub(&($<node>$),&($<node>1),&($<node>3))) YYABORT;
    }
    ;

shift_expression
    : additive_expression
    | shift_expression LEFT_OP additive_expression
    | shift_expression RIGHT_OP additive_expression
    ;

relational_expression
    : shift_expression
    | relational_expression '<' shift_expression
    | relational_expression '>' shift_expression
    | relational_expression LE_OP shift_expression
    | relational_expression GE_OP shift_expression
    ;

equality_expression
    : relational_expression
    | equality_expression EQ_OP relational_expression
    | equality_expression NE_OP relational_expression
    ;

and_expression
    : equality_expression
    | and_expression '&' equality_expression
    ;

exclusive_or_expression
    : and_expression
    | exclusive_or_expression '^' and_expression
    ;

inclusive_or_expression
    : exclusive_or_expression
    | inclusive_or_expression '|' exclusive_or_expression
    ;

logical_and_expression
    : inclusive_or_expression
    | logical_and_expression AND_OP inclusive_or_expression
    ;

logical_or_expression
    : logical_and_expression
    | logical_or_expression OR_OP logical_and_expression
    ;

conditional_expression
    : logical_or_expression
    | logical_or_expression '?' expression ':' conditional_expression
    ;

assignment_expression
    : conditional_expression
    | unary_expression assignment_operator assignment_expression
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
    | expression ',' assignment_expression
    ;

constant_expression
    : conditional_expression    /* with constraints */
    ;

declaration
    : declaration_specifiers ';' {
        if (!c_node_finish_declaration(&($<node>1))) YYABORT;
    }
    | declaration_specifiers init_declarator_list ';' {
        if (!c_node_add_declaration_init_decl(&($<node>1),&($<node>2))) YYABORT;
        if (!c_node_finish_declaration(&($<node>1))) YYABORT;
    }
    | static_assert_declaration
    ;

declaration_specifiers
    : storage_class_specifier declaration_specifiers {
        if (!c_node_on_storage_class_spec(&($<node>1))) YYABORT;
        if (!c_node_add_type_to_decl(&($<node>2),&($<node>1))) YYABORT;
        $<node>$ = $<node>2;
    }
    | storage_class_specifier {
        if (!c_node_on_storage_class_spec(&($<node>$))) YYABORT;
        if (!c_node_type_to_decl(&($<node>$))) YYABORT;
    }
    | type_specifier declaration_specifiers {
        if (!c_node_on_type_spec(&($<node>1))) YYABORT;
        if (!c_node_add_type_to_decl(&($<node>2),&($<node>1))) YYABORT;
        $<node>$ = $<node>2;
    }
    | type_specifier {
        if (!c_node_on_type_spec(&($<node>$))) YYABORT;
        if (!c_node_type_to_decl(&($<node>$))) YYABORT;
    }
    | type_qualifier declaration_specifiers {
        if (!c_node_on_type_qual(&($<node>1))) YYABORT;
        if (!c_node_add_type_to_decl(&($<node>2),&($<node>1))) YYABORT;
        $<node>$ = $<node>2;
    }
    | type_qualifier {
        if (!c_node_on_type_qual(&($<node>$))) YYABORT;
        if (!c_node_type_to_decl(&($<node>$))) YYABORT;
    }
    | function_specifier declaration_specifiers {
        if (!c_node_on_func_spec(&($<node>1))) YYABORT;
        if (!c_node_add_type_to_decl(&($<node>2),&($<node>1))) YYABORT;
        $<node>$ = $<node>2;
    }
    | function_specifier {
        if (!c_node_on_func_spec(&($<node>$))) YYABORT;
        if (!c_node_type_to_decl(&($<node>$))) YYABORT;
    }
    | alignment_specifier declaration_specifiers {
        /* TODO */
        $<node>$ = $<node>2;
    }
    | alignment_specifier
    ;

init_declarator_list
    : init_declarator
    | init_declarator_list ',' init_declarator {
        if (!c_node_add_init_decl(&($<node>1),&($<node>3))) YYABORT;
        $<node>$ = $<node>1;
    }
    ;

init_declarator
    : declarator '=' initializer {
        if (!c_node_convert_to_initializer(&($<node>3))) YYABORT;
        if (!c_node_on_init_decl(&($<node>1))) YYABORT;
        if (!c_node_init_decl_attach_initializer(&($<node>1),&($<node>3))) YYABORT;
        $<node>$ = $<node>1;
    }
    | declarator {
        if (!c_node_on_init_decl(&($<node>$))) YYABORT;
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
    : struct_or_union '{' struct_declaration_list '}'
    | struct_or_union IDENTIFIER '{' struct_declaration_list '}'
    | struct_or_union IDENTIFIER
    ;

struct_or_union
    : STRUCT
    | UNION
    ;

struct_declaration_list
    : struct_declaration
    | struct_declaration_list struct_declaration
    ;

struct_declaration
    : specifier_qualifier_list ';'  /* for anonymous struct/union */
    | specifier_qualifier_list struct_declarator_list ';'
    | static_assert_declaration
    ;

specifier_qualifier_list
    : type_specifier specifier_qualifier_list
    | type_specifier
    | type_qualifier specifier_qualifier_list
    | type_qualifier
    ;

struct_declarator_list
    : struct_declarator
    | struct_declarator_list ',' struct_declarator
    ;

struct_declarator
    : ':' constant_expression
    | declarator ':' constant_expression
    | declarator
    ;

enum_specifier
    : ENUM '{' enumerator_list '}'
    | ENUM '{' enumerator_list ',' '}'
    | ENUM IDENTIFIER '{' enumerator_list '}'
    | ENUM IDENTIFIER '{' enumerator_list ',' '}'
    | ENUM IDENTIFIER
    ;

enumerator_list
    : enumerator
    | enumerator_list ',' enumerator
    ;

enumerator  /* identifiers must be flagged as ENUMERATION_CONSTANT */
    : enumeration_constant '=' constant_expression
    | enumeration_constant
    ;

atomic_type_specifier
    : ATOMIC '(' type_name ')'
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
    : ALIGNAS '(' type_name ')'
    | ALIGNAS '(' constant_expression ')'
    ;

declarator
    : pointer direct_declarator
    | direct_declarator
    ;

direct_declarator
    : IDENTIFIER
    | '(' declarator ')'
    | direct_declarator '[' ']'
    | direct_declarator '[' '*' ']'
    | direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
    | direct_declarator '[' STATIC assignment_expression ']'
    | direct_declarator '[' type_qualifier_list '*' ']'
    | direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
    | direct_declarator '[' type_qualifier_list assignment_expression ']'
    | direct_declarator '[' type_qualifier_list ']'
    | direct_declarator '[' assignment_expression ']'
    | direct_declarator '(' parameter_type_list ')'
    | direct_declarator '(' ')'
    | direct_declarator '(' identifier_list ')'
    ;

pointer
    : '*' type_qualifier_list pointer
    | '*' type_qualifier_list
    | '*' pointer
    | '*'
    ;

type_qualifier_list
    : type_qualifier
    | type_qualifier_list type_qualifier
    ;


parameter_type_list
    : parameter_list ',' ELLIPSIS
    | parameter_list
    ;

parameter_list
    : parameter_declaration
    | parameter_list ',' parameter_declaration
    ;

parameter_declaration
    : declaration_specifiers declarator {
        if (!c_node_finish_declaration(&($<node>1))) YYABORT;
    }
    | declaration_specifiers abstract_declarator {
        if (!c_node_finish_declaration(&($<node>1))) YYABORT;
    }
    | declaration_specifiers {
        if (!c_node_finish_declaration(&($<node>1))) YYABORT;
    }
    ;

identifier_list
    : IDENTIFIER
    | identifier_list ',' IDENTIFIER
    ;

type_name
    : specifier_qualifier_list abstract_declarator
    | specifier_qualifier_list
    ;

abstract_declarator
    : pointer direct_abstract_declarator
    | pointer
    | direct_abstract_declarator
    ;

direct_abstract_declarator
    : '(' abstract_declarator ')'
    | '[' ']'
    | '[' '*' ']'
    | '[' STATIC type_qualifier_list assignment_expression ']'
    | '[' STATIC assignment_expression ']'
    | '[' type_qualifier_list STATIC assignment_expression ']'
    | '[' type_qualifier_list assignment_expression ']'
    | '[' type_qualifier_list ']'
    | '[' assignment_expression ']'
    | direct_abstract_declarator '[' ']'
    | direct_abstract_declarator '[' '*' ']'
    | direct_abstract_declarator '[' STATIC type_qualifier_list assignment_expression ']'
    | direct_abstract_declarator '[' STATIC assignment_expression ']'
    | direct_abstract_declarator '[' type_qualifier_list assignment_expression ']'
    | direct_abstract_declarator '[' type_qualifier_list STATIC assignment_expression ']'
    | direct_abstract_declarator '[' type_qualifier_list ']'
    | direct_abstract_declarator '[' assignment_expression ']'
    | '(' ')'
    | '(' parameter_type_list ')'
    | direct_abstract_declarator '(' ')'
    | direct_abstract_declarator '(' parameter_type_list ')'
    ;

initializer
    : '{' initializer_list '}'
    | '{' initializer_list ',' '}'
    | assignment_expression
    ;

initializer_list
    : designation initializer
    | initializer
    | initializer_list ',' designation initializer
    | initializer_list ',' initializer
    ;

designation
    : designator_list '='
    ;

designator_list
    : designator
    | designator_list designator
    ;

designator
    : '[' constant_expression ']'
    | '.' IDENTIFIER
    ;

static_assert_declaration
    : STATIC_ASSERT '(' constant_expression ',' STRING_LITERAL ')' ';'
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
    : IDENTIFIER ':' statement
    | CASE constant_expression ':' statement
    | DEFAULT ':' statement
    ;

compound_statement
    : '{' '}'
    | '{'  block_item_list '}'
    ;

block_item_list
    : block_item
    | block_item_list block_item
    ;

block_item
    : declaration
    | statement
    ;

expression_statement
    : ';'
    | expression ';'
    ;

selection_statement
    : IF '(' expression ')' statement ELSE statement
    | IF '(' expression ')' statement
    | SWITCH '(' expression ')' statement
    ;

iteration_statement
    : WHILE '(' expression ')' statement
    | DO statement WHILE '(' expression ')' ';'
    | FOR '(' expression_statement expression_statement ')' statement
    | FOR '(' expression_statement expression_statement expression ')' statement
    | FOR '(' declaration expression_statement ')' statement
    | FOR '(' declaration expression_statement expression ')' statement
    ;

jump_statement
    : GOTO IDENTIFIER ';'
    | CONTINUE ';'
    | BREAK ';'
    | RETURN ';'
    | RETURN expression ';'
    ;

translation_unit
    : external_declaration
    | translation_unit external_declaration
    ;

external_declaration
    : function_definition
    | declaration
    ;

function_definition
    : declaration_specifiers declarator declaration_list compound_statement {
        if (!c_node_finish_declaration(&($<node>1))) YYABORT;
    }
    | declaration_specifiers declarator compound_statement {
        if (!c_node_finish_declaration(&($<node>1))) YYABORT;
    }
    ;

declaration_list
    : declaration
    | declaration_list declaration
    ;

%%
#include <stdio.h>

void yyerror(const char *s)
{
    fflush(stdout);
    fprintf(stderr, "*** %s\n", s);
}

