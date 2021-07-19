#include "../debug.h"
#include "./vm.h"
#include "./bytecode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#include <sys/stat.h>
#endif // __linux__

#define LEN(x) (sizeof(x) / sizeof(x[0]))

const OpcodeReader* get_reader(Opcode op) {
    for (int i = 0; i < LEN(reader_map); ++i) {
        if (reader_map[i].op==op)
            return &reader_map[i];
    }
    return NULL;
}

void write_bytes(Instruction* instruction, char* bytecode, size_t* cursor, uint16_t bytes_to_read) {
    arg_type arg = instruction->args[0];
    uint8_t* d = (uint8_t*)&arg;
    for (int i = 0; i < bytes_to_read; ++i) {
        bytecode[(*cursor)++] = *d++;
    }
}

void save_to_file(Instruction* instructions, size_t length, const char* path) {
    FILE* file = fopen(path, "wb");
    char* bytecode = (char*)ss_malloc(length * 8);
    size_t cursor = 0;
    bytecode[cursor++] = 0x32;
    for (size_t i = 0; i < length; ++i) {
        Instruction* instruction = &instructions[i];
        bytecode[cursor++] = instruction->op;
        const OpcodeReader* reader = get_reader(instruction->op);
        if (reader == NULL) {
            ss_throw("Invalid instruction '%s'", instruction_to_string(instruction->op));
        }
        write_bytes(instruction, bytecode, &cursor, reader->bytes_to_read);
    }
    fwrite(bytecode, 1, cursor, file);
    fclose(file);
    free(bytecode);
}

void push_holder_instruction(InstructionHolder* holder, Instruction instruction) {
    if (holder->length == holder->size) {
        holder->size += 25;
        holder->instructions = realloc(holder->instructions, holder->size);
    }
    holder->instructions[holder->length++] = instruction;
}

void free_holder(InstructionHolder* holder) {
    free_and_null(holder->instructions);
    holder->size = -1;
    holder->length = -1;
}

InstructionHolder read_from_file(const char* path) {
    InstructionHolder holder;
    holder.instructions = ss_malloc(100 * sizeof(Instruction));
    holder.size = 100;
    holder.length = 0;
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        ss_throw("Invalid path '%s'", path);
    }
    long byte_size = get_file_size(path);
    int cursor = 0;
    unsigned char* bytecode = (unsigned char*)ss_malloc(byte_size + 1);
    fread(bytecode, 1, byte_size, file);
    if (bytecode[cursor++] != 0x32) {
        ss_throw("Invalid bytecode file, missing magic number.");
    }
    while (cursor != byte_size) {
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
