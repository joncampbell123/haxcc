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

#if 1,0
Hello
#else
OOPS
#endif

#if 1,0,1,0,1
Hello
#else
OOPS
#endif

#if 1=4=3
Hello
#else
OOPS
#endif

#if 1=4=3,2=3=1
Hello
#else
OOPS
#endif

#if a
Hello
#else
OOPS
#endif

#if a,b
Hello
#else
OOPS
#endif

#if a=b=c
Hello
#else
OOPS
#endif

#if a += b += c
Hello
#else
OOPS
#endif

