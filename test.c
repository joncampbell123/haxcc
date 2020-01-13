BBBBB
#include "test.h"
Aaaaa
#define HELLO0
#define HELLO1() HELLO!WORLD!
#define HELLO2(x) 123 x 456
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
#define HELLO16(xx,yy,zzzz,...) (zzzz,yy,xx,__VA_ARGS__)
#define STRINGY(a) #a
#define CONCAT3(a,b,c) a##b##c
#define HELLO17(x,y,z,...) (z,y,x __VA_OPT__(,) __VA_ARGS__)
#define HELLO18(xx,yy,zzzz,...) (zzzz,yy,xx,#__VA_ARGS__)
#define HELLO19(...) Hello __VA_ARGS__
#define HELLO20(a...) Hello a there
#define HELLO21(a,b,c...) Hello c there b a
#define HELLO22(x,y,z,...) (z,y,x##__VA_OPT__(,)##__VA_ARGS__)
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
I said HELLO13 for the win
I said "HELLO13" 123 for the HELLO13 win and then HELLO14
#define MACRO1 1234
#define MACRO2 123 MACRO1 12345
#define MACRO3 abc MACRO1 def MACRO2 ghi
MACRO1
MACRO2
MACRO3
Hello1 HELLO1 Hello1() HELLO1()
HELLO2
HELLO2(abc)
HELLO7(a,b,c)
HELLO7(a,(b),c)
HELLO7(a,(b),((((c)))))
HELLO7(a,b,c,d)
HELLO16(a,b,c)
HELLO16(a,b,c,d)
HELLO16(a,b,c,d,e,f,g,h)
Yeah!
STRINGY(hello)
CONCAT3(hello,world,123)
HELLO17(a,b,c)
HELLO17(a,b,c,d)
HELLO17(a,b,c,d,e)
HELLO17(a,b,c,d,e,f)
HELLO18(a,b,c,hello,world)
HELLO19()
HELLO19(x)
HELLO19(x,y,z)
HELLO20(a)
HELLO20(a,b,c)
HELLO20(a,b,c,d)
HELLO21(a,b,c)
HELLO21(a,b,c,d)
HELLO21(a,b,c,d,e,f,g)
HELLO22(a,b,c)
HELLO22(a,b,c,d)
HELLO22(a,b,c,d,e)
HELLO22(a,b,c,d,e,f)
HELLO22(a,"b",("c"))
HELLO22(a,"(((b",("c"))
HELLO22(a,"(b",("c))"))
HELLO22(a,'b','c','d')
HELLO22(a,'(','c','d')
HELLO22(a,'(',')',')')
#if 0
Should not see, 0
#endif
#if 1
Should see, 1
#endif
#if 0
Should not see, 0
#else
Should see, 0 #else case
#endif
#if 0
Should not see ,0
#elif 1
Should see, 0 #elif 1
#else
Should not see, 0 #elif 1 #else
#endif
#if 0
Should not see ,0
#elif 0
Should see, 0 #elif 0
#elif 1
Should see, 0 #elif 0 #elif 1
#elif 1
Should not see, 0 #elif 0 #elif 1 #elif 1
#elif 0
Should not see, 0 #elif 0 #elif 1 #elif 1 #elif 0
#else
Should not see, 0 #elif 0 #elif 1 #elif 1 #elif 0 #else
#endif
#if NOTEXIST
Should not see, NOTEXIST
#endif
#if MACRO1
Should see, MACRO1
#endif
#if defined(NOTEXIST)
Should not see, defined("NOTEXIST")
#endif
#if defined(MACRO1)
Should see, defined("MACRO1")
#endif
#if defined NOTEXIST
Should not see, defined("NOTEXIST")
#endif
#if defined MACRO1
Should see, defined("MACRO1")
#endif
#if defined(NOTEXIST)
Should not see, defined("NOTEXIST")
#elif defined(MACRO1)
Should see, defined("MACRO1") elif
#endif
#if defined(NOTEXIST)
Should not see, defined("NOTEXIST")
#elif defined(ALSONOTEXIST)
Should not see, defined("ALSONOTEXIST") elif
#elif defined(MACRO1)
Should see, defined("MACRO1") elif elif
#else
Should not see, else
#endif
Should not see // C++ comments
// or this comment
But "this // should be visible"
Should see this /* but not this */but this also
/* this comment */Should see this /*comment*/blah
/* another comment*/This/*comment blah*/
/* comment /* nesting */ blah */See this
More nesting/* here /* and here /* and and here */ end */ end */ here
Here follows a multi-line comment/* this is a multiple line comment.
This comment explains things.
End of comment*/ and this is the end.
Here is another one/* that has multiple lines
and preprocessor macros in the comment like
#endif
that might otherwise confuse the preprocessor*/ with #endif in it.
Multiple line nested/* comment in comment
for comments /* in side comments */ blah */ end it.
Testing /* multiple line /* nested
stuff /* level3 /* blah
blah */ wwww */ wwww */ aaaa */ end.
/* Test case seen in Linux headers */
#if defined __GNUC__ && defined __GNUC_MINOR__
#elif 1 + 1 == 2
Duh
#endif
#if 1 == 1 && \
    3 <= 4
Multiline #if works
#endif
#define REDEFINE 123
#define REDEFINE 123
#define REDEFINE 123
#if 4u > 3u
Duh
#endif
#if 50L > 25L
Duh
#endif
#if 200ull == 200
Duh
#endif
#if '\x30' == '0'
Duh
#endif
#if '\x30' == 0x30
Duh
#endif
#if '\a' == 0x07
Duh
#endif
#if '\0' == 0
Duh
#endif
#if '\377' == 0xff
Duh
#endif
#if '\u1000' == 0x1000
Duh
#endif
#pragma echovalif L'u'
#pragma echovalif L'\u1000'
#pragma echovalif L'\U10001000'
#pragma echovalif L'\U10001001'
