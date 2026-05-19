#include <memory>

#include "../src/CPU8080.h"
#include "../src/Memory8080.h"
#include "doctest.h"

TEST_CASE("Testing INR B: B += 1") {
  std::shared_ptr<intel_8080::Memory8080> mem =
      std::make_shared<intel_8080::Memory8080>(intel_8080::Memory8080());
  std::shared_ptr<input::InputHandler> input_handler =
      std::make_shared<input::InputHandler>();
  intel_8080::CPU8080 emu = intel_8080::CPU8080(mem, input_handler);

  SUBCASE("Incrementing B from 0 to 1") {
    std::vector<uint8_t> data = {0x04};
    mem->load_data(data);
    emu.step();
    intel_8080::CPU8080::State state = emu.get_state();
    CHECK_EQ(state.registers.reg_b, 1);
    CHECK_EQ(state.registers.reg_c, 0);
    CHECK_EQ(state.registers.reg_a, 0);
    CHECK_EQ(state.registers.reg_d, 0);
    CHECK_EQ(state.registers.reg_h, 0);
    CHECK_EQ(state.registers.reg_l, 0);
    CHECK_EQ(state.program_counter, 1);
    CHECK_EQ(state.flags.sign, 0);
    CHECK_EQ(state.flags.zero, 0);
    CHECK_EQ(state.flags.parity, 0);
    CHECK_EQ(state.flags.carry, 0);
  }

  SUBCASE("Incrementing B 256 times") {
    std::vector<uint8_t> data;
    data.reserve(256);
    for (int i = 0; i < 256; i++) {
      data.emplace_back(0x04);
    }
    mem->load_data(data);
    for (int i = 0; i < 256; i++) {
      emu.step();
    }

    intel_8080::CPU8080::State state = emu.get_state();
    CHECK_EQ(state.registers.reg_b, 0);
    CHECK_EQ(state.registers.reg_c, 0);
    CHECK_EQ(state.registers.reg_a, 0);
    CHECK_EQ(state.registers.reg_d, 0);
    CHECK_EQ(state.registers.reg_h, 0);
    CHECK_EQ(state.registers.reg_l, 0);
    CHECK_EQ(state.program_counter, 256);
    CHECK_EQ(state.flags.sign, 0);
    CHECK_EQ(state.flags.zero, 1);
    CHECK_EQ(state.flags.parity, 1);
    CHECK_EQ(state.flags.carry, 0);
  }
}
