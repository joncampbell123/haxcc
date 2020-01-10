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
Aaaaaaa
