#include "SoundManager.h"

#include <SDL3/SDL.h>
#include <memory>
#include <phosg/Encoding.hh>
#include <unordered_map>

#include "Types-CC.hpp"

// Min frequency allowed by SDL_SetAudioStreamFormat
#define SDL_MIN_FREQ 4000
// Size in bytes of RIFF WAV header
#define RIFF_WAV_HEADER_BYTES 44

typedef struct Sound {
  uint8_t channels;
  uint32_t freq;
  const void* data;
  uint32_t len;
} Sound;

typedef struct {
  be_uint32_t riff_magic; // 0x52494646 ('RIFF')
  le_uint32_t file_size; // size of file - 8
  be_uint32_t wave_magic; // 0x57415645

  be_uint32_t fmt_magic; // 0x666d7420 ('fmt ')
  le_uint32_t fmt_size; // 16
  le_uint16_t format; // 1 = PCM
  le_uint16_t num_channels;
  le_uint32_t sample_rate;
  le_uint32_t byte_rate; // num_channels * sample_rate * bits_per_sample / 8
  le_uint16_t block_align; // num_channels * bits_per_sample / 8
  le_uint16_t bits_per_sample;

  union {
    struct {
      be_uint32_t smpl_magic;
      le_uint32_t smpl_size;
      le_uint32_t manufacturer;
      le_uint32_t product;
      le_uint32_t sample_period;
      le_uint32_t base_note;
      le_uint32_t pitch_fraction;
      le_uint32_t smpte_format;
      le_uint32_t smpte_offset;
      le_uint32_t num_loops; // = 1
      le_uint32_t sampler_data;

      le_uint32_t loop_cue_point_id; // Can be zero? We'll only have at most one loop in this context
      le_uint32_t loop_type; // 0 = normal, 1 = ping-pong, 2 = reverse
      le_uint32_t loop_start; // Start and end are byte offsets into the wave data, not sample indexes
      le_uint32_t loop_end;
      le_uint32_t loop_fraction; // Fraction of a sample to loop (0)
      le_uint32_t loop_play_count; // 0 = loop forever

      be_uint32_t data_magic; // 0x64617461 ('data')
      le_uint32_t data_size; // num_samples * num_channels * bits_per_sample / 8
      uint8_t data[0];
    } __attribute__((packed)) with;

    struct {
      be_uint32_t data_magic; // 0x64617461 ('data')
      le_uint32_t data_size; // num_samples * num_channels * bits_per_sample / 8
      uint8_t data[0];
    } __attribute__((packed)) without;
  } __attribute__((packed)) loop;

  bool has_loop() const {
    return (this->loop.with.smpl_magic == 0x736D706C);
  }

  size_t size() const {
    if (this->has_loop()) {
      return sizeof(*this);
    } else {
      return sizeof(*this) - sizeof(this->loop.with) + sizeof(this->loop.without);
    }
  }
} WaveFileHeader;

class SoundManager {
public:
  SoundManager() = default;
  ~SoundManager();

  void initialize(void);

  bool is_sound_registered(int16_t id);
  void register_sound(int16_t id, Sound sound);
  void play_sound(SDL_AudioStream* sdlAudioStream, int16_t id);
  void bind_audio_stream(SDL_AudioStream* sdlAudioStream);

private:
  SDL_AudioDeviceID audioDeviceID;
  std::unordered_map<int16_t, Sound> library;
};

void SoundManager::initialize(void) {
  if (this->audioDeviceID > 0) {
    return;
  }

  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    SDL_Log("Could not initialize audio subsystem: %s\n", SDL_GetError());
  } else {
    this->audioDeviceID = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    SDL_Log("Using device: %s ", SDL_GetAudioDeviceName(this->audioDeviceID));
    if (this->audioDeviceID == 0) {
      SDL_Log("Failed to open audio: %s", SDL_GetError());
    }

    SDL_Log("Audio device paused: %d", SDL_AudioDevicePaused(this->audioDeviceID));
  }
}

SoundManager::~SoundManager() {
  SDL_CloseAudioDevice(this->audioDeviceID);
}

void SoundManager::register_sound(int16_t id, Sound sound) {
  this->library.insert({id, sound});
};

bool SoundManager::is_sound_registered(int16_t id) {
  return this->library.count(id) > 0;
}

