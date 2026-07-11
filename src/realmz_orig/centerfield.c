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
  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * Clamp fieldy at 77 rather than 76 so the last row of the field can be
   * scrolled into view. The look window shows 15 columns by 13 rows, so the
   * bottom visible row is fieldy + 12; stopping fieldy at 76 left row 89
   * unreachable while the fieldx clamp of 75 already allowed column 89.
   * centerpict clamps at 75 and 77 for the same window, which is the pattern
   * followed here. The redraw loop below bounds its indices to the field, so
   * the extra row it scans past the window stays in range. */
  if (newy > 77)
    y -= (newy - 77);
  /* *** END CHANGES *** */

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

  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * Stop both loops at the edge of the field. They scan 16 columns by 14 rows
   * starting at fieldx, fieldy, one more than the 15 by 13 the look window can
   * show, so the last column and row are drawn outside lookrect and never
   * reach the screen. With fieldx at its clamp of 75 the column loop ran to
   * t == 90, and field is short[90][90], so field[90][tt] read past the end of
   * the array into the globals that follow it. Whatever was there became
   * tempicon, and any value from 0 to 999 was then treated as a body id: an
   * in-range field cell only ever holds terrain (1000 and up) or a real body
   * id (0 to maxloop-1), never anything between. That is where the stray id
   * came from that made bq[tempicon] = TRUE overwrite the stack. */
  for (tt = fieldy; (tt < fieldy + 14) && (tt < 90); tt++) // Myriad (+11)
  {
    icon.top += 32;
    icon.bottom += 32;
    icon.left = -32;
    icon.right = 0;
    for (t = fieldx; (t < fieldx + 16) && (t < 90); t++) // Myriad (+11)
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
        /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
         * Bound the body id before using it to index bq and the body arrays.
         * bq is char[maxloop] and only body ids 0..maxloop-1 are valid (party
         * 0..8, monsters 10..10+maxmon-1), so any larger id writes past the end
         * of bq on the stack. Bounding the loops above stops the field read
         * that produced such an id, and the redraw loop below already bounds
         * its index with t < maxloop; keep the write to bq in range too, since
         * a bad id from any other source would smash the stack here. */
        if (tempicon < maxloop) {
          bodyground(tempicon, 1);
          bq[tempicon] = TRUE;
        }
        /* *** END CHANGES *** */
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
