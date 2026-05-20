#include "../src/SpaceInvadersVRamDecoder.h"
#include "doctest.h"

TEST_CASE("Sample test") { CHECK(1); }

TEST_CASE("Space Invaders Pixel Decoding") {
  SUBCASE("All pixel values set") {
    std::vector<char> buffer(7168, 0xFF);
    REQUIRE(buffer.size() == 7168);

    std::vector<SDL_FPoint> topPixels =
        SpaceInvadersVRamDecoder::DecodeTopPixels(buffer.data());

    std::vector<SDL_FPoint> bottomPixels =
        SpaceInvadersVRamDecoder::DecodeTopPixels(buffer.data());

    CHECK(topPixels.size() == 28672);
    CHECK(bottomPixels.size() == 28672);
  }

  SUBCASE("All pixel values cleared") {
    std::vector<char> buffer(7168, 0x00);
    REQUIRE(buffer.size() == 7168);

    std::vector<SDL_FPoint> topPixels =
        SpaceInvadersVRamDecoder::DecodeTopPixels(buffer.data());

    std::vector<SDL_FPoint> bottomPixels =
        SpaceInvadersVRamDecoder::DecodeTopPixels(buffer.data());

    CHECK(topPixels.size() == 0);
    CHECK(bottomPixels.size() == 0);
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

    std::vector<SDL_FPoint> topPixels =
        SpaceInvadersVRamDecoder::DecodeTopPixels(buffer.data());

    std::vector<SDL_FPoint> bottomPixels =
        SpaceInvadersVRamDecoder::DecodeBottomPixels(buffer.data());

    CHECK(topPixels.size() == 24);
    CHECK(bottomPixels.size() == 24);

    CHECK(bottomPixels[0].x == 0);
    CHECK(bottomPixels[0].y == 0);
    CHECK(bottomPixels[7].x == 7);
    CHECK(bottomPixels[7].y == 0);

    CHECK(topPixels[0].x == 0);
    CHECK(topPixels[0].y == 112);
    CHECK(topPixels[7].x == 7);
    CHECK(topPixels[7].y == 112);

    CHECK(bottomPixels[8].x == 248);
    CHECK(bottomPixels[8].y == 55);
    CHECK(bottomPixels[15].x == 255);
    CHECK(bottomPixels[15].y == 55);

    CHECK(topPixels[8].x == 248);
    CHECK(topPixels[8].y == 167);
    CHECK(topPixels[15].x == 255);
    CHECK(topPixels[15].y == 167);

    CHECK(bottomPixels[16].x == 248);
    CHECK(bottomPixels[16].y == 111);
    CHECK(bottomPixels[23].x == 255);
    CHECK(bottomPixels[23].y == 111);

    CHECK(topPixels[16].x == 248);
    CHECK(topPixels[16].y == 223);
    CHECK(topPixels[23].x == 255);
    CHECK(topPixels[23].y == 223);
  }
}
