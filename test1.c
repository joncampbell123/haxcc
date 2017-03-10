
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

enum enumnothing;
enum {
    joe,
    bob,
    harry,
    jerkface
};
enum enumnothing {
    blah1,
    blah2,blah3
};
enum {
    joe2 = 9,
    bob2 = 99999,
    harry2 = 111,
    jerface2 = 11111,
    fromanotherenum = joe2, /* = 9 apparently it's legal (in GCC) to declare an enum const from another enum const */
    thenanother
};
enum {
    thisenum,
    willkill = 3333,
    your,
    ocd, /* <- see? */
};
enum named_enum {
    asdf,
    qwerty = 333,
    keeeeeeee,
    qweeeeee = 0x10000,
    ppppppp,
    fromexpr1 = (9),
    fromexpr2 = (qweeeeee),
    fromexpr3 = ((((((((((99999)))))))))),
    fromexpr4 = 123000 + 456,
    fromexpr5 = (1230000 + 4500 + 60) + 7,
    fromexpr6 = ((qwerty + 222000) + 1000000) + 990000000,
    fromexpr7 = (999999 - 12345),
    fromexpr8 = (223 - 23) - 100,
    framexpr9 = (9 * 111111111),
    framexpr10 = (99999999 / 9),
    framexpr11 = (4 + (99 - 11) + willkill) * (4 + 1 + 1 - 1 * 5),
    framexpr12 = (4 + (99 - 11) + ocd + willkill * your) * (4 + 1 + 1 + (2 + 2 * 6)) /* deliberate mistake, to test eval */
};
enum named_enum_trailing {
    wowoiqwr = 22222,
    wrqqqqq,
    wpwpwpwsks = 0x40000,
    qjfjajrja = 0777,
    ocd_trigger,
};

void functiondef1() {
}

void functiondef2(void) {
}

void functiondef3(int a,const char * const b,const float) {
}

void oldfunctiondef1(a)
int a;
{
    return;
}

long oldfunctiondef2(a,b,c,d,e,f,g,h)
long a;
int b,g,h;
float c,f;
char d,e;
{
    return a + b - c;
}

