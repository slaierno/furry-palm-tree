CC = g++
DEPS = \
	lc3-hw.hpp \
	lc3-video.hpp \
	lc3-debug.hpp \
	memory.hpp \
	InputBuffer.hpp
COMMON_OBJ = \
	lc3-hw.o \
	memory.o

OBJ = $(COMMON_OBJ) main.o
GUI_OBJ = $(COMMON_OBJ) main_gui.o
TEST_OBJ = $(COMMON_OBJ) main_test.o

PROG = fpt.exe
GUI_PROG = fpt-gui.exe
TEST_PROG = fpt-test.exe
PLAYGROUND_PROG = fpt-playground.exe

GUI_LDFLAGS = `wx-config --libs`
TEST_LDFLAGS = -lgtest
PLAYGROUND_LDFLAGS = $(TEST_LDFLAGS)

CPPFLAGS = -pthread -std=c++17 -Wall -Werror

main_gui.o : CPPFLAGS += `wx-config --cxxflags`
playground.o main_test.o : CPPFLAGS += -g
%.o: %.cpp $(DEPS)
	$(CC) -g -c -o $@ $< $(CPPFLAGS)

release: $(OBJ)
	$(CC) -g -o $(PROG) $^

test: $(TEST_OBJ)
	$(CC) -g -o $(TEST_PROG) $(CPPFLAGS) $^ $(TEST_LDFLAGS)
	./$(TEST_PROG)

gui: $(GUI_OBJ)
	$(CC) -g -o $(GUI_PROG) $(GUI_CPPFLAGS) $^ $(GUI_LDFLAGS)
	
all: test release gui

playground:
	$(CC) -o $(PLAYGROUND_PROG) playground.cpp $(CPPFLAGS) $(PLAYGROUND_LDFLAGS)
	./$(PLAYGROUND_PROG)

clean:
	rm -f ./*.o
