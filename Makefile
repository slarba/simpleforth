
CONFFLAGS=-DUSE_GC=1 -DSAFE_INTERPRETER=1
CC = gcc-4.3
# PROF=-ftest-coverage -fprofile-arcs
PROF=
CFLAGS = -Wall $(CONFFLAGS) -O4 -I./gc/boehmgc/include $(PROF)
LDFLAGS = -L./gc/boehmgc/lib $(PROF) -lgc

all: forth.S forth forth

clean:
	rm -f forth core gmon.out *.gcov *.gcno *.gcda *.o *.S *~

forth.S: forth.c bytecodes.h
	$(CC) -c -S $(CFLAGS) -o forth.S forth.c

forth.o: forth.c bytecodes.h

forth: forth.o
