#include "prototypes.h"
#include "variables.h"

/************************* itemcost *************************/
void itemcost(short mode) /****** 1 = sell, else buy ****/
{
  float newcost;

  newcost = abs(item.cost);
  if (!mode)
    newcost *= theshop.inflation;
  else {
    if (theshop.inflation > 100)
      newcost *= 100;
    else
      newcost *= theshop.inflation;
  }

  newcost /= 100;
  if (newcost > 32000)
    newcost = 32000;

  item.cost = newcost;
}

/****************************** moveicon *********************/
void moveicon(void) {
  float charge1, charge2 = 0;
  short valu, max, tempshop;
  Rect iconpict, iconnp, iconop, iconstore, store;
  Point oldpoint;
  Boolean empty = 0;
  Boolean doupdateshop = 0;
  Boolean prob = 0;
  Boolean truejoin = 0;
  short leftvalue, temp;
  struct itemattr tempitem;
  Boolean baditem = 0;
  int index;
  BitMap src;
  BitMap* dst = GetPortBitMapForCopyBits(gedge);
  GetQDGlobalsScreenBits(&src);

  store.top = 0;
  store.bottom = 32;
  store.left = 0;
  store.right = 32;

  TextMode(1);
  TextFont(font);

  if (theControl != charitemsvert)
    theControl = shopitemsvert;
  curControlValue = GetControlValue(theControl);
  index = curControlValue + gshopitem;
  leftvalue = GetControlValue(charitemsvert);

  if (point.h <= (320 + (leftshift / 2))) {
    tempid = characterl.items[gshopitem + GetControlValue(charitemsvert) - 1].id;
    charge1 = characterl.items[gshopitem + GetControlValue(charitemsvert) - 1].charge;
    lg = characterl.items[gshopitem + GetControlValue(charitemsvert) - 1].ident;

    if (((acceptlowrange1) || (acceptlowrange2)) && (cr == -1)) //** shop only deals with specific items.
    {
      baditem = 0;

      if ((!twixt(tempid, acceptlowrange1, accepthighrange1) && (acceptlowrange1)))
        baditem++;
      if ((!twixt(tempid, acceptlowrange2, accepthighrange2) && (acceptlowrange2)))
        baditem++;

      if (baditem == 2) {
        ShowCursor();
        warn(140);
        return;
      }
    }
  }

  if (point.h > (320 + (leftshift / 2))) {
    if (cr > -1) {
      tempid = characterr.items[gshopitem + GetControlValue(shopitemsvert) - 1].id;
      lg = characterr.items[gshopitem + GetControlValue(shopitemsvert) - 1].ident;
    } else {
      tempid = theshop.id[shopselection + index - 1];
      lg = TRUE; /*** shop items are always identified ***/

      if (((acceptlowrange1) || (acceptlowrange2)) && (cr == -1)) //** shop only deals with specific items.
      {
        baditem = 0;

        if ((!twixt(tempid, acceptlowrange1, accepthighrange1) && (acceptlowrange1)))
          baditem++;
        if ((!twixt(tempid, acceptlowrange2, accepthighrange2) && (acceptlowrange2)))
          baditem++;

        if (baditem == 2) {
          ShowCursor();
          warn(140);
          return;
        }
      }
    }
  }

  loaditem(tempid);

  item.cost = abs(item.cost);

  charge2 = item.charge;

  if (point.h > (320 + (leftshift / 2))) /********* get correct item wieght *********/
  {
    if (cr > -1)
      item.charge = characterr.items[gshopitem + GetControlValue(shopitemsvert) - 1].charge;
  } else
    item.charge = charge1;

  TextSize(14);
  RGBForeColor(&cyancolor);
  MoveTo(212 + (leftshift / 2), 427);
  string(item.wieght + item.charge * item.xcharge);

  if (cr != -1)
    item.cost = 0;

  if ((point.h > (320 + (leftshift / 2))) && (cr == -1)) {
    ForeColor(yellowColor);
    MoveTo(109 + (leftshift / 2), 427);
    itemcost(0);
    string(item.cost);
  } else if (shopavail) {

    if (item.charge > 0)
      charge1 = item.charge;
    else
      charge1 = 0;

    item.cost /= 2;

    charge2 = item.charge / charge2;

    item.cost *= charge2;

    if (cr == -1) {
      MoveTo(159 + (leftshift / 2), 427);
      RGBForeColor(&greencolor);
      itemcost(1);
      if (!lg)
        item.cost = item.cost / 50; /*** unidentified item ****/
      string(item.cost);
    } else
      item.cost = 0;
  }

  skip = FALSE;

  iconstore.top = 395;
  iconstore.left = 68 + (leftshift / 2);
  iconstore.right = iconstore.left + 32;
  iconstore.bottom = 427;

  iconpict.top = 40 * gshopitem - 24;
  iconpict.left = 390 - 380 * ischar;

  if ((screensize) && (!ischar)) {
    iconpict.left += (leftshift / 2);
  }

  iconpict.right = iconpict.left + 32;
  iconpict.bottom = iconpict.top + 32;

  iconnp.top = point.v;
  iconnp.left = point.h;
  iconnp.bottom = iconnp.top + 32;
  iconnp.right = iconnp.left + 32;

  iconop = iconnp;

  ForeColor(yellowColor);
  eraseshopname(0); /*** left ***/

  TextSize(13);
  GetIndString(myString, getselection(item.itemid) + lg, item.itemid - tempselection + 1);

  if (item.iscurse) {
    temp = getselection(item.iscurse);
    GetIndString(myString, temp + lg, item.iscurse - temp + 1);
    if ((cr == -1) && (point.h > (320 + (leftshift / 2))))
      GetIndString(myString, temp + lg, item.itemid - temp + 1);
  }

  PtoCstr(myString);

  MyrDrawCString((Ptr)myString);
  TextSize(16);
  ForeColor(blackColor);
  BackColor(whiteColor);

  if (point.h <= (320 + (leftshift / 2))) {
    if (cr != -1) {
      if (!canuse(-1, characterr.race, characterr.caste))
        prob = TRUE;
    }
  } else {
    if (!canuse(-1, characterl.race, characterl.caste))
      prob = TRUE;
  }

  if (gTheEvent.modifiers & optionKey) {
    ShowCursor();
    if ((point.h > (320 + (leftshift / 2))) && (cr == -1)) {
      charselectnew = cl;
      showiteminfo(item.itemid, lg, item.charge, TRUE);
    } else {
      if (point.h > (320 + (leftshift / 2))) {
        charselectnew = cr;
        temp = gshopitem + GetControlValue(shopitemsvert) - 1;
        showiteminfo(item.itemid, lg, item.charge, characterr.items[temp].equip);
      } else {
        charselectnew = cl;
        temp = gshopitem + GetControlValue(charitemsvert) - 1;
        showiteminfo(item.itemid, lg, item.charge, characterl.items[temp].equip);
      }
    }

    Showitems(1);
    updateshopwings(-2, -2);
    return;
  }

  if (prob) {
    SetCCursor(stop);
    sound(674);
  } else {
    HideCursor();
    sound(130);
  }

  ploticon3(lookupicon(item.iconid, lg), iconstore);

  if ((theshop.inflation == 0) && (item.charge > -1) && (cr == -1) && (point.h <= (320 + (leftshift / 2)))) /********* warhouse takes no charged weapons ****/
  {
    ShowCursor();
    warn(80);
    RGBBackColor(&greycolor);
    ForeColor(yellowColor);
    EraseRect(&iconstore);
    eraseshopstats(1);
    return;
  }

  if (BitAnd(gTheEvent.modifiers, shiftKey)) /***** shift click mode ****/
  {
    GetMouse(&point);
    FlushEvents(everyEvent, 0);
    if (point.h <= 320 + (leftshift / 2))
      point.h = 500;
    else
      point.h = 35;
    goto fastmove;
  }

  iconop = iconpict;
  OffsetRect(&iconpict, GlobalLeft, GlobalTop);

  CopyBits(&src, dst, &iconpict, &store, 0, NIL);
  do {
    GetMouse(&point);

    if ((point.h != oldpoint.h) || (point.v != oldpoint.v)) {
      iconnp.top = point.v;
      iconnp.left = point.h;
      iconnp.bottom = iconnp.top + 32;
      iconnp.right = iconnp.left + 32;

      OffsetRect(&iconnp, GlobalLeft, GlobalTop);
      OffsetRect(&iconop, GlobalLeft, GlobalTop);

      CopyBits(dst, &src, &store, &iconop, 0, NIL);
      CopyBits(&src, dst, &iconnp, &store, 0, NIL);
      CopyBits(&src, &src, &iconpict, &iconnp, 39, NIL);
      OffsetRect(&iconnp, -GlobalLeft, -GlobalTop);

      iconop = iconnp;
      oldpoint = point;
    }
  } while (StillDown());

  OffsetRect(&iconop, GlobalLeft, GlobalTop);
  CopyBits(dst, &src, &store, &iconop, 0, NIL);

fastmove:

  BackPixPat(base);
  EraseRect(&iconstore);
  sound(83);
  ForeColor(yellowColor);
  TextSize(16);

  eraseshopname(0); /**** left ****/

  MyrDrawCString(c[cl].name);

  if (cr == -1)
    eraseshopstats(1);
  else
    eraseshopstats(0);

  SetCCursor(sword);
  ShowCursor();

  if (((point.h <= (320 + (leftshift / 2))) && (((characterl.numitems == 30) && (!ischar)) || ((characterl.load + item.wieght + (item.xcharge * item.charge) > characterl.loadmax) && (!ischar)))) || (((point.h > (320 + (leftshift / 2))) && (cr != -1)) && ((characterr.numitems == 30) || (characterr.load + item.wieght + (item.xcharge * item.charge) > characterr.loadmax)))) {
    ShowCursor();
    warn(19);
    updateshopwings(-2, -2);
    return;
  }

  if ((point.h > (320 + (leftshift / 2))) && (ischar) && (cr > -1)) /*******start move item stuff *********/
  {
    if (abs(item.type) == 13) {
      for (t = 0; t < characterr.numitems; t++)
        if (twixt(characterr.items[t].id, 800, 802)) {
        twoscrolls:
          ShowCursor();
          warn(32);
          return;
        }
      for (t = 0; t < 5; t++) {
        characterr.scrollcase[t] = characterl.scrollcase[t];
        characterl.scrollcase[t].powerlevel = characterl.scrollcase[t].castnum = characterl.scrollcase[t].castcaste = characterl.scrollcase[t].castlevel = 0;
      }
    }

    theControl = shopitemsvert;
    valu = GetControlValue(shopitemsvert);
    max = GetControlMaximum(shopitemsvert);
    temp = characterr.numitems;
    characterr.load += (item.wieght + item.xcharge * item.charge); /***** cr from cl ***************/
    characterl.load -= (item.wieght + item.xcharge * item.charge);
    movecalc(-2);
    movecalc(-1);

    if (autojoin) {
      if (item.xcharge) {
        for (t = 0; t < characterr.numitems; t++) {
          if (characterr.items[t].id == item.itemid) {
            characterr.items[t].charge += item.charge;
            truejoin = TRUE;

            if ((t - valu < 9) && (t - value > -1)) {
              icon.top = 16 + 40 * (t - valu);
              lg = characterr.items[t].ident;
              shopequip = characterr.items[t].equip;
              tempitem = item;
              ploticon(tempid, FALSE);
              item = tempitem;
              shopequip = FALSE;
              showcharge2(characterr.items[t].charge, lg);
            }

            goto pastjoin;
          }
        }
      }
    }

  pastjoin:

    if (!truejoin) {
      if (valu < max) {
        for (t = characterr.numitems; t > valu + 9; t--)
          characterr.items[t] = characterr.items[t - 1];
        temp = valu + 9;
      }

      characterr.items[temp] = characterl.items[leftvalue + gshopitem - 1];
      characterr.items[temp].equip = FALSE;

      if (characterr.numitems < 9) {
        icon.top = 16 + 40 * characterr.numitems;
        lg = characterr.items[temp].ident;
        tempitem = item;
        ploticon(tempid, FALSE);
        item = tempitem;
        showcharge2(characterr.items[temp].charge, lg);
      }
      characterr.numitems++;
      updateshopwings(-2, -2);
      if (characterr.numitems > 9)
        SetControlMaximum(shopitemsvert, characterr.numitems - 9);
      {
        theCode = kControlDownButtonPart;
        if (valu < max)
          skip = FALSE;
        ScrollProc(shopitemsvert, theCode);
      }
    }
  }

  if ((point.h < (250 + (leftshift / 2))) && (!ischar)) {
    if (abs(item.type) == 13) {
      for (t = 0; t < characterl.numitems; t++)
        if (twixt(characterl.items[t].id, 800, 802))
          goto twoscrolls;
      for (t = 0; t < 5; t++) {
        characterl.scrollcase[t] = characterr.scrollcase[t];
        characterr.scrollcase[t].powerlevel = 0;
      }
    }

    theControl = charitemsvert;
    valu = GetControlValue(charitemsvert);
    max = GetControlMaximum(charitemsvert);
    temp = characterl.numitems; /*********** cl from shop **************/

  trymore:

    if (item.cost > characterl.money[0] + moneypool[0]) {
      if (autocash) {
        if (moneypool[1]) {
          moneypool[1]--;
          moneypool[0] += 100;
          goto trymore;
        } else if (characterl.money[1]) {
          characterl.money[1]--;
          characterl.load--;
          moneypool[0] += 100;
          goto trymore;
        } else if (moneypool[2]) {
          moneypool[2]--;
          moneypool[1] += 5;
          goto trymore;
        } else if (characterl.money[2]) {
          characterl.money[2]--;
          characterl.load -= 15;
          moneypool[1] += 5;
          goto trymore;
        } else {
          ShowCursor();
          warn(37);
          return;
        }
      } else {
        ShowCursor();
        warn(37);
        return;
      }
      movecalc(-1);
      updateshop();
    }

    if (autojoin) {
      if (item.xcharge) {
        for (t = 0; t < characterl.numitems; t++) {
          if (characterl.items[t].id == item.itemid) {
            characterl.items[t].charge += item.charge;
            truejoin = TRUE;

            if ((t - valu < 9) && (t - value > -1)) {
              icon.top = 16 + 40 * (t - valu);
              lg = characterl.items[t].ident;
              shopequip = characterl.items[t].equip;
              tempitem = item;
              ploticon(tempid, FALSE);
              item = tempitem;
              shopequip = FALSE;
              showcharge2(characterl.items[t].charge, lg);
            }

            goto pastjoin2;
          }
        }
      }
    }

  pastjoin2:

    if (!truejoin) {
      if (valu < max) {
        for (t = characterl.numitems; t > valu + 9; t--)
          characterl.items[t] = characterl.items[t - 1];
        temp = valu + 9;
      }
    }

    moneypool[0] -= item.cost;

    if (moneypool[0] < 0) {
      characterl.money[0] += moneypool[0];
      characterl.load += moneypool[0];
      moneypool[0] = 0;
    }

    movecalc(-1);
    updateshop();

    if (!truejoin)
      characterl.items[temp].equip = FALSE;
    if (!truejoin)
      characterl.numitems++;
    updateshopwings(-2, -2);
    if (cr == -1) {
      tempshop = shopselection + gshopitem + GetControlValue(shopitemsvert) - 1;
      characterl.load += (item.wieght + item.xcharge * item.charge);

      if (!truejoin) {
        characterl.items[temp].id = item.itemid;
        characterl.items[temp].ident = TRUE;
        characterl.items[temp].charge = item.charge;
      }

      if (theshop.num[tempshop]-- == 1) {
        theshop.id[tempshop] = 0;
        empty = TRUE;
      }
    } else
      characterl.items[temp] = characterr.items[GetControlValue(shopitemsvert) + gshopitem - 1];

    if (!truejoin) {
      if (characterl.numitems > 8) {
        SetControlMaximum(charitemsvert, characterl.numitems - 9);
        theCode = kControlDownButtonPart;
        skip = TRUE;
        if (valu < max)
          skip = FALSE;
        ScrollProc(charitemsvert, theCode);
      } else {
        icon.top = 16 + 40 * temp;
        lg = characterl.items[temp].ident;
        tempitem = item;
        ploticon(tempid, FALSE);
        item = tempitem;
        showcharge2(characterl.items[temp].charge, lg);
      }
    }
    skip = FALSE;
  }

  if (empty) {
    icon.top = 40 * gshopitem - 24;
    icon.left = 390 + (leftshift / 2);
    icon.bottom = icon.top + 32;
    icon.right = icon.left + 250;
    EraseRect(&icon);
    theControl = shopitemsvert;
    ploticon2(2000);
  }

  if ((point.h < (250 + (leftshift / 2))) && (!ischar) && (cr != -1)) {
    characterl.load += (item.wieght + item.xcharge * item.charge); /**** cl from cr **************/
    characterr.load -= (item.wieght + item.xcharge * item.charge);
    movecalc(-1);
    movecalc(-2);
    for (t = GetControlValue(shopitemsvert) + gshopitem - 1; t < characterr.numitems; t++)
      characterr.items[t] = characterr.items[t + 1];

    theControl = shopitemsvert;
    max = GetControlMaximum(shopitemsvert);
    value = GetControlValue(shopitemsvert);
    if (max)
      SetControlMaximum(shopitemsvert, max - 1);
    characterr.numitems--;
    updateshopwings(-2, -2);
    skiptest = TRUE;

    if (characterr.numitems - value > 9)
      displaytag = 2;
    Display(0);
    displaytag = FALSE;
    skiptest = FALSE;
  }

  if ((point.h > (320 + (leftshift / 2))) && (ischar)) {
    if (cr == -1) {
      moneypool[0] += item.cost;
      tempid = characterl.items[GetControlValue(charitemsvert) + gshopitem - 1].id;
      characterl.load -= (item.wieght + item.charge * item.xcharge);
      getselection(tempid);
      for (t = tempselection; t < 200 + tempselection; t++) {
        if (theshop.id[t] == tempid) {
          if (tempselection == shopselection) {
            if (!theshop.num[t])
              doupdateshop = TRUE;
          }
          theshop.num[t]++;
          goto bustout;
        }
      }

      for (t = tempselection; t < 200 + tempselection; t++) {
        if (!theshop.num[t]) {
          theshop.num[t]++;
          doupdateshop = TRUE;
          theshop.id[t] = tempid;
          goto bustout;
        }
      }
    }

  bustout:

    ShowCursor();

    for (t = GetControlValue(charitemsvert) + gshopitem - 1; t < characterl.numitems; t++)
      characterl.items[t] = characterl.items[t + 1];

    movecalc(-1);
    theControl = charitemsvert;
    max = GetControlMaximum(charitemsvert);
    value = GetControlValue(charitemsvert);
    if (max)
      SetControlMaximum(charitemsvert, max - 1);
    characterl.numitems--;
    updateshopwings(-2, -2);
    if (doupdateshop)
      skip = TRUE;
    if (characterl.numitems - value > 9)
      displaytag = 2;
    Display(0);
    displaytag = FALSE;
  }

  updateshopwings(-2, -2);

  if (doupdateshop) {
    theControl = shopitemsvert;
    shop = TRUE;
    skip = FALSE;
    Display(0);
  }
}
