
CC = gcc-4.3
CFLAGS = -O4

all: forth

clean:
	rm -f forth core *.o *~

forth.o: forth.c bytecodes.h

forth: forth.o
