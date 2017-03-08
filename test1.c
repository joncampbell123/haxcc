
char char1 = 'a';
int char2 = 'x';
int char3 = 'RIFF';
long multiples1 = 4,multiples2 = 99,multiples3,multiples4 = 77;
char *str1 = "string1";
char *str2 = "string 2 very long blah blah";
const char *strlong = "constant string";
static const signed long int combot1 = 444L;
int abc = 123;
int abcnoinit;
float float1 = 1.0f;
float float2 = 5.0f;
double double1 = 1.0;
double double2 = 5.0;
long double longdouble1 = 1.0l;
long double longdouble2 = 5.0l;
unsigned int abcu = 244U;
const unsigned int caca = 1234U;
static signed short int blah = 1222;
long def = 12345678UL;
static const
    unsigned long
        i_like_to_span_lines =
            12345678UL;
int expressions1 = (4 + 3);
long expressions2 = (1 + 3 + 5 + 7 + 9 + 11 + 13 + 15);
unsigned long expressions_with_nested_parens = (((4) + 3 + 2) + 1);
int expressions_with_identifiers1 = (1 + 3 + 5 + caca + 4 + caca) - 99 - caca;
long multipadd1 = ((4 + 3 * 7) / 9) - (99 % 4);
char **********************************pp1;
char * const * const * const * const * const * const * const *pp2;

int typc1 = (int)(long)(char)19;
int typc2 = (int) ((long) ((char)45));
int typc3 = (int)(long long unsigned int)(signed short int)99;
int typc4 = (int)(long*)(char****)(const char * const **)999;

/* clearly illegal, but here to test declspec / typespec combining */
static const static static restrict static static restrict static extern const unsigned unsigned const extern const short static long int static red = 1;
static extern register const restrict volatile int illegal_combo = 5;
char * const volatile restrict const volatile restrict const volatile restrict * const volatile const volatile * restrict * const * const * const * const restrict * pp3;
/* ------ */

char array0[];
char array1[64];
char array2[*]; /* <- FIXME: what does this do in C? */
char arraymd0[][];
char arraymd1[64][128];
char arraymd2[*][*];
char manyarrays[8][9][10][11][12][13];

int (apparently_this_is_valid); /* <- GCC likes it, apparently */
int ((apparently_so_is_this)); /* <- GCC doesn't complain */
int (*this_is_more_common);
int (***this_less_so);
int *(what_about_this);
int *(*or_this1);
int *(****(***(**(**about_this1))));
int (*stuff)[64];
int ((*stuff)[32])[64];

void function1();
int function2();
void oldcfunc0(param1,param2,param3,param4);
long oldcfunction(a,b,c,d,e,f,g);
long function3(void);
long function4(int,long,float);
long function5(int a,long b,float c);
long function6(int **a,long *b,const char * const c);
long function7(int **a,long *b,...);
long function8(int (*something),long (**(*something)));
int (*funcptr)();
int (*anotherfuncptr)(int,long,float,const char *s);
int (*anotherfuncptr2)(int a,long *b,float (*c)(int,long),const char * const d);

void functiondef1() {
}

void functiondef2(void) {
}

void functiondef3(int a,const char * const b,const float) {
}

void oldfunctiondef1(a) {
}

long oldfunctiondef2(a,b,c,d,e,f,g,h) {
}

void functhatdoesthings1(int a) {
    int blah = 45;
    float flah = 33.33;
    double dah = ((flah + blah) * 3) + 35;
}

long *functhatdoesthings2(int * const a,const char * const * const * const b) {
    int c = *a;
    char d = ***b;
}

unsigned int unarystuff(unsigned int a,unsigned int b) {
    unsigned int c = ~a,cc = ~~a;
    unsigned int *addr = &a;
    unsigned int aaa = +a;
    unsigned int bbb = -a + b;
    unsigned int notty = !a,notty2 = !!a;
    unsigned int sz1 = sizeof a;
    unsigned int sz2 = sizeof(a);
    unsigned int sz3 = sizeof(*addr);
    ++a;
    --b;
    ;;;;;
    a++;
    b++;
    /* illegal, for testing only */
    ++++++++a;
    a++++++++;
    ++--++--a;
    a--++--++;
    (((a++)--)++)--;
    (((a--)++)--)++;

    function1();

    {
        int a = 5;
        int b = ++a; /* b = 6 */
        int c = a++; /* c = 6, then a = 7 */
        char *p;
        char c = *p++;
        char d = *++p;
        int x = function2();
        int y = (*funcptr)();
        long xx = function4(1,2,3.333);
        long yy = function4(x,xx,float1);
        functiondef3(22222,&b,float2);
        functiondef3(22222,(char*)(&c),(float)xx);
    }
}

