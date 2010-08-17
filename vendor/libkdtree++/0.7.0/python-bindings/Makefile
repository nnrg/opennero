
INCLUDE_DIR=..
CXX=g++

# CPPFLAGS is used by the default rules. Using "override" and "+="
# allows the user to prepend things to CPPFLAGS on the command line.
override CPPFLAGS += -I$(INCLUDE_DIR) -pedantic -Wno-long-long -Wall -ansi -pedantic
# These options are set by the configure script.
override CPPFLAGS += -DHAVE_CONFIG_H

ifeq ($(MINUSG),1)
override CPPFLAGS += -g
else
override CPPFLAGS += -O3
endif

ifeq ($(MINUSPG),1)
override CPPFLAGS += -pg
endif


# swig bindings
py-kdtree: _kdtree.so
	cp _kdtree.so kdtree.py ../../

py-kdtree_test: py-kdtree.hpp py-kdtree_test.cpp
	$(CXX) $(CPPFLAGS) -I/usr/include/python2.5 -c -o py-kdtree_test.o py-kdtree_test.cpp
	$(CXX) $(CPPFLAGS) $(LDLIBS) py-kdtree_test.o -o py-kdtree_test

py-kdtree_wrap.cxx: py-kdtree.i py-kdtree.hpp
	swig -python -modern -c++ py-kdtree.i


_kdtree.so: py-kdtree_wrap.cxx 
	$(CXX) $(CPPFLAGS) -c py-kdtree_wrap.cxx -I/usr/include/python2.5 -I$(INCLUDE_DIR)
	$(CXX) $(CPPFLAGS) -shared  py-kdtree_wrap.o $(LDLIBS) -o _kdtree.so

clean:
	rm -f test_kdtree *.so py-kdtree_wrap.cxx *.o py-kdtree_test kdtree.py *.pyc

.PHONY: clean

