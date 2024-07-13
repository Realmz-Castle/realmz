#include "prototypes.h"
#include "variables.h"

/****************************** temple ********************/
void temple(void) {
  short cost[9] = {250, 350, 850, 200, 750, 200, 350, 550, 1500};
  short templong, conditionindex = 0;
  float tempcost;

  in();

  music(10); /**** Temple music ***/

  if (bankavailable) {
    moneypool[0] += bank[0];
    moneypool[1] += bank[1];
    moneypool[2] += bank[2];
    bank[0] = bank[1] = bank[2] = 0;
  }

  for (t = 0; t < 9; t++) {
    tempcost = cost[t];
    tempcost *= templecost;
    tempcost /= 100;
    cost[t] = tempcost;
  }

  templewindow = GetNewDialog(159, 0L, (WindowPtr)-1L);
  if (!reducesound)
    sound(10105);

  MoveWindow(GetDialogWindow(templewindow), GlobalLeft + 20, GlobalTop + 19, FALSE);
  ShowWindow(GetDialogWindow(templewindow));
  SetPortDialogPort(templewindow);
  BackPixPat(base);

bigupdate:

  SelectWindow(GetDialogWindow(templewindow));
  SetPortDialogPort(templewindow);
  TextFont(defaultfont);
  gCurrent = templewindow;
  TextFont(font);
  TextSize(14);
  TextFace(bold);
  ErasePortRect();
  ForeColor(yellowColor);
  DrawDialog(templewindow);

  for (t = 15; t < 24; t++) {
    GetDialogItem(templewindow, t, &itemType, &itemHandle, &buttonrect);
    upbutton(0);
    DialogNum(t + 31, cost[t - 15]);
  }

  for (t = 24; t < 37; t++) {
    GetDialogItem(templewindow, t, &itemType, &itemHandle, &buttonrect);
    downbutton(0);
  }
update:

  SetPortDialogPort(templewindow);
  ForeColor(yellowColor);
  TextSize(14);

  CtoPstr((Ptr)c[charselectnew].name);
  MyrPascalDiStr(65, (StringPtr)c[charselectnew].name);
  PtoCstr((StringPtr)c[charselectnew].name);

  if (c[charselectnew].stamina < c[charselectnew].staminamax)
    ForeColor(whiteColor);
  DialogNum(66, c[charselectnew].stamina);
  ForeColor(yellowColor);
  DialogNum(67, c[charselectnew].staminamax);

  MyrCDiStr(5, (StringPtr) "");
  pict(0, itemRect);
  MyrCDiStr(14, (StringPtr) "");
  plotportrait(c[charselectnew].pictid, itemRect, c[charselectnew].caste, charselectnew);

  ForeColor(yellowColor);

  if (c[charselectnew].stamina < 1) {
    InsetRect(&itemRect, -9, -9);
    ploticon3(2019, itemRect);
    InsetRect(&itemRect, 9, 9);
    if (c[charselectnew].stamina < -9)
      ploticon3(2015, itemRect);
    else
      ploticon3(177, itemRect);
  }

  DialogNum(55, c[charselectnew].money[0]);
  DialogNumLong(56, moneypool[0]);

  for (t = 0; t < 5; t++)
    MyrCDiStr(59 + t, (StringPtr) "");
  for (t = 0; t < 40; t++) {
    if (c[charselectnew].condition[t]) {
      GetIndString(myString, 133, t + 1);
      MyrPascalDiStr(59 + conditionindex++, myString);
      if (conditionindex == 5)
        goto out;
    }
  }

out:
  conditionindex = 0;
  for (;;) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(templewindow, itemHit, &itemType, &itemHandle, &buttonrect);

    if ((itemHit > 14) && (itemHit < 24)) {
      downbutton(0);
      sound(10129);
      if (cost[itemHit - 15] <= (c[charselectnew].money[0] + moneypool[0])) {
        templong = cost[itemHit - 15];
        moneypool[0] -= templong;
        if (moneypool[0] < 0) {
          c[charselectnew].money[0] += moneypool[0];
          c[charselectnew].load += moneypool[0];
          moneypool[0] = 0;
        }

        switch (itemHit) {
          case 15:
            heal(charselectnew, Rand(8), FALSE);
            break;

          case 16:
            heal(charselectnew, randrange(3, 24), FALSE);
            break;

          case 17:
            heal(charselectnew, 5000, FALSE);
            break;

          case 18:
            c[charselectnew].condition[28] = 0;
            break;

          case 19:
            c[charselectnew].condition[26] = 0;
            break;

          case 20:
            c[charselectnew].condition[9] = 0;
            break;

          case 21:
            c[charselectnew].condition[27] = 0;
            break;

          case 22:
            spelllist(charselectnew, 62);
            break;

          case 23:
            spelllist(charselectnew, 64);
            c[charselectnew].condition[25] = c[charselectnew].condition[26] = 0;
            break;
        }

        updatechar(charselectnew, 3);
        upbutton(0);
        goto update;
      } else
        warn(49);
      upbutton(0);
    }

    ForeColor(yellowColor);

    if (itemHit == 10) /**** swap ****/
    {
      ploticon3(129, buttonrect);
      in();
      swap();
      updatemain(FALSE, -1);
      goto bigupdate;
    }

    if (itemHit == 5) {
      ploticon3(129, buttonrect);
      viewcharacter(charselectnew, 0);
      updatemain(FALSE, -1);
      in();
      DrawDialog(templewindow);
      goto bigupdate;
    }

    if ((itemHit == 6) || (itemHit == 7)) {
      ploticon3(135, buttonrect);
      sound(141);
      charselectnew++;
      if (itemHit == 6)
        charselectnew -= 2;
      if (charselectnew > charnum)
        charselectnew = 0;
      if (charselectnew < 0)
        charselectnew = charnum;
      ploticon3(136, buttonrect);
      goto update;
    }

    if ((itemHit == 11) || (itemHit == 1)) /**** done ****/
    {
      ploticon3(129, buttonrect);
      in();
      if ((moneypool[0]) || (moneypool[1]) || (moneypool[2])) {
        if (!bankavailable) {
          if (question(6))
            swap();
          else
            moneypool[0] = moneypool[1] = moneypool[2] = 0;
        } else {
          bank[0] += moneypool[0];
          bank[1] += moneypool[1];
          bank[2] += moneypool[2];
          moneypool[0] = moneypool[1] = moneypool[2] = 0;
        }
      }
      DisposeDialog(templewindow);
      updatemain(FALSE, -1);
      intemple = FALSE;
      return;
    }

    if ((itemHit == 8) || (itemHit == 9)) {
      ploticon3(133, buttonrect);
      if (itemHit == 8)
        pool();
      else
        share();
      DialogNum(55, c[charselectnew].money[0]);
      DialogNumLong(56, moneypool[0]);
      ploticon3(134, buttonrect);
    }

    if (itemHit == 3) {
      downbutton(FALSE);
      if (showcondition(charselectnew, charselectnew, 1, 0, charselectnew) > -1) {
        characterl = c[charselectnew];
        upbutton(FALSE);
        goto bigupdate;
      }
    }
    if (itemHit == 2) {
      downbutton(FALSE);
      showcondition(charselectnew, charselectnew, 0, 0, charselectnew);
    }
    upbutton(FALSE);
  }
}
