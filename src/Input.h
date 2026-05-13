#pragma once
#include <SDL3/SDL.h>

#include <array>
#include <memory>

#include "CPU8080.h"

namespace input {
struct InputDevice {
  uint8_t bit0 : 1;
  uint8_t bit1 : 1;
  uint8_t bit2 : 1;
  uint8_t bit3 : 1;
  uint8_t bit4 : 1;
  uint8_t bit5 : 1;
  uint8_t bit6 : 1;
  uint8_t bit7 : 1;

  uint8_t to_byte() const;

  InputDevice() {
    bit0 = 0;
    bit1 = 0;
    bit2 = 0;
    bit3 = 0;
    bit4 = 0;
    bit5 = 0;
    bit6 = 0;
    bit7 = 0;
  }
};

class InputHandler {
 public:
  InputHandler();

  uint8_t read_input(uint8_t port_no);

 private:
  void clear_ports();

  SDL_AppResult handle_key_press(SDL_Scancode keycode);

  SDL_AppResult poll_for_event();

  std::array<InputDevice, 3> devices_;
};

}  // namespace input
