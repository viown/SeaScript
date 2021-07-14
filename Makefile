FLAGS = -Wall -O2 -g -std=c99
SEAVM = src/seavm
EXE_NAME = seascript.exe
CC = gcc
REQUIREMENTS = setup seascript.o compiler.o debug.o lex.o parser.o ssfunctions.o seavm/bytecode.o seavm/stack.o seavm/vm.o
TRGT = build/seascript.o build/compiler.o build/debug.o build/lex.o build/parser.o build/ssfunctions.o build/bytecode.o build/stack.o build/vm.o

release: $(REQUIREMENTS)
	$(CC) -o $(EXE_NAME) $(TRGT) -s

debug: $(REQUIREMENTS)
	$(CC) -o $(EXE_NAME) $(TRGT) -g

setup:
	if test -d build; then 										\
	echo Build folder already exists, no need to recreate;		\
	else														\
	mkdir build; 												\
    fi

seascript.o: src/seascript.c
	$(CC) $(FLAGS) -c src/seascript.c -o build/seascript.o

compiler.o: src/compiler.c
	$(CC) $(FLAGS) -c src/compiler.c -o build/compiler.o

debug.o: src/debug.c
	$(CC) $(FLAGS) -c src/debug.c -o build/debug.o

lex.o: src/lex.c
	$(CC) $(FLAGS) -c src/lex.c -o build/lex.o

parser.o: src/parser.c
	$(CC) $(FLAGS) -c src/parser.c -o build/parser.o

ssfunctions.o: src/ssfunctions.c
	$(CC) $(FLAGS) -c src/ssfunctions.c -o build/ssfunctions.o

seavm/bytecode.o: src/seavm/bytecode.c
	$(CC) $(FLAGS) -c $(SEAVM)/bytecode.c -o build/bytecode.o

seavm/stack.o: src/seavm/stack.c
	$(CC) $(FLAGS) -c $(SEAVM)/stack.c -o build/stack.o

seavm/vm.o: src/seavm/vm.c
	$(CC) $(FLAGS) -c $(SEAVM)/vm.c -o build/vm.o
	
clean:
	rm build/*.o
	rm seascript.exe