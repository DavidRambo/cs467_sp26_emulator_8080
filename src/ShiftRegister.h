#pragma once

#include <cstdint>

namespace hardware {
class ShiftRegister {
 public:
  // [IN 3] Returned value is fed into IN 3
  uint8_t GetShiftedByte();

  // [OUT 4] Accumulator bits passed in from OUT 4 opcode
  // This loads a 16 bit pixel buffer 8 bits at a time
  uint16_t LoadBuffer(uint8_t accumulator_bits);

  // [OUT 2] Accumulator bits passed in from OUT 2 opcode
  // This sets how many shifts are to be taken
  uint8_t SetOffset(uint8_t accumulator_bits);

 private:
  uint16_t buffer_ = 0;
  uint8_t offset_ = 0;
};
}  // namespace hardware