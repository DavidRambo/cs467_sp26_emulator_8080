#include "CPU8080.h"

#include <memory>

#include "Memory8080.h"

intel_8080::CPU8080::CPU8080(std::shared_ptr<intel_8080::Memory8080> new_mem)
    : mem_access_(std::move(new_mem)) {
  // Registers: B=0, C=1, D=2, E=3, H=4, L=5 Memory=6, A=7
  registers_.fill(0x00);
  stack_pointer_ = 0x0000;
  program_counter_ = 0x0000;
  flags_ = 0x02;  // 00000010
  INTE_ = false;
  mem_access_ = new_mem;
};

void intel_8080::CPU8080::step() {
  std::uint8_t instruction = fetch_byte();
  execute(instruction);
};

std::uint8_t intel_8080::CPU8080::fetch_byte() {
  std::uint8_t byte = mem_access_->Read(program_counter_);
  program_counter_++;
  return byte;
};

std::uint8_t intel_8080::CPU8080::fetch_byte(std::uint16_t mem_location) {
  std::uint8_t byte = mem_access_->Read(mem_location);
  return byte;
};

std::uint16_t intel_8080::CPU8080::fetch_word() {
  std::uint8_t low_byte = fetch_byte();
  std::uint8_t high_byte = fetch_byte();
  std::uint16_t word = ((std::uint16_t)high_byte << 8) | low_byte;
  return word;
}

std::uint16_t intel_8080::CPU8080::fetch_word(std::uint16_t mem_location) {
  std::uint8_t low_byte = mem_access_->Read(mem_location);
  std::uint8_t high_byte = mem_access_->Read(mem_location + 1);
  std::uint16_t word = ((std::uint16_t)high_byte << 8) | low_byte;
  return word;
};

void intel_8080::CPU8080::reset() { program_counter_ = 0x0000; };

void intel_8080::CPU8080::execute(std::uint8_t opcode) {
  /*
   Will utilize an arrray with indicees 0 to 256 for all opcodes found in
   the intel 8080. Will use a struct containg data for the specific instruction,
   source and/or destination, pointer to generic methods such as MOV, ADD, etc.
   */
}
