CXX      := g++
LD       := g++
CPPFLAGS := -O3 -Wall -std=c++11 -g
LIBS     := -lrt -lnl

all: reactivejam

clean:
	rm -rf *.o *.d *~ reactivejam

.PHONY: clean all


SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)
include $(OBJ:%.o=%.d)
%.d: %.cpp
	./depend.sh $(CPPFLAGS) $*.cpp > $@

reactivejam: reactivejam.o osal_wi.o osal_nl.o util.o MacAddr.o crc.o
	$(LD) $(LIBS) reactivejam.o osal_wi.o osal_nl.o util.o MacAddr.o crc.o -o reactivejam

