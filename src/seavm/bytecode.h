#ifndef SEAVM_BYTECODE_H
#define SEAVM_BYTECODE_H
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include "opcodes.h"
#include "instruction.h"
#include "stack.h"


typedef struct {
    Opcode op;
    int bytes_to_read;
} OpcodeReader;

static const OpcodeReader reader_map[] = {
    {EXIT, sizeof(int)},
    {LOADBOOL, sizeof(bool)},
    {ICONST, sizeof(int32_t)},
    {LCONST, sizeof(int64_t)},
    {DCONST, sizeof(double)},
    {CAST, 1},
    {POP, 0},
    {INC, 0},
    {DEC, 0},
    {EQ, 0},
    {LT, 0},
    {GT, 0},
    {JUMP, sizeof(int32_t)},
    {JUMPIF, sizeof(int32_t)},
    {NOT, 0},
    {IADD, 0},
    {ISUB, 0},
    {IMUL, 0},
    {IDIV, 0},
    {CALL, sizeof(int32_t)},
    {RET, 0},
    {CALLC, sizeof(int32_t)},
    {STORE, sizeof(int32_t)},
    {LOAD, sizeof(int32_t)},
    {LBL, sizeof(int32_t)},
    {LBLJMP, sizeof(int32_t)},
    {LBLJMPIF, sizeof(int32_t)},
    {IPRINT, 0},
};

const OpcodeReader* get_reader(Opcode op);

void save_to_file(Instruction* instructions, size_t length, const char* path);
InstructionHolder read_from_file(const char* path);
long get_file_size(const char* path);

void free_holder(InstructionHolder* holder);

#endif // SEAVM_BYTECODE_H
