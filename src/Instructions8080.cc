#include <iostream>

#include "CPU8080.h"

namespace intel_8080 {
// IN Input
//
// Prior to this function being called, a byte is read from memory after the
// opcode to designate the port number. That data is written to the
// accumulator.
void CPU8080::in(uint8_t port_no) {
  if (port_no != 1 && port_no != 2) {
    std::cerr << "<opcode 0xDB> Invalid input port number: " << port_no
              << std::endl;
  }

  registers_.reg_a = read_input_port(port_no);
}

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
}  // namespace intel_8080
