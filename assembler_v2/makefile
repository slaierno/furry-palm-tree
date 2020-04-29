CC = g++-10
INCLUDE_DIR = ../include
VM_DIR = ..
ASM_DEPS = \
	Token.hpp \
	Instruction.hpp \
	commons.hpp \
	assembler.hpp \
	assembler_test.hpp \
	$(VM_DIR)/lc3-hw.hpp \
	$(INCLUDE_DIR)/cx.hpp \
	$(INCLUDE_DIR)/cx_string.hpp \
	$(INCLUDE_DIR)/cx_vector.hpp \
	$(INCLUDE_DIR)/cx_map.hpp \
	$(INCLUDE_DIR)/cx_algorithm.hpp
OBJS = Token.o \
       Instruction.o \
	   assembler.o
ASM_OBJ = main.o \
		  $(OBJS)
TEST_OBJ = main_test.o \
		  $(OBJS)
ASM_PROG = fpt-asm.exe
TEST_PROG = fpt-asm_test.exe
ASM_CPPFLAGS = -g -O0 -std=gnu++20 -Wall -Werror \
			   -D_NO_W32_PSEUDO_MODIFIERS -pthread \
               -isystem $(VM_DIR)/googletest/googletest/include \
			   -I$(INCLUDE_DIR)
LDFLAGS = -lpthread
TEST_LDFLAGS = $(LDFLAGS) -L../googletest/lib -lgtest

%.o: %.cpp $(ASM_DEPS)
	$(CC) $(ASM_CPPFLAGS) -c -o $@ $< $(LDFLAGS)

release: $(ASM_OBJ)
	$(CC) -o $(ASM_PROG) $(ASM_CPPFLAGS) $^ $(LDFLAGS)

test: $(TEST_OBJ)
	$(CC) -o $(TEST_PROG) $(ASM_CPPFLAGS) $^ $(TEST_LDFLAGS)
	./$(TEST_PROG)

all: release test

clean:
	rm -f *.o
