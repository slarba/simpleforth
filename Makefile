
CC = gcc-4.3
CFLAGS = -I./gc/boehmgc/include -O4
LDFLAGS = -L./gc/boehmgc/lib -lgc

all: forth

clean:
	rm -f forth core *.o *~

forth.o: forth.c bytecodes.h

forth: forth.o
