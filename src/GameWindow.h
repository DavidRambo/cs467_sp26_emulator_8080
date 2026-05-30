#ifndef GAMEWINDOW
#define GAMEWINDOW
#include <SDL3/SDL.h>

#include <span>
#include <vector>

#include "SpaceInvadersVRamDecoder.h"

namespace graphics_display {
class GameWindow {
 private:
  const char* window_name_;
  int window_width_;
  int window_height_;

  SDL_Window* window_;
  SDL_Renderer* renderer_;

 public:
  GameWindow(int windowWidth, int windowHeight, const char* windowName);
  ~GameWindow();

  void UpdateDisplay(const space_invaders_vram_decoder::Pixels& pixels);
};
}  // namespace graphics_display
#endif
