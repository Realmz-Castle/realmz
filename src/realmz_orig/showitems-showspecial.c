#include "prototypes.h"
#include "variables.h"

/* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
 * NOTE(danapplegate): I'm not sure how the goshopinshop and other ControlHandles were
 * initialized in the original code, but it appears that this was an area of active development.
 * Based on a fix commit by a previous developer, it seems this code was broken in the initial commit
 * of the archive repo we have access to. I've restored his fix by introducing the MoveControlPair
 * struct and updating MoveControlByID in this file.
 */
typedef struct {
  ControlHandle* h;
  int16_t id;
} MoveControlPair;

static inline ControlHandle MoveControlByID(int16_t id, WindowPtr dlg, int16_t dx, int16_t dy) {
  Rect r;
  ControlHandle h = GetNewControl(id, dlg);
  GetControlBounds(h, &r);
  MoveControl(h, r.left + dx, r.top + dy);
  return h;
}

/***************************** Showitems ********************************/
void Showitems(short mode) {
  inshop = TRUE;

  SetMenuBar(copywright);
  DrawMenuBar();

  SetCCursor(sword);
  for (t = 0; t <= numchannel; t++)
    quiet(t);

  compactheap();

  if (!mode) {
    MoveControlPair move_ids[] = {
        {&goshopinshop, 144},
        {&rightcharacter, 153},
        {&leftcharacter, 137},
        {&poolshop, 138},
        {&shareshop, 139},
        {&leftarrow, 200},
        {&rightarrow, 201},
        {&charitemsvert, 128},
        {&shopitemsvert, 129},
        {&departshop, 141},
        {&moneybut, 136},
        {&goitems, 133}};
    unsigned int i = 0;
    unsigned int count = sizeof(move_ids) / sizeof(move_ids[0]);

    compactitems(cl);
    compactitems(cr);
    gshop = GetNewWindow(129, 0L, (WindowPtr)-1L);
#ifdef PC
    AcamWindowRgbBackColor(gshop, 0xFFFF, 0xFFFF, 0xFFFF);
#endif
    SetPort(GetWindowPort(gshop));
    TextFace(NIL);

    for (i = 0; i < count; i++) {
      MoveControlPair p = move_ids[i];
      *(p.h) = MoveControlByID(p.id, gshop, (leftshift / 2), 0);
    }

    if (shopavail) {
      MoveControlPair move_ids[] = {
          {&weapons, 131},
          {&armor, 130},
          {&helms, 132},
          {&magic, 134},
          {&supplies, 135}};

      i = 0;
      count = sizeof(move_ids) / sizeof(move_ids[0]);

      for (i = 0; i < count; i++) {
        MoveControlPair p = move_ids[i];
        *(p.h) = MoveControlByID(p.id, gshop, (leftshift / 2), 0);
      }
    }
  }
  /* *** END CHANGES *** */

  blank = TRUE;
  SetPort(GetWindowPort(gshop));
  TextSize(14);
  TextFont(font);
  TextMode(0);

  itemRect.top = 387;
  itemRect.bottom = itemRect.top + 73;
  itemRect.left = 0 + (leftshift / 2);
  itemRect.right = 640 + (leftshift / 2);

  MoveWindow(gshop, GlobalLeft, GlobalTop, FALSE);
  ShowWindow(gshop);

  if (mode != 2) {
    pict(148, itemRect);
    if (screensize) {
      itemRect.left = 0;
      itemRect.right = 80;
      pict(217, itemRect);
      itemRect.left += 720;
      itemRect.right += 720;
      pict(217, itemRect);
    }
  }

  if (!reducesound)
    sound(30005);

  itemRect.top = 0;
  itemRect.bottom = 387;
  itemRect.left = 263 + (leftshift / 2);
  itemRect.right = itemRect.left + 114;
  HideControl(charitemsvert);
  HideControl(shopitemsvert);

  pict(170, itemRect);

  ShowControl(charitemsvert);
  ShowControl(shopitemsvert);

  if (!shopavail) {
    RGBBackColor(&greycolor);
    itemRect.bottom -= 50;
    itemRect.left += 32;
    itemRect.right -= 32;
    BackPixPat(base);
    EraseRect(&itemRect);
    BackPixPat(whitepat);
    BackColor(whiteColor);
  }

  if (characterl.numitems - 9 > 0)
    SetControlMaximum(charitemsvert, characterl.numitems - 9);
  else
    SetControlMaximum(charitemsvert, 0);
  if (!mode)
    shopselection = 0;

  if (mode == 2) {
    theControl = rightarrow;
    change(0, 2);
    return;
  }

  theControl = leftarrow;
  change(0, 0);
  theControl = rightarrow;
  change(0, 0);

  quickinfo(cl, 0, c[cl].items[0].id, 0);
}

