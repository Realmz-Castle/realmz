#pragma once

#include <SDL3/SDL_events.h>
#include <phosg/Image.hh>

// In-window menu bar for platforms without a native one (Linux). The game lays out its windows
// for a Mac screen whose top kMenuBarHeight rows belong to the menu bar (see GlobalTop in
// main.c), so the bar is drawn directly into those rows of the composited screen.

inline constexpr int kMenuBarHeight = 20;

// Draws the menu bar into the top rows of the composited screen. Called after each recomposite.
void MenuBar_Draw(phosg::ImageRGBA8888N& screen);

// Handles clicks in the menu bar and Ctrl+key menu shortcuts. The event must already be in render
// (logical screen) coordinates. Returns true if the event was consumed.
bool MenuBar_HandleEvent(const SDL_Event& e);
