#include "prototypes.h"
#include "variables.h"
#include "MusicManager.h"

/* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
 * NOTE(iSynic): Keep the restored Music menu synchronized with playback.
 */
static const char* const musicnames[] = {
    "Outdoor Music", "Dungeon Music", "Indoor Music", "Cave Music", "Create Music", "Items Music",
    "Treasure Music", "Shop Music", "Camp Music", "Temple Music", "Battle Music", "Outdoor Music (Desert)",
    "Outdoor Music (Swamp)", "Outdoor Music (Snow)", "Custom 1 Music", "Custom 2 Music", "Custom 3 Music"};

static void setmusicmenutext(const char* text) {
  Str255 title;

  strcpy((Ptr)title, text);
  CtoPstr((Ptr)title);
  SetMenuItemText(musicmenu, 5, title);
}

void syncmusicmenu(void) {
  short item;

  if (!musicmenu || (CountMItems(musicmenu) < 27))
    return;

  if (nomusic) {
    setmusicmenutext("Music Disabled In Preferences");
    for (item = 1; item < 28; item++)
      DisableItem(musicmenu, item);
    EnableItem(musicmenu, 2);
    EnableItem(musicmenu, 5);
  } else {
    if ((currentplay > 0) && (currentplay < 18))
      setmusicmenutext(musicnames[currentplay - 1]);
    else
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
  short toggle;

  if ((playlist < 1) || (playlist > 20) || nomusic || Stopmusic) {
    RealmzMusicStop();
    currentplay = -1;
    musicplaying = FALSE;
    syncmusicmenu();
    return;
  }

  toggle = musictoggle[playlist - 1];
  if (toggle == 2)
    return;
  if (!toggle) {
    RealmzMusicStop();
    currentplay = -1;
    musicplaying = FALSE;
    syncmusicmenu();
    return;
  }
  if (currentplay == playlist) {
    RealmzMusicPlay(playlist, scenarioname);
    return;
  }

  if (RealmzMusicPlay(playlist, scenarioname)) {
    currentplay = playlist;
    musicplaying = TRUE;
    syncmusicmenu();
  }
}
/* *** END CHANGES *** */
