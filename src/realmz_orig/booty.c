#include "prototypes.h"
#include "variables.h"

/*************************** updatebootymoney *************/
void updatebootymoney(Rect icon, Rect swapicon) {
  short moneytop, goldleft, gemleft, jewleft;

  SetPort(GetWindowPort(bootywindow));
  TextFont(ID_FONT_DEFAULT);
  TextSize(10);
  TextFace(bold);

  pict(163, icon);

  RGBForeColor(&cyancolor);
  moneytop = icon.top + 14;
  goldleft = icon.left + 29;
  gemleft = icon.left + 34;
  jewleft = icon.left + 42;

  MoveTo(goldleft + leftshift, moneytop);
  MyrNumToString(moneypool[0], myString);
  MyrDrawCString((Ptr)myString);

  MoveTo(gemleft + leftshift, moneytop + 15);
  MyrNumToString(moneypool[1], myString);
  MyrDrawCString((Ptr)myString);

  MoveTo(jewleft + leftshift, moneytop + 30);
  MyrNumToString(moneypool[2], myString);
  MyrDrawCString((Ptr)myString);

  if ((!moneypool[0]) && (!moneypool[1]) && (!moneypool[2]))
    ploticon3(2019, swapicon);
  else
    pict(167, swapicon);
}

/******************* booty ******************/
void booty(short mode) {
  FILE* fp = NULL;
  Boolean detectavial, detectflag, flag[6], truejoin, didfum, identflag, make = TRUE;
  WindowPtr whichWindow;
  RGBColor tempcolor;
  char special[255];
  float tempshare;
  long shareamount, dividers;
  ControlHandle one, two, three, four, five, six;
  ControlHandle identhan[6];
  short listindex, element, numtreasure, itemid, t, tt, baseexp, inc, oldinc, a, tempidflag;
  Rect magicrect, selectrect, scroll, box, icon, temprect, moneyicon, textboxrect, treasurebox, oldbox, infobox;
  ControlHandle poolbut, sharebut, charbut, uparrow, downarrow, swapbut, done, detect;
  Rect r;

  idtoggle = identflag = 0;

  SetRect(&box, 0, 0, 0, 0);
  oldbox = box;

  treasurebox.top = 10;
  treasurebox.left = 7;
  treasurebox.right = 310 + leftshift - (10 * screensize);
  treasurebox.bottom = 425;

  blankround = killparty = detectflag = incombat = truejoin = didfum = identflag = idtoggle = up = monsterturn = charup = FALSE;

  compactheap();
  updatecanlist(); /************ update canheal,canidentify,candetect status ********/

  if (mode == 10) /*********** restore parties equipment ***********/
  {
    moneypool[0] = wealthstore[0];
    moneypool[1] = wealthstore[1];
    moneypool[2] = wealthstore[2];

    wealthstore[0] = wealthstore[1] = wealthstore[2] = 0;

    for (t = 0; t < 1000; t++)
      list[t] = 0;
    for (t = 0; t < 180; t++)
      list[t] = storageitems[t];

    tempshare = totalexp = shareamount = 0;

    goto restoreitems;
  }

  if (blankround > 8)
    mode = 2;

  for (t = 0; t <= charnum; t++) {
    c[t].condition[1] = c[t].bleeding = c[t].traiter = FALSE;
    for (tt = 0; tt < 4; tt++)
      if (c[t].condition[tt] > 0)
        c[t].condition[tt] = 0;
    if ((c[t].stamina < 1) || (c[t].condition[25] < 0))
      killparty++;
    if (c[t].condition[25] > 0)
      c[t].condition[25] = 0;
  }

  EnableItem(gFile, 3);
  EnableItem(gFile, 4);
  for (t = 0; t < 60; t++)
    que[t].duration = que[t].phase = 0;
  for (t = 0; t < 1000; t++)
    list[t] = 0;

  EnableItem(gScenario, 0);
  EnableItem(gParty, 0);
  inbooty = TRUE;
  SetCCursor(sword);

  if (partycondition[0])
    loaddark((partycondition[0] / 30) + 1);
  else
    loaddark(0);

  if (((mode == 2) || (coward)) && (!fumtotal)) {
    inbooty = FALSE;

    for (t = 0; t < maxmon; t++)
      if (monster[t].cansum != -1)
        monster[t].stamina = 0;
    if (!suspendallies) {
      bodycount();
      for (t = 0; t < heldover; t++) {
        if (holdover[t].items[0] < 200)
          holdover[t].weapon = holdover[t].items[0];
      }
    }
    PenSize(1, 1);
    return;
  }

  if (!suspendallies) //** accounts for code 105
  {
    bodycount();
    for (t = 0; t < heldover; t++) {
      if (holdover[t].items[0] < 200)
        holdover[t].weapon = holdover[t].items[0];
    }
  }

  if (make)
    sound(20); /***** booty sound ****/

  SetMenuBar(copywright);
  DrawMenuBar();

  music(7); /**** treasure music ***/

update:
  needdungeonupdate = TRUE;
  leftindex = downindex = bootyitemindex = listindex = detectavial = numtreasure = tempshare = shareamount = dividers = 0;

  for (t = 0; t < fumtotal; t++) {
    list[listindex++] = -(fumque[t]);
    fumque[t] = 0;
  }

  if (fumtotal)
    didfum = fumtotal;
  fumtotal = 0;

  if ((mode) && (mode != 5) && (!didfum)) {
    if (mode != 5) {
      for (t = 0; t < 20; t++) {
        if (treasure.itemid[t] > 0)
          list[listindex++] = treasure.itemid[t];
        treasure.itemid[t] = 0;
      }

      if (treasure.exp < 0)
        totalexp += Rand(abs(treasure.exp));
      else
        totalexp += treasure.exp;
      if (treasure.gold < 0)
        moneypool[0] += Rand(abs(treasure.gold));
      else
        moneypool[0] += treasure.gold;
      if (treasure.gems < 0)
        moneypool[1] += Rand(abs(treasure.gems));
      else
        moneypool[1] += treasure.gems;
      if (treasure.jewelry < 0)
        moneypool[2] += Rand(abs(treasure.jewelry));
      else
        moneypool[2] += treasure.jewelry;

      if (moneypool[0] < 0)
        moneypool[0] = 0;
      if (moneypool[1] < 0)
        moneypool[1] = 0;
      if (moneypool[2] < 0)
        moneypool[2] = 0;
    }
  }

  if (nummon) {
    for (t = 0; t < nummon; t++) {
      if ((monster[t].traiter) && (monster[t].stamina < 1)) {
        moneypool[0] += randrange(0, monster[t].money[0]);
        moneypool[1] += randrange(0, monster[t].money[1]);
        moneypool[2] += randrange(0, monster[t].money[2]);

        switch (monster[t].hd) {
          case 0:
            baseexp = 15;
            inc = 3;
            break;

          case 1:
            baseexp = 30;
            inc = 6;
            break;

          case 2:
            baseexp = 45;
            inc = 9;
            break;

          case 3:
            baseexp = 65;
            inc = 12;
            break;

          case 4:
            baseexp = 80;
            inc = 15;
            break;

          case 5:
            baseexp = 100;
            inc = 18;
            break;

          case 6:
            baseexp = 140;
            inc = 21;
            break;

          case 7:
            baseexp = 200;
            inc = 24;
            break;

          case 8:
            baseexp = 300;
            inc = 27;
            break;

          case 9:
            baseexp = 450;
            inc = 30;
            break;

          case 10:
            baseexp = 700;
            inc = 33;
            break;

          case 11:
            baseexp = 1100;
            inc = 36;
            break;

          case 12:
            baseexp = 1800;
            inc = 39;
            break;

          case 13:
            baseexp = 2300;
            inc = 42;
            break;

          case 14:
            baseexp = 2800;
            inc = 45;
            break;

          case 15:
            baseexp = 3200;
            inc = 50;
            break;

          case 16:
            baseexp = 3700;
            inc = 55;
            break;

          case 17:
            baseexp = 4200;
            inc = 60;
            break;

          case 18:
            baseexp = 4700;
            inc = 65;
            break;

          case 19:
            baseexp = 5200;
            inc = 70;
            break;

          case 20:
            baseexp = 5700;
            inc = 75;
            break;

          default:
            baseexp = 6200;
            inc = 80;
            break;
        }

        if (mode != 5) /**** award items *****/
        {
          for (tt = 0; tt < 6; tt++)
            if (monster[t].items[tt])
              list[listindex++] = monster[t].items[tt];
        } else {
          for (tt = didfum; tt < 1000; tt++)
            list[tt] = 0;
          moneypool[0] = 0;
          moneypool[1] = 0;
          moneypool[2] = 0;
        }

        totalexp += baseexp;
        totalexp += monster[t].exp;
        totalexp += monster[t].staminamax * inc;
      }
    }
    for (tt = 0; tt < 40; tt++)
      monster[t].condition[tt] = 0;
    monster[t].stamina = monster[t].traiter = 0;
  }

  mode = 0;

  if (nummon) {
    for (t = 0; t < nummon; t++) {
      if ((monster[t].type[0]) && (!monster[t].type[7]) && (Rand(100) < 10) && (monster[t].cansum != -1)) {
        /**** add parchment ****/
        list[listindex++] = 806;
      }

      if ((!monster[t].type[7]) && (!monster[t].type[1]) && (Rand(100) < 10) && (monster[t].cansum != -1)) {
        /**** add ration ****/
        list[listindex++] = 877;
      }
    }
  }

  moneypool[0] *= hardpercent;
  moneypool[1] *= hardpercent;
  moneypool[2] *= hardpercent;

  numfoes = nummon = 0;

  for (t = 0; t <= charnum; t++) {
    if ((c[t].stamina > 0) && (c[t].condition[25] > -1) && (c[t].inbattle))
      dividers++;
  }

  dividers += deduction;
  deduction = 0;

  if (dividers) {
    tempshare = totalexp;
    tempshare *= percent;
    if (!allowfumble)
      tempshare *= .95;
    if (!allowunique)
      tempshare *= .9;
    shareamount = tempshare;
    shareamount /= dividers;
    if (shareamount < 0)
      shareamount *= -1;
  }

restoreitems:

  for (t = 0; t < 1000; t++)
    if (list[t])
      numtreasure++;

  listindex = 0;
  for (t = 0; t <= charnum; t++) {
    if ((c[t].stamina > 0) && (c[t].condition[25] > -1) && (c[t].inbattle)) {
      tempshare = shareamount;
      loadprofile(c[t].race, 0);
      if (c[t].age / 365 >= races.maxage)
        tempshare *= .6666666;
      c[t].exp += tempshare;
    }
  }

  bootywindow = GetNewWindow(130, 0L, (WindowPtr)-1L);
  SetPort(GetWindowPort(bootywindow));
  BackPixPat(whitepat);
  TextFont(defaultfont);
  MoveWindow(bootywindow, GlobalLeft, GlobalTop, FALSE);
  ShowWindow(bootywindow);

  one = GetNewControl(178, bootywindow);
  GetControlBounds(one, &r);
  MoveControl(one, r.left + leftshift, r.top);

  if (charnum > 0) {
    two = GetNewControl(179, bootywindow);
    GetControlBounds(two, &r);
    MoveControl(two, r.left + leftshift, r.top);
  }

  if (charnum > 1) {
    three = GetNewControl(180, bootywindow);
    GetControlBounds(three, &r);
    MoveControl(three, r.left + leftshift, r.top);
  }

  if (charnum > 2) {
    four = GetNewControl(181, bootywindow);
    GetControlBounds(four, &r);
    MoveControl(four, r.left + leftshift, r.top);
  }

  if (charnum > 3) {
    five = GetNewControl(182, bootywindow);
    GetControlBounds(five, &r);
    MoveControl(five, r.left + leftshift, r.top);
  }

  if (charnum > 4) {
    six = GetNewControl(183, bootywindow);
    GetControlBounds(six, &r);
    MoveControl(six, r.left + leftshift, r.top);
  }

  for (t = 0; t <= charnum; t++) {
    identhan[t] = GetNewControl(184 + t, bootywindow);
    GetControlBounds(identhan[t], &r);
    MoveControl(identhan[t], r.left + leftshift, r.top);
  }

  if ((autoid) && (list[0])) {
    for (t = 0; t <= charnum; t++) {
      if ((cancast(t, 1)) && (c[t].canidentify) && (c[t].spellpoints > 24)) {
        idtoggle = TRUE;
        c[t].spellpoints -= 25;
        select[t] = TRUE;

        for (t = 0; t < 1000; t++) {
          if (list[t]) {
            loaditem(list[t]);
            if (item.ismagical)
              list[t] = -abs(list[t]);
          }
        }
        goto update2;
      }
    }
  }

update2:

  SetPort(GetWindowPort(bootywindow));
  BackPixPat(whitepat);
  for (t = 0; t < 7; t++) {
    for (tt = 0; tt < (6 + 3 * screensize); tt++) {
      temp = list[bootyitemindex * (6 + (3 * screensize)) + t * (6 + (3 * screensize)) + tt];
      if (temp) {
        loaditem(temp);
        iconhand = NIL;

        if ((didfum) && ((bootyitemindex * (6 + (3 * screensize)) + t * (6 + (3 * screensize)) + tt) < didfum)) {
          iconhand = GetCIcon(lookupicon(item.iconid, TRUE));
        } else
          iconhand = GetCIcon(lookupicon(item.iconid, idtoggle));

        icon.top = t * 60 + 25;
        icon.left = tt * 50 + 16;
        icon.right = icon.left + 32;
        icon.bottom = icon.top + 32;
        if (list[bootyitemindex * (6 + (3 * screensize)) + t * (6 + (3 * screensize)) + tt] < 0) {
          magicrect.top = t * 60 + 17;
          magicrect.left = tt * 50 + 8;
          magicrect.right = magicrect.left + 48;
          magicrect.bottom = magicrect.top + 48;
          if (showmagic)
            PlotCIcon(&magicrect, showmagic);
        }
        if (iconhand) {
          PlotCIcon(&icon, iconhand);
          DisposeCIcon(iconhand);
        }
      }
    }
  }

  TextFace(bold);
  RGBBackColor(&greycolor);
  listindex = 0;

  icon.left = 320 + leftshift;
  icon.right = 640 + leftshift;
  icon.top = 0;
  icon.bottom = 50 * (charnum + 1);

  if (make) {
    SetPort(GetWindowPort(bootywindow));
    BackPixPat(whitepat);
    charbut = NewControl(bootywindow, &icon, (StringPtr) "", 1, 0, 0, 0, 0, 0);

    updatechar(0, 4);
    icon.top = 0;
    icon.bottom = 50;
    icon.left = 380 + leftshift;
    icon.right = 640 + leftshift;
    temprect = icon;
    SetPort(GetWindowPort(bootywindow));
    BackPixPat(whitepat);
    ForeColor(blackColor);
    BackColor(whiteColor);

    for (t = 1; t <= charnum; t++) {
      BitMapPtr src = GetPortBitMapForCopyBits(GetWindowPort(bootywindow));
      temprect.top += 50;
      temprect.bottom += 50;
      CopyBits(src, src, &icon, &temprect, 0, NIL);
    }

    temprect = icon;
    TextSize(15);
    ForeColor(yellowColor);

    for (t = 0; t <= charnum; t++) {
      updatepictbox(t, 0, 1);
      temprect.top += 50;
      temprect.bottom += 50;
      MoveTo(420 + leftshift, temprect.top - 31);
      ForeColor(yellowColor);
      MyrDrawCString(c[t].name);
    }
  }

  TextSize(14);
  RGBBackColor(&greycolor);
  RGBForeColor(&cyancolor);
  for (t = 0; t <= charnum; t++)
    updatebooty(t);

  icon.left = 320 + leftshift;
  icon.top = 50 * (charnum + 1);
  icon.right = 640 + leftshift;
  icon.bottom = 300;
  BackPixPat(base);
  EraseRect(&icon);
  BackPixPat(whitepat);

  icon.top = icon.bottom;
  icon.bottom = icon.top + 110;
  pict(198, icon);
  infobox = icon;
  ForeColor(yellowColor);

  icon.top += 33;
  icon.left += 9;
  icon.right -= 9;
  icon.bottom -= 8;

  textboxrect = icon;

  if (!messageafter) {
    temp = 71;
    if ((numtreasure) || (moneypool[0]) || (moneypool[1]) || (moneypool[2]))
      temp = 35;
    if ((totalexp) && (temp == 35))
      temp = 36;
    else if (totalexp)
      temp = 34;
    textbox(3, temp, FALSE, TRUE, textboxrect); /************ hardwire for no message ************/
  } else
    textbox(-1, -(messageafter), FALSE, TRUE, textboxrect);

  icon.bottom += 8;

  if (shareamount) {
    TextSize(12);

    itemRect.top = 439;
    itemRect.bottom = 460;
    itemRect.left = 0;
    itemRect.right = 320;
    pict(169, itemRect);

    if (screensize) {
      itemRect.left = itemRect.right;
      itemRect.right += leftshift;
      EraseRect(&itemRect);
    }

    totalexp = 0;
    MoveTo(250, itemRect.top + 16);
    MyrNumToString(shareamount, myString);
    MyrDrawCString((Ptr)myString);
  }

  scroll.left = 8;
  scroll.right = 312 + leftshift;

  icon.top = icon.bottom;
  icon.bottom = icon.top + 50;
  icon.left = 320 + leftshift;

  icon.right = 640 + leftshift;
  EraseRect(&icon);

  icon.right = 370 + leftshift;
  if (make)
    done = NewControl(bootywindow, &icon, (StringPtr) "", 1, 0, 0, 0, 0, 0);

  pict(168, icon);

  if (numtreasure > 42 + (21 * screensize)) {
    icon.right += 50;
    icon.left += 50;
    icon.bottom = icon.top + 25;
    if (make)
      uparrow = NewControl(bootywindow, &icon, (StringPtr) "", 1, 0, 0, 0, 0, 0);
    icon.top += 25;
    icon.bottom += 25;
    if (make)
      downarrow = NewControl(bootywindow, &icon, (StringPtr) "", 1, 0, 0, 0, 0, 0);
    icon.top -= 25;
    pict(132, icon);
  }

  icon.left += 50;
  icon.right += 50;
  if (make)
    swapbut = NewControl(bootywindow, &icon, (StringPtr) "", 1, 0, 0, 0, 0, 0);

  pict(167, icon);
  if ((!moneypool[0]) && (!moneypool[1]) && (!moneypool[2]))
    ploticon3(2019, icon);

  icon.left += 50;
  icon.right += 50;
  icon.bottom -= 25;
  if (make)
    poolbut = NewControl(bootywindow, &icon, (StringPtr) "", 1, 0, 0, 0, 0, 0);

  icon.top += 25;
  icon.bottom += 25;
  if (make)
    sharebut = NewControl(bootywindow, &icon, (StringPtr) "", 1, 0, 0, 0, 0, 0);

  icon.top -= 25;
  pict(196, icon);

  icon.left += 50;
  icon.right += 70;

  moneyicon = icon;

  GetControlBounds(swapbut, &r);
  updatebootymoney(moneyicon, r);

  for (t = 0; t <= charnum; t++) {
    if (c[t].spellpoints > 4) {
      if ((cancast(t, 1)) && (c[t].candetect)) {
        icon.left = 590 + leftshift;
        icon.right = 640 + leftshift;
        if (make) {
          detect = NewControl(bootywindow, &icon, (StringPtr) "", 1, 0, 0, 0, 0, 0);
        }
        icon.left = 590 + leftshift;
        icon.right = 640 + leftshift;
        pict(139, icon);
        detectavial = t + 1;
        goto moveon;
      }
    }
  }

moveon:
  selectrect.top = 50 * charselectnew;
  selectrect.left = 320 + leftshift;
  selectrect.right = 330 + leftshift;
  selectrect.bottom = selectrect.top + 50;
  DrawPicture(marker, &selectrect);

  if (didfum)
    warn(115);

  if (screensize)
    quickinfo(-1, 0, list[0], 2);

backup:

  PenSize(2, 2);

  for (;;) {

    if (gTheEvent.modifiers & alphaLock)
      warn(21);

    SystemTask();
    a = GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
    DoCorrectBugMADRepeat();
#endif
    MyrCheckMemory(2);

    GetMouse(&point);

    if (PtInRect(point, &treasurebox)) {
      RGBForeColor(&cyancolor);
      temp = (point.v - 10) / 60;
      TextSize(12);
      RGBBackColor(&greycolor);
      TextFont(defaultfont);
      box.top = 10 + temp * 60;
      box.bottom = box.top + 60;

      tt = (point.h - 10) / 50;
      inc = tt + ((temp + bootyitemindex) * (6 + (3 * screensize)));

      if (inc != oldinc) {
        box.left = 7 + tt * 50;
        box.right = box.left + 50;

        PenMode(2);
        FrameOval(&oldbox);

        oldbox = box;
        oldinc = inc;

        FrameOval(&box);

        pict(206, infobox);

        if (list[inc]) {
          if (screensize)
            quickinfo(-1, inc, list[inc], 2);
          TextSize(12);
          loaditem(list[inc]);
          MoveTo(330 + leftshift, textboxrect.top - 12);
          if (idtoggle > 1)
            idtoggle = 1;

          tempidflag = idtoggle;

          if (inc < didfum)
            tempidflag = 1; /********* auto id fumbled stuff ************/

          temp = item.iconid;

          if (item.iscurse)
            loaditem(item.iscurse); /******** load in fake item for cursed so stats match. ***/

          item.iconid = temp;

          // Myriad : this don't work because you can't assume than tempselection is computed
          // GetIndString(myString,getselection(item.itemid)+tempidflag,item.itemid-tempselection+1);
          GetIndString(myString, getselection(item.itemid) + tempidflag, item.itemid - getselection(item.itemid) + 1);
          PtoCstr(myString);
          MyrDrawCString((Ptr)myString);

          temp = item.vssmall;
          if (temp) {
            MoveTo(381 + leftshift, textboxrect.top + 49);
            if (temp)
              temp += (item.damage + item.heat + item.cold + item.electric + item.vsundead + item.vsdd + item.vsevil);
            string(temp);
          }

          MoveTo(381 + leftshift, textboxrect.top + 66);
          string(item.wieght + item.xcharge * item.charge);

          MoveTo(505 + leftshift, textboxrect.top + 49);
          if (tempidflag)
            string(item.ac);
          else
            MyrDrawCString("?");

          if (item.nohands) {
            MoveTo(505 + leftshift, textboxrect.top + 66);
            string(item.nohands);
          }

          MoveTo(603 + leftshift, textboxrect.top + 49);
          if (tempidflag)
            string(item.damage);
          else
            MyrDrawCString("?");

          MoveTo(603 + leftshift, textboxrect.top + 66);
          temp = item.charge;
          if (temp < 0)
            temp = 0;
          if (tempidflag)
            string(temp);
          else
            MyrDrawCString("?");

          MoveTo(603 + leftshift, textboxrect.top + 32);
          if (canuse(-1, c[charselectnew].race, c[charselectnew].caste))
            SetCCursor(sword);
          else
            SetCCursor(stop);
          RGBForeColor(&cyancolor);

          /************************* show special start **************/
          MoveTo(372 + leftshift, textboxrect.top + 13);
          if (tempidflag) {

            if (item.sp1 == 122) /******* adds attacks *********/
            {
              switch (item.sp2) {
                case 1:
                  MyrDrawCString("Gives An Extra 1/2 Attack");
                  break;

                case 2:
                  MyrDrawCString("Gives An Extra Attack");
                  break;

                case 3:
                  MyrDrawCString("Gives An Extra 1 1/2 Attacks");
                  break;

                case 4:
                  MyrDrawCString("Gives An Extra 2 Attacks");
                  break;
              }
            } else if (item.sp1 == 121) /******* penetration weapon *********/
            {
              MyrDrawCString("Double Magical TOHIT In Combat.");
            } else if (item.sp1 == 120) /******* auto hit weapon *********/
            {
              MyrDrawCString("This weapon ALWAYS hits in combat.");
            } else if (item.sp1 > 19) {
              GetIndString(myString, 134, item.sp1 - 19);
              PtoCstr(myString);
              MyrDrawCString((Ptr)myString);
            } else if (item.sp2 > 1100) {
              loadspell2(item.sp2);
              strcpy((StringPtr)special, (StringPtr) "Stores Spell: ");
              GetIndString(myString, 1000 * (castcaste + 1) + castlevel, castnum + 1);
              PtoCstr(myString);
              strcat((StringPtr)special, myString);
              if (strlen(myString))
                MyrDrawCString(special); // Myriad, confuse strlen<->StringWidth
            } else if (item.heat + item.cold + item.electric + item.vsundead + item.vsdd + item.vsevil) {
              MyrDrawCString("This weapon does special damage.");
            } else if (item.sp3 < 0) /****** ability to hit monster types ****/
            {
              strcpy((StringPtr)special, (StringPtr) "");

              if (item.sp5 > 0)
                strcat((StringPtr)special, (StringPtr) "+");

              MyrNumToString(item.sp5, myString);
              strcat((StringPtr)special, myString);
              GetIndString(myString, 132, abs(item.sp3));
              PtoCstr(myString);
              strcat((StringPtr)special, myString);
              MyrDrawCString(special);
            } else if ((item.sp3 > 0) && (item.sp3 < 16)) /****** add to special abilitys like pick lock ****/
            {
              strcpy((StringPtr)special, (StringPtr) "");

              if (item.sp5 > 0)
                strcat((StringPtr)special, (StringPtr) "+");

              MyrNumToString(item.sp5, myString);
              strcat((StringPtr)special, myString);
              strcat((StringPtr)special, (StringPtr) " to ");
              GetIndString(myString, 1201, abs(item.sp3));
              PtoCstr(myString);
              strcat((StringPtr)special, myString);
              MyrDrawCString(special);
            } else if ((item.sp4 > 0) && (item.sp4 < 16)) /****** add to special abilitys like pick lock ****/
            {
              strcpy((StringPtr)special, (StringPtr) "");

              if (item.sp5 > 0)
                strcat((StringPtr)special, (StringPtr) "+");

              MyrNumToString(item.sp5, myString);
              strcat((StringPtr)special, myString);
              strcat((StringPtr)special, (StringPtr) " to ");
              GetIndString(myString, 1201, abs(item.sp4));
              PtoCstr(myString);
              strcat((StringPtr)special, myString);
              MyrDrawCString(special);
            }
          } else
            MyrDrawCString("Specials are unknown.");
          /************************* show special end **************/

          itemRect.top = textboxrect.top + 1;
          itemRect.bottom = itemRect.top + 32;
          itemRect.left = 330 + leftshift;
          itemRect.right = itemRect.left + 32;

          iconhand = NIL;

          iconhand = GetCIcon(lookupicon(item.iconid, tempidflag));

          if (iconhand) {
            PlotCIcon(&itemRect, iconhand);
            DisposeCIcon(iconhand);
          }
        } else
          SetCCursor(sword);
      }
    } else if (oldinc) {
      PenMode(2);
      FrameOval(&box);
      oldbox.top = oldbox.left = oldbox.right = oldbox.bottom = oldinc = 0;
      pict(198, infobox);

      if (!messageafter) {
        temp = 71;
        if ((numtreasure) || (moneypool[0]) || (moneypool[1]) || (moneypool[2]))
          temp = 35;
        if ((totalexp) && (temp == 35))
          temp = 36;
        else if (totalexp)
          temp = 34;
        textbox(3, temp, FALSE, TRUE, textboxrect); /************ hardwire for no message ************/
      } else
        textbox(-1, -(messageafter), FALSE, TRUE, textboxrect);
    }

    switch (gTheEvent.what) {
      case (updateEvt):
        BeginUpdate(bootywindow);
        EndUpdate(bootywindow);

        if (mat) {
          BeginUpdate(mat);
          EndUpdate(mat);
        }
        break;

      case (osEvt):

        if ((gTheEvent.message >> 24) == suspendResumeMessage) {
          if (hide) {
            hide = FALSE;

            updatechar(0, 4);
            icon.top = 0;
            icon.bottom = 50;
            icon.left = 380;
            icon.right = 640;

            temprect = icon;

            TextSize(15);

            for (t = 0; t <= charnum; t++) {
              updatepictbox(t, 0, 1);

              temprect.top += 50;
              temprect.bottom += 50;

              ForeColor(blackColor);
              BackColor(whiteColor);
              SetPort(GetWindowPort(bootywindow));
              BackPixPat(whitepat);

              if (t < charnum) {
                BitMapPtr src = GetPortBitMapForCopyBits(GetWindowPort(bootywindow));
                CopyBits(src, src, &icon, &temprect, 0, NIL);
              }
              icon.top += 50;
              icon.bottom += 50;
              ForeColor(yellowColor);
              MoveTo(420 + leftshift, temprect.top - 31);
              MyrDrawCString(c[t].name);
            }

            goto iteminfoupdate;
          } else
            hide = TRUE;
        }
        break;

      case mouseDown:

        point = gTheEvent.where;
        theControl = NIL;
        GlobalToLocal(&(point));

        thePart = FindWindow(gTheEvent.where, &whichWindow);
        switch (thePart) {
          case inMenuBar:
            compactheap();
            menuChoice = MenuSelect(gTheEvent.where);
            HiliteMenu(0);
            break;

          case inContent:
            thePart = FindControl(point, bootywindow, &theControl);

            if (theControl != NIL) {
              if (theControl == done) {
                Rect r;
                GetControlBounds(done, &r);
                ploticon3(129, r);
                sound(141);

                for (t = 0; t < 1000; t++) /*********** check for scenairo items *********/
                {
                  if (list[t]) {
                    loaditem(list[t]);
                    if ((item.type == 25) && (item.itemid > 800)) {
                      BitMapPtr src = GetPortBitMapForCopyBits(GetWindowPort(bootywindow));
                      BitMapPtr dst = GetPortBitMapForCopyBits(gbuff2);

                      magicrect.top = 0;
                      magicrect.left = 320;
                      magicrect.right = 640;
                      magicrect.bottom = 320;

                      SetPort(GetWindowPort(bootywindow));
                      BackPixPat(whitepat);
                      ForeColor(blackColor);
                      BackColor(whiteColor);

                      CopyBits(src, dst, &magicrect, &lookrect, 0, NIL);

                      if (question(16)) {
                        Rect r;
                        BitMapPtr src = GetPortBitMapForCopyBits(gbuff2);
                        BitMapPtr dst = GetPortBitMapForCopyBits(GetWindowPort(bootywindow));

                        SetPort(GetWindowPort(bootywindow));
                        BackPixPat(whitepat);
                        ForeColor(blackColor);
                        BackColor(whiteColor);

                        CopyBits(src, dst, &lookrect, &magicrect, 0, NIL);
                        RGBBackColor(&greycolor);
                        make = FALSE;
                        GetControlBounds(done, &r);
                        ploticon3(130, r);
                        goto update2;
                      } else
                        goto push2;
                    }
                  }
                }

              push2:

                if (((moneypool[0]) || (moneypool[1]) || (moneypool[2])) && (forgettreasure)) {
                  if (question(6))
                    swap();
                }
                messageafter = 0;
                for (t = 0; t <= charnum; t++) {
                  flag[t] = 0;
                  if ((c[t].exp > 0) && (c[t].stamina > 0)) {
                    getexp(c[t].level, c[t].caste, t);

                    if ((levelup(t, 0)) && (c[t].spellpointsmax)) {
                      if (c[t].spellcastertype) /********** only allow spell selection if they are above neccesary level *********/
                      {
                        if (caste.spellcasters[c[t].spellcastertype - 1][1] <= c[t].level)
                          flag[t] = TRUE;
                      }
                    }
                  }
                }

                for (t = 0; t <= charnum; t++) {
                  if ((flag[t]) && (c[t].spellcastertype)) {
                    characterl = c[t];
                    Spellselect();
                    c[t] = characterl;

                    for (tt = 0; tt < 10; tt++) {
                      definespells[t][tt][0] = c[t].definespells[tt][0];
                      definespells[t][tt][1] = c[t].definespells[tt][1];
                      definespells[t][tt][2] = c[t].definespells[tt][2];
                      definespells[t][tt][3] = c[t].definespells[tt][3];
                    }
                  }
                }

                DisposeWindow(bootywindow);
                bootywindow = NIL;
                inbooty = didfum = FALSE;
                totalexp = moneypool[0] = moneypool[1] = moneypool[2] = 0;
                needupdate = TRUE;
                for (tt = 0; tt <= charnum; tt++)
                  c[tt].inbattle = TRUE;
                if (GetQDGlobalsThePort()) // Myriad current port can be NULL
                  PenSize(1, 1);
                return;
              }

              if (theControl == swapbut) {
                BitMapPtr src = GetPortBitMapForCopyBits(GetWindowPort(bootywindow));
                BitMapPtr dst = GetPortBitMapForCopyBits(gbuff2);
                Rect r;

                magicrect.top = 0;
                magicrect.left = 320;
                magicrect.right = 640;
                magicrect.bottom = 320;

                SetPort(GetWindowPort(bootywindow));
                BackPixPat(whitepat);
                ForeColor(blackColor);
                BackColor(whiteColor);

                CopyBits(src, dst, &magicrect, &lookrect, 0, NIL);

                GetControlBounds(swapbut, &r);
                ploticon3(129, r);
                swap();

              iteminfoupdate:

                SetPort(GetWindowPort(bootywindow));
                BackPixPat(whitepat);
                ForeColor(blackColor);
                BackColor(whiteColor);

                src = GetPortBitMapForCopyBits(gbuff2);
                dst = GetPortBitMapForCopyBits(GetWindowPort(bootywindow));
                CopyBits(src, dst, &lookrect, &magicrect, 0, NIL);

                RGBBackColor(&greycolor);
                make = FALSE;
                goto update2;
              }

              for (t = 0; t <= charnum; t++) {
                if (theControl == identhan[t]) {
                  if ((!c[t].condition[5]) && (!c[t].condition[39]) && (!c[t].condition[25]) && (c[t].stamina > 0)) {
                    GetControlBounds(identhan[t], &buttonrect);
                    // buttonrect = *&(**(identhan[t])).contrlRect;
                    PenMode(0);

                    if ((c[t].canidentify) && (c[t].spellpoints > 24) && (cancast(t, 0)) && (idtoggle == 0)) {
                      idtoggle = TRUE;
                      c[t].spellpoints -= 25;
                      sound(647);
                      identflag = TRUE;
                      goto forceident;
                    } else if (!idtoggle) {
                      warn(112);
                      make = FALSE;
                      goto update2;
                    }
                  } else {
                    warn(112);
                    make = FALSE;
                    goto update2;
                  }
                }
              }

              if (theControl == one) {
                GetControlBounds(one, &buttonrect);
                // buttonrect = *&(**(one)).contrlRect;
                PenMode(0);

                temp = showcondition(0, 0, 1, 1, 0);

                if (temp > -1)
                  dropitem(0, c[0].items[temp].id, temp, TRUE, FALSE);

                updatebooty(0);
              }

              if (theControl == two) {
                GetControlBounds(two, &buttonrect);
                // buttonrect = *&(**(two)).contrlRect;
                PenMode(0);

                temp = showcondition(1, 1, 1, 1, 1);

                if (temp > -1)
                  dropitem(1, c[1].items[temp].id, temp, TRUE, FALSE);

                updatebooty(1);
              }

              if (theControl == three) {
                GetControlBounds(three, &buttonrect);
                // buttonrect = *&(**(three)).contrlRect;
                PenMode(0);

                temp = showcondition(2, 2, 1, 1, 2);

                if (temp > -1)
                  dropitem(2, c[2].items[temp].id, temp, TRUE, FALSE);

                updatebooty(2);
              }

              if (theControl == four) {
                GetControlBounds(four, &buttonrect);
                // buttonrect = *&(**(four)).contrlRect;
                PenMode(0);

                temp = showcondition(3, 3, 1, 1, 3);

                if (temp > -1)
                  dropitem(3, c[3].items[temp].id, temp, TRUE, FALSE);

                updatebooty(3);
              }

              if (theControl == five) {
                GetControlBounds(five, &buttonrect);
                // buttonrect = *&(**(five)).contrlRect;
                PenMode(0);

                temp = showcondition(4, 4, 1, 1, 4);

                if (temp > -1)
                  dropitem(4, c[4].items[temp].id, temp, TRUE, FALSE);

                updatebooty(4);
              }

              if (theControl == six) {
                GetControlBounds(six, &buttonrect);
                // buttonrect = *&(**(six)).contrlRect;
                PenMode(0);

                temp = showcondition(5, 5, 1, 1, 5);

                if (temp > -1)
                  dropitem(5, c[5].items[temp].id, temp, TRUE, FALSE);

                updatebooty(5);
              }

              if (theControl == poolbut) {
                Rect r;
                GetControlBounds(poolbut, &r);
                ploticon3(133, r);
                pool();
                SetPort(GetWindowPort(bootywindow));
                BackPixPat(whitepat);
                TextSize(14);
                RGBBackColor(&greycolor);
                RGBForeColor(&cyancolor);
                for (t = 0; t <= charnum; t++)
                  updatebooty(t);
                GetControlBounds(swapbut, &r);
                updatebootymoney(moneyicon, r);
                GetControlBounds(poolbut, &r);
                ploticon3(134, r);
              }

              if (theControl == sharebut) {
                Rect r;
                GetControlBounds(sharebut, &r);
                ploticon3(133, r);
                share();
                SetPort(GetWindowPort(bootywindow));
                BackPixPat(whitepat);
                TextSize(14);
                RGBBackColor(&greycolor);
                RGBForeColor(&cyancolor);
                for (t = 0; t <= charnum; t++)
                  updatebooty(t);
                GetControlBounds(swapbut, &r);
                updatebootymoney(moneyicon, r);
                GetControlBounds(sharebut, &r);
                ploticon3(134, r);
              }

              if (theControl == uparrow) {
                Rect r;
                GetControlBounds(uparrow, &r);
                ploticon3(133, r);
                scroll.top = 0;
                scroll.bottom = 425;
                sound(141);
                BackColor(whiteColor);
                BackPixPat(whitepat); // Myriad

                while ((bootyitemindex > 0) && (StillDown())) {
                  bootyitemindex--;
                  for (t = 0; t < 4; t++)
                    ScrollRect(&scroll, 0, 15, 0L);
                  delay(2); // Fantasoft 7.1  Slows it down a bit on fast machines.

                  for (t = 0; t < (6 + (3 * screensize)); t++) {
                    temp = list[bootyitemindex * (6 + (3 * screensize)) + t];
                    if (temp) {
                      loaditem(temp);
                      icon.top = 25;
                      icon.bottom = icon.top + 32;
                      icon.left = 50 * t + 16;
                      icon.right = icon.left + 32;
                      iconhand = NIL;

                      if (temp < 0)
                        iconhand = GetCIcon(lookupicon(item.iconid, TRUE));
                      else
                        iconhand = GetCIcon(lookupicon(item.iconid, idtoggle));

                      if (temp < 0) {
                        magicrect.top = 17;
                        magicrect.left = 50 * t + 8;
                        magicrect.right = magicrect.left + 48;
                        magicrect.bottom = magicrect.top + 48;
                        if (showmagic)
                          PlotCIcon(&magicrect, showmagic);
                      }
                      if (iconhand) {
                        PlotCIcon(&icon, iconhand);
                        DisposeCIcon(iconhand);
                      }
                    }
                  }
                }
                ploticon3(134, r);
                RGBBackColor(&greycolor);
              }

              if ((theControl == detect) && (detectavial)) {
                Rect r;
              forceident:

                detectflag = TRUE;
                if (!identflag)
                  flashmessage((StringPtr) "Discovering magical items.", 350, 312, 60, 6001);
                else
                  flashmessage((StringPtr) "Identifying items.", 350, 312, 60, 6001);
                SetPort(GetWindowPort(bootywindow));
                BackPixPat(whitepat);
                GetControlBounds(detect, &r);
                if (!identflag)
                  ploticon3(129, r);
                for (t = 0; t < 1000; t++) {
                  if (list[t]) {
                    loaditem(list[t]);
                    if (item.ismagical)
                      list[t] = -abs(list[t]);
                  }
                }

                if ((!identflag) && (detectflag < 2) && (detectavial)) {
                  c[detectavial - 1].spellpoints -= 5;
                  select[detectavial - 1] = TRUE;
                }
                listindex = 0;

                BackPixPat(whitepat);

                for (t = 0; t < 7; t++) {
                  for (tt = 0; tt < (6 + (3 * screensize)); tt++) {
                    temp = list[bootyitemindex * (6 + (3 * screensize)) + t * (6 + (3 * screensize)) + tt];
                    if (temp) {
                      loaditem(temp);
                      iconhand = NIL;
                      iconhand = GetCIcon(lookupicon(item.iconid, idtoggle));

                      icon.top = t * 60 + 17;
                      icon.left = tt * 50 + 8;
                      icon.right = icon.left + 48;
                      icon.bottom = icon.top + 48;

                      SetPort(GetWindowPort(bootywindow));
                      BackColor(whiteColor);

                      if (icon.top <= 460) {
                        EraseRect(&icon);
                        InsetRect(&icon, 8, 8);

                        if (temp < 0) {
                          magicrect.top = t * 60 + 17;
                          magicrect.left = tt * 50 + 8;
                          magicrect.right = magicrect.left + 48;
                          magicrect.bottom = magicrect.top + 48;
                          if (showmagic)
                            PlotCIcon(&magicrect, showmagic);
                        }

                        if (iconhand) {
                          PlotCIcon(&icon, iconhand);
                          DisposeCIcon(iconhand);
                        }
                      }
                    }
                  }
                }
                SetPort(GetWindowPort(bootywindow));
                BackPixPat(whitepat);
                if (!identflag)
                  ploticon3(130, r);
                identflag = FALSE;
              }

              if ((theControl == downarrow) && (numtreasure > (42 + (21 * screensize)))) {
                Rect r;
                scroll.top = 15;
                scroll.bottom = 438;
                sound(141);
                GetControlBounds(downarrow, &r);
                ploticon3(133, r);
                BackColor(whiteColor);
                BackPixPat(whitepat); // Myriad

                while ((bootyitemindex * (6 + (3 * screensize)) < numtreasure - (42 + (21 * screensize))) && (StillDown())) {
                  bootyitemindex++;
                  for (t = 0; t < 4; t++)
                    ScrollRect(&scroll, 0, -15, 0L);
                  delay(2); // Fantasoft 7.1  Slows it down a bit on fast machines.

                  for (t = 0; t < 6 + (3 * screensize); t++) {
                    temp = list[bootyitemindex * (6 + (3 * screensize)) + t + 36 + (18 * screensize)];
                    if (temp) {
                      loaditem(temp);
                      iconhand = NIL;
                      if (temp < 0)
                        iconhand = GetCIcon(lookupicon(item.iconid, TRUE));
                      else
                        iconhand = GetCIcon(lookupicon(item.iconid, idtoggle));

                      icon.top = 385;
                      icon.bottom = icon.top + 32;
                      icon.left = 50 * t + 16;
                      icon.right = icon.left + 32;
                      if (temp < 0) {
                        magicrect.top = 377;
                        magicrect.left = 50 * t + 8;
                        magicrect.right = magicrect.left + 48;
                        magicrect.bottom = magicrect.top + 48;
                        if (showmagic)
                          PlotCIcon(&magicrect, showmagic);
                      }
                      if (iconhand) {
                        PlotCIcon(&icon, iconhand);
                        DisposeCIcon(iconhand);
                      }
                    }
                  }
                }
                ploticon3(134, r);
                RGBBackColor(&greycolor);
              }

              if (theControl == charbut) {
                charselectnew = point.v / 50;
                if (charselectnew <= charnum) {
                  sound(141);
                  buttonrect.top = 50 * charselectnew;
                  buttonrect.left = 330 + leftshift;
                  buttonrect.right = 380 + leftshift;
                  buttonrect.bottom = buttonrect.top + 50;
                  ploticon3(129, buttonrect);
                  DrawPicture(grey, &selectrect);
                  selectrect.top = buttonrect.top;
                  selectrect.bottom = buttonrect.bottom;
                  DrawPicture(marker, &selectrect);
                  ploticon3(130, buttonrect);
                }
              }
            } else if (charselectnew > -1) {
            newitem:

              leftindex = (point.h - 10) / 50;
              downindex = ((point.v - 10) / 60) + bootyitemindex;

              if (leftindex < 6 + (3 * screensize)) {
                element = leftindex + downindex * (6 + (3 * screensize));
                if (list[element]) {
                  SetPort(GetWindowPort(bootywindow));
                  BackPixPat(whitepat);
                  itemid = abs(list[element]);
                  if (itemid) {
                    loaditem(itemid);

                    if (gTheEvent.modifiers & optionKey) /**** Getinfo ****/
                    {
                      BitMapPtr src = GetPortBitMapForCopyBits(GetWindowPort(bootywindow));
                      BitMapPtr dst = GetPortBitMapForCopyBits(gbuff2);

                      magicrect.top = 0;
                      magicrect.left = 320 + leftshift;
                      magicrect.right = 640 + leftshift;
                      magicrect.bottom = 320;

                      SetPort(GetWindowPort(bootywindow));
                      BackPixPat(whitepat);
                      ForeColor(blackColor);
                      BackColor(whiteColor);

                      CopyBits(src, dst, &magicrect, &lookrect, 0, NIL);

                      sound(30005);

                      PenMode(2);
                      FrameOval(&box);
                      temp = idtoggle;

                      if (list[element] < didfum)
                        temp = TRUE;

                      reply = showiteminfo(itemid, temp, item.charge, 0);
                      if (reply)
                        goto newitem;
                      SetRect(&oldbox, 0, 0, 0, 0);
                      SetRect(&box, 0, 0, 0, 0);
                      goto iteminfoupdate;
                    }

                    if (item.type == 13) {
                      for (t = 0; t < c[charselectnew].numitems; t++) {
                        loaditem(c[charselectnew].items[t].id);
                        if (item.type == 13) /********** check for multiple scroll cases ***/
                        {
                          warn(32);
                          make = FALSE;
                          goto update2;
                        }
                      }
                    }

                    loaditem(itemid);

                    if ((c[charselectnew].numitems < 30) && (c[charselectnew].load + item.wieght < c[charselectnew].loadmax)) {
                      if ((item.cost < 0) && (allowunique)) /**** check for unique item *****/
                      {
                        if (checkforitem(abs(list[element]), FALSE, -1)) {
                          warn(116);
                          make = FALSE;
                          goto keeptaking;
                        }
                      }

                      if (autojoin) {
                        if (item.xcharge) {
                          for (t = 0; t < c[charselectnew].numitems; t++) {
                            if (c[charselectnew].items[t].id == item.itemid) {
                              c[charselectnew].items[t].charge += item.charge;
                              truejoin = TRUE;

                              goto pastjoin;
                            }
                          }
                        }
                      }

                    pastjoin:

                      c[charselectnew].load += (item.wieght + (item.xcharge * item.charge));

                      if (!truejoin) {
                        c[charselectnew].items[c[charselectnew].numitems].id = itemid;
                        c[charselectnew].items[c[charselectnew].numitems].charge = item.charge;
                        c[charselectnew].items[c[charselectnew].numitems].ident = c[charselectnew].items[c[charselectnew].numitems].equip = 0;
                        if ((abs(item.type) == 24) || (idtoggle) || (list[element] < didfum))
                          c[charselectnew].items[c[charselectnew].numitems].ident = TRUE;
                        c[charselectnew].numitems++;
                      }

                      truejoin = FALSE;

                      movecalc(charselectnew);
                      updatebooty(charselectnew);

                    keeptaking:

                      list[element] = 0;
                      icon.top = (downindex - bootyitemindex) * 60 + 17;
                      icon.left = leftindex * 50 + 8;
                      icon.right = icon.left + 48;
                      icon.bottom = icon.top + 48;
                      temprect = icon;
                      InsetRect(&temprect, 10, 10);

                      PenMode(2);
                      FrameOval(&oldbox);
                      PenMode(0);
                      for (t = 0; t < 24; t++) {
                        RGBForeColor(&tempcolor);
                        InsetRect(&icon, 1, 1);
                        InsetRect(&temprect, -1, -1);
                        PenMode(2);
                        FrameOval(&temprect);
                        PenMode(0);
                        FrameOval(&icon);
                        PenMode(2);
                        tempcolor.red = 2 * Rand(32760);
                        tempcolor.blue = 2 * Rand(32760);
                        tempcolor.green = 2 * Rand(32760);
                        FrameOval(&temprect);
                        PenMode(0);
                        ForeColor(whiteColor);
                        FrameOval(&icon);
                      }
                      PenMode(2);
                      FrameOval(&oldbox);
                      PenMode(0);
                      sound(6002);
                    } else
                      warn(43);
                  }
                }
              }
            }
            break;
        }
    }
  }
}
