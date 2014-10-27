PREFIX  = ../build
CFLAGS += -std=c99

all: test_main install


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


$(PREFIX): 
	mkdir -p $(PREFIX) 2>/dev/null

INSTALL_FILES += $(PREFIX)/libexec_once.so
INSTALL_FILES += $(PREFIX)/exec_once.h

$(PREFIX)/libexec_once.so: libexec_once.so  $(PREFIX)
	install libexec_once.so $(PREFIX)/
$(PREFIX)/exec_once.h: exec_once.h $(PREFIX)
	install exec_once.h $(PREFIX)/exec_once.h

install: $(INSTALL_FILES)
