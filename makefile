SHELL = /bin/sh

SRCDIR = src
BUILDDIR = build
TESTDIR = tests

OBJECTS = $(wildcard $(BUILDDIR)/*.o)
TESTOBJS = $(wildcard $(TESTDIR)/test_*.cc)

test: $(OBJECTS) $(TESTDIR)/test_declaration.h $(TESTDIR)/doctest.h
	g++ -o all_tests $(TESTOBJS) $(OBJECTS) && ./all_tests

emu8080: CPU8080.o Memory8080.o Instructions8080.o
	g++ -o emu8080 $(SRCDIR)/main.cc $(OBJECTS)

CPU8080.o: $(SRCDIR)/CPU8080.cc $(SRCDIR)/CPU8080.h
	g++ -o $(BUILDDIR)/CPU8080.o -c $(SRCDIR)/CPU8080.cc

Memory8080.o: $(SRCDIR)/Memory8080.cc $(SRCDIR)/Memory8080.h
	g++ -o $(BUILDDIR)/Memory8080.o -c $(SRCDIR)/Memory8080.cc

Instructions8080.o: $(SRCDIR)/Instructions8080.cc $(SRCDIR)/Instructions8080.h
	g++ -o $(BUILDDIR)/Instructions8080.o -c $(SRCDIR)/Instructions8080.cc
