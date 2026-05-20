#include "GameWindow.h"

#include <SDL3/SDL.h>

namespace graphics_display {
GameWindow::GameWindow(std::span<unsigned char, 0x1C00> videoData,
                       int windowWidth, int windowHeight,
                       const char* windowName)
    : video_data_(videoData) {
  this->window_name_ = windowName;
  this->window_width_ = windowWidth;
  this->window_height_ = windowHeight;

  window_ = SDL_CreateWindow(windowName, windowWidth, windowHeight,
                             SDL_WINDOW_RESIZABLE);
  renderer_ = SDL_CreateRenderer(window_, nullptr);
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
