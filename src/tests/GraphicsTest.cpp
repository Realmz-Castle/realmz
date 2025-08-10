#include <iostream>

#include "QuickDraw.h"
#include "WindowManager.hpp"

#include <phosg/Strings.hh>

#include "QuickDraw.hpp"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Big wind
#define WIND_RES_ID 128

constexpr RGBColor blue{0, 0, 0xFFFF};
constexpr RGBColor white{0xFFFF, 0xFFFF, 0xFFFF};

int main() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    phosg::log_error_f("Couldn't initialize video driver: {}", SDL_GetError());
    return 1;
  }

  // Have to define the starting default port, so that WindowManager can copy its
  // parameters when creating new windows.
  InitGraf(&qd);

  auto& wm = WindowManager::instance();

  wm.create_sdl_window();

  auto bounds = Rect{0, 0, WINDOW_HEIGHT, WINDOW_WIDTH};

  RGBColor background_color = {0x0000, 0x0000, 0x0000};
  auto wp = wm.create_window(
      "GraphicsTest",
      bounds,
      true,
      true,
      0,
      0,
      false,
      background_color,
      {});
  auto window = wm.window_for_port(wp);

  auto& port = window->get_port();

  port.rgbFgColor = white;
  port.draw_rect(Rect{20, 20, 120, 170});

  // Circle
  Rect bounding_box{140, 20, 240, 120};
  port.rgbFgColor = blue;
  port.draw_rect(bounding_box);
  port.rgbFgColor = white;
  port.draw_oval(bounding_box);

  // Horizontal ellipse
  bounding_box = Rect{260, 20, 300, 120};
  port.rgbFgColor = blue;
  port.draw_rect(bounding_box);
  port.rgbFgColor = white;
  port.draw_oval(bounding_box);

  // Vertical ellipse
  bounding_box = Rect{320, 20, 420, 60};
  port.rgbFgColor = blue;
  port.draw_rect(bounding_box);
  port.rgbFgColor = white;
  port.draw_oval(bounding_box);

  wm.recomposite(window);

  for (;;)
    ;
}
