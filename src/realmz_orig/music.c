#include "prototypes.h"
#include "variables.h"
/* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
 * NOTE(iSynic): Use the SDL music interface and synchronize the restored menu.
 */
#include "MusicManager.h"

static void setmusicmenutext(const char* text) {
  Str255 title;

  strcpy((Ptr)title, text);
  CtoPstr((Ptr)title);
  SetMenuItemText(musicmenu, 5, title);
}

void syncmusicmenu(void) {
  short item;
  Str255 path;
  char* name;

  if (!musicmenu || (CountMItems(musicmenu) < 27))
    return;

  if (nomusic) {
    setmusicmenutext("Music Disabled In Preferences");
    for (item = 1; item < 28; item++)
      DisableItem(musicmenu, item);
    EnableItem(musicmenu, 2);
    EnableItem(musicmenu, 5);
  } else {
    if ((currentplay > 0) && (currentplay < 18)) {
      GetIndString(path, 140, currentplay);
      PtoCstr(path);
      name = strrchr((Ptr)path, ':');
      setmusicmenutext(name ? name + 1 : (Ptr)path);
    } else
      setmusicmenutext("No Music Selected");
    for (item = 1; item < 25; item++)
      EnableItem(musicmenu, item);
    DisableItem(musicmenu, 3);
    DisableItem(musicmenu, 4);
    DisableItem(musicmenu, 5);
    DisableItem(musicmenu, 6);
    DisableItem(musicmenu, 7);
    DisableItem(musicmenu, 25);
    DisableItem(musicmenu, 26);
    DisableItem(musicmenu, 27);
  }
  CheckItem(musicmenu, 1, (!nomusic) && (!Stopmusic));
}
/* *** END CHANGES *** */

/******************************* music ************************/
/* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
 * NOTE(iSynic): Restore playback through the modern SDL audio backend while
 * preserving Realmz's saved playlist state.
 */
void music(short playlist) {
  if ((playlist < 1) || (playlist > 20) || nomusic || Stopmusic || !musictoggle[playlist - 1]) {
    RealmzMusicStop();
    currentplay = -1;
    musicplaying = FALSE;
  } else if (musictoggle[playlist - 1] == 2)
    return;
  else {
    musicplaying = RealmzMusicPlay(playlist, scenarioname);
    currentplay = musicplaying ? playlist : -1;
  }
  syncmusicmenu();
}
/* *** END CHANGES *** */