long oldfunctiondef3() {
    return(99);
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

void funcwithparamsarrays(int a[],char b[5],char *c[17]) {
    /* illegal, for testing */
    goto label1;
    continue;
    goto label2;
    break;
label1:
    a[4] = 6;
label2:
    a[3] = 7;
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

        {
            int x[qweeeeee];
            int abc_init_array[] = {1, 2, 3, 4, 5, -1, -2, -3, -4, -5, (char)99, (char)(long)1111};
            int abc_init_array_ocd[] = {1, 2, 3, 4, 5, -1, -2, -3, -4, -5, (char)99, (char)(long)1111,};
            /* not legal, but for testing */
            int layers1[] = {1,2,3,4,5,{6,7,8,9},10,11,12,13,14,{15,16,{17,{18,19}}}};
            int desigarray[] = {1, 2, [4]=4, 5, 6, 7, [8][8]=8}; /* <- C11 designators */
            int desigarray2[] = {1, 2, [4]=7, 2, 1, .member=4}; /* <- C11 designators */
            _Atomic(int) val1,val2;
            _Atomic int val3;
        }
    }

    {
        int a = 1 | 2,b = 1 || 2,c = 1 ^ 3,d = 3 & 2,e = 1 && 1,f = 1 == 1,g = 1 != 1,
            h = 1 >= 1,i = 1 <= 1,j = 1 > 1,k = 1 < 1,l = 1 >> 1,m = 1 << 1;
        char aa = array1[0],bb = array1[32] + array1[16];
        char cc = array1[aa] + array[aa+2] - array[aa*3];
        char dd = array1[3 + array1[45] - array1[3]];
        char ee = struc.val,ff = struc.mem.val.blah.ea;
        char gg = sp->val.blah,hh = sp->val->a->b->c->d->f->g;
        char hh = spa.bla[i]->asdfqwerty.qwertyuiop[a][b]->b->c;
        char ii = ((long*)xps)[j];
        char jj = _Alignof(int);
        char _Alignas(int) kk = 55;
        char _Alignas(8) ll = 111;

        a  = b + c;
        a += 1;
        b += a;
        c += (a + b * 3 - 5);
        d -= 3 + a;
        e -= 5;
        f *= 2;
        g /= 5;
        h %= 3;
        i &= 3;
        j ^= 1;
        k |= 1;
        a <<= 1;
        b >>= 2;
        a <<= c + d;
        d >>= a + c - 3;

        while (c > 500)
            c--;

        while (j < 1000) {
            j += k;
            k += 1 + (1 << k);
        }

        do {
            a /= 2;
            b -= 4;
        } while ((a+b) > c);

        for (k=6;k < 9;) {
            j = k + 1;
        }

        for (k=6;k < 9;k++) {
            j = k + 2;
            l = j * 3;
        }

        for (int c=5;c < 99;) {
            d = c * 6 + 3;
        }

        for (int c=5;c < 99;c += 3) {
            d = c * 6 + 5;
            e = d - 3;
        }

        /* NTS: This is obscure, but the comma ',' operator causes the evaluation of both sides then
         *      discards the first, returns the second */
        {
            long x = *functhatdoesthings2(&a,p);
            a = (b + c, b = c + 3, g + h, 4, 6 + h); /* result should be 6 + h after evaluating everything else including b = c + 3 */
            *p = (char)(a + b);

            {
                int ternary1 = (a == 5 ? 9 : a + b);
                int ternary2 = (a == (b == 7 ? a + c : a + d) ? i : (j == k ? 11 : 4 + c));
            }
        }

        if (0) {
            int this_will_never_happen = 1;
        }
        else {
            int this_will_always_happen = 1;
        }

        if (a == b) {
            int c = a + b;
        }

        if (a == b) {
            int c = a + b;
        }
        else if (c == d) {
            int d = c + d;
        }
        else if (e == f) {
            int e = a + b;
        }
        else {
            int f = a + b;
        }

        if (a == b) {
            a += b;
        }
        else if (c == d) {
            c += d;
        }
    }

    switch (c) {
        case asdf:
            break;
        case qwerty:
            break;
        case keeeeeeee:
            break;
        case qweeeeee:
            break;
        case ppppppp:
            break;
    }

    switch (a) {
        case 1:
            b = 5;
            break;
        case 4:
            b = 99;
            break;
        case 7:
            b = 111;
            break;
        default:
            b = -1;
            break;
    };

    /* TEST: we declared an enum 'joe' 'bob' etc. this test is whether the code remembers that they are enum constants */
    {
        int x = joe + 5;
        int y = joe + bob * 3;
        int sarrr[joe+bob];
        void *xptraa = (int){1, 2, 3, 4};
        void *xp2 = (long){3, 1, -1, 4, 2, 1, (char){'a', 'b', 'c'}, 3, 1};
        void *xp3 = (int){1, 1, 1, 1, 1,};
        /* invalid, for testing */
        {
            int typecastarrayabs[] = (int[])y;
            int typecastarrayabs2[] = (int[][])y;
            int typecastarrayabs2x[] = (int[][64])y;
            int typecastarrayabs3[] = (int[32][64])y;
            int typecastarraya = (int[]())y;
            int typecastarraya = (int[](int,long,float)(int))y;
        }

        _Static_assert(1 == 1,"Hello");
        _Static_assert((1 + 1) == 2,"AAAA");
    }
}

struct somestruct;

struct {
    int         x;
    int         y;
};

struct somestruct2 {
    int         x,a;
    int         y;
    int         z;
};

struct somestruct3 {
    int         y;
    int         :4;
    int         x:3;
};

union {
    int         x;
    char        y;
};

struct xyz {
    int         x;
    int         y,z;
    char        aaa:3;
} aaa;

struct xyz {
    int         c;
    char        x,y,z;
    struct abc {
        int     z:4;
        char    a,b,c,d;
        struct hww {
            char    u,w,x;
        };
    };
    struct xyz2 {
        int     q,w:2,e,r:1;
    };
};

typedef unsigned int uitype;
typedef struct xyz xyztype;
typedef float fltype1,fltype2;

typedef int uiblob[32];
typedef int uiblobblob[16][16];
typedef int *intptrwhat;
typedef int **intptrptr;
typedef int *intptrarray[32];
typedef int (*funcptr1_t)();
typedef int (*funcptr2_t)(int,long,float,char*);
typedef int (*funcptrs1_t[32])(int,long,float,char*);

void functt(const uitype x) {
    funcptr1_t f1;
    funcptr2_t f2;
    funcptrs1_t ft1;
    intptrarray whatpa;
    intptrptr whatptr;
    intptrwhat what;
    uiblobblob uu;
    xyztype xyz;
    uiblob xx;

    fltype1 t[5];
    t[0] = (fltype2)x;
    xyz.c = (int)x;
}

