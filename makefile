SHELL = /bin/sh

SRCDIR = src
BUILDDIR = build
TESTDIR = tests
TESTSRCFILES = $(wildcard $(TESTDIR)/test_*.cc)

# Creates a list of object files
OBJS = $(addprefix $(BUILDDIR)/, CPU8080.o Memory8080.o Instructions8080.o)

emu8080: $(OBJS) $(SRCDIR)/main.cc
	g++ -o emu8080 $^ -lSDL3

test: all_tests
	./all_tests

SDLApp.o: $(SRCDIR)/SDLApp.cc $(SRCDIR)/SDLApp.h
	g++ -o $(BUILDDIR)/SDLApp.o -c $(SRCDIR)/SDLApp.cc

all_tests: $(OBJS) $(TESTSRCFILES)
	g++ -o all_tests $^ 

$(BUILDDIR)/CPU8080.o: $(SRCDIR)/CPU8080.cc $(SRCDIR)/CPU8080.h
	g++ -o $(BUILDDIR)/CPU8080.o -c $(SRCDIR)/CPU8080.cc

$(BUILDDIR)/Memory8080.o: $(SRCDIR)/Memory8080.cc $(SRCDIR)/Memory8080.h
	g++ -o $(BUILDDIR)/Memory8080.o -c $(SRCDIR)/Memory8080.cc

$(BUILDDIR)/Instructions8080.o: $(SRCDIR)/Instructions8080.cc $(SRCDIR)/Instructions8080.h
	g++ -o $(BUILDDIR)/Instructions8080.o -c $(SRCDIR)/Instructions8080.cc

# The @ sign indicates a shell command.
clean:
	@if [ -f emu8080 ]; then\
		rm emu8080;\
	fi
	@(rm -r $(BUILDDIR) 2> /dev/null && mkdir $(BUILDDIR)) || mkdir $(BUILDDIR) 
