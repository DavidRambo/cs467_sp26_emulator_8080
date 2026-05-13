CXX = g++
CXX_FLAGS = -std=c++17 -ggdb

SRCDIR = src
BUILDDIR = build
TESTDIR = tests

LIBRARY = /usr/local/lib/

$(BUILDDIR)/prog: $(BUILDDIR)/main.o $(BUILDDIR)/GameWindow.o $(BUILDDIR)/SpaceInvadersVRamDecoder.o
	$(CXX) $(CXX_FLAGS) $^ -o $@ -L$(LIBRARY) -lSDL3

$(BUILDDIR)/main.o: $(SRCDIR)/main.cc
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(BUILDDIR)/GameWindow.o: $(SRCDIR)/GameWindow.cc $(SRCDIR)/GameWindow.h
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(BUILDDIR)/SpaceInvadersVRamDecoder.o: $(SRCDIR)/SpaceInvadersVRamDecoder.cc $(SRCDIR)/SpaceInvadersVRamDecoder.h
	$(CXX) $(CXX_FLAGS) -c $< -o $@

RunAllTests: $(TESTDIR)/test_prog

$(TESTDIR)/test_prog: $(TESTDIR)/test_sample.o $(SRCDIR)/SpaceInvadersVRamDecoder.o
	g++ $^ -o $@ -L$(LIBRARY) -lSDL3

$(TESTDIR)/test_sample.o: $(TESTDIR)/test_sample.cc 
	g++ -c $< -o $@

clean:
	rm -r ./build
	mkdir ./build