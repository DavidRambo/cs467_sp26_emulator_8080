#pragma once

#include <cstdint>

namespace hardware {
class ShiftRegister {
 public:
  // Returns 8 bits from the buffer_ starting at offset_ bits from the most
  // significant bit.
  //
  // For example: an offset_ of 2 would returns bits 13 through
  // 5 from the buffer_.
  //
  // Triggered by the [IN 3] instruction to place a byte into the data bus,
  // which is then transfered to the accumulator by the 8080.
  uint8_t GetShiftedByte();

  // Right-shifts the buffer_ by 8 bits and then loads accumulator_bits into the
  // buffer_'s 8 most significant bits.
  //
  // Triggered by the [OUT 4] instruction, which loads the accumulator's data
  // to the data bus for the Write 4 device.
  uint16_t LoadBuffer(uint8_t accumulator_bits);

  // Sets the shift register's offset for when data is fetched.
  //
  // Triggered by the [OUT 2] instruction, which loads the accumulator's data
  // to the data bus for the Write 2 device.
  uint8_t SetOffset(uint8_t accumulator_bits);

 private:
  uint16_t buffer_ = 0;
  uint8_t offset_ = 0;
};
}  // namespace hardware
