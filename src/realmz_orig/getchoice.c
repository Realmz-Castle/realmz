#include "prototypes.h"
#include "variables.h"

/************************** getchoice ***************************/
short getchoice(short number, short initems, short candead) {
  char loop, count, testvalue;
  short key, tempkillparty = 0;
  CCrsrHandle compassnew;
  Rect choicerect;
  choicerect.top = 0;
  choicerect.bottom = 50 * (charnum + 1);
  choicerect.left = 332 + leftshift;
  choicerect.right = choicerect.left + 60;

  if (!candead)
    tempkillparty = killparty;

  if (number > charnum - tempkillparty)
    number -= (number - (charnum - tempkillparty));

  count = number + 1;
  for (loop = 0; loop < 8; loop++)
    track[loop] = 0;

  compassnew = GetCCursor(147 + count);
  SetCCursor(compassnew);
  DisposeCCursor(compassnew);
  for (;;) {
  loopback:

    WaitNextEvent(everyEvent, &gTheEvent, 0L, 0L);
#ifdef PC // Myriad
    DoCorrectBugMADRepeat();
#endif

    if (gTheEvent.modifiers & alphaLock)
      warn(21);

    switch (gTheEvent.what) {
      case keyDown:
        key = gTheEvent.message & charCodeMask;

        if (key == 'a') /**** a ** abort **************/
        {
          itemHit = -1;
          for (t = 0; t < 8; t++)
            track[t] = 0;
          inspell = 0;
          goto abort;
        }

        if ((key == ' ') && (inspell)) /***** space *********** cast *****/
        {
          goto abort;
        }
        break;

      case mouseDown:
        point = gTheEvent.where;
        GlobalToLocal(&(point));
        if (PtInRect(point, &choicerect)) {
          itemHit = point.v / 50;
          itemRect.top = itemHit * 50;
          itemRect.left = 330 + leftshift;
          itemRect.right = itemRect.left + 50;
          itemRect.bottom = itemRect.top + 50;
          buttonrect = itemRect;
          goto bustout;
        } else {
          sound(6000);
          goto loopback;
        }
        break;
    }
  }

bustout:

  if ((!candead) && (c[itemHit].stamina < 1)) {
    sound(6000);
    goto loopback;
  }

  ploticon3(129, buttonrect);
  if (!track[itemHit]) {
    count--;
    sound(141);
    compassnew = GetCCursor(147 + count);
    SetCCursor(compassnew);
    DisposeCCursor(compassnew);
    track[itemHit] = count + 1;

    icon.top = itemRect.top + 9;
    icon.left = itemRect.left + 9;
    ploticon2(2201 + count);
  } else {
    count++;
    sound(144);
    testvalue = track[itemHit];
    track[itemHit] = FALSE;
    for (t = 0; t <= charnum; t++)
      if ((track[t] < testvalue) && (track[t]))
        track[t]++;
    compassnew = GetCCursor(147 + count);
    SetCCursor(compassnew);
    DisposeCCursor(compassnew);

    for (t = 0; t <= charnum; t++) {
      updatepictbox(t, FALSE, 1);
      if (track[t])
        ploticon2(2200 + track[t]);
    }
    updatecontrols();
  }
  ploticon3(130, buttonrect);

  if (count != 0)
    goto loopback;

abort:
  SetCCursor(sword);
  if (!initems) {
    if (!incombat)
      EraseRect(&textrect);
    for (t = 0; t <= charnum; t++)
      updatepictbox(t, TRUE, 1);
  }
  updatecontrols();
  return (itemHit + 1);
}
