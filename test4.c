#if 1
Hello
#else
OOPS
#endif

#if 0
OOPS
#else
Hello
#endif

#if 0,1
Hello
#else
OOPS
#endif

#if 1,0,1,0,1
Hello
#else
OOPS
#endif

#if 3 ? 10 : 20
Hello
#endif

#if 0 ? 10 : 20
Hello
#endif

//#if 1=4=3
//Hello
//#else
//OOPS
//#endif

//#if 1=4=3,2=3=1
//Hello
//#else
//OOPS
//#endif

#if a
OOPS
#else
Hello
#endif

#if a,b
OOPS
#else
Hello
#endif

//#if a=b=c
//Hello
//#else
//OOPS
//#endif

//#if a += b += c
//Hello
//#else
//OOPS
//#endif

#if 1 ? a : b
Hello
#endif

#if y ? f ? a : b : z
Hello
#endif

#if 4 + 3 || a >= b && c >= d || a < b
Hello
#endif

#if 4 + 3 - 3 + 1
Hello
#endif

#if -4 + 4
Hello
#endif

#if -4
#endif

#if +4
#endif

#if !0 + !!1
Hello
#endif

#if ~0 + ~~0 - !0
Hello
#endif

//#if a++
//Hello
//#endif

//#if ++a
//Hello
//#endif

//#if *a
//#endif

#if 4 * 3
#endif

#if 4|3
#endif

#if 4&3
#endif

#if 4^3
#endif

//#if abc.def
//#endif

//#if ~abc.def * 2
//#endif

//#if abc->def + 3
//#endif

//#if &a
//#endif

#if 3 + (4)
#endif

#if 4 * (2 + 3*5 - 3) + (2 + 4 + 8) + (4 * (3 + (5 / 2)))
#endif

#if 1 || 1
#endif

#if 1 || 0
#endif

#if 0 || 0
#endif

#if 1 && 1
#endif

#if 2 && 1
#endif

#if 0 && 1
#endif

#if 1 == 1
#endif

#if 2 == 1
#endif

#if 2 != 1
#endif

#if 1 > 1
#endif

#if 1 >= 1
#endif

#if 2 > 1
#endif

#if 1 < 2
#endif

#if 1 <= 2
#endif

#if c >= d || defined(MACRO1)
#endif

#if c >= d || defined(MACRO1) || !defined(MACRO2)
#endif

#define c 4
#define d 7

#if c >= d || defined(MACRO1)
#endif

#define MACRO1 11

#if c >= d || defined(MACRO1)
#endif

#undef MACRO1
#undef d
#undef c

