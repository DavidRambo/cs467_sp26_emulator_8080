#include "AudioMixer.h"

namespace Audio {
void Mixer::Load() {
  device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
  if (device == 0) {
    SDL_Log("Couldn't open audio device: %s", SDL_GetError());
    return;
  }

  SDL_GetAudioDeviceFormat(device, &deviceSpec, nullptr);

  struct SoundEffect {
    SoundId id;
    const char* file;
    bool looping;
  };

  constexpr SoundEffect sounds[]{
      // Sound ID           File                Looping
      {SoundId::Background, "background.wav",   true},
      {SoundId::Explosion,  "explosion.wav",    false},
      {SoundId::InvaderHit, "invader_hit.wav",  false},
      {SoundId::Torpedo,    "torpedo.wav",      false},
      {SoundId::UfoHit,     "ufo_hit.wav",      false},
      {SoundId::Ufo,        "ufo.wav",          true},
  };

  for (const auto& sound : sounds) {
    Channel& chan = channels[static_cast<std::size_t>(sound.id)];
    SDL_LoadWAV(sound.file, &chan.spec, &chan.wav_buf, &chan.wav_len);
    chan.looping = sound.looping;
    chan.stream = SDL_CreateAudioStream(&chan.spec, &deviceSpec);
    SDL_BindAudioStream(device, chan.stream);
  }
}

void Mixer::Play(SoundId id) {
  Channel& chan = channels[static_cast<std::size_t>(id)];
  if (!chan.stream) return;

  SDL_ClearAudioStream(chan.stream);

  if (chan.looping) {
    // TODO: use a callback function as a parameter to loop Wav file
    SDL_SetAudioStreamGetCallback(chan.stream, nullptr, nullptr);
  } else {
    SDL_SetAudioStreamGetCallback(chan.stream, nullptr, nullptr);
  }

  SDL_PutAudioStreamData(chan.stream, chan.wav_buf,
                         static_cast<int>(chan.wav_len));
  chan.playing = true;
}

void Mixer::Stop(SoundId id) {
  Channel& chan = channels[static_cast<std::size_t>(id)];
  if (!chan.stream || !chan.playing) return;

  SDL_SetAudioStreamGetCallback(chan.stream, nullptr, nullptr);
  SDL_ClearAudioStream(chan.stream);
  chan.playing = false;
}

void Mixer::SetOut3(uint8_t accumulatorBits) {
  uint8_t changed = accumulatorBits ^ prevPort3;

  // Bit 1
  if ((changed & 0x01) && (accumulatorBits & 0x01)) Play(SoundId::Background);

  // Bit 2
  if ((changed & 0x02) && (accumulatorBits & 0x02)) Play(SoundId::Explosion);

  // Bit 3
  if ((changed & 0x04) && (accumulatorBits & 0x04)) Play(SoundId::InvaderHit);

  // Bit 4
  if ((changed & 0x08) && (accumulatorBits & 0x08)) Play(SoundId::Background);

  // Bit 5
  if ((changed & 0x10) && (accumulatorBits & 0x10)) Play(SoundId::Explosion);

  // Bit 6
  if ((changed & 0x20) && (accumulatorBits & 0x20)) Play(SoundId::InvaderHit);

  // Bit 7
  if ((changed & 0x40) && (accumulatorBits & 0x40)) Play(SoundId::Background);

  // Bit 8
  if ((changed & 0x80) && (accumulatorBits & 0x80)) Play(SoundId::Explosion);
}

void Mixer::SetOut5(uint8_t accumulatorBits) {
  uint8_t changed = accumulatorBits ^ prevPort5;

  // Bit 1
  if ((changed & 0x01) && (accumulatorBits & 0x01)) Play(SoundId::Torpedo);

  // Bit 2
  if ((changed & 0x02) && (accumulatorBits & 0x02)) Play(SoundId::UfoHit);

  // Bit 3
  if ((changed & 0x04) && (accumulatorBits & 0x04)) Play(SoundId::Ufo);

  // Bit 4
  if ((changed & 0x08) && (accumulatorBits & 0x08)) Play(SoundId::Torpedo);

  // Bit 5
  if ((changed & 0x10) && (accumulatorBits & 0x10)) Play(SoundId::UfoHit);

  // Bit 6
  if ((changed & 0x20) && (accumulatorBits & 0x20)) Play(SoundId::Ufo);

  // Bit 7
  if ((changed & 0x40) && (accumulatorBits & 0x40)) Play(SoundId::Torpedo);

  // Bit 8
  if ((changed & 0x80) && (accumulatorBits & 0x80)) Play(SoundId::UfoHit);
}
}  // namespace Audio
