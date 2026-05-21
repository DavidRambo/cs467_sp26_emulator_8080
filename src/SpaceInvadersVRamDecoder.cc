#include "SpaceInvadersVRamDecoder.h"

#include <SDL3/SDL.h>

#include <vector>

namespace space_invaders_vram_decoder {
constexpr int kMmCols = 32;
constexpr int kMmRows = 224;
constexpr int kMmPixPerByte = 8;

std::vector<SDL_FPoint> const& DecodeTopPixels(
    std::vector<SDL_FPoint>& points, std::span<unsigned char, 0x1C00> data) {
  for (int row = 112; row < kMmRows; row++) {
    for (int col = 0; col < kMmCols; col++) {
      for (int bit = 0; bit < kMmPixPerByte; bit++) {
        bool pixel_set =
            static_cast<bool>(data[(row * kMmCols) + col] & (0x01 << bit));
        if (pixel_set) {
          SDL_FPoint pixel{.x = static_cast<float>((col * kMmPixPerByte) + bit),
                           .y = static_cast<float>(row)};
          points.push_back(pixel);
        }
      }
    }
  }
  return points;
}

std::vector<SDL_FPoint> const& DecodeBottomPixels(
    std::vector<SDL_FPoint>& points, std::span<unsigned char, 0x1C00> data) {
  for (int row = 0; row < kMmRows / 2; row++) {
    for (int col = 0; col < kMmCols; col++) {
      for (int bit = 0; bit < kMmPixPerByte; bit++) {
        bool pixel_set =
            static_cast<bool>(data[(row * kMmCols) + col] & (0x01 << bit));
        if (pixel_set) {
          SDL_FPoint pixel{.x = static_cast<float>((col * kMmPixPerByte) + bit),
                           .y = static_cast<float>(row)};
          points.push_back(pixel);
        }
      }
    }
  }

  return points;
}
}  // namespace space_invaders_vram_decoder
