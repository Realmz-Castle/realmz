#include "prototypes.h"
#include "variables.h"

/************************* showtargets ***************************/
void showtargets(short where) /**************** 0 = look 1 = buff **********/
{
  if (spellinfo.targettype > 1)
    return;
  if (!targetnum)
    return;
  SetPort(GetWindowPort(look));

  for (t = 0; t < targetnum; t++) {
    if (target[t][1] == 1) {
      icon.top = 32 * pos[target[t][0]][1];
      icon.left = 32 * pos[target[t][0]][0];
    } else if (target[t][1] == 2) {
      icon.top = 32 * monpos[target[t][0] - 10][1];
      icon.left = 32 * monpos[target[t][0] - 10][0];
      size = monster[target[t][0] - 10].size;

      if ((size == 1) || (size == 3))
        icon.top -= 16;
      if (size > 1)
        icon.left -= 16;
    } else if (target[t][0] != -1) {
      icon.top = 32 * (target[t][1] - fieldy);
      icon.left = 32 * (target[t][0] - fieldx);

      size = monpick.size;
      if ((size == 1) || (size == 3))
        icon.top -= 16;

      if (size > 1)
        icon.left -= 16;
    }
    icon.bottom = icon.top + 32;
    icon.right = icon.left + 32;
    fastplot(205, icon, 36, where);
  }
}
