#pragma once

#include "Types.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <memory>

template <auto fn>
struct deleter_from_fn {
  template <typename T>
  constexpr void operator()(T* arg) const { fn(arg); }
};

template <typename T, auto fn>
using sdl_unique_ptr = std::unique_ptr<T, deleter_from_fn<fn>>;

template <typename T>
using sdl_shared_ptr = std::shared_ptr<T>;

using sdl_surface_ptr = sdl_unique_ptr<SDL_Surface, SDL_DestroySurface>;
using sdl_texture_ptr = sdl_unique_ptr<SDL_Texture, SDL_DestroyTexture>;
// In SDL3, SDL_DestroyWindow also destroys the associated renderer, if present.
// So, we only need to create unique pointers for renderers that are not associated
// with a window.
using sdl_renderer_ptr = sdl_unique_ptr<SDL_Renderer, SDL_DestroyRenderer>;
using sdl_renderer_shared = sdl_shared_ptr<SDL_Renderer>;
using sdl_window_shared = sdl_shared_ptr<SDL_Window>;

sdl_surface_ptr sdl_make_unique(SDL_Surface* s);
sdl_texture_ptr sdl_make_unique(SDL_Texture* t);
sdl_renderer_ptr sdl_make_unique(SDL_Renderer* r);
sdl_renderer_shared sdl_make_shared(SDL_Renderer* r);
sdl_window_shared sdl_make_shared(SDL_Window* w);

SDL_FRect sdl_frect(const Rect& rect);
SDL_Rect sdl_rect(const Rect& rect);
