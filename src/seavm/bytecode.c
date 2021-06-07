#include "./bytecode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#include <sys/stat.h>
#endif // __linux__

void to_bytecode(Bytecode* bytecode, Instruction* instructions, size_t length) {
    bytecode->length = length;
    stack_type* buffer = (stack_type*)malloc((length * 4) * sizeof(stack_type));
    int cursor = 0;
    for (size_t i = 0; i < length; ++i) {
        Instruction current_instruction = instructions[i];
        buffer[cursor] = current_instruction.op;
        buffer[++cursor] = current_instruction.args[0];
        buffer[++cursor] = current_instruction.args[1];
        buffer[++cursor] = current_instruction.args[2];
        cursor++;
    }
    bytecode->raw_data = buffer;
}

Instruction* to_instructions(Bytecode* bytecode) {
    Instruction* instructions = (Instruction*)malloc(sizeof(Instruction) * bytecode->length);
    int cursor = 0;
    for (size_t i = 0; i < bytecode->length; ++i) {
        Instruction instruction;
        instruction.op = bytecode->raw_data[cursor];
        instruction.args[0] = bytecode->raw_data[++cursor];
        instruction.args[1] = bytecode->raw_data[++cursor];
        instruction.args[2] = bytecode->raw_data[++cursor];

        cursor++;
        instructions[i] = instruction;
    }
    return instructions;
}

void save_to_file(Bytecode* bytecode, const char* path) {
    FILE* file = fopen(path, "wb");
    int cursor = 0;
    for (size_t i = 0; i < bytecode->length; ++i) {
        stack_type buffer[4 /* * sizeof(stack_type) */] = {bytecode->raw_data[cursor], bytecode->raw_data[++cursor], bytecode->raw_data[++cursor], bytecode->raw_data[++cursor]};
        cursor++;
        fwrite(buffer, sizeof(buffer), 1, file);
    }
    fclose(file);
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

void read_from_file(Bytecode* bytecode, const char* path) {
    FILE* file = fopen(path, "rb");
    long file_size = get_file_size(path);
    stack_type buffer[file_size / sizeof(stack_type)];
    fread(buffer, sizeof(buffer), 1, file);
    bytecode->length = ((file_size) / sizeof(stack_type)) / 4;
    stack_type* raw_data = (stack_type*)malloc(file_size);
    memcpy(raw_data, buffer, file_size);
    bytecode->raw_data = raw_data;
}

void free_bytecode(Bytecode* bytecode) {
    free(bytecode->raw_data);
}
