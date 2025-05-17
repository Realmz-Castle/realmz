#include <iostream>

#include "QuickDraw.h"
#include "WindowManager.hpp"

#include <phosg/Strings.hh>

#include "QuickDraw.hpp"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Big wind
#define WIND_RES_ID 128

static WindowManager wm;
static phosg::PrefixedLogger wm_log("[WindowManager] ");

constexpr RGBColor blue{0, 0, 255};
constexpr RGBColor white{255, 255, 255};

int main() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    wm_log.error("Couldn't initialize video driver: %s\n", SDL_GetError());
    return 1;
  }

  // Have to define the starting default port, so that WindowManager can copy its
  // parameters when creating new windows.
  InitGraf(&qd);

  wm.init();

  auto bounds = Rect{0, 0, WINDOW_HEIGHT, WINDOW_WIDTH};

  auto wp = wm.create_window(
      "GraphicsTest",
      bounds,
      true,
      true,
      0,
      0,
      false,
      {});
  auto window = wm.window_for_record(wp);

  auto canvas = lookup_canvas(window->get_port());

  canvas->set_draw_color(white);

  canvas->draw_rect(Rect{20, 20, 120, 170});

  // Circle
  Rect bounding_box{140, 20, 240, 120};
  canvas->set_draw_color(blue);
  canvas->draw_rect(bounding_box);
  canvas->set_draw_color(white);
  canvas->draw_oval(bounding_box);

  // Horizontal ellipse
  bounding_box = Rect{260, 20, 300, 120};
  canvas->set_draw_color(blue);
  canvas->draw_rect(bounding_box);
  canvas->set_draw_color(white);
  canvas->draw_oval(bounding_box);

  // Vertical ellipse
  bounding_box = Rect{320, 20, 420, 60};
  canvas->set_draw_color(blue);
  canvas->draw_rect(bounding_box);
  canvas->set_draw_color(white);
  canvas->draw_oval(bounding_box);

  window->render();

  for (;;)
    ;
}