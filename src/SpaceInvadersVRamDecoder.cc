#include "SpaceInvadersVRamDecoder.h"

#include <SDL3/SDL.h>

#include <vector>

namespace space_invaders_vram_decoder {
constexpr int kMmCols = 32;
constexpr int kMmRows = 224;
constexpr int kMmPixPerByte = 8;

void Pixels::clear() {
  white_points.clear();
  green_points.clear();
}

// Traverses the span video_data, which provides a view into the video ram
// segment of the 8080's memory (0x2400-0x3FFF), creating points for every bit
// that is set, and pushing them onto the provided points vector. The points
// vector will be used by the GameWindow class object to update the screen.
//
// A note of explanation about how the bits in video_data map onto the SDL3
// window. The Space Invaders display was physically a 256-pixel-wide by
// 224-pixel-high screen, which was rotated counter-clockwise 90 degrees. Thus,
// physically, memory address 0x2400, bit 0 would map to the top-left pixel
// (0,0). Rotated, this would be perceived as the bottom-left pixel by the
// player. Since we're using SDL3 to create a window that matches the rotated
// dimensions, this function maps the video data to the rotated pixels.
// The memory can be represented accordingly, where the bracketed numbers are
// the bits:
//
// 0x241F[7] ...           0x3FFF[7]
// ...
// 0x2402[1] ...
// 0x2402[0] 0x2422[0] ... ...
// 0x2401[7] 0x2421[7] ... 0x3FE1[7]
// ...
// 0x2400[2] 0x2420[2] ... 0x3FE0[2]
// 0x2400[1] 0x2420[1] ... 0x3FE0[1]
// 0x2400[0] 0x2420[0] ... 0x3FE0[0]
//
// Note that each column's byte address jumps by 0x20. This means there are 224
// columns: ((0x3FE0 - 0x2400) / 0x20) = 223d (or 0xDF), and include the zeroth
// column to get 224.
//
// There are 256 rows of pixels: 32 bytes * 8 bits. If each "row" in the outer
// for lop corresponds to a byte in memory, then there are 32 such rows.
//
// The iteration goes through the game window pixels in video_bata byte/bit
// order. To match the window point cooridnates, the loops begin at bottom left,
// bottom of column to top, and then move over to the right to the next column
// at the bottom of the screen. The outer loop goes left to right, the inner
// from bottom to top. A variable, `v_idx`, counts the current bit, which is
// used to get the current address in video_data.
void DecodePixels(Pixels& pixels, std::span<unsigned char, 7168> video_data) {
  // v_idx will increment for each bit, from 0 to (256*224 - 1).
  uint16_t v_idx{0};
  pixels.clear();

  // Start at bottom left coordinate.
  for (int col = 0; col < 224; col++) {
    for (int row = 32; row > 0; row--) {
      for (int bit{0}; bit < 8; bit++) {
        // Calculate which byte the loop is on by reducing the pixel number by a
        // factor of 8.
        uint16_t video_byte = v_idx / 8;

        if (video_data[video_byte] & (0x01 << bit)) {
          int y = (row * 8) - 1 - bit;
          if (y > 190) {
            pixels.green_points.push_back(SDL_FPoint{
                .x = static_cast<float>(col), .y = static_cast<float>(y)});
          } else {
            pixels.white_points.push_back(SDL_FPoint{
                .x = static_cast<float>(col), .y = static_cast<float>(y)});
          }
        }

        v_idx++;
      }
    }
  }
}

}  // namespace space_invaders_vram_decoder
