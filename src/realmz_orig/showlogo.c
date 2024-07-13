#include "prototypes.h"
#include "realmzbuild.h"
#include "variables.h"

/************** showlogo **************************/
void showlogo(short howlong) {
  Rect baserect;
  WindowPtr mat2;
  DialogRef logowin;

  if (nologo)
    return;

  HideCursor();

  if ((!nofade) && (!nologo)) {
    sound(624);
    fadeinout(25, fadeout); /******** fade to black ******/
  }

  hideMenuBar();
  mat2 = GetNewWindow(132, 0L, (WindowPtr)-1L);
  SizeWindow(mat2, width, depth, FALSE);

  logowin = GetNewDialog(175, NIL, (WindowPtr)-1L);
  MoveWindow(GetDialogWindow(logowin), GlobalLeft + 1 + (leftshift / 2), GlobalTop + 1 + (downshift / 2), FALSE);
#ifdef PC
  SetPort(mat2);
  BackColor(blackColor);
  EraseRect(&qd.thePort->portRect);

  SetPort(logowin);
#endif
  DrawDialog(logowin);

  baserect.top = baserect.left = 0;
  baserect.right = 640;
  baserect.bottom = 460;

  if ((!nofade) && (!nologo))
    fadeinout(125, fadein); /******** fade to black ******/

  if (howlong < 0) {
    ShowCursor();
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
  } else if ((howlong) && (!development))
    delay2(howlong);

  if ((!nofade) && (!nologo)) {
    sound(624);
    fadeinout(50, fadeout); /******** fade to black ******/
    delay(20);
  }

  showMenuBar();
  DrawMenuBar();
  DisposeWindow(mat2);
  DisposeDialog(logowin);
  ShowCursor();
}

/************** delay2 ***********************/
void delay2(short timedelay) {
  long oldtick;

  if (!timedelay)
    timedelay = delayspeed;
  oldtick = TickCount();

  while (!Button())
    if (TickCount() - oldtick > timedelay)
      return;
}

/***************** hideMenuBar *********************************/
void hideMenuBar(void) {
  GDHandle mainScreen; // the information on the main screen
  Rect mainScreenRect; // the rect that bounds the menu bar
  RgnHandle mainScreenRgn; // the region of the menu bar

  // record the menu bar height
  sMenuBarHeight = GetMBarHeight();

// set the menu bar to no height
#ifndef CARBON
  LMSetMBarHeight(0);
#else
  HideMenuBar();
#endif

  // save the original gray regions (so we can restore it later)
  sOriginalGrayRgn = NewRgn();
  CopyRgn(GetGrayRgn(), sOriginalGrayRgn);

  // make sure that the entire main screen is ours to play with (no menu bars or corners)
  mainScreenRgn = NewRgn();
  mainScreen = GetMainDevice();
  mainScreenRect = ((*mainScreen)->gdRect);
  RectRgn(mainScreenRgn, &mainScreenRect);
  UnionRgn(sOriginalGrayRgn, mainScreenRgn, GetGrayRgn());

  // draw the desktop
  PaintOne(NIL, sOriginalGrayRgn);
}

/***************** showMenuBar *********************************/
void showMenuBar(void) {
// set the menu bar to its normal height
#ifndef CARBON
  LMSetMBarHeight(sMenuBarHeight);
#else
  ShowMenuBar();
#endif

  // restore the original gray region
  SectRgn(sOriginalGrayRgn, GetGrayRgn(), GetGrayRgn());

  // draw the menu bar
  DrawMenuBar();
}
