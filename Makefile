
CONFFLAGS=-DUSE_GC=1 -DSAFE_INTERPRETER=1
CC = gcc-4.3
# PROF=-ftest-coverage -fprofile-arcs
PROF=
INCLUDES=-I./gc/boehmgc/include -I./dyncall/include $(PROF)
CFLAGS = -Wall $(CONFFLAGS) -O4 $(INCLUDES) $(PROF)
LDFLAGS = -L./gc/boehmgc/lib -L./dyncall/lib $(PROF) -lgc -lreadline -ldyncall_s -ldynload_s

all: forth.S forth

clean:
	rm -f forth core gmon.out *.gcov *.gcno *.gcda *.o *.S *~

forth.S: forth.c bytecodes.h
	$(CC) -c -S $(CFLAGS) -o forth.S forth.c

forth.o: forth.c bytecodes.h

forth: forth.o
	$(CC) -o $@ $< $(LDFLAGS)