/***************************** showitemstats *******************/
void showitemstats(short ident) {
  if ((!iteminfo) || (!ident))
    return;

  if (item.iscurse)
    loaditem(item.iscurse); /********* so details dont give away cursed items ****/ // Fantasoft v7.0

  MoveTo(icon.left + 34, icon.top + 30);
  ForeColor(blackColor);
  TextSize(10);
  TextMode(1);
  TextFont(genevafont);

  if (item.vssmall > 0) {
    ForeColor(redColor);
    MyrDrawCString("Damage ");
    string(1 + item.damage);
    MyrDrawCString(" to ");
    string(item.damage + item.vssmall);

    if (item.heat + item.cold + item.electric + item.vsundead + item.vsdd + item.vsevil)
      MyrDrawCString(" + Special  ");
    else
      MyrDrawCString("  ");
  }

  if (item.damage > 0) {
    if (!item.vssmall) {
      ForeColor(redColor);
      MyrDrawCString("Damage +");
      string(item.damage);
      MyrDrawCString("  ");
    }

    ForeColor(blackColor);
    MyrDrawCString("Hit +");
    string(item.damage * 5);
    MyrDrawCString("%");
    MyrDrawCString("   ");
  } else if (item.vssmall)
    MyrDrawCString("  ");

  if (item.ac > 0) {
    ForeColor(blueColor);
    MyrDrawCString("Armor +");
    string(item.ac);
  }
  ForeColor(blackColor);
  TextFont(font);
  TextFace(NIL);
  TextSize(15);
  TextMode(0);
}

/********************************* showspecial *******************/
void showspecial(void) {
  DialogRef special;
  short index = 1;
  short temp;

  special = GetNewDialog(160, 0L, (WindowPtr)-1L);

  SetPortDialogPort(special);
  TextFont(defaultfont);
  gCurrent = special;
  BackPixPat(base);
  ForeColor(yellowColor);
  if (!reducesound)
    sound(647);

  for (t = 17; t < 31; t++) {
    if (characterl.spec[t - 17]) {
      GetIndString(myString, 1201, t - 16);

      if (StringWidth(myString)) {
        MyrPascalDiStr(index + 16, myString);
        DialogNum(index++ + 29, characterl.spec[t - 17]);
      }
    }
  }
  SizeWindow(GetDialogWindow(special), 300, 32 * index, 0);
  MoveWindow(GetDialogWindow(special), GlobalLeft + 150 + (leftshift / 2), GlobalTop + 100 + (downshift / 2), FALSE);
  ShowWindow(GetDialogWindow(special));
  ErasePortRect();

  FlushEvents(everyEvent, 0);
  ModalDialog(0L, &temp);

  sound(141);
  DisposeDialog(special);
}

