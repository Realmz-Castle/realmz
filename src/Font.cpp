#include "Font.hpp"

#include "FileManager.hpp"
#include "Font.h"
#include "MemoryManager.h"
#include "ResourceManager.h"
#include "StringConvert.hpp"

static std::unordered_map<int16_t, TTF_Font*> tt_fonts_by_id;
static std::unordered_map<int16_t, ResourceDASM::BitmapFontRenderer>
    bm_renderers_by_id;
static std::array<std::string, 4> param_text_entries;

void init_fonts() {
  // Pre-populate Black Chancery, since it doesnt' come from the resource fork.
  // We can't pre-populate Theldrow because the resource files aren't loaded at
  // the time WindowManager_Init is called.

  // TODO: Is 1602 the correct font ID for Black Chancery?
  auto font_filename =
      host_filename_for_mac_filename(":Black Chancery.ttf", true);
  tt_fonts_by_id[BLACK_CHANCERY_FONT_ID] =
      TTF_OpenFont(font_filename.c_str(), 16);
  font_filename = host_filename_for_mac_filename(":Geneva.ttf", true);
  auto geneva_font = TTF_OpenFont(font_filename.c_str(), 16);
  tt_fonts_by_id[GENEVA_FONT_ID] = geneva_font;
  tt_fonts_by_id[REALMZ_GENEVA_FONT_ID] = geneva_font;
  font_filename = host_filename_for_mac_filename(":Chicago.ttf", true);
  tt_fonts_by_id[CHICAGO_FONT_ID] = TTF_OpenFont(font_filename.c_str(), 16);
}

// Tries to load a TrueType font first; if it's not available, use a
// bitmapped font instead.
Font load_font(int16_t font_id) {
  try {
    return tt_fonts_by_id.at(font_id);
  } catch (const std::out_of_range&) {
  }

  try {
    return bm_renderers_by_id.at(font_id);
  } catch (const std::out_of_range&) {
    auto data_handle = GetResource(ResourceDASM::RESOURCE_TYPE_FONT, font_id);
    auto decoded =
        std::make_shared<ResourceDASM::ResourceFile::DecodedFontResource>(
            ResourceDASM::ResourceFile::decode_FONT_only(
                *data_handle, GetHandleSize(data_handle)));
    return bm_renderers_by_id.emplace(font_id, decoded).first->second;
  }
}

void set_font_face(TTF_Font* font, int16_t face) {
  TTF_FontStyleFlags styles{TTF_STYLE_NORMAL};

  if (face == bold) {
    styles |= TTF_STYLE_BOLD;
  } else if (face == outline) {
    styles |= TTF_STYLE_UNDERLINE;
  }

  TTF_SetFontStyle(font, styles);
}

void ParamText(ConstStr255Param param0, ConstStr255Param param1,
    ConstStr255Param param2, ConstStr255Param param3) {
  param_text_entries[0] = string_for_pstr<256>(param0);
  param_text_entries[1] = string_for_pstr<256>(param1);
  param_text_entries[2] = string_for_pstr<256>(param2);
  param_text_entries[3] = string_for_pstr<256>(param3);
}

std::string replace_param_text(const std::string& text) {
  char prev = 0;
  std::string ret;
  for (size_t z = 0; z < text.size(); z++) {
    char ch = text[z];
    if (ch == '^' && z < text.size() - 1) {
      char index = text[++z];
      if (index == '0' && !param_text_entries[0].empty()) {
        ret += param_text_entries[0];
      } else if (index == '1' && !param_text_entries[1].empty()) {
        ret += param_text_entries[1];
      } else if (index == '2' && !param_text_entries[2].empty()) {
        ret += param_text_entries[2];
      } else if (index == '3' && !param_text_entries[3].empty()) {
        ret += param_text_entries[3];
      } else {
        ret.append(1, '^');
        ret.append(1, (index == '\r') ? '\n' : index);
      }
    } else {
      ret.append(1, (ch == '\r') ? '\n' : ch);
    }
  }
  return ret;
}
