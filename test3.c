Hello world
#ifdef HELLO
In HELLO
#else
Else HELLO
#endif
Out of HELLO

#ifdef HELLO
Hello
# ifdef WORLD
Hello world
# else
Hello !world
# endif
~Hello
#else
!Hello
#endif

#ifndef HELLO
!Hello
# ifndef WORLD
!Hello !world
# else
!Hello world
# endif
#else
Hello
#endif

#define HELLO world
#undef HELLO

#define HELLO world blah 1 2 3 , abcd
#undef HELLO

#define HELLO() world
#undef HELLO

#define HELLO(x) world x
#undef HELLO

#define HELLO(x,y) world x y
#undef HELLO

#define HELLO(a,b,c,d,e,f) world a b c d e f
#undef HELLO

#define HELLO(...) world __VA_ARGS__
#undef HELLO

#define HELLO(a...) world a
#undef HELLO

#define HELLO(a,b...) world a b
#undef HELLO

#define HELLO(a,b,...) world a __VA_OPT__(x) b __VA_ARGS__
#undef HELLO

#define HELLO(a,b,...) world a __VA_OPT__( hello there blah ) b c d e __VA_ARGS__
#undef HELLO

#define HELLO(a,b,...) world a __VA_OPT__( (hello) ((there)) (((blah))) ) b c d e __VA_ARGS__
#undef HELLO

#define HELLO(a,b,c) world a #b #c
#undef HELLO

#define HELLO(a,b,c) world a##b ## c
#undef HELLO

#define HELLO world
#define HELLO world
#define HELLO world
#define HELLO world
#undef HELLO

#define HELLO() world
#define HELLO() world
#define HELLO() world
#define HELLO() world
#undef HELLO

#define HELLO(x) world
#define HELLO(x) world
#define HELLO(x) world
#define HELLO(x) world
#undef HELLO

#define HELLO(x,y) world
#define HELLO(x,y) world
#define HELLO(x,y) world
#define HELLO(x,y) world
#undef HELLO

#define HELLO(x,y,...) world
#define HELLO(x,y,...) world
#define HELLO(x,y,...) world
#define HELLO(x,y,...) world
#undef HELLO

testing the HELLO today
#define HELLO world
testing the HELLO today
testing the HELLO() today
#undef HELLO

#define HELLO() world
testing the HELLO() today
testing the HELLO () today
#undef HELLO

#define HELLO(x,y) world
testing the HELLO(3,4) today
testing the HELLO (3,4) today
#undef HELLO

#define HELLO(x,y) x world y
testing the HELLO(3,4) today
testing the HELLO (3,4) today
#undef HELLO

#define HELLO(x,y) y world x
testing the HELLO(3,4) today
testing the HELLO (3,4) today
#undef HELLO

#define HELLO(x,y) y x world x y
testing the HELLO(3,4) today
testing the HELLO (3,4) today
#undef HELLO

#define HELLO(x,y) y x world x y
testing the HELLO((3),((4))) today
testing the HELLO (((3)),(((4)))) today
#undef HELLO

#define HELLO(x,y) y x world x y
testing the HELLO("3","4") today
testing the HELLO ("3","4") today
#undef HELLO

#define HELLO(x,y,z) z y x world x y z
testing the HELLO(3,4,5) today
testing the HELLO (3, 4 ,5   ) today
#undef HELLO

