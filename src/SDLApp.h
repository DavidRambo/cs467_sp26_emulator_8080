#pragma once
#include <SDL3/SDL.h>

#include "CPU8080.h"

void handle_key_press(SDL_Scancode keycode, bool* running,
                      std::unique_ptr<CPU8080::CPU8080>& cpu);

struct SDLApplication {
  static constexpr int SDL_WINDOW_WIDTH = 224;
  static constexpr int SDL_WINDOW_HEIGHT = 256;

  SDL_Window* window{nullptr};
  SDL_Renderer* renderer{nullptr};
  bool running = true;
  std::unique_ptr<CPU8080::CPU8080> cpu_ptr;

  SDLApplication();
  ~SDLApplication();
  void PollForEvent();
  void MainLoop();
};
