#include "AudioMixer.h"

#include <SDL3/SDL.h>

#include <array>
#include <cstdint>
#include <map>

namespace audio {
void Mixer::Load() {
  device_ = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
  if (device_ == 0) {
    SDL_Log("Couldn't open audio device: %s", SDL_GetError());
    return;
  }

  SDL_GetAudioDeviceFormat(device_, &device_spec_, nullptr);

  struct SoundEffect {
    SoundId id_;
    const char* file_;
    bool looping_;
  };

  constexpr std::array kSounds{
    // clang-format off
    //          Sound ID              File                Looping
    SoundEffect{SoundId::kBackground, "background.wav",   true},
    SoundEffect{SoundId::kExplosion,  "explosion.wav",    false},
    SoundEffect{SoundId::kInvaderHit, "invader_hit.wav",  false},
    SoundEffect{SoundId::kTorpedo,    "torpedo.wav",      false},
    SoundEffect{SoundId::kUfoHit,     "ufo_hit.wav",      false},
    SoundEffect{SoundId::kUfo,        "ufo.wav",          true},
    // clang-format on
  };

  for (const auto& sound : kSounds) {
    Channel& chan = channels_[static_cast<std::size_t>(sound.id_)];
    SDL_LoadWAV(sound.file_, &chan.spec_, &chan.wav_buf_, &chan.wav_len_);
    chan.looping_ = sound.looping_;
    chan.stream_ = SDL_CreateAudioStream(&chan.spec_, &device_spec_);
    SDL_BindAudioStream(device_, chan.stream_);
  }
}

void SDLCALL Mixer::LoopCallback(void* userdata, SDL_AudioStream* stream,
                                 int additional_amount, int /*total_amount*/) {
  auto* chan = static_cast<Channel*>(userdata);
  while (additional_amount > 0) {
    int remaining =
        static_cast<int>(chan->wav_len_) - static_cast<int>(chan->pos_);
    int chunk = std::min(additional_amount, remaining);
    SDL_PutAudioStreamData(stream, chan->wav_buf_ + chan->pos_, chunk);
    chan->pos_ += static_cast<uint32_t>(chunk);
    additional_amount -= chunk;
    if (chan->pos_ >= chan->wav_len_) {
      chan->pos_ = 0;
    }
  }
}

void Mixer::Play(SoundId id) {
  Channel& chan = channels_[static_cast<std::size_t>(id)];
  if (!chan.stream_) {
    return;
  }

  SDL_ClearAudioStream(chan.stream_);
  chan.pos_ = 0;

  if (chan.looping_) {
    SDL_SetAudioStreamGetCallback(chan.stream_, LoopCallback, &chan);
  } else {
    SDL_SetAudioStreamGetCallback(chan.stream_, nullptr, nullptr);
  }

  SDL_PutAudioStreamData(chan.stream_, chan.wav_buf_,
                         static_cast<int>(chan.wav_len_));
  chan.playing_ = true;
}

void Mixer::Stop(SoundId id) {
  Channel& chan = channels_[static_cast<std::size_t>(id)];
  if (!chan.stream_ || !chan.playing_) {
    return;
  }

  SDL_SetAudioStreamGetCallback(chan.stream_, nullptr, nullptr);
  SDL_ClearAudioStream(chan.stream_);
  chan.pos_ = 0;
  chan.playing_ = false;
}

std::array<Mixer::AudioAction, 8> Mixer::DecodePort3(uint8_t accumulator_bits,
                                                     uint8_t prev_port) {
  std::map<int, SoundId> kSoundsPort3{
    // clang-format off
  // Bit    Sound
    {1,     SoundId::kBackground},
    {2,     SoundId::kExplosion},
    {4,     SoundId::kInvaderHit},
    {8,     SoundId::kBackground},
    {16,    SoundId::kExplosion},
    {32,    SoundId::kInvaderHit},
    {64,    SoundId::kBackground},
    {128,   SoundId::kExplosion},
    // clang-format on
  };

  uint8_t changed = accumulator_bits ^ prev_port;
  std::array<AudioAction, 8> actions;

  for (int i = 0; i < 8; ++i) {
    uint8_t bit = 1 << i;
    if (changed & bit) {
      (accumulator_bits & bit)
          ? actions[i] = {AudioAction::Action::kPlay, kSoundsPort3[bit]}
          : actions[i] = {AudioAction::Action::kStop, kSoundsPort3[bit]};
    } else {
      actions[i] = {AudioAction::Action::kContinue, kSoundsPort3[bit]};
    }
  }
  return actions;
}

std::array<Mixer::AudioAction, 8> Mixer::DecodePort5(uint8_t accumulator_bits,
                                                     uint8_t prev_port) {
  std::map<int, SoundId> kSoundsPort5{
    // clang-format off
  // Bit    Sound
    {1,     SoundId::kTorpedo},
    {2,     SoundId::kUfoHit},
    {4,     SoundId::kUfo},
    {8,     SoundId::kTorpedo},
    {16,    SoundId::kUfoHit},
    {32,    SoundId::kUfo},
    {64,    SoundId::kTorpedo},
    {128,   SoundId::kUfoHit},
    // clang-format on
  };

  uint8_t changed = accumulator_bits ^ prev_port;
  std::array<AudioAction, 8> actions;

  for (int i = 0; i < 8; ++i) {
    uint8_t bit = 1 << i;
    if (changed & bit) {
      (accumulator_bits & bit)
          ? actions[i] = {AudioAction::Action::kPlay, kSoundsPort5[bit]}
          : actions[i] = {AudioAction::Action::kStop, kSoundsPort5[bit]};
    } else {
      actions[i] = {AudioAction::Action::kContinue, kSoundsPort5[bit]};
    }
  }
  return actions;
}

void Mixer::SetOut3(uint8_t accumulator_bits) {
  for (const auto& action : DecodePort3(accumulator_bits, prev_port3_)) {
    switch (action.action_) {
      case AudioAction::Action::kPlay:
        Play(action.id_);
        break;
      case AudioAction::Action::kStop:
        Stop(action.id_);
        break;
      case AudioAction::Action::kContinue:
        break;
      default:
        break;
    }
  }
  prev_port3_ = accumulator_bits;
}

void Mixer::SetOut5(uint8_t accumulator_bits) {
  for (const auto& action : DecodePort5(accumulator_bits, prev_port5_)) {
    switch (action.action_) {
      case AudioAction::Action::kPlay:
        Play(action.id_);
        break;
      case AudioAction::Action::kStop:
        Stop(action.id_);
        break;
      case AudioAction::Action::kContinue:
        break;
      default:
        break;
    }
  }
  prev_port5_ = accumulator_bits;
}

void Mixer::Quit() {
  for (auto& chan : channels_) {
    if (chan.stream_) {
      SDL_UnbindAudioStream(chan.stream_);
      SDL_DestroyAudioStream(chan.stream_);
      chan.stream_ = nullptr;
    }
    if (chan.wav_buf_) {
      SDL_free(chan.wav_buf_);
      chan.wav_buf_ = nullptr;
    }
  }
  if (device_) {
    SDL_CloseAudioDevice(device_);
    device_ = 0;
  }
}
}  // namespace audio
