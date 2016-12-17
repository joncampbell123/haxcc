OBJ=o
LIB=a

HAXCPP=haxcpp
HAXCPP_OBJ=haxcpp.$(OBJ)
HAXCPP_DEPS=

TARGETS=$(HAXCPP)

all: $(TARGETS)

clean:
	rm -vf *.$(OBJ) *.$(LIB)
	rm -vf $(TARGETS)

$(HAXCPP): $(HAXCPP_OBJ) $(HAXCPP_DEPS)
	gcc -o $@ $(HAXCPP_OBJ)

.c.o:
	gcc -DLINUX -c -o $@ $<

