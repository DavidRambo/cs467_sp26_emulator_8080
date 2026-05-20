#ifndef GAMEWINDOW
#define GAMEWINDOW
#include <SDL3/SDL.h>

#include <span>
#include <vector>

namespace graphics_display {
class GameWindow {
 private:
  std::span<unsigned char, 0x1C00> video_data_;
  const char* window_name_;
  int window_width_;
  int window_height_;

  SDL_Window* window_;
  SDL_Renderer* renderer_;

 public:
  GameWindow(std::span<unsigned char, 0x1C00> videoData, int windowWidth,
             int windowHeight, const char* windowName);
  ~GameWindow();
  void UpdateDisplayTop(const std::vector<SDL_FPoint>& points);
  void UpdateDisplayBottom(const std::vector<SDL_FPoint>& points);
};
}  // namespace graphics_display
#endif
