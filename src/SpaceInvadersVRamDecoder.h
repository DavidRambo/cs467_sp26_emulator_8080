#ifndef SPACEINVADERSVRAMDECODER
#define SPACEINVADERSVRAMDECODER
#include <SDL3/SDL.h>

#include <span>
#include <vector>

namespace space_invaders_vram_decoder {
void DecodeTopPixels(std::vector<SDL_FPoint>& points,
                     std::span<unsigned char, 0x1C00> data);

void DecodeBottomPixels(std::vector<SDL_FPoint>& points,
                        std::span<unsigned char, 0x1C00> data);

void DecodePixels(std::vector<SDL_FPoint>& points,
                  std::span<unsigned char, 0x1C00> video_data);
}  // namespace space_invaders_vram_decoder
#endif
