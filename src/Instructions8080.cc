#include "CPU8080.h"

namespace intel_8080 {
// INR Increment Register or Memory value
//
// Condition bits affected: Zero, Sign, Parity
void CPU8080::inr(uint8_t* reg) {
  reg += 1;
  update_flags_szp(*reg);
}
}  // namespace intel_8080
