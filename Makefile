SRCDIR = src

CXX = g++
CXXFLAGS = -pedantic -Wall -Wextra -g -std=c++11
LDFLAGS =
SRC =  cfile.cpp cgate.cpp cactions.cpp cbackup.cpp main.cpp
OBJ = $(SRC:%.cpp=%.o)

all: compile doc
	

compile: letsimon
	

letsimon: $(OBJ)
	$(CXX) $^ $(LDFLAGS) -o letsimon

main.o: $(SRCDIR)/main.cpp cfile.o cgate.o cactions.o cbackup.o
cfile.o: $(SRCDIR)/cfile.cpp $(SRCDIR)/cfile.hpp
cgate.o: $(SRCDIR)/cgate.cpp $(SRCDIR)/cgate.hpp
cactions.o: $(SRCDIR)/cactions.cpp $(SRCDIR)/cactions.hpp cfile.o cgate.o
cbackup.o: $(SRCDIR)/cbackup.cpp $(SRCDIR)/cbackup.hpp cfile.o cgate.o cactions.o

%.o:
	$(CXX) -c $(SRCDIR)/$*.cpp $(CXXFLAGS) -o $@

run:
	./letsimon

clean:
	rm *o letsimon 2>/dev/null || echo "No object files to delete"
	rm -rf doc 2>/dev/null || echo "No documentation to delete"

doc:
	doxygen src/doxy/Doxyfile && mv html doc 
