#include "prototypes.h"
#include "variables.h"

/******************* bodyground **************/
void bodyground(short body, short where) /***** 0 = look, 1 = buff ******/
{
  short x, y, t, tt;
  Rect groundrect;

  if (!incombat)
    return;

  SetPort(look);

  if (body > 9) {
    body -= 10;
    x = monpos[body][0] - 1;
    y = monpos[body][1] - 1;
    for (t = 0; t < 2; t++) {
      for (tt = 0; tt < 2; tt++) {
        if (monster[body].underneath[t][tt]) {
          groundrect.top = 32 * (y + tt);
          groundrect.left = 32 * (x + t);
          groundrect.right = groundrect.left + 32;
          groundrect.bottom = groundrect.top + 32;
          fastplot(monster[body].underneath[t][tt], groundrect, 0, where);
        }
      }
    }
  } else {
    groundrect.top = 32 * pos[body][1];
    groundrect.left = 32 * pos[body][0];
    groundrect.right = groundrect.left + 32;
    groundrect.bottom = groundrect.top + 32;
    fastplot(charunder[body], groundrect, 0, where);
  }
}
