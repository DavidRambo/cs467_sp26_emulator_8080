#include <memory>

#include "../src/CPU8080.h"
#include "../src/Memory8080.h"
#include "doctest.h"

TEST_CASE("Testing INR B: B += 1") {
  std::shared_ptr<intel_8080::Memory8080> mem =
      std::make_shared<intel_8080::Memory8080>(intel_8080::Memory8080());
  intel_8080::CPU8080 emu = intel_8080::CPU8080(mem);

  SUBCASE("Incrementing B from 0 to 1") {
    std::vector<uint8_t> data = {0x04};
    mem->LoadData(data);
    emu.step();
    auto curr_regs = emu.get_registers();
    CHECK_EQ(curr_regs.reg_b, 1);
    CHECK_EQ(curr_regs.reg_c, 0);
    CHECK(curr_regs.reg_a == 0);
    CHECK(curr_regs.reg_d == 0);
    CHECK(curr_regs.reg_h == 0);
    CHECK(curr_regs.reg_l == 0);
  }
}
