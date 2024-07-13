#include "prototypes.h"
#include "variables.h"

/************************ centerfield ************************/
void centerfield(short x, short y) {
  register t, tt;
  char newx, newy;
  short tempicon, ten, single;
  char bq[maxloop];

  if (!incombat) {
    centerpict();
    return;
  }

  // SelectWindow(look);
  SetPort((GrafPtr)GetWindowPort(look));

  lookrect.left = 0;

  for (t = 0; t < maxloop; t++)
    bq[t] = 0;

  newx = fieldx + x - 7;
  newy = fieldy + y - 6;

  if (newx < 0)
    x -= newx;
  if (newy < 0)
    y -= newy;

  if (newx > 75)
    x -= (newx - 75);
  if (newy > 76)
    y -= (newy - 76);

  fieldx += (x - 7);
  fieldy += (y - 6);

  for (t = 0; t <= charnum; t++) {
    pos[t][0] -= (x - 7);
    pos[t][1] -= (y - 6);
  }

  for (t = 0; t < maxmon; t++) {
    monpos[t][0] -= (x - 7);
    monpos[t][1] -= (y - 6);
  }

  SetPort((GrafPtr)GetWindowPort(look));

  icon.top = -32;
  icon.bottom = 0;

  ForeColor(blackColor);
  BackColor(whiteColor);

  for (tt = fieldy; tt < fieldy + 14; tt++) // Myriad (+11)
  {
    icon.top += 32;
    icon.bottom += 32;
    icon.left = -32;
    icon.right = 0;
    for (t = fieldx; t < fieldx + 16; t++) // Myriad (+11)
    {
      icon.left += 32;
      icon.right += 32;
      tempicon = field[t][tt];
      point.h = t;
      point.v = tt;

      if (tempicon > 999) {
        BitMap* src = GetPortBitMapForCopyBits(gthePixels);
        BitMap* dst = GetPortBitMapForCopyBits(gbuff);
        tempicon -= 1000;
        ten = (tempicon - 1) / 20;
        single = tempicon - (ten * 20) - 1;
        itemRect.top = 32 * ten;
        itemRect.left = 32 * single;
        itemRect.right = itemRect.left + 32;
        itemRect.bottom = itemRect.top + 32;
        CopyBits(src, dst, &itemRect, &icon, 0, NIL);

      } else if (tempicon > -1) {
        bodyground(tempicon, 1);
        bq[tempicon] = TRUE;
      }
    }
  }
  showque();
  itemRect.top = itemRect.left = -32;
  itemRect.right = itemRect.bottom = 0;
  for (t = 0; t < maxloop; t++)
    if (bq[t])
      drawbody(t, inspell, 1);

  if (inspell)
    showtargets(1);

  if ((q[up] < 9) && (usehashmarks)) {
    BitMap* buf = GetPortBitMapForCopyBits(gbuff);
    BitMap* edgepix = GetPortBitMapForCopyBits(gedge);
    BitMap* lookpix = GetPortBitMapForCopyBits(GetWindowPort(look));

    CopyBits(buf, edgepix, &lookrect, &lookrect, 0, NIL);
    showrange(TRUE);
    InsetRect(&lookrect, 10, 10);
    CopyBits(buf, edgepix, &lookrect, &lookrect, 0, NIL);
    InsetRect(&lookrect, -10, -10);
    CopyBits(edgepix, lookpix, &lookrect, &lookrect, 0, NIL);
  } else {
    BitMap* buf = GetPortBitMapForCopyBits(gbuff);
    BitMap* lookpix = GetPortBitMapForCopyBits(GetWindowPort(look));
    CopyBits(buf, lookpix, &lookrect, &lookrect, 0, NIL);
  }

  SetPort((GrafPtr)GetWindowPort(screen));
}
