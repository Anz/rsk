
all: bin src/scanner.c src/parser.c
	@gcc -std=gnu99 src/list.c src/map.c src/buffer.c src/scanner.c src/parser.c src/semantic.c src/elf.c src/x86.c src/main.c -g -o bin/rev
	@bin/rev -o test/prog -i test/test.rev

bin:
	@mkdir bin

src/yyscanner.c: src/scanner.l
	@flex -o src/yyscanner.c src/scanner.l
	
src/parser.c: src/parser.y
	@bison -d -o src/parser.c src/parser.y
