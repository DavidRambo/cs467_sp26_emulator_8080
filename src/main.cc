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
#ifdef DEBUG
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
  while (running) {
    Uint64 start_tick = SDL_GetTicks();
    if (input_handler->PollForEvents() != SDL_APP_CONTINUE) {
      running = false;
      break;
    }

    // Run CPU for 8 ms to approximate the "first half" of 60 fps.
    // NOTE: the emulated cpu will perform a much greater number of cycles in
    // this time than it would as physical hardware running at 2Mhz. The way to
    // account for this would be to count cycles and to limit it to however many
    // cycles could occur at 2Mhz within 8ms (i.e. 30hz) = 2Mhz/30hz.
    Uint64 current_tick = SDL_GetTicks();
    while (cpu.is_not_stopped() && current_tick < start_tick + 8) {
      cpu.step();
      current_tick = SDL_GetTicks();
    }

    // Trigger first half of screen update with RST 1.
    // TODO: call RST(1) diectly? -> instruction methods are private. But
    // neither can we "manually" do so by pushing the current program_counter_
    // onto the stack and then placing the RST opcode into it, since it's
    // possible the CPU is STOPPED from a HLT instruction.

    // Run CPU for another 8 ms to approximate the "second half" of 60 fps.
    while (cpu.is_not_stopped() && current_tick < start_tick + 8) {
      cpu.step();
      current_tick = SDL_GetTicks();
    }

    // Trigger first half of screen update with RST 2.
    // TODO: call RST(2) diectly?

    // Update display
    std::vector<SDL_FPoint> points;
    space_invaders_vram_decoder::DecodePixels(points, mem->get_vram_span());
    game_window.UpdateDisplay(points);
  }

  // Shut down application.
  SDL_Quit();

  return 0;
}
