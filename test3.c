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

