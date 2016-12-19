
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

extern c_stringref_t sconst_parse(char *str);
extern c_identref_t identifier_parse(char *str);
extern void fconst_parse(struct c_node_val_float *val,char *str);
extern void iconst_parse(struct c_node_val_int *val,char *str,const unsigned int base);
extern void iconst_parse_char(struct c_node_val_int *val,char *str);

extern void yyerror(const char *);  /* prints grammar violation message */

extern int sym_type(const char *);  /* returns type from symbol table */

#define sym_type(identifier) IDENTIFIER /* with no symbol table, fake it */

static void comment(void);
static int check_type(void);
%}

%%
"/*"                                    { comment(); }
"//".*                                    { /* consume //-comment */ }

"auto"                  { return(yylval.node.token=AUTO); }
"break"                 { return(yylval.node.token=BREAK); }
"case"                  { return(yylval.node.token=CASE); }
"char"                  { return(yylval.node.token=CHAR); }
"const"                 { return(yylval.node.token=CONST); }
"continue"              { return(yylval.node.token=CONTINUE); }
"default"               { return(yylval.node.token=DEFAULT); }
"do"                    { return(yylval.node.token=DO); }
"double"                { return(yylval.node.token=DOUBLE); }
"else"                  { return(yylval.node.token=ELSE); }
"enum"                  { return(yylval.node.token=ENUM); }
"extern"                { return(yylval.node.token=EXTERN); }
"float"                 { return(yylval.node.token=FLOAT); }
"for"                   { return(yylval.node.token=FOR); }
"goto"                  { return(yylval.node.token=GOTO); }
"if"                    { return(yylval.node.token=IF); }
"inline"                { return(yylval.node.token=INLINE); }
"int"                   { return(yylval.node.token=INT); }
"long"                  { return(yylval.node.token=LONG); }
"register"              { return(yylval.node.token=REGISTER); }
"restrict"              { return(yylval.node.token=RESTRICT); }
"return"                { return(yylval.node.token=RETURN); }
"short"                 { return(yylval.node.token=SHORT); }
"signed"                { return(yylval.node.token=SIGNED); }
"sizeof"                { return(yylval.node.token=SIZEOF); }
"static"                { return(yylval.node.token=STATIC); }
"struct"                { return(yylval.node.token=STRUCT); }
"switch"                { return(yylval.node.token=SWITCH); }
"typedef"               { return(yylval.node.token=TYPEDEF); }
"union"                 { return(yylval.node.token=UNION); }
"unsigned"              { return(yylval.node.token=UNSIGNED); }
"void"                  { return(yylval.node.token=VOID); }
"volatile"              { return(yylval.node.token=VOLATILE); }
"while"                 { return(yylval.node.token=WHILE); }
"_Alignas"                              { return yylval.node.token=ALIGNAS; }
"_Alignof"                              { return yylval.node.token=ALIGNOF; }
"_Atomic"                               { return yylval.node.token=ATOMIC; }
"_Bool"                                 { return yylval.node.token=BOOL; }
"_Complex"                              { return yylval.node.token=COMPLEX; }
"_Generic"                              { return yylval.node.token=GENERIC; }
"_Imaginary"                            { return yylval.node.token=IMAGINARY; }
"_Noreturn"                             { return yylval.node.token=NORETURN; }
"_Static_assert"                        { return yylval.node.token=STATIC_ASSERT; }
"_Thread_local"                         { return yylval.node.token=THREAD_LOCAL; }
"__func__"                              { return yylval.node.token=FUNC_NAME; }

{L}{A}*                 { yylval.node.value.val_identifier=identifier_parse(yytext); return yylval.node.token=check_type(); }