/********************* canuse *****************************/
short canuse(short itemid, short raceid, short casteid) {
  register short tt;
  loadprofile(raceid, casteid);
  if (itemid > 0)
    loaditem(itemid);

  cantuseflag = cantuseflag2 = 0;

  /*************** check for specific race/caste required *************/

  if (item.specificrace) {
    if (raceid != item.specificrace)
      cantuseflag = TRUE;
  }

  if (item.specificcaste) {
    if (casteid != item.specificcaste)
      cantuseflag2 = TRUE;
  }

  if (!cantuseflag) {
    for (tt = 0; tt < 58; tt++) {
      if (MyrBitTstLong(&item.itemcat[tt / 32], tt % 32))
        goto pushon; /********** locate item catagory ********/
    }

  pushon:

    if ((MyrBitTstLong(&caste.itemtypes[tt / 32], tt % 32)) && (MyrBitTstLong(&races.itemtypes[tt / 32], tt % 32))) /********* can use item catagory ? ****/
    {
      /************** they CAN use the item.  Check for other restrictions *****************/

      for (tt = 0; tt < 9; tt++) /******** look for race restriction ************/
      {
        if (MyrBitTstShort(&item.racerestrictions, tt)) /******** test to see if there is some type of race restriction ***/
        {
          if (MyrBitTstShort(&races.descriptors, tt))
            cantuseflag = TRUE;
        }
      }

      for (tt = 0; tt < 9; tt++) /******** look for race class restriction ************/
      {
        if (MyrBitTstShort(&item.raceclassonly, tt)) /******** test to see if there is some type of race class restriction ***/
        {
          if (!MyrBitTstShort(&races.descriptors, tt))
            cantuseflag = TRUE;
        }
      }

      for (tt = 0; tt < 7; tt++) /******** look for caste restriction ************/
      {
        if (MyrBitTstShort(&item.casterestrictions, tt)) /******** test to see if there is some type of caste restriction ***/
        {
          if (tt == (caste.casteclass - 1))
            cantuseflag2 = TRUE;
        }
      }

      if (item.casteclassonly) {
        if (!MyrBitTstShort(&item.casteclassonly, caste.casteclass - 1))
          cantuseflag2 = TRUE; /******** test to see if there is some type of caste class restriction ***/
      }

      if ((!cantuseflag) && (!cantuseflag2))
        return (TRUE);
    } else {
      if (!MyrBitTstLong(&caste.itemtypes[tt / 32], tt % 32))
        cantuseflag2 = TRUE;
      else
        cantuseflag = TRUE;
    }
  }

  return (FALSE);
}

