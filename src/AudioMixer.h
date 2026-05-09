#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <cstdint>
#include <string>

namespace audio {

class Mixer {
 public:
  Mixer() { Load(); }
  ~Mixer() { Quit(); }

  // Pass in Reg A (accumulator) bits for OUT port 3
  void SetOut3(uint8_t accumulator_bits);

  // Pass in Reg A (accumulator) bits for OUT port 5
  void SetOut5(uint8_t accumulator_bits);

 private:
  enum class SoundId : int {
    kBackground,
    kExplosion,
    kInvaderHit,
    kTorpedo,
    kUfoHit,
    kUfo
  };

  struct Channel {
    SDL_AudioStream* stream_ = nullptr;
    SDL_AudioSpec spec_{};
    uint8_t* wav_buf_ = nullptr;
    uint32_t wav_len_ = 0;
    bool looping_ = false;
    bool playing_ = false;
    uint32_t pos_ = 0;
  };

  struct AudioAction {
    enum class Action : int { kStop, kPlay, kContinue };
    Action action_;
    SoundId id_;
  };

  SDL_AudioDeviceID device_ = 0;
  SDL_AudioSpec device_spec_;
  std::array<Channel, 6> channels_{};
  uint8_t prev_port3_ = 0;
  uint8_t prev_port5_ = 0;

  void Load();

  static void SDLCALL LoopCallback(void* userdata, SDL_AudioStream* stream,
                                   int additional_amount, int total_amount);

  static std::array<AudioAction, 8> DecodePort3(uint8_t accumulator_bits,
                                                uint8_t prev_port);

  static std::array<AudioAction, 8> DecodePort5(uint8_t accumulator_bits,
                                                uint8_t prev_port);

  void Play(SoundId id);

  void Stop(SoundId id);

  void Quit();
};

}  // namespace audio