{HP}{H}+{IS}?               { iconst_parse(&yylval.node.value.val_uint,yytext,16); return yylval.node.token=I_CONSTANT; }
{NZ}{D}*{IS}?               { iconst_parse(&yylval.node.value.val_uint,yytext,10); return yylval.node.token=I_CONSTANT; }
"0"{O}*{IS}?                { iconst_parse(&yylval.node.value.val_uint,yytext,8); return yylval.node.token=I_CONSTANT; }
{CP}?"'"([^'\\\n]|{ES})+"'"     { iconst_parse_char(&yylval.node.value.val_uint,yytext); return yylval.node.token=I_CONSTANT; }

{D}+{E}{FS}?                { fconst_parse(&yylval.node.value.val_float,yytext); return yylval.node.token=F_CONSTANT; }
{D}*"."{D}+{E}?{FS}?            { fconst_parse(&yylval.node.value.val_float,yytext); return yylval.node.token=F_CONSTANT; }
{D}+"."{E}?{FS}?            { fconst_parse(&yylval.node.value.val_float,yytext); return yylval.node.token=F_CONSTANT; }
{HP}{H}+{P}{FS}?            { fconst_parse(&yylval.node.value.val_float,yytext); return yylval.node.token=F_CONSTANT; }
{HP}{H}*"."{H}+{P}{FS}?         { fconst_parse(&yylval.node.value.val_float,yytext); return yylval.node.token=F_CONSTANT; }
{HP}{H}+"."{P}{FS}?         { fconst_parse(&yylval.node.value.val_float,yytext); return yylval.node.token=F_CONSTANT; }

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+   { yylval.node.value.val_string=sconst_parse(yytext); return STRING_LITERAL; }

"..."                   { return yylval.node.token=ELLIPSIS; }
">>="                   { return yylval.node.token=RIGHT_ASSIGN; }
"<<="                   { return yylval.node.token=LEFT_ASSIGN; }
"+="                    { return yylval.node.token=ADD_ASSIGN; }
"-="                    { return yylval.node.token=SUB_ASSIGN; }
"*="                    { return yylval.node.token=MUL_ASSIGN; }
"/="                    { return yylval.node.token=DIV_ASSIGN; }
"%="                    { return yylval.node.token=MOD_ASSIGN; }
"&="                    { return yylval.node.token=AND_ASSIGN; }
"^="                    { return yylval.node.token=XOR_ASSIGN; }
"|="                    { return yylval.node.token=OR_ASSIGN; }
">>"                    { return yylval.node.token=RIGHT_OP; }
"<<"                    { return yylval.node.token=LEFT_OP; }
"++"                    { return yylval.node.token=INC_OP; }
"--"                    { return yylval.node.token=DEC_OP; }
"->"                    { return yylval.node.token=PTR_OP; }
"&&"                    { return yylval.node.token=AND_OP; }
"||"                    { return yylval.node.token=OR_OP; }
"<="                    { return yylval.node.token=LE_OP; }
">="                    { return yylval.node.token=GE_OP; }
"=="                    { return yylval.node.token=EQ_OP; }
"!="                    { return yylval.node.token=NE_OP; }
";"                 { return yylval.node.token=';'; }
("{"|"<%")              { return yylval.node.token='{'; }
("}"|"%>")              { return yylval.node.token='}'; }
","                 { return yylval.node.token=','; }
":"                 { return yylval.node.token=':'; }
"="                 { return yylval.node.token='='; }
"("                 { return yylval.node.token='('; }
")"                 { return yylval.node.token=')'; }
("["|"<:")              { return yylval.node.token='['; }
("]"|":>")              { return yylval.node.token=']'; }
"."                 { return yylval.node.token='.'; }
"&"                 { return yylval.node.token='&'; }
"!"                 { return yylval.node.token='!'; }
"~"                 { return yylval.node.token='~'; }
"-"                 { return yylval.node.token='-'; }
"+"                 { return yylval.node.token='+'; }
"*"                 { return yylval.node.token='*'; }
"/"                 { return yylval.node.token='/'; }
"%"                 { return yylval.node.token='%'; }
"<"                 { return yylval.node.token='<'; }
">"                 { return yylval.node.token='>'; }
"^"                 { return yylval.node.token='^'; }
"|"                 { return yylval.node.token='|'; }
"?"                 { return yylval.node.token='?'; }

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

