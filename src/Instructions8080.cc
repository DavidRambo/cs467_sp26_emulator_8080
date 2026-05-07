#include "CPU8080.h"

namespace intel_8080 {
// INR Increment Register or Memory value
//
// Condition bits affected: Zero, Sign, Parity
void CPU8080::inr(uint8_t* byte) {
  *byte += 1;
  update_flags_szp(*byte);
}

// MOV Move data byte into register or memory address
void CPU8080::mov(uint8_t* addr, uint8_t data) { *addr = data; }

// RLC Rotate Accumulator Left
void CPU8080::rlc() {
  if ((registers_.reg_a & 0x80) == 0x80) {
    flags_.carry = 1;
  }
  registers_.reg_a = registers_.reg_a << 1;
  if (flags_.carry == 1) {
    registers_.reg_a++;
  }
}
}  // namespace intel_8080
