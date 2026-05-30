#include <SDL3/SDL.h>

#include <iostream>

#include "AudioMixer.h"
#include "CPU8080.h"
#include "GameWindow.h"
#include "Input.h"
#include "Memory8080.h"
#include "ShiftRegister.h"
#include "SpaceInvadersVRamDecoder.h"

// Intel 8080: 2MHz; Refresh Rate: 60Hz
// Cycles Per Frame: 2MHz / 60Hz = 33.333K
constexpr int kCyclesHalfFrame = 16667;

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
      std::make_shared<intel_8080::Memory8080>();

  // Load ROM
  mem->load_rom(argv[1]);  // invaders ROM as one file

  // Create GameWindow
  graphics_display::GameWindow game_window = graphics_display::GameWindow(
      kWindowWidth, kWindowHeight, "Space Invaders");

  space_invaders_vram_decoder::Pixels pixels;
  space_invaders_vram_decoder::DecodePixels(pixels, mem->get_vram_span());
  game_window.UpdateDisplay(pixels);

  std::shared_ptr<audio::Mixer> mixer = std::make_shared<audio::Mixer>();

  std::shared_ptr<hardware::ShiftRegister> shift_reg_ptr =
      std::make_shared<hardware::ShiftRegister>();

  // Shared with cpu and main loop for event polling.
  std::shared_ptr<input::InputHandler> input_handler =
      std::make_shared<input::InputHandler>();

  intel_8080::CPU8080 cpu =
      intel_8080::CPU8080(mem, input_handler, mixer, shift_reg_ptr);

  // Main loop
  bool running{true};
#ifdef DEBUG
  int loop_count{0};
#endif
  while (running) {
#ifdef DEBUG
    if (loop_count > 0) {
      std::cerr << "On loop # " << loop_count << "\n";
    }
#endif
    if (input_handler->PollForEvents() != SDL_APP_CONTINUE) {
      running = false;
      break;
    }

    // Run the CPU for one half-frame's worth of cycles (~16,667),
    // emulating what the hardware would do between interrupts.
    Uint64 start_tick = SDL_GetTicks();
    uint16_t cycles = 0;
    while (cpu.is_not_stopped() && cycles < kCyclesHalfFrame) {
      cycles += cpu.step();
    }
#ifdef DEBUG
    std::cout << "RST 1\n";
#endif

    // Trigger interrupt for first half of screen update with RST + 8, which has
    // exp = 1.
    cpu.queue_interrupt(0xCF);

    // Run the second half-frame's worth of cycles before next interrupt.
    cycles = 0;
    while (cpu.is_not_stopped() && cycles < kCyclesHalfFrame) {
      cycles += cpu.step();
    }

#ifdef DEBUG
    std::cout << "RST 2\n";
#endif

    // Trigger interrupt for second half of screen update with RST + 10, which
    // has exp = 2.
    cpu.queue_interrupt(0xD7);

    // Frame pacing: target ~60 fps (16 ms per frame).
    uint64_t elapsed = SDL_GetTicks() - start_tick;
    if (elapsed < 16) {
      SDL_Delay(static_cast<uint32_t>(16 - elapsed));
    }

    // Update display
    space_invaders_vram_decoder::DecodePixels(pixels, mem->get_vram_span());
    game_window.UpdateDisplay(pixels);
  }

  return 0;
}
