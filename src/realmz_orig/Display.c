#include "prototypes.h"
#include "variables.h"

/***************************** Display ****************************/
void Display(short mode) {
  short t, charge;
  short start, stop;

  compactheap();
  music(8); /******* shop music ********/

  SetPort(gshop);
  updateshop();
  updateshopwings(cl, cr);
  if (theControl != charitemsvert)
    theControl = shopitemsvert;
  curControlValue = GetControlValue(theControl);

  icon.left = 390;

  start = 0;
  stop = 9;

  if ((displaytag) && (!mode)) {
    start = gshopitem - 1;
    if (displaytag == 1)
      stop = gshopitem;
  }

  displaytag = 0;

  for (t = start; t < stop; t++) {
    charge = 0;
    if (theControl == shopitemsvert) {
      if (shop) {
        tempid = theshop.id[shopselection + t + curControlValue];
        if (tempid < 0)
          return;
        if (tempid < 2000) {
          loaditem(tempid);
          charge = item.charge;
        }
        lg = TRUE;
      } else {
        tempid = characterr.items[curControlValue + t].id;
        if (t > characterr.numitems - 1)
          tempid = 0;
        lg = characterr.items[curControlValue + t].ident;
        charge = characterr.items[curControlValue + t].charge;
        shopequip = characterr.items[curControlValue + t].equip;
      }
    } else {
      icon.left = 10;
      tempid = characterl.items[curControlValue + t].id;
      if (t > characterl.numitems - 1)
        tempid = 0;
      lg = characterl.items[curControlValue + t].ident;
      charge = characterl.items[curControlValue + t].charge;
      shopequip = characterl.items[curControlValue + t].equip;
    }

    icon.top = 16 + t * 40;
    if ((cr == -1) && (theControl == shopitemsvert))
      ploticon(tempid, TRUE);
    else
      ploticon(tempid, FALSE);

    if (tempid)
      showcharge2(charge, lg);

    shopequip = FALSE;
  }

  if ((skiptest) || (skip))
    return;
}
