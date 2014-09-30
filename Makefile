all: test_exec_once_1 


test_exec_once_1: test_exec_once_1.o
test_exec_once_1.o: exec_once.h

.PHONY: all clean

clean:
	rm *.o test_exec_once_1 test_exec_once_1.out
test_exec_once_1.out:
	./test_exec_once_1 > $@