CC = gcc
CFLAGS = -Wall -O2 -std=c99
OUTPUT = seascript.exe

SOURCES = \
	src/compiler.c \
	src/debug.c \
	src/lex.c \
	src/parser.c \
	src/seascript.c \
	src/ssfunctions.c \
	src/seavm/bytecode.c \
	src/seavm/stack.c \
	src/seavm/vm.c 
	
HEADERS = \
	src/compiler.h \
	src/debug.h \
	src/lex.h \
	src/parser.h \
	src/ssfunctions.h \
	src/seavm/bytecode.h \
	src/seavm/instruction.h \
	src/seavm/opcodes.h \
	src/seavm/stack.h \
	src/seavm/vm.h
	
$(OUTPUT): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTPUT) -s
	
debug: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTPUT) -g
	
debug_mem: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTPUT) -fsanitize=address -static-libasan -g