SHELL = /bin/sh

SRCDIR = src
BUILDDIR = build
TESTDIR = tests
TESTSRCFILES = $(wildcard $(TESTDIR)/test_*.cc)

# Creates a list of object files
OBJS = $(addprefix $(BUILDDIR)/, CPU8080.o Memory8080.o Instructions8080.o)

emu8080: $(OBJS)
	g++ -o emu8080 $(OBJECTS) $(SRCDIR)/main.cc

test: all_tests
	./all_tests

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
	@(rm -r $(BUILDDIR) && mkdir $(BUILDDIR)) || mkdir $(BUILDDIR) 
