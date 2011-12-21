
CONFFLAGS=-DUSE_GC=1 -DSAFE_INTERPRETER=1
# CC = gcc # -4.5
# PROF=-ftest-coverage -fprofile-arcs
PROF=
INCLUDES=-I./boehmgc/include -I./dyncall/include $(PROF)
CFLAGS = -Wall $(CONFFLAGS) -O4 $(INCLUDES) $(PROF)
LDFLAGS = -L./boehmgc/lib -L./dyncall/lib $(PROF) -lm -lgc -lreadline -ldyncall_s -ldynload_s
INSTALL = install
prefix=""
bindir = $(prefix)/usr/bin

all: dyncall boehmgc forth

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

install: forth
	$(INSTALL) forth $(bindir)

dyncall:
	tar xzvf dyncall-0.6.tar.gz
	(cd dyncall-0.6; ./configure --prefix=`pwd`/../dyncall; make; make install)

boehmgc:
	tar xzvf gc-7.1.tar.gz
	(cd gc-7.1; ./configure --disable-gcj-support --host=i686-linux --prefix=`pwd`/../boehmgc; make; make install)
