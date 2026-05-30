#include "GameWindow.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>

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

  SDL_SetRenderLogicalPresentation(renderer_, windowWidth, windowHeight,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer_);
  SDL_RenderPresent(renderer_);
}

void GameWindow::UpdateDisplay(
    const space_invaders_vram_decoder::Pixels& pixels) {
  // Clear screen to black.
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer_);

  // CRT green for lower portion of the screen.
  SDL_SetRenderDrawColor(renderer_, 65, 255, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderPoints(renderer_, pixels.green_points.data(),
                   pixels.green_points.size());

  // White for the upper portion.
  SDL_SetRenderDrawColor(renderer_, 255, 255, 255, SDL_ALPHA_OPAQUE);
  SDL_RenderPoints(renderer_, pixels.white_points.data(),
                   pixels.white_points.size());
  SDL_RenderPresent(renderer_);
}

GameWindow::~GameWindow() {
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}
}  // namespace graphics_display
