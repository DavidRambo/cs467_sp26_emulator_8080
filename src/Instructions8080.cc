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
//
// Left shifts accumulator and uses carry bit to "rotate" most-significant bit
// to least-significant bit.
//
// Condition bits affected: carry.
void CPU8080::rlc() {
  // Set or clear carry bit.
  flags_.carry = registers_.reg_a >> 7;

  registers_.reg_a = registers_.reg_a << 1;

  if (flags_.carry) {
    registers_.reg_a++;
  }
}

// STAX: Store Accumulator
// Contents of A are stoed in the memory location from either BC, or DE.
// No flags affected
void CPU8080::stax(uint16_t mem_location) {
  mem_access_->write(mem_location, registers_.reg_a);
}

// LDAX: Load Accumulator
// Contents of memory location from either BC, or DE are stored in Accumulator
// No flags affected
void CPU8080::ldax(uint16_t mem_location) {
  registers_.reg_a = mem_access_->read(mem_location);
}

}  // namespace intel_8080
