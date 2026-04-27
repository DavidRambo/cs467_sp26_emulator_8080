#include "CPU8080.h"

namespace CPU8080 {
CPU8080::Port::Port() {
  bit0 = 0;
  bit1 = 0;
  bit2 = 0;
  bit3 = 0;
  bit4 = 0;
  bit5 = 0;
  bit6 = 0;
  bit7 = 0;
}

CPU8080::Port::~Port() {}

CPU8080::CPU8080() {
  port1_ = Port();
  port2_ = Port();
  clear_input_ports();
}

CPU8080::~CPU8080() {}

void CPU8080::clear_input_ports() {
  port1_.bit0 = 0;
  port1_.bit1 = 0;
  port1_.bit2 = 0;
  port1_.bit3 = 1;
  port1_.bit4 = 0;
  port1_.bit5 = 0;
  port1_.bit6 = 0;
  port1_.bit7 = 0;

  port2_.bit0 = 0;
  port2_.bit1 = 0;
  port2_.bit2 = 0;
  port2_.bit3 = 0;
  port2_.bit4 = 0;
  port2_.bit5 = 0;
  port2_.bit6 = 0;
  port2_.bit7 = 0;
}
}  // namespace CPU8080
