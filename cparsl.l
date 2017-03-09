%e  1019
%p  2807
%n  371
%k  284
%a  1213
%o  1117

O   [0-7]
D   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]
H   [a-fA-F0-9]
HP  (0[xX])
E   ([Ee][+-]?{D}+)
P   ([Pp][+-]?{D}+)
FS  (f|F|l|L)
IS  (((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))
CP  (u|U|L)
SP  (u8|u|U|L)
ES  (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS  [ \t\v\n\f]

%option yylineno

%{
#include <stdio.h>
#include "cnode.h"
#include "cparsb.c.h"

extern void yyerror(const char *);  /* prints grammar violation message */

extern int sym_type(const char *);  /* returns type from symbol table */

static void comment(void);
static int check_type(void);
%}

%%
"/*"                                    { comment(); }
"//".*                                    { /* consume //-comment */ }

"auto"                  { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=AUTO); }
"break"                 { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=BREAK); }
"case"                  { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=CASE); }
"char"                  { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=CHAR); }
"const"                 { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=CONST); }
"continue"              { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=CONTINUE); }
"default"               { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=DEFAULT); }
"do"                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=DO); }
"double"                { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=DOUBLE); }
"else"                  { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=ELSE); }
"enum"                  { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=ENUM); }
"extern"                { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=EXTERN); }
"float"                 { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=FLOAT); }
"for"                   { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=FOR); }
"goto"                  { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=GOTO); }
"if"                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=IF); }
"inline"                { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=INLINE); }
"int"                   { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=INT); }
"long"                  { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=LONG); }
"register"              { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=REGISTER); }
"restrict"              { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=RESTRICT); }
"return"                { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=RETURN); }
"short"                 { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=SHORT); }
"signed"                { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=SIGNED); }
"sizeof"                { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=SIZEOF); }
"static"                { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=STATIC); }
"struct"                { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=STRUCT); }
"switch"                { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=SWITCH); }
"typedef"               { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=TYPEDEF); }
"union"                 { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=UNION); }
"unsigned"              { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=UNSIGNED); }
"void"                  { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=VOID); }
"volatile"              { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=VOLATILE); }
"while"                 { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return(yylval.node->token=WHILE); }
"_Alignas"              { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=ALIGNAS; }
"_Alignof"              { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=ALIGNOF; }
"_Atomic"               { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=ATOMIC; }
"_Bool"                 { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=BOOL; }
"_Complex"              { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=COMPLEX; }
"_Generic"              { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=GENERIC; }
"_Imaginary"            { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=IMAGINARY; }
"_Noreturn"             { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=NORETURN; }
"_Static_assert"        { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=STATIC_ASSERT; }
"_Thread_local"         { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=THREAD_LOCAL; }
"__func__"              { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=FUNC_NAME; }

{L}{A}*                 { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); c_node_identifier_parse(yylval.node,yytext); return yylval.node->token=check_type(); }

{HP}{H}+{IS}?                       { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); c_node_i_constant_parse(yylval.node,yytext,16); return yylval.node->token=I_CONSTANT; }
{NZ}{D}*{IS}?                       { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); c_node_i_constant_parse(yylval.node,yytext,10); return yylval.node->token=I_CONSTANT; }
"0"{O}*{IS}?                        { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); c_node_i_constant_parse(yylval.node,yytext,8); return yylval.node->token=I_CONSTANT; }
{CP}?"'"([^'\\\n]|{ES})+"'"         { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); c_node_i_constant_char_parse(yylval.node,yytext); return yylval.node->token=I_CONSTANT; }

{D}+{E}{FS}?                        { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); c_node_f_constant_parse(yylval.node,yytext); return yylval.node->token=F_CONSTANT; }
{D}*"."{D}+{E}?{FS}?                { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); c_node_f_constant_parse(yylval.node,yytext); return yylval.node->token=F_CONSTANT; }
{D}+"."{E}?{FS}?                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); c_node_f_constant_parse(yylval.node,yytext); return yylval.node->token=F_CONSTANT; }
{HP}{H}+{P}{FS}?                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); c_node_f_constant_parse(yylval.node,yytext); return yylval.node->token=F_CONSTANT; }
{HP}{H}*"."{H}+{P}{FS}?             { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); c_node_f_constant_parse(yylval.node,yytext); return yylval.node->token=F_CONSTANT; }
{HP}{H}+"."{P}{FS}?                 { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); c_node_f_constant_parse(yylval.node,yytext); return yylval.node->token=F_CONSTANT; }

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+   { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); yylval.node->value.value_STRING_LITERAL=sconst_parse(yytext); return yylval.node->token=STRING_LITERAL; }

"..."                   { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=ELLIPSIS; }
">>="                   { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=RIGHT_ASSIGN; }
"<<="                   { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=LEFT_ASSIGN; }
"+="                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=ADD_ASSIGN; }
"-="                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=SUB_ASSIGN; }
"*="                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=MUL_ASSIGN; }
"/="                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=DIV_ASSIGN; }
"%="                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=MOD_ASSIGN; }
"&="                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=AND_ASSIGN; }
"^="                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=XOR_ASSIGN; }
"|="                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=OR_ASSIGN; }
">>"                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=RIGHT_OP; }
"<<"                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=LEFT_OP; }
"++"                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=INC_OP; }
"--"                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=DEC_OP; }
"->"                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=PTR_OP; }
"&&"                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=AND_OP; }
"||"                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=OR_OP; }
"<="                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=LE_OP; }
">="                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=GE_OP; }
"=="                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=EQ_OP; }
"!="                    { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=NE_OP; }
";"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=';'; }
("{"|"<%")              { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='{'; }
("}"|"%>")              { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='}'; }
","                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=','; }
":"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=':'; }
"="                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='='; }
"("                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='('; }
")"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=')'; }
("["|"<:")              { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='['; }
("]"|":>")              { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token=']'; }
"."                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='.'; }
"&"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='&'; }
"!"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='!'; }
"~"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='~'; }
"-"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='-'; }
"+"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='+'; }
"*"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='*'; }
"/"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='/'; }
"%"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='%'; }
"<"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='<'; }
">"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='>'; }
"^"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='^'; }
"|"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='|'; }
"?"                     { yylval.node=c_node_alloc_or_die(); yylval.node->lineno = yylineno; c_node_addref(&yylval.node); return yylval.node->token='?'; }

{WS}+                   { /* whitespace separates tokens */ }
.                       { /* discard bad characters */ }

%%

int yywrap(void)        /* called at end of input */
{
    return 1;           /* terminate now */
}

static void comment(void)
{
    int c;

    while ((c = input()) != 0)
        if (c == '*')
        {
            while ((c = input()) == '*')
                ;

            if (c == '/')
                return;

            if (c == 0)
                break;
        }
    yyerror("unterminated comment");
}

static int check_type(void)
{
    switch (sym_type(yytext))
    {
    case TYPEDEF_NAME:                /* previously defined */
        return TYPEDEF_NAME;
    case ENUMERATION_CONSTANT:        /* previously defined */
        return ENUMERATION_CONSTANT;
    default:                          /* includes undefined */
        return IDENTIFIER;
    }
}

