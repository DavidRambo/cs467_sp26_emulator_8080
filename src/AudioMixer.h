#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <cstdint>
#include <string>

namespace Audio {

class Mixer {
 public:
  Mixer() { Load(); }

  // Pass in Reg A (accumulator) bits for OUT port 3
  void SetOut3(uint8_t accumulatorBits);

  // Pass in Reg A (accumulator) bits for OUT port 5
  void SetOut5(uint8_t accumulatorBits);

 private:
  enum class SoundId : int {
    Background,
    Explosion,
    InvaderHit,
    Torpedo,
    UfoHit,
    Ufo
  };

  struct Channel {
    SDL_AudioStream* stream = nullptr;
    SDL_AudioSpec spec{};
    Uint8* wav_buf = nullptr;
    Uint32 wav_len = 0;
    bool looping = false;
    bool playing = false;
  };

  SDL_AudioDeviceID device = 0;
  SDL_AudioSpec deviceSpec;
  std::array<Channel, 6> channels{};
  uint8_t prevPort3 = 0;
  uint8_t prevPort5 = 0;

  void Load();

  void Play(SoundId id);

  void Stop(SoundId id);
};

}  // namespace Audio