/***************************** showiteminfo ************************/
short showiteminfo(short id, short ident, short charge, short showcurse) {
  char special[255];
  DialogRef info;
  WindowPtr whichWindow;
  Boolean comma;
  short showindex;

  info = GetNewDialog(149, 0L, (WindowPtr)-1L);
  SetPortDialogPort(info);

  MoveWindow(GetDialogWindow(info), GlobalLeft + 285 + leftshift, GlobalTop, FALSE);
  ShowWindow(GetDialogWindow(info));
  BackPixPat(base);
  DrawDialog(info);

reload:
  SetPortDialogPort(info);
  gCurrent = info;
  ForeColor(yellowColor);
  TextFont(defaultfont);
  loaditem(id);

  GetDialogItem(info, 4, &itemType, &itemHandle, &icon);

  if ((item.iscurse) && (!showcurse)) {
    temp = getselection(item.iscurse);
    textbox(temp + (2 * ident), item.iscurse - temp + 1, FALSE, TRUE, icon);
  } else {
    getselection(item.itemid);

    textbox(tempselection + (2 * ident), item.itemid - tempselection + 1, FALSE, TRUE, icon);
  }

  if ((item.iscurse) && (!showcurse)) {
    temp = item.iconid; // Store original icon ID
    loaditem(item.iscurse); // load in cursed items so stats match. ***/
    item.iconid = temp; // Restore icon of original item so it does not give cursed item away.
  }

  TextFont(defaultfont);
  TextSize(12);
  TextFace(0);
  RGBForeColor(&greencolor);
  DialogNum(18, item.wieght + item.xcharge * charge);
  if (item.nohands)
    DialogNum(21, item.nohands);
  RGBForeColor(&cyancolor);
  if (item.vssmall)
    DialogNum(29, item.vssmall);
  showindex = 5;

  if (ident) {
    ForeColor(whiteColor);
    if (item.sp1 > 19) {
      GetIndString(myString, 136, item.sp1 - 19);
      MyrPascalDiStr(showindex++, myString);
    }

    if (item.sp1 == -10) /*********** inflicts condition ******/
    {
      strcpy(special, (StringPtr) "This item inflicts the condition:   ");
      GetIndString(myString, 133, item.sp3 - 19);
      PtoCstr(myString);
      strcat(special, myString);
      CtoPstr(special);
      MyrPascalDiStr(showindex++, (StringPtr)special);
    }

    if (item.sp2 > 1100) {
      loadspell2(item.sp2);
      strcpy(special, (StringPtr) "This item stores the spell:   ");
      GetIndString(myString, 1000 * (castcaste + 1) + castlevel, castnum + 1);
      PtoCstr(myString);
      strcat(special, myString);
      CtoPstr(special);
      MyrPascalDiStr(showindex++, (StringPtr)special);
    }

    if (item.sp1 == 122) /******* adds attacks *********/
    {
      switch (item.sp2) {
        case 1:
          MyrCDiStr(showindex++, (StringPtr) "Gives Character An Extra 1/2 Attack Per combatround");
          break;

        case 2:
          MyrCDiStr(showindex++, (StringPtr) "Gives Character An Extra Attack Per combatround");
          break;

        case 3:
          MyrCDiStr(showindex++, (StringPtr) "Gives Character An Extra 1 and 1/2 Attacks Per combatround");
          break;

        case 4:
          MyrCDiStr(showindex++, (StringPtr) "Gives Character An Extra 2 Attacks Per combatround");
          break;
      }
    }

    if (item.heat + item.cold + item.electric + item.vsundead + item.vsdd + item.vsevil) {
      MyrCDiStr(showindex++, (StringPtr) "This weapon does special damage.");
    }

    if ((item.type < 0) || (item.type == 25) || (item.type == 23)) {
      MyrCDiStr(showindex++, (StringPtr) "This special item is only available in this scenario.  It will not transfer to other scenarios.");
    }

    if (item.sp1 == 121) /******* penetration weapon *********/
    {
      MyrCDiStr(showindex++, (StringPtr) "This weapon gains double magical plus to hit during combat.  Example: A +2 weapon will hit as a +4 weapon but do damage as a +2 weapon.");
    }

    if (item.sp1 == 120) /******* auto hit weapon *********/
    {
      MyrCDiStr(showindex++, (StringPtr) "This weapon ALWAYS hits in combat.");
    }

    if (item.sp3 < 0) /****** ability to hit monster types ****/
    {
      strcpy(special, (StringPtr) "");

      if (item.sp5 > 0)
        strcpy(special, (StringPtr) "+");

      MyrNumToString(item.sp5, myString);
      strcat(special, myString);
      GetIndString(myString, 132, abs(item.sp3));
      PtoCstr(myString);
      strcat(special, myString);
      CtoPstr(special);
      MyrPascalDiStr(showindex++, (StringPtr)special);
    }

    if ((item.sp3 > 0) && (item.sp3 < 16)) /****** add to characters special abilitys like pick lock ****/
    {
      strcpy(special, (StringPtr) "");

      if (item.sp5 > 0)
        strcpy(special, (StringPtr) "+");

      MyrNumToString(item.sp5, myString);
      strcat(special, myString);
      strcat(special, (StringPtr) " to ");
      GetIndString(myString, 1201, item.sp3);
      PtoCstr(myString);
      strcat(special, myString);
      CtoPstr(special);
      MyrPascalDiStr(showindex++, (StringPtr)special);
    }

    if ((item.sp4 > 0) && (item.sp4 < 16)) /****** add to characters special abilitys like pick lock ****/
    {
      strcpy(special, (StringPtr) "");

      if (item.sp5 > 0)
        strcpy(special, (StringPtr) "+");

      MyrNumToString(item.sp5, myString);
      strcat(special, myString);
      strcat(special, (StringPtr) " to ");
      GetIndString(myString, 1201, item.sp4);
      PtoCstr(myString);
      strcat(special, myString);
      CtoPstr(special);
      MyrPascalDiStr(showindex++, (StringPtr)special);
    }

    RGBForeColor(&greencolor);
    if (item.damage)
      DialogNum(15, item.damage);
    if (item.st)
      DialogNum(16, item.st);
    if (item.lu)
      DialogNum(17, item.lu);
    if (item.ac)
      DialogNum(19, item.ac);
    if (item.movement)
      DialogNum(20, item.movement);
    if (item.magres)
      DialogNum(22, item.magres);
    if (item.spellpoints)
      DialogNum(23, item.spellpoints);

    RGBForeColor(&cyancolor);

    if (item.heat)
      DialogNum(9, item.heat);
    if (item.cold)
      DialogNum(10, item.cold);
    if (item.electric)
      DialogNum(11, item.electric);
    if (item.vsundead)
      DialogNum(12, item.vsundead);
    if (item.vsdd)
      DialogNum(13, item.vsdd);
    if (item.vsevil)
      DialogNum(14, item.vsevil);

    GetDialogItem(info, 27, &itemType, &itemHandle, &icon);
    if (item.cost < 0)
      DrawPicture(on, &icon);
    else
      DrawPicture(non, &icon);
  }

  ForeColor(whiteColor);
  TextFont(defaultfont);
  TextSize(12);
  TextFace(bold);

  if (!canuse(-1, c[charselectnew].race, c[charselectnew].caste)) {
    if (cantuseflag)
      MyrCDiStr(26, (StringPtr) "Racial Conflict");
    if (cantuseflag2)
      MyrCDiStr(28, (StringPtr) "Caste Conflict");
    ForeColor(redColor);
    MyrCDiStr(25, (StringPtr) "N");
  } else
    MyrCDiStr(25, (StringPtr) "Y");

  ForeColor(whiteColor);
  TextFace(0);

  if (item.specificcaste) /******* show usable by specific caste ******/
  {
    strcpy(special, (StringPtr) "Usable Only To A ");
    GetIndString(myString, 131, item.specificcaste);
    PtoCstr(myString);
    strcat(special, myString);
    CtoPstr(special);
    MyrPascalDiStr(showindex++, (StringPtr)special);
  }

  if (item.specificrace) /******* show usable by specific race ******/
  {
    strcpy(special, (StringPtr) "Usable Only To A ");
    GetIndString(myString, 129, item.specificrace);
    PtoCstr(myString);
    strcat(special, myString);
    CtoPstr(special);
    MyrPascalDiStr(showindex++, (StringPtr)special);
  }

  if (item.casterestrictions) /******* show nonusable by specific caste class******/
  {
    comma = 0;
    strcpy(special, (StringPtr) "This item is not usable by ");
    for (t = 0; t < 7; t++) {
      if (MyrBitTstShort(&item.casterestrictions, t)) {
        GetIndString(myString, 141, t + 1);
        PtoCstr(myString);
        if (comma)
          strcat(special, (StringPtr) ", ");
        strcat(special, myString);
        comma = TRUE;
      }
    }
    if (comma) {
      CtoPstr(special);
      MyrPascalDiStr(showindex++, (StringPtr)special);
    }
  }

  if (item.racerestrictions) /******* show non-usable by specific race class******/
  {
    comma = 0;
    strcpy(special, (StringPtr) "This item is not usable by ");
    for (t = 0; t < 9; t++) {
      if (MyrBitTstShort(&item.racerestrictions, t)) {
        GetIndString(myString, 130, t + 1);
        PtoCstr(myString);
        if (comma)
          strcat(special, (StringPtr) ", ");
        strcat(special, myString);
        comma = TRUE;
      }
    }
    if (comma) {
      CtoPstr(special);
      MyrPascalDiStr(showindex++, (StringPtr)special);
    }
  }

  if (item.casteclassonly) /******* show usable by specific caste classes ******/
  {
    comma = 0;
    strcpy(special, (StringPtr) "This is usable only by ");
    for (t = 0; t < 7; t++) {
      if (MyrBitTstShort(&item.casteclassonly, t)) {
        GetIndString(myString, 141, t + 1);
        PtoCstr(myString);
        if (comma)
          strcat(special, (StringPtr) ", ");
        strcat(special, myString);
        comma = TRUE;
      }
    }
    if (comma) {
      CtoPstr(special);
      MyrPascalDiStr(showindex++, (StringPtr)special);
    }
  }

  if (item.raceclassonly) /******* show usable by specific race classes ******/
  {
    comma = 0;
    strcpy(special, (StringPtr) "This is usable only by ");
    for (t = 0; t < 9; t++) {
      if (MyrBitTstShort(&item.raceclassonly, t)) {
        GetIndString(myString, 130, t + 1);
        PtoCstr(myString);
        if (comma)
          strcat(special, (StringPtr) ", ");
        strcat(special, myString);
        comma = TRUE;
      }
    }
    if (comma) {
      CtoPstr(special);
      MyrPascalDiStr(showindex++, (StringPtr)special);
    }
  }

  GetDialogItem(info, 24, &itemType, &itemHandle, &icon);
  ploticon2(lookupicon(item.iconid, ident));

backup:

  BeginUpdate(GetDialogWindow(info));
  EndUpdate(GetDialogWindow(info));

  FlushEvents(everyEvent, 0);

  for (;;) {
    WaitNextEvent(everyEvent, &gTheEvent, 0L, NIL);
#ifdef PC // Myriad
    DoCorrectBugMADRepeat();
#endif

    if (IsDialogEvent(&gTheEvent)) {
      if (DialogSelect(&gTheEvent, &dummy, &itemHit)) {
        if (itemHit == 2) {
          loadprofile(0, c[charselectnew].caste);
          showcondition(charselectnew, charselectnew, 3, 1, charselectnew);
          goto backup;
        }

        if (itemHit == 1) {
          showcondition(charselectnew, charselectnew, 4, 1, charselectnew);
          goto backup;
        }

        if (itemHit)
          goto out;
      }
    } else {
      switch (gTheEvent.what) {
        case mouseDown:
          thePart = FindWindow(gTheEvent.where, &whichWindow);
          point = gTheEvent.where;

          if (whichWindow == itemswindow) {
            SetPort(GetWindowPort(itemswindow));
            GlobalToLocal(&point);
            if (point.h < 280) {
              itemselectnew = incr + ((point.v - 11) / 40);
              if (itemselectnew < c[charselectnew].numitems) {
                id = c[charselectnew].items[itemselectnew].id;
                ident = c[charselectnew].items[itemselectnew].ident;
                charge = c[charselectnew].items[itemselectnew].charge;
                showcurse = c[charselectnew].items[itemselectnew].equip;
                for (t = 4; t < 27; t++)
                  MyrCDiStr(t, (StringPtr) "");
                MyrCDiStr(28, (StringPtr) "");
                goto reload;
              }
            } else
              goto out;
          } else if (whichWindow == gshop) {
            SetPort(GetWindowPort(gshop));
            GlobalToLocal(&point);
            if (point.h < 280) {
              itemselectnew = (point.v - 11) / 40 + GetControlValue(charitemsvert);
              SetPort(GetWindowPort(gshop));
              id = characterl.items[itemselectnew].id;
              ident = characterl.items[itemselectnew].ident;
              charge = characterl.items[itemselectnew].charge;
              showcurse = characterl.items[itemselectnew].equip;
              for (t = 4; t < 27; t++)
                MyrCDiStr(t, (StringPtr) "");
              MyrCDiStr(28, (StringPtr) "");
              goto reload;
            }
          } else if (whichWindow == bootywindow) {
            SetPort(GetWindowPort(bootywindow));
            GlobalToLocal(&point);
            leftindex = (point.h - 10) / 50;
            downindex = ((point.v - 10) / 60) + bootyitemindex;

            if (leftindex < 6 + (3 * screensize)) {
              if (list[leftindex + downindex * (6 + (3 * screensize))]) {
                loaditem(list[leftindex + downindex * (6 + (3 * screensize))]);
                id = item.itemid;
                charge = item.charge;
                ident = idtoggle;
                showcurse = FALSE;
                for (t = 4; t < 27; t++)
                  MyrCDiStr(t, (StringPtr) "");
                MyrCDiStr(28, (StringPtr) "");
                goto reload;
              }
            }
          }

          break;
      }
    }
  }
out:
  DisposeDialog(info);
  FlushEvents(everyEvent, 0);
  return (0);
}
