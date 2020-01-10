BBBBB
#include "test.h"
Aaaaa
#define HELLO0
#define HELLO1()
#define HELLO2(x) (x)
#define HELLO3(xx,yy) (xx) (yy)
#define HELLO4(x,y,z) (x) (y) (z)
#define HELLO5(xx,yy,zzzz,...) (xx) (yy) (zzzz)
#define HELLO6(xx,yy,zzzz,...) (xx/3) (2,yy )   (      zzzz )
#define HELLO7(xx,yy,zzzz,...) (zzzz,yy,xx,xxx)
#define HELLO8(xx,yy,zzzz,...) (zzzz,xx,yy,xxx)
#define HELLO9(xx,yy,zzzz,...) (zzzz,__VA_ARGS__,xx,yy,xxx)
#define HELLO10(xx,yy,zzzz,...) (zzzz,__VA_ARGS__,xx,#yy,xxx)
#define HELLO11(xx,yy,zzzz,...) (zzzz,__VA_ARGS__,xx##yy##zzzz,#yy,xxx)
#define HELLO12(xx,yy,zzzz,...) (zzzz,__VA_ARGS__,#xx #yy #zzzz,#yy,xxx)
#define HELLO13 WORLD
#undef  HELLO13
#define HELLO13 WORLD HELLO
#define HELLO14 MULTIPLE \
    LINE SUPPORT \
    HERE
#define HELLO15(xx,yy,...) xx,yy __VA_OPT__(,xx)
#ifdef NOT
Should not see, no such NOT
#endif
#ifdef HELLO0
Hello0 defined, should see
#endif
#ifndef NOT
Should see, no such NOT
#endif
#ifndef HELLO0
Hello0 defined, should not see
#endif
#ifdef NOT
Should not see, no such NOT
#ifdef HELLO0
Hello0 defined, no such NOT, should not see
#endif
Still should not see, no such NOT
#endif
#ifdef HELLO0
Should see, hello0 defined 1
#ifdef NOT
Should not see, no such NOT, though hello0 defined
#ifdef HELLO13
Should not see, hello13 defined but no such NOT
#endif
#endif
Should see, hello0 defined 2
#endif
#ifdef HELLO0
Should see, hello0 defined
#else
Should not see, hello0 defined else condition
#endif
#ifdef NOT
Should not see, no such NOT, #if..#else case
#else
Should see, no such NOT #else condition
#endif
#ifdef NOT
Should not see, no such NOT
# ifdef HELLO0
Should not see, no such NOT but hello0 defined
# else
Should not see, no such NOT and not defined hello0
# endif
#else
Should see, no such NOT
# ifdef FOO
Should not see, no such NOT and no such FOO
# else
Should see, no such NOT and no such FOO
# endif
#endif
Aaaaaaa
