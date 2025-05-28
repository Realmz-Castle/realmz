#include "SoundManager.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>
#include <phosg/Encoding.hh>
#include <resource_file/ResourceFile.hh>
#include <unordered_map>

#include "MemoryManager.hpp"
#include "ResourceManager.h"
#include "Types.hpp"

static phosg::PrefixedLogger sm_log("[SoundManager] ");

constexpr size_t OUTPUT_SAMPLE_RATE = 48000;

class SoundManager {
public:
  SoundManager() = default;

  ~SoundManager() {
    if (this->device_id > 0) {
      SDL_CloseAudioDevice(this->device_id);
    }
  }

  void lazy_initialize(void) {
    if (this->device_id > 0) {
      return;
    }
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
      sm_log.warning_f("Could not initialize audio subsystem: {}", SDL_GetError());
      return;
    }
    this->device_id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    sm_log.info_f("Using device: {}", SDL_GetAudioDeviceName(this->device_id));
    if (this->device_id == 0) {
      sm_log.warning_f("Failed to open audio: {}", SDL_GetError());
    } else {
      sm_log.info_f("Audio device paused: {}", SDL_AudioDevicePaused(this->device_id));
    }
  }

  std::shared_ptr<SndChannel> create_channel() {
    this->lazy_initialize();

    auto channel = std::make_shared<SndChannel>();
    channel->qLength = 0;

    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_S16LE;
    spec.channels = 2;
    spec.freq = OUTPUT_SAMPLE_RATE;
    channel->sdlAudioStream = SDL_CreateAudioStream(&spec, &spec);
    if (channel->sdlAudioStream == NULL) {
      sm_log.warning_f("Could not create SDL audio stream: {}", SDL_GetError());
      return nullptr;
    }

    SDL_BindAudioStream(this->device_id, channel->sdlAudioStream);
    sm_log.info_f("Created output channel on audio stream device: {}", SDL_GetAudioStreamDevice(channel->sdlAudioStream));

    if (SDL_SetAudioStreamFormat(channel->sdlAudioStream, &spec, NULL) < 0) {
      sm_log.warning_f("Could not set audio stream format: {}", SDL_GetError());
    }

    this->all_channels.emplace(channel);
    return channel;
  }

  void play_sound(SDL_AudioStream* sdlAudioStream, Handle data_handle) {
    std::shared_ptr<const Sound> sound;
    try {
      sound = this->sound_for_handle(data_handle);
    } catch (const std::exception& e) {
      sm_log.warning_f("Can't find or decode sound: {}", e.what());
      return;
    }

    if (SDL_PutAudioStreamData(sdlAudioStream, sound->data.data(), sound->data.size()) < 0) {
      sm_log.warning_f("Could not put audio stream data: {}", SDL_GetError());
    }
  }

private:
  struct Sound {
    uint32_t sample_rate;
    uint8_t num_channels;
    uint8_t bits_per_sample;
    std::string data;
  };

  std::shared_ptr<const Sound> sound_for_handle(Handle data_handle) {
    try {
      return this->decoded_sounds.at(data_handle);
    } catch (const std::out_of_range&) {
    }

    auto decoded = ResourceDASM::ResourceFile::decode_snd_data(*data_handle, GetHandleSize(data_handle));

    bool is_16bit;
    if (decoded.bits_per_sample == 8) {
      is_16bit = false;
    } else if (decoded.bits_per_sample == 16) {
      is_16bit = true;
    } else {
      throw std::runtime_error("Unsupported sample format");
    }

    bool is_stereo;
    if (decoded.num_channels == 1) {
      is_stereo = false;
    } else if (decoded.num_channels == 2) {
      is_stereo = true;
    } else {
      throw std::runtime_error("Unsupported number of channels");
    }

    auto r = phosg::StringReader(decoded.data).sub(decoded.sample_start_offset);
    auto read_sample = [&]() -> float {
      if (is_16bit) {
        return static_cast<float>(r.get_s16l()) / 0x8000;
      } else {
        int8_t s8_sample = r.get_u8() - 0x80;
        return static_cast<float>(s8_sample) / 0x80;
      }
    };

    phosg::StringWriter w;
    auto write_sample = [&](float sample) -> void {
      w.put_s16l(std::clamp<int64_t>(static_cast<int64_t>(sample * 0x7FFF), -0x8000, 0x7FFF));
    };

    double expansion_factor = static_cast<double>(OUTPUT_SAMPLE_RATE) / static_cast<double>(decoded.sample_rate);
    float l_prev_sample = read_sample();
    float r_prev_sample = is_stereo ? read_sample() : l_prev_sample;
    while (!r.eof()) {
      float l_sample = read_sample();
      float r_sample = is_stereo ? read_sample() : l_sample;
      size_t in_sample_index = r.where() >> (is_16bit + is_stereo); // 1-4 bytes per frame, depending on is_16bit and is_stereo
      size_t samples_to_write =
          static_cast<size_t>(ceil((in_sample_index + 1) * expansion_factor)) -
          static_cast<size_t>(ceil(in_sample_index * expansion_factor));
      for (size_t z = 0; z < samples_to_write; z++) {
        // Linearly interpolate this output sample between the previous and next input samples
        float progress_factor = static_cast<float>(z) / samples_to_write;
        write_sample(l_prev_sample * (1.0 - progress_factor) + l_sample * progress_factor);
        write_sample(r_prev_sample * (1.0 - progress_factor) + r_sample * progress_factor);
      }
      l_prev_sample = l_sample;
      r_prev_sample = r_sample;
    }

    auto ret = std::make_shared<Sound>();
    ret->bits_per_sample = 16;
    ret->num_channels = 2;
    ret->sample_rate = OUTPUT_SAMPLE_RATE;
    ret->data = std::move(w.str());
    this->decoded_sounds.emplace(data_handle, ret);
    return ret;
  }

  SDL_AudioDeviceID device_id;
  std::unordered_set<std::shared_ptr<SndChannel>> all_channels;
  std::unordered_map<Handle, std::shared_ptr<const Sound>> decoded_sounds;
};

static SoundManager sm;

OSErr SndNewChannel(SndChannelPtr* chan, uint16_t synth, int32_t init, void* userRoutine) {
  // Realmz only passes null pointers to SndNewChannel, so no need to check for
  // an existing SndChannel
  try {
    auto sm_chan = sm.create_channel();
    *chan = sm_chan.get();
    return noErr;
  } catch (const std::exception& e) {
    sm_log.warning_f("Failed to create audio channel: {}", e.what());
    return badChannel;
  }
}

OSErr SndDoImmediate(SndChannelPtr chan, const SndCommand* cmd) {
  return 0;
}

OSErr SndPlay(SndChannelPtr chan, Handle data_handle, Boolean async) {
  // Some Realmz data, such as the mapstats array loaded in loadland-loadpixmap.c, appears to
  // specify a snd resource with id 0, which cannot be loaded. In these cases, we simply
  // return an error.
  if (data_handle == nullptr) {
    return resProblem;
  }
  sm.play_sound(chan->sdlAudioStream, data_handle);
  return noErr;
}
