# for Windows, put .exe here
SUFFIX=

# outputs
OUTPUTS=haxpp haxlr

# library
LIBOBJ=util.o linesrc.o linesink.o linesrst.o

# default CFLAGS (GCC+Linux)
CFLAGS=-Wall -Wextra -pedantic -std=c11
CXXFLAGS=-Wall -Wextra -pedantic -std=c++11

# how to compile in general
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

# all target
all: $(OUTPUTS)

# clean target
clean:
	rm -f *.o

# distclean
distclean: clean
	rm -f $(OUTPUTS)

# how to link haxlr
haxlr: haxlr.o $(LIBOBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

# how to link haxpp
haxpp: haxpp.o $(LIBOBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

