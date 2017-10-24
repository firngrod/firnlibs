IDIR=firnlibs
GPP=g++
#CFLAGS=-I$(IDIR) $(shell pkg-config --cflags opencv) -pthread -std=c++14 -O2
CFLAGS=-pthread -std=c++14 -O3

ODIR=obj
LDIR=

#LIBS=-lgmpxx -lgmp -lmpfr -ljsoncpp $(shell pkg-config --libs opencv) -lcryptopp
LIBS=-lcryptopp -ljsoncpp

#_DEPS=complex.hpp mandelbrot.hpp buildimage.hpp colors.hpp workerthread.hpp CyclicVar.hpp saveroutine.hpp cryptozip.hpp halfspan.hpp
_DEPS=
DEPS=$(LDEPS)

_LDEPS=
LDEPS=$(patsubst %,$(IDIR)/%,$(_LDEPS))

_OBJ=pieplayer.o firnlibs/crypto/aes.o firnlibs/crypto/zip.o firnlibs/crypto/general.o firnlibs/threading/threadpool.o firnlibs/networking/networking.o firnlibs/networking/listener.o firnlibs/networking/client.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS) $(LDEPS)
	mkdir -p $(dir $(@))
	$(GPP) -c -o $@ $< $(CFLAGS)

pieplayer: $(OBJ)
	$(GPP) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY:clean

clean:
	rm -rf $(ODIR)/*

test: clean pieplayer
	./pieplayer
