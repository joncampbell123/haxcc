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

#if !0 + !!1
Hello
#endif

#if ~0 + ~~0 - !0
Hello
#endif

#if a++
Hello
#endif

#if ++a
Hello
#endif

#if *a
#endif

#if &a
#endif

