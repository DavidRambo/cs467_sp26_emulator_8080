#pragma once
#include <array>
#include <cstdint>
#include <memory>

#include "Memory8080.h"

namespace intel_8080 {

class CPU8080 {
  struct Port {
    std::uint8_t bit0 : 1;
    std::uint8_t bit1 : 1;
    std::uint8_t bit2 : 1;
    std::uint8_t bit3 : 1;
    std::uint8_t bit4 : 1;
    std::uint8_t bit5 : 1;
    std::uint8_t bit6 : 1;
    std::uint8_t bit7 : 1;
  };

  struct Flags {
    std::uint8_t sign : 1;
    std::uint8_t zero : 1;
    std::uint8_t aux_carry : 1;
    std::uint8_t parity : 1;
    std::uint8_t carry : 1;

    std::uint8_t to_byte();
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

  CPU8080(std::shared_ptr<intel_8080::Memory8080> new_mem);

  void step();

  void reset();

  State get_state();

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
  void inr(uint8_t* byte);
  void mov(uint8_t* addr, uint8_t data);
  void stax(uint16_t mem_location);
  void ldax(uint16_t mem_location);
  void rlc();
  void cmc();
  void stc();
  void dcr(uint8_t* byte);
  void cma();
  void daa();  // Are we ignoring this one?
  void nop();
  void add(uint8_t data);
  void adc(uint8_t data);
  void sub(uint8_t data);
  void sbb(uint8_t data);
  void ana(uint8_t data);
  void xra(uint8_t data);
  void ora(uint8_t data);
  void cmp(uint8_t data);
  void rrc();
  void ral();
  void rar();
  void push(uint16_t reg_pair);
  uint16_t pop();
  void dad(uint16_t reg_pair);
  void lxi(uint8_t* first_reg, uint8_t* second_reg, uint8_t third_byte,
           uint8_t second_byte);
  void dcx(uint16_t* data);
  void xchg();
  void sphl();
  void lxi(uint16_t data);
  void call(uint16_t mem_location);
  void ret();
  void rst();
  void ei();
  void di();
  void hlt();

  Flags flags_;
  Registers registers_;
  std::uint16_t stack_pointer_;
  std::uint16_t program_counter_;
  // false = 0 and true = 1
  bool INTE_;
  std::shared_ptr<intel_8080::Memory8080> mem_access_;
};
}  // namespace intel_8080
