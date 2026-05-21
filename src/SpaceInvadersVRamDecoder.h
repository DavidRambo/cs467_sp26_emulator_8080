#ifndef SPACEINVADERSVRAMDECODER
#define SPACEINVADERSVRAMDECODER
#include <SDL3/SDL.h>

#include <span>
#include <vector>

namespace space_invaders_vram_decoder {
std::vector<SDL_FPoint> const& DecodeTopPixels(
    std::vector<SDL_FPoint>& points, std::span<unsigned char, 0x1C00> data);

std::vector<SDL_FPoint> const& DecodeBottomPixels(
    std::vector<SDL_FPoint>& points, std::span<unsigned char, 0x1C00> data);
}  // namespace space_invaders_vram_decoder
#endif
