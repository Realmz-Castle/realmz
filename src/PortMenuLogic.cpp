#include "PortMenu.hpp"
#include "WindowManager.hpp"

void PortMenu_Apply(PortCmdKind kind, int index) {
  WindowManager& wm = WindowManager::instance();
  switch (kind) {
    case PortCmdFilter:
      wm.set_scale_mode(kPortFilters[index].mode);
      break;
    case PortCmdScale:
      wm.set_window_size(kPortScales[index].width, kPortScales[index].height);
      break;
    case PortCmdAspectLock:
      wm.set_aspect_locked(!wm.get_aspect_locked());
      break;
    case PortCmdGamma:
      wm.set_gamma_idx(index);
      break;
  }
}

void PortMenu_ItemState(PortCmdKind kind, int index, int* checked, int* enabled) {
  WindowManager& wm = WindowManager::instance();
  int is_checked = 0;
  int is_enabled = 1;
  bool fullscreen = wm.is_fullscreen();
  switch (kind) {
    case PortCmdFilter:
      is_checked = kPortFilters[index].mode == wm.get_scale_mode();
      break;
    case PortCmdScale: {
      const auto& scale = kPortScales[index];
      is_enabled = !fullscreen && wm.size_fits(scale.width, scale.height);
      int cur_w = 0, cur_h = 0;
      wm.get_window_size(&cur_w, &cur_h);
      is_checked = !fullscreen && (cur_w == scale.width) && (cur_h == scale.height);
      break;
    }
    case PortCmdAspectLock:
      is_enabled = !fullscreen;
      is_checked = wm.get_aspect_locked();
      break;
    case PortCmdGamma:
      is_checked = index == wm.get_gamma_idx();
      break;
  }
  if (checked) {
    *checked = is_checked;
  }
  if (enabled) {
    *enabled = is_enabled;
  }
}
