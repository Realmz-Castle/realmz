#include "prototypes.h"
#include "variables.h"

/**************** compactitems *****************/
void compactitems(short who) {
  struct item tempitem;

  if ((who < 0) || (who > charnum))
    return;

  for (tt = 1; tt < c[who].numitems; tt++) {
    for (t = 1; t < c[who].numitems; t++) {
      if ((c[who].items[t].equip) && (!c[who].items[t - 1].equip)) {
        tempitem = c[who].items[t - 1];
        c[who].items[t - 1] = c[who].items[t];
        c[who].items[t] = tempitem;
      }
    }
  }
  for (t = c[who].numitems; t < 30; t++) {
    c[who].items[t] = blank100;
  }
}
