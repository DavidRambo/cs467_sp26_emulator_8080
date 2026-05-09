#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <cstdint>
#include <string>

namespace audio {

// Manages SDL3 audio playback for Space Invaders sound effects.
//
// Loads WAV files for each sound effect at construction, binds them to a
// single SDL audio device, and exposes the two OUT port interfaces used by
// the 8080 CPU to trigger and stop sounds. One Channel object per SoundId
// owns its SDL_AudioStream for the lifetime of the Mixer.
class Mixer {
 public:
  Mixer() { Load(); }
  ~Mixer() { Quit(); }

  enum class SoundId : int {
    kBackground,
    kExplosion,
    kInvaderHit,
    kTorpedo,
    kUfoHit,
    kUfo
  };

  // Carries the decoded intent for a single bit change from a port OUT value.
  struct AudioAction {
    // kContinue means the bit was unchanged; no playback change is needed.
    enum class Action : int { kStop, kPlay, kContinue };
    Action action_;
    SoundId id_;
  };

  // Interprets the accumulator value written to OUT port 3 and starts or stops
  // the corresponding sound effects. Each bit maps to a specific sound; a
  // rising edge starts playback and a falling edge stops it.
  std::array<AudioAction, 8> SetOut3(uint8_t accumulator_bits);

  // Interprets the accumulator value written to OUT port 5 and starts or stops
  // the corresponding sound effects. Each bit maps to a specific sound; a
  // rising edge starts playback and a falling edge stops it.
  std::array<AudioAction, 8> SetOut5(uint8_t accumulator_bits);

 private:
  // Holds all SDL resources and playback state for a single WAV sound effect.
  struct Channel {
    SDL_AudioStream* stream_ = nullptr;
    SDL_AudioSpec spec_{};     // WAV file's native format.
    uint8_t* wav_buf_ = nullptr;
    uint32_t wav_len_ = 0;
    bool looping_ = false;
    bool playing_ = false;
    uint32_t pos_ = 0;  // Current read position within wav_buf_ (bytes).
  };

  SDL_AudioDeviceID device_ = 0;  // Handle to the open SDL audio device.
  SDL_AudioSpec device_spec_;     // Hardware format used for stream conversion.
  std::array<Channel, 6> channels_;  // Array of all loaded sound effects.
  uint8_t prev_port3_ = 0;  // Last accumulator value seen on port 3.
  uint8_t prev_port5_ = 0;  // Last accumulator value seen on port 5.

  // Opens the default SDL audio device and loads all WAV files into channels_.
  void Load();

  // SDL_AudioStream callback that feeds data for looping channels.
  // Wraps pos_ back to 0 each time the buffer is exhausted so playback loops
  // indefinitely until Stop() is called.
  static void SDLCALL LoopCallback(void* userdata, SDL_AudioStream* stream,
                                   int additional_amount, int total_amount);

  // Compares accumulator_bits to prev_bits and returns one AudioAction per bit:
  // kPlay on a rising edge, kStop on a falling edge, kContinue if unchanged.
  static std::array<Mixer::AudioAction, 8> DecodePort(
      uint8_t accumulator_bits, uint8_t prev_bits,
      const std::array<SoundId, 8>& soundMap);

  // Decodes accumulator_bits relative to prev_bits using soundMap and calls
  // Play() or Stop() for each bit whose edge changed.
  std::array<Mixer::AudioAction, 8> ApplyPort(
      uint8_t accumulator_bits, uint8_t prev_bits,
      const std::array<SoundId, 8>& soundMap);

  // Rewinds and starts playback of the channel identified by id. If the
  // channel is configured for looping, installs LoopCallback; otherwise feeds
  // the entire buffer once.
  void Play(SoundId id);

  // Halts playback of the channel identified by id and resets its position.
  void Stop(SoundId id);

  // Unbinds and destroys all SDL streams, frees WAV buffers, and closes the
  // audio device.
  void Quit();
};

}  // namespace audio