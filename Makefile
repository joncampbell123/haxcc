OBJ=o
LIB=a

HAXCC=haxcc
HAXCC_OBJ=haxcc.$(OBJ) cparsb.$(OBJ) cparsl.$(OBJ)

TARGETS=$(HAXCC)

$(HAXCC): $(HAXCC_OBJ)
	gcc -o $@ $^

all: $(TARGETS)

clean:
	rm -vf *.$(OBJ) *.$(LIB)
	rm -vf $(TARGETS)
	rm -vf *~

.l.c: .SECONDARY
	flex --header-file="$@.h" -o $@ $^

.y.c: .SECONDARY
	bison "--defines=$@.h" -o $@ $^

.c.o:
	gcc -std=gnu99 -DLINUX -Wall -pedantic -c -o $@ $<

