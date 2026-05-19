#include <SDL3/SDL.h>

#include "AudioMixer.h"
#include "CPU8080.h"
#include "Input.h"
#include "Memory8080.h"
int main() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO)) {
    SDL_Log("Initialization failed!");
    // return SDL_APP_FAILURE;
  }

  // TODO: Create GameWindow
  SDL_Window* window;
  SDL_Renderer* renderer;
  if (!SDL_CreateWindowAndRenderer("Test SDL3", 224, 256, SDL_WINDOW_RESIZABLE,
                                   &window, &renderer)) {
    SDL_Log("Failed to create window and renderer.");
    return SDL_APP_FAILURE;
  }

  // SDL_SetRenderLogicalPresentation(renderer, SDL_WINDOW_WIDTH,
  //                                  SDL_WINDOW_HEIGHT,
  //                                  SDL_LOGICAL_PRESENTATION_LETTERBOX);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderPresent(renderer);

  std::shared_ptr<audio::Mixer> mixer =
      std::make_shared<audio::Mixer>(audio::Mixer());

  // Shared with cpu and main loop for event polling.
  std::shared_ptr<input::InputHandler> input_handler =
      std::make_shared<input::InputHandler>();

  std::shared_ptr<intel_8080::Memory8080> mem =
      std::make_shared<intel_8080::Memory8080>(intel_8080::Memory8080());

  intel_8080::CPU8080 cpu = intel_8080::CPU8080(mem, input_handler, mixer);

  // Main loop
  bool running{true};
  Uint64 last_time{0};
  while (running) {
    Uint64 start_tick = SDL_GetTicks();

    if (input_handler->PollForEvents() == SDL_APP_SUCCESS) {
      running = false;
      continue;
    }

    // Run CPU for 16 ns to approximate 60 fps.
    Uint64 current_tick = SDL_GetTicks();
    while (current_tick < start_tick + 16) {
      cpu.step();
      current_tick = SDL_GetTicks();
    }

    // Update display
  }

  // Shut down application.
  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }
  SDL_Quit();

  return 0;
}
