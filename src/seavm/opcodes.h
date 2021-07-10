#ifndef SEAVM_OPCODES_H
#define SEAVM_OPCODES_H
#include "./stack.h"

typedef unsigned char Opcode;

#define EXIT        0x0
#define ICONST      0x2
#define LCONST      0x3
#define DCONST      0x4
#define CAST        0x5
#define EQ          0x6
#define IPRINT      0x11


#endif // SEAVM_OPCODES_H
