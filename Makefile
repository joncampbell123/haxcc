OBJ=o
LIB=a

HAXCC=haxcc
# haxcc.OBJ must come LAST, must make cparsb.o and cparsl.o first
HAXCC_OBJ=cparsb.$(OBJ) cparsl.$(OBJ) haxcc.$(OBJ)

TARGETS=$(HAXCC)

$(HAXCC): $(HAXCC_OBJ)
	gcc -o $@ $^

all: $(TARGETS)

clean:
	rm -vf cparsb.c cparsb.c.h cparsl.c cparsl.c.h
	rm -vf *.$(OBJ) *.$(LIB)
	rm -vf $(TARGETS)
	rm -vf *~

.l.c: .SECONDARY
	flex --header-file="$@.h" -o $@ $^

.y.c: .SECONDARY
	bison "--defines=$@.h" -o $@ $^

.c.o:
	gcc -std=gnu99 -DLINUX -DYYDEBUG=1 -DYYERROR_VERBOSE=1 -DYYTOKEN_TABLE=1 -Wall -pedantic -g3 -O0 -c -o $@ $<

tests: $(HAXCC)
	./$(HAXCC) <test1.c

vtests: $(HAXCC)
	valgrind --leak-check=full --show-reachable=yes --track-origins=yes ./$(HAXCC) <test1.c

