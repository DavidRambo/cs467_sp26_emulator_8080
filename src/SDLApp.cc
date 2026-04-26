#include "SDLApp.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#include "CPU8080.h"

/* The 8080's IN instruction (opcode 0xdb plus the following byte) tells the CPU
 * to place the data in the input port addressed by the byte that follows the
 * opcode into the accumulator (register A). The bit set in the port corresponds
 * to what the input ought to do. The Space Invaders ROM has `0xdb 0x01` and
 * `0xdb 0x02` to indicate read from port 1 and 2, respectively. The bits in
 * port 1 have the corresponding functions:
 *  bit 0 = CREDIT (1 if deposit)
 *  bit 1 = 2P start (1 if pressed)
 *  bit 2 = 1P start (1 if pressed)
 *  bit 3 = Always 1
 *  bit 4 = 1P shot (1 if pressed)
 *  bit 5 = 1P left (1 if pressed)
 *  bit 6 = 1P right (1 if pressed)
 *  bit 7 = Not connected
 *
 * Port 2
 *  bit 0 = DIP3 00 = 3 ships  10 = 5 ships
 *  bit 1 = DIP5 01 = 4 ships  11 = 6 ships
 *  bit 2 = Tilt
 *  bit 3 = DIP6 0 = extra ship at 1500, 1 = extra ship at 1000
 *  bit 4 = P2 shot (1 if pressed)
 *  bit 5 = P2 left (1 if pressed)
 *  bit 6 = P2 right (1 if pressed)
 *  bit 7 = DIP7 Coin info displayed in demo screen 0=ON
 * From https://www.computerarcheology.com/Arcade/SpaceInvaders/Hardware.html
 *
 * Thus, a key press sets the corresponding bit in the corresponding input port.
 * Currently, I would guess that the emulator either gets an interrupt that
 * jumps to an IN instruction or it periodically runs the IN instructions as
 * part of its 60hz refresh of the screen.
 */
void handle_key_press(SDL_Scancode keycode, bool* running,
                      std::unique_ptr<CPU8080::CPU8080>& cpu) {
  switch (keycode) {
    case SDL_SCANCODE_ESCAPE:
    case SDL_SCANCODE_Q:
      *running = false;
      break;
    case SDL_SCANCODE_LEFT:
    case SDL_SCANCODE_A:
      SDL_Log("Left or 'a' scancode received.");
      cpu->port1.bit5 = 1;
      break;
    case SDL_SCANCODE_RIGHT:
    case SDL_SCANCODE_D:
      SDL_Log("Right or 'd' scancode received.");
      cpu->port1.bit6 = 1;
      break;
    case SDL_SCANCODE_UP:
      SDL_Log("Up scancode received.");
      break;
    case SDL_SCANCODE_DOWN:
      SDL_Log("Down scancode received.");
      break;
    case SDL_SCANCODE_SPACE:
      SDL_Log("Space scancode received.");
      cpu->port1.bit4 = 1;
      break;
    default:
      SDL_Log("A key was pressed: scancode %d", keycode);
      break;
  }
}

// Constructor
SDLApplication::SDLApplication() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    SDL_Log("Initialization failed!");
    // return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("Test SDL3", 224, 256, SDL_WINDOW_RESIZABLE,
                                   &window, &renderer)) {
    SDL_Log("Failed to create window and renderer.");
    // return SDL_APP_FAILURE;
  }
  // SDL_SetRenderLogicalPresentation(renderer, SDL_WINDOW_WIDTH,
  //                                  SDL_WINDOW_HEIGHT,
  //                                  SDL_LOGICAL_PRESENTATION_LETTERBOX);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderPresent(renderer);

  cpu_ptr = std::make_unique<CPU8080::CPU8080>();
}

// Destructor
SDLApplication::~SDLApplication() {
  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }
  SDL_Quit();
}

void SDLApplication::PollForEvent() {
  SDL_Event event{0};

  // Event handling loop
  (SDL_PollEvent(&event));
  switch (event.type) {
    case SDL_EVENT_QUIT:
      running = false;
      break;
    case SDL_EVENT_KEY_DOWN:  // This is how snake.c handles input.
      handle_key_press(event.key.scancode, &running, cpu_ptr);
      // Trigger IN instruction on cpu.
      // Then clear the input ports.
      cpu_ptr->clear_input_ports();
      break;
  }

  // Alternatively, after a PollEvent, can use
  // SDL_GetKeyboardState(nullptr), which returns an array of bools, which
  // can check which keys are pressed.
}

void SDLApplication::MainLoop() {
  while (running) {
    // App logic
    PollForEvent();
  }
};
