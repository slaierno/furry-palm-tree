CC = g++
DEPS = \
	lc3-hw.hpp \
	lc3-video.hpp \
	lc3-debug.hpp \
	memory.hpp
OBJ = main.o \
	lc3-hw.o \
	memory.o
PROG = fpt.exe
PLAYGROUND_PROG = fpt-playground.exe
TEST_OBJ = main_test.o \
		   lc3-hw.o \
		   memory.o
TEST_PROG = main_test.exe
TEST_LDFLAGS = -lgtest
CPPFLAGS = -pthread -std=c++17 -Wall -Werror
TEST_CPPFLAGS = -g $(CPPFLAGS)

%.o: %.cpp $(DEPS)
	$(CC) -g -c -o $@ $< $(CPPFLAGS)

test: $(TEST_OBJ)
	$(CC) -g -o $(TEST_PROG) $(TEST_CPPFLAGS) $^ $(TEST_LDFLAGS)
	./$(TEST_PROG)

release: $(OBJ)
	$(CC) -g -o $(PROG) $^

all: test release

playground:
	$(CC) -g -O0 -o playground.exe playground.cpp $(CPPFLAGS) $(TEST_LDFLAGS)
	./$(PLAYGROUND_PROG)

clean:
	rm -f ./*.o
