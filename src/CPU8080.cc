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
  port1 = Port();
  port2 = Port();
  clear_input_ports();
}

CPU8080::~CPU8080() {}

void CPU8080::clear_input_ports() {
  port1.bit0 = 0;
  port1.bit1 = 0;
  port1.bit2 = 0;
  port1.bit3 = 1;
  port1.bit4 = 0;
  port1.bit5 = 0;
  port1.bit6 = 0;
  port1.bit7 = 0;

  port2.bit0 = 0;
  port2.bit1 = 0;
  port2.bit2 = 0;
  port2.bit3 = 0;
  port2.bit4 = 0;
  port2.bit5 = 0;
  port2.bit6 = 0;
  port2.bit7 = 0;
}
}  // namespace CPU8080
