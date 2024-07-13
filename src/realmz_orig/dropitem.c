#include "prototypes.h"
#include "variables.h"

/************************* dropitem **********************/
void dropitem(short who, short itemid, short itemnum, short dropsound, short force) {
  Boolean todrop = 1;
  short t;

  if ((who < 0) || (who > charnum))
    return;

  loaditem(itemid);
  if (dropsound)
    sound(655); /***** drop sound *****/

  if (c[who].items[itemnum].equip) {
    if (!removeitem(who, itemnum, FALSE, force))
      todrop = FALSE;
  }

  if (todrop) {
    c[who].load -= (item.wieght + c[who].items[itemnum].charge * item.xcharge);
    for (t = itemnum; t < 29; t++)
      c[who].items[t] = c[who].items[t + 1];
    c[who].numitems--;
    c[who].items[29].id = c[who].items[29].ident = c[who].items[29].equip = c[who].items[29].charge = 0;
    movecalc(who);
  }
}
