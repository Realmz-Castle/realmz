#include "prototypes.h"
#include "variables.h"

//
// Myriad : As the /p don't work on the PC side, i change
// the first parameter to a C string (6-7-99)
//

/************ flashmessage ***********************/
void flashmessage(Str255 strc, short x, short y, short duration, short toplay) /********** - duration = putup toggle ***** 0 duration = Wait for click *****/
{
  long oldtick;
  Rect blit, itemRect;
  char string[255];

  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(danapplegate): Realmz often calls this flashmessage function and passes in
   * string literals as the strc argument. It then copies 255 bytes from that location
   * in program memory, regardless of the length of the string. This causes invalid access
   * to program memory beyond the bounds of the string, and could cause undefined behavior.
   * For safety here, we limit the number of bytes copied to the length of the string.
   */
  // BlockMove(strc, string, 255);
  BlockMove(strc, string, strlen((const char*)strc) + 1);
  /* *** END CHANGES *** */
  CtoPstr(string);

  blit.top = 0;
  blit.bottom = 62;
  blit.left = 0;
  blit.right = 260;

  itemRect.top = y - 1;
  itemRect.left = x - 1;
  itemRect.right = x + 261;
  itemRect.bottom = y + 61;

  OffsetRect(&itemRect, GlobalLeft, GlobalTop);

  if (look != NIL)
    SetPort(GetWindowPort(look));

  ForeColor(blackColor);
  BackColor(whiteColor);

  if (toplay)
    sound(toplay);

  if (!putup) {
    BitMap* src = GetPortBitMapForCopyBits(GetQDGlobalsThePort());
    // GetQDGlobalsScreenBits(&src);
    BitMap* dst = GetPortBitMapForCopyBits(gbuff2);
    CopyBits(src, dst, &itemRect, &blit, 0, NIL);

    ParamText((StringPtr) "", (StringPtr)string, (StringPtr) "", (StringPtr) "");

    flashwindow = GetNewDialog(156, 0L, (WindowPtr)-1L);

    SetPortDialogPort(flashwindow);
    BackPixPat(base);
    TextFont(defaultfont);
    ForeColor(yellowColor);
    MoveWindow(GetDialogWindow(flashwindow), x + GlobalLeft, y + GlobalTop, FALSE);
    ShowWindow(GetDialogWindow(flashwindow));
    ErasePortRect();
    DrawDialog(flashwindow);
  }

  if (duration > 0) {
    oldtick = TickCount();

    while (oldtick + duration != TickCount()) {
    }
  } else if (duration < 0) {
    if (putup) {
      putup = FALSE;
      goto out;
    } else {
      putup = TRUE;
      return;
    }
  } else {
    SetCCursor(mouse);
    delay(15);
    FlushEvents(everyEvent, 0);
    for (;;) {
      WaitNextEvent(everyEvent, &gTheEvent, 0L, 0L);
#ifdef PC // Myriad
      DoCorrectBugMADRepeat();
#endif

      if ((gTheEvent.what == mouseDown) || (gTheEvent.what == keyDown))
        break;
    }
    FlushEvents(everyEvent, 0);
    SetCCursor(sword);
  }

out:

  if (flashwindow != NIL) {
    DisposeDialog(flashwindow);
    flashwindow = NIL;
  }

  if (look != NIL) {
    SetPort(GetWindowPort(look));
    ForeColor(blackColor);
    BackColor(whiteColor);
  }
  // Myriad : the qd.thePort is closed
  if (background != NIL)
    SetPortDialogPort(background);

  {
    BitMap* src = GetPortBitMapForCopyBits(gbuff2);
    BitMap* dst = GetPortBitMapForCopyBits(GetQDGlobalsThePort());
    // GetQDGlobalsScreenBits(&dst);
    CopyBits(src, dst, &blit, &itemRect, 0, NIL);
  }
}
