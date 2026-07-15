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
#include "SoundManager.h"

namespace {

constexpr size_t MUSIC_SAMPLE_RATE = 48000;
constexpr int MUSIC_MAX_QUEUED_BYTES = static_cast<int>(MUSIC_SAMPLE_RATE * 2 * sizeof(float) / 4);
constexpr size_t LEGACY_OUTDOOR_MUSIC_SIZE = 60224;
constexpr const char* LEGACY_OUTDOOR_MUSIC_MD5 = "1A2E7CC637BCF082D21204E2DA1028B2";

phosg::PrefixedLogger music_log("[MusicManager] ");

class StopPlayback : public std::exception {};

struct PlaylistEntry {
  short playlist;
  const char* name;
  const char* filename;
  bool scenario_music;
};

constexpr PlaylistEntry PLAYLISTS[] = {
    {1, "Outdoor", "Outdoor Music", false},
    {2, "Dungeon", "Dungeon Music", false},
    {3, "Indoor", "Indoor Music", false},
    {4, "Cave", "Cave Music", false},
    {5, "Create", "Create Music", false},
    {6, "Items", "Items Music", false},
    {7, "Treasure", "Treasure Music", false},
    {8, "Shop", "Shop Music", false},
    {9, "Camp", "Camp Music", false},
    {10, "Temple", "Temple Music", false},
    {11, "Battle", "Battle Music", false},
    {12, "Desert", "Desert Music", false},
    {13, "Swamp", "Swamp Music", false},
    {14, "Snow", "Snow Music", false},
    {15, "Custom 1", "Custom 1 Music", true},
    {16, "Custom 2", "Custom 2 Music", true},
    {17, "Custom 3", "Custom 3 Music", true},
};

const PlaylistEntry* entry_for_playlist(short playlist) {
  for (const auto& entry : PLAYLISTS) {
    if (entry.playlist == playlist) {
      return &entry;
    }
  }
  return nullptr;
}

std::string path_for_entry(const PlaylistEntry& entry, const char* scenario_path) {
  if (!entry.scenario_music) {
    return std::format(":Realmz Music:{}", entry.filename);
  }
  if (!scenario_path || !scenario_path[0]) {
    return {};
  }
  std::string path = scenario_path;
  if (path.back() != ':') {
    path += ':';
  }
  return path + entry.filename;
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
    if (this->stop_requested.load()) {
      throw StopPlayback();
    }
    while (!this->stop_requested.load() && SDL_GetAudioStreamQueued(this->stream) > MUSIC_MAX_QUEUED_BYTES) {
      SDL_Delay(20);
    }
    if (this->stop_requested.load()) {
      throw StopPlayback();
    }
    if (!SDL_PutAudioStreamData(this->stream, samples.data(), static_cast<int>(samples.size() * sizeof(float)))) {
      music_log.warning_f("Could not queue music audio: {}", SDL_GetError());
      return false;
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
    const auto* entry = entry_for_playlist(playlist);
    if (!entry) {
      music_log.warning_f("Ignoring unknown playlist {}", playlist);
      return false;
    }
    const std::string path = path_for_entry(*entry, scenario_path);
    if (path.empty()) {
      music_log.warning_f("No file found for playlist {} ({})", playlist, entry->name);
      return false;
    }
    if ((this->current_playlist == playlist) && (this->current_path == path)) {
      return true;
    }

    std::shared_ptr<ResourceDASM::Audio::Module> module;
    try {
      module = parse_module_file(path);
    } catch (const std::exception& e) {
      music_log.warning_f("Could not load playlist {} ({}): {}", playlist, entry->name, e.what());
      return false;
    }

    SDL_AudioSpec spec{};
    spec.format = SDL_AUDIO_F32LE;
    spec.channels = 2;
    spec.freq = MUSIC_SAMPLE_RATE;
    auto* new_stream = CreateDefaultOutputAudioStream(&spec, gain_for_volume(this->volume));
    if (!new_stream) {
      return false;
    }

    this->stop();
    this->stream = new_stream;
    this->stop_requested.store(false);
    this->current_playlist = playlist;
    this->current_path = path;
    music_log.info_f("Starting playlist {} ({}) from {}", playlist, entry->name, path);
    this->worker = std::thread(&MusicManager::play_worker, this, playlist, entry->name, std::move(module));
    return true;
  }

  void stop() {
    this->stop_requested.store(true);
    if (this->worker.joinable()) {
      this->worker.join();
    }
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
  void play_worker(short playlist, std::string name, std::shared_ptr<const ResourceDASM::Audio::Module> module) {
    try {
      auto opts = std::make_shared<ResourceDASM::Audio::MODSynthesizer::Options>();
      opts->sample_rate = MUSIC_SAMPLE_RATE;
      opts->global_volume = 2.0f / static_cast<float>(std::max<size_t>(1, module->num_tracks));
      opts->log_level = phosg::LogLevel::L_WARNING;

      music_log.info_f("Loaded playlist {} ({}) with {} tracks", playlist, name, module->num_tracks);
      while (!this->stop_requested.load()) {
        StreamingMODPlayer player(module, opts, this->stream, this->stop_requested);
        player.run_all();
      }
    } catch (const StopPlayback&) {
    } catch (const std::exception& e) {
      music_log.warning_f("Playlist {} ({}) stopped: {}", playlist, name, e.what());
    }
  }

  std::atomic_bool stop_requested = false;
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
