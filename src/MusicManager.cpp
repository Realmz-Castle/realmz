#include "MusicManager.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <atomic>
#include <exception>
#include <format>
#include <memory>
#include <phosg/Filesystem.hh>
#include <phosg/Hash.hh>
#include <phosg/Strings.hh>
#include <resource_file/Audio/MODSynthesizer.hh>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "FileManager.hpp"
#include "ResourceManager.h"
#include "SoundManager.h"
#include "StringConvert.hpp"

namespace {

constexpr size_t MUSIC_SAMPLE_RATE = 48000;
constexpr int MUSIC_MAX_QUEUED_BYTES = static_cast<int>(MUSIC_SAMPLE_RATE * 2 * sizeof(float) / 4);
constexpr size_t LEGACY_OUTDOOR_MUSIC_SIZE = 60224;
constexpr const char* LEGACY_OUTDOOR_MUSIC_MD5 = "1A2E7CC637BCF082D21204E2DA1028B2";

phosg::PrefixedLogger music_log("[MusicManager] ");

std::string path_for_playlist(short playlist, const char* scenario_path) {
  if (playlist < 1 || playlist > 17) {
    return {};
  }
  Str255 name;
  GetIndString(name, 140, playlist);
  auto path = string_for_pstr<256>(name);
  if (path.empty()) {
    return {};
  }
  if (playlist > 14) {
    if (!scenario_path || !scenario_path[0]) {
      return {};
    }
    path = std::string(scenario_path) + path;
  }
  return path;
}

std::string read_music_file(const std::string& path) {
  auto f = mac_fopen_unique(path, "rb");
  if (!f) {
    throw std::runtime_error(std::format("could not open {}", path));
  }
  return phosg::read_all(f.get());
}

std::shared_ptr<ResourceDASM::Audio::Module> parse_module_file(const std::string& path) {
  std::string data = read_music_file(path);
  if ((data.size() == LEGACY_OUTDOOR_MUSIC_SIZE) &&
      (phosg::MD5(data).hex() == LEGACY_OUTDOOR_MUSIC_MD5)) {
    music_log.info_f("Using bundled Outdoor Music for legacy MADG track {}", path);
    data = read_music_file(":Realmz Music:Outdoor Music");
  }
  return ResourceDASM::Audio::Module::parse(data);
}

float gain_for_volume(short volume) {
  return std::clamp(static_cast<float>(volume) / 7.0f, 0.0f, 1.0f);
}

class StreamingMODPlayer : public ResourceDASM::Audio::MODSynthesizer {
public:
  StreamingMODPlayer(
      std::shared_ptr<const ResourceDASM::Audio::Module> mod,
      std::shared_ptr<const Options> opts,
      SDL_AudioStream* stream,
      std::atomic_bool& stop_requested)
      : MODSynthesizer(std::move(mod), std::move(opts)),
        stream(stream),
        stop_requested(stop_requested) {}

protected:
  bool on_tick_samples_ready(std::vector<float>&& samples) override {
    while (!this->stop_requested.load()) {
      int queued = SDL_GetAudioStreamQueued(this->stream);
      if (queued < 0) {
        throw std::runtime_error(std::format("could not read music audio queue: {}", SDL_GetError()));
      }
      if (queued <= MUSIC_MAX_QUEUED_BYTES) {
        break;
      }
      SDL_Delay(20);
    }
    if (this->stop_requested.load()) {
      return false;
    }
    if (!SDL_PutAudioStreamData(this->stream, samples.data(), static_cast<int>(samples.size() * sizeof(float)))) {
      throw std::runtime_error(std::format("could not queue music audio: {}", SDL_GetError()));
    }
    return true;
  }

private:
  SDL_AudioStream* stream;
  std::atomic_bool& stop_requested;
};

class MusicManager {
public:
  ~MusicManager() {
    this->stop();
  }

  bool play(short playlist, const char* scenario_path) {
    const std::string path = path_for_playlist(playlist, scenario_path);
    if (this->playing.load() && (this->current_playlist == playlist) && (this->current_path == path)) {
      return true;
    }
    this->stop();
    if (path.empty()) {
      music_log.warning_f("No file found for playlist {}", playlist);
      return false;
    }

    try {
      auto module = parse_module_file(path);
      if (module->partition_count == 0) {
        throw std::runtime_error("music module has an empty song sequence");
      }
      SDL_AudioSpec spec{};
      spec.format = SDL_AUDIO_F32LE;
      spec.channels = 2;
      spec.freq = MUSIC_SAMPLE_RATE;
      this->stream = CreateDefaultOutputAudioStream(&spec, gain_for_volume(this->volume));
      if (!this->stream) {
        return false;
      }

      this->stop_requested.store(false);
      this->current_playlist = playlist;
      this->current_path = path;
      this->playing.store(true);
      music_log.info_f("Starting playlist {} from {}", playlist, path);
      this->worker = std::thread(&MusicManager::play_worker, this, playlist, std::move(module));
      return true;
    } catch (const std::exception& e) {
      this->stop();
      music_log.warning_f("Could not play playlist {}: {}", playlist, e.what());
      return false;
    }
  }

  void stop() {
    this->stop_requested.store(true);
    if (this->worker.joinable()) {
      this->worker.join();
    }
    this->playing.store(false);
    if (this->stream) {
      SDL_ClearAudioStream(this->stream);
      SDL_DestroyAudioStream(this->stream);
      this->stream = nullptr;
    }
    this->current_playlist = -1;
    this->current_path.clear();
  }

  void set_volume(short volume) {
    this->volume = std::clamp<short>(volume, 0, 7);
    if (this->stream && !SDL_SetAudioStreamGain(this->stream, gain_for_volume(this->volume))) {
      music_log.warning_f("Could not set music volume: {}", SDL_GetError());
    }
  }

private:
  void play_worker(short playlist, std::shared_ptr<const ResourceDASM::Audio::Module> module) {
    try {
      auto opts = std::make_shared<ResourceDASM::Audio::MODSynthesizer::Options>();
      opts->sample_rate = MUSIC_SAMPLE_RATE;
      opts->global_volume = 2.0f / static_cast<float>(std::max<size_t>(1, module->num_tracks));
      opts->log_level = phosg::LogLevel::L_WARNING;

      music_log.info_f("Loaded playlist {} with {} tracks", playlist, module->num_tracks);
      while (!this->stop_requested.load()) {
        StreamingMODPlayer player(module, opts, this->stream, this->stop_requested);
        while (!this->stop_requested.load() && !player.done()) {
          player.run_one();
        }
      }
    } catch (const std::exception& e) {
      music_log.warning_f("Playlist {} stopped: {}", playlist, e.what());
    }
    this->playing.store(false);
  }

  std::atomic_bool stop_requested = false;
  std::atomic_bool playing = false;
  short volume = 4;
  short current_playlist = -1;
  std::string current_path;
  SDL_AudioStream* stream = nullptr;
  std::thread worker;
};

MusicManager manager;

} // namespace

Boolean RealmzMusicPlay(short playlist, const char* scenario_path) {
  return manager.play(playlist, scenario_path);
}

void RealmzMusicStop(void) {
  manager.stop();
}

void RealmzMusicSetVolume(short volume) {
  manager.set_volume(volume);
}
