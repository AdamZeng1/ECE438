EXEC_LINK_STATE = linkstate
EXEC_DISTANCE_VECTOR = distvec

COMPILER = g++

all: linkstate distvec

linkstate: linkstate.o node.o
	$(COMPILER) -pthread linkstate.o node.o -o linkstate

distvec: distvec.o node.o
	$(COMPILER) -pthread distvec.o node.o -o distvec

linkstate.o: src/linkstate.cpp
	$(COMPILER) -c src/linkstate.cpp

distvec.o: src/distvec.cpp
	$(COMPILER) -c src/distvec.cpp

node.o: src/node.cpp
	$(COMPILER) -c src/node.cpp
clean:
	-rm *.o
