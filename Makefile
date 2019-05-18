.PHONY: all
all: load

load: loader.c changeFlag test
	gcc -m32 loader.c -o load -T elf_i386.x -static -Wall
	./changeFlag load

changeFlag: changeFlag.c
	gcc -m32 changeFlag.c -o changeFlag

test: test.c
	gcc -m32 test.c -o test -static -Wall

.PHONY: clean
clean:
	rm changeFlag load test

