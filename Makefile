# for Windows, put .exe here
SUFFIX=

# outputs
OUTPUTS=haxpp

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

# how to link haxpp
haxpp: haxpp.o util.o linesrc.o
	$(CXX) $(LDFLAGS) -o $@ $^

