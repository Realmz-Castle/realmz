#include "prototypes.h"
#include "variables.h"

/***************************** choice *** shop buttons ********************/
short choice(short key) {
  short t;
  Boolean update = FALSE;
  char reply = 0;
  Rect checkrect;
  Rect r;
  int index;

  didattack = FALSE;
  SetPort(GetWindowPort(gshop));

  if (!key) {
    point = gTheEvent.where;
    GlobalToLocal(&(point));
    thePart = FindControl(point, gshop, &theControl);
  }

  c[cl] = characterl;
  if (cr != -1)
    c[cr] = characterr;

  key = gTheEvent.message & charCodeMask;
  GetControlBounds(theControl, &r);

  if ((theControl != shopitemsvert) && (theControl != charitemsvert)) {
    temp = shopselection;
    if ((theControl == goitems) || (key == 'i' /* 'i' */)) {
      ploticon3(132, r);
      if (shopavail)
        saveshop(1, (Ptr) ""); /***** update current game shop ***************/

      if ((moneypool[0]) || (moneypool[1]) || (moneypool[2])) {
        c[cl] = characterl;
        if (cr != -1)
          c[cr] = characterr;
        needupdate = TRUE;
        if (question(-6))
          swap();
      }
      if (bankavailable) {
        bank[0] += moneypool[0];
        bank[1] += moneypool[1];
        bank[2] += moneypool[2];
        moneypool[0] = moneypool[1] = moneypool[2] = 0;
      } else
        moneypool[0] = moneypool[1] = moneypool[2] = 0;
      DisposeWindow(gshop);
      gshop = NIL;
      inshop = FALSE;
      charselectnew = cl;
      return (-1);
    }

    if ((theControl == moneybut) || (key == 'm' /* 'm' */)) {
      sound(141);
      ploticon3(132, r);
      c[cl] = characterl;
      if (cr != -1)
        c[cr] = characterr;
      swap();
      characterl = c[cl];
      if (cr != -1)
        characterr = c[cr];
      Showitems(1);
    }

    if (shopavail) {
      if ((theControl == goshopinshop) && (cr != -1))
        theControl = weapons;

      if (theControl == weapons) {
        sound(141);
        didattack = 1;
        buttonrect = r;
        ploticon3(129, buttonrect);
        shopselection = 0;
        update = TRUE;
      }

      if (theControl == helms) {
        sound(141);
        didattack = 4;
        buttonrect = r;
        ploticon3(129, buttonrect);
        shopselection = 400;
        update = TRUE;
      }

      if (theControl == armor) {
        sound(141);
        didattack = 1;
        buttonrect = r;
        ploticon3(129, buttonrect);
        shopselection = 200;
        update = TRUE;
      }

      if (theControl == magic) {
        sound(141);
        didattack = 1;
        buttonrect = r;
        ploticon3(129, buttonrect);
        shopselection = 600;
        update = TRUE;
      }

      if (theControl == supplies) {
        sound(141);
        didattack = 1;
        buttonrect = r;
        ploticon3(129, buttonrect);
        shopselection = 800;
        update = TRUE;
      }
    }
    if (shop)
      SetControlMaximum(shopitemsvert, 191);

    if ((theControl == poolshop) || (key == 'p' /* 'p'*/)) {
      ploticon3(133, r);
      pool();
      updateshop();
      ploticon3(134, r);
    }

    if ((theControl == shareshop) || (key == 's' /* 's'*/)) {
      ploticon3(133, r);
      share();
      updateshop();
      ploticon3(134, r);
    }

    if (theControl == rightarrow) {
      GetMouse(&point);

      checkrect.top = 388;
      checkrect.left = 314 + (leftshift / 2);
      checkrect.right = checkrect.left + 22;
      checkrect.bottom = checkrect.top + 22;

      for (t = 0; t <= charnum; t++) {
        if (t == 3)
          OffsetRect(&checkrect, 24, -71);

        if (PtInRect(point, &checkrect)) {
          if ((t != cl) && (t != cr) && (t <= charnum)) {
            if (cr != -1)
              c[cr] = characterr;
            movecalc(cr);
            compactitems(cr);
            cr = t;
            change(1, 0);
            SetPort(GetWindowPort(gshop));
          } else
            sound(143);
        }

        OffsetRect(&checkrect, 0, 24);
      }
    }

    if (theControl == leftarrow) {
      GetMouse(&point);

      checkrect.top = 388;
      checkrect.left = 262 + (leftshift / 2);
      checkrect.right = checkrect.left + 22;
      checkrect.bottom = checkrect.top + 22;

      for (t = 0; t <= charnum; t++) {
        if (t == 3)
          OffsetRect(&checkrect, 24, -71);

        if (PtInRect(point, &checkrect)) {
          if ((t != cl) && (t != cr) && (t <= charnum)) {
            c[cl] = characterl;
            movecalc(cl);
            compactitems(cl);
            cl = t;
            change(1, 0);
            SetPort(GetWindowPort(gshop));
          } else
            sound(143);
        }

        OffsetRect(&checkrect, 0, 24);
      }
    }

    if (theControl == leftcharacter) {
      ploticon3(129, r);
      sound(141);
      charselectnew = cl;
      movecalc(cl);
      viewcharacter(cl, 0);
      Showitems(1);
    }

    if ((theControl == rightcharacter) && (shop == FALSE)) {
      ploticon3(129, r);
      sound(141);
      charselectnew = cr;
      movecalc(cr);
      viewcharacter(cr, 0);
      charselectnew = cl;
      Showitems(1);
    }

    if ((update) && (cr > -1) && (shopavail)) {
      itemRect.top = 387;
      itemRect.bottom = itemRect.top + 73;
      itemRect.left = 0 + (leftshift / 2);
      itemRect.right = itemRect.left + 640;
      pict(148, itemRect);

      if (screensize) {
        itemRect.left = 0;
        itemRect.right = 80;
        pict(217, itemRect);
        itemRect.left += 720;
        itemRect.right += 720;
        pict(217, itemRect);
      }

      itemRect.left = 17;
      itemRect.top = 402;
      itemRect.bottom = itemRect.top + 32;

      TextSize(16);
      ForeColor(yellowColor);

      eraseshopname(0); /*** left ***/
      TextMode(1);

      MyrDrawCString(c[cl].name);
      plotportrait(characterl.pictid, itemRect, characterl.caste, cl);

      theControl = rightarrow;
      SetControlValue(theControl, 0);
      cr = -1;
      change(1, 0);
    }

    if (((temp != shopselection) && (cr == -1)) || (update)) {
      theControl = shopitemsvert;
      SetControlValue(theControl, 0);
      Display(0);
    }
  }

  if ((theControl == departshop) || (key == 0x0d /* 'return'*/) || (key == 3 /* 'enter'*/)) {
    sound(141);
    ploticon3(132, r);
    if (shopavail)
      saveshop(1, (Ptr) ""); /***** update current game shop ***************/
    if ((moneypool[0]) || (moneypool[1]) || (moneypool[2])) {
      c[cl] = characterl;
      if (cr != -1)
        c[cr] = characterr;
      needupdate = TRUE;
      if (question(-6))
        swap();
    }
    if (bankavailable) {
      bank[0] += moneypool[0];
      bank[1] += moneypool[1];
      bank[2] += moneypool[2];
      moneypool[0] = moneypool[1] = moneypool[2] = 0;
    } else
      moneypool[0] = moneypool[1] = moneypool[2] = 0;

    DisposeWindow(gshop);
    gshop = NIL;
    inshop = FALSE;
    if (indung) {
      updatemain(TRUE, -1);
      threed(-1L, 0, 0, 0);
    }
    return (-3);
  }

  if (theControl == shopitemsvert) {
    if (thePart == kControlIndicatorPart) {
      thePart = TrackControl(shopitemsvert, point, 0L);
      skip = TRUE;
      Display(0);
      skip = FALSE;
      return (0);
    } else {
      thePart = TrackControl(shopitemsvert, point, 0L);
      ScrollProc(shopitemsvert, thePart);
      return (0);
    }
  } else if (theControl == charitemsvert) {
    if (thePart == kControlIndicatorPart) {
      thePart = TrackControl(charitemsvert, point, 0L);
      skip = TRUE;
      Display(0);
      skip = FALSE;
      return (0);
    } else {
      thePart = TrackControl(charitemsvert, point, 0L);
      ScrollProc(shopitemsvert, thePart);
    }
  } else {
    if (((point.h > (389 + (leftshift / 2))) && (point.h < (423 + (leftshift / 2)))) || ((point.h > 9) && (point.h < 43))) {
      gshopitem = 0;
      theControl = shopitemsvert;
      curControlValue = GetControlValue(shopitemsvert);
      ischar = FALSE;
      if (point.h < 320) {
        ischar = TRUE;
        theControl = charitemsvert;
        curControlValue = GetControlValue(charitemsvert);
      }

      if ((point.v > 15) && (point.v < 48))
        gshopitem = 1;
      if ((point.v > 55) && (point.v < 88))
        gshopitem = 2;
      if ((point.v > 95) && (point.v < 128))
        gshopitem = 3;
      if ((point.v > 135) && (point.v < 168))
        gshopitem = 4;
      if ((point.v > 175) && (point.v < 208))
        gshopitem = 5;
      if ((point.v > 215) && (point.v < 248))
        gshopitem = 6;
      if ((point.v > 255) && (point.v < 288))
        gshopitem = 7;
      if ((point.v > 295) && (point.v < 328))
        gshopitem = 8;
      if ((point.v > 335) && (point.v < 368))
        gshopitem = 9;

      index = curControlValue + gshopitem;
      if ((gshopitem > characterl.numitems) && (ischar))
        gshopitem = 0;
      if ((gshopitem > characterr.numitems) && (!ischar) && (shop == FALSE))
        gshopitem = 0;
      if ((!theshop.num[shopselection + gshopitem + curControlValue - 1]) && (!ischar) && (cr == -1))
        gshopitem = 0;

      if (screensize) {
        if (point.h < 300)
          quickinfo(cl, index - 1, c[cl].items[index - 1].id, 1);
        else if (cr > -1)
          quickinfo(cr, index - 1, c[cr].items[index - 1].id, 1);
        else
          quickinfo(-1, -1, theshop.id[shopselection + gshopitem + curControlValue - 1], 1);
      }

      if ((gshopitem) && (!ischar)) {
        if (gTheEvent.modifiers & cmdKey) {
          if (cr != -1) {
            if (c[cr].items[index - 1].equip)
              removeitem(cr, index - 1, TRUE, FALSE);
            else
              wear(cr, index - 1, TRUE);
            if (cr != -1)
              characterr = c[cr];
            theControl = rightarrow;
            change(FALSE, TRUE);
            select[cr] = TRUE;
          }
        } else if ((cr != -1) && (!c[cr].items[index - 1].equip)) {
          moveicon();
          ischar = FALSE;
        } else if ((cr != -1) && (c[cr].items[index - 1].equip)) {
          if (gTheEvent.modifiers & optionKey)
            moveicon();
          else if (!BitAnd(gTheEvent.modifiers, controlKey)) {
            if (!screensize)
              warn(60); //*** give slight delay before warning to give chance for just update item info check
            else {
              oldtime = TickCount();
              while (StillDown()) {
                if (TickCount() > oldtime + GetDblTime()) {
                  warn(60);
                  delay(20);
                }
              }
            }
          }
        } else
          moveicon();
      }

      if ((gshopitem) && (ischar)) {
        if (gTheEvent.modifiers & cmdKey) {
          if (c[cl].items[index - 1].equip) {
            removeitem(cl, index - 1, TRUE, FALSE);
            characterl = c[cl];
            theControl = leftarrow;
            change(FALSE, TRUE);
          } else {
            wear(cl, index - 1, TRUE);
            characterl = c[cl];
            theControl = leftarrow;
            change(FALSE, TRUE);
          }
        } else if (c[cl].items[index - 1].equip) {
          if (gTheEvent.modifiers & optionKey)
            moveicon();
          else if (!BitAnd(gTheEvent.modifiers, controlKey)) {
            if (!screensize)
              warn(60); //*** give slight delay before warning to give chance for just update item info check
            else {
              oldtime = TickCount();
              while (StillDown()) {
                if (TickCount() > oldtime + GetDblTime()) {
                  warn(60);
                  delay(20);
                }
              }
            }
          }
        } else
          moveicon();
      }
    }
  }
  return (NIL);
}
