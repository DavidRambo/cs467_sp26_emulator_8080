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

  SUBCASE("Testing STA") {
    std::vector<uint8_t> data = {0x3E, 0x78, 0x032, 0xB3, 0x05};
    mem->load_data(data);

    emu.step();
    emu.step();

    CHECK_EQ(mem->read(0x05B3), 0x78);
  }

  SUBCASE("Testing LDA") {
    std::vector<uint8_t> data = {0x3A, 0x00, 0x03};
    mem->load_data(data);
    mem->write(0x0300, 0x78);

    emu.step();

    auto state = emu.get_state();
    CHECK_EQ(state.registers.reg_a, 0x78);
  }

  SUBCASE("Testing SHLD") {
    std::vector<uint8_t> data = {0x26, 0xAE, 0x2E, 0x29, 0x22, 0x0A, 0x01};
    mem->load_data(data);

    emu.step();
    emu.step();
    emu.step();

    auto state = emu.get_state();

    CHECK_EQ(mem->read(0x010A), 0x29);
    CHECK_EQ(mem->read(0x010B), 0xAE);
  }

  SUBCASE("Testing LHLD") {
    std::vector<uint8_t> data = {0x2A, 0x5B, 0x02};
    mem->load_data(data);

    mem->write(0x025B, 0xFF);
    mem->write(0x025C, 0x03);

    emu.step();

    auto state = emu.get_state();

    CHECK_EQ(state.registers.reg_h, 0x03);
    CHECK_EQ(state.registers.reg_l, 0xFF);
  }
}
}  // namespace intel_8080
