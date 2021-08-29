IGNORED_WARNINGS = \
	-Wno-unused-result

CC = gcc
CFLAGS = $(IGNORED_WARNINGS) -Os -std=c99

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
	src/shell/shell.c
	
HEADERS = \
	include/compiler.h \
	include/debug.h \
	include/lex.h \
	include/parser.h \
	include/ssfunctions.h \
	include/seavm/bytecode.h \
	include/seavm/instruction.h \
	include/seavm/opcodes.h \
	include/seavm/stack.h \
	include/seavm/vm.h \
	include/shell/shell.h
	
INCLUDES = \
	-Iinclude \
	-Iinclude/seavm \
	-Iinclude/shell \
	
seascript: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -lreadline -o seascript -s
	
debug: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -lreadline -o seascript -g
	
debug_mem: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -lreadline -o seascript -fsanitize=address -static-libasan -g
	
.PHONY: seascript debug debug_mem
