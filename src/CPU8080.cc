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
  input_handler_ = input_handler_ptr;
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
  switch (opcode) {
    case 0x00:
      std::cout << "NOP" << std::endl;
      break;
    case 0x01:
      std::cout << "LXI B, d16\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        lxi(&registers_.reg_b, &registers_.reg_c, b1, b2);
      }
      break;
    case 0x02:
      std::cout << "STAX B\n";
      stax(registers_.bc());
      break;
    case 0x03:
      std::cout << "INX B\n";
      inx(&registers_.reg_b, &registers_.reg_c);
      break;
    case 0x04:  // INR B: B += 1 flags
      std::cout << "INR B\n";
      inr(&registers_.reg_b);
      break;
    case 0x05:
      std::cout << "DCR B\n";
      dcr(&registers_.reg_b);
      break;
    case 0x06: {
      std::cout << "MOV B, D8 (";
      uint8_t b1 = fetch_byte();
      print_hex_byte(b1);
      std::cout << ")\n";
      mov(&registers_.reg_b, b1);
      break;
    }
    case 0x07:  // RLC
      std::cout << "RLC\n";
      rlc();
      break;
    case 0x08:
      std::cout << "NOP*" << std::endl;
      break;
    case 0x09:
      std::cout << "DAD B\n";
      dad(&registers_.reg_b, &registers_.reg_c);
      break;
    case 0x0A:
      std::cout << "LDAX B\n";
      ldax(registers_.bc());
      break;
    case 0x0B:
      std::cout << "DCX B\n";
      dcx(&registers_.reg_b, &registers_.reg_c);
      break;
    case 0x0C:
      std::cout << "INR C\n";
      inr(&registers_.reg_c);
      break;
    case 0x0D:
      std::cout << "DCR C\n";
      dcr(&registers_.reg_c);
      break;
    case 0x0E:
      std::cout << "MOV C, D8\n";
      {
        uint8_t b1 = fetch_byte();
        mov(&registers_.reg_c, b1);
      }
      break;
    case 0x0F:
      std::cout << "RRC" << std::endl;
      rrc();
      break;
    case 0x10:
      std::cout << "NOP*" << std::endl;
      break;
    case 0x11:
      std::cout << "LXI D, D16\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        lxi(&registers_.reg_d, &registers_.reg_e, b1, b2);
      }
      break;
    case 0x12:
      std::cout << "STAX D\n";
      stax(registers_.de());
      break;
    case 0x13:
      std::cout << "INX D\n";
      inx(&registers_.reg_d, &registers_.reg_e);
      break;
    case 0x14:
      std::cout << "INR D\n";
      inr(&registers_.reg_d);
      break;
    case 0x15:
      std::cout << "DCR D\n";
      dcr(&registers_.reg_d);
      break;
    case 0x16:
      std::cout << "MOV D, D8\n";
      {
        uint8_t b1 = fetch_byte();
        mov(&registers_.reg_d, b1);
      }
      break;
    case 0x17:
      std::cout << "RAL \n";
      ral();
      break;
    case 0x18:
      std::cout << "*NOP" << std::endl;
      break;
    case 0x19:
      std::cout << "DAD D\n";
      dad(&registers_.reg_d, &registers_.reg_e);
      break;
    case 0x1A:
      std::cout << "LDAX D\n";
      ldax(registers_.de());
      break;
    case 0x1B:
      std::cout << "DCX D\n";
      dcx(&registers_.reg_d, &registers_.reg_e);
      break;
    case 0x1C:
      std::cout << "INR E\n";
      inr(&registers_.reg_e);
      break;
    case 0x1D:
      std::cout << "DCR E\n";
      dcr(&registers_.reg_e);
      break;
    case 0x1E:
      std::cout << "MOV E, d8\n";
      {
        uint8_t b1 = fetch_byte();
        mov(&registers_.reg_e, b1);
      }
      break;
    case 0x1F:
      std::cout << "RAR\n";
      rar();
      break;
    case 0x20:
      std::cout << "*NOP" << std::endl;
      break;
    case 0x21:
      std::cout << "LXI H, d16\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        lxi(&registers_.reg_h, &registers_.reg_l, b1, b2);
      }
      break;
    case 0x22:
      std::cout << "SHLD D8\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        shld(b1, b2);
      }
      break;
    case 0x23:
      std::cout << "INX H" << std::endl;
      inx(&registers_.reg_h, &registers_.reg_l);
      break;
    case 0x24:
      std::cout << "INR H" << std::endl;
      inr(&registers_.reg_h);
      break;
    case 0x25:
      std::cout << "DCR H" << std::endl;
      dcr(&registers_.reg_h);
      break;
    case 0x26:
      std::cout << "MVI H, d8\n";
      {
        uint8_t b1 = fetch_byte();
        mov(&registers_.reg_h, b1);
      }
      break;
    case 0x27:
      std::cout << "DAA" << std::endl;
      break;
    case 0x28:
      std::cout << "*NOP" << std::endl;
      break;
    case 0x29:
      std::cout << "DAD H" << std::endl;
      dad(&registers_.reg_h, &registers_.reg_l);
      break;
    case 0x2A:
      std::cout << "LHLD d8\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        lhld(b1, b2);
      }
      break;
    case 0x2B:
      std::cout << "DCX H" << std::endl;
      dcx(&registers_.reg_h, &registers_.reg_l);
      break;
    case 0x2C:
      std::cout << "INR L" << std::endl;
      inr(&registers_.reg_l);
      break;
    case 0x2D:
      std::cout << "DCR L" << std::endl;
      dcr(&registers_.reg_l);
      break;
    case 0x2E:
      std::cout << "MVI L, d8\n";
      {
        uint8_t b1 = fetch_byte();
        mov(&registers_.reg_l, b1);
      }
      break;
    case 0x2F:
      std::cout << "CMA" << std::endl;
      cma();
      break;
    case 0x30:
      std::cout << "*NOP" << std::endl;
      break;
    case 0x31: {
      std::cout << "LXI SP, d16 (";
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      print_hex_byte(b2);
      print_hex_byte(b1);
      std::cout << ")\n";
      lxi_sp(b1, b2);
      break;
    }
    case 0x32:
      std::cout << "STA, d16\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        sta(b1, b2);
      }
      break;
    case 0x33:
      std::cout << "INX SP" << std::endl;
      stack_pointer_ += 1;  // inx sp
      break;
    case 0x34: {
      std::cout << "INR M" << std::endl;
      auto mem_location =
          static_cast<uint16_t>((registers_.reg_h << 8) | registers_.reg_l);
      uint8_t data = mem_access_->read(mem_location);
      inr(&data);
      mem_access_->write(mem_location, data);
    } break;
    case 0x35: {
      std::cout << "DCR M" << std::endl;
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
      std::cout << "STC" << std::endl;
      stc();
      break;
    case 0x38:
      std::cout << "NOP*\n";
      break;
    case 0x39: {
      std::cout << "DAD SP" << std::endl;
      uint8_t sp_high = stack_pointer_ >> 8;
      uint8_t sp_low = stack_pointer_ | 0xFF;
      dad(&sp_high, &sp_low);
      stack_pointer_ = static_cast<uint16_t>((sp_high << 8) | sp_low);
    } break;
    case 0x3A:
      std::cout << "LDA ";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        lda(b1, b2);
      }
      break;
    case 0x3B: {
      std::cout << "DCX SP" << std::endl;
      uint8_t sp_high = stack_pointer_ >> 8;
      uint8_t sp_low = stack_pointer_ | 0xFF;
      dcx(&sp_high, &sp_low);
      stack_pointer_ = static_cast<uint16_t>((sp_high << 8) | sp_low);
    } break;
    case 0x3C:
      std::cout << "INR A" << std::endl;
      inr(&registers_.reg_a);
      break;
    case 0x3D:
      std::cout << "DCR A" << std::endl;
      dcr(&registers_.reg_a);
      break;
    case 0x3E:  // MVI A, D8
      std::cout << "MVI A, D8" << std::endl;
      {
        uint8_t b1 = fetch_byte();
        mov(&registers_.reg_a, b1);
        break;
      }
    case 0x3F:
      std::cout << "CMC" << std::endl;
      cmc();
      break;
    case 0x40:
      std::cout << "MOV B, B\n";
      mov(&registers_.reg_b, registers_.reg_b);
      break;
    case 0x41:
      std::cout << "MOV B, C\n";
      mov(&registers_.reg_b, registers_.reg_c);
      break;
    case 0x42:
      std::cout << "MOV B, D\n";
      mov(&registers_.reg_b, registers_.reg_d);
      break;
    case 0x43:
      std::cout << "MOV B, E\n";
      mov(&registers_.reg_b, registers_.reg_e);
      break;
    case 0x44:
      std::cout << "MOV B, H\n";
      mov(&registers_.reg_b, registers_.reg_h);
      break;
    case 0x45:
      std::cout << "MOV B, L\n";
      mov(&registers_.reg_b, registers_.reg_l);
      break;
    case 0x46:
      std::cout << "MOV B, M\n";
      mov(&registers_.reg_b, mem_access_->read(registers_.hl()));
    case 0x47:
      std::cout << "MOV B, A\n";
      mov(&registers_.reg_b, registers_.reg_a);
      break;
    case 0x48:
      mov(&registers_.reg_c, registers_.reg_b);
      std::cout << "MOV C, B\n";
      break;
    case 0x49:
      mov(&registers_.reg_c, registers_.reg_c);
      std::cout << "MOV c, c\n";
      break;
    case 0x4A:
      mov(&registers_.reg_c, registers_.reg_d);
      std::cout << "MOV c, d\n";
      break;
    case 0x4B:
      mov(&registers_.reg_c, registers_.reg_e);
      std::cout << "MOV c, e\n";
      break;
    case 0x4C:
      mov(&registers_.reg_c, registers_.reg_h);
      std::cout << "MOV c, h\n";
      break;
    case 0x4D:
      mov(&registers_.reg_c, registers_.reg_l);
      std::cout << "MOV c, l\n";
      break;
    case 0x4E:
      mov(&registers_.reg_c, mem_access_->read(registers_.hl()));
      std::cout << "MOV c, m\n";
      break;
    case 0x4F:
      mov(&registers_.reg_c, registers_.reg_a);
      std::cout << "MOV c, a\n";
      break;
    case 0x50:
      mov(&registers_.reg_d, registers_.reg_b);
      std::cout << "MOV d, C\n";
      break;
    case 0x51:
      mov(&registers_.reg_d, registers_.reg_c);
      std::cout << "MOV D, C\n";
      break;
    case 0x52:
      mov(&registers_.reg_d, registers_.reg_d);
      std::cout << "MOV D, d\n";
      break;
    case 0x53:
      mov(&registers_.reg_d, registers_.reg_e);
      std::cout << "MOV D, e\n";
      break;
    case 0x54:
      mov(&registers_.reg_d, registers_.reg_h);
      std::cout << "MOV D, h\n";
      break;
    case 0x55:
      mov(&registers_.reg_d, registers_.reg_l);
      std::cout << "MOV D, L\n";
      break;
    case 0x56:
      mov(&registers_.reg_d, mem_access_->read(registers_.hl()));
      std::cout << "MOV D, M\n";
      break;
    case 0x57:
      mov(&registers_.reg_d, registers_.reg_a);
      std::cout << "MOV D, C\n";
      break;
    case 0x58:
      mov(&registers_.reg_e, registers_.reg_b);
      std::cout << "MOV E, b\n";
      break;
    case 0x59:
      mov(&registers_.reg_e, registers_.reg_c);
      std::cout << "MOV E, C\n";
      break;
    case 0x5A:
      mov(&registers_.reg_e, registers_.reg_d);
      std::cout << "MOV E, d\n";
      break;
    case 0x5B:
      mov(&registers_.reg_e, registers_.reg_e);
      std::cout << "MOV E, e\n";
      break;
    case 0x5C:
      std::cout << "MOV E, h\n";
      mov(&registers_.reg_e, registers_.reg_h);
      break;
    case 0x5D:
      std::cout << "MOV E, l\n";
      mov(&registers_.reg_e, registers_.reg_l);
      break;
    case 0x5E:
      std::cout << "MOV E, M\n";
      mov(&registers_.reg_e, mem_access_->read(registers_.hl()));
      break;
    case 0x5F:
      std::cout << "MOV E, a\n";
      mov(&registers_.reg_e, registers_.reg_a);
      break;
    case 0x60:
      std::cout << "MOV H, b\n";
      mov(&registers_.reg_h, registers_.reg_b);
      break;
    case 0x61:
      std::cout << "MOV H, C\n";
      mov(&registers_.reg_h, registers_.reg_c);
      break;
    case 0x62:
      std::cout << "MOV H, d\n";
      mov(&registers_.reg_h, registers_.reg_d);
      break;
    case 0x63:
      std::cout << "MOV H, e\n";
      mov(&registers_.reg_h, registers_.reg_e);
      break;
    case 0x64:
      std::cout << "MOV H, h\n";
      mov(&registers_.reg_h, registers_.reg_h);
      break;
    case 0x65:
      std::cout << "MOV H, l\n";
      mov(&registers_.reg_h, registers_.reg_l);
      break;
    case 0x66:
      std::cout << "MOV H, M\n";
      mov(&registers_.reg_h, mem_access_->read(registers_.hl()));
      break;
    case 0x67:
      std::cout << "MOV H, A\n";
      mov(&registers_.reg_h, registers_.reg_a);
      break;
    case 0x68:
      std::cout << "MOV L, B\n";
      mov(&registers_.reg_l, registers_.reg_b);
      break;
    case 0x69:
      std::cout << "MOV L, C\n";
      mov(&registers_.reg_l, registers_.reg_c);
      break;
    case 0x6A:
      std::cout << "MOV L, D\n";
      mov(&registers_.reg_l, registers_.reg_d);
      break;
    case 0x6B:
      std::cout << "MOV L, E\n";
      mov(&registers_.reg_l, registers_.reg_e);
      break;
    case 0x6C:
      std::cout << "MOV L, H\n";
      mov(&registers_.reg_l, registers_.reg_h);
      break;
    case 0x6D:
      std::cout << "MOV L, L\n";
      mov(&registers_.reg_l, registers_.reg_l);
      break;
    case 0x6E:
      std::cout << "MOV L, M\n";
      mov(&registers_.reg_l, mem_access_->read(registers_.hl()));
      break;
    case 0x6F:
      std::cout << "MOV L, a\n";
      mov(&registers_.reg_l, registers_.reg_a);
      break;
    case 0x70:
      std::cout << "MOV M, B\n";
      mem_access_->write(registers_.hl(), registers_.reg_b);
      break;
    case 0x71:
      std::cout << "MOV M, C\n";
      mem_access_->write(registers_.hl(), registers_.reg_c);
      break;
    case 0x72:
      std::cout << "MOV M, d\n";
      mem_access_->write(registers_.hl(), registers_.reg_d);
      break;
    case 0x73:
      std::cout << "MOV M, e\n";
      mem_access_->write(registers_.hl(), registers_.reg_e);
      break;
    case 0x74:
      std::cout << "MOV M, h\n";
      mem_access_->write(registers_.hl(), registers_.reg_h);
      break;
    case 0x75:
      std::cout << "MOV M, l\n";
      mem_access_->write(registers_.hl(), registers_.reg_l);
      break;
    case 0x76:
      std::cout << "HLT\n";
      break;
    case 0x77:
      std::cout << "MOV M, a\n";
      mem_access_->write(registers_.hl(), registers_.reg_a);
      break;
    case 0x78:
      std::cout << "MOV A, b\n";
      mov(&registers_.reg_a, registers_.reg_b);
      break;
    case 0x79:
      std::cout << "MOV A, c\n";
      mov(&registers_.reg_a, registers_.reg_c);
      break;
    case 0x7A:
      std::cout << "MOV A, d\n";
      mov(&registers_.reg_a, registers_.reg_d);
      break;
    case 0x7B:
      std::cout << "MOV A, e\n";
      mov(&registers_.reg_a, registers_.reg_e);
      break;
    case 0x7C:
      std::cout << "MOV A, h\n";
      mov(&registers_.reg_a, registers_.reg_h);
      break;
    case 0x7D:
      std::cout << "MOV A, l\n";
      mov(&registers_.reg_a, registers_.reg_l);
      break;
    case 0x7E:
      std::cout << "MOV A, M\n";
      mov(&registers_.reg_a, mem_access_->read(registers_.hl()));
      break;
    case 0x7F:
      std::cout << "MOV A, A\n";
      mov(&registers_.reg_a, registers_.reg_a);
      break;
    case 0x80:
      std::cout << "ADD B\n";
      add(registers_.reg_b);
      break;
    case 0x81:
      std::cout << "ADD C\n";
      add(registers_.reg_c);
      break;
    case 0x82:
      std::cout << "ADD d\n";
      add(registers_.reg_d);
      break;
    case 0x83:
      std::cout << "ADD e\n";
      add(registers_.reg_e);
      break;
    case 0x84:
      std::cout << "ADD h\n";
      add(registers_.reg_h);
      break;
    case 0x85:
      std::cout << "ADD l\n";
      add(registers_.reg_l);
      break;
    case 0x86:
      std::cout << "ADD M\n";
      add(mem_access_->read(registers_.hl()));
      break;
    case 0x87:
      std::cout << "ADD A\n";
      add(registers_.reg_a);
      break;
    case 0x88:
      std::cout << "ADC b\n";
      adc(registers_.reg_b);
      break;
    case 0x89:
      std::cout << "ADC c\n";
      adc(registers_.reg_c);
      break;
    case 0x8A:
      std::cout << "ADC d\n";
      adc(registers_.reg_d);
      break;
    case 0x8B:
      std::cout << "ADC e\n";
      adc(registers_.reg_e);
      break;
    case 0x8C:
      std::cout << "ADC h\n";
      adc(registers_.reg_h);
      break;
    case 0x8D:
      std::cout << "ADC l\n";
      adc(registers_.reg_l);
      break;
    case 0x8E:
      std::cout << "ADC m\n";
      adc(mem_access_->read(registers_.hl()));
      break;
    case 0x8F:
      std::cout << "ADC a\n";
      adc(registers_.reg_a);
      break;
    case 0x90:
      std::cout << "SUB B\n";
      sub(registers_.reg_b);
      break;
    case 0x91:
      std::cout << "SUB C\n";
      sub(registers_.reg_c);
      break;
    case 0x92:
      std::cout << "SUB d\n";
      sub(registers_.reg_d);
      break;
    case 0x93:
      std::cout << "SUB e\n";
      sub(registers_.reg_e);
      break;
    case 0x94:
      std::cout << "SUB h\n";
      sub(registers_.reg_h);
      break;
    case 0x95:
      std::cout << "SUB l\n";
      sub(registers_.reg_l);
      break;
    case 0x96:
      std::cout << "SUB M\n";
      sub(mem_access_->read(registers_.hl()));
      break;
    case 0x97:
      std::cout << "SUB A\n";
      sub(registers_.reg_a);
      break;
    case 0x98:
      std::cout << "SUB B\n";
      sbb(registers_.reg_b);
      break;
    case 0x99:
      std::cout << "SUB C\n";
      sbb(registers_.reg_c);
      break;
    case 0x9A:
      std::cout << "SUB D\n";
      sbb(registers_.reg_d);
      break;
    case 0x9B:
      std::cout << "SUB E\n";
      sbb(registers_.reg_e);
      break;
    case 0x9C:
      std::cout << "SUB H\n";
      sbb(registers_.reg_h);
      break;
    case 0x9D:
      std::cout << "SUB L\n";
      sbb(registers_.reg_l);
      break;
    case 0x9E:
      std::cout << "SUB M\n";
      sbb(mem_access_->read(registers_.hl()));
      break;
    case 0x9F:
      std::cout << "SUB A\n";
      sbb(registers_.reg_a);
      break;
    case 0xA0:
      std::cout << "ANA B\n";
      ana(registers_.reg_b);
      break;
    case 0xA1:
      std::cout << "ANA c\n";
      ana(registers_.reg_c);
      break;
    case 0xA2:
      std::cout << "ANA d\n";
      ana(registers_.reg_d);
      break;
    case 0xA3:
      std::cout << "ANA e\n";
      ana(registers_.reg_e);
      break;
    case 0xA4:
      std::cout << "ANA h\n";
      ana(registers_.reg_h);
      break;
    case 0xA5:
      std::cout << "ANA l\n";
      ana(registers_.reg_l);
      break;
    case 0xA6:
      std::cout << "ANA M\n";
      ana(mem_access_->read(registers_.hl()));
      break;
    case 0xA7:
      std::cout << "ANA a\n";
      ana(registers_.reg_a);
      break;
    case 0xA8:
      std::cout << "XRA B\n";
      xra(registers_.reg_b);
      break;
    case 0xA9:
      std::cout << "XRA C\n";
      xra(registers_.reg_c);
      break;
    case 0xAA:
      std::cout << "XRA D\n";
      xra(registers_.reg_d);
      break;
    case 0xAB:
      std::cout << "XRA E\n";
      xra(registers_.reg_e);
      break;
    case 0xAC:
      std::cout << "XRA H\n";
      xra(registers_.reg_h);
      break;
    case 0xAD:
      std::cout << "XRA L\n";
      xra(registers_.reg_l);
      break;
    case 0xAE:
      std::cout << "XRA M\n";
      xra(mem_access_->read(registers_.hl()));
      break;
    case 0xAF:
      std::cout << "XRA A\n";
      xra(registers_.reg_a);
      break;
    case 0xB0:
      std::cout << "ORA B\n";
      ora(registers_.reg_b);
      break;
    case 0xB1:
      std::cout << "ORA C\n";
      ora(registers_.reg_c);
      break;
    case 0xB2:
      std::cout << "ORA D\n";
      ora(registers_.reg_d);
      break;
    case 0xB3:
      std::cout << "ORA E\n";
      ora(registers_.reg_e);
      break;
    case 0xB4:
      std::cout << "ORA H\n";
      ora(registers_.reg_h);
      break;
    case 0xB5:
      std::cout << "ORA L\n";
      ora(registers_.reg_l);
      break;
    case 0xB6: {
      std::cout << "ORA M\n";
      uint8_t data = mem_access_->read(registers_.hl());
      ora(data);
      break;
    }
    case 0xB7:
      std::cout << "ORA A\n";
      ora(registers_.reg_a);
      break;
    case 0xB8:
      std::cout << "CMP B\n";
      cmp(registers_.reg_b);
      break;
    case 0xB9:
      std::cout << "CMP c\n";
      cmp(registers_.reg_c);
      break;
    case 0xBA:
      std::cout << "CMP d\n";
      cmp(registers_.reg_d);
      break;
    case 0xBB:
      std::cout << "CMP e\n";
      cmp(registers_.reg_e);
      break;
    case 0xBC:
      std::cout << "CMP h\n";
      cmp(registers_.reg_h);
      break;
    case 0xBD:
      std::cout << "CMP l\n";
      cmp(registers_.reg_l);
      break;
    case 0xBE: {
      std::cout << "CMP M\n";
      uint8_t data = mem_access_->read(registers_.hl());
      cmp(data);
      break;
    }
    case 0xBF:
      std::cout << "CMP A\n";
      cmp(registers_.reg_a);
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
      std::cout << "JNZ\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        jmp(JumpCondition::kNotZero, b1, b2);
      }
      break;
    case 0xC3:
      std::cout << "JMP ";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        print_hex_byte(b2);
        print_hex_byte(b1);
        std::cout << "\n";
        jmp(JumpCondition::kTrue, b1, b2);
        break;
      }
    case 0xC4:
      std::cout << "CNZ\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        call(JumpCondition::kNotZero, b1, b2);
      }
      break;
    case 0xC5:
      std::cout << "PUSH B\n";
      push(registers_.reg_b, registers_.reg_c);
      break;
    case 0xC6:
      std::cout << "ADI D8\n";
      {
        uint8_t b1 = fetch_byte();
        add(b1);
        break;
      }
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
      std::cout << "JZ\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        jmp(JumpCondition::kZero, b1, b2);
        break;
      }
    case 0xCB:
      std::cout << "JMP\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        jmp(JumpCondition::kTrue, b1, b2);
        break;
      }
    case 0xCC:
      std::cout << "CZ\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        call(JumpCondition::kZero, b1, b2);
      }
      break;
    case 0xCD: {
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      std::cout << "CALL \n";
      print_hex_byte(b2);
      print_hex_byte(b1);
      std::cout << "\n";
      call(JumpCondition::kTrue, b1, b2);
      break;
    }
    case 0xCE:
      std::cout << "ACI, D8\n";
      ana(fetch_byte());
      break;
    case 0xCF:
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xD0:
      std::cout << "RST 1\n";
      ret(JumpCondition::kNotCarry);
      break;
    case 0xD1:
      std::cout << "POP D\n";
      pop(&registers_.reg_d, &registers_.reg_e);
      break;
    case 0xD2:
      std::cout << "JNC\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        jmp(JumpCondition::kNotCarry, b1, b2);
        break;
      }
    case 0xD3:
      std::cout << "OUT d8\n";
      out(fetch_byte());
      break;
    case 0xD4:
      std::cout << "CNC\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        call(JumpCondition::kNotCarry, b1, b2);
        break;
      }
    case 0xD5:
      std::cout << "PUSH D\n";
      push(registers_.reg_d, registers_.reg_e);
      break;
    case 0xD6:
      std::cout << "SUI d8\n";
      sub(fetch_byte());
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
      std::cout << "RET\n";
      ret(JumpCondition::kTrue);
      break;
    case 0xDA:
      std::cout << "JC\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        jmp(JumpCondition::kCarry, b1, b2);
        break;
      }
    case 0xDB:  // IN instruction + D8 (input port number)
      std::cout << "IN d8\n";
      in(fetch_byte());
      break;
    case 0xDC:
      std::cout << "CC\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        call(JumpCondition::kCarry, b1, b2);
      }
      break;
    case 0xDD:
      std::cout << "*CALL A16\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        call(JumpCondition::kTrue, b1, b2);
        break;
      }
    case 0xDE:
      std::cout << "SBI d8\n";
      sbb(fetch_byte());
      break;
    case 0xDF:
      std::cout << "RST 3\n";
      rst((opcode >> 3) & 0b0000'0111);
      break;
    case 0xE0:
      std::cout << "RPO\n";
      ret(JumpCondition::kParityOdd);
      break;
    case 0xE1:
      std::cout << "POP H\n";
      pop(&registers_.reg_h, &registers_.reg_l);
      break;
    case 0xE2:
      std::cout << "JPO\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        jmp(JumpCondition::kParityOdd, b1, b2);
        break;
      }
    case 0xE3:
      std::cout << "XTHL\n";
      xthl();
      break;
    case 0xE4:
      std::cout << "CPO\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        call(JumpCondition::kParityOdd, b1, b2);
      }
      break;
    case 0xE5:
      std::cout << "PUSH H\n";
      push(registers_.reg_h, registers_.reg_l);
      break;
    case 0xE6:
      std::cout << "ANI D8\n";
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
      std::cout << "JPE\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        jmp(JumpCondition::kParityEven, b1, b2);
      }
      break;
    case 0xEB:
      std::cout << "XCHG\n";
      xchg();
      break;
    case 0xEC:
      std::cout << "CPE\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        call(JumpCondition::kParityEven, b1, b2);
        break;
      }
    case 0xED: {
      std::cout << "*CALL\n";
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kTrue, b1, b2);
      break;
    }
    case 0xEE: {
      std::cout << "XRA\n";
      uint8_t b1 = fetch_byte();
      xra(b1);
      break;
    }
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
      std::cout << "JP\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        jmp(JumpCondition::kPositive, b1, b2);
      }
      break;
    case 0xF3:
      std::cout << "DI\n";
      di();
      break;
    case 0xF4:
      std::cout << "CP\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        call(JumpCondition::kPositive, b1, b2);
      }
      break;
    case 0xF5:
      std::cout << "PUSH PSW\n";
      push(registers_.reg_a, flags_.to_byte());
      break;
    case 0xF6:
      std::cout << "ORA\n";
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
      std::cout << "JM\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        jmp(JumpCondition::kMinus, b1, b2);
      }
      break;
    case 0xFB:
      std::cout << "EI\n";
      ei();
      break;
    case 0xFC:
      std::cout << "CM\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        call(JumpCondition::kMinus, b1, b2);
      }
      break;
    case 0xFD:
      std::cout << "*CALL\n";
      {
        uint8_t b1 = fetch_byte();
        uint8_t b2 = fetch_byte();
        call(JumpCondition::kTrue, b1, b2);
      }
      break;
    case 0xFE:
      std::cout << "CPI\n";
      cpi(fetch_byte());
      break;
    case 0xFF:
      std::cout << "RST 7\n";
      rst((opcode >> 3) & 0b0000'0111);
      break;
    default:
      std::cerr << "CPU8080:execute() : default case for opcode switch\n";
      break;
  }
}
}  // namespace intel_8080
