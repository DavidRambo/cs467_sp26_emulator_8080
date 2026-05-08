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

  SDL_GetAudioDeviceFormat(device_, &device_spec_, nullptr);

  struct SoundEffect {
    SoundId id_;
    const char* file_;
    bool looping_;
  };

  constexpr std::array kSounds {
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

void SDLCALL Mixer::LoopCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int /*total_amount*/)
{
  auto* chan    = static_cast<Channel*>(userdata);
  while (additional_amount > 0) {
    int remaining = static_cast<int>(chan->wav_len_) - static_cast<int>(chan->pos_);
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
  if (!chan.stream_) { return; }

  SDL_ClearAudioStream(chan.stream_);
  chan.pos_ = 0;

  if (chan.looping_) {
    // TODO(BrandonAG): use a callback function as a parameter to loop Wav file
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
  if (!chan.stream_ || !chan.playing_) { return; }

  SDL_SetAudioStreamGetCallback(chan.stream_, nullptr, nullptr);
  SDL_ClearAudioStream(chan.stream_);
  chan.pos_ = 0;
  chan.playing_ = false;
}

void Mixer::SetOut3(uint8_t accumulator_bits) {
  uint8_t changed = accumulator_bits ^ prev_port3_;
  prev_port3_ = accumulator_bits;

  // Bit 1
  if (changed & 0x01) {
    (accumulator_bits & 0x01) ? Play(SoundId::kBackground) : Stop(SoundId::kBackground);
  }

  // Bit 2
  if ((changed & 0x02) && (accumulator_bits & 0x02)) { Play(SoundId::kExplosion); }

  // Bit 3
  if ((changed & 0x04) && (accumulator_bits & 0x04)) { Play(SoundId::kInvaderHit); }

  // Bit 4
  if (changed & 0x08) {
    (accumulator_bits & 0x08) ? Play(SoundId::kBackground) : Stop(SoundId::kBackground);
  }

  // Bit 5
  if ((changed & 0x10) && (accumulator_bits & 0x10)) { Play(SoundId::kExplosion); }

  // Bit 6
  if ((changed & 0x20) && (accumulator_bits & 0x20)) { Play(SoundId::kInvaderHit); }

  // Bit 7
  if (changed & 0x40) {
    (accumulator_bits & 0x40) ? Play(SoundId::kBackground) : Stop(SoundId::kBackground);
  }

  // Bit 8
  if ((changed & 0x80) && (accumulator_bits & 0x80)) { Play(SoundId::kExplosion); }
}

void Mixer::SetOut5(uint8_t accumulator_bits) {
  uint8_t changed = accumulator_bits ^ prev_port5_;
  prev_port5_ = accumulator_bits;

  // Bit 1
  if ((changed & 0x01) && (accumulator_bits & 0x01)) { Play(SoundId::kTorpedo); }

  // Bit 2
  if ((changed & 0x02) && (accumulator_bits & 0x02)) { Play(SoundId::kUfoHit); }

  // Bit 3
  if (changed & 0x04) {
    (accumulator_bits & 0x04) ? Play(SoundId::kUfo) : Stop(SoundId::kUfo);
  }

  // Bit 4
  if ((changed & 0x08) && (accumulator_bits & 0x08)) { Play(SoundId::kTorpedo); }

  // Bit 5
  if ((changed & 0x10) && (accumulator_bits & 0x10)) { Play(SoundId::kUfoHit); }

  // Bit 6
  if (changed & 0x20) {
    (accumulator_bits & 0x20) ? Play(SoundId::kUfo) : Stop(SoundId::kUfo);
  }

  // Bit 7
  if ((changed & 0x40) && (accumulator_bits & 0x40)) { Play(SoundId::kTorpedo); }

  // Bit 8
  if ((changed & 0x80) && (accumulator_bits & 0x80)) { Play(SoundId::kUfoHit); }
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
