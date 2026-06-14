#include "PortPrefs.hpp"

#include <SDL3/SDL_filesystem.h>

#include <algorithm>
#include <string>

#include "PortMenu.hpp"

#include <phosg/Filesystem.hh>
#include <phosg/JSON.hh>
#include <phosg/Strings.hh>

#include "Types.hpp"

static phosg::PrefixedLogger prefs_log("[PortPrefs] ", DEFAULT_LOG_LEVEL);

static constexpr int MIN_DIM = 1;
static constexpr int MAX_W = 800 * 4;
static constexpr int MAX_H = 600 * 4;

static std::string prefs_path() {
  char* base = SDL_GetPrefPath("Fantasoft", "Realmz");
  if (!base) {
    return std::string();
  }
  std::string path = std::string(base) + "port_settings.json";
  SDL_free(base);
  return path;
}

static const char* name_for_scale_mode(SDL_ScaleMode mode) {
  switch (mode) {
    case SDL_SCALEMODE_LINEAR:
      return "linear";
    case SDL_SCALEMODE_NEAREST:
      return "nearest";
    default:
      return "pixelart";
  }
}

static SDL_ScaleMode scale_mode_for_name(const std::string& name) {
  if (name == "linear") {
    return SDL_SCALEMODE_LINEAR;
  } else if (name == "nearest") {
    return SDL_SCALEMODE_NEAREST;
  }
  return SDL_SCALEMODE_PIXELART;
}

PortPrefs load_port_prefs() {
  PortPrefs prefs;

  std::string path = prefs_path();
  if (path.empty()) {
    prefs_log.warning_f("Could not get pref path: {}; using defaults", SDL_GetError());
    return prefs;
  }

  std::string data;
  try {
    data = phosg::load_file(path);
  } catch (const std::exception& e) {
    prefs_log.info_f("Could not read {} ({}); using defaults", path, e.what());
    return prefs;
  }

  try {
    auto root = phosg::JSON::parse(data);
    prefs.window_w = std::clamp(static_cast<int>(root.get_int("window_w", prefs.window_w)), MIN_DIM, MAX_W);
    prefs.window_h = std::clamp(static_cast<int>(root.get_int("window_h", prefs.window_h)), MIN_DIM, MAX_H);
    prefs.window_x = static_cast<int>(root.get_int("window_x", prefs.window_x));
    prefs.window_y = static_cast<int>(root.get_int("window_y", prefs.window_y));
    prefs.scale_mode = scale_mode_for_name(root.get_string("filter", name_for_scale_mode(prefs.scale_mode)));
    prefs.aspect_locked = root.get_bool("aspect_locked", prefs.aspect_locked);
    prefs.gamma_idx = std::clamp(static_cast<int>(root.get_int("gamma_idx", prefs.gamma_idx)), 0, kPortGammaCount - 1);
  } catch (const std::exception& e) {
    prefs_log.warning_f("Could not parse {} ({}); using defaults", path, e.what());
    return PortPrefs{};
  }

  prefs_log.info_f("Loaded prefs: window {}x{}, filter {}", prefs.window_w, prefs.window_h, name_for_scale_mode(prefs.scale_mode));
  return prefs;
}

void save_port_prefs(const PortPrefs& prefs) {
  std::string path = prefs_path();
  if (path.empty()) {
    prefs_log.warning_f("Could not get pref path: {}; not saving prefs", SDL_GetError());
    return;
  }

  phosg::JSON root = phosg::JSON::dict();
  root.emplace("window_w", static_cast<int64_t>(prefs.window_w));
  root.emplace("window_h", static_cast<int64_t>(prefs.window_h));
  if (!SDL_WINDOWPOS_ISCENTERED(prefs.window_x) && !SDL_WINDOWPOS_ISUNDEFINED(prefs.window_x) &&
      !SDL_WINDOWPOS_ISCENTERED(prefs.window_y) && !SDL_WINDOWPOS_ISUNDEFINED(prefs.window_y)) {
    root.emplace("window_x", static_cast<int64_t>(prefs.window_x));
    root.emplace("window_y", static_cast<int64_t>(prefs.window_y));
  }
  root.emplace("filter", name_for_scale_mode(prefs.scale_mode));
  root.emplace("aspect_locked", prefs.aspect_locked);
  root.emplace("gamma_idx", static_cast<int64_t>(prefs.gamma_idx));

  try {
    phosg::save_file(path, root.serialize());
  } catch (const std::exception& e) {
    prefs_log.warning_f("Could not write {} ({})", path, e.what());
  }
}
