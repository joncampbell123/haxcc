
extern int duplicate;

int parens1 = (5);
int parens2 = ((23));
int parens3 = (11+22);
int parens4 = (11+(22+33));
int xyz,abc,faa,baa = 10 * 0x111,wwwwwww;
static long longcat;
float floatey;
double doubleback;
long long longlongcat = 0x999 / 0x9;
long double doublelong;
int abcd = 0x1234 + 0x1111;
int efgh = 0x1234 - 0x1111;
int morecomplex = 10000 + 2000 + 300 + 40 + 5 - 11111;
int modu = 12345 % 100;
int shifty = 4 << 4; /* = 4 * 16 = 64 */
int shift2 = 4 >> 2; /* = 4 / 4 = 1 */
/*extern register er_contradiction;*/
/*extern static es_contradiction;*/
int duplicate = 12345;
/*int duplicate = 12345;*/
/*int dividebyzero = 12345678 / 0;*/

extern int duplicate;

/*extern int duplicate = 12345678;*/

long function0() {
    int olddup = duplicate;
    int duplicate = 12345; /* shadow */
}

void function1(void) {
    long whatwhat = (unsigned int)((char)0xFF);
    long what2 = (char)0xFF; /* should sign extend to 0xFFFFFFFFFFFFFFFFULL */
    int what = (unsigned long)0x4444444ULL;
    int shift1 = -1 >> 4; /* should be -1 */
    int shift2x = ~0 >> 4; /* should be -1 */
    int shift2y = ~0U >> 4U; /* should be ~0 >> 4U. in our compiler, 0x0FFFFFFFFFFFFFFFULL */
    int negnegdiv = -4 / -1; /* = 4 */
    int unegdiv = 4U / -1; /* = -4 */
    int val = -1;
    int val2 = 0x111 + -1;
    int val3 = ~0;
    unsigned long long typecastia = (unsigned char)((long)((unsigned short)((char)((unsigned char)((unsigned int)val)))));
    long vall = (unsigned long)((char)val);
    int a = 2 * 4 * 8 * 16;
    int x = 246,xxx = 0x777;
    int y = x,yy = x,zzz = 0x1234,yyy = x;
    int z = notdefined;
    const unsigned long xx = 0x12345678UL;
    const unsigned long long m = 0x123456789ABCDEFULL;

    {
    }

    {
        int x = 123;
        long y = 0x1111UL;
    }
}

void function2(int x,long y,char) {
    int a = (int)y;
    long b = (long)x;
    int pabcd = abcd; /* <- ID should refer to global */
    int abcd = 0x1234; /* <- deliberate shadow */
    int refabcd = abcd; /* <- ID should refer to shadow, not global */
    /* abcd shadow in this scope should be deleted. reference should refer to global again */
}

char function3(int a,long b,...) {
    long c = 444L;
    int refabcd = abcd; /* <- ID should refer to global */
}

long oldstyle_function(a,b,c)
int a;
long b;
char c;
{
    long x = a;
    long y = b;
    long a = b; /* <- ref to "b" should show "b" with same ID */
}

long func_decl1();
int func_decl2(int a,long b);
int func_decl2(int a,long b); /* duplicate declaration, not definition. not an error */

/* this is legal apparently, even GCC accepts it, though it shows a warning */
void func_oldstyle1(a,b,c);

