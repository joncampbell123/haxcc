
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
unsigned long long longconst;
signed long long longconst2;
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
//static const static static restrict static static restrict static extern const unsigned unsigned const extern const short static long int static red = 1;
//static extern register const restrict volatile int illegal_combo = 5;
//char * const volatile restrict const volatile restrict const volatile restrict * const volatile const volatile * restrict * const * const * const * const restrict * pp3;
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
    framexpr12 = (4 + (99 - 11) + ocd + willkill * your) * (4 + 1 + 1 + (2 + 2 * 6)), /* deliberate mistake, to test eval */
    framexpr13 = 1U << 8U,
    framexpr14 = (1U << 12) + (3U << 8) + (7U << 2U << 2U),
    framexpr15 = 256U >> 8U,
    framexpr16 = (256U >> 2U >> 2U),
    framexpr17 = -9,
    framexpr18 = -((3 - 2)),
    framexpr19 = 5 + (-(2 + 2)),
    framexpr20 = +5,
    framexpr21 = 5 + +5,
    framexpr22 = ~0,
    framexpr23 = ~~1,
    framexpr24 = (~1 + 1),
    framexpr25 = ~~~~~((~2) + 1),
    framexpr26 = ((7 % 4) + 7),
    framexpr27 = 7 % 4 % 2,
    framexpr28 = !0,
    framexpr29 = !1,
    framexpr30 = !(1 + 1 + 1 + 1),
    framexpr31 = (3 == 3),
    framexpr32 = (3 == 4),
    framexpr33 = !(3 == 4),
    framexpr34 = (3 != 4),
    framexpr35 = (3 != 3),
    framexpr36 = !(3 != 3),
    framexpr37 = (3 > 2),
    framexpr38 = (2 > 3),
    framexpr39 = (-2 > -3), /* which is true! */
    framexpr40 = (-3 > -2), /* which is false */
    framexpr41 = (2 < 3),
    framexpr42 = (3 < 2),
    framexpr43 = (-3 < -2), /* which is true! */
    framexpr44 = (-2 < -3), /* which is false */
    framexpr45 = (3 >= 2),
    framexpr46 = (2 >= 3),
    framexpr47 = (-2 >= -3), /* which is true! */
    framexpr48 = (-3 >= -2), /* which is false */
    framexpr49 = (2 <= 3),
    framexpr50 = (3 <= 2),
    framexpr51 = (-3 <= -2), /* which is true! */
    framexpr52 = (-2 <= -3), /* which is false */
    framexpr53 = (1 & 1),
    framexpr54 = (7 & 4),
    framexpr55 = (7 & 1 & 1 & 1 & 1 & 1),
    framexpr56 = (1 | 2 | 4),
    framexpr57 = (7 ^ 2 ^ 4),
    framexpr58 = (((5 + 2) ^ 2 ^ 4) + 2) + 1, /* = 4 */
    framexpr59 = (1 && 1),
    framexpr60 = (1 && 0),
    framexpr61 = (0 && 1),
    framexpr62 = (0 && 0),
    framexpr63 = (1 || 1),
    framexpr64 = (1 || 0),
    framexpr65 = (0 || 1),
    framexpr66 = (0 || 0),
    framexpr67 = ((1 + 1) == 2 ? 88 : 44),
    framexpr68 = ((1 + 1) == 3 ? 88 : 44),
    framexpr69 = 4 + ((1 + 1) ? (2 ? 40 : 20) : (0)),
    framexpr70 = (char)1,
    framexpr71 = (unsigned char)1,
    framexpr72 = (int)(char)1,
    framexpr73 = (char)255, /* should become 0xFFFFFFFFFFFFFFFF */
    framexpr74 = (unsigned char)255,
    framexpr75 = (short)65535, /* should become 0xFFFFFFFFFFFFFFFF */
    framexpr76 = (unsigned short)65535,
    framexpr77 = (unsigned char)0x101,
    framexpr78 = (unsigned short)0x10001,
    framexpr79 = (int)0xFFFFFFFF, /* should become 0xFFFFFFFFFFFFFFFF */
    framexpr80 = (unsigned int)0xFFFFFFFF,
    framexpr81 = (signed short int)65535,
    framexpr82 = (unsigned long long)1,
    framexpr83 = (unsigned long)1
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

int enumref1 = willkill;
int doublenegative3x = *(xx + (5 + 5));

