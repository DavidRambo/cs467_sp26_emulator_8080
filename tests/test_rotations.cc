#include <memory>

#include "../src/CPU8080.h"
#include "../src/Memory8080.h"
#include "doctest.h"

// This case is from the manual's example.
// 1111'0010 << 1 = 1110'0100
// carry => 1110'0101
//
// It requires opcode 0x3E, MVI A, D8.
TEST_CASE("Testing RLC instruction") {
  std::shared_ptr<intel_8080::Memory8080> mem =
      std::make_shared<intel_8080::Memory8080>(intel_8080::Memory8080());
  std::shared_ptr<input::InputHandler> input_handler =
      std::make_shared<input::InputHandler>();
  intel_8080::CPU8080 emu = intel_8080::CPU8080(mem, input_handler);

  std::vector<uint8_t> data = {0x3E, 0xF2, 0x07};
  mem->load_data(data);
  emu.step();
  emu.step();

  intel_8080::CPU8080::State state = emu.get_state();
  CHECK_EQ(state.registers.reg_b, 0);
  CHECK_EQ(state.registers.reg_c, 0);
  CHECK_EQ(state.registers.reg_a, 0xE5);
  CHECK_EQ(state.registers.reg_d, 0);
  CHECK_EQ(state.registers.reg_h, 0);
  CHECK_EQ(state.registers.reg_l, 0);
  CHECK_EQ(state.stack_pointer, 0);
  CHECK_EQ(state.program_counter, 3);
  CHECK_EQ(state.flags.sign, 0);
  CHECK_EQ(state.flags.zero, 0);
  CHECK_EQ(state.flags.parity, 0);
  CHECK_EQ(state.flags.carry, 1);
}
