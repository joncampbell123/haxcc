
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

%{
#include "cnode.h"
#include "cparsb.c.h"
long line_num = 1;

extern void c_node_init(struct c_node *node);
extern c_stringref_t sconst_parse(const char *str);
extern void identifier_parse(struct c_node *node,const char *str);
extern void fconst_parse(struct c_node_val_float *val,const char *str);
extern void iconst_parse(struct c_node_val_int *val,const char *str,const unsigned int base);
extern void iconst_parse_char(struct c_node_val_int *val,const char *str);

extern void yyerror(const char *);  /* prints grammar violation message */

extern int sym_type(const char *);  /* returns type from symbol table */

#define sym_type(identifier) IDENTIFIER /* with no symbol table, fake it */

static void comment(void);
static int check_type(void);
%}

%%
"/*"                                    { comment(); }
"//".*                                    { /* consume //-comment */ }

"auto"                  { c_node_init(&yylval.node); return(yylval.node.token=AUTO); }
"break"                 { c_node_init(&yylval.node); return(yylval.node.token=BREAK); }
"case"                  { c_node_init(&yylval.node); return(yylval.node.token=CASE); }
"char"                  { c_node_init(&yylval.node); return(yylval.node.token=CHAR); }
"const"                 { c_node_init(&yylval.node); return(yylval.node.token=CONST); }
"continue"              { c_node_init(&yylval.node); return(yylval.node.token=CONTINUE); }
"default"               { c_node_init(&yylval.node); return(yylval.node.token=DEFAULT); }
"do"                    { c_node_init(&yylval.node); return(yylval.node.token=DO); }
"double"                { c_node_init(&yylval.node); return(yylval.node.token=DOUBLE); }
"else"                  { c_node_init(&yylval.node); return(yylval.node.token=ELSE); }
"enum"                  { c_node_init(&yylval.node); return(yylval.node.token=ENUM); }
"extern"                { c_node_init(&yylval.node); return(yylval.node.token=EXTERN); }
"float"                 { c_node_init(&yylval.node); return(yylval.node.token=FLOAT); }
"for"                   { c_node_init(&yylval.node); return(yylval.node.token=FOR); }
"goto"                  { c_node_init(&yylval.node); return(yylval.node.token=GOTO); }
"if"                    { c_node_init(&yylval.node); return(yylval.node.token=IF); }
"inline"                { c_node_init(&yylval.node); return(yylval.node.token=INLINE); }
"int"                   { c_node_init(&yylval.node); return(yylval.node.token=INT); }
"long"                  { c_node_init(&yylval.node); return(yylval.node.token=LONG); }
"register"              { c_node_init(&yylval.node); return(yylval.node.token=REGISTER); }
"restrict"              { c_node_init(&yylval.node); return(yylval.node.token=RESTRICT); }
"return"                { c_node_init(&yylval.node); return(yylval.node.token=RETURN); }
"short"                 { c_node_init(&yylval.node); return(yylval.node.token=SHORT); }
"signed"                { c_node_init(&yylval.node); return(yylval.node.token=SIGNED); }
"sizeof"                { c_node_init(&yylval.node); return(yylval.node.token=SIZEOF); }
"static"                { c_node_init(&yylval.node); return(yylval.node.token=STATIC); }
"struct"                { c_node_init(&yylval.node); return(yylval.node.token=STRUCT); }
"switch"                { c_node_init(&yylval.node); return(yylval.node.token=SWITCH); }
"typedef"               { c_node_init(&yylval.node); return(yylval.node.token=TYPEDEF); }
"union"                 { c_node_init(&yylval.node); return(yylval.node.token=UNION); }
"unsigned"              { c_node_init(&yylval.node); return(yylval.node.token=UNSIGNED); }
"void"                  { c_node_init(&yylval.node); return(yylval.node.token=VOID); }
"volatile"              { c_node_init(&yylval.node); return(yylval.node.token=VOLATILE); }
"while"                 { c_node_init(&yylval.node); return(yylval.node.token=WHILE); }
"_Alignas"                              { c_node_init(&yylval.node); return yylval.node.token=ALIGNAS; }
"_Alignof"                              { c_node_init(&yylval.node); return yylval.node.token=ALIGNOF; }
"_Atomic"                               { c_node_init(&yylval.node); return yylval.node.token=ATOMIC; }
"_Bool"                                 { c_node_init(&yylval.node); return yylval.node.token=BOOL; }
"_Complex"                              { c_node_init(&yylval.node); return yylval.node.token=COMPLEX; }
"_Generic"                              { c_node_init(&yylval.node); return yylval.node.token=GENERIC; }
"_Imaginary"                            { c_node_init(&yylval.node); return yylval.node.token=IMAGINARY; }
"_Noreturn"                             { c_node_init(&yylval.node); return yylval.node.token=NORETURN; }
"_Static_assert"                        { c_node_init(&yylval.node); return yylval.node.token=STATIC_ASSERT; }
"_Thread_local"                         { c_node_init(&yylval.node); return yylval.node.token=THREAD_LOCAL; }
"__func__"                              { c_node_init(&yylval.node); return yylval.node.token=FUNC_NAME; }

