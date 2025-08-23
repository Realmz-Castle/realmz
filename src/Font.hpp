#pragma once

#include <SDL3_ttf/SDL_ttf.h>
#include <resource_file/BitmapFontRenderer.hh>
#include <variant>

#define BLACK_CHANCERY_FONT_ID 1602
#define GENEVA_FONT_ID 1
#define CHICAGO_FONT_ID 0
#define ALTERNATIVE_GENEVA_FONT_ID 3

// variables.h defines "genevafont" as 10. It doesn't seem like Classic Mac had a
// standard FONT resource with id 10, but clearly they expected it to be Geneva.
#define REALMZ_GENEVA_FONT_ID 10

typedef std::variant<TTF_Font*, ResourceDASM::BitmapFontRenderer> Font;

void init_fonts();
Font load_font(int16_t font_id);
void set_font_style(TTF_Font* font, int16_t face);
std::string replace_param_text(const std::string& text);
