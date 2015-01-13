CXX      := g++
LD       := g++
CPPFLAGS := -O3 -Wall -std=c++11 -g
LIBS     := -lrt -lnl
LIBSSL   := -lssl -lcrypto

all: reactivejam channelmitm constantjam fastreply

clean:
	rm -rf *.o *.d *~ reactivejam channelmitm

.PHONY: clean all


SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)
include $(OBJ:%.o=%.d)
%.d: %.cpp
	./depend.sh $(CPPFLAGS) $*.cpp > $@

reactivejam: reactivejam.o osal_wi.o osal_nl.o util.o MacAddr.o crc.o
	$(LD) $(LIBS) reactivejam.o osal_wi.o osal_nl.o util.o MacAddr.o crc.o -o reactivejam

channelmitm: channelmitm.o osal_wi.o osal_nl.o util.o MacAddr.o crc.o ClientInfo.o SeqnumType.o SeqnumStats.o eapol.o crypto.o pcap.o chopstate.o
	$(LD) $(LIBS) $(LIBSSL) channelmitm.o osal_wi.o osal_nl.o util.o MacAddr.o crc.o ClientInfo.o SeqnumType.o SeqnumStats.o eapol.o crypto.o pcap.o chopstate.o -o channelmitm

constantjam: constantjam.o osal_wi.o osal_nl.o util.o MacAddr.o crc.o
	$(LD) $(LIBS) constantjam.o osal_wi.o osal_nl.o util.o MacAddr.o crc.o -o constantjam

fastreply: fastreply.o osal_wi.o osal_nl.o util.o MacAddr.o crc.o
	$(LD) $(LIBS) fastreply.o osal_wi.o osal_nl.o util.o MacAddr.o crc.o -o fastreply

