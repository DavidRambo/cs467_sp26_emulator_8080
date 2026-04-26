#pragma once
#include <cstdint>
#include <memory>

namespace CPU8080 {
struct CPU8080 {
  // Input Ports
  struct Port {
    uint8_t bit0 : 1;  // CREDIT (1 if deposit)
    uint8_t bit1 : 1;  // 2P start (1 if pressed)
    uint8_t bit2 : 1;  // 1P start (1 if pressed)
    uint8_t bit3 : 1;  // Always 1
    uint8_t bit4 : 1;  // 1P shot (1 if pressed)
    uint8_t bit5 : 1;  // 1P left (1 if pressed)
    uint8_t bit6 : 1;  // 1P right (1 if pressed)
    uint8_t bit7 : 1;  // Not connected

    Port();
    ~Port();
  };

  Port port1;
  Port port2;

  CPU8080();

  ~CPU8080();

  void clear_input_ports();
};
}  // namespace CPU8080
