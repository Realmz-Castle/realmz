#include "prototypes.h"
#include "variables.h"

/************************ movie ************************/
void movie(short textID, short dataid, short justify) // � 'About...' dialog box.
{
  long mouseuptime = 0;
  long newcount;
  TEHandle textHand;
  Rect txtRect = lookrect;
  WindowRef about; // � Pointer to dialog.
  short x;
  Handle handle;
  StScrpHandle styleHdl;
  GWorldPtr backdrop;
  PixMapHandle backdroppix;
  PixPatHandle backpat;

  GetGWorld(&savedPort, &savedDevice); // � Save the old port info.

  // lookrect.right = lookrect.bottom = 320;
  // lookrect.left = lookrect.top = 0;

  // � Setup scrolling window for text.
  about = GetNewWindow(133, nil, (WindowPtr)-1);
  SetPort(GetWindowPort(about));
  ForeColor(blackColor);
  BackColor(whiteColor);
  backpat = GetPixPat(dataid);
  BackPixPat(backpat);
  TextSize(10);
  TextFont(genevafont);
  TextMode(1);
  if (FrontWindow() != GetDialogWindow(background))
    MoveWindow(about, GlobalLeft, GlobalTop, TRUE);
  ShowWindow(about);
  EraseRect(&lookrect);

  GetGWorld(&savedPort, &savedDevice); /**** create offworld text area *****/
  NewGWorld(&backdrop, 8, &lookrect, 0L, 0L, 0L);
  SetGWorld(backdrop, NIL);
#ifndef PC // Myriad
  backdroppix = GetPortPixMap(backdrop);
  LockPixels(backdroppix);
#endif
  BackPixPat(backpat);
  ForeColor(blackColor);
  BackColor(whiteColor);
  TextMode(1);

  EraseRect(&lookrect);

  // � Create styled TERecord.
  textHand = TEStyleNew(&txtRect, &txtRect);

  // � Read the TEXT resource.
  handle = GetResource('TEXT', textID);
  HLock(handle); // � Lock handle.

  // � Get the style handle.
  styleHdl = (StScrpHandle)(GetResource('styl', textID));
#ifdef PC
  AcamCvtStScrpHandle(styleHdl);
#endif

  TEStyleInsert(*handle, GetResourceSizeOnDisk(handle), styleHdl, textHand); // � move text into text record.

  TESetAlignment(justify, textHand);
  HUnlock(handle); // � Unlock handle.
  ReleaseResource((Handle)handle); // Myriad : more clean...
  ReleaseResource((Handle)styleHdl); // Myriad : more clean...

  TEUpdate(&txtRect, textHand); // � Draw text in viewRect.
  InsetRect(&txtRect, -5, -1); // � Leave margins for text.

  ShowWindow(about); // � Show dialog box now.

backtoit:

  if (!Button()) {
    BitMap* src = GetPortBitMapForCopyBits(backdrop);
    BitMap* dst = GetPortBitMapForCopyBits(GetWindowPort(about));

    newcount = TickCount();
    // � scroll up a pixel
    TEScroll(0, -1, textHand);
    EraseRect(&lookrect);
    TEUpdate(&txtRect, textHand); // � Draw text in viewRect.
    CopyBits(src, dst, &lookrect, &lookrect, 0, NIL);
    do {

    } while (TickCount() < newcount + 3);

    SystemTask();
    x = GetNextEvent(everyEvent, &gTheEvent);
    MyrCheckMemory(2);
#ifdef PC // Myriad
    DoCorrectBugMADRepeat();
#endif

    switch (gTheEvent.what) {
      case mouseDown:
        if (gTheEvent.when <= mouseuptime)
          goto out;
        break;

      case mouseUp:
        mouseuptime = gTheEvent.when + GetDblTime();
        break;
    }

    while (StillDown()) {
      BitMap* src = GetPortBitMapForCopyBits(backdrop);
      BitMap* dst = GetPortBitMapForCopyBits(GetWindowPort(about));
      GetMouse(&point);
      if (point.v < gTheEvent.where.v)
        TEScroll(0, -25, textHand);
      else if (point.v > gTheEvent.where.v)
        TEScroll(0, 25, textHand);

      gTheEvent.where.v = point.v;
      EraseRect(&lookrect);
      TEUpdate(&txtRect, textHand); // � Draw text in viewRect.
      CopyBits(src, dst, &lookrect, &lookrect, 0, NIL);
    }

  } else {
  }

  goto backtoit;

out:
#ifndef PC
  UnlockPixels(backdroppix);
#endif
  DisposeGWorld(backdrop);
  DisposePixPat(backpat);
  TEDispose(textHand); // � Reclaim heap space.
  DisposeWindow(about); // � Get rid of dialog box.
  SetGWorld(savedPort, savedDevice); // � Restore the old port.
}
