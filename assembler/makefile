CC = g++
ASM_DEPS = \
	Token.hpp \
	utils.hpp \
	commons.hpp \
	errors.hpp \
	validationSM.hpp \
	../lc3-hw.hpp
OBJS = Token.o \
	   validationSM.o \
	   utils.o
ASM_OBJ = assembler.o \
		  $(OBJS)
TEST_OBJ = assembler_test.o \
		  $(OBJS)
ASM_PROG = assembler.exe
TEST_PROG = assembler_test.exe
ASM_CPPFLAGS = -g -std=c++17 -Wall -Werror \
			   -D_NO_W32_PSEUDO_MODIFIERS -pthread
TEST_LDFLAGS = -lgtest

%.o: %.cpp $(ASM_DEPS)
	$(CC) $(ASM_CPPFLAGS) -c -o $@ $<

release: $(ASM_OBJ)
	$(CC) -o $(ASM_PROG) $(ASM_CPPFLAGS) $^

test: $(TEST_OBJ)
	$(CC) -o $(TEST_PROG) $(ASM_CPPFLAGS) $^ $(TEST_LDFLAGS)
	./$(TEST_PROG)

test_hard: $(TEST_OBJ)
	$(CC) -o $(TEST_PROG) $(ASM_CPPFLAGS) $^ $(TEST_LDFLAGS)
	./$(TEST_PROG) --gtest_break_on_failure

all: release test

clean:
	rm -f *.o
