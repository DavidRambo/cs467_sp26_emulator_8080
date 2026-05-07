#include "CPU8080.h"

namespace intel_8080 {
// INR Increment Register or Memory value
//
// Condition bits affected: Zero, Sign, Parity
void CPU8080::inr(uint8_t* byte) {
  *byte += 1;
  update_flags_szp(*byte);
}

// RLC Rotate Accumulator Left
void CPU8080::rlc() {
  if ((registers_.reg_a & 0x08) == 0x08) {
    flags_.carry = 1;
  }
  registers_.reg_a = registers_.reg_a << 1;
  if (flags_.carry == 1) {
    registers_.reg_a++;
  }
}
}  // namespace intel_8080
