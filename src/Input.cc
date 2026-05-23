#include "Input.h"

#include <SDL3/SDL.h>

// Anonymous namespace for global variable
namespace {
// true = player 1, false = player 2
bool first_player{true};
}  // namespace

namespace input {

// Constructor initializes the three input device ports.
//
//
// Per https://www.computerarcheology.com/Arcade/SpaceInvaders/Hardware.html,
// certain bits are always set.
InputHandler::InputHandler() {
  devices_[0].bit1 = 1;
  devices_[0].bit2 = 1;
  devices_[0].bit3 = 1;

  devices_[1].bit3 = 1;
}

// Converts the InputDevice bitfield into a unified byte of data.
uint8_t InputDevice::to_byte() const {
  return static_cast<uint8_t>((bit0) | (bit1 << 1) | (bit2 << 2) | (bit3 << 3) |
                              (bit4 << 4) | (bit5 << 5) | (bit6 << 6) |
                              (bit7 << 7));
}

// Returns the data corresponding to the specified input device.
uint8_t InputHandler::ReadInput(uint8_t port_no) {
  return devices_[port_no].to_byte();
}

// Sets the appropriate device data bits for the given key press.
SDL_AppResult InputHandler::HandleKeyPress(SDL_Scancode keycode) {
  switch (keycode) {
    case SDL_SCANCODE_ESCAPE:
    case SDL_SCANCODE_Q:
      return SDL_APP_SUCCESS;
    case SDL_SCANCODE_LEFT:
    case SDL_SCANCODE_A:
      if (first_player) {
        // Player 1 left: set Port 1 bit 5.
        devices_[1].bit5 = 1;
      } else {
        // Player 2 left: set Port 2 bit 5.
        devices_[2].bit5 = 1;
      }
      break;
    case SDL_SCANCODE_RIGHT:
    case SDL_SCANCODE_D:
      if (first_player) {
        // Player 1 right: set Port 1 bit 6.
        devices_[1].bit6 = 1;
      } else {
        // Player 2 right: set Port 2 bit 6.
        devices_[2].bit6 = 1;
      }
      break;
    case SDL_SCANCODE_C:
      // Set Port 1 bit 0 (Deposit Credit)
      devices_[1].bit0 = 1;
      break;
    case SDL_SCANCODE_W:
    case SDL_SCANCODE_UP:
    case SDL_SCANCODE_SPACE:
      if (first_player) {
        // Player 1 shoot: set Port 1 bit 4.
        devices_[1].bit4 = 1;
      } else {
        // Player 2 shoot: set Port 2 bit 4.
        devices_[2].bit4 = 1;
      }
      break;
    case SDL_SCANCODE_RETURN:
      // Set Port 1 bit 2 (Player 1 Start)
      devices_[1].bit2 = 1;
      break;
    default:
      break;
  }

  return SDL_APP_CONTINUE;
}

// Clears the device bits for the given key release.
SDL_AppResult InputHandler::HandleKeyRelease(SDL_Scancode keycode) {
  switch (keycode) {
    case SDL_SCANCODE_ESCAPE:
    case SDL_SCANCODE_Q:
      return SDL_APP_SUCCESS;
    case SDL_SCANCODE_LEFT:
    case SDL_SCANCODE_A:
      if (first_player) {
        // Player 1 left: set Port 1 bit 5.
        devices_[1].bit5 = 0;
      } else {
        // Player 2 left: set Port 2 bit 5.
        devices_[2].bit5 = 0;
      }
      break;
    case SDL_SCANCODE_RIGHT:
    case SDL_SCANCODE_D:
      if (first_player) {
        // Player 1 right: set Port 1 bit 6.
        devices_[1].bit6 = 0;
      } else {
        // Player 2 right: set Port 2 bit 6.
        devices_[2].bit6 = 0;
      }
      break;
    case SDL_SCANCODE_C:
      // Set Port 1 bit 0 (Deposit Credit)
      devices_[1].bit0 = 0;
      break;
    case SDL_SCANCODE_W:
    case SDL_SCANCODE_UP:
    case SDL_SCANCODE_SPACE:
      if (first_player) {
        // Player 1 shoot: set Port 1 bit 4.
        devices_[1].bit4 = 0;
      } else {
        // Player 2 shoot: set Port 2 bit 4.
        devices_[2].bit4 = 0;
      }
      break;
    case SDL_SCANCODE_RETURN:
      // Set Port 1 bit 2 (Player 1 Start)
      devices_[1].bit2 = 0;
      break;
    default:
      break;
  }

  return SDL_APP_CONTINUE;
}
// Polls for an SDL_Event and handles it.
//
// Note that this must be called in sync with the rest of the game code.
// Otherwise, were the polling to happen at a greater rate, then it would likely
// clear the input device state before it had a chance to be read by the 8080.
SDL_AppResult InputHandler::PollForEvents() {
  SDL_Event event{0};

  // Event handling loop
  (SDL_PollEvent(&event));
  switch (event.type) {
    case SDL_EVENT_QUIT:
      // NOTE: This is here in case we have another SDL event to quit.
      break;
    case SDL_EVENT_KEY_DOWN:
      return HandleKeyPress(event.key.scancode);
      break;
    case SDL_EVENT_KEY_UP:
      return HandleKeyRelease(event.key.scancode);
      break;
    default:
      break;
  }

  return SDL_APP_CONTINUE;
}
}  // namespace input
