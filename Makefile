# for Windows, put .exe here
SUFFIX=

# outputs
OUTPUTS=haxpp

# library
LIBOBJ=

# default CFLAGS (GCC+Linux)
LDFLAGS=-lm
CFLAGS=-Wall -Wextra -pedantic -std=c11 -g3 -O0
CXXFLAGS=-Wall -Wextra -pedantic -std=c++11 -g3 -O0

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
haxpp: haxpp.o $(LIBOBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

