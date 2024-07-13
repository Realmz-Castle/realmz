#include "prototypes.h"
#include "variables.h"

/*************************** showque ***************************/
void showque(void) {
  register char t, tt;
  short startx, starty, showloop;
  Rect temprect;
  short spellsize;

  for (showloop = 0; showloop < 60; showloop++) {
    if (que[showloop].duration) {
      spellsize = que[showloop].size - 1;

      startx = que[showloop].x;
      starty = que[showloop].y;

      temprect.top = (starty - fieldy) * 32 - 32;
      temprect.bottom = temprect.top + 32;

      for (t = 0; t < 7; t++) {
        temprect.top += 32;
        temprect.bottom += 32;
        temprect.left = (startx - fieldx) * 32 - 32;
        temprect.right = temprect.left + 32;
        for (tt = 0; tt < 7; tt++) {
          temprect.left += 32;
          temprect.right += 32;
          if (spellarea[spellsize][t][tt]) {
            if (field[tt + startx][t + starty] > 999) {
              if (!mapstats[field[tt + startx][t + starty] - 1000].solid) {
                fastplot(200 + que[showloop].icon, temprect, 36, 1);
              }
            }
          }
        }
      }
    }
  }
}
