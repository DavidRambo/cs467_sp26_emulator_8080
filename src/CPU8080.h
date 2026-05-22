#pragma once
#include <array>
#include <cstdint>
#include <memory>

#include "AudioMixer.h"
#include "Input.h"
#include "Memory8080.h"
#include "ShiftRegister.h"

namespace intel_8080 {
enum class JumpCondition {
  kNotZero,
  kZero,
  kNotCarry,
  kCarry,
  kParityOdd,
  kParityEven,
  kPositive,
  kMinus,
  kTrue,
};

class CPU8080 {
  // Define private structs before public for sake of State struct.
 private:
  struct Flags {
    std::uint8_t sign : 1;
    std::uint8_t zero : 1;
    std::uint8_t aux_carry : 1;
    std::uint8_t parity : 1;
    std::uint8_t carry : 1;

    // Converts the struct of condition bits to a byte representation. Combined
    // with the accumulator register, it composes the Program Status Word.
    std::uint8_t to_byte();

    // Sets and resets flags according to the byte of data.
    void from_byte(uint8_t data);
  };

  struct Registers {
    std::uint8_t reg_a = 0x00;
    std::uint8_t reg_b = 0x00;
    std::uint8_t reg_c = 0x00;
    std::uint8_t reg_d = 0x00;
    std::uint8_t reg_e = 0x00;
    std::uint8_t reg_h = 0x00;
    std::uint8_t reg_l = 0x00;

    std::uint16_t hl() { return (std::uint16_t)reg_h << 8 | reg_l; };
    std::uint16_t bc() { return (std::uint16_t)reg_b << 8 | reg_c; };
    std::uint16_t de() { return (std::uint16_t)reg_d << 8 | reg_e; };
  };

 public:
  struct State {
    Registers registers;
    Flags flags;
    uint16_t stack_pointer;
    uint16_t program_counter;
  };

  CPU8080(std::shared_ptr<intel_8080::Memory8080> new_mem,
          std::shared_ptr<input::InputHandler> input_handler_ptr,
          std::shared_ptr<audio::Mixer> new_mixer,
          std::shared_ptr<hardware::ShiftRegister> shift_reg_ptr);

  void step();

  void reset();

  State get_state();

  void print_instruction(uint8_t opcode);

 private:
  std::uint8_t fetch_byte();

  std::uint8_t fetch_byte(std::uint16_t mem_location);

  std::uint16_t fetch_word();

  std::uint16_t fetch_word(std::uint16_t mem_location);

  void execute(std::uint8_t opcode);

  void update_flags_szp(uint8_t byte);

  void update_parity(uint8_t byte);

  void update_parity(uint16_t word);

  // Immediate instructions will utilize their register/mem
  // counterpart except for lxi
  // Instructions
  void in(uint8_t port_no);
  void out(uint8_t port_no);
  void inr(uint8_t* reg);
  static void mov(uint8_t* addr, uint8_t data);
  void stax(uint16_t mem_location);
  void ldax(uint16_t mem_location);
  void rlc();
  void cmc();
  void stc();
  void dcr(uint8_t* reg);
  void cma();
  void nop();
  void add(uint8_t data);
  void adc(uint8_t data);
  void sub(uint8_t data);
  void sbb(uint8_t data);
  void ana(uint8_t data);
  void xra(uint8_t data);
  void ora(uint8_t data);
  void cpi(uint8_t data);
  void cmp(uint8_t data);
  void rrc();
  void ral();
  void rar();
  void push(uint8_t reg_1, uint8_t reg_2);
  void pop(uint8_t* reg_1, uint8_t* reg_2);
  void dad(const uint8_t* reg_1, const uint8_t* reg_2);
  void lxi_sp(uint8_t byte_2, uint8_t byte_3);
  static void lxi(uint8_t* reg_1, uint8_t* reg_2, uint8_t byte_2,
                  uint8_t byte_3);
  static void inx(uint8_t* reg_1, uint8_t* reg_2);
  static void dcx(uint8_t* reg_1, uint8_t* reg_2);
  void xthl();
  void xchg();
  void sphl();
  void sta(uint8_t byte_2, uint8_t byte_3);
  void lda(uint8_t byte_2, uint8_t byte_3);
  void shld(uint8_t byte_2, uint8_t byte_3);
  void lhld(uint8_t byte_2, uint8_t byte_3);
  void pchl();
  void jmp(JumpCondition jump_condition, uint8_t byte_2, uint8_t byte_3);
  void call(JumpCondition jump_condition, uint8_t byte_2, uint8_t byte_3);
  void ret(JumpCondition jump_condition);
  void rst(uint8_t exp);
  void ei();
  void di();
  void hlt();
  bool check_jump_condition(JumpCondition jump_condition) const;

  // State
  Flags flags_;
  Registers registers_;
  std::uint16_t stack_pointer_;
  std::uint16_t program_counter_;
  // false = 0 and true = 1
  bool INTE_;

  // Address Spaces
  std::shared_ptr<intel_8080::Memory8080> mem_access_;
  std::shared_ptr<input::InputHandler> input_handler_;
  std::shared_ptr<audio::Mixer> mixer_;
  std::shared_ptr<hardware::ShiftRegister> shift_register_;
};
}  // namespace intel_8080
