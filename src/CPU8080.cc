#include "CPU8080.h"

#include <iostream>
#include <memory>

#include "Memory8080.h"

namespace intel_8080 {

CPU8080::CPU8080(std::shared_ptr<intel_8080::Memory8080> new_mem,
                 std::shared_ptr<input::InputHandler> input_handler_ptr,
                 std::shared_ptr<audio::Mixer> new_mixer,
                 std::shared_ptr<hardware::ShiftRegister> shift_reg_ptr)
    : mem_access_(std::move(new_mem)),
      mixer_(std::move(new_mixer)),
      shift_register_(std::move(shift_reg_ptr)) {
  stack_pointer_ = 0x0000;
  program_counter_ = 0x0000;
  flags_ = Flags();
  registers_ = Registers();
  INTE_ = false;
  // Do not move shared pointer, as it is shared with the main loop for event
  // polling.
  input_handler_ = std::move(input_handler_ptr);
};

CPU8080::Flags::Flags() {
  sign = 0;
  zero = 0;
  aux_carry = 0;
  parity = 0;
  carry = 0;
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

void CPU8080::Flags::from_byte(uint8_t data) {
  carry = data & 1;
  parity = (data >> 2) & 1;
  aux_carry = (data >> 4) & 1;
  zero = (data >> 6) & 1;
  sign = (data >> 7) & 1;
}

void CPU8080::step() {
  uint8_t instruction;
  if (!interupt_queue_.empty() && INTE_) {
    instruction = interupt_queue_.front();
    interupt_queue_.pop();
  } else {
    instruction = fetch_byte();
  }
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
  return State{.registers = registers_,
               .flags = flags_,
               .stack_pointer = stack_pointer_,
               .program_counter = program_counter_};
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
#ifdef DEBUG
  char ch{0};
  std::cout << "Next instruction: ";
  print_instruction(opcode);
  std::cout << "Execution paused, press Enter to continue... ";
  std::cin.get(ch);
#endif

  switch (opcode) {
    case 0x00:
      break;
    case 0x01: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      lxi(&registers_.reg_b, &registers_.reg_c, b1, b2);
    } break;
    case 0x02:
      stax(registers_.bc());
      break;
    case 0x03:
      inx(&registers_.reg_b, &registers_.reg_c);
      break;
    case 0x04:  // INR B: B += 1 flags
      inr(&registers_.reg_b);
      break;
    case 0x05:
      dcr(&registers_.reg_b);
      break;
    case 0x06: {
      uint8_t b1 = fetch_byte();
      mov(&registers_.reg_b, b1);
      break;
    }
    case 0x07:  // RLC
      rlc();
      break;
    case 0x08:
      break;
    case 0x09:
      dad(&registers_.reg_b, &registers_.reg_c);
      break;
    case 0x0A:
      ldax(registers_.bc());
      break;
    case 0x0B:
      dcx(&registers_.reg_b, &registers_.reg_c);
      break;
    case 0x0C:
      inr(&registers_.reg_c);
      break;
    case 0x0D:
      dcr(&registers_.reg_c);
      break;
    case 0x0E: {
      uint8_t b1 = fetch_byte();
      mov(&registers_.reg_c, b1);
    } break;
    case 0x0F:
      rrc();
      break;
    case 0x10:
      break;
    case 0x11: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      lxi(&registers_.reg_d, &registers_.reg_e, b1, b2);
    } break;
    case 0x12:
      stax(registers_.de());
      break;
    case 0x13:
      inx(&registers_.reg_d, &registers_.reg_e);
      break;
    case 0x14:
      inr(&registers_.reg_d);
      break;
    case 0x15:
      dcr(&registers_.reg_d);
      break;
    case 0x16: {
      uint8_t b1 = fetch_byte();
      mov(&registers_.reg_d, b1);
    } break;
    case 0x17:
      ral();
      break;
    case 0x18:
      break;
    case 0x19:
      dad(&registers_.reg_d, &registers_.reg_e);
      break;
    case 0x1A:
      ldax(registers_.de());
      break;
    case 0x1B:
      dcx(&registers_.reg_d, &registers_.reg_e);
      break;
    case 0x1C:
      inr(&registers_.reg_e);
      break;
    case 0x1D:
      dcr(&registers_.reg_e);
      break;
    case 0x1E: {
      uint8_t b1 = fetch_byte();
      mov(&registers_.reg_e, b1);
    } break;
    case 0x1F:
      rar();
      break;
    case 0x20:
      break;
    case 0x21: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      lxi(&registers_.reg_h, &registers_.reg_l, b1, b2);
    } break;
    case 0x22: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      shld(b1, b2);
    } break;
    case 0x23:
      inx(&registers_.reg_h, &registers_.reg_l);
      break;
    case 0x24:
      inr(&registers_.reg_h);
      break;
    case 0x25:
      dcr(&registers_.reg_h);
      break;
    case 0x26: {
      uint8_t b1 = fetch_byte();
      mov(&registers_.reg_h, b1);
    } break;
    case 0x27:
      // DAA
    case 0x28:
      break;
    case 0x29:
      dad(&registers_.reg_h, &registers_.reg_l);
      break;
    case 0x2A: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      lhld(b1, b2);
    } break;
    case 0x2B:
      dcx(&registers_.reg_h, &registers_.reg_l);
      break;
    case 0x2C:
      inr(&registers_.reg_l);
      break;
    case 0x2D:
      dcr(&registers_.reg_l);
      break;
    case 0x2E: {
      uint8_t b1 = fetch_byte();
      mov(&registers_.reg_l, b1);
    } break;
    case 0x2F:
      cma();
      break;
    case 0x30:
      break;
    case 0x31: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      lxi_sp(b1, b2);
      break;
    }
    case 0x32: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      sta(b1, b2);
    } break;
    case 0x33:
      stack_pointer_ += 1;  // inx sp
      break;
    case 0x34: {
      auto mem_location =
          static_cast<uint16_t>((registers_.reg_h << 8) | registers_.reg_l);
      uint8_t data = mem_access_->read(mem_location);
      inr(&data);
      mem_access_->write(mem_location, data);
    } break;
    case 0x35: {
      auto mem_location =
          static_cast<uint16_t>((registers_.reg_h << 8) | registers_.reg_l);
      uint8_t data = mem_access_->read(registers_.hl());
      dcr(&data);
      mem_access_->write(mem_location, data);
    } break;
    case 0x36: {
      uint8_t b1 = fetch_byte();
      mem_access_->write(registers_.hl(), b1);
      break;
    }
    case 0x37:
      stc();
      break;
    case 0x38:
      break;
    case 0x39: {
      uint8_t sp_high = stack_pointer_ >> 8;
      uint8_t sp_low = stack_pointer_ | 0xFF;
      dad(&sp_high, &sp_low);
      stack_pointer_ = static_cast<uint16_t>((sp_high << 8) | sp_low);
    } break;
    case 0x3A: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      lda(b1, b2);
    } break;
    case 0x3B: {
      uint8_t sp_high = stack_pointer_ >> 8;
      uint8_t sp_low = stack_pointer_ | 0xFF;
      dcx(&sp_high, &sp_low);
      stack_pointer_ = static_cast<uint16_t>((sp_high << 8) | sp_low);
    } break;
    case 0x3C:
      inr(&registers_.reg_a);
      break;
    case 0x3D:
      dcr(&registers_.reg_a);
      break;
    case 0x3E:  // MVI A, D8
    {
      uint8_t b1 = fetch_byte();
      mov(&registers_.reg_a, b1);
      break;
    }
    case 0x3F:
      cmc();
      break;
    case 0x40:
      mov(&registers_.reg_b, registers_.reg_b);
      break;
    case 0x41:
      mov(&registers_.reg_b, registers_.reg_c);
      break;
    case 0x42:
      mov(&registers_.reg_b, registers_.reg_d);
      break;
    case 0x43:
      mov(&registers_.reg_b, registers_.reg_e);
      break;
    case 0x44:
      mov(&registers_.reg_b, registers_.reg_h);
      break;
    case 0x45:
      mov(&registers_.reg_b, registers_.reg_l);
      break;
    case 0x46:
      mov(&registers_.reg_b, mem_access_->read(registers_.hl()));
    case 0x47:
      mov(&registers_.reg_b, registers_.reg_a);
      break;
    case 0x48:
      mov(&registers_.reg_c, registers_.reg_b);
      break;
    case 0x49:
      mov(&registers_.reg_c, registers_.reg_c);
      break;
    case 0x4A:
      mov(&registers_.reg_c, registers_.reg_d);
      break;
    case 0x4B:
      mov(&registers_.reg_c, registers_.reg_e);
      break;
    case 0x4C:
      mov(&registers_.reg_c, registers_.reg_h);
      break;
    case 0x4D:
      mov(&registers_.reg_c, registers_.reg_l);
      break;
    case 0x4E:
      mov(&registers_.reg_c, mem_access_->read(registers_.hl()));
      break;
    case 0x4F:
      mov(&registers_.reg_c, registers_.reg_a);
      break;
    case 0x50:
      mov(&registers_.reg_d, registers_.reg_b);
      break;
    case 0x51:
      mov(&registers_.reg_d, registers_.reg_c);
      break;
    case 0x52:
      mov(&registers_.reg_d, registers_.reg_d);
      break;
    case 0x53:
      mov(&registers_.reg_d, registers_.reg_e);
      break;
    case 0x54:
      mov(&registers_.reg_d, registers_.reg_h);
      break;
    case 0x55:
      mov(&registers_.reg_d, registers_.reg_l);
      break;
    case 0x56:
      mov(&registers_.reg_d, mem_access_->read(registers_.hl()));
      break;
    case 0x57:
      mov(&registers_.reg_d, registers_.reg_a);
      break;
    case 0x58:
      mov(&registers_.reg_e, registers_.reg_b);
      break;
    case 0x59:
      mov(&registers_.reg_e, registers_.reg_c);
      break;
    case 0x5A:
      mov(&registers_.reg_e, registers_.reg_d);
      break;
    case 0x5B:
      mov(&registers_.reg_e, registers_.reg_e);
      break;
    case 0x5C:
      mov(&registers_.reg_e, registers_.reg_h);
      break;
    case 0x5D:
      mov(&registers_.reg_e, registers_.reg_l);
      break;
    case 0x5E:
      mov(&registers_.reg_e, mem_access_->read(registers_.hl()));
      break;
    case 0x5F:
      mov(&registers_.reg_e, registers_.reg_a);
      break;
    case 0x60:
      mov(&registers_.reg_h, registers_.reg_b);
      break;
    case 0x61:
      mov(&registers_.reg_h, registers_.reg_c);
      break;
    case 0x62:
      mov(&registers_.reg_h, registers_.reg_d);
      break;
    case 0x63:
      mov(&registers_.reg_h, registers_.reg_e);
      break;
    case 0x64:
      mov(&registers_.reg_h, registers_.reg_h);
      break;
    case 0x65:
      mov(&registers_.reg_h, registers_.reg_l);
      break;
    case 0x66:
      mov(&registers_.reg_h, mem_access_->read(registers_.hl()));
      break;
    case 0x67:
      mov(&registers_.reg_h, registers_.reg_a);
      break;
    case 0x68:
      mov(&registers_.reg_l, registers_.reg_b);
      break;
    case 0x69:
      mov(&registers_.reg_l, registers_.reg_c);
      break;
    case 0x6A:
      mov(&registers_.reg_l, registers_.reg_d);
      break;
    case 0x6B:
      mov(&registers_.reg_l, registers_.reg_e);
      break;
    case 0x6C:
      mov(&registers_.reg_l, registers_.reg_h);
      break;
    case 0x6D:
      mov(&registers_.reg_l, registers_.reg_l);
      break;
    case 0x6E:
      mov(&registers_.reg_l, mem_access_->read(registers_.hl()));
      break;
    case 0x6F:
      mov(&registers_.reg_l, registers_.reg_a);
      break;
    case 0x70:
      mem_access_->write(registers_.hl(), registers_.reg_b);
      break;
    case 0x71:
      mem_access_->write(registers_.hl(), registers_.reg_c);
      break;
    case 0x72:
      mem_access_->write(registers_.hl(), registers_.reg_d);
      break;
    case 0x73:
      mem_access_->write(registers_.hl(), registers_.reg_e);
      break;
    case 0x74:
      mem_access_->write(registers_.hl(), registers_.reg_h);
      break;
    case 0x75:
      mem_access_->write(registers_.hl(), registers_.reg_l);
      break;
    case 0x76:
      break;
    case 0x77:
      mem_access_->write(registers_.hl(), registers_.reg_a);
      break;
    case 0x78:
      mov(&registers_.reg_a, registers_.reg_b);
      break;
    case 0x79:
      mov(&registers_.reg_a, registers_.reg_c);
      break;
    case 0x7A:
      mov(&registers_.reg_a, registers_.reg_d);
      break;
    case 0x7B:
      mov(&registers_.reg_a, registers_.reg_e);
      break;
    case 0x7C:
      mov(&registers_.reg_a, registers_.reg_h);
      break;
    case 0x7D:
      mov(&registers_.reg_a, registers_.reg_l);
      break;
    case 0x7E:
      mov(&registers_.reg_a, mem_access_->read(registers_.hl()));
      break;
    case 0x7F:
      mov(&registers_.reg_a, registers_.reg_a);
      break;
    case 0x80:
      add(registers_.reg_b);
      break;
    case 0x81:
      add(registers_.reg_c);
      break;
    case 0x82:
      add(registers_.reg_d);
      break;
    case 0x83:
      add(registers_.reg_e);
      break;
    case 0x84:
      add(registers_.reg_h);
      break;
    case 0x85:
      add(registers_.reg_l);
      break;
    case 0x86:
      add(mem_access_->read(registers_.hl()));
      break;
    case 0x87:
      add(registers_.reg_a);
      break;
    case 0x88:
      adc(registers_.reg_b);
      break;
    case 0x89:
      adc(registers_.reg_c);
      break;
    case 0x8A:
      adc(registers_.reg_d);
      break;
    case 0x8B:
      adc(registers_.reg_e);
      break;
    case 0x8C:
      adc(registers_.reg_h);
      break;
    case 0x8D:
      adc(registers_.reg_l);
      break;
    case 0x8E:
      adc(mem_access_->read(registers_.hl()));
      break;
    case 0x8F:
      adc(registers_.reg_a);
      break;
    case 0x90:
      sub(registers_.reg_b);
      break;
    case 0x91:
      sub(registers_.reg_c);
      break;
    case 0x92:
      sub(registers_.reg_d);
      break;
    case 0x93:
      sub(registers_.reg_e);
      break;
    case 0x94:
      sub(registers_.reg_h);
      break;
    case 0x95:
      sub(registers_.reg_l);
      break;
    case 0x96:
      sub(mem_access_->read(registers_.hl()));
      break;
    case 0x97:
      sub(registers_.reg_a);
      break;
    case 0x98:
      sbb(registers_.reg_b);
      break;
    case 0x99:
      sbb(registers_.reg_c);
      break;
    case 0x9A:
      sbb(registers_.reg_d);
      break;
    case 0x9B:
      sbb(registers_.reg_e);
      break;
    case 0x9C:
      sbb(registers_.reg_h);
      break;
    case 0x9D:
      sbb(registers_.reg_l);
      break;
    case 0x9E:
      sbb(mem_access_->read(registers_.hl()));
      break;
    case 0x9F:
      sbb(registers_.reg_a);
      break;
    case 0xA0:
      ana(registers_.reg_b);
      break;
    case 0xA1:
      ana(registers_.reg_c);
      break;
    case 0xA2:
      ana(registers_.reg_d);
      break;
    case 0xA3:
      ana(registers_.reg_e);
      break;
    case 0xA4:
      ana(registers_.reg_h);
      break;
    case 0xA5:
      ana(registers_.reg_l);
      break;
    case 0xA6:
      ana(mem_access_->read(registers_.hl()));
      break;
    case 0xA7:
      ana(registers_.reg_a);
      break;
    case 0xA8:
      xra(registers_.reg_b);
      break;
    case 0xA9:
      xra(registers_.reg_c);
      break;
    case 0xAA:
      xra(registers_.reg_d);
      break;
    case 0xAB:
      xra(registers_.reg_e);
      break;
    case 0xAC:
      xra(registers_.reg_h);
      break;
    case 0xAD:
      xra(registers_.reg_l);
      break;
    case 0xAE:
      xra(mem_access_->read(registers_.hl()));
      break;
    case 0xAF:
      xra(registers_.reg_a);
      break;
    case 0xB0:
      ora(registers_.reg_b);
      break;
    case 0xB1:
      ora(registers_.reg_c);
      break;
    case 0xB2:
      ora(registers_.reg_d);
      break;
    case 0xB3:
      ora(registers_.reg_e);
      break;
    case 0xB4:
      ora(registers_.reg_h);
      break;
    case 0xB5:
      ora(registers_.reg_l);
      break;
    case 0xB6: {
      uint8_t data = mem_access_->read(registers_.hl());
      ora(data);
      break;
    }
    case 0xB7:
      ora(registers_.reg_a);
      break;
    case 0xB8:
      cmp(registers_.reg_b);
      break;
    case 0xB9:
      cmp(registers_.reg_c);
      break;
    case 0xBA:
      cmp(registers_.reg_d);
      break;
    case 0xBB:
      cmp(registers_.reg_e);
      break;
    case 0xBC:
      cmp(registers_.reg_h);
      break;
    case 0xBD:
      cmp(registers_.reg_l);
      break;
    case 0xBE: {
      uint8_t data = mem_access_->read(registers_.hl());
      cmp(data);
      break;
    }
    case 0xBF:
      cmp(registers_.reg_a);
      break;
    case 0xC0:
      ret(JumpCondition::kNotZero);
      break;
    case 0xC1:
      pop(&registers_.reg_b, &registers_.reg_c);
      break;
    case 0xC2: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kNotZero, b1, b2);
    } break;
    case 0xC3: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kTrue, b1, b2);
      break;
    }
    case 0xC4: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kNotZero, b1, b2);
    } break;
    case 0xC5:
      push(registers_.reg_b, registers_.reg_c);
      break;
    case 0xC6: {
      uint8_t b1 = fetch_byte();
      add(b1);
      break;
    }
    case 0xC7:
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xC8:
      ret(JumpCondition::kZero);
      break;
    case 0xC9:
      ret(JumpCondition::kTrue);
      break;
    case 0xCA: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kZero, b1, b2);
      break;
    }
    case 0xCB: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kTrue, b1, b2);
      break;
    }
    case 0xCC: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kZero, b1, b2);
    } break;
    case 0xCD: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kTrue, b1, b2);
      break;
    }
    case 0xCE:
      ana(fetch_byte());
      break;
    case 0xCF:
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xD0:
      ret(JumpCondition::kNotCarry);
      break;
    case 0xD1:
      pop(&registers_.reg_d, &registers_.reg_e);
      break;
    case 0xD2: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kNotCarry, b1, b2);
      break;
    }
    case 0xD3:
      break;
    case 0xD4: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kNotCarry, b1, b2);
      break;
    }
    case 0xD5:
      push(registers_.reg_d, registers_.reg_e);
      break;
    case 0xD6:
      sub(fetch_byte());
      break;
    case 0xD7:
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xD8:
      ret(JumpCondition::kCarry);
      break;
    case 0xD9:
      ret(JumpCondition::kTrue);
      break;
    case 0xDA: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kCarry, b1, b2);
      break;
    }
    case 0xDB:  // IN instruction + D8 (input port number)
      in(fetch_byte());
      break;
    case 0xDC: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kCarry, b1, b2);
    } break;
    case 0xDD: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kTrue, b1, b2);
      break;
    }
    case 0xDE:
      sbb(fetch_byte());
      break;
    case 0xDF:
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xE0:
      ret(JumpCondition::kParityOdd);
      break;
    case 0xE1:
      pop(&registers_.reg_h, &registers_.reg_l);
      break;
    case 0xE2: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kParityOdd, b1, b2);
      break;
    }
    case 0xE3:
      xthl();
      break;
    case 0xE4: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kParityOdd, b1, b2);
    } break;
    case 0xE5:
      push(registers_.reg_h, registers_.reg_l);
      break;
    case 0xE6:
      ana(fetch_byte());
      break;
    case 0xE7:
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xE8:
      ret(JumpCondition::kParityEven);
      break;
    case 0xE9:
      pchl();
      break;
    case 0xEA: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kParityEven, b1, b2);
    } break;
    case 0xEB:
      xchg();
      break;
    case 0xEC: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kParityEven, b1, b2);
      break;
    }
    case 0xED: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kTrue, b1, b2);
      break;
    }
    case 0xEE: {
      uint8_t b1 = fetch_byte();
      xra(b1);
      break;
    }
    case 0xEF:
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xF0:
      ret(JumpCondition::kPositive);
      break;
    case 0xF1: {
      uint8_t temp_flags{0};
      pop(&registers_.reg_a, &temp_flags);
      flags_.from_byte(temp_flags);
      break;
    }
    case 0xF2: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kPositive, b1, b2);
    } break;
    case 0xF3:
      di();
      break;
    case 0xF4: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kPositive, b1, b2);
    } break;
    case 0xF5:
      push(registers_.reg_a, flags_.to_byte());
      break;
    case 0xF6:
      ora(fetch_byte());
      break;
    case 0xF7:
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xF8:
      ret(JumpCondition::kMinus);
      break;
    case 0xF9:
      sphl();
      break;
    case 0xFA: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kMinus, b1, b2);
    } break;
    case 0xFB:
      ei();
      break;
    case 0xFC: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kMinus, b1, b2);
    } break;
    case 0xFD: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kTrue, b1, b2);
    } break;
    case 0xFE:
      cpi(fetch_byte());
      break;
    case 0xFF:
      rst((opcode >> 3) & 0b0000'0111);
      break;
    default:
      std::cerr << "CPU8080:execute() : default case for opcode switch\n";
      break;
  }
#ifdef DEBUG
  print_debug();
#endif
}
}  // namespace intel_8080
