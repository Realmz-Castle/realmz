#include "prototypes.h"
#include "variables.h"

/*************************** updateitems **********************/
void updateitems(short top, short bottom) {
  short t;

  int enable_recomposite = WindowManager_SetEnableRecomposite(0);

  SetPort(GetWindowPort(itemswindow));
  box.left = 10;
  box.right = 284 + leftshift;
  TextSize(15);
  TextFont(font);
  ForeColor(blackColor);
  characterl = c[charselectnew];

  if (theControl == NIL)
    theControl = monsterbut; /***** keeps ploticon in working order, leave in ***/

  for (t = top; t < bottom + 1; t++) {
    icon.top = 15 + 40 * t;
    icon.bottom = icon.top + 32;
    icon.left = 10;
    icon.right = 42;

    if (characterl.items[t + incr].id) {
      shopequip = characterl.items[t + incr].equip;
      lg = characterl.items[t + incr].ident;
      ploticon(characterl.items[t + incr].id, FALSE);
      if (characterl.items[t + incr].equip)
        lg = 1;
      showcharge2(characterl.items[t + incr].charge, lg);
    } else {
      icon.left = 10;
      icon.right = 284 + leftshift;
      EraseRect(&icon);
      icon.right = 42;
    }
  }

  if (characterl.numitems <= itemselectnew) {
    itemselectnew = characterl.numitems - 1;
    if (itemselectnew == -1)
      itemselectnew++;
    EraseRect(&box3);
    box3.top = 11 + itemselectnew * 40;
    box3.bottom = box3.top + 40;
    DrawPicture(marker, &box3);
  }
  c[charselectnew] = characterl;

  WindowManager_SetEnableRecomposite(enable_recomposite);
}