unsigned int unarystuff(unsigned int a,unsigned int b) {
    static int alloc_me_in_data;
    int enumref1 = willkill;
    unsigned int c = ~a,cc = ~~a;
    unsigned int *addr = &a;
    unsigned int aaa = +a;
    unsigned int bbb = -a + b;
    unsigned int notty = !a,notty2 = !!a,notty3 = !!!a,notty4 = !!!!a;
    /* NTS: We expect ! to result in a boolean, even that !!2 == 1 and !!!2 == 0 */
    unsigned int noctty = !0,noctty2 = !!0,noctty3 = !!!0,noctty4 = !!!!0,noctty5 = !!!!!0,noctty6 = !!!!!!0,noctty7 = !2,noctty8 = !!2,noctty9 = !!!2, noctty10 = !!!!2;
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
        int enumref1 = willkill;
        int enumref2[willkill];
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
        char arrayrefexpr1 = array1[(8*4)+1];
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
                int ternary3 = (((3 + 3 + 3) ? (2 + 2 + 2) : (1 + 1 + 1)) ? 2 : 1);
                static int allocmedeeper;
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

        {
            int addsortme0 = a + b + a + b + a + b + 4 + a + b + a + 3 + b + a + b;
            int addsortme0a= 1 + 2 + 3 + 4 - 1;
            int addsortme0b= a + b + a + b + a + b + (4 + 3 + 7) + a + b + a + 3 + b + a + b;
            int addsortme0c= a + b + a + b + a + b + 4 + 3 + 2 + 6 + 3 + a + b + a + 2 + 1 + 8 + a + 6 + b + 3;
            int addsortme0d= 2 + a + b + a + b + a + b + 4 + 3 + 2 + 6 + 3 + a + b + a + 2 + 1 + 8 + a + 6 + b + 3;
            int addsortme1 = a + b + a + b + a + b + a + b + a + b + a + b;
            int addsortme1a= a + a + b + b + a + b + a + b + a + b + a + b;
            int addsortme1b= a + b + c + a + b + c + a + b + c;
            int addsortme1c= a + b + c + a + b + c + a + b + c + a + b + c + a + b + c;
            int addsortme1d= a + b + c + d + a + b + c + d + a + b + c + d + a + b + c + d + a + b + c + d;
            int addsortme1e= a + b + c * d + a + b + c + d + a * b + c + d + a + b + c + d + a + b + c + d;
            int addsortme1f= a + b + c * (d + e * f + d + e) * f + a + b + b + a;
            int addsortme2 = a + b - c + a + b - c + a + b - c;
            int addsortme2b= a + b + b + a + a + b - c + b + a + b + a;
            int addsortme2c= a + b + b + a + a + b - c + b + a + b + a + b + b + a + a + b + a + b + a - c + a + b + b + a + b + a;
            int addsortme3 = a + b - b - a + a + b - b - a;
            int addsortme3b= (a + b - b - a + b + a + b - b - a) + a - b;
            int addsortme3c= a + b + a - b * a + a - b - a + b - a;
            int addsortme3d= a + b + a + b * a + a - b - a + b - a;
            int addsortme3e= a + b + a + b * a + a - b - a * b - a;
            int addsortme4 = (a + b + a) + b + (a + b) + a + b;
            int addsortme4b= (a + b + c + a) + b + c + a + b + (c + a + b + (c + a + (b + c) + a + b) + c + a) + b + c;
            int addsortme5 = a + 0;
            int addsortme5b= a + b + c + 0;
            int addsortme5c= a + b + 0 + a + b + 0 + c + 0 + 0 + a;
            int addsortme5d= ((a + b + 0 + a) + 0 + a) + 0 + 0 + 0 + 0 + a;
            int addsortme5e= 0;
            int addsortme5f= a + b + 0.0 + a + b + 0 + 0.0 + 0.0 + a + b + 0;
            int subsortme1 = a - b - a - b - b - a - b - a - b - a;
            int subsortme1b= a - b - a - b + b - a - b + a - b - a;
            int subsortme1c= c - a - b - b - a - b - a - c - c - b - a;
            int subsortme1d= c - b - a - b * a - b - a - b - c - a - c;
            int subsortme1e= a - b - a - b + b - a - b + a - b - a + a + b - a + b;
            int subsortme1f= a - b - a * b + b - a - b + a - b * a + a + b - a + b + b * a + b + a + b - a - b - a - a * b - a - b - a - b - a - b;
            int subsortme1g= a - b - a - (b - a - b - a) - b - a - b - (a - b);
            int subsortme1h= c - b - a - b * a - b - a * b - c - a - c;
            int subsortme1i= a - b - 0 - a - b - 0 - 0 - a - 0 - a;
            int subsortme1j= a - b - 0.0 - a - b - 0.0 - 0.0 - a;
            int subsortme1k= 0 - a;
            int mulsortme1 = a * b * a * b * a * b * a * b;
            int mulsortme1a= a * a * b * b * a * b * a * b;
            int mulsortme1b= a * a * b * b * a * c * d * b * c * c * d * a * c * d * b;
            int mulsortme1c=(a * b * c * a) * b * c * a * (b * c * a * b * (c * a * (b * (c * a * b))) * c) * a * b * c;
            int mulsortme1d= a * 0 + b * 0 + c * 0;
            int mulsortme1e= a * 0 + a + b * c * 0 - b - a;
            int mulsortme1f= ((a + b + c + b + a + 4 + 1) * 0) + a + b + c * 0 - b - a;
            int mulsortme1g= a * -1 + b * -1 + c * -1;
            int mulsortme1h= -1 * a + -1 * b + -1 * c;
            int mulsortme1i= -1 * a + -1 * b + -1 * c - -1 * d - -1 * e;
            int mulsortme1j= a * -1;
            int combsortme0= a + b + b + a + b + a + b + a * b + a + b + a + b + c + a + b + a + b + a;
            int combsortme0b=a + b + b + a * b + a + b + a * b + a + b + a + b + c * a + b + a + b + a;
            int combsortme1=(a + b + b + a + b + a + b + a) * (b + a + b + a + b + c + a + b + a + b + a);
            int combsortme2= a + b + b + a + a + b + a + b + a * c * a * c * b;
            int combsortme3= a + b + b + b + a + b + a * b * a * b * a * b * a;
            int combsortme4= a + b + b + a + a + b + a + b + a * b + a + b + b + a + b + a;
            int combsortme4b=a + b + b + a + a + b + a + b + a * b * b * a * b + a + b + b + a + b + a;
            int combsortme5= a * b * a * b * a / b * a * b * a * b / a * b;
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
            c = 7;
            break;
        case 4:
            b = 99;
            c = 9;
            d = 2;
            break;
        case 7:
            b = 111;
            c = 22;
            break;
        default:
            b = -1;
            break;
    };

    /* TEST: we declared an enum 'joe' 'bob' etc. this test is whether the code remembers that they are enum constants */
    {
        int x = joe + 5;
        int y = joe + bob * 3;
        int sarrr[joe+bob+harry];
        void *xptraa = (int){1, 2, 3, 4};
        void *xp2 = (long){3, 1, -1, 4, +2, 1, (char){'a', 'b', 'c'}, 3, 1};
        void *xp3 = (int){1, 1, 1, 1, 1,};
        /* invalid, for testing */
        {
            int *xx = &y;
            int typecastarrayabs[] = (int[])y;
            int typecastarrayabs2[] = (int[][])y;
            int typecastarrayabs2x[] = (int[][64])y;
            int typecastarrayabs3[] = (int[32][64])y;
            int typecastarraya = (int[]())y;
            int typecastarraya = (int[](int,long,float)(int))y;
            int doublenegative0 = ~~(1 + 1);
            int doublenegative1 = ~~(x + y);
            int doublenegative1b = ~~~(x + y);
            int doublenegative2 = ~~~~(x + y + ~~(5 + 5 + 5 + (y * (3 + 3))));
            int doublenegative3 = ~~~~~~~~(x + y + ~~(5 + 5 + 5 + *(xx + (5 + 5))));
            int doublenegative3x = ~~~~~~~~(x + y + ~~(5 + (5 + 5 + 5 + (3 + 3 + 3)) + 5 + 5 + *(xx + (5 + 5))));
            int doublenegative4 = -(-1);
            int doublenegative5 = -(-(-1));
            int doublenegative6 = -(-(-(-(-(-(-(-(-(-(-(-(-(-(-(-(-(-1)))))))))))))))));
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

uitype ui_blah;

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

    float floatexpr1 = 3.1 + 0.9;
    float floatexpr1b = 3.1f + 0.9f; /* result should be float */
    float floatexpr1c = 3.1f + 0.9; /* should promote result to double */
    float floatexpr1d = 3.1f + 0.9l; /* should promote result to long double */
    float floatexpr2 = (3.1 + 0.9) + ((4.0 + 4.0) - 7.0); /* =5.0 */
    float floatexpr3 = ((10.0 - 3.3) + 0.3 - 6.0) - 0.5; /* =0.5 */
    float floatexpr4 = -9.0;
    float floatexpr4b = 0.0f + 1;
    float floatexpr4c = 0.0f + 0xFFFFF;
    float floatexpr4d = 0.0f + 0xFFFFFF;
    float floatexpr4e = 0.0f + 0xFFFFFFFF;
    float floatexpr4f = 0.0f + 0xFFFFFFFFFFFFFFF;
    float floatexpr4g = 1 + 0.0f;
    float floatexpr4h = 0xFFFFF + 0.0f;
    float floatexpr4i = 0xFFFFFF + 0.0f;
    float floatexpr4j = 0xFFFFFFFF + 0.0f;
    float floatexpr4k = 0xFFFFFFFFFFFFFFF + 0.0f;
    float floatexpr5 = 1.5 + 1; /* float + int = 2.5 */
    float floatexpr5b = 1.5 + -1; /* float + -int = 0.5 */
    float floatexpr5c = 1.5 - 1; /* float - int = 0.5 */
    float floatexpr6 = (1.5 * 3) + 2.5; /* 4.5 + 2.5 = 7.0 */
    float floatexpr7 = (1.5 * -3) + 2.5; /* -4.5 + 2.5 = -2.0 */
    float floatexpr8 = (9.0 / 3) + 2; /* 3.0 + 2 = 5.0 */
    float floatexpr9 = (9.0 / 2 / 3) + 0.5; /* 1.5 + 0.5 = 2.0 */
    float floatexpr10 = 5.0 + +5;
    float floatexpr10b = 5.0 + +5.0;
    float floatexpr11 = 4.3 > 4.0;
    float floatexpr11b = 4.3 > 4;
    float floatexpr11c = 4 > 3.3;
    float floatexpr11d = 3.3 > 4.0;
    float floatexpr11e = -4.3 < -4.0;
    float floatexpr11f = -4.3 < -4;
    float floatexpr11g = -4 < -3.3;
    float floatexpr11h = -3.3 < -4.0;
    float floatexpr11i = 4.3 == 4.3;
    float floatexpr11j = 4.3 == 4.29;
    float floatexpr11k = 4.3 != 3;
    float floatexpr11l = 4.3 != 4.29;
    float floatexpr11m = 4.3 != 4.3;
    float floatexpr11n = 4.3 >= 4.0;
    float floatexpr11o = 4.3 >= 4;
    float floatexpr11p = 4 >= 3.3;
    float floatexpr11q = 3.3 >= 4.0;
    float floatexpr11r = -4.3 <= -4.0;
    float floatexpr11s = -4.3 <= -4;
    float floatexpr11t = -4 <= -3.3;
    float floatexpr11u = -3.3 <= -4.0;
    float floatexpr11v = (float)4;
    float floatexpr11w = (double)4;
    float floatexpr11x = (long double)4;
    float floatexpr11y = (float)(long double)4;
    float floatexpr12a = !0.0;
    float floatexpr12b = !1.0;
    float floatexpr12c = !(1.0 - 1.0);
    float floatexpr12d = !(1.0 + 1.5 - 0.5);
    float floatexpr12e = 4 ? 2 : 1;
    float floatexpr12f = 4 ? 9.9 : 3.3;
    float floatexpr12f2= 0 ? 9.9 : 3.3;
    float floatexpr12g = 4.0 ? 9.9 : 3.3;
    float floatexpr12h = 0.0 ? 9.9 : 3.3;
    float floatexpr12i = (1.0 || 0.0);
    float floatexpr12j = (0.0 || 1.0);
    float floatexpr12k = (0.0 || 0.0);
    float floatexpr12l = (1.0 || 0.0) ? 4.1 : 1.5;
    float floatexpr12m = (0.0 || 0.0) ? 4.1 : 1.5;
    float floatexpr12n = (1.0 && 0.0);
    float floatexpr12o = (0.0 && 1.0);
    float floatexpr12p = (0.0 && 0.0);
    float floatexpr12p2= (1.0 && 1.0);
    float floatexpr12q = (1.0 && 0.0) ? 4.1 : 1.5;
    float floatexpr12r = (0.0 && 0.0) ? 4.1 : 1.5;
    float floatexpr12s = (1.0 && 1.0) ? 4.1 : 1.5;
    float floatexpr12t = (0 || 0.5) ? 4.1 : 1.5;
    float floatexpr12u = (1 && 0.5) ? 4.1 : 1.5;

}

