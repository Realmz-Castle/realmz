#include "PortMenu.hpp"
#include "WindowManager.hpp"

void PortMenu_Apply(int id) {
  WindowManager& wm = WindowManager::instance();
  if (id < kPortScaleId) {
    wm.set_scale_mode(kPortFilters[id - kPortFilterId].mode);
  } else if (id < kPortAspectLockId) {
    const auto& scale = kPortScales[id - kPortScaleId];
    wm.set_window_size(scale.width, scale.height);
  } else if (id < kPortGammaId) {
    wm.set_aspect_locked(!wm.get_aspect_locked());
  } else {
    wm.set_gamma_idx(id - kPortGammaId);
  }
}

void PortMenu_ItemState(int id, int* checked, int* enabled) {
  WindowManager& wm = WindowManager::instance();
  int is_checked = 0;
  int is_enabled = 1;
  bool fullscreen = wm.is_fullscreen();
  if (id < kPortScaleId) {
    is_checked = kPortFilters[id - kPortFilterId].mode == wm.get_scale_mode();
  } else if (id < kPortAspectLockId) {
    const auto& scale = kPortScales[id - kPortScaleId];
    is_enabled = !fullscreen && wm.size_fits(scale.width, scale.height);
    int cur_w = 0, cur_h = 0;
    wm.get_window_size(&cur_w, &cur_h);
    is_checked = !fullscreen && (cur_w == scale.width) && (cur_h == scale.height);
  } else if (id < kPortGammaId) {
    is_enabled = !fullscreen;
    is_checked = wm.get_aspect_locked();
  } else {
    is_checked = (id - kPortGammaId) == wm.get_gamma_idx();
  }
  if (checked) {
    *checked = is_checked;
  }
  if (enabled) {
    *enabled = is_enabled;
  }
}
