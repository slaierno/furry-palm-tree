CC = g++
ASM_DEPS = \
	Token.hpp \
	Instruction.hpp \
	commons.hpp \
	assembler.hpp \
	assembler_test.hpp
OBJS = Token.o \
       Instruction.o \
	   assembler.o
ASM_OBJ = main.o \
		  $(OBJS)
TEST_OBJ = main_test.o \
		  $(OBJS)
ASM_PROG = fpt-asm.exe
TEST_PROG = fpt-asm_test.exe
ASM_CPPFLAGS = -g -std=c++2a -Wall -Werror \
			   -D_NO_W32_PSEUDO_MODIFIERS -pthread \
               -isystem /workspace/googletest-release-1.10.0/googletest/include
TEST_LDFLAGS = -L/workspace/googletest-release-1.10.0/lib -lgtest

%.o: %.cpp $(ASM_DEPS)
	$(CC) $(ASM_CPPFLAGS) -c -o $@ $<

release: $(ASM_OBJ)
	$(CC) -o $(ASM_PROG) $(ASM_CPPFLAGS) $^

test: $(TEST_OBJ)
	$(CC) -o $(TEST_PROG) $(ASM_CPPFLAGS) $^ $(TEST_LDFLAGS)
	./$(TEST_PROG)

all: release test

clean:
	rm -f *.o
