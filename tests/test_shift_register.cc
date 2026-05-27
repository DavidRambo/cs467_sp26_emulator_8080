#include "../src/ShiftRegister.h"
#include "doctest.h"

TEST_CASE("Testing External Shift Register") {
  hardware::ShiftRegister shift_reg = hardware::ShiftRegister();

  SUBCASE("Shift in 0xFF, Offset 0") {
    uint8_t data_in = 0xFF;
    uint8_t offset = 0x00;
    shift_reg.LoadBuffer(data_in);
    shift_reg.SetOffset(offset);
    CHECK_EQ(shift_reg.GetShiftedByte(), 0xFF);
  }

  SUBCASE("Shift in 0xFF, Offset 2") {
    uint8_t data_in = 0xFF;
    uint8_t offset = 0x02;
    shift_reg.LoadBuffer(data_in);
    shift_reg.SetOffset(offset);
    CHECK_EQ(shift_reg.GetShiftedByte(), 0xFC);
  }

  SUBCASE("Shift in 0xFF, Offset 7") {
    uint8_t data_in = 0xFF;
    uint8_t offset = 0x07;
    shift_reg.LoadBuffer(data_in);
    shift_reg.SetOffset(offset);
    CHECK_EQ(shift_reg.GetShiftedByte(), 0x80);
  }
}
