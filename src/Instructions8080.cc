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
// Contents of A are stored in the memory location from either BC or DE.
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
// Flags affected: N/Ahow to isolate lsbv in cpp
void CPU8080::cma() { registers_.reg_a = ~registers_.reg_a; }

// ADD: Add Register or Memory to Accumulator
// Adds the specified byte to A and the result is stored in A using two's
// complement arithmetic Flags affected: Carry, Sign, Zero, Parity, Aux Carry
void CPU8080::add(uint8_t data) {
  uint16_t result = registers_.reg_a + data;
  flags_.carry = (result > 0xFF) ? 1 : 0;
  registers_.reg_a = static_cast<uint8_t>(result);
  update_flags_szp(registers_.reg_a);
}

// ADC: Add Register or Memory To Accumulator w/ Carry
// Adds the specified byte + the carry flag to A and store in A.
// Flags affected: Carry, Sign, Zero, Parity, Aux Carry
void CPU8080::adc(uint8_t data) {
  data += flags_.carry;
  uint16_t result = registers_.reg_a + data + flags_.carry;
  flags_.carry = (result > 0xFF) ? 1 : 0;
  registers_.reg_a = static_cast<uint8_t>(result);
  update_flags_szp(registers_.reg_a);
}

// SUB: Subtract Register or Memory From Accumulator
// The specified byte is subtracted from A, result is stored in A.
// Flags affected: Carry, Sign, Zero, Parity, Aux Carry
void CPU8080::sub(uint8_t data) {
  uint16_t result = registers_.reg_a - data;
  flags_.carry = (result > 0xFF) ? 0 : 1;
  registers_.reg_a = static_cast<uint8_t>(result);
  update_flags_szp(registers_.reg_a);
}

// SBB: Subtract Register Or Memory From Accumulator
// The specified byte plus the carry is subtracted from A.
// The result is stored in A.
// Flags affected: Carry, Sign, Zero, Parity, Aux Carry
void CPU8080::sbb(uint8_t data) {
  uint16_t result = registers_.reg_a - (data + flags_.carry);
  flags_.carry = (result > 0xFF) ? 0 : 1;
  registers_.reg_a = static_cast<uint8_t>(result);
  update_flags_szp(registers_.reg_a);
}

// ANA: Logical And Register or Memory w/ Accumulator
// The specified byte is logically and'd with A. The carry bit is reset.
// Logical AND is 1 if and only if both bits are different
// Flags affected: Carry, Zero, Sign, Parity
void CPU8080::ana(uint8_t data) {
  registers_.reg_a = registers_.reg_a & data;
  flags_.carry = 0;
  update_flags_szp(registers_.reg_a);
}

// XRA: Logical Exclusive Or Register Or Memory w/ Accumulator
// The specified byte is ORd with A. The carry bit is reset.
// Logical XOR is 1 if and only if both bits are different.
// Flags affected: Carry, Zero, Sign, Parity
void CPU8080::xra(uint8_t data) {
  registers_.reg_a = registers_.reg_a ^ data;
  flags_.carry = 0;
  update_flags_szp(registers_.reg_a);
}

// ORA: Logical Or Register or Memory w/ Accumulator
// The specified byte is logicallyu ORd w/ A. The carry bit is reset.
// Logical OR is zero if and only if both bits are zero.
// Flags affected: Carry, Zero, Sign, Parity
void CPU8080::ora(uint8_t data) {
  registers_.reg_a = registers_.reg_a | data;
  flags_.carry = 0;
  update_flags_szp(registers_.reg_a);
}

// CMP: Compare Register or Memory w/ Accumulator
// THe specified byte is compared to the contents of A. Internally
// subtracts the byte from A, leaving both unchanged. Condition bits
// are set based on the result, simlar to the SUB instruction.
// Flags affected: Carry, Sign, Zero, Parity, Aux Carry
void CPU8080::cmp(uint8_t data) {
  uint16_t result = registers_.reg_a - data;
  flags_.carry = (result > 0xFF) ? 0 : 1;
  update_flags_szp(result);
}

// RRC: Rotate Accumulator Right
// The carry bit is set equal to the low order bit of the accumulator.
// The contents of the A are rotated one bit to the right, with the low
// order bit being transferred to the high order bit.
// Flags affected: Carry
void CPU8080::rrc() {
  uint8_t lsb = registers_.reg_a & 0x01;
  flags_.carry = lsb;
  registers_.reg_a = (registers_.reg_a >> 7) | (lsb << 7);
}

// RAL: Rotate Accumulator Left Through Carry
// The contents of A are rotate one bit to the left. THe high order bit
// replaces the Carry bit, the Carry bit replaces the high order bit.
// Flags affected: Carry
void CPU8080::ral() {
  uint8_t msb = registers_.reg_a >> 7;
  uint8_t temp = registers_.reg_a << 1;
  registers_.reg_a = temp | flags_.carry;
  flags_.carry = msb;
}

// RAR: Rotate Accumulator Right Through Carry
// THe contents of A are rotated one bit to the right through the
// carry bit.
// Flags affected: Carry
void CPU8080::rar() {
  uint8_t lsb = registers_.reg_a & 0x01;
  uint8_t temp = registers_.reg_a >> 1;
  registers_.reg_a = temp | flags_.carry << 7;
  flags_.carry = lsb;
}

}  // namespace intel_8080
