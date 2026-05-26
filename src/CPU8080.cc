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
      input_handler_(std::move(input_handler_ptr)),
      mixer_(std::move(new_mixer)),
      shift_register_(std::move(shift_reg_ptr)) {
  stack_pointer_ = 0x0000;
  program_counter_ = 0x0000;
  flags_ = Flags();
  registers_ = Registers();
  INTE_ = false;
  halted_ = false;
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
  if (!interrupt_queue_.empty() && INTE_) {
    di();  // Disable interrupts. The subroutine will re-enable.
    instruction = interrupt_queue_.front();
    interrupt_queue_.pop();
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

bool CPU8080::is_not_stopped() const { return !halted_; }

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

// Updates the auxiliary carry bit based on the addition of two four-bit
// numbers.
void CPU8080::update_aux_carry_add(uint8_t x, uint8_t y, bool with_carry) {
  uint8_t carry = with_carry ? flags_.carry : 0;
  flags_.aux_carry = ((x & 0xF) + (y & 0xF) + carry) > 0xF;
}

// Updates the auxiliary carry bit based on the subtraction of two four-bit
// numbers. If a carry out from bit 3 occurs when subtracting the least
// significant four bits of y from those of x, then the aux carry flag should be
// set.
void CPU8080::update_aux_carry_sub(uint8_t x, uint8_t y, bool with_carry) {
  uint8_t carry = with_carry ? flags_.carry : 0;
  flags_.aux_carry = (x & 0x0F) < ((y & 0x0F) + carry);
}

void CPU8080::queue_interrupt(uint8_t opcode) {
  interrupt_queue_.push(opcode);

  // CPU enters a STOPPED state to await an interrupt.
  // Main loop checks for halted_ while the CPU is stepping.
  halted_ = false;
}

void CPU8080::execute(uint8_t opcode) {
#ifdef DEBUG_STATE
  char ch{0};
  std::cout << "Next instruction (with above cpu state): ";
#endif
#ifdef DEBUG
  print_instruction(opcode);
#endif
#ifdef DEBUG_STATE
  std::cout << "Execution paused, press Enter to continue...\n";
  std::cin.get(ch);
#endif

  switch (opcode) {
    case 0x00:  // NOP
      break;
    case 0x01: {  // LXI B,d16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      lxi(&registers_.reg_b, &registers_.reg_c, b1, b2);
    } break;
    case 0x02:  // STAX B
      stax(registers_.bc());
      break;
    case 0x03:  // INX B
      inx(&registers_.reg_b, &registers_.reg_c);
      break;
    case 0x04:  // INR B: B += 1 flags
      inr(&registers_.reg_b);
      break;
    case 0x05:  // DCR B
      dcr(&registers_.reg_b);
      break;
    case 0x06: {  // MVI B,d8
      uint8_t b1 = fetch_byte();
      mov(&registers_.reg_b, b1);
    } break;

    case 0x07:  // RLC
      rlc();
      break;
    case 0x08:  // *NOP
      break;
    case 0x09:  // DAD B
      dad(&registers_.reg_b, &registers_.reg_c);
      break;
    case 0x0A:  // LDAX B
      ldax(registers_.bc());
      break;
    case 0x0B:  // DCX B
      dcx(&registers_.reg_b, &registers_.reg_c);
      break;
    case 0x0C:  // INR C
      inr(&registers_.reg_c);
      break;
    case 0x0D:  // DCR C
      dcr(&registers_.reg_c);
      break;
    case 0x0E: {  // MVI C,d8
      uint8_t b1 = fetch_byte();
      mov(&registers_.reg_c, b1);
    } break;
    case 0x0F:  // RRC
      rrc();
      break;
    case 0x10:  // *NOP
      break;
    case 0x11: {  // LXI D,d16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      lxi(&registers_.reg_d, &registers_.reg_e, b1, b2);
    } break;
    case 0x12:  // STAX D
      stax(registers_.de());
      break;
    case 0x13:  // INX D
      inx(&registers_.reg_d, &registers_.reg_e);
      break;
    case 0x14:  // INR D
      inr(&registers_.reg_d);
      break;
    case 0x15:  // DCR D
      dcr(&registers_.reg_d);
      break;
    case 0x16: {  // MVI D,d8
      uint8_t b1 = fetch_byte();
      mov(&registers_.reg_d, b1);
    } break;
    case 0x17:  // RAL
      ral();
      break;
    case 0x18:  // *NOP
      break;
    case 0x19:  // DAD D
      dad(&registers_.reg_d, &registers_.reg_e);
      break;
    case 0x1A:  // LDAX D
      ldax(registers_.de());
      break;
    case 0x1B:  // DCX D
      dcx(&registers_.reg_d, &registers_.reg_e);
      break;
    case 0x1C:  // INR E
      inr(&registers_.reg_e);
      break;
    case 0x1D:  // DCR E
      dcr(&registers_.reg_e);
      break;
    case 0x1E: {  // MVI E,d8
      uint8_t b1 = fetch_byte();
      mov(&registers_.reg_e, b1);
    } break;
    case 0x1F:  // RAR
      rar();
      break;
    case 0x20:  // *NOP
      break;
    case 0x21: {  // LXI H,d16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      lxi(&registers_.reg_h, &registers_.reg_l, b1, b2);
    } break;
    case 0x22: {  // SHLD a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      shld(b1, b2);
    } break;
    case 0x23:  // INX H
      inx(&registers_.reg_h, &registers_.reg_l);
      break;
    case 0x24:  // INR H
      inr(&registers_.reg_h);
      break;
    case 0x25:  // DCR H
      dcr(&registers_.reg_h);
      break;
    case 0x26: {  // MVI H,d8
      uint8_t b1 = fetch_byte();
      mov(&registers_.reg_h, b1);
    } break;
    case 0x27:  // DAA
      daa();
      break;
    case 0x28:  // *NOP
      break;
    case 0x29:  // DAD H
      dad(&registers_.reg_h, &registers_.reg_l);
      break;
    case 0x2A: {  // LHLD a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      lhld(b1, b2);
    } break;
    case 0x2B:  // DCX H
      dcx(&registers_.reg_h, &registers_.reg_l);
      break;
    case 0x2C:  // INR L
      inr(&registers_.reg_l);
      break;
    case 0x2D:  // DCR L
      dcr(&registers_.reg_l);
      break;
    case 0x2E: {  // MVI L,d8
      uint8_t b1 = fetch_byte();
      mov(&registers_.reg_l, b1);
    } break;
    case 0x2F:  // CMA
      cma();
      break;
    case 0x30:  // *NOP
      break;
    case 0x31: {  // LXI SP,d16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      lxi_sp(b1, b2);
      break;
    }
    case 0x32: {  // STA a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      sta(b1, b2);
    } break;
    case 0x33:              // INX SP
      stack_pointer_ += 1;  // inx sp
      break;
    case 0x34: {  // INR M
      auto mem_location = static_cast<uint16_t>(registers_.hl());
      uint8_t data = mem_access_->read(mem_location);
      inr(&data);
      mem_access_->write(mem_location, data);
    } break;
    case 0x35: {  // DCR M
      auto mem_location = static_cast<uint16_t>(registers_.hl());
      uint8_t data = mem_access_->read(registers_.hl());
      dcr(&data);
      mem_access_->write(mem_location, data);
    } break;
    case 0x36: {  // MVI M,d8
      uint8_t b1 = fetch_byte();
      mem_access_->write(registers_.hl(), b1);
      break;
    }
    case 0x37:  // STC
      stc();
      break;
    case 0x38:  // *NOP
      break;
    case 0x39: {  // DAD SP
      uint8_t sp_high = stack_pointer_ >> 8;
      uint8_t sp_low = stack_pointer_ & 0xFF;
      dad(&sp_high, &sp_low);
      stack_pointer_ = static_cast<uint16_t>((sp_high << 8) | sp_low);
    } break;
    case 0x3A: {  // LDA a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      lda(b1, b2);
    } break;
    case 0x3B: {  // DCX SP
      uint8_t sp_high = stack_pointer_ >> 8;
      uint8_t sp_low = stack_pointer_ & 0xFF;
      dcx(&sp_high, &sp_low);
      stack_pointer_ = static_cast<uint16_t>((sp_high << 8) | sp_low);
    } break;
    case 0x3C:  // INR A
      inr(&registers_.reg_a);
      break;
    case 0x3D:  // DCR A
      dcr(&registers_.reg_a);
      break;
    case 0x3E:  // MVI A, D8
    {
      uint8_t b1 = fetch_byte();
      mov(&registers_.reg_a, b1);
      break;
    }
    case 0x3F:  // CMC
      cmc();
      break;
    case 0x40:  // MOV B,B
      mov(&registers_.reg_b, registers_.reg_b);
      break;
    case 0x41:  // MOV B,C
      mov(&registers_.reg_b, registers_.reg_c);
      break;
    case 0x42:  // MOV B,D
      mov(&registers_.reg_b, registers_.reg_d);
      break;
    case 0x43:  // MOV B,E
      mov(&registers_.reg_b, registers_.reg_e);
      break;
    case 0x44:  // MOV B,H
      mov(&registers_.reg_b, registers_.reg_h);
      break;
    case 0x45:  // MOV B,L
      mov(&registers_.reg_b, registers_.reg_l);
      break;
    case 0x46:  // MOV B,M
      mov(&registers_.reg_b, mem_access_->read(registers_.hl()));
      break;
    case 0x47:  // MOV B,A
      mov(&registers_.reg_b, registers_.reg_a);
      break;
    case 0x48:  // MOV C,B
      mov(&registers_.reg_c, registers_.reg_b);
      break;
    case 0x49:  // MOV C,C
      mov(&registers_.reg_c, registers_.reg_c);
      break;
    case 0x4A:  // MOV C,D
      mov(&registers_.reg_c, registers_.reg_d);
      break;
    case 0x4B:  // MOV C,E
      mov(&registers_.reg_c, registers_.reg_e);
      break;
    case 0x4C:  // MOV C,H
      mov(&registers_.reg_c, registers_.reg_h);
      break;
    case 0x4D:  // MOV C,L
      mov(&registers_.reg_c, registers_.reg_l);
      break;
    case 0x4E:  // MOV C,M
      mov(&registers_.reg_c, mem_access_->read(registers_.hl()));
      break;
    case 0x4F:  // MOV C,A
      mov(&registers_.reg_c, registers_.reg_a);
      break;
    case 0x50:  // MOV D,B
      mov(&registers_.reg_d, registers_.reg_b);
      break;
    case 0x51:  // MOV D,C
      mov(&registers_.reg_d, registers_.reg_c);
      break;
    case 0x52:  // MOV D,D
      mov(&registers_.reg_d, registers_.reg_d);
      break;
    case 0x53:  // MOV D,E
      mov(&registers_.reg_d, registers_.reg_e);
      break;
    case 0x54:  // MOV D,H
      mov(&registers_.reg_d, registers_.reg_h);
      break;
    case 0x55:  // MOV D,L
      mov(&registers_.reg_d, registers_.reg_l);
      break;
    case 0x56:  // MOV D,M
      mov(&registers_.reg_d, mem_access_->read(registers_.hl()));
      break;
    case 0x57:  // MOV D,A
      mov(&registers_.reg_d, registers_.reg_a);
      break;
    case 0x58:  // MOV E,B
      mov(&registers_.reg_e, registers_.reg_b);
      break;
    case 0x59:  // MOV E,C
      mov(&registers_.reg_e, registers_.reg_c);
      break;
    case 0x5A:  // MOV E,D
      mov(&registers_.reg_e, registers_.reg_d);
      break;
    case 0x5B:  // MOV E,E
      mov(&registers_.reg_e, registers_.reg_e);
      break;
    case 0x5C:  // MOV E,H
      mov(&registers_.reg_e, registers_.reg_h);
      break;
    case 0x5D:  // MOV E,L
      mov(&registers_.reg_e, registers_.reg_l);
      break;
    case 0x5E:  // MOV E,M
      mov(&registers_.reg_e, mem_access_->read(registers_.hl()));
      break;
    case 0x5F:  // MOV E,A
      mov(&registers_.reg_e, registers_.reg_a);
      break;
    case 0x60:  // MOV H,B
      mov(&registers_.reg_h, registers_.reg_b);
      break;
    case 0x61:  // MOV H,C
      mov(&registers_.reg_h, registers_.reg_c);
      break;
    case 0x62:  // MOV H,D
      mov(&registers_.reg_h, registers_.reg_d);
      break;
    case 0x63:  // MOV H,E
      mov(&registers_.reg_h, registers_.reg_e);
      break;
    case 0x64:  // MOV H,H
      mov(&registers_.reg_h, registers_.reg_h);
      break;
    case 0x65:  // MOV H,L
      mov(&registers_.reg_h, registers_.reg_l);
      break;
    case 0x66:  // MOV H,M
      mov(&registers_.reg_h, mem_access_->read(registers_.hl()));
      break;
    case 0x67:  // MOV H,A
      mov(&registers_.reg_h, registers_.reg_a);
      break;
    case 0x68:  // MOV L,B
      mov(&registers_.reg_l, registers_.reg_b);
      break;
    case 0x69:  // MOV L,C
      mov(&registers_.reg_l, registers_.reg_c);
      break;
    case 0x6A:  // MOV L,D
      mov(&registers_.reg_l, registers_.reg_d);
      break;
    case 0x6B:  // MOV L,E
      mov(&registers_.reg_l, registers_.reg_e);
      break;
    case 0x6C:  // MOV L,H
      mov(&registers_.reg_l, registers_.reg_h);
      break;
    case 0x6D:  // MOV H,H
      mov(&registers_.reg_l, registers_.reg_l);
      break;
    case 0x6E:  // MOV H,M
      mov(&registers_.reg_l, mem_access_->read(registers_.hl()));
      break;
    case 0x6F:  // MOV H,A
      mov(&registers_.reg_l, registers_.reg_a);
      break;
    case 0x70:  // MOV M,B
      mem_access_->write(registers_.hl(), registers_.reg_b);
      break;
    case 0x71:  // MOV M,C
      mem_access_->write(registers_.hl(), registers_.reg_c);
      break;
    case 0x72:  // MOV M,D
      mem_access_->write(registers_.hl(), registers_.reg_d);
      break;
    case 0x73:  // MOV M,E
      mem_access_->write(registers_.hl(), registers_.reg_e);
      break;
    case 0x74:  // MOV M,H
      mem_access_->write(registers_.hl(), registers_.reg_h);
      break;
    case 0x75:  // MOV M,L
      mem_access_->write(registers_.hl(), registers_.reg_l);
      break;
    case 0x76:  // HLT (Not a needed opcode for space invaders)
      break;
    case 0x77:  // MOV M,A
      mem_access_->write(registers_.hl(), registers_.reg_a);
      break;
    case 0x78:  // MOV A,B
      mov(&registers_.reg_a, registers_.reg_b);
      break;
    case 0x79:  // MOV A,C
      mov(&registers_.reg_a, registers_.reg_c);
      break;
    case 0x7A:  // MOV A,D
      mov(&registers_.reg_a, registers_.reg_d);
      break;
    case 0x7B:  // MOV A,E
      mov(&registers_.reg_a, registers_.reg_e);
      break;
    case 0x7C:  // MOV A,H
      mov(&registers_.reg_a, registers_.reg_h);
      break;
    case 0x7D:  // MOV A,L
      mov(&registers_.reg_a, registers_.reg_l);
      break;
    case 0x7E:  // MOV A,M
      mov(&registers_.reg_a, mem_access_->read(registers_.hl()));
      break;
    case 0x7F:  // MOV A,A
      mov(&registers_.reg_a, registers_.reg_a);
      break;
    case 0x80:  // ADD B
      add(registers_.reg_b);
      break;
    case 0x81:  // ADD C
      add(registers_.reg_c);
      break;
    case 0x82:  // ADD D
      add(registers_.reg_d);
      break;
    case 0x83:  // ADD E
      add(registers_.reg_e);
      break;
    case 0x84:  // ADD H
      add(registers_.reg_h);
      break;
    case 0x85:  // ADD L
      add(registers_.reg_l);
      break;
    case 0x86:  // ADD M
      add(mem_access_->read(registers_.hl()));
      break;
    case 0x87:  // ADD A
      add(registers_.reg_a);
      break;
    case 0x88:  // ADC B
      adc(registers_.reg_b);
      break;
    case 0x89:  // ADC C
      adc(registers_.reg_c);
      break;
    case 0x8A:  // ADC D
      adc(registers_.reg_d);
      break;
    case 0x8B:  // ADC E
      adc(registers_.reg_e);
      break;
    case 0x8C:  // ADC H
      adc(registers_.reg_h);
      break;
    case 0x8D:  // ADC L
      adc(registers_.reg_l);
      break;
    case 0x8E:  // ADC M
      adc(mem_access_->read(registers_.hl()));
      break;
    case 0x8F:  // ADC A
      adc(registers_.reg_a);
      break;
    case 0x90:  // SUB B
      sub(registers_.reg_b);
      break;
    case 0x91:  // SUB C
      sub(registers_.reg_c);
      break;
    case 0x92:  // SUB D
      sub(registers_.reg_d);
      break;
    case 0x93:  // SUB E
      sub(registers_.reg_e);
      break;
    case 0x94:  // SUB H
      sub(registers_.reg_h);
      break;
    case 0x95:  // SUB L
      sub(registers_.reg_l);
      break;
    case 0x96:  // SUB M
      sub(mem_access_->read(registers_.hl()));
      break;
    case 0x97:  // SUB A
      sub(registers_.reg_a);
      break;
    case 0x98:  // SBB B
      sbb(registers_.reg_b);
      break;
    case 0x99:  // SBB C
      sbb(registers_.reg_c);
      break;
    case 0x9A:  // SBB D
      sbb(registers_.reg_d);
      break;
    case 0x9B:  // SBB E
      sbb(registers_.reg_e);
      break;
    case 0x9C:  // SBB H
      sbb(registers_.reg_h);
      break;
    case 0x9D:  // SBB L
      sbb(registers_.reg_l);
      break;
    case 0x9E:  // SBB M
      sbb(mem_access_->read(registers_.hl()));
      break;
    case 0x9F:  // SBB A
      sbb(registers_.reg_a);
      break;
    case 0xA0:  // ANA B
      ana(registers_.reg_b);
      break;
    case 0xA1:  // ANA C
      ana(registers_.reg_c);
      break;
    case 0xA2:  // ANA D
      ana(registers_.reg_d);
      break;
    case 0xA3:  // ANA E
      ana(registers_.reg_e);
      break;
    case 0xA4:  // ANA H
      ana(registers_.reg_h);
      break;
    case 0xA5:  // ANA L
      ana(registers_.reg_l);
      break;
    case 0xA6:  // ANA M
      ana(mem_access_->read(registers_.hl()));
      break;
    case 0xA7:  // ANA A
      ana(registers_.reg_a);
      break;
    case 0xA8:  // XRA B
      xra(registers_.reg_b);
      break;
    case 0xA9:  // XRA C
      xra(registers_.reg_c);
      break;
    case 0xAA:  // XRA D
      xra(registers_.reg_d);
      break;
    case 0xAB:  // XRA E
      xra(registers_.reg_e);
      break;
    case 0xAC:  // XRA H
      xra(registers_.reg_h);
      break;
    case 0xAD:  // XRA L
      xra(registers_.reg_l);
      break;
    case 0xAE:  // XRA M
      xra(mem_access_->read(registers_.hl()));
      break;
    case 0xAF:  // XRA A
      xra(registers_.reg_a);
      break;
    case 0xB0:  // ORA B
      ora(registers_.reg_b);
      break;
    case 0xB1:  // ORA C
      ora(registers_.reg_c);
      break;
    case 0xB2:  // ORA D
      ora(registers_.reg_d);
      break;
    case 0xB3:  // ORA E
      ora(registers_.reg_e);
      break;
    case 0xB4:  // ORA H
      ora(registers_.reg_h);
      break;
    case 0xB5:  // ORA L
      ora(registers_.reg_l);
      break;
    case 0xB6: {  // ORA M
      uint8_t data = mem_access_->read(registers_.hl());
      ora(data);
      break;
    }
    case 0xB7:  // ORA A
      ora(registers_.reg_a);
      break;
    case 0xB8:  // CMP B
      cmp(registers_.reg_b);
      break;
    case 0xB9:  // CMP C
      cmp(registers_.reg_c);
      break;
    case 0xBA:  // CMP D
      cmp(registers_.reg_d);
      break;
    case 0xBB:  // CMP E
      cmp(registers_.reg_e);
      break;
    case 0xBC:  // CMP H
      cmp(registers_.reg_h);
      break;
    case 0xBD:  // CMP L
      cmp(registers_.reg_l);
      break;
    case 0xBE: {  // CMP M
      uint8_t data = mem_access_->read(registers_.hl());
      cmp(data);
      break;
    }
    case 0xBF:  // CMP A
      cmp(registers_.reg_a);
      break;
    case 0xC0:  // RNZ
      ret(JumpCondition::kNotZero);
      break;
    case 0xC1:  // POP B
      pop(&registers_.reg_b, &registers_.reg_c);
      break;
    case 0xC2: {  // JNZ a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kNotZero, b1, b2);
    } break;
    case 0xC3: {  // JMP a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kTrue, b1, b2);
      break;
    }
    case 0xC4: {  // CNZ a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kNotZero, b1, b2);
    } break;
    case 0xC5:  // PUSH B
      push(registers_.reg_b, registers_.reg_c);
      break;
    case 0xC6: {  // ADI d8
      uint8_t b1 = fetch_byte();
      add(b1);
      break;
    }
    case 0xC7:                           // RST 0
      rst((opcode >> 3) & 0b0000'0111);  // RST 0
      break;
    case 0xC8:  // RZ
      ret(JumpCondition::kZero);
      break;
    case 0xC9:  // RET
      ret(JumpCondition::kTrue);
      break;
    case 0xCA: {  // JZ a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kZero, b1, b2);
      break;
    }
    case 0xCB: {  // *JMP a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kTrue, b1, b2);
      break;
    }
    case 0xCC: {  // CZ a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kZero, b1, b2);
    } break;
    case 0xCD: {  // CALL a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kTrue, b1, b2);
      break;
    }
    case 0xCE:  // ACI d8
      adc(fetch_byte());
      break;
    case 0xCF:                           // RST 1
      rst((opcode >> 3) & 0b0000'0111);  // RST 1
      break;
    case 0xD0:  // RNC
      ret(JumpCondition::kNotCarry);
      break;
    case 0xD1:  // POP D
      pop(&registers_.reg_d, &registers_.reg_e);
      break;
    case 0xD2: {  // JNC a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kNotCarry, b1, b2);
      break;
    }
    case 0xD3:  // OUT d8
      out(fetch_byte());
      break;
    case 0xD4: {  // CNC a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kNotCarry, b1, b2);
      break;
    }
    case 0xD5:  // PUSH D
      push(registers_.reg_d, registers_.reg_e);
      break;
    case 0xD6:  // SUI d8
      sub(fetch_byte());
      break;
    case 0xD7:                           // RST 2
      rst((opcode >> 3) & 0b0000'0111);  // RST 2
      break;
    case 0xD8:  // RC
      ret(JumpCondition::kCarry);
      break;
    case 0xD9:  // *RET
      ret(JumpCondition::kTrue);
      break;
    case 0xDA: {  // JC a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kCarry, b1, b2);
      break;
    }
    case 0xDB:  // IN instruction + D8 (input port number)
      in(fetch_byte());
      break;
    case 0xDC: {  // CC a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kCarry, b1, b2);
    } break;
    case 0xDD: {  // *CALL a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kTrue, b1, b2);
      break;
    }
    case 0xDE:  // SBI d8
      sbb(fetch_byte());
      break;
    case 0xDF:                           // RST 3
      rst((opcode >> 3) & 0b0000'0111);  // RST 3
      break;
    case 0xE0:  // RPO
      ret(JumpCondition::kParityOdd);
      break;
    case 0xE1:  // POP H
      pop(&registers_.reg_h, &registers_.reg_l);
      break;
    case 0xE2: {  // JPO a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kParityOdd, b1, b2);
      break;
    }
    case 0xE3:  // XTHL
      xthl();
      break;
    case 0xE4: {  // CPO a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kParityOdd, b1, b2);
    } break;
    case 0xE5:  // PUSH H
      push(registers_.reg_h, registers_.reg_l);
      break;
    case 0xE6:  // ANI d8
      ana(fetch_byte());
      break;
    case 0xE7:                           // RST 4
      rst((opcode >> 3) & 0b0000'0111);  // RST 4
      break;
    case 0xE8:  // RPE
      ret(JumpCondition::kParityEven);
      break;
    case 0xE9:  // PCHL
      pchl();
      break;
    case 0xEA: {  // JPE a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kParityEven, b1, b2);
    } break;
    case 0xEB:  // XCHG
      xchg();
      break;
    case 0xEC: {  // CPE a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kParityEven, b1, b2);
      break;
    }
    case 0xED: {  // *CALL a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kTrue, b1, b2);
      break;
    }
    case 0xEE: {  // XRI d8
      uint8_t b1 = fetch_byte();
      xra(b1);
      break;
    }
    case 0xEF:                           // RST 5
      rst((opcode >> 3) & 0b0000'0111);  // RST 5
      break;
    case 0xF0:  // RP
      ret(JumpCondition::kPositive);
      break;
    case 0xF1: {  // POP PSW
      uint8_t temp_flags{0};
      pop(&registers_.reg_a, &temp_flags);
      flags_.from_byte(temp_flags);
      break;
    }
    case 0xF2: {  // JP a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kPositive, b1, b2);
    } break;
    case 0xF3:  // DI
      di();
      break;
    case 0xF4: {  // CP a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kPositive, b1, b2);
    } break;
    case 0xF5:  // PUSH PSW
      push(registers_.reg_a, flags_.to_byte());
      break;
    case 0xF6:  // ORI d8
      ora(fetch_byte());
      break;
    case 0xF7:                           // RST 6
      rst((opcode >> 3) & 0b0000'0111);  // RST 6
      break;
    case 0xF8:  // RM
      ret(JumpCondition::kMinus);
      break;
    case 0xF9:  // SPHL
      sphl();
      break;
    case 0xFA: {  // JM a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      jmp(JumpCondition::kMinus, b1, b2);
    } break;
    case 0xFB:  // EI
      ei();
      break;
    case 0xFC: {  // CM a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kMinus, b1, b2);
    } break;
    case 0xFD: {  // *CALL a16
      uint8_t b1 = fetch_byte();
      uint8_t b2 = fetch_byte();
      call(JumpCondition::kTrue, b1, b2);
    } break;
    case 0xFE:  // CI d8
      cpi(fetch_byte());
      break;
    case 0xFF:                           // RST 7
      rst((opcode >> 3) & 0b0000'0111);  // RST 7
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
