TESTS = \
	defer_1 \
	post_1

CXXFLAGS = -std=c++14 -Wall -Wextra -O3 -g

all: $(TESTS)

clean:
	rm -f $(TESTS) $(TESTS:%=%.o)

$(TESTS): %: %.o
	g++ -o$@ $<

$(TESTS:%=%.o): $(wildcard *.hpp)

.cpp.o:
	g++ $(CXXFLAGS) -c -o$@ $<
