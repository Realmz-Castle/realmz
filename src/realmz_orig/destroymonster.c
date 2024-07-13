#include "prototypes.h"
#include "variables.h"

/************************** destroymonster *******************/
void destroymonster(short tt) {
  short ttt, size;
  Rect itemRect;

  monster[tt].stamina = 1;
  center(tt + 10);
  monster[tt].stamina = 0;
  size = monster[tt].size;
  itemRect.top = monpos[tt][1] * 32;
  itemRect.left = monpos[tt][0] * 32;
  if (monster[tt].traiter)
    killmon++;
  monster[tt].movement = monster[tt].traiter = 0;
  for (ttt = 0; ttt < 40; ttt++)
    monster[tt].condition[ttt] = 0;

  itemRect.bottom = itemRect.top + 32;
  itemRect.right = itemRect.left + 32;

  if ((size == 1) || (size == 3))
    itemRect.top -= 32;
  if (size > 1)
    itemRect.left -= 32;

  SetPort((GrafPtr)GetWindowPort(look));

  iconhand = NIL;

  iconhand = GetCIcon(2015 + size);
  if (iconhand) {
    PlotCIcon(&itemRect, iconhand);
    DisposeCIcon(iconhand);
  }

  for (ttt = 1; ttt < maxloop; ttt++) {
    if (q[ttt] == (tt + 10)) {
      q[ttt] = -1;
    }
  }

  showresults(tt + 10, -14, tt + 10); /****** destroyed *****/

  bodyground(tt + 10, 0);
  bodyfield(tt + 10);

  for (ttt = 0; ttt < nummon; ttt++)
    if (monster[ttt].target == tt)
      monster[ttt].target = -1;
}
