OBJ=o
LIB=a

TARGETS=

all: $(TARGETS)

clean:
	rm -vf *.$(OBJ) *.$(LIB)
	rm -vf $(TARGETS)

.c.o:
	gcc -DLINUX -Wall -pedantic -c -o $@ $<

