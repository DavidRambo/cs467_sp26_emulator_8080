#include "AudioMixer.h"
#include "CPU8080.h"
#include "GameWindow.h"
#include "Input.h"
#include "Memory8080.h"
#include "SpaceInvadersVRamDecoder.h"

// Display is 256x224
constexpr int kDisplayWidth = 256;
constexpr int kDisplayHeight = 224;

int main() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;
  }

  auto mem = std::make_shared<intel_8080::Memory8080>();
  mem->load_rom("invaders");

  auto input = std::make_shared<input::InputHandler>();
  auto mixer = std::make_shared<audio::Mixer>();
  intel_8080::CPU8080 cpu(mem, input, mixer);

  graphics_display::GameWindow window(nullptr, kDisplayWidth, kDisplayHeight,
                                      "Space Invaders");

  bool running = true;
  while (running) {
    if (input->PollForEvents() != SDL_APP_CONTINUE) {
      running = false;
    }

    // TODO: CPU steps through opcodes

    // TODO: CPU checks for interrupts

    // TODO: Game Window grabs latest VRAM data and updates screen
  }

  SDL_Quit();
  return 0;
}
