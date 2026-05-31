#include "../src/AudioMixer.h"
#include "doctest.h"

using Action = audio::Mixer::AudioAction::Action;

TEST_CASE("Testing Audio Mixer") {
  audio::Mixer mixer = audio::Mixer();

  SUBCASE("Play Audio OUT3 Bit 0, Bit 0 Not Previously Set") {
    uint8_t bit_select = 0x01;
    auto actions = mixer.SetOut3(bit_select);
    CHECK_EQ(actions[0].action_, Action::kPlay);
    CHECK_EQ(actions[1].action_, Action::kContinue);
    CHECK_EQ(actions[2].action_, Action::kContinue);
    CHECK_EQ(actions[3].action_, Action::kContinue);
    CHECK_EQ(actions[4].action_, Action::kContinue);
    CHECK_EQ(actions[5].action_, Action::kContinue);
    CHECK_EQ(actions[6].action_, Action::kContinue);
    CHECK_EQ(actions[7].action_, Action::kContinue);
  }

  SUBCASE("Play Audio OUT3 Bit 1, Bit 1 Previously Set") {
    uint8_t bit_select = 0x02;
    mixer.SetOut3(bit_select);
    auto actions = mixer.SetOut3(bit_select);
    CHECK_EQ(actions[0].action_, Action::kContinue);
    CHECK_EQ(actions[1].action_, Action::kContinue);
    CHECK_EQ(actions[2].action_, Action::kContinue);
    CHECK_EQ(actions[3].action_, Action::kContinue);
    CHECK_EQ(actions[4].action_, Action::kContinue);
    CHECK_EQ(actions[5].action_, Action::kContinue);
    CHECK_EQ(actions[6].action_, Action::kContinue);
    CHECK_EQ(actions[7].action_, Action::kContinue);
  }

  SUBCASE("Stop Audio OUT3 Bit 7, Bit 7 Previously Set") {
    uint8_t bit_select = 0x80;
    mixer.SetOut3(bit_select);
    bit_select = 0x00;
    auto actions = mixer.SetOut3(bit_select);
    CHECK_EQ(actions[0].action_, Action::kContinue);
    CHECK_EQ(actions[1].action_, Action::kContinue);
    CHECK_EQ(actions[2].action_, Action::kContinue);
    CHECK_EQ(actions[3].action_, Action::kContinue);
    CHECK_EQ(actions[4].action_, Action::kContinue);
    CHECK_EQ(actions[5].action_, Action::kContinue);
    CHECK_EQ(actions[6].action_, Action::kContinue);
    CHECK_EQ(actions[7].action_, Action::kStop);
  }

  SUBCASE("Play Audio OUT3 Bit 1 and 5, Bit 1 Previously Set, Bit 5 Not") {
    uint8_t bit_select = 0x02;
    mixer.SetOut3(bit_select);
    bit_select = 0x22;
    auto actions = mixer.SetOut3(bit_select);
    CHECK_EQ(actions[0].action_, Action::kContinue);
    CHECK_EQ(actions[1].action_, Action::kContinue);
    CHECK_EQ(actions[2].action_, Action::kContinue);
    CHECK_EQ(actions[3].action_, Action::kContinue);
    CHECK_EQ(actions[4].action_, Action::kContinue);
    CHECK_EQ(actions[5].action_, Action::kPlay);
    CHECK_EQ(actions[6].action_, Action::kContinue);
    CHECK_EQ(actions[7].action_, Action::kContinue);
  }

  SUBCASE("Play Audio OUT5 Bit 0, Bit 0 Not Previously Set") {
    uint8_t bit_select = 0x01;
    auto actions = mixer.SetOut5(bit_select);
    CHECK_EQ(actions[0].action_, Action::kPlay);
    CHECK_EQ(actions[1].action_, Action::kContinue);
    CHECK_EQ(actions[2].action_, Action::kContinue);
    CHECK_EQ(actions[3].action_, Action::kContinue);
    CHECK_EQ(actions[4].action_, Action::kContinue);
    CHECK_EQ(actions[5].action_, Action::kContinue);
    CHECK_EQ(actions[6].action_, Action::kContinue);
    CHECK_EQ(actions[7].action_, Action::kContinue);
  }

  SUBCASE("Play Audio OUT5 Bit 1, Bit 1 Previously Set") {
    uint8_t bit_select = 0x02;
    mixer.SetOut5(bit_select);
    auto actions = mixer.SetOut5(bit_select);
    CHECK_EQ(actions[0].action_, Action::kContinue);
    CHECK_EQ(actions[1].action_, Action::kContinue);
    CHECK_EQ(actions[2].action_, Action::kContinue);
    CHECK_EQ(actions[3].action_, Action::kContinue);
    CHECK_EQ(actions[4].action_, Action::kContinue);
    CHECK_EQ(actions[5].action_, Action::kContinue);
    CHECK_EQ(actions[6].action_, Action::kContinue);
    CHECK_EQ(actions[7].action_, Action::kContinue);
  }

  SUBCASE("Stop Audio OUT5 Bit 7, Bit 7 Previously Set") {
    uint8_t bit_select = 0x80;
    mixer.SetOut5(bit_select);
    bit_select = 0x00;
    auto actions = mixer.SetOut5(bit_select);
    CHECK_EQ(actions[0].action_, Action::kContinue);
    CHECK_EQ(actions[1].action_, Action::kContinue);
    CHECK_EQ(actions[2].action_, Action::kContinue);
    CHECK_EQ(actions[3].action_, Action::kContinue);
    CHECK_EQ(actions[4].action_, Action::kContinue);
    CHECK_EQ(actions[5].action_, Action::kContinue);
    CHECK_EQ(actions[6].action_, Action::kContinue);
    CHECK_EQ(actions[7].action_, Action::kStop);
  }
}
