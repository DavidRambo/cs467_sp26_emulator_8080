CXX = g++
CXX_FLAGS = -std=c++17 -ggdb

SRCDIR = src
BUILDDIR = build
TESTDIR = tests

LIBRARY = /usr/local/lib/

$(BUILDDIR)/prog: $(BUILDDIR)/main.o $(BUILDDIR)/GraphicsDisplay.o
	g++ $(BUILDDIR)/main.o $(BUILDDIR)/GraphicsDisplay.o -o prog -L$(LIBRARY) -lSDL3

$(BUILDDIR)/main.o: $(SRCDIR)/main.cc
	g++ -c $(SRCDIR)/main.cc -o $(BUILDDIR)/main.o

$(BUILDDIR)/GraphicsDisplay.o: $(SRCDIR)/GraphicsDisplay.cc $(SRCDIR)/GraphicsDisplay.h
	g++ -c $(SRCDIR)/GraphicsDisplay.cc -o $(BUILDDIR)/GraphicsDisplay.o