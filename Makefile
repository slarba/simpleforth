
CONFFLAGS=-DUSE_GC=1 -DSAFE_INTERPRETER=1
CC = gcc-4.3
# PROF=-ftest-coverage -fprofile-arcs
PROF=
INCLUDES=-I./boehmgc/include -I./dyncall/include $(PROF)
CFLAGS = -Wall $(CONFFLAGS) -O4 $(INCLUDES) $(PROF)
LDFLAGS = -L./boehmgc/lib -L./dyncall/lib $(PROF) -lgc -lreadline -ldyncall_s -ldynload_s

all: boehmgc dyncall forth.S forth

clean:
	rm -f forth core gmon.out *.gcov *.gcno *.gcda *.o *.S *~

reallyclean: clean
	rm -rf dyncall dyncall-0.6
	rm -rf boehmgc gc-7.1

forth.S: forth.c bytecodes.h
	$(CC) -c -S $(CFLAGS) -o forth.S forth.c

forth.o: forth.c bytecodes.h

forth: forth.o
	$(CC) -o $@ $< $(LDFLAGS)

dyncall:
	tar xzvf dyncall-0.6.tar.gz
	(cd dyncall-0.6; ./configure --prefix=`pwd`/../dyncall; make; make install)

boehmgc:
	tar xzvf gc-7.1.tar.gz
	(cd gc-7.1; ./configure --prefix=`pwd`/../boehmgc; make; make install)
