#include "SDLHelpers.hpp"

sdl_surface_ptr sdl_make_unique(SDL_Surface* s) {
  return sdl_surface_ptr(s);
}

sdl_texture_ptr sdl_make_unique(SDL_Texture* t) {
  return sdl_texture_ptr(t);
}

sdl_renderer_ptr sdl_make_unique(SDL_Renderer* r) {
  return sdl_renderer_ptr(r);
}

sdl_renderer_shared sdl_make_shared(SDL_Renderer* r) {
  return sdl_renderer_shared(r, SDL_DestroyRenderer);
}

sdl_window_shared sdl_make_shared(SDL_Window* w) {
  return sdl_window_shared(w, SDL_DestroyWindow);
}

SDL_FRect sdl_frect(const Rect& rect) {
  SDL_FRect dest;
  dest.x = rect.left;
  dest.y = rect.top;
  dest.w = rect.right - rect.left;
  dest.h = rect.bottom - rect.top;
  return dest;
}

SDL_Rect sdl_rect(const Rect& rect) {
  SDL_Rect dest;
  dest.x = rect.left;
  dest.y = rect.top;
  dest.w = rect.right - rect.left;
  dest.h = rect.bottom - rect.top;
  return dest;
}