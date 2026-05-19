#include <SDL3/SDL.h>

#include "AudioMixer.h"
#include "CPU8080.h"
#include "Input.h"
#include "Memory8080.h"
int main() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    SDL_Log("Initialization failed!");
    // return SDL_APP_FAILURE;
  }

  // TODO: Create GameWindow

  std::shared_ptr<audio::Mixer> mixer =
      std::make_shared<audio::Mixer>(audio::Mixer());

  // Shared with cpu and main loop for event polling.
  std::shared_ptr<input::InputHandler> input_handler =
      std::make_shared<input::InputHandler>();

  std::shared_ptr<intel_8080::Memory8080> mem =
      std::make_shared<intel_8080::Memory8080>(intel_8080::Memory8080());

  intel_8080::CPU8080 cpu = intel_8080::CPU8080(mem, input_handler, mixer);

  // TODO: Main loop

  return 0;
}
