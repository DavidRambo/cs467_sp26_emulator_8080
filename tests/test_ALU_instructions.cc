#include <memory>

#include "../src/CPU8080.h"
#include "../src/Memory8080.h"
#include "doctest.h"
namespace intel_8080 {
TEST_CASE("Testing ALU Instructions") {
  std::shared_ptr<intel_8080::Memory8080> mem =
      std::make_shared<intel_8080::Memory8080>(intel_8080::Memory8080());
  std::shared_ptr<input::InputHandler> input_handler =
      std::make_shared<input::InputHandler>();
  std::shared_ptr<audio::Mixer> mixer =
      std::make_shared<audio::Mixer>(audio::Mixer());
  std::shared_ptr<hardware::ShiftRegister> shift_reg_ptr =
      std::make_shared<hardware::ShiftRegister>(hardware::ShiftRegister());
  intel_8080::CPU8080 emu =
      intel_8080::CPU8080(mem, input_handler, mixer, shift_reg_ptr);

  SUBCASE("Testing ADD") {
    std::vector<uint8_t> data = {0x3E, 0x6C, 0x16, 0x2E, 0x82};
    mem->load_data(data);
    emu.step();
    emu.step();
    emu.step();

    auto state = emu.get_state();

    CHECK_EQ(state.registers.reg_d, 0x2E);
    CHECK_EQ(state.registers.reg_a, 0x9A);
    CHECK_EQ(state.flags.zero, 0);
    CHECK_EQ(state.flags.carry, 0);
    CHECK_EQ(state.flags.parity, 1);
    CHECK_EQ(state.flags.sign, 1);
  }
  SUBCASE("Testing ADC") {
    std::vector<uint8_t> data = {0x3E, 0x41, 0x0E, 0x3D, 0x37, 0x89};
    mem->load_data(data);
    emu.step();
    emu.step();
    emu.step();
    emu.step();

    auto state = emu.get_state();

    CHECK_EQ(state.registers.reg_a, 0x7F);
    CHECK_EQ(state.registers.reg_c, 0x3D);
    CHECK_EQ(state.flags.zero, 0);
    CHECK_EQ(state.flags.carry, 0);
    CHECK_EQ(state.flags.parity, 0);
    CHECK_EQ(state.flags.sign, 0);
  }
  SUBCASE("Testing SUB") {
    std::vector<uint8_t> data = {0x3E, 0x3E, 0x97};
    mem->load_data(data);

    emu.step();
    emu.step();

    auto state = emu.get_state();

    CHECK_EQ(state.registers.reg_a, 0x00);
    CHECK_EQ(state.flags.zero, 1);
    CHECK_EQ(state.flags.carry, 1);
    CHECK_EQ(state.flags.parity, 1);
    CHECK_EQ(state.flags.sign, 0);
  }
  SUBCASE("Testing SBB") {
    std::vector<uint8_t> data = {0x3E, 0x04, 0x2E, 0x02, 0x37, 0x9D};
    mem->load_data(data);

    emu.step();
    emu.step();
    emu.step();
    emu.step();

    auto state = emu.get_state();

    CHECK_EQ(state.registers.reg_a, 0x01);
    CHECK_EQ(state.flags.zero, 0);
    CHECK_EQ(state.flags.carry, 1);
    CHECK_EQ(state.flags.parity, 0);
    CHECK_EQ(state.flags.sign, 0);
  }
  SUBCASE("Testing ANA") {
    std::vector<uint8_t> data = {0x3E, 0xFC, 0x0E, 0x0F, 0xA1};
    mem->load_data(data);

    emu.step();
    emu.step();
    emu.step();

    auto state = emu.get_state();

    CHECK_EQ(state.registers.reg_a, 0x0C);
    CHECK_EQ(state.flags.zero, 0);
    CHECK_EQ(state.flags.carry, 0);
    CHECK_EQ(state.flags.parity, 1);
    CHECK_EQ(state.flags.sign, 0);
  }
  SUBCASE("Testing XRA") {
    std::vector<uint8_t> data = {0x3E, 0xFF, 0x0E, 0xFF, 0xA9};
    mem->load_data(data);

    emu.step();
    emu.step();
    emu.step();

    auto state = emu.get_state();

    CHECK_EQ(state.registers.reg_a, 0x00);
    CHECK_EQ(state.flags.zero, 1);
    CHECK_EQ(state.flags.carry, 0);
    CHECK_EQ(state.flags.parity, 1);
    CHECK_EQ(state.flags.sign, 0);
  }
  SUBCASE("Testing ORA") {
    std::vector<uint8_t> data = {0x3E, 0x33, 0x0E, 0x0F, 0xB1};
    mem->load_data(data);

    emu.step();
    emu.step();
    emu.step();

    auto state = emu.get_state();

    CHECK_EQ(state.registers.reg_a, 0x3F);
    CHECK_EQ(state.flags.zero, 0);
    CHECK_EQ(state.flags.carry, 0);
    CHECK_EQ(state.flags.parity, 1);
    CHECK_EQ(state.flags.sign, 0);
  }
  SUBCASE("Testing CMP") {
    std::vector<uint8_t> data = {0x3E, 0x0A, 0x1E, 0x05, 0xBB};
    mem->load_data(data);

    emu.step();
    emu.step();
    emu.step();

    auto state = emu.get_state();

    CHECK_EQ(state.registers.reg_a, 0x0A);
    CHECK_EQ(state.flags.zero, 0);
    CHECK_EQ(state.flags.carry, 0);
    CHECK_EQ(state.flags.parity, 1);
    CHECK_EQ(state.flags.sign, 0);
  }
}
}  // namespace intel_8080
