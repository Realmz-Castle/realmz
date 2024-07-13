#include "prototypes.h"
#include "variables.h"

/******************* updatelight ********************/
void updatelight(short who, short offpict) {
  Rect rect;

  if ((who < 0) || (who > charnum))
    return;

  SetPort(GetWindowPort(screen));
  rect.top = 50 * who + 9;
  rect.left = 387 + leftshift;
  rect.right = rect.left + 9;
  rect.bottom = rect.top + 9;

  if (offpict) {
    DrawPicture(non, &rect);
    return;
  }

  if ((up <= c[who].position) && (c[who].position != -1))
    DrawPicture(on, &rect);
  else
    DrawPicture(off, &rect);
}
