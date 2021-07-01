## SeaScript's Instruction Set

Note: The following instruction set has not been implemented yet, this is just for reference for me to implement later.
Subject to change (a lot).

| Instruction | Opcode  | Description  |
|---|---|---|
| EXIT  | 0x0  |  Terminates the process while returning an exit code
| SCONST  | 0x1  | Pushes a short onto the stack
| ICONST  | 0x2  | Pushes a 32-bit integer onto the stack
| LCONST | 0x3 | Pushes a long onto the stack
| DCONST | 0x4 | Pushes a double onto the stack
| FCONST | 0x5 | Pushes a float onto the stack
| CONSTPUSH | 0x6 | Pushes a constant onto the stack from the constant pool (either string or int)
| EQ | 0x7 | Tests if two integers are equal, if the number types are different, the smaller type will be cast
| GT | 0x8 | Tests for int_1 > int_2
| LT | 0x9 | Tests for int_1 < int_2
| NOT | 0xa | NOT operator
| IADD | 0xb | Adds two 32-bit integers
| ISUB | 0xc | Subtracts two 32-bit integers
| IMUL | 0xd | Multiplies two 32-bit integers
| IDIV | 0xe | Divides two 32-bit integers
| LADD | 0x10 | Adds two longs
| LSUB | 0x11 | Subtracts two longs
| LMUL | 0x12 | Multiplies two longs
| LDIV | 0x13 | Divides two longs
| DADD | 0x14 | Adds two doubles
| DSUB | 0x15 | Subtracts two doubles
| DMUL | 0x16 | Multiplies two doubles
| DDIV | 0x17 | Divides two doubles
| STORE | 0x18 | Stores an integer (of any type) as a global
| LOAD | 0x19 | Loads an integer onto the stack
| NEWARRAY | 0x1a | Creates a dynamic array capable of storing any type (note: arrays aren't available on the stack)
| ARRAYPUSH | 0x1b | Pushes a value onto an array
| ARRAYPOP | 0x1c | Pops the last value from the array
| CALLC | 0x1d | Calls a C function (note: The VM can't guarantee memory safety when this is used, that must be handled by the function itself)
| JUMP | 0x1e | Unconditional Jump
| JUMPIF | 0x1f | Jumps if true
| POP | 0x20 | Pops the last value on the stack

## Extra

You may notice that floats don't have math instructions, this is because DADD, DSUB, DMUL and DDIV should be used  instead for better precision.

There is also no type checking so the compiler should keep track of this.
