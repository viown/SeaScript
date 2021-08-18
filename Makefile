IGNORED_WARNINGS = \
	-Wno-unused-result

CC = gcc
CFLAGS = $(IGNORED_WARNINGS) -Os -std=c99
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
	src/seavm/vm.c  \
	src/seavm/cpu.c \
	src/shell/shell.c
	
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
	src/seavm/vm.h \
	src/seavm/cpu.h \
	src/shell/shell.h
	
INCLUDES = \
	-Isrc \
	-Isrc/seavm \
	-Isrc/shell
	
seascript: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $@ -s
	
debug: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $(OUTPUT) -g
	
debug_mem: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $(OUTPUT) -fsanitize=address -static-libasan -g
	
.PHONY: seascript
