#include "CPU8080.h"

#include <memory>

#include "Memory8080.h"

// For printing hex bytes to stdout
#include <iomanip>
#include <iostream>

// For printing hex bytes to stdout
void print_hex_byte(uint8_t value) {
  std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
            << static_cast<int>(value) << std::dec;
}

namespace intel_8080 {

CPU8080::CPU8080(std::shared_ptr<intel_8080::Memory8080> new_mem)
    : mem_access_(std::move(new_mem)) {
  stack_pointer_ = 0x0000;
  program_counter_ = 0x0000;
  flags_ = Flags();
  registers_ = Registers();
  INTE_ = false;
};

std::uint8_t CPU8080::Flags::to_byte() {
  std::uint8_t return_byte = 0x02;

  // Shift flags to proper location
  return_byte = return_byte | (sign << 7);
  return_byte = return_byte | (zero << 6);
  return_byte = return_byte | (aux_carry << 4);
  return_byte = return_byte | (parity << 2);
  return_byte = return_byte | carry;
  return return_byte;
};

void CPU8080::Flags::from_byte(uint8_t data) {
  carry = data & 0x1;
  parity = data & (1 << 2);
  aux_carry = data & (1 << 4);
  zero = data & (1 << 6);
  sign = data & (1 << 7);
}

void CPU8080::step() {
  uint8_t instruction = fetch_byte();
  execute(instruction);
};

uint8_t CPU8080::fetch_byte() {
  uint8_t byte = mem_access_->read(program_counter_);
  program_counter_++;
  return byte;
};

uint8_t CPU8080::fetch_byte(std::uint16_t mem_location) {
  uint8_t byte = mem_access_->read(mem_location);
  return byte;
};

uint16_t CPU8080::fetch_word() {
  uint8_t low_byte = fetch_byte();
  uint8_t high_byte = fetch_byte();
  auto word = static_cast<uint16_t>((high_byte << 8) | low_byte);
  return word;
}

uint16_t CPU8080::fetch_word(std::uint16_t mem_location) {
  uint8_t low_byte = mem_access_->read(mem_location);
  uint8_t high_byte = mem_access_->read(mem_location + 1);
  auto word = static_cast<uint16_t>((high_byte << 8) | low_byte);
  return word;
};

// Copies the state of the CPU and returns in a State struct.
CPU8080::State CPU8080::get_state() {
  return State{registers_, flags_, stack_pointer_, program_counter_};
}

void CPU8080::reset() { program_counter_ = 0x0000; };

// Sets parity flag if byte has even parity, otherwise resets it.
void CPU8080::update_parity(uint8_t byte) {
  if (__builtin_parity(byte)) {
    flags_.parity = 0;
  } else {
    flags_.parity = 1;
  }
}

// Sets parity flag if word has even parity, otherwise resets it.
void CPU8080::update_parity(uint16_t word) {
  if (__builtin_parity(word)) {
    flags_.parity = 0;
  } else {
    flags_.parity = 1;
  }
}

// Updates condition codes for sign, zero, auxiliary carry, and parity.
// Carry is handled separately for two reasons:
// 1. Not all instructions that affect these flags also affect the carry bit
// and vice versa.
// 2. How to update the carry bit depends on the data type of the instruction.
// NOTE: Does not update auxiliary carry bit because we are not implementing the
// DAA instruction. This flag's sole purpose is to enable that instruction.
void CPU8080::update_flags_szp(uint8_t byte) {
  flags_.zero = byte == 0;
  flags_.sign = ((byte & 0b1000'0000) == 0b1000'0000);
  update_parity(byte);
}

void CPU8080::execute(uint8_t opcode) {
  switch (opcode) {
    case 0x00:
      std::cout << "NOP" << std::endl;
      break;
    case 0x01:
      lxi(&registers_.reg_b, &registers_.reg_c, fetch_byte(), fetch_byte());
      break;
    case 0x02: {
      auto reg =
          static_cast<uint16_t>((registers_.reg_b << 8) | registers_.reg_c);
      stax(reg);
      std::cout << "STAX B" << std::endl;
      break;
    }
    case 0x03:
      inx(&registers_.reg_b, &registers_.reg_c);
      break;
    case 0x04:  // INR B: B += 1 flags
      inr(&registers_.reg_b);
      break;
    case 0x05:
      dcr(&registers_.reg_b);
      break;
    case 0x06:
      mov(&registers_.reg_b, fetch_byte());
      break;
    case 0x07:  // RLC
      rlc();
      break;
    case 0x08:
      std::cout << "NOP*" << std::endl;
      break;
    case 0x09:
      dad(&registers_.reg_b, &registers_.reg_c);
      break;
    case 0x0A:
      std::cout << "LDAX B" << std::endl;
      break;
    case 0x0B:
      std::cout << "DCX B" << std::endl;
      break;
    case 0x0C:
      std::cout << "INR C" << std::endl;
      break;
    case 0x0D:
      std::cout << "DCR C" << std::endl;
      break;
    case 0x0E:
      std::cout << "MVI C,";
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0x0F:
      std::cout << "RRC" << std::endl;
      break;
    case 0x10:
      std::cout << "NOP*" << std::endl;
      break;
    case 0x11:
      std::cout << "LXI D,";
      print_hex_byte(fetch_byte());
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0x12:
      std::cout << "STAX D" << std::endl;
      break;
    case 0x13:
      std::cout << "INX D" << std::endl;
      break;
    case 0x14:
      std::cout << "INR D" << std::endl;
      break;
    case 0x15:
      std::cout << "DCR D" << std::endl;
      break;
    case 0x16:
      std::cout << "MVI D,";
      print_hex_byte(fetch_byte());
      break;
    case 0x17:
      std::cout << "RAL" << std::endl;
      break;
    case 0x18:
      std::cout << "*NOP" << std::endl;
      break;
    case 0x19:
      std::cout << "DAD D" << std::endl;
      break;
    case 0x1A:
      std::cout << "LDAX D" << std::endl;
      break;
    case 0x1B:
      std::cout << "DCX D" << std::endl;
      break;
    case 0x1C:
      std::cout << "INR E" << std::endl;
      break;
    case 0x1D:
      std::cout << "DCR E" << std::endl;
      break;
    case 0x1E:
      std::cout << "MVI E,";
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0x1F:
      std::cout << "RAR" << std::endl;
      break;
    case 0x20:
      std::cout << "*NOP" << std::endl;
      break;
    case 0x21:
      std::cout << "LXI H,";
      print_hex_byte(fetch_byte());
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0x22:
      std::cout << "SHLD ";
      print_hex_byte(fetch_byte());
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0x23:
      std::cout << "INX H" << std::endl;
      break;
    case 0x24:
      std::cout << "INR H" << std::endl;
      break;
    case 0x25:
      std::cout << "DCR H" << std::endl;
      break;
    case 0x26:
      std::cout << "MVI H,";
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0x27:
      std::cout << "DAA" << std::endl;
      break;
    case 0x28:
      std::cout << "*NOP" << std::endl;
      break;
    case 0x29:
      std::cout << "DAD H" << std::endl;
      break;
    case 0x2A:
      std::cout << "LHLD ";
      print_hex_byte(fetch_byte());
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0x2B:
      std::cout << "DCX H" << std::endl;
      break;
    case 0x2C:
      std::cout << "INR L" << std::endl;
      break;
    case 0x2D:
      std::cout << "DCR L" << std::endl;
      break;
    case 0x2E:
      std::cout << "MVI L,";
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0x2F:
      std::cout << "CMA" << std::endl;
      break;
    case 0x30:
      std::cout << "*NOP" << std::endl;
      break;
    case 0x31:
      std::cout << "LXI SP,";
      print_hex_byte(fetch_byte());
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0x32:
      std::cout << "STA ";
      print_hex_byte(fetch_byte());
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0x33:
      std::cout << "INX SP" << std::endl;
      break;
    case 0x34:
      std::cout << "INR M" << std::endl;
      break;
    case 0x35:
      std::cout << "DCR M" << std::endl;
      break;
    case 0x36:
      std::cout << "MVI M,";
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0x37:
      std::cout << "STC" << std::endl;
      break;
    case 0x38:
      std::cout << "NOP*" << std::endl;
      break;
    case 0x39:
      std::cout << "DAD SP" << std::endl;
      break;
    case 0x3A:
      std::cout << "LDA ";
      print_hex_byte(fetch_byte());
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0x3B:
      std::cout << "DCX SP" << std::endl;
      break;
    case 0x3C:
      std::cout << "INR A" << std::endl;
      break;
    case 0x3D:
      std::cout << "DCR A" << std::endl;
      break;
    case 0x3E:  // MVI A, D8
      mov(&registers_.reg_a, fetch_byte());
      break;
    case 0x3F:
      std::cout << "CMC" << std::endl;
      break;
    case 0x40:
      std::cout << "MOV B,B" << std::endl;
      break;
    case 0x41:
      std::cout << "MOV B,C" << std::endl;
      break;
    case 0x42:
      std::cout << "MOV B,D" << std::endl;
      break;
    case 0x43:
      std::cout << "MOV B,E" << std::endl;
      break;
    case 0x44:
      std::cout << "MOV, B,H" << std::endl;
      break;
    case 0x45:
      std::cout << "MOV B,L" << std::endl;
      break;
    case 0x46:
      std::cout << "MOV B,M" << std::endl;
      break;
    case 0x47:
      std::cout << "MOV B,A" << std::endl;
      break;
    case 0x48:
      std::cout << "MOV C,B" << std::endl;
      break;
    case 0x49:
      std::cout << "MOV C,C" << std::endl;
      break;
    case 0x4A:
      std::cout << "MOV C,D" << std::endl;
      break;
    case 0x4B:
      std::cout << "MOV C,E" << std::endl;
      break;
    case 0x4C:
      std::cout << "MOV C,H" << std::endl;
      break;
    case 0x4D:
      std::cout << "MOV C,L" << std::endl;
      break;
    case 0x4E:
      std::cout << "MOV C,M" << std::endl;
      break;
    case 0x4F:
      std::cout << "MOV C,A" << std::endl;
      break;
    case 0x50:
      std::cout << "MOV D,B" << std::endl;
      break;
    case 0x51:
      std::cout << "MOV D,C" << std::endl;
      break;
    case 0x52:
      std::cout << "MOV D,D" << std::endl;
      break;
    case 0x53:
      std::cout << "MOV D,E" << std::endl;
      break;
    case 0x54:
      std::cout << "MOV D,H" << std::endl;
      break;
    case 0x55:
      std::cout << "MOV D,L" << std::endl;
      break;
    case 0x56:
      std::cout << "MOV D,M" << std::endl;
      break;
    case 0x57:
      std::cout << "MOV D,A" << std::endl;
      break;
    case 0x58:
      std::cout << "MOV E,B" << std::endl;
      break;
    case 0x59:
      std::cout << "MOV E,C" << std::endl;
      break;
    case 0x5A:
      std::cout << "MOV E,D" << std::endl;
      break;
    case 0x5B:
      std::cout << "MOV E,E" << std::endl;
      break;
    case 0x5C:
      std::cout << "MOV E,H" << std::endl;
      break;
    case 0x5D:
      std::cout << "MOV E,L" << std::endl;
      break;
    case 0x5E:
      std::cout << "MOV E,M" << std::endl;
      break;
    case 0x5F:
      std::cout << "MOV E,A" << std::endl;
      break;
    case 0x60:
      std::cout << "MOV H,B" << std::endl;
      break;
    case 0x61:
      std::cout << "MOV H,C" << std::endl;
      break;
    case 0x62:
      std::cout << "MOV H,D" << std::endl;
      break;
    case 0x63:
      std::cout << "MOV H,E" << std::endl;
      break;
    case 0x64:
      std::cout << "MOV H,H" << std::endl;
      break;
    case 0x65:
      std::cout << "MOV H,L" << std::endl;
      break;
    case 0x66:
      std::cout << "MOV H,M" << std::endl;
      break;
    case 0x67:
      std::cout << "MOV H,A" << std::endl;
      break;
    case 0x68:
      std::cout << "MOV E,B" << std::endl;
      break;
    case 0x69:
      std::cout << "MOV E,C" << std::endl;
      break;
    case 0x6A:
      std::cout << "MOV E,D" << std::endl;
      break;
    case 0x6B:
      std::cout << "MOV E,E" << std::endl;
      break;
    case 0x6C:
      std::cout << "MOV E,H" << std::endl;
      break;
    case 0x6D:
      std::cout << "MOV E,L" << std::endl;
      break;
    case 0x6E:
      std::cout << "MOV E,M" << std::endl;
      break;
    case 0x6F:
      std::cout << "MOV E,A" << std::endl;
      break;
    case 0x70:
      std::cout << "MOV M,B" << std::endl;
      break;
    case 0x71:
      std::cout << "MOV M,C" << std::endl;
      break;
    case 0x72:
      std::cout << "MOV M,D" << std::endl;
      break;
    case 0x73:
      std::cout << "MOV M,E" << std::endl;
      break;
    case 0x74:
      std::cout << "MOV M,H" << std::endl;
      break;
    case 0x75:
      std::cout << "MOV M,L" << std::endl;
      break;
    case 0x76:
      std::cout << "HLT" << std::endl;
      break;
    case 0x77:
      std::cout << "MOV M,A" << std::endl;
      break;
    case 0x78:
      std::cout << "MOV A,B" << std::endl;
      break;
    case 0x79:
      std::cout << "MOV A,C" << std::endl;
      break;
    case 0x7A:
      std::cout << "MOV A,D" << std::endl;
      break;
    case 0x7B:
      std::cout << "MOV A,E" << std::endl;
      break;
    case 0x7C:
      std::cout << "MOV A,H" << std::endl;
      break;
    case 0x7D:
      std::cout << "MOV A,L" << std::endl;
      break;
    case 0x7E:
      std::cout << "MOV A,M" << std::endl;
      break;
    case 0x7F:
      std::cout << "MOV A,A" << std::endl;
      break;
    case 0x80:
      std::cout << "ADD B" << std::endl;
      break;
    case 0x81:
      std::cout << "ADD C" << std::endl;
      break;
    case 0x82:
      std::cout << "ADD D" << std::endl;
      break;
    case 0x83:
      std::cout << "ADD E" << std::endl;
      break;
    case 0x84:
      std::cout << "ADD H" << std::endl;
      break;
    case 0x85:
      std::cout << "ADD L" << std::endl;
      break;
    case 0x86:
      std::cout << "ADD M" << std::endl;
      break;
    case 0x87:
      std::cout << "ADD A" << std::endl;
      break;
    case 0x88:
      std::cout << "ADC B" << std::endl;
      break;
    case 0x89:
      std::cout << "ADC C" << std::endl;
      break;
    case 0x8A:
      std::cout << "ADC D" << std::endl;
      break;
    case 0x8B:
      std::cout << "ADC E" << std::endl;
      break;
    case 0x8C:
      std::cout << "ADC H" << std::endl;
      break;
    case 0x8D:
      std::cout << "ADC L" << std::endl;
      break;
    case 0x8E:
      std::cout << "ADC M" << std::endl;
      break;
    case 0x8F:
      std::cout << "ADC A" << std::endl;
      break;
    case 0x90:
      std::cout << "SUB B" << std::endl;
      break;
    case 0x91:
      std::cout << "SUB C" << std::endl;
      break;
    case 0x92:
      std::cout << "SUB D" << std::endl;
      break;
    case 0x93:
      std::cout << "SUB E" << std::endl;
      break;
    case 0x94:
      std::cout << "SUB H" << std::endl;
      break;
    case 0x95:
      std::cout << "SUB L" << std::endl;
      break;
    case 0x96:
      std::cout << "SUB M" << std::endl;
      break;
    case 0x97:
      std::cout << "SUB A" << std::endl;
      break;
    case 0x98:
      std::cout << "SBB B" << std::endl;
      break;
    case 0x99:
      std::cout << "SBB C" << std::endl;
      break;
    case 0x9A:
      std::cout << "SBB D" << std::endl;
      break;
    case 0x9B:
      std::cout << "SBB E" << std::endl;
      break;
    case 0x9C:
      std::cout << "SBB H" << std::endl;
      break;
    case 0x9D:
      std::cout << "SBB L" << std::endl;
      break;
    case 0x9E:
      std::cout << "SBB M" << std::endl;
      break;
    case 0x9F:
      std::cout << "SBB A" << std::endl;
      break;
    case 0xA0:
      std::cout << "ANA B" << std::endl;
      break;
    case 0xA1:
      std::cout << "ANA C" << std::endl;
      break;
    case 0xA2:
      std::cout << "ANA D" << std::endl;
      break;
    case 0xA3:
      std::cout << "ANA E" << std::endl;
      break;
    case 0xA4:
      std::cout << "ANA H" << std::endl;
      break;
    case 0xA5:
      std::cout << "ANA L" << std::endl;
      break;
    case 0xA6:
      std::cout << "ANA M" << std::endl;
      break;
    case 0xA7:
      std::cout << "ANA A" << std::endl;
      break;
    case 0xA8:
      std::cout << "XRA B" << std::endl;
      break;
    case 0xA9:
      std::cout << "XRA C" << std::endl;
      break;
    case 0xAA:
      std::cout << "XRA D" << std::endl;
      break;
    case 0xAB:
      std::cout << "XRA E" << std::endl;
      break;
    case 0xAC:
      std::cout << "XRA H" << std::endl;
      break;
    case 0xAD:
      std::cout << "XRA L" << std::endl;
      break;
    case 0xAE:
      std::cout << "XRA M" << std::endl;
      break;
    case 0xAF:
      std::cout << "XRA A" << std::endl;
      break;
    case 0xB0:
      std::cout << "ORA B" << std::endl;
      break;
    case 0xB1:
      std::cout << "ORA C" << std::endl;
      break;
    case 0xB2:
      std::cout << "ORA D" << std::endl;
      break;
    case 0xB3:
      std::cout << "ORA E" << std::endl;
      break;
    case 0xB4:
      std::cout << "ORA H" << std::endl;
      break;
    case 0xB5:
      std::cout << "ORA L" << std::endl;
      break;
    case 0xB6:
      std::cout << "ORA M" << std::endl;
      break;
    case 0xB7:
      std::cout << "ORA A" << std::endl;
      break;
    case 0xB8:
      std::cout << "CMP B" << std::endl;
      break;
    case 0xB9:
      std::cout << "CMP C" << std::endl;
      break;
    case 0xBA:
      std::cout << "CMP D" << std::endl;
      break;
    case 0xBB:
      std::cout << "CMP E" << std::endl;
      break;
    case 0xBC:
      std::cout << "CMP H" << std::endl;
      break;
    case 0xBD:
      std::cout << "CMP L" << std::endl;
      break;
    case 0xBE:
      std::cout << "CMP M" << std::endl;
      break;
    case 0xBF:
      std::cout << "CMP A" << std::endl;
      break;
    case 0xC0:
      std::cout << "RNZ\n";
      ret(JumpCondition::kNotZero);
      break;
    case 0xC1:
      std::cout << "POP B\n";
      pop(&registers_.reg_b, &registers_.reg_c);
      break;
    case 0xC2:
      std::cout << "JNZ \n";
      jmp(JumpCondition::kNotZero, fetch_byte(), fetch_byte());
      break;
    case 0xC3:
      std::cout << "JMP \n";
      jmp(JumpCondition::kTrue, fetch_byte(), fetch_byte());
      break;
    case 0xC4:
      std::cout << "CNZ \n";
      call(JumpCondition::kNotZero, fetch_byte(), fetch_byte());
      break;
    case 0xC5:
      std::cout << "PUSH B\n";
      push(registers_.reg_b, registers_.reg_c);
      break;
    case 0xC6:
      std::cout << "ADI ";
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0xC7:
      std::cout << "RST 0\n";
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xC8:
      std::cout << "RZ\n";
      ret(JumpCondition::kZero);
      break;
    case 0xC9:
      std::cout << "RET\n";
      ret(JumpCondition::kTrue);
      break;
    case 0xCA:
      std::cout << "JZ \n";
      jmp(JumpCondition::kZero, fetch_byte(), fetch_byte());
      break;
    case 0xCB:
      std::cout << "*JMP \n";
      jmp(JumpCondition::kTrue, fetch_byte(), fetch_byte());
      break;
    case 0xCC:
      std::cout << "CZ \n";
      call(JumpCondition::kZero, fetch_byte(), fetch_byte());
      break;
    case 0xCD:
      std::cout << "CALL \n";
      call(JumpCondition::kTrue, fetch_byte(), fetch_byte());
      break;
    case 0xCE:
      std::cout << "ACI ";
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0xCF:
      std::cout << "RST 1\n";
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xD0:
      std::cout << "RNC\n";
      ret(JumpCondition::kNotCarry);
      break;
    case 0xD1:
      std::cout << "POP D\n";
      pop(&registers_.reg_d, &registers_.reg_e);
      break;
    case 0xD2:
      std::cout << "JNZ \n";
      jmp(JumpCondition::kNotZero, fetch_byte(), fetch_byte());
      break;
    case 0xD3:
      std::cout << "OUT ";
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0xD4:
      std::cout << "CNC \n";
      call(JumpCondition::kNotCarry, fetch_byte(), fetch_byte());
      break;
    case 0xD5:
      std::cout << "PUSH D\n";
      push(registers_.reg_d, registers_.reg_e);
      break;
    case 0xD6:
      std::cout << "SUI ";
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0xD7:
      std::cout << "RST 2\n";
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xD8:
      std::cout << "RC\n";
      ret(JumpCondition::kCarry);
      break;
    case 0xD9:
      std::cout << "*RET\n";
      ret(JumpCondition::kTrue);
      break;
    case 0xDA:
      std::cout << "JC \n";
      jmp(JumpCondition::kCarry, fetch_byte(), fetch_byte());
      break;
    case 0xDB:
      std::cout << "IN ";
      print_hex_byte(fetch_byte());
      std::cout << std::endl;
      break;
    case 0xDC:
      std::cout << "CC \n";
      call(JumpCondition::kCarry, fetch_byte(), fetch_byte());
      break;
    case 0xDD:
      std::cout << "*CALL \n";
      call(JumpCondition::kTrue, fetch_byte(), fetch_byte());
      break;
    case 0xDE:
      std::cout << "SBI \n";
      sbb(fetch_byte());
      break;
    case 0xDF:
      std::cout << "RST 3\n";
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xE0:
      std::cout << "RPO" << std::endl;
      ret(JumpCondition::kParityOdd);
      break;
    case 0xE1:
      std::cout << "POP H\n";
      pop(&registers_.reg_h, &registers_.reg_l);
      break;
    case 0xE2:
      std::cout << "JPO \n";
      jmp(JumpCondition::kParityOdd, fetch_byte(), fetch_byte());
      break;
    case 0xE3:
      std::cout << "XTHL\n";
      xthl();
      break;
    case 0xE4:
      std::cout << "CPO \n";
      call(JumpCondition::kParityOdd, fetch_byte(), fetch_byte());
      break;
    case 0xE5:
      std::cout << "PUSH H\n";
      push(registers_.reg_h, registers_.reg_l);
      break;
    case 0xE6:
      std::cout << "ANI \n";
      ana(fetch_byte());
      break;
    case 0xE7:
      std::cout << "RST 4\n";
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xE8:
      std::cout << "RPE\n";
      ret(JumpCondition::kParityEven);
      break;
    case 0xE9:
      std::cout << "PCHL\n";
      pchl();
      break;
    case 0xEA:
      std::cout << "JPE \n";
      jmp(JumpCondition::kParityEven, fetch_byte(), fetch_byte());
      break;
    case 0xEB:
      std::cout << "XCHG\n";
      xchg();
      break;
    case 0xEC:
      std::cout << "CPE \n";
      call(JumpCondition::kParityEven, fetch_byte(), fetch_byte());
      break;
    case 0xED:
      std::cout << "*CALL !n";
      call(JumpCondition::kTrue, fetch_byte(), fetch_byte());
      break;
    case 0xEE:
      std::cout << "XRI \n";
      xra(fetch_byte());
      break;
    case 0xEF:
      std::cout << "RST 5\n";
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xF0:
      std::cout << "RP\n";
      ret(JumpCondition::kPositive);
      break;
    case 0xF1: {
      std::cout << "POP PSW\n";
      uint8_t temp_flags{0};
      pop(&registers_.reg_a, &temp_flags);
      flags_.from_byte(temp_flags);
      break;
    }
    case 0xF2:
      std::cout << "JP \n";
      jmp(JumpCondition::kPositive, fetch_byte(), fetch_byte());
      break;
    case 0xF3:
      std::cout << "DI\n";
      di();
      break;
    case 0xF4:
      std::cout << "CP \n";
      call(JumpCondition::kPositive, fetch_byte(), fetch_byte());
      break;
    case 0xF5:
      std::cout << "PUSH PSW\n";
      push(registers_.reg_a, flags_.to_byte());
      break;
    case 0xF6:
      std::cout << "ORI \n";
      ora(fetch_byte());
      break;
    case 0xF7:
      std::cout << "RST 6\n";
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xF8:
      std::cout << "RM\n";
      ret(JumpCondition::kMinus);
      break;
    case 0xF9:
      std::cout << "SPHL\n";
      sphl();
      break;
    case 0xFA:
      std::cout << "JM \n";
      jmp(JumpCondition::kMinus, fetch_byte(), fetch_byte());
      break;
    case 0xFB:
      std::cout << "EI\n";
      ei();
      break;
    case 0xFC:
      std::cout << "CM \n";
      call(JumpCondition::kMinus, fetch_byte(), fetch_byte());
      break;
    case 0xFD:
      std::cout << "*CALL \n";
      call(JumpCondition::kTrue, fetch_byte(), fetch_byte());
      break;
    case 0xFE:
      std::cout << "CPI \n";
      cpi(fetch_byte());
      break;
    case 0xFF:
      std::cout << "RST 7\n";
      rst((opcode >> 3) & 0b0000'0111);
      break;
  }
}
}  // namespace intel_8080
