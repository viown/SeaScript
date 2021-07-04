GCC_VERSION = -std=c99
OPTIMIZATION = -O2
SEAVM = src/seavm
EXE_NAME = seascript.exe
CC = gcc

release: setup seascript.o compiler.o debug.o lex.o parser.o ssfunctions.o seavm/bytecode.o seavm/stack.o seavm/vm.o
	$(CC) -o $(EXE_NAME) build/seascript.o build/compiler.o build/debug.o build/lex.o build/parser.o build/ssfunctions.o build/bytecode.o build/stack.o build/vm.o -s
	
debug: setup seascript.o compiler.o debug.o lex.o parser.o ssfunctions.o seavm/bytecode.o seavm/stack.o seavm/vm.o
	$(CC) -o $(EXE_NAME) build/seascript.o build/compiler.o build/debug.o build/lex.o build/parser.o build/ssfunctions.o build/bytecode.o build/stack.o build/vm.o -g
	
setup:
	mkdir -p build
	
seascript.o: src/seascript.c
	$(CC) -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c src/seascript.c -o build/seascript.o
	
compiler.o: src/compiler.c
	$(CC) -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c src/compiler.c -o build/compiler.o
	
debug.o: src/debug.c
	$(CC) -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c src/debug.c -o build/debug.o
	
lex.o: src/lex.c
	$(CC) -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c src/lex.c -o build/lex.o
	
parser.o: src/parser.c
	$(CC) -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c src/parser.c -o build/parser.o
	
ssfunctions.o: src/ssfunctions.c
	$(CC) -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c src/ssfunctions.c -o build/ssfunctions.o
	
seavm/bytecode.o: src/seavm/bytecode.c
	$(CC) -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c $(SEAVM)/bytecode.c -o build/bytecode.o
	
seavm/stack.o: src/seavm/stack.c
	$(CC) -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c $(SEAVM)/stack.c -o build/stack.o
	
seavm/vm.o: src/seavm/vm.c
	$(CC) -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c $(SEAVM)/vm.c -o build/vm.o
	
clean:
	rm build/*.o
	rm seascript.exe
