#ifndef SEAVM_BYTECODE_H
#define SEAVM_BYTECODE_H
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include "opcodes.h"
#include "instruction.h"
#include "stack.h"
#if __has_include("conf.h")
#include "conf.h"
#else
#define VERSION "0.0.0"
#endif

#define STRING_POOL 177

typedef struct {
    char** constants;
    size_t size;
    size_t length;
} StringPool;

StringPool create_string_pool();
size_t string_exists(StringPool* pool, char* str);
size_t push_to_pool(StringPool* pool, char* str);
void free_string_pool(StringPool* pool);

typedef struct {
    Opcode op;
    int bytes_to_read;
} OpcodeReader;

static const OpcodeReader reader_map[] = {
    {NOP, 0},
    {EXIT, sizeof(int)},
    {LOADBOOL, sizeof(bool)},
    {LOADC, sizeof(double)},
    {LOADPOOL, sizeof(int32_t)},
    {POP, 0},
    {INC, 0},
    {DEC, 0},
    {EQ, 0},
    {LT, 0},
    {GT, 0},
    {JUMP, sizeof(int32_t)},
    {JUMPIF, sizeof(int32_t)},
    {NOT, 0},
    {ADD, 0},
    {SUB, 0},
    {MUL, 0},
    {DIV, 0},
    {CALL, sizeof(int32_t)},
    {RET, 0},
    {CALLC, sizeof(int32_t)},
    {STORE, sizeof(int32_t)},
    {LOAD, sizeof(int32_t)},
    {LBL, sizeof(int32_t)},
    {LBLJMP, sizeof(int32_t)},
    {LBLJMPIF, sizeof(int32_t)},
};

const OpcodeReader* get_reader(Opcode op);
void save_to_file(Instruction* instructions, StringPool* pool, size_t length, const char* path);
InstructionHolder read_from_file(const char* path, StringPool* pool);
long get_file_size(const char* path);
void free_holder(InstructionHolder* holder);

#endif // SEAVM_BYTECODE_H
