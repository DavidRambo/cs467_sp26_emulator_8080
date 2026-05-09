#include "../src/CPU8080.h"
#include "../src/Memory8080.h"
#include "doctest.h"

TEST_CASE("Test IN input instruction") {
  std::shared_ptr<intel_8080::Memory8080> mem =
      std::make_shared<intel_8080::Memory8080>(intel_8080::Memory8080());
  intel_8080::CPU8080 emu = intel_8080::CPU8080(mem);
  std::vector<uint8_t> data = {0xdb, 1};  // opcode, port number
  mem->load_data(data);

  emu.write_input_port(1, 0b100);  // set port1.bit2

  emu.step();
  intel_8080::CPU8080::State state = emu.get_state();
  CHECK_EQ(state.registers.reg_b, 0);
  CHECK_EQ(state.registers.reg_c, 0);
  CHECK_EQ(state.registers.reg_a, 0b100);
  CHECK_EQ(state.registers.reg_d, 0);
  CHECK_EQ(state.registers.reg_h, 0);
  CHECK_EQ(state.registers.reg_l, 0);
  CHECK_EQ(state.program_counter, 2);
  CHECK_EQ(state.flags.sign, 0);
  CHECK_EQ(state.flags.zero, 0);
  CHECK_EQ(state.flags.parity, 0);
  CHECK_EQ(state.flags.carry, 0);
}
