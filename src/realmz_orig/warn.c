#include "variables.h"

/************ warn ***********************/
void warn(short string) {
  FILE* fp = NULL;
  RGBColor fore, back;
  DialogRef warning;
  GrafPtr oldport;
  Rect blit, itemRect;

  BitMap* src;
  BitMap* dst = GetPortBitMapForCopyBits(gbuff2);

  GetPort(&oldport);
  if (oldport) // Myriad, current port must be null
  {
    GetForeColor(&fore);
    GetBackColor(&back);
  }
  SetCCursor(sword);

  blit.top = 0;
  blit.bottom = 62;
  blit.left = 0;
  blit.right = 261;

  itemRect.top = 40;
  itemRect.left = 30;
  itemRect.right = 291;
  itemRect.bottom = 102;

  OffsetRect(&itemRect, GlobalLeft, GlobalTop);

  if (look)
    SetPort(GetWindowPort(screen));

  if (inswap)
    SetPortDialogPort(gswap);
  else if (inbooty)
    SetPort(GetWindowPort(bootywindow));
  else if (initems)
    SetPort(GetWindowPort(itemswindow));
  else if (inshop)
    SetPort(GetWindowPort(gshop));
  else if (intemple)
    SetPortDialogPort(templewindow);
  else if (spellwindow)
    SetPortDialogPort(spellwindow);
  else if (inscroll)
    SetPortDialogPort(scroll);
  else if (background)
    SetPortDialogPort(background);
  else if (about)
    SetPortDialogPort(about);
  else if (screen)
    SetPort(GetWindowPort(screen));

  ForeColor(blackColor);
  BackColor(whiteColor);

  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(fuzziqersoftware): Realmz tries to avoid redrawing the entire
   * screen by just copying out the part of the screen that the message
   * window would overlap. However, our virtual windowing system maintains
   * window contents for obscured windows, so this is unnecessary in our
   * implementation.
   */
  // // GetQDGlobalsScreenBits(&src);
  // src = GetPortBitMapForCopyBits(GetQDGlobalsThePort());
  // CopyBits(src, dst, &itemRect, &blit, 0, NIL);
  /* *** END CHANGES *** */

  GetIndString(myString, 3, abs(string));
  ParamText(myString, (StringPtr) "", (StringPtr) "", (StringPtr) "");

  warning = GetNewDialog(140, 0L, (WindowPtr)-1L);
  SetPortDialogPort(warning);
  BackPixPat(base);
  TextFont(defaultfont);
  SetCCursor(mouse);
  ForeColor(yellowColor);
  MoveWindow(GetDialogWindow(warning), GlobalLeft + 31, GlobalTop + 41, FALSE);
  ShowWindow(GetDialogWindow(warning));
  ErasePortRect();
  DrawDialog(warning);
  sound(6000);
  delay(10);
  FlushEvents(everyEvent, 0);

  for (;;) {
    WaitNextEvent(everyEvent, &gTheEvent, 0L, 0L);
#ifdef PC // Myriad
    DoCorrectBugMADRepeat();
#endif

    if ((gTheEvent.what == mouseDown) || (gTheEvent.what == keyDown))
      break;
  }

  DisposeDialog(warning);
  SetCCursor(sword);
  itemHit = tagnew = 0;
  SetPort(oldport);
  if (oldport) // Myriad : current port can be null
  {
    ForeColor(blackColor);
    BackColor(whiteColor);
  }

  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(fuzziqersoftware): Realmz tries to avoid redrawing the entire
   * screen by just copying out the part of the screen that the message
   * window would overlap. However, our virtual windowing system maintains
   * window contents for obscured windows, so this is unnecessary in our
   * implementation.
   */
  // src = GetPortBitMapForCopyBits(GetQDGlobalsThePort());
  // CopyBits(dst, src, &blit, &itemRect, 0, NIL);
  /* *** END CHANGES *** */

  if (oldport) // Myriad : current port can be null
  {
    RGBBackColor(&back);
    RGBForeColor(&fore);
  }
  FlushEvents(everyEvent, 0);
}
