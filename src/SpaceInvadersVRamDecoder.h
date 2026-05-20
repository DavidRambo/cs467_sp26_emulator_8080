#ifndef SPACEINVADERSVRAMDECODER
#define SPACEINVADERSVRAMDECODER
#include <SDL3/SDL.h>

#include <span>
#include <vector>

namespace space_invaders_vram_decoder {
std::vector<SDL_FPoint> DecodeTopPixels(std::span<unsigned char, 0x1C00> data);
std::vector<SDL_FPoint> DecodeBottomPixels(
    std::span<unsigned char, 0x1C00> data);
}  // namespace space_invaders_vram_decoder
#endif
