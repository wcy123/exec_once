PREFIX  = ../build
CFLAGS += -std=c99

C_SRCS += test_1.c
C_SRCS += test_2.c
C_SRCS += test_3.c
C_SRCS += test_4.c
C_SRCS += exec_once.c
C_SRCS += test_main.c
OBJS += $(patsubst %.c,%.o,$(C_SRCS))

all: test_main install

test_main: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^

run:
	LD_LIBRARY_PATH=. EXEC_ONCE_DEBUG=1 ./test_main


.PHONY: all clean
clean:
	rm *.o test_main

libexec_once.so: exec_once.o
	$(CC) -shared -Wl,-soname,$@  -o $@ $+ $(LDFLAGS)
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

.PHONY: depend
depend: 
	for i in $(C_SRCS); do $(CC) -MM $(CFLAGS) $$i; done > $@.inc

-include depend.inc
