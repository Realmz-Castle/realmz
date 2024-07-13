#include "prototypes.h"
#include "variables.h"

/********************************* fastplot ***************/
void fastplot(short id, Rect destrect, short mode, short where) /***** 0 = look, 1 = buff:  From 0 =thePixesl, 1 = Combatpict ******/
{
  if (id < 0)
    return;
  SetPort(GetWindowPort(look));
  ForeColor(blackColor);
  BackColor(whiteColor);

  if (id > 999)
    id -= 1000; /***** to account for bumpup ***/

  tempid = (id - 1) / 20;
  temp = id - (tempid * 20) - 1;
  itemRect.top = 32 * tempid;
  itemRect.left = 32 * temp;
  itemRect.right = itemRect.left + 32;
  itemRect.bottom = itemRect.top + 32;

  if (!where) {
    BitMap* src = GetPortBitMapForCopyBits(gthePixels);
    BitMap* dst = GetPortBitMapForCopyBits(GetWindowPort(look));
    CopyBits(src, dst, &itemRect, &destrect, mode, NIL);
  } else {
    if (where == 1) {
      BitMap* src = GetPortBitMapForCopyBits(gthePixels);
      BitMap* dst = GetPortBitMapForCopyBits(gbuff);
      CopyBits(src, dst, &itemRect, &destrect, mode, NIL);
    }
  }
}
