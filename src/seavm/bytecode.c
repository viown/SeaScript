#include "vm.h"
#include "bytecode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#include <sys/stat.h>
#endif // __linux__

#define LEN(x) (sizeof(x) / sizeof(x[0]))

StringPool create_string_pool() {
    StringPool pool;
    pool.constants = (char**)ss_malloc(10 * sizeof(char*));
    pool.size = 10;
    pool.length = 0;
    return pool;
}

size_t string_exists(StringPool* pool, char* str) {
    for (size_t i = 0; i < pool->length; ++i) {
        if (!strcmp(pool->constants[i], str))
            return i;
    }
    return -1;
}

size_t count_pool_size(StringPool* pool) {
    size_t counter = 0;
    for (size_t i = 0; i < pool->length; ++i) {
        counter += strlen(pool->constants[i]) + 1;
    }
    return counter;
}

size_t push_to_pool(StringPool* pool, char* str) {
    if (pool->size == pool->length) {
        pool->size *= 2;
        pool->constants = realloc(pool->constants, pool->size * sizeof(char*));
    }
    pool->constants[pool->length++] = str; // note: must be heap allocated
    return pool->length - 1;
}

void free_string_pool(StringPool* pool) {
    for (size_t i = 0; i < pool->length; ++i) {
        free_and_null(pool->constants[i]);
    }
    free_and_null(pool->constants);
    pool->length = -1;
}

const OpcodeReader* get_reader(Opcode op) {
    for (int i = 0; i < LEN(reader_map); ++i) {
        if (reader_map[i].op==op)
            return &reader_map[i];
    }
    return NULL;
}

void write_byte(unsigned char* bytecode, size_t* cursor, uint8_t byte) {
    bytecode[(*cursor)++] = byte;
}

void write_bytes(Instruction* instruction, unsigned char* bytecode, size_t* cursor, uint16_t bytes_to_read) {
    arg_type arg = instruction->args[0];
    uint8_t* d = (uint8_t*)&arg;
    for (int i = 0; i < bytes_to_read; ++i) {
        write_byte(bytecode, cursor, *d++);
    }
}

void append_string(unsigned char* bytecode, size_t* cursor, char* string) {
    for (int i = 0; i < strlen(string); ++i) {
        bytecode[(*cursor)++] = string[i];
    }
    bytecode[++(*cursor)] = '\0';
}

void save_to_file(Instruction* instructions, StringPool* pool, size_t length, const char* path) {
    FILE* file = fopen(path, "wb");
    unsigned char* bytecode = (unsigned char*)ss_malloc(length * 8 + count_pool_size(pool));
    size_t size = length * 8;
    size_t cursor = 0;
    const char* version = VERSION;
    for (int i = 0; i < strlen(version); ++i) {
        bytecode[cursor++] = version[i];
    }
    for (size_t i = 0; i < length; ++i) {
        Instruction* instruction = &instructions[i];
        bytecode[cursor++] = instruction->op;
        const OpcodeReader* reader = get_reader(instruction->op);
        if (reader == NULL) {
            ss_throw("Invalid instruction '%s'\n", instruction_to_string(instruction->op));
        }
        write_bytes(instruction, bytecode, &cursor, reader->bytes_to_read);
    }
    if (pool->length != 0) {
        write_byte(bytecode, &cursor, STRING_POOL);
        for (size_t i = 0; i < pool->length; ++i) {
            append_string(bytecode, &cursor, pool->constants[i]);
        }
    }
    fwrite(bytecode, 1, cursor, file);
    fclose(file);
    free(bytecode);
}

void push_holder_instruction(InstructionHolder* holder, Instruction instruction) {
    if (holder->length == holder->size) {
        holder->size *= 2;
        holder->instructions = realloc(holder->instructions, holder->size * sizeof(Instruction));
    }
    holder->instructions[holder->length++] = instruction;
}

void free_holder(InstructionHolder* holder) {
    free_and_null(holder->instructions);
    holder->size = -1;
    holder->length = -1;
}

void load_version(unsigned char* bytecode, char* version) {
    for (int i = 0; i < 6; ++i) {
        version[i] = bytecode[i];
    }
    version[5] = '\0';
}

InstructionHolder read_from_file(const char* path, StringPool* pool) {
    InstructionHolder holder;
    holder.instructions = ss_malloc(100 * sizeof(Instruction));
    holder.size = 100;
    holder.length = 0;
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        ss_throw("Invalid path '%s'\n", path);
    }
    long byte_size = get_file_size(path);
    int cursor = 5;
    unsigned char* bytecode = (unsigned char*)ss_malloc(byte_size + 1);
    fread(bytecode, 1, byte_size, file);
    char version[10];
    load_version(bytecode, version);
    if (strcmp(version, VERSION) != 0) {
        ss_throw("This bytecode file uses Seascript v%s, which differs from the current version (%s). Running it can be dangerous.\nHalt.\n", version, VERSION);
    }
    while (cursor != byte_size) {
        if (bytecode[cursor] == STRING_POOL) {
            break;
        }
        Instruction instruction;
        instruction.op = bytecode[cursor++];
        const OpcodeReader* reader = get_reader(instruction.op);
        int bytes_to_read = reader->bytes_to_read;
        unsigned char bytes[bytes_to_read];
        for (int i = 0; i < bytes_to_read; ++i) {
            bytes[i] = bytecode[cursor++];
        }
        arg_type argument = 0; /* TODO: Support for multiple arguments, but for now we don't need them. */
        memcpy(&argument, bytes, bytes_to_read);
        instruction.args[0] = argument;
        push_holder_instruction(&holder, instruction);
    }
    if (bytecode[cursor] == STRING_POOL) {
        cursor++;
        char* string = malloc(100);
        size_t string_size = 100;
        size_t length = 0;
        while (cursor != byte_size) {
            do {
                if (length == string_size) {
                    string_size *= 2;
                    string = realloc(string, string_size);
                }
                string[length++] = bytecode[cursor];
            } while (bytecode[cursor++] != '\0');
            string[length++] = '\0';
            push_to_pool(pool, string);
            string = malloc(100);
            string_size = 100;
            length = 0;
        }
    }
    fclose(file);
    free(bytecode);
    return holder;
}

long get_file_size(const char* path) {
#ifdef __linux__
    struct stat st;
    stat(path, &st);
    return st.st_size;
#else
    FILE *file = fopen(path, "r");
    long size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    fclose(file);
    return size;
#endif
}
