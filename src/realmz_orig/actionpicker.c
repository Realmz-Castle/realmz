#include "prototypes.h"
#include "variables.h"

/**************************************** actionpicker ********************************/
short actionpicker(void) {
  FILE* fp = NULL;
  char count = 0;
  char countmax = 0;
  Rect temp;
  CCrsrHandle counter;

  in();
  for (t = 0; t < 8; t++)
    if (enc2.group[t])
      countmax++;
  if (!count)
    count = 1;
  count = countmax;
  counter = GetCCursor(147 + count);
  SetCCursor(counter);
  DisposeCCursor(counter);

  gGeneration = GetNewDialog(148, 0L, (WindowPtr)-1L);
  gCurrent = gGeneration;
  SetPortDialogPort(gGeneration);
  BackPixPat(base);
  TextFont(defaultfont);
  ForeColor(blackColor);
  BackColor(whiteColor);

  MoveWindow(GetDialogWindow(gGeneration), GlobalLeft + 407 + leftshift, GlobalTop + 4, FALSE);
  ShowWindow(GetDialogWindow(gGeneration));
  ErasePortRect();

  GetDialogItem(gGeneration, 9, &itemType, &itemHandle, &temp);
  pict(156, temp);
  GetDialogItem(gGeneration, 1, &itemType, &itemHandle, &itemRect);
  pict(179, itemRect);

  itemRect = temp;

  for (t = 1; t < 8; t++) {
    BitMapPtr src = GetPortBitMapForCopyBits(GetDialogPort(gGeneration));
    itemRect.top += 50;
    itemRect.bottom += 50;
    CopyBits(src, src, &temp, &itemRect, 0, NIL);
  }
  itemRect = temp;
  ForeColor(yellowColor);
  RGBBackColor(&greycolor);
  for (t = 0; t < 8; t++) {
    actiontaken[t] = 0;
    MoveTo(itemRect.left + 30, itemRect.top + 30);
    itemRect.top += 50;
    MyrPascalDiStr(19 + t, buffer[t]);
  }

  for (;;) {
  over:

    if (count < 0)
      counter = GetCCursor(147);
    else
      counter = GetCCursor(147 + count);
    SetCCursor(counter);
    DisposeCCursor(counter);

    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(gGeneration, itemHit, &itemType, &itemHandle, &buttonrect);

    if (itemHit == 17) {
      if (count) {
        SetPort(GetWindowPort(look));
        warn(58);
        SetPortDialogPort(gGeneration);
        goto over;
      } else {
        ploticon3(129, buttonrect);
        reply = TRUE;
        goto out;
      }
    }

    if (itemHit == 18) {
      ploticon3(129, buttonrect);
      reply = FALSE;
      goto out;
    } else {
      GetDialogItem(gGeneration, itemHit - 8, &itemType, &itemHandle, &itemRect);
      itemRect.top += 2;
      itemRect.left += 2;
      itemRect.right -= 2;
      itemRect.bottom -= 2;
      if (actiontaken[itemHit - 9]) {
        DrawPicture(non, &itemRect);
        sound(144);
        actiontaken[itemHit - 9] = FALSE;
        count++;
      } else {
        DrawPicture(on, &itemRect);
        sound(145);
        actiontaken[itemHit - 9] = TRUE;
        count--;
      }
    }
  }
out:
  DisposeDialog(gGeneration);
  SetCCursor(sword);
  out();
  return (reply);
}