void SoundManager::play_sound(SDL_AudioStream* sdlAudioStream, int16_t id) {
  if (!this->is_sound_registered(id)) {
    return;
  }
  Sound sound = this->library.at(id);
  SDL_AudioSpec spec;

  SDL_IOStream* wav = SDL_IOFromConstMem(sound.data, sound.len);
  if (wav == NULL) {
    SDL_Log("Could not load WAV data id %d: %s", id, SDL_GetError());
    return;
  }

  uint8_t* audio_buf;
  uint32_t audio_len;

  if (SDL_LoadWAV_IO(wav, SDL_TRUE, &spec, &audio_buf, &audio_len) < 0) {
    SDL_Log("Could not load WAV data id %d: %s", id, SDL_GetError());
    return;
  }

  spec.freq = std::max<int>(spec.freq, SDL_MIN_FREQ);

  if (SDL_SetAudioStreamFormat(sdlAudioStream, &spec, NULL) < 0) {
    SDL_Log("Could not set audio stream format id %d: %s", id, SDL_GetError());
    return;
  }
  if (SDL_PutAudioStreamData(sdlAudioStream, audio_buf, audio_len) < 0) {
    SDL_Log("Could not put audio stream data id %d: %s", id, SDL_GetError());
    return;
  }
}

void SoundManager::bind_audio_stream(SDL_AudioStream* sdlAudioStream) {
  SDL_BindAudioStream(this->audioDeviceID, sdlAudioStream);
}

static SoundManager sm;

SDL_AudioStream* SoundManager_new_audio_stream(void) {
  sm.initialize();
  SDL_AudioSpec spec;
  spec.format = SDL_AUDIO_U8;
  spec.channels = 1;
  spec.freq = SDL_MIN_FREQ;
  SDL_AudioStream* sdlAudioStream = SDL_CreateAudioStream(&spec, &spec);
  if (sdlAudioStream == NULL) {
    SDL_Log("Could not create audio stream: %s\n", SDL_GetError());
  }

  sm.bind_audio_stream(sdlAudioStream);

  SDL_Log("Audio stream device: %d", SDL_GetAudioStreamDevice(sdlAudioStream));

  return sdlAudioStream;
}

uint32_t upsample(const uint8_t* src, void** dst, uint32_t src_freq, size_t src_len) {
  // Determine size of WAV header for file size calcs
  WaveFileHeader* src_wfh = (WaveFileHeader*)src;
  size_t wave_header_size = src_wfh->size();

  size_t src_num_samples = src_len - wave_header_size;
  double ratio = (double)src_freq / SDL_MIN_FREQ;
  double seconds = (double)src_num_samples / src_freq;
  uint32_t dst_num_samples = (seconds * SDL_MIN_FREQ);

  size_t dst_size = wave_header_size + dst_num_samples;
  *dst = malloc(dst_size);
  memcpy(*dst, src, wave_header_size);

  // Update some of the values in the header
  WaveFileHeader* dst_wfh = (WaveFileHeader*)*dst;
  dst_wfh->file_size = dst_size - 8;
  dst_wfh->sample_rate = SDL_MIN_FREQ;
  dst_wfh->byte_rate = SDL_MIN_FREQ;

  for (int i = 0; i < dst_num_samples; i++) {
    ((uint8_t*)*dst)[i + wave_header_size] = *(src + wave_header_size + (uint32_t)((double)src_num_samples * i / dst_num_samples));
  }

  FILE* f = fopen("/tmp/output.wav", "wb");
  fclose(f);

  return dst_size;
}

bool SoundManager_is_sound_registered(int16_t id) {
  return sm.is_sound_registered(id);
}

void SoundManager_register_sound(int16_t id, uint32_t freq, void* data, uint32_t len) {
  // SDL has a minimum supported sample frequency, but unfortunately some of the
  // sounds in Realmz have a significantly lower rate. If this is one of them, run
  // a quick and dirty upsampling routine.
  if (freq < SDL_MIN_FREQ) {
    void* d;
    len = upsample((uint8_t*)data, &d, freq, len);
    freq = SDL_MIN_FREQ;
    free(data);
    data = d;
  }

  sm.register_sound(id, Sound{.channels = 1, .freq = freq, .data = data, .len = len});
}

void SoundManager_play_sound(SDL_AudioStream* sdlAudioStream, int16_t id) {
  sm.play_sound(sdlAudioStream, id);
}
