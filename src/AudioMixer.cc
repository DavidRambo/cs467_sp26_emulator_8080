#include "AudioMixer.h"

#include <SDL3/SDL.h>

#include <array>
#include <cstdint>

namespace audio {
void Mixer::Load() {
  device_ = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
  if (device_ == 0) {
    SDL_Log("Couldn't open audio device: %s", SDL_GetError());
    return;
  }

  // Capture the hardware format so each stream can resample/convert as needed.
  SDL_GetAudioDeviceFormat(device_, &device_spec_, nullptr);

  struct SoundEffect {
    SoundId id_;
    const char* file_;
    bool looping_;
  };

  constexpr std::array kSounds{
      // clang-format off
    //          Sound ID              File                Looping
    SoundEffect{.id_=SoundId::kBackground, .file_="assets/audio/background.wav",   .looping_=true},
    SoundEffect{.id_=SoundId::kExplosion,  .file_="assets/audio/explosion.wav",    .looping_=false},
    SoundEffect{.id_=SoundId::kInvaderHit, .file_="assets/audio/invader_hit.wav",  .looping_=false},
    SoundEffect{.id_=SoundId::kTorpedo,    .file_="assets/audio/torpedo.wav",      .looping_=false},
    SoundEffect{.id_=SoundId::kUfoHit,     .file_="assets/audio/ufo_hit.wav",      .looping_=false},
    SoundEffect{.id_=SoundId::kUfo,        .file_="assets/audio/ufo.wav",          .looping_=true},
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
  // Feed chunks until SDL's request is satisfied, wrapping pos_ at the end of
  // the buffer to achieve seamless looping without re-queuing the whole file.
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

  // Discard queued audio so the sound restarts cleanly from the beginning.
  SDL_ClearAudioStream(chan.stream_);
  chan.pos_ = 0;

  if (chan.looping_) {
    // Pass a callback function to continuosly loop sound effect.
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

  // Remove callback before flushing to prevent it from feeding data into a
  // stream that is being cleared.
  SDL_SetAudioStreamGetCallback(chan.stream_, nullptr, nullptr);
  SDL_ClearAudioStream(chan.stream_);
  chan.pos_ = 0;
  chan.playing_ = false;
}

std::array<Mixer::AudioAction, 8> Mixer::DecodePort(
    uint8_t accumulator_bits, uint8_t prev_bits,
    const std::array<SoundId, 8>& soundMap) {
  // XOR isolates bits that changed between the previous and current port value.
  uint8_t changed = accumulator_bits ^ prev_bits;
  std::array<AudioAction, 8> actions;

  for (int i = 0; i < 8; ++i) {
    uint8_t bit = 1 << i;
    // Use bit as mask to determine which action to select for each accumulator
    // bit.
    if (changed & bit) {
      (accumulator_bits & bit)
          ? actions[i] = {.action_ = AudioAction::Action::kPlay,
                          .id_ = soundMap[i]}
          : actions[i] = {.action_ = AudioAction::Action::kStop,
                          .id_ = soundMap[i]};
    } else {
      actions[i] = {.action_ = AudioAction::Action::kContinue,
                    .id_ = soundMap[i]};
    }
  }
  return actions;
}

std::array<Mixer::AudioAction, 8> Mixer::ApplyPort(
    uint8_t accumulator_bits, uint8_t prev_bits,
    const std::array<SoundId, 8>& soundMap) {
  auto actions = DecodePort(accumulator_bits, prev_bits, soundMap);
  for (const auto& action : actions) {
    switch (action.action_) {
      case AudioAction::Action::kPlay:
        Play(action.id_);
        break;
      case AudioAction::Action::kStop:
        Stop(action.id_);
        break;
      case AudioAction::Action::kContinue:
      default:
        break;
    }
  }
  return actions;
}

std::array<Mixer::AudioAction, 8> Mixer::SetOut3(uint8_t accumulator_bits) {
  auto actions = ApplyPort(accumulator_bits, prev_port3_, kSoundMap);
  prev_port3_ = accumulator_bits;
  return actions;
}

std::array<Mixer::AudioAction, 8> Mixer::SetOut5(uint8_t accumulator_bits) {
  auto actions = ApplyPort(accumulator_bits, prev_port5_, kSoundMap);
  prev_port5_ = accumulator_bits;
  return actions;
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
