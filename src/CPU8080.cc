#include "CPU8080.h"

#include <iostream>
#include <memory>

#include "Memory8080.h"

namespace intel_8080 {

CPU8080::CPU8080(std::shared_ptr<intel_8080::Memory8080> new_mem)
    : mem_access_(std::move(new_mem)) {
  stack_pointer_ = 0x0000;
  program_counter_ = 0x0000;
  flags_ = Flags();
  registers_ = Registers();
  INTE_ = false;
  input_port_1_ = Port();
  input_port_2_ = Port();
};

std::uint8_t CPU8080::Port::to_byte() {
  std::uint8_t return_byte = bit0 | bit1 << 1 | bit2 << 2 | bit3 << 3 |
                             bit4 << 4 | bit5 << 5 | bit6 << 6 | bit7 << 7;
  return return_byte;
}

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

void CPU8080::step() {
  std::uint8_t instruction = fetch_byte();
  execute(instruction);
};

std::uint8_t CPU8080::fetch_byte() {
  std::uint8_t byte = mem_access_->read(program_counter_);
  program_counter_++;
  return byte;
};

std::uint8_t CPU8080::fetch_byte(std::uint16_t mem_location) {
  std::uint8_t byte = mem_access_->read(mem_location);
  return byte;
};

std::uint16_t CPU8080::fetch_word() {
  std::uint8_t low_byte = fetch_byte();
  std::uint8_t high_byte = fetch_byte();
  std::uint16_t word = ((std::uint16_t)high_byte << 8) | low_byte;
  return word;
}

std::uint16_t CPU8080::fetch_word(std::uint16_t mem_location) {
  std::uint8_t low_byte = mem_access_->read(mem_location);
  std::uint8_t high_byte = mem_access_->read(mem_location + 1);
  std::uint16_t word = ((std::uint16_t)high_byte << 8) | low_byte;
  return word;
};

// Copies the state of the CPU and returns in a State struct.
CPU8080::State CPU8080::get_state() {
  return State{registers_, flags_, stack_pointer_, program_counter_};
}

// Returns the data in input port 1 or 2 as a byte.
//
// When calling, ensure that the port number is either 1 or 2.
uint8_t CPU8080::read_input_port(uint8_t port_no) {
  uint8_t data{0};
  if (port_no == 1) {
    data = input_port_1_.to_byte();
  } else if (port_no == 2) {
    data = input_port_2_.to_byte();
  }
  return data;
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

void CPU8080::write_input_port(uint8_t port_no, uint8_t data) {
  Port* port;
  if (port_no == 1) {
    port = &input_port_1_;
  } else if (port_no == 2) {
    port = &input_port_2_;
  } else {
    std::cerr << "<write_input_port> Invalid input port number: " << port_no
              << std::endl;
    return;
  }

  port->bit0 = (data & 0x1) == 0x1;
  port->bit1 = (data & 0x2) == 0x2;
  port->bit2 = (data & 0x4) == 0x4;
  port->bit3 = (data & 0x8) == 0x8;
  port->bit4 = (data & 0x10) == 0x10;
  port->bit5 = (data & 0x20) == 0x20;
  port->bit6 = (data & 0x40) == 0x40;
  port->bit7 = (data & 0x80) == 0x80;
}

void CPU8080::execute(std::uint8_t opcode) {
  switch (opcode) {
    case 0x00:
      break;
    case 0x01:
      break;
    case 0x02:
      break;
    case 0x03:
      break;
    case 0x04:  // INR B: B += 1 flags
      inr(&registers_.reg_b);
      break;
    case 0x05:
      break;
    case 0x06:
      break;
    case 0x07:  // RLC
      rlc();
      break;
    case 0x08:
      break;
    case 0x09:
      break;
    case 0x0A:
      break;
    case 0x0B:
      break;
    case 0x0C:
      break;
    case 0x0D:
      break;
    case 0x0E:
      break;
    case 0x0F:
      break;
    case 0x10:
      break;
    case 0x11:
      break;
    case 0x12:
      break;
    case 0x13:
      break;
    case 0x14:
      break;
    case 0x15:
      break;
    case 0x16:
      break;
    case 0x17:
      break;
    case 0x18:
      break;
    case 0x19:
      break;
    case 0x1A:
      break;
    case 0x1B:
      break;
    case 0x1C:
      break;
    case 0x1D:
      break;
    case 0x1E:
      break;
    case 0x1F:
      break;
    case 0x20:
      break;
    case 0x21:
      break;
    case 0x22:
      break;
    case 0x23:
      break;
    case 0x24:
      break;
    case 0x25:
      break;
    case 0x26:
      break;
    case 0x27:
      break;
    case 0x28:
      break;
    case 0x29:
      break;
    case 0x2A:
      break;
    case 0x2B:
      break;
    case 0x2C:
      break;
    case 0x2D:
      break;
    case 0x2E:
      break;
    case 0x2F:
      break;
    case 0x30:
      break;
    case 0x31:
      break;
    case 0x32:
      break;
    case 0x33:
      break;
    case 0x34:
      break;
    case 0x35:
      break;
    case 0x36:
      break;
    case 0x37:
      break;
    case 0x38:
      break;
    case 0x39:
      break;
    case 0x3A:
      break;
    case 0x3B:
      break;
    case 0x3C:
      break;
    case 0x3D:
      break;
    case 0x3E:  // MVI A, D8
      mov(&registers_.reg_a, fetch_byte());
      break;
    case 0x3F:
      break;
    case 0x40:
      break;
    case 0x41:
      break;
    case 0x42:
      break;
    case 0x43:
      break;
    case 0x44:
      break;
    case 0x45:
      break;
    case 0x46:
      break;
    case 0x47:
      break;
    case 0x48:
      break;
    case 0x49:
      break;
    case 0x4A:
      break;
    case 0x4B:
      break;
    case 0x4C:
      break;
    case 0x4D:
      break;
    case 0x4E:
      break;
    case 0x4F:
      break;
    case 0x50:
      break;
    case 0x51:
      break;
    case 0x52:
      break;
    case 0x53:
      break;
    case 0x54:
      break;
    case 0x55:
      break;
    case 0x56:
      break;
    case 0x57:
      break;
    case 0x58:
      break;
    case 0x59:
      break;
    case 0x5A:
      break;
    case 0x5B:
      break;
    case 0x5C:
      break;
    case 0x5D:
      break;
    case 0x5E:
      break;
    case 0x5F:
      break;
    case 0x60:
      break;
    case 0x61:
      break;
    case 0x62:
      break;
    case 0x63:
      break;
    case 0x64:
      break;
    case 0x65:
      break;
    case 0x66:
      break;
    case 0x67:
      break;
    case 0x68:
      break;
    case 0x69:
      break;
    case 0x6A:
      break;
    case 0x6B:
      break;
    case 0x6C:
      break;
    case 0x6D:
      break;
    case 0x6E:
      break;
    case 0x6F:
      break;
    case 0x70:
      break;
    case 0x71:
      break;
    case 0x72:
      break;
    case 0x73:
      break;
    case 0x74:
      break;
    case 0x75:
      break;
    case 0x76:
      break;
    case 0x77:
      break;
    case 0x78:
      break;
    case 0x79:
      break;
    case 0x7A:
      break;
    case 0x7B:
      break;
    case 0x7C:
      break;
    case 0x7D:
      break;
    case 0x7E:
      break;
    case 0x7F:
      break;
    case 0x80:
      break;
    case 0x81:
      break;
    case 0x82:
      break;
    case 0x83:
      break;
    case 0x84:
      break;
    case 0x85:
      break;
    case 0x86:
      break;
    case 0x87:
      break;
    case 0x88:
      break;
    case 0x89:
      break;
    case 0x8A:
      break;
    case 0x8B:
      break;
    case 0x8C:
      break;
    case 0x8D:
      break;
    case 0x8E:
      break;
    case 0x8F:
      break;
    case 0x90:
      break;
    case 0x91:
      break;
    case 0x92:
      break;
    case 0x93:
      break;
    case 0x94:
      break;
    case 0x95:
      break;
    case 0x96:
      break;
    case 0x97:
      break;
    case 0x98:
      break;
    case 0x99:
      break;
    case 0x9A:
      break;
    case 0x9B:
      break;
    case 0x9C:
      break;
    case 0x9D:
      break;
    case 0x9E:
      break;
    case 0x9F:
      break;
    case 0xA0:
      break;
    case 0xA1:
      break;
    case 0xA2:
      break;
    case 0xA3:
      break;
    case 0xA4:
      break;
    case 0xA5:
      break;
    case 0xA6:
      break;
    case 0xA7:
      break;
    case 0xA8:
      break;
    case 0xA9:
      break;
    case 0xAA:
      break;
    case 0xAB:
      break;
    case 0xAC:
      break;
    case 0xAD:
      break;
    case 0xAE:
      break;
    case 0xAF:
      break;
    case 0xB0:
      break;
    case 0xB1:
      break;
    case 0xB2:
      break;
    case 0xB3:
      break;
    case 0xB4:
      break;
    case 0xB5:
      break;
    case 0xB6:
      break;
    case 0xB7:
      break;
    case 0xB8:
      break;
    case 0xB9:
      break;
    case 0xBA:
      break;
    case 0xBB:
      break;
    case 0xBC:
      break;
    case 0xBD:
      break;
    case 0xBE:
      break;
    case 0xBF:
      break;
    case 0xC0:
      break;
    case 0xC1:
      break;
    case 0xC2:
      break;
    case 0xC3:
      break;
    case 0xC4:
      break;
    case 0xC5:
      break;
    case 0xC6:
      break;
    case 0xC7:
      break;
    case 0xC8:
      break;
    case 0xC9:
      break;
    case 0xCA:
      break;
    case 0xCB:
      break;
    case 0xCC:
      break;
    case 0xCD:
      break;
    case 0xCE:
      break;
    case 0xCF:
      break;
    case 0xD0:
      break;
    case 0xD1:
      break;
    case 0xD2:
      break;
    case 0xD3:
      break;
    case 0xD4:
      break;
    case 0xD5:
      break;
    case 0xD6:
      break;
    case 0xD7:
      break;
    case 0xD8:
      break;
    case 0xD9:
      break;
    case 0xDA:
      break;
    case 0xDB: {
      uint8_t port_no = fetch_byte();
      if (port_no != 1 && port_no != 2) {
        std::cerr << "<opcode 0xDB> Invalid input port number: " << port_no
                  << std::endl;
        break;
      }
      in(read_input_port(port_no), &registers_.reg_a);
    } break;
    case 0xDC:
      break;
    case 0xDD:
      break;
    case 0xDE:
      break;
    case 0xDF:
      break;
    case 0xE0:
      break;
    case 0xE1:
      break;
    case 0xE2:
      break;
    case 0xE3:
      break;
    case 0xE4:
      break;
    case 0xE5:
      break;
    case 0xE6:
      break;
    case 0xE7:
      break;
    case 0xE8:
      break;
    case 0xE9:
      break;
    case 0xEA:
      break;
    case 0xEB:
      break;
    case 0xEC:
      break;
    case 0xED:
      break;
    case 0xEE:
      break;
    case 0xEF:
      break;
    case 0xF0:
      break;
    case 0xF1:
      break;
    case 0xF2:
      break;
    case 0xF3:
      break;
    case 0xF4:
      break;
    case 0xF5:
      break;
    case 0xF6:
      break;
    case 0xF7:
      break;
    case 0xF8:
      break;
    case 0xF9:
      break;
    case 0xFA:
      break;
    case 0xFB:
      break;
    case 0xFC:
      break;
    case 0xFD:
      break;
    case 0xFE:
      break;
    case 0xFF:
      break;
  }
}
}  // namespace intel_8080
