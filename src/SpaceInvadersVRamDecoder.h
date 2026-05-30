#ifndef SPACEINVADERSVRAMDECODER
#define SPACEINVADERSVRAMDECODER
#include <SDL3/SDL.h>

#include <span>
#include <vector>

namespace space_invaders_vram_decoder {
struct Pixels {
  std::vector<SDL_FPoint> purple_points;
  std::vector<SDL_FPoint> white_points;
  std::vector<SDL_FPoint> green_points;

  void clear();
};

void DecodePixels(Pixels& pixels, std::span<unsigned char, 0x1C00> video_data);
}  // namespace space_invaders_vram_decoder
#endif
