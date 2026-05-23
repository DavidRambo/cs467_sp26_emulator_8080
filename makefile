CXX = g++
CXX_FLAGS = -std=c++20 -lSDL3 -O1 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wnull-dereference
DEBUG_FLAGS = -ggdb -DDEBUG=1

SRCDIR = src
BUILDDIR = build
TESTDIR = tests
TESTSRCFILES = $(wildcard $(TESTDIR)/test_*.cc)
LIBRARY = /usr/local/lib/

# Creates a list of object files
OBJS = $(addprefix $(BUILDDIR)/, CPU8080.o Memory8080.o Instructions8080.o Input.o AudioMixer.o GameWindow.o SpaceInvadersVRamDecoder.o ShiftRegister.o)

# For passing doctest query flags when running tests.
# Example use, which will run test cases with a name including "input":
#   make test TESTFLAGS='--test-case=*input*'
# This will run the tests in test_io.cc:
#   make test TESTFLAGS='--source-file=tests/test_io.cc'
# For more: https://github.com/doctest/doctest/blob/master/doc/markdown/commandline.md
TESTFLAGS =

emu8080: $(OBJS) $(SRCDIR)/main.cc
	$(CXX) -o emu8080 $^ -L$(LIBRARY) $(CXX_FLAGS)

debug: $(SRCDIR)/main.cc $(SRCDIR)/debug.cc $(SRCDIR)/CPU8080.cc $(SRCDIR)/CPU8080.h $(BUILDDIR)/Memory8080.o $(BUILDDIR)/Instructions8080.o $(BUILDDIR)/Input.o $(BUILDDIR)/AudioMixer.o $(BUILDDIR)/GameWindow.o $(BUILDDIR)/SpaceInvadersVRamDecoder.o $(BUILDDIR)/ShiftRegister.o
	$(CXX) -o emu8080 $^ -L$(LIBRARY) $(CXX_FLAGS) $(DEBUG_FLAGS)

test: all_tests
	./all_tests $(TESTFLAGS)

all_tests: $(OBJS) $(TESTSRCFILES)
	$(CXX) $(CXX_FLAGS) -o all_tests $^

$(BUILDDIR)/CPU8080.o: $(SRCDIR)/CPU8080.cc $(SRCDIR)/CPU8080.h
	$(CXX) $(CXX_FLAGS) -o $(BUILDDIR)/CPU8080.o -c $(SRCDIR)/CPU8080.cc

$(BUILDDIR)/Memory8080.o: $(SRCDIR)/Memory8080.cc $(SRCDIR)/Memory8080.h
	$(CXX) $(CXX_FLAGS) -o $(BUILDDIR)/Memory8080.o -c $(SRCDIR)/Memory8080.cc

$(BUILDDIR)/Instructions8080.o: $(SRCDIR)/Instructions8080.cc
	$(CXX) $(CXX_FLAGS) -o $(BUILDDIR)/Instructions8080.o -c $(SRCDIR)/Instructions8080.cc

$(BUILDDIR)/GameWindow.o: $(SRCDIR)/GameWindow.cc $(SRCDIR)/GameWindow.h
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(BUILDDIR)/SpaceInvadersVRamDecoder.o: $(SRCDIR)/SpaceInvadersVRamDecoder.cc $(SRCDIR)/SpaceInvadersVRamDecoder.h
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(BUILDDIR)/Input.o: $(SRCDIR)/Input.cc $(SRCDIR)/Input.h
	$(CXX) $(CXX_FLAGS) -o $(BUILDDIR)/Input.o -c $(SRCDIR)/Input.cc

$(BUILDDIR)/AudioMixer.o: $(SRCDIR)/AudioMixer.cc $(SRCDIR)/AudioMixer.h
	$(CXX) $(CXX_FLAGS) -o $(BUILDDIR)/AudioMixer.o -c $(SRCDIR)/AudioMixer.cc

$(BUILDDIR)/ShiftRegister.o: $(SRCDIR)/ShiftRegister.cc $(SRCDIR)/ShiftRegister.h
	g++ -o $(BUILDDIR)/ShiftRegister.o -c $(SRCDIR)/ShiftRegister.cc

# The @ sign indicates a shell command.
clean:
	@if [ -f emu8080 ]; then\
		rm emu8080;\
	fi
	@(rm -r $(BUILDDIR) && mkdir $(BUILDDIR)) || mkdir $(BUILDDIR) 
