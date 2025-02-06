#pragma once

#include <SDL3_ttf/SDL_ttf.h>
#include <resource_file/BitmapFontRenderer.hh>

#define BLACK_CHANCERY_FONT_ID 1602
#define GENEVA_FONT_ID 0
#define CHICAGO_FONT_ID 1

void init_fonts();
bool load_font(int16_t font_id, TTF_Font** ttf_font_handle, ResourceDASM::BitmapFontRenderer** bm_font_handle);
void set_font_face(TTF_Font* font, int16_t face);
std::string replace_param_text(const std::string& text);