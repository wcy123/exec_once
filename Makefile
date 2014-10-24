PREFIX  = ../build
CFLAGS += -std=c99

all: install test_main.out 


test_main: test_1.o test_2.o test_3.o test_main.o
	$(CC) -o $@ $(LDFLAGS) $+ -lexec_once -L$(PREFIX)

#-Wl,-rpath,$(PREFIX)

test_main.o: exec_once.h

.PHONY: all clean

clean:
	rm *.o test_main test_main.out
test_main.out: test_main
	LD_LIBRARY_PATH=$(PREFIX) ./test_main > $@

libexec_once.so: exec_once.o
	$(CC) -shared -Wl,-soname,$@  -o $@ $+
exec_once.o: exec_once.c exec_once.h
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

install: libexec_once.so
	mkdir -p $(PREFIX) 2>/dev/null
	install libexec_once.so $(PREFIX)/
	install exec_once.h $(PREFIX)/exec_once.h

