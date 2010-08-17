all: test_kdtree test_hayne

test_kdtree: test_kdtree.cpp
	g++ -I.. -Wall -ansi -pedantic -g -O2 -o test_kdtree test_kdtree.cpp

test_hayne: test_hayne.cpp
	g++ -I.. -Wall -ansi -pedantic -g -O2 -o test_hayne test_hayne.cpp

all_gcc: test_kdtree-gcc3.4 test_kdtree-gcc4.3 test_kdtree-gcc4.2

test_kdtree-gcc3.4: test_kdtree.cpp
	g++-3.4 -I.. -Wall -ansi -pedantic -g -O2 -o test_kdtree-gcc3.4 test_kdtree.cpp

test_kdtree-gcc4.2: test_kdtree.cpp
	g++-4.2 -I.. -Wall -ansi -pedantic -g -O2 -o test_kdtree-gcc4.2 test_kdtree.cpp

test_kdtree-gcc4.3: test_kdtree.cpp
	g++-4.3 -I.. -Wall -ansi -pedantic -g -O2 -o test_kdtree-gcc4.3 test_kdtree.cpp

clean:
	rm -f test_kdtree

.PHONY: clean