{L}{A}*                 { c_node_init(&yylval.node); identifier_parse(&yylval.node,yytext); return yylval.node.token=check_type(); }

{HP}{H}+{IS}?               { c_node_init(&yylval.node); iconst_parse(&yylval.node.value.val_uint,yytext,16); return yylval.node.token=I_CONSTANT; }
{NZ}{D}*{IS}?               { c_node_init(&yylval.node); iconst_parse(&yylval.node.value.val_uint,yytext,10); return yylval.node.token=I_CONSTANT; }
"0"{O}*{IS}?                { c_node_init(&yylval.node); iconst_parse(&yylval.node.value.val_uint,yytext,8); return yylval.node.token=I_CONSTANT; }
{CP}?"'"([^'\\\n]|{ES})+"'"     { c_node_init(&yylval.node); iconst_parse_char(&yylval.node.value.val_uint,yytext); return yylval.node.token=I_CONSTANT; }

{D}+{E}{FS}?                { c_node_init(&yylval.node); fconst_parse(&yylval.node.value.val_float,yytext); return yylval.node.token=F_CONSTANT; }
{D}*"."{D}+{E}?{FS}?            { c_node_init(&yylval.node); fconst_parse(&yylval.node.value.val_float,yytext); return yylval.node.token=F_CONSTANT; }
{D}+"."{E}?{FS}?            { c_node_init(&yylval.node); fconst_parse(&yylval.node.value.val_float,yytext); return yylval.node.token=F_CONSTANT; }
{HP}{H}+{P}{FS}?            { c_node_init(&yylval.node); fconst_parse(&yylval.node.value.val_float,yytext); return yylval.node.token=F_CONSTANT; }
{HP}{H}*"."{H}+{P}{FS}?         { c_node_init(&yylval.node); fconst_parse(&yylval.node.value.val_float,yytext); return yylval.node.token=F_CONSTANT; }
{HP}{H}+"."{P}{FS}?         { c_node_init(&yylval.node); fconst_parse(&yylval.node.value.val_float,yytext); return yylval.node.token=F_CONSTANT; }

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+   { c_node_init(&yylval.node); yylval.node.value.val_string=sconst_parse(yytext); return STRING_LITERAL; }

"..."                   { c_node_init(&yylval.node); return yylval.node.token=ELLIPSIS; }
">>="                   { c_node_init(&yylval.node); return yylval.node.token=RIGHT_ASSIGN; }
"<<="                   { c_node_init(&yylval.node); return yylval.node.token=LEFT_ASSIGN; }
"+="                    { c_node_init(&yylval.node); return yylval.node.token=ADD_ASSIGN; }
"-="                    { c_node_init(&yylval.node); return yylval.node.token=SUB_ASSIGN; }
"*="                    { c_node_init(&yylval.node); return yylval.node.token=MUL_ASSIGN; }
"/="                    { c_node_init(&yylval.node); return yylval.node.token=DIV_ASSIGN; }
"%="                    { c_node_init(&yylval.node); return yylval.node.token=MOD_ASSIGN; }
"&="                    { c_node_init(&yylval.node); return yylval.node.token=AND_ASSIGN; }
"^="                    { c_node_init(&yylval.node); return yylval.node.token=XOR_ASSIGN; }
"|="                    { c_node_init(&yylval.node); return yylval.node.token=OR_ASSIGN; }
">>"                    { c_node_init(&yylval.node); return yylval.node.token=RIGHT_OP; }
"<<"                    { c_node_init(&yylval.node); return yylval.node.token=LEFT_OP; }
"++"                    { c_node_init(&yylval.node); return yylval.node.token=INC_OP; }
"--"                    { c_node_init(&yylval.node); return yylval.node.token=DEC_OP; }
"->"                    { c_node_init(&yylval.node); return yylval.node.token=PTR_OP; }
"&&"                    { c_node_init(&yylval.node); return yylval.node.token=AND_OP; }
"||"                    { c_node_init(&yylval.node); return yylval.node.token=OR_OP; }
"<="                    { c_node_init(&yylval.node); return yylval.node.token=LE_OP; }
">="                    { c_node_init(&yylval.node); return yylval.node.token=GE_OP; }
"=="                    { c_node_init(&yylval.node); return yylval.node.token=EQ_OP; }
"!="                    { c_node_init(&yylval.node); return yylval.node.token=NE_OP; }
";"                 { c_node_init(&yylval.node); return yylval.node.token=';'; }
("{"|"<%")              { c_node_init(&yylval.node); return yylval.node.token='{'; }
("}"|"%>")              { c_node_init(&yylval.node); return yylval.node.token='}'; }
","                 { c_node_init(&yylval.node); return yylval.node.token=','; }
":"                 { c_node_init(&yylval.node); return yylval.node.token=':'; }
"="                 { c_node_init(&yylval.node); return yylval.node.token='='; }
"("                 { c_node_init(&yylval.node); return yylval.node.token='('; }
")"                 { c_node_init(&yylval.node); return yylval.node.token=')'; }
("["|"<:")              { c_node_init(&yylval.node); return yylval.node.token='['; }
("]"|":>")              { c_node_init(&yylval.node); return yylval.node.token=']'; }
"."                 { c_node_init(&yylval.node); return yylval.node.token='.'; }
"&"                 { c_node_init(&yylval.node); return yylval.node.token='&'; }
"!"                 { c_node_init(&yylval.node); return yylval.node.token='!'; }
"~"                 { c_node_init(&yylval.node); return yylval.node.token='~'; }
"-"                 { c_node_init(&yylval.node); return yylval.node.token='-'; }
"+"                 { c_node_init(&yylval.node); return yylval.node.token='+'; }
"*"                 { c_node_init(&yylval.node); return yylval.node.token='*'; }
"/"                 { c_node_init(&yylval.node); return yylval.node.token='/'; }
"%"                 { c_node_init(&yylval.node); return yylval.node.token='%'; }
"<"                 { c_node_init(&yylval.node); return yylval.node.token='<'; }
">"                 { c_node_init(&yylval.node); return yylval.node.token='>'; }
"^"                 { c_node_init(&yylval.node); return yylval.node.token='^'; }
"|"                 { c_node_init(&yylval.node); return yylval.node.token='|'; }
"?"                 { c_node_init(&yylval.node); return yylval.node.token='?'; }

{WS}+                   { /* whitespace separates tokens */ }
.                   { /* discard bad characters */ }

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

