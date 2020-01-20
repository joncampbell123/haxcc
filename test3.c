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

#define HELLO(...) world __VA_ARGS__
testing the HELLO() today
testing the HELLO (3) today
testing the HELLO (3,4,5) today
testing the HELLO (3, 4   ,5) today
#undef HELLO

#define HELLO(a...) a world __VA_ARGS__
testing the HELLO (3) today
testing the HELLO (3,4,5) today
testing the HELLO (3, 4   ,5) today
#undef HELLO

#define HELLO(a,...) a world __VA_ARGS__
testing the HELLO (3) today
testing the HELLO (3,4,5) today
testing the HELLO (3, 4   ,5) today
#undef HELLO

#define HELLO(a,b) a##b world
testing the HELLO (a,b) today
testing the HELLO (funny_,man) today
#undef HELLO

#define HELLO(a,b) a##b##a##b##a world
testing the HELLO (a,b) today
testing the HELLO (abc,xyz) today
#undef HELLO

#define HELLO(a,b) #a #b world #a#b
testing the HELLO (a,b) today
testing the HELLO (funny,man) today
#undef HELLO

#define HELLO(a,...) a world __VA_OPT__(,) __VA_ARGS__
testing the HELLO (funny) today
testing the HELLO (funny,man) today
#undef HELLO

#define HELLO(a,b,c...) #a #b world #a#b#c
testing the HELLO (funny,man,hey) today
testing the HELLO (funny,man,lol) today
testing the HELLO (funny,man,lol,omg,wtf) today
#undef HELLO

/* NTS: GCC behavior says that macro##name combines the value of "macro" with the string "name".
 *      So this becomes worldABCD -> world peace, not worldwrong */
#define ABCD wrong
#define worldABCD world peace
#define HELLO(a,b,c...) #a #b world##ABCD #a#b#c
testing the HELLO (funny,man,hey) today
testing the HELLO (funny,man,lol) today
testing the HELLO (funny,man,lol,omg,wtf) today
#undef worldABCD
#undef HELLO
#undef ABCD

#define HELLO(a,b,...) #a #b world #a#b#__VA_ARGS__
testing the HELLO (funny,man) today
testing the HELLO (funny,man,lol) today
testing the HELLO (funny,man,lol,omg,wtf) today
#undef HELLO

#define ABCD peace
#define HELLO world ABCD
testing the HELLO today
#undef HELLO
#undef ABCD

#define ABCD(x) x
#define HELLO(z,t) world z ABCD(t)
testing the HELLO(funny,peace) today
testing the HELLO(freeze,peach) today
#undef HELLO
#undef ABCD

#ifdef HELLO
Should not HELLO
#else
OK
#endif

#define HELLO
#ifdef HELLO
OK
#else
Should not !HELLO
#endif
#undef HELLO

#define HELLO(a,b,...) a, b, __VA_ARGS__
testing the HELLO (funny,man) today
testing the HELLO (funny,man,lol) today
testing the HELLO (funny,man,lol,omg,wtf) today
#undef HELLO

#define HELLO(a,b,...) a, b __VA_OPT__(,) __VA_ARGS__
testing the HELLO (funny,man) today
testing the HELLO (funny,man,lol) today
testing the HELLO (funny,man,lol,omg,wtf) today
#undef HELLO

#define HELLO(a,b,...) a,b,##__VA_ARGS__
testing the HELLO (funny,man) today
testing the HELLO (funny,man,lol) today
testing the HELLO (funny,man,lol,omg,wtf) today
#undef HELLO

#define HELLO(a,b,...) a, b, ##__VA_ARGS__
testing the HELLO (funny,man) today
testing the HELLO (funny,man,lol) today
testing the HELLO (funny,man,lol,omg,wtf) today
#undef HELLO

#define HELLO(a,b,...) a, b, ## __VA_ARGS__
testing the HELLO (funny,man) today
testing the HELLO (funny,man,lol) today
testing the HELLO (funny,man,lol,omg,wtf) today
#undef HELLO

