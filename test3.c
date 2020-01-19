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

