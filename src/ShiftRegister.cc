#include "ShiftRegister.h"

#include <cstdint>

namespace hardware {
uint8_t ShiftRegister::GetShiftedByte() {
  uint16_t shifted_word = buffer_ << offset_;
  return static_cast<uint8_t>((shifted_word >> 8) & 0xFF);
}

uint16_t ShiftRegister::LoadBuffer(uint8_t accumulator_bits) {
  buffer_ = (static_cast<uint16_t>(accumulator_bits) << 8) | (buffer_ >> 8);
  return buffer_;
}

uint8_t ShiftRegister::SetOffset(uint8_t accumulator_bits) {
  offset_ = accumulator_bits;
  return ShiftRegister::GetShiftedByte();
}
}  // namespace hardware