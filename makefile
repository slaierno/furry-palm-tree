CC = g++
DEPS = \
	lc3-hw.hpp \
	lc3-video.hpp \
	memory.hpp
OBJ = main.o \
	lc3-hw.o \
	memory.o
PROG = main.exe
TEST_OBJ = main_test.o \
		   lc3-hw.o \
		   memory.o
TEST_PROG = main_test.exe
TEST_LDFLAGS = -lgtest
TEST_CPPFLAGS = -g

%.o: %.cpp $(DEPS)
	$(CC) -g -c -o $@ $< $(CPPFLAGS)

test: $(TEST_OBJ)
	$(CC) -g -o $(TEST_PROG) $(TEST_CPPFLAGS) $^ $(TEST_LDFLAGS)
	./$(TEST_PROG)

release: $(OBJ)
	$(CC) -g -o $(PROG) $^

clean: 
	rm -f ./*.o
