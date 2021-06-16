GCC_VERSION = -std=c99
OPTIMIZATION = -O2
SEAVM = src/seavm
EXE_NAME = seascript.exe

output: setup seascript.o compiler.o debug.o lex.o parser.o ssfunctions.o seavm/bytecode.o seavm/stack.o seavm/vm.o
	gcc -o build/$(EXE_NAME) build/seascript.o build/compiler.o build/debug.o build/lex.o build/parser.o build/ssfunctions.o build/bytecode.o build/stack.o build/vm.o -s
	make clean
	
setup:
	mkdir -p build
	
seascript.o: src/seascript.c
	gcc -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c src/seascript.c -o build/seascript.o
	
compiler.o: src/compiler.c
	gcc -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c src/compiler.c -o build/compiler.o
	
debug.o: src/debug.c
	gcc -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c src/debug.c -o build/debug.o
	
lex.o: src/lex.c
	gcc -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c src/lex.c -o build/lex.o
	
parser.o: src/parser.c
	gcc -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c src/parser.c -o build/parser.o
	
ssfunctions.o: src/ssfunctions.c
	gcc -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c src/ssfunctions.c -o build/ssfunctions.o
	
seavm/bytecode.o: src/seavm/bytecode.c
	gcc -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c $(SEAVM)/bytecode.c -o build/bytecode.o
	
seavm/stack.o: src/seavm/stack.c
	gcc -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c $(SEAVM)/stack.c -o build/stack.o
	
seavm/vm.o: src/seavm/vm.c
	gcc -Wall $(OPTIMIZATION) -g $(GCC_VERSION) -c $(SEAVM)/vm.c -o build/vm.o
	
clean:
	rm build/*.o
