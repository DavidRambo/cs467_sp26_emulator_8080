#include "GameWindow.h"

#include <SDL3/SDL.h>

#include <cstdlib>

namespace graphics_display {
GameWindow::GameWindow(int windowWidth, int windowHeight,
                       const char* windowName) {
  window_name_ = windowName;
  window_width_ = windowWidth;
  window_height_ = windowHeight;

  if (!SDL_CreateWindowAndRenderer("Test SDL3", windowWidth, windowHeight,
                                   SDL_WINDOW_RESIZABLE, &window_,
                                   &renderer_)) {
    SDL_Log("Failed to create window and renderer.");
    std::exit(1);
    // return SDL_APP_FAILURE;
  }

  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer_);
  SDL_RenderPresent(renderer_);
}

void GameWindow::UpdateDisplayTop(const std::vector<SDL_FPoint>& points) {
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer_);

  SDL_SetRenderDrawColor(renderer_, 255, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderPoints(renderer_, points.data(), points.size());
  SDL_RenderPresent(renderer_);
}

void GameWindow::UpdateDisplayBottom(const std::vector<SDL_FPoint>& points) {
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer_);

  SDL_SetRenderDrawColor(renderer_, 0, 255, 255, SDL_ALPHA_OPAQUE);
  SDL_RenderPoints(renderer_, points.data(), points.size());
  SDL_RenderPresent(renderer_);
}

GameWindow::~GameWindow() {
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
}
}  // namespace graphics_display
