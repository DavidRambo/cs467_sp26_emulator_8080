#include <cstdint>

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
// Flags affected: N/A
void CPU8080::stax(uint16_t mem_location) {
  mem_access_->write(mem_location, registers_.reg_a);
}

// LDAX: Load Accumulator
// Contents of memory location from either BC, or DE are stored in Accumulator
// Flags affected: N/A
void CPU8080::ldax(uint16_t mem_location) {
  registers_.reg_a = mem_access_->read(mem_location);
}

// CMC: Complement carry
// Flips the value of Carry flag.
// Flags affected: Carry
void CPU8080::cmc() { flags_.carry = flags_.carry ^ 0x01; }

// STC: Set carry
// Sets the carry to 1
// Flags affected: Carry
void CPU8080::stc() { flags_.carry = 1; }

// DCR: Decrement Register/Memory
// Reduces the value of the register or memory by 1.
// Flags affected: Zero, Sign, Parity, Aux Carry.
void CPU8080::dcr(uint8_t* byte) {
  *byte -= 1;
  update_flags_szp(*byte);
}

// CMA: Complement Accumulator
// Each bit of the accumulator is complemented
// Flags affected: N/A
void CPU8080::cma() { registers_.reg_a = ~registers_.reg_a; }

// ADD: Add Register or Memory to Accumulator
// Adds the specified byte to A and the result is stored in A using two's
// complement arithmetic Flags affected: Carry, Sign, Zero, Parity, Aux Carry
void CPU8080::add(uint8_t data) {
  uint16_t result = registers_.reg_a + data;
  flags_.carry = (result > 0xFF) ? 1 : 0;
  registers_.reg_a = (uint8_t)result;
  update_flags_szp(registers_.reg_a);
}

// ADC: Add Register or Memory To Accumulator w/ Carry
// Adds the specified byte + the carry flag to A and store in A.
// Flags affected: Carry, Sign, Zero, Parity, Aux Carry
void CPU8080::adc(uint8_t data) {
  data += flags_.carry;
  uint16_t result = registers_.reg_a + data + flags_.carry;
  flags_.carry = (result > 0xFF) ? 1 : 0;
  registers_.reg_a = (uint8_t)result;
  update_flags_szp(registers_.reg_a);
}

// SUB: Subtract Register or Memory From Accumulator
// The specified byte is subtracted from A, result is stored in A.
// Flags affected: Carry, Sign, Zero, Parity, Aux Carry
void CPU8080::sub(uint8_t data) {
  uint16_t result = registers_.reg_a - data;
  flags_.carry = (result > 0xFF) ? 0 : 1;
  registers_.reg_a = (uint8_t)result;
  update_flags_szp(registers_.reg_a);
}

// SBB: Subtract Register Or Memory From Accumulator
// The specified byte plus the carry is subtracted from A.
// The result is stored in A.
// Flags affected: Carry, Sign, Zero, Parity, Aux Carry
void CPU8080::sbb(uint8_t data) {
  uint16_t result = registers_.reg_a - data;
  flags_.carry = (result > 0xFF) ? 0 : 1;
  registers_.reg_a = (uint8_t)result;
  update_flags_szp(registers_.reg_a);
}

}  // namespace intel_8080
