#pragma once
#include <array>
#include <cstdint>
#include <memory>

#include "Memory8080.h"

namespace intel_8080 {
class CPU8080 {
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
  std::uint8_t flags_;
  // false = 0 and true = 1
  bool INTE_;
  std::shared_ptr<intel_8080::Memory8080> mem_access_;
};
}  // namespace intel_8080
