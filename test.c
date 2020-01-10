BBBBB
#include "test.h"
Aaaaa
#define HELLO
#define HELLO()
#define HELLO(x) (x)
#define HELLO(xx,yy) (xx) (yy)
#define HELLO(x,y,z) (x) (y) (z)
#define HELLO(xx,yy,zzzz,...) (xx) (yy) (zzzz)
#define HELLO(xx,yy,zzzz,...) (xx/3) (2,yy )   (      zzzz )
#define HELLO(xx,yy,zzzz,...) (zzzz,yy,xx,xxx)
#define HELLO(xx,yy,zzzz,...) (zzzz,xx,yy,xxx)
#define HELLO(xx,yy,zzzz,...) (zzzz,__VA_ARGS__,xx,yy,xxx)
#define HELLO(xx,yy,zzzz,...) (zzzz,__VA_ARGS__,xx,#yy,xxx)
#define HELLO(xx,yy,zzzz,...) (zzzz,__VA_ARGS__,xx##yy##zzzz,#yy,xxx)
#define HELLO(xx,yy,zzzz,...) (zzzz,__VA_ARGS__,#xx #yy #zzzz,#yy,xxx)
#define HELLO WORLD
Aaaaaaa
