#include <iostream>

#include "CPU8080.h"

namespace intel_8080 {
// IN Input
//
// Prior to this function being called, a byte is read from memory after the
// opcode to designate the port number. That data is written to the
// accumulator.
void CPU8080::in(uint8_t port_no) {
  if (port_no < 3) {
    registers_.reg_a = input_handler_->ReadInput(port_no);
  } else if (port_no == 3) {
    registers_.reg_a = shift_register_->GetShiftedByte();
  } else {
    std::cerr << "<opcode 0xDB> Invalid input port number: " << port_no
              << std::endl;
  }
}

// OUT Output
//
// The 8080 would write data from the accumulator to the data bus and indicate
// which peripheral device should fetch that data by setting bits in the address
// bus. Here, the port_no is an immediate byte following the OUT opcode, which
// is used to call the respective device's emulation.
void CPU8080::out(uint8_t port_no) {
  if (port_no == 2) {
    shift_register_->SetOffset(registers_.reg_a);
  } else if (port_no == 4) {
    shift_register_->LoadBuffer(registers_.reg_a);
  } else if (port_no == 3) {
    mixer_->SetOut3(registers_.reg_a);
  } else if (port_no == 5) {
    mixer_->SetOut5(registers_.reg_a);
  } else if (port_no == 6) {
    // "Watchdog" external device address checks for reset after a certain
    // amount of cycles have occurred without read or write. This triggers a
    // bunch because this emulator is not cycle-accurate.
    return;
  } else {
    std::cerr << "<opcode 0xD3> invalid output port number: "
              << static_cast<int>(port_no) << std::endl;
  }
}

// INR Increment Register or Memory value
//
// Condition bits affected: Zero, Sign, Parity
void CPU8080::inr(uint8_t* reg) {
  *reg += 1;
  update_flags_szp(*reg);
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
void CPU8080::dcr(uint8_t* reg) {
  *reg -= 1;
  update_flags_szp(*reg);
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
  flags_.carry = (result > 0xFF);
  registers_.reg_a = static_cast<uint8_t>(result);
  update_flags_szp(registers_.reg_a);
}

// ADC: Add Register or Memory To Accumulator w/ Carry
// Adds the specified byte + the carry flag to A and store in A.
// Flags affected: Carry, Sign, Zero, Parity, Aux Carry
void CPU8080::adc(uint8_t data) {
  uint16_t result = registers_.reg_a + data + flags_.carry;
  flags_.carry = (result > 0xFF);
  registers_.reg_a = static_cast<uint8_t>(result);
  update_flags_szp(registers_.reg_a);
}

// SUB: Subtract Register or Memory From Accumulator
// The specified byte is subtracted from A, result is stored in A.
// Flags affected: Carry, Sign, Zero, Parity, Aux Carry
void CPU8080::sub(uint8_t data) {
  uint16_t result = registers_.reg_a - data;
  flags_.carry = (result < 0xFF);
  registers_.reg_a = static_cast<uint8_t>(result);
  update_flags_szp(registers_.reg_a);
}

// SBB: Subtract Register Or Memory From Accumulator
// The specified byte plus the carry is subtracted from A.
// The result is stored in A.
// Flags affected: Carry, Sign, Zero, Parity, Aux Carry
void CPU8080::sbb(uint8_t data) {
  uint16_t result = registers_.reg_a - (data + flags_.carry);
  flags_.carry = (result < 0xFF);
  registers_.reg_a = static_cast<uint8_t>(result);
  update_flags_szp(registers_.reg_a);
}

// ANA: Logical And Register or Memory w/ Accumulator
// The specified byte is logically and'd with A. The carry bit is reset.
// Logical AND is 1 if and only if both bits are set.
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

// CPI: Compare Immediate with AccumulatorHow to separate the high byte and the
// low byte from a 16bit value? Performs a comparison by subtracting a data byte
// from the accumulator without updating the accumulator and checking the
// condition bits. Zero flag is set if the are equal, reset otherwise. Carry bit
// is set if data is larger than accumulator. Flags affected: Carry, Zero, Sign,
// Parity
void CPU8080::cpi(uint8_t data) {
  uint16_t result = registers_.reg_a - data;
  flags_.carry = (result > 0xFF);
  update_flags_szp(static_cast<uint8_t>(result));
}

// CMP: Compare Register or Memory w/ Accumulator
// The specified byte is compared to the contents of A. Internally
// subtracts the byte from A, leaving both unchanged. Condition bits
// are set based on the result, simlar to the SUB instruction.
// Flags affected: Carry, Sign, Zero, Parity, Aux Carry
void CPU8080::cmp(uint8_t data) {
  flags_.carry = data > registers_.reg_a;
  uint16_t result = registers_.reg_a - data;
  update_flags_szp(static_cast<uint8_t>(result));
}

// RRC: Rotate Accumulator Right
// The carry bit is set equal to the low order bit of the accumulator.
// The contents of the A are rotated one bit to the right, with the low
// order bit being transferred to the high order bit.
// Flags affected: CarryHow to separate the high byte and the low byte from a
// 16bit value?
void CPU8080::rrc() {
  uint8_t lsb = registers_.reg_a & 0x01;
  flags_.carry = lsb;
  registers_.reg_a = (registers_.reg_a >> 1) | (lsb << 7);
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
/*
 * PUSH: Push Data onto stack
 * The contents of the specified register pair is saved in two bytes
 * of memory indicated by stack_pointer_. The first register is saved
 * at one less than the stack pointer, and the second register is saved
 * at two less than the stack pointer. The stack pointer is decremented
 * by 2.
 * Flags affected: N/A
 */
void CPU8080::push(uint8_t reg_1, uint8_t reg_2) {
  stack_pointer_ -= 1;
  mem_access_->write(stack_pointer_, reg_1);
  stack_pointer_ -= 1;
  mem_access_->write(stack_pointer_, reg_2);
}

/*
 * POP: Pop Data Off Stack
 * The contents of the specified register pair are restored
 * by the stack pointer. The byte of data at the stack pointer is
 * loaded into the second register. The byte of data at the stack
 * pointer + 1 is loaded intp the first register.
 * Flags affected: N/A
 */
void CPU8080::pop(uint8_t* reg_1, uint8_t* reg_2) {
  *reg_2 = mem_access_->read(stack_pointer_);
  stack_pointer_ += 1;
  *reg_1 = mem_access_->read(stack_pointer_);
  stack_pointer_ += 1;
}
/*
 * DAD: Double Add.
 * The 16 bit number in the specified register pair is added to the
 * 16 bit number held in the HL register pair. The result replaces
 * the contents of HL.
 * Flags affected: N/A
 */
void CPU8080::dad(const uint8_t* reg_1, const uint8_t* reg_2) {
  uint16_t reg_pair = (*reg_1 << 8) | *reg_2;
  uint32_t result = reg_pair + registers_.hl();
  flags_.carry = (result > 0xFFFF);
  registers_.reg_h = static_cast<uint8_t>((result >> 8) & 0xFF);
  registers_.reg_l = static_cast<uint8_t>(result & 0xFF);
}

/*<<<<<<< dev_main_loop
 * INX: Increment register pair
 * The 16 bit number held in the specified register pair is
 * incremented by 1.
 * Flags affected: N/A
 */
void CPU8080::inx(uint8_t* reg_1, uint8_t* reg_2) {
  auto reg_pair = static_cast<uint16_t>((*reg_1 << 8) | *reg_2);
  reg_pair += 1;
  *reg_1 = static_cast<uint8_t>(reg_pair >> 8);
  *reg_2 = static_cast<uint8_t>(reg_pair & 0xFF);
}

/*
 * DCX: Decrement register pair
 * The 16 bit number in the specified register pair is decremented by 1.
 * Flags affected: N/A
 */
void CPU8080::dcx(uint8_t* reg_1, uint8_t* reg_2) {
  auto reg_pair = static_cast<uint16_t>((*reg_1 << 8) | *reg_2);
  reg_pair -= 1;
  *reg_1 = static_cast<uint8_t>(reg_pair >> 8);
  *reg_2 = static_cast<uint8_t>(reg_pair & 0xFF);
}

void CPU8080::xchg() {
  std::swap(registers_.reg_h, registers_.reg_d);
  std::swap(registers_.reg_l, registers_.reg_e);
}

void CPU8080::xthl() {
  uint8_t byte_2 = mem_access_->read(stack_pointer_);
  uint8_t byte_1 = mem_access_->read(stack_pointer_ + 1);
  mem_access_->write(stack_pointer_, registers_.reg_l);
  mem_access_->write(stack_pointer_ + 1, registers_.reg_h);
  registers_.reg_h = byte_1;
  registers_.reg_l = byte_2;
}

void CPU8080::sphl() {
  stack_pointer_ =
      static_cast<uint16_t>(registers_.reg_h << 8) | registers_.reg_l;
}

void CPU8080::lxi_sp(uint8_t byte_2, uint8_t byte_3) {
  stack_pointer_ = static_cast<uint16_t>(byte_3 << 8) | byte_2;
}

void CPU8080::lxi(uint8_t* reg_1, uint8_t* reg_2, uint8_t byte_2,
                  uint8_t byte_3) {
  *reg_1 = byte_3;
  *reg_2 = byte_2;
}

void CPU8080::sta(uint8_t byte_2, uint8_t byte_3) {
  auto mem_location = static_cast<uint16_t>((byte_3 << 8) | byte_2);
  mem_access_->write(mem_location, registers_.reg_a);
}

void CPU8080::lda(uint8_t byte_2, uint8_t byte_3) {
  auto mem_location = static_cast<uint16_t>((byte_3 << 8) | byte_2);
  registers_.reg_a = mem_access_->read(mem_location);
}

void CPU8080::shld(uint8_t byte_2, uint8_t byte_3) {
  auto mem_location = static_cast<uint16_t>((byte_3 << 8) | byte_2);
  mem_access_->write(mem_location, registers_.reg_l);
  mem_access_->write(mem_location + 1, registers_.reg_h);
}

void CPU8080::lhld(uint8_t byte_2, uint8_t byte_3) {
  auto mem_location = static_cast<uint16_t>((byte_3 << 8) | byte_2);
  registers_.reg_l = mem_access_->read(mem_location);
  registers_.reg_h = mem_access_->read(mem_location + 1);
}

void CPU8080::pchl() {
  auto mem_location =
      static_cast<uint16_t>((registers_.reg_h << 8) | registers_.reg_l);
  program_counter_ = mem_location;
}

// JUMP Instructions
// JMP always transfers program control, while the others do so under a
// condition. For example, JM, "Jump if Minus", jumps if the sign bit is set.
// Sice the CALL instructions work the same way, this function reuses the
// CallType enum class.
void CPU8080::jmp(JumpCondition jump_condition, uint8_t byte_2,
                  uint8_t byte_3) {
  if (!check_jump_condition(jump_condition)) {
    return;
  }

  auto mem_location = static_cast<uint16_t>((byte_3 << 8) | byte_2);
  program_counter_ = mem_location;
}

// CALL Subroutine Instructions
// CALL always transfers program control, while the others do so under a
// condition. For example, CNZ will Call if Not Zero, i.e. if the zero flag is
// clear. Pushes the program counter address onto the stack and then points the
// program counter to the provided memory address.
void CPU8080::call(JumpCondition jump_condition, uint8_t byte_2,
                   uint8_t byte_3) {
  if (!check_jump_condition(jump_condition)) {
    return;
  }

  auto mem_location = static_cast<uint16_t>((byte_3 << 8) | byte_2);
  uint8_t high_byte = program_counter_ >> 8;
  uint8_t low_byte = program_counter_ & 0xFF;

  push(low_byte, high_byte);
  program_counter_ = mem_location;
}

// RETURN Instructions
// These instructions pop the last address off the stack and assign it to the
// program counter in order to return from a subroutine. RET always does it,
// while other return instructions do so under a condition.
void CPU8080::ret(JumpCondition jump_condition) {
  if (!check_jump_condition(jump_condition)) {
    return;
  }

  uint8_t high_byte{0};
  uint8_t low_byte{0};
  pop(&low_byte, &high_byte);
  program_counter_ = static_cast<uint16_t>((high_byte << 8) | low_byte);
}

// Restart instructions
// A particular RST instruction is called by an interrupting device to transfer
// control to a subroutine that handles the situation. A RET (return)
// instruction causes the previously running program to resume control.
//
// For example, according to computer archaeology, the two display update
// interrupts are RST 8 and RST 10. The 3-bit `exp` value is bits 3–5, so
// for the first interrupt, it would need to call rst(1), and for the second,
// rst(2).
//
// WARN: The CPU's state must be preserved before and restored following an
// interrupt subroutine, and a subroutine must conclude with a RET (return)
// instruction. Since it is the programmer's responsibility to do this, I
// suspect it is accounted for by the hardware interrupt subroutines in the
// Space Invaders ROM.
void CPU8080::rst(uint8_t exp) {
#ifdef DEBUG
  if (exp == 1) {
    print_instruction(0xCF);
  } else if (exp == 2) {
    print_instruction(0xD7);
  }
#endif
  call(JumpCondition::kTrue, static_cast<uint8_t>(exp << 3), 0x00);

  // CPU enters a STOPPED state to await an interrupt.
  // Main loop checks for halted_ while the CPU is stepping.
  halted_ = false;
}

void CPU8080::ei() { INTE_ = true; }

void CPU8080::di() { INTE_ = false; }

// HLT Halt instruction
// Increments the program counter by one, then tells the CPU to await an
// interrupt.
//
// Could be emulated by checking the CPU8080::halted_ member variable, and
// reseting when an interrupt occurs. However, that may not be necessary based
// on interrupts are implemented.
void CPU8080::hlt() { halted_ = true; }

// Returns true if the specified condition is true, otherwise false.
// Called by JMP, RET, and CALL instructions that depend on a condition check.
bool CPU8080::check_jump_condition(JumpCondition jump_condition) const {
  switch (jump_condition) {
    case JumpCondition::kTrue:
      return true;
    case JumpCondition::kNotZero:
      return !flags_.zero;
    case JumpCondition::kZero:
      return flags_.zero;
    case JumpCondition::kNotCarry:
      return !flags_.carry;
    case JumpCondition::kCarry:
      return flags_.carry;
    case JumpCondition::kParityOdd:
      return !flags_.parity;
    case JumpCondition::kParityEven:
      return flags_.parity;
    case JumpCondition::kPositive:
      return !flags_.sign;
    case JumpCondition::kMinus:
      return flags_.sign;
  }
  // The switch case is exhaustive, so the following is to silence the warning:
  std::cerr << "Failed to match a jump condition!\n";
  return false;
}

}  // namespace intel_8080
