#pragma once
#include <SDL3/SDL.h>

#include <array>
#include <memory>

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

  // Made public for the sake of testing.
  SDL_AppResult HandleKeyPress(SDL_Scancode keycode);
  SDL_AppResult HandleKeyRelease(SDL_Scancode keycode);

  // Polls for input event, to be called by SDL3 game loop.
  SDL_AppResult PollForEvents();

  // Emulates the transfer of device data over bus, to be called by cpu.
  uint8_t ReadInput(uint8_t port_no);

 private:
  // Corresponds to Input (Read) Devices 0–2. Only 1 and 2 are used by the game.
  std::array<InputDevice, 3> devices_;
};

}  // namespace input
