#include pingas
#define hello
 #define hello
#    define hello
    #	define		  hello
hello
"hello"
"hello world"
"this string has \"quotes\" in it"
"this string\
splits across lines"
Hello world
Hello "world" today
"hello" "world"
"Hello\tworld"
"Hello \x31\x32\x33"
"Hello\40world"
"Hello\040world"
"Hello\11world" // \11 = TAB
"Hello\u0020world"
"Hello\U00000020world"
'a' 'b' 'c' '0' '\t' '\40' '\040' '\0' '\000' '\xff' 'abcd' 'abcdefgh'
0 1 2 10 11 19 333 12345678 07 010 040 0x20 0x33 0xAA 0x123456789ABCDEF0 2u 4ul 6l 8ll 10ull 12i64 14I64 2U 6L
1.0 1.1 1.1234 5.0 5.5 6.666
1.1
1.11
1.111
1.1111
1.11111
1.111111
1.1111111
1.11111111
1.111111111
1.1111111111
1.11111111111
1.111111111111
1.1111111111111
1.11111111111111
1.111111111111111
1.1111111111111111
1.11111111111111111
1.111111111111111111
1.1111111111111111111
1.11111111111111111111
1.111111111111111111111
1.1111111111111111111111
1.11111111111111111111111
1.111111111111111111111111
1.1111111111111111111111111
1.11111111111111111111111111
1.111111111111111111111111111
0.1
0.11
0.111
0.1111
0.11111
0.111111
0.1111111
0.11111111
0.111111111
0.1111111111
0.11111111111
0.111111111111
0.1111111111111
0.11111111111111
0.111111111111111
0.1111111111111111
0.11111111111111111
0.111111111111111111
0.1111111111111111111
0.11111111111111111111
0.111111111111111111111
0.1111111111111111111111
0.11111111111111111111111
0.111111111111111111111111
0.1111111111111111111111111
0.11111111111111111111111111
0.000000000000000000000000000000000000000000000000000000000001
1.000000000000000000000000000000000000000000000000000000000001
0.0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001
100000000.0
1000000000000000000000000000000000000000000000.0
1.0f 2.0
1.0d
1.0l
0.1
1e-1
1e9
1e-20
1e20
0.1e1
1.0
0.000000000000000000000000000000000000000000000000000001e54
0x1.0p0
0x2.0p0
0x8.0p0
0x8.0p-3
0x1.0p+3
0x1.4p0
0x1.8p0
0x1.Cp0
0x1.FFFFFFFFFFFFFFFFFFFp0
0x1.0p-1
0x1.0p-2
0x1.0p-3
0x1.0p-4
-4 -- +3 ++ , , ,,,, ~ ~~ 5 !4   !0 4 & 3  &amp * 2 *indir **ptr () ( ) (x) ((x)) [4] [4[2]] [2][3] . a . a .. a . a . a . a . a ... a . a . a -> b -> c -> -> d {} { { } }
sizeof(char[6]) 5/3 5%3 5*3 5<<3 5>>3 5<=3 5>=3 5<3 5>3 5==3 5!=3 5=3 5&3 5^3 5|3 5&&3 5||3 5?3:1
5=3
5         =   3
5+=3
5-=3
5*=3
5/=3
5%=3
5<<=3
5>>=3
5&=3
5^=3
5|=3
macrodef #stringify #me token##paste
auto break case char const continue default do double else enum extern float for goto if inline int long register restrict return short signed sizeof static struct switch typedef union unsigned void volatile while __VA_OPT__ __VA_ARGS__
_Alignas _Alignof _Atomic _Bool _Complex _Generic _Imaginary _Noreturn _Static_assert _Thread_local _Pragma
if elif else endif defined ifdef ifndef define undef include line error pragma
/* should not be auto-identified as keywords */
alignas alignof
