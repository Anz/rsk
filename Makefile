
all: bin src/scanner.c src/parser.c
	@gcc -std=gnu99 src/scanner.c src/parser.c src/elf.c src/x86.c src/ir.c src/main.c -g -o bin/rev
	@bin/rev -o test/exec.o -i test/test.rev
	@gcc test/exec.o test/test.c -o test/prog

bin:
	@mkdir bin

src/yyscanner.c: src/scanner.l
	@flex -o src/yyscanner.c src/scanner.l
	
src/parser.c: src/parser.y
	@bison -d -o src/parser.c src/parser.y
