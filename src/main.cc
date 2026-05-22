#include <SDL3/SDL.h>

#include <iostream>

#include "AudioMixer.h"
#include "CPU8080.h"
#include "GameWindow.h"
#include "Input.h"
#include "Memory8080.h"
#include "ShiftRegister.h"
#include "SpaceInvadersVRamDecoder.h"

int main(int argc, char* argv[]) {
  // if (argc > 5) {
  //   std::cerr << "Too many arguments. Usage: ./emu8008 invaders.h invaders.g
  //   "
  //                "invaders.f invaders.e\n";
  //                }
  if (argc > 2) {
#ifdef debug
    std::cerr << "Too many arguments. Use: ./emu8080 invaders 1>outpt.txt "
                 "2>err.txt\n";
#endif
    std::cerr << "Too many arguments. Use: ./emu8080 invaders\n";
    return 1;
  }

  constexpr int kWindowWidth = 224;
  constexpr int kWindowHeight = 256;

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO)) {
    SDL_Log("Initialization failed!");
    // return SDL_APP_FAILURE;
  }

  // Initialize and fill memory first, so it can be passed to the GameWindow.
  std::shared_ptr<intel_8080::Memory8080> mem =
      std::make_shared<intel_8080::Memory8080>(intel_8080::Memory8080());

  // Load ROM
  mem->load_rom(argv[1]);  // invaders ROM as one file

  mem->fill_vram();

  // Create GameWindow
  graphics_display::GameWindow game_window = graphics_display::GameWindow(
      kWindowWidth, kWindowHeight, "Space Invaders");

  std::vector<SDL_FPoint> points;
  space_invaders_vram_decoder::DecodePixels(points, mem->get_vram_span());
  game_window.UpdateDisplay(points);

  SDL_Delay(3000);

  std::shared_ptr<audio::Mixer> mixer =
      std::make_shared<audio::Mixer>(audio::Mixer());

  std::shared_ptr<hardware::ShiftRegister> shift_reg_ptr =
      std::make_shared<hardware::ShiftRegister>(hardware::ShiftRegister());

  // Shared with cpu and main loop for event polling.
  std::shared_ptr<input::InputHandler> input_handler =
      std::make_shared<input::InputHandler>();

  intel_8080::CPU8080 cpu =
      intel_8080::CPU8080(mem, input_handler, mixer, shift_reg_ptr);

  // Main loop
  bool running{true};
  Uint64 last_time{0};
  while (running) {
    Uint64 start_tick = SDL_GetTicks();

    if (input_handler->PollForEvents() != SDL_APP_CONTINUE) {
      running = false;
      continue;
    }

    // Run CPU for 16 ms to approximate 60 fps.
    Uint64 current_tick = SDL_GetTicks();
    while (current_tick < start_tick + 16) {
      cpu.step();
      current_tick = SDL_GetTicks();
    }

    // Update display
    std::vector<SDL_FPoint> points;
    space_invaders_vram_decoder::DecodePixels(points, mem->get_vram_span());
    game_window.UpdateDisplay(points);
  }

  // Shut down application.
  SDL_Quit();

  return 0;
}
