#ifndef SEAVM_CPU_H
#define SEAVM_CPU_H
#include "stack.h"
#include "vm.h"

StackObject cpu_add(VirtualMachine* vm);
StackObject cpu_sub(VirtualMachine* vm);
StackObject cpu_mul(VirtualMachine* vm);
StackObject cpu_div(VirtualMachine* vm);

#endif
