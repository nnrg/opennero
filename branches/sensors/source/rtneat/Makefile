RTNEAT_CPP=$(wildcard *.cpp)
RTNEAT_H=$(wildcard *.h)
RTNEAT_O=$(RTNEAT_CPP:.cpp=.o)
RTNEAT_LIB=librtneat.a
TARGETS=$(RTNEAT_LIB)
CXXFLAGS = -g

all: $(TARGETS)

$(RTNEAT_LIB): $(RTNEAT_O)
	$(AR) ru $@ $(RTNEAT_O)
	ranlib $@

clean:
	@rm -f $(RTNEAT_O) *~ 

deepclean: clean
	@rm -f $(TARGETS)

