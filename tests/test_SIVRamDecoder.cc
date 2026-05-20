#include "../src/SpaceInvadersVRamDecoder.h"
#include "doctest.h"

TEST_CASE("Sample test") { CHECK(1); }

TEST_CASE("Space Invaders Pixel Decoding") {
  SUBCASE("All pixel values set") {
    std::vector<char> buffer(7168, 0xFF);
    REQUIRE(buffer.size() == 7168);

    std::vector<SDL_FPoint> top_pixels =
        space_invaders_vram_decoder::DecodeTopPixels(buffer.data());

    std::vector<SDL_FPoint> bottom_pixels =
        space_invaders_vram_decoder::DecodeTopPixels(buffer.data());

    CHECK(top_pixels.size() == 28672);
    CHECK(bottom_pixels.size() == 28672);
  }

  SUBCASE("All pixel values cleared") {
    std::vector<char> buffer(7168, 0x00);
    REQUIRE(buffer.size() == 7168);

    std::vector<SDL_FPoint> top_pixels =
        space_invaders_vram_decoder::DecodeTopPixels(buffer.data());

    std::vector<SDL_FPoint> bottom_pixels =
        space_invaders_vram_decoder::DecodeTopPixels(buffer.data());

    CHECK(top_pixels.size() == 0);
    CHECK(bottom_pixels.size() == 0);
  }

  SUBCASE("Three pixels set in each array half") {
    std::vector<char> buffer(7168, 0x00);

    buffer[0] = 0xFF;
    buffer[1791] = 0xFF;
    buffer[3583] = 0xFF;
    buffer[3584] = 0xFF;
    buffer[5375] = 0xFF;
    buffer[7167] = 0xFF;

    REQUIRE(buffer.size() == 7168);
    REQUIRE(buffer[0] == (char)0xFF);
    REQUIRE(buffer[1791] == (char)0xFF);
    REQUIRE(buffer[3583] == (char)0xFF);
    REQUIRE(buffer[3584] == (char)0xFF);
    REQUIRE(buffer[5375] == (char)0xFF);
    REQUIRE(buffer[7167] == (char)0xFF);

    std::vector<SDL_FPoint> top_pixels =
        space_invaders_vram_decoder::DecodeTopPixels(buffer.data());

    std::vector<SDL_FPoint> bottom_pixels =
        space_invaders_vram_decoder::DecodeBottomPixels(buffer.data());

    CHECK(top_pixels.size() == 24);
    CHECK(bottom_pixels.size() == 24);

    CHECK(bottom_pixels[0].x == 0);
    CHECK(bottom_pixels[0].y == 0);
    CHECK(bottom_pixels[7].x == 7);
    CHECK(bottom_pixels[7].y == 0);

    CHECK(top_pixels[0].x == 0);
    CHECK(top_pixels[0].y == 112);
    CHECK(top_pixels[7].x == 7);
    CHECK(top_pixels[7].y == 112);

    CHECK(bottom_pixels[8].x == 248);
    CHECK(bottom_pixels[8].y == 55);
    CHECK(bottom_pixels[15].x == 255);
    CHECK(bottom_pixels[15].y == 55);

    CHECK(top_pixels[8].x == 248);
    CHECK(top_pixels[8].y == 167);
    CHECK(top_pixels[15].x == 255);
    CHECK(top_pixels[15].y == 167);

    CHECK(bottom_pixels[16].x == 248);
    CHECK(bottom_pixels[16].y == 111);
    CHECK(bottom_pixels[23].x == 255);
    CHECK(bottom_pixels[23].y == 111);

    CHECK(top_pixels[16].x == 248);
    CHECK(top_pixels[16].y == 223);
    CHECK(top_pixels[23].x == 255);
    CHECK(top_pixels[23].y == 223);
  }
}
