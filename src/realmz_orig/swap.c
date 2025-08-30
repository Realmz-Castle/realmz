#include "prototypes.h"
#include "variables.h"

/*************** swap **********************/
void swap(void) {
  Rect rect2, rect1;
  short direction, wieght, times, moneytype, looktype = 154;

  whichchar = charselectnew;

  if (bankavailable) {
    moneypool[0] += bank[0];
    moneypool[1] += bank[1];
    moneypool[2] += bank[2];
    bank[0] = bank[1] = bank[2] = 0;
  }

  int enable_recomposite = WindowManager_SetEnableRecomposite(0);

  SetCCursor(sword);
  for (t = 0; t <= numchannel; t++)
    quiet(t);
  compactheap();

  gswap = GetNewDialog(141, 0L, (WindowPtr)-1L);
  gCurrent = gswap;
  SetPortDialogPort(gswap);
  BackPixPat(base);
  TextFont(font);
  TextSize(22);

  MoveWindow(GetDialogWindow(gswap), GlobalLeft + 80 + (leftshift / 2), GlobalTop + 5 + (downshift / 2), FALSE);
  ShowWindow(GetDialogWindow(gswap));
  ErasePortRect();

  if ((shopavail) || (templeavail))
    looktype = 0;

  inswap = TRUE;
  if (!reducesound)
    sound(3003);
  GetDialogItem(gswap, 7, &itemType, &itemHandle, &itemRect);
  pict(looktype, itemRect);
  InsetRect(&itemRect, 9, 9);
  ploticon3(2002, itemRect);

  GetDialogItem(gswap, 8, &itemType, &itemHandle, &itemRect);
  pict(looktype, itemRect);
  InsetRect(&itemRect, 9, 9);
  ploticon3(2011, itemRect);

  GetDialogItem(gswap, 9, &itemType, &itemHandle, &itemRect);
  pict(looktype, itemRect);
  InsetRect(&itemRect, 9, 9);
  ploticon3(2014, itemRect);

  GetDialogItem(gswap, 10, &itemType, &itemHandle, &itemRect);
  pict(looktype, itemRect);
  InsetRect(&itemRect, 9, 9);
  ploticon3(2012, itemRect);

  GetDialogItem(gswap, 4, &itemType, &itemHandle, &buttonrect);
  downbutton(FALSE);

  GetDialogItem(gswap, 5, &itemType, &itemHandle, &buttonrect);
  downbutton(FALSE);

  GetDialogItem(gswap, 6, &itemType, &itemHandle, &buttonrect);
  downbutton(FALSE);

  ForeColor(blackColor);
  BackColor(whiteColor);
  GetDialogItem(gswap, 14, &itemType, &itemHandle, &rect1);
  pict(161, rect1);
  rect2 = rect1;

  for (t = 1; t <= charnum; t++) {
    BitMap* src = GetPortBitMapForCopyBits(GetQDGlobalsThePort());
    rect2.top += 75;
    rect2.bottom += 75;
    CopyBits(src, src, &rect1, &rect2, 0, NIL);
  }
  TextFace(bold);
  ForeColor(yellowColor);
  RGBBackColor(&greycolor);
  for (t = 0; t <= charnum; t++) {
    GetDialogItem(gswap, 29 + t, &itemType, &itemHandle, &itemRect);
    pict(1, itemRect);
    itemRect.top += 9;
    itemRect.left += 19;
    itemRect.right -= 9;
    itemRect.bottom -= 9;

    plotportrait(c[t].pictid, itemRect, c[t].caste, -1);
  }
  updatemoney(0);

  GetDialogItem(gswap, 23 + charselectnew, &itemType, &itemHandle, &charselectrect);
  DrawPicture(marker, &charselectrect);

  if ((shopavail) || (templeavail))
    MyrCDiStr(64, (StringPtr) "Money Changing is available.");
  else
    MyrCDiStr(64, (StringPtr) "Money Changing is not available.");

  WindowManager_SetEnableRecomposite(enable_recomposite);

  for (;;) {
  tryover:
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(gswap, itemHit, &itemType, &itemHandle, &buttonrect);

    if (itemHit == 2) {
      int enable_recomposite = WindowManager_SetEnableRecomposite(0);
      ploticon3(133, buttonrect);
      pool();
      updatemoney(0);
      ploticon3(134, buttonrect);
      WindowManager_SetEnableRecomposite(enable_recomposite);
    }
    if (itemHit == 3) {
      int enable_recomposite = WindowManager_SetEnableRecomposite(0);
      ploticon3(133, buttonrect);
      share();
      updatemoney(0);
      ploticon3(134, buttonrect);
      WindowManager_SetEnableRecomposite(enable_recomposite);
    }
    if ((itemHit > 16) && (itemHit < 23)) {
      ploticon3(135, buttonrect);
      moneytype = (itemHit - 17) / 2;
      wieght = times = 1;
      if (moneytype == 0)
        times = 5;
      if (moneytype == 2)
        wieght = 15;

      while (Button()) {
        if (((itemHit == 17) || (itemHit == 19) || (itemHit == 21)) && (c[whichchar].money[moneytype]) >= times)
          direction = -1;
        else if ((((itemHit == 18) || (itemHit == 20) || (itemHit == 22))) && (moneypool[moneytype] >= times))
          direction = 1;
        else
          direction = 0;

        int enable_recomposite = WindowManager_SetEnableRecomposite(0);
        if ((c[whichchar].load + (wieght * times) > c[whichchar].loadmax) && (direction == 1))
          direction = 0;
        moneypool[moneytype] -= direction * times;
        TextSize(22);
        if (direction > 0)
          sound(10051);
        else
          sound(663);
        DialogNumLong(11 + moneytype, moneypool[moneytype]);
        c[whichchar].money[moneytype] += direction * times;
        c[whichchar].load += direction * wieght * times;
        movecalc(whichchar);
        TextSize(14);
        DialogNum(38 + 4 * whichchar, c[whichchar].movementmax);
        DialogNum(35 + moneytype + 4 * whichchar, c[whichchar].money[moneytype]);
        WindowManager_SetEnableRecomposite(enable_recomposite);
      }
      ploticon3(136, buttonrect);
    }

    if (itemHit == 10) {
      if ((!shopavail) && (!templeavail)) {
        sound(6000);
        goto tryover;
      }
      int enable_recomposite = WindowManager_SetEnableRecomposite(0);
      ploticon3(129, buttonrect);
      while ((Button()) && (moneypool[2])) {
        sound(-10129);
        moneypool[2]--;
        moneypool[1] += 5;
        updatemoney(1);
      }
      ploticon3(130, buttonrect);
      WindowManager_SetEnableRecomposite(enable_recomposite);
    }

    if ((itemHit == 8) || (itemHit == 9)) {
      if ((!shopavail) && (!templeavail)) {
        sound(6000);
        goto tryover;
      }
      ploticon3(129, buttonrect);
      while ((Button()) && (moneypool[1])) {
        if (itemHit == 9)
          sound(10129);
        else
          sound(-10129);
        moneypool[1]--;
        moneypool[0] += 100;
        updatemoney(1);
      }
      ploticon3(130, buttonrect);
    }

    if ((itemHit > 28) && (itemHit < 35)) {
      if (itemHit - 29 <= charnum) {
        int enable_recomposite = WindowManager_SetEnableRecomposite(0);
        buttonrect.left += 10;
        ploticon3(129, buttonrect);
        sound(141);
        whichchar = itemHit - 29;
        DrawPicture(grey, &charselectrect);
        GetDialogItem(gswap, itemHit - 6, &itemType, &itemHandle, &charselectrect);
        DrawPicture(marker, &charselectrect);
        ploticon3(130, buttonrect);
        WindowManager_SetEnableRecomposite(enable_recomposite);
      }
    }

    if (itemHit == 7) {
      if ((!shopavail) && (!templeavail)) {
        sound(6000);
        goto tryover;
      }
      ploticon3(129, buttonrect);
      while ((Button()) && (moneypool[0] > 114)) {
        sound(10129);
        moneypool[0] -= 115;
        moneypool[1]++;
        updatemoney(1);
      }
      ploticon3(130, buttonrect);
    }

    if ((itemHit == 1) || (itemHit == 59)) {
      sound(141);
      ploticon3(133, buttonrect);
      DisposeDialog(gswap);
      inswap = FALSE;
      return;
    }
  }
}
