#include "prototypes.h"
#include "variables.h"
#include "MusicManager.h"

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
    return;
  }

  toggle = musictoggle[playlist - 1];
  if (toggle == 2)
    return;
  if (!toggle) {
    RealmzMusicStop();
    currentplay = -1;
    musicplaying = FALSE;
    return;
  }
  if (currentplay == playlist) {
    RealmzMusicPlay(playlist, scenarioname);
    return;
  }

  if (RealmzMusicPlay(playlist, scenarioname)) {
    currentplay = playlist;
    musicplaying = TRUE;
  }
}
/* *** END CHANGES *** */
