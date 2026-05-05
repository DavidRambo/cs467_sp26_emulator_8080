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

    std::uint8_t psw();
  };

 public:
  CPU8080(std::shared_ptr<intel_8080::Memory8080> new_mem);

  void step();

  void reset();

 private:
  std::uint8_t fetch_byte();

  std::uint8_t fetch_byte(std::uint16_t mem_location);

  std::uint16_t fetch_word();

  std::uint16_t fetch_word(std::uint16_t mem_location);

  void execute(std::uint8_t opcode);

  std::array<std::uint8_t, 8> registers_;
  std::uint16_t stack_pointer_;
  std::uint16_t program_counter_;
  Flags flags_;
  // false = 0 and true = 1
  bool INTE_;
  std::shared_ptr<intel_8080::Memory8080> mem_access_;
};
}  // namespace intel_8080
