CC = gcc
CLFAGS = -Wall -std=c99 -O2
FILES = $(wildcard src/*.c)
FILES += $(wildcard src/*/*.c) 
OBJ = $(patsubst %.c, %.o, $(FILES))
EXEC = seascript


all: $(EXEC)

debug: all
debug: CFLAGS += -DDEBUG -g -fsanitize=memory

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@

%.o: src/%.c src/seavm/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm src/*.o
	rm src/*/*.o
	rm $(EXEC)
