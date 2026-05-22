#include <memory>

#include "../src/CPU8080.h"
#include "../src/Memory8080.h"
#include "doctest.h"

TEST_CASE("Testing MVI A, D8") {
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

  std::vector<uint8_t> data = {0x3E, 0xAB};
  mem->load_data(data);
  emu.step();

  auto state = emu.get_state();
  CHECK_EQ(state.registers.reg_b, 0);
  CHECK_EQ(state.registers.reg_c, 0);
  CHECK_EQ(state.registers.reg_a, 0xAB);
  CHECK_EQ(state.registers.reg_d, 0);
  CHECK_EQ(state.registers.reg_h, 0);
  CHECK_EQ(state.registers.reg_l, 0);
  CHECK_EQ(state.stack_pointer, 0);
  CHECK_EQ(state.program_counter, 2);
  CHECK_EQ(state.flags.sign, 0);
  CHECK_EQ(state.flags.zero, 0);
  CHECK_EQ(state.flags.parity, 0);
  CHECK_EQ(state.flags.carry, 0);
}
