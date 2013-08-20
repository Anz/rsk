
all: bin src/scanner.c src/parser.c src/x86_fun.c
	@gcc -std=gnu99 -D_GNU_SOURCE  -g -o bin/rev src/list.c src/map.c src/buffer.c src/ir.c src/scanner.c src/parser.c src/semantic.c src/i32.c src/main.c 

bin:
	@mkdir bin
	
src/x86_fun.c: src/x86_fun.s
	@cd src; xxd -i x86_fun.s x86_fun.c

src/yyscanner.c: src/scanner.l
	@flex -o src/yyscanner.c src/scanner.l
	
src/parser.c: src/parser.y
	@bison -d -o src/parser.c src/parser.y
