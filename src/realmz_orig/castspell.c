#include "prototypes.h"
#include "variables.h"

/*************************** castspell *************************/
short castspell(void) {
  Rect light, typerect;
  short type, smallclickdirection, keylevel[3], oldkeylevel[3], tempcaste;
  DialogRef describe = NIL;
  Boolean loop, nopower, try, oldred, def, shhh, skipdefault = 0;

  nopower = tier = oldred = def = try = keylevel[1] = keylevel[0] = oldkeylevel[1] = oldkeylevel[0] = 0;

  keylevel[2] = oldkeylevel[2] = 1;

  for (t = 0; t < maxloop; t++)
    track[t] = 0;

  for (t = 0; t <= numchannel; t++)
    quiet(t);

  compactheap();

  in();

  if (incombat)
    charselectnew = charup;

  smallclickdirection = powerlevel = 1;

selectagain:

  if (!cancast(charselectnew, 1)) {
    charselectnew++;
    if (charselectnew > charnum + 1) {
      charselectnew = 0;
      try++;
      if (try > 2) {
        warn(13);
        if (describe)
          DisposeDialog(describe);
        return (1100);
      }
    }
    goto selectagain;
  }

  inspell = TRUE;

  spellwindow = GetNewDialog(144 + (1000 * screensize), 0L, (WindowPtr)-1L);
  if (!reducesound)
    sound(20002);
  gCurrent = spellwindow;
  SetPortDialogPort(spellwindow);
  BackPixPat(base);
  TextFont(defaultfont);

  if (!screensize)
    MoveWindow(GetDialogWindow(spellwindow), GlobalLeft + 350 + leftshift, GlobalTop, FALSE);
  else
    MoveWindow(GetDialogWindow(spellwindow), GlobalLeft + 469, GlobalTop, FALSE);

  ShowWindow(GetDialogWindow(spellwindow));
  ErasePortRect();
  DrawDialog(spellwindow);
wayback:

  int enable_recomposite = WindowManager_SetEnableRecomposite(0);

  if ((incombat) || (!charnum)) {
    GetDialogItem(spellwindow, 45, &itemType, &itemHandle, &itemRect);
    ploticon3(0, itemRect);
  }

  GetDialogItem(spellwindow, 49, &itemType, &itemHandle, &itemRect);
  plotportrait(c[charselectnew].pictid, itemRect, c[charselectnew].caste, -1);
  ForeColor(yellowColor);

  light.top = -50;
  light.bottom = -20;
  light.left = -20;
  light.right = -10;

  SetCCursor(sword);

  ForeColor(yellowColor);
  CtoPstr((Ptr)c[charselectnew].name);
  MyrPascalDiStr(43, (StringPtr)c[charselectnew].name);
  PtoCstr((StringPtr)c[charselectnew].name);
  DialogNum(44, c[charselectnew].spellpoints);

  SetRect(&typerect, 0, 0, 0, 0);

  BeginUpdate(GetDialogWindow(spellwindow));
  EndUpdate(GetDialogWindow(spellwindow));

  tempcaste = c[charselectnew].spellcastertype - 1;

  if (defaultspell) {
    if (!skipdefault) {
      castlevel = lastspell[charselectnew][0];
      castnum = lastspell[charselectnew][1];
      powerlevel = oldpowerlevel[charselectnew];
    }

    skipdefault = 0;
    keylevel[1] = castnum;
    keylevel[0] = castlevel;
    keylevel[2] = powerlevel;

    if (!powerlevel)
      powerlevel = 1;
    GetDialogItem(spellwindow, 29 + powerlevel, &itemType, &itemHandle, &buttonrect);
    downbutton(FALSE);

    if ((castnum > -1) || (lastcaste != tempcaste)) {
      if (castnum > -1) {
        SetPortDialogPort(spellwindow);
      }

      showspelllist();
      if (castnum > -1) {
        loadspell(tempcaste, castlevel, castnum);
        GetDialogItem(spellwindow, 17, &itemType, &itemHandle, &light);

        light.top += 17 * (castnum) + 1;
        light.left += 2;
        light.right -= 2;
        light.bottom = light.top + 9;

        if (((incombat) && (spellinfo.incombat)) || ((!incombat) && (spellinfo.incamp)) || (scribing == 2) || (encountflag)) {
          DrawPicture(on, &light);
          RGBForeColor(&cyancolor);
          GetDialogItem(spellwindow, castnum + 1, &itemType, &itemHandle, &itemRect);
          GetDialogItemText(itemHandle, myString);
          MyrPascalDiStr(castnum + 1, myString);
          ForeColor(yellowColor);
        } else {
          DrawPicture(off, &light);
          oldred = TRUE;
        }

        GetDialogItem(spellwindow, 23 + castlevel, &itemType, &itemHandle, &buttonrect);
        downbutton(FALSE);

        if (spellinfo.cost < 0)
          nopower = TRUE;

        ForeColor(yellowColor);
        DialogNum(41, abs(spellinfo.cost * scribing * powerlevel));
        DialogNum(42, abs(spellinfo.cost * scribing * powerlevel));
        spellinfoupdate();
      }
    }
  } else {
    castnum = -1;
    castlevel = 0;
    GetDialogItem(spellwindow, 23 + castlevel, &itemType, &itemHandle, &buttonrect);
    downbutton(FALSE);
    showspelllist();
  }

  if ((showdescript) && (castnum > -1)) {
    if (!describe)
      describe = GetNewDialog(169 + (1000 * screensize), 0L, (WindowPtr)-1L);
    SetPortDialogPort(describe);
    TextFont(defaultfont);
    BackPixPat(base);
    ForeColor(yellowColor);
    GetIndString(myString, -(1000 * (tempcaste + 1) + castlevel), castnum + 1);
    GetDialogItem(describe, 2, &itemType, &itemHandle, &itemRect);
    MoveWindow(GetDialogWindow(describe), GlobalLeft + -1, GlobalTop + 321 + downshift, FALSE);
    ShowWindow(GetDialogWindow(describe));
    ErasePortRect();
    DrawDialog(describe);
    SetDialogItemText(itemHandle, myString);

    BeginUpdate(GetDialogWindow(describe));
    EndUpdate(GetDialogWindow(describe));
  }

  SetPortDialogPort(spellwindow);
  BringToFront(GetDialogWindow(spellwindow));

  WindowManager_SetEnableRecomposite(enable_recomposite);

back:
  FlushEvents(everyEvent, 0);
  updatetier(1);

  for (;;) {

    SystemTask();
    t = GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
    DoCorrectBugMADRepeat();
#endif
    MyrCheckMemory(2);

    if (gTheEvent.what == keyDown)
      goto dokey;

    if (IsDialogEvent(&gTheEvent)) {
      if (DialogSelect(&gTheEvent, &dummy, &itemHit)) {
        GetDialogItem(spellwindow, itemHit, &itemType, &itemHandle, &buttonrect);

        tempcaste = c[charselectnew].spellcastertype - 1;

        if (itemHit == 37) {

        castspellnow:

          GetDialogItem(spellwindow, 37, &itemType, &itemHandle, &buttonrect);
          loadspell(tempcaste, castlevel, castnum);

          if (castnum == -1) {
            warn(12);
            GetDialogItem(spellwindow, 37, &itemType, &itemHandle, &buttonrect);
            DrawDialog(spellwindow);
            goto wayback;
          } else {
            ploticon3(129, buttonrect);
            if (abs(spellinfo.cost * powerlevel * scribing) > c[charselectnew].spellpoints) {
              warn(18);
              GetDialogItem(spellwindow, 37, &itemType, &itemHandle, &buttonrect);
              DrawDialog(spellwindow);
              goto wayback;
            }
            goto cast;
          }
        }

        if (itemHit == 38) {
        abortspell:
          ploticon3(129, buttonrect);
          inspell = castlevel = castcaste = 0;
          castnum = -1;
          goto out;
        }

        if (((itemHit == 39) || (itemHit == 40)) && (!incombat) && (charnum - killparty > 0)) {
          loop = def = TRUE;

          int enable_recomposite = WindowManager_SetEnableRecomposite(0);

          GetDialogItem(spellwindow, castlevel + 23, &itemType, &itemHandle, &buttonrect);
          upbutton(FALSE);

          GetDialogItem(spellwindow, 29 + powerlevel, &itemType, &itemHandle, &buttonrect);
          upbutton(FALSE);

          GetDialogItem(spellwindow, itemHit, &itemType, &itemHandle, &itemRect);
          ploticon3(135, itemRect);
          while (loop == TRUE) {
            if (itemHit == 39) {
              charselectnew--;
              if (charselectnew < 0)
                charselectnew = charnum;
              if (cancast(charselectnew, 1))
                loop = FALSE;
            } else {
              charselectnew++;
              if (charselectnew > charnum)
                charselectnew = 0;
              if (cancast(charselectnew, 1))
                loop = FALSE;
            }
          }
          sound(141);

          GetDialogItem(spellwindow, 55, &itemType, &itemHandle, &itemRect);
          pict(154, itemRect);
          GetDialogItem(spellwindow, 49, &itemType, &itemHandle, &itemRect);

          plotportrait(c[charselectnew].pictid, itemRect, c[charselectnew].caste, -1);

          ForeColor(yellowColor);

          CtoPstr((Ptr)c[charselectnew].name);
          MyrPascalDiStr(43, (StringPtr)c[charselectnew].name);
          PtoCstr((StringPtr)c[charselectnew].name);
          DialogNum(44, c[charselectnew].spellpoints);
          castlevel = -1;
          GetDialogItem(spellwindow, itemHit, &itemType, &itemHandle, &itemRect);
          ploticon3(136, itemRect);
          itemHit = 23;

          WindowManager_SetEnableRecomposite(enable_recomposite);
          goto newjump;
        }

        if ((itemHit > 22) && (itemHit < 37)) {
          if (itemHit < 30) {
          bigclick:

            if (tier != 0) {
              updatetier(0);
              tier = 0;
              updatetier(1);
            }

            if (itemHit - 23 == castlevel)
              goto back;
            GetDialogItem(spellwindow, castlevel + 23, &itemType, &itemHandle, &buttonrect);
            upbutton(FALSE);

            sound(10129); /********* big click *************/
          newjump:
            DrawPicture(non, &light);
            MyrCDiStr(41, (StringPtr) "");
            MyrCDiStr(42, (StringPtr) "");
            MyrCDiStr(13, (StringPtr) "");
            MyrCDiStr(14, (StringPtr) "");
            MyrCDiStr(50, (StringPtr) "");
            MyrCDiStr(15, (StringPtr) "");
            MyrCDiStr(18, (StringPtr) "");
            MyrCDiStr(19, (StringPtr) "");
            DrawPicture(non, &typerect);

            if (def) {
              def = FALSE;
              goto wayback;
            }

            castnum = -1;
            keylevel[1] = 0;
            keylevel[0] = castlevel = itemHit - 23;

            GetDialogItem(spellwindow, castlevel + 23, &itemType, &itemHandle, &buttonrect);
            downbutton(FALSE);

            showspelllist();
            itemHit = 1;
            shhh = TRUE;
            goto smallclick;
          } else {
          powertest:
            if (nopower) {
              itemHit = 30;
              goto maxpower2;
            }

            if ((castnum == -1) || (itemHit - 29 == powerlevel))
              goto back;
            if (abs((itemHit - 29) * spellinfo.cost * scribing) <= c[charselectnew].spellpoints) {
            maxpower:
              if (tier != 2) {
                updatetier(0);
                tier = 2;
                updatetier(1);
              }
            maxpower2:

              ForeColor(yellowColor);
              sound(137); /************* power level ************/
              keylevel[2] = powerlevel = itemHit - 29;
              if (castnum > -1)
                DialogNum(42, abs(spellinfo.cost * powerlevel * scribing));
              if (nopower) {
                itemHit = castnum + 1;
                keylevel[2] = itemHit - 29;
                GetDialogItem(spellwindow, 57, &itemType, &itemHandle, &itemRect);
                ploticon3(2019, itemRect);
                GetDialogItem(spellwindow, 30, &itemType, &itemHandle, &buttonrect);
                downbutton(FALSE);
              } else {
                GetDialogItem(spellwindow, 48, &itemType, &itemHandle, &itemRect);
                pict(129, itemRect);
                GetDialogItem(spellwindow, 29 + powerlevel, &itemType, &itemHandle, &buttonrect);
                downbutton(FALSE);
              }

              updatetier(1);
            } else {
            maxout:
              sound(612); /************* not enough power ***********/
              itemHit = (c[charselectnew].spellpoints / abs((spellinfo.cost * scribing))) + 29;
              if (itemHit < 30)
                itemHit = 30;
              goto maxpower2;
            }
          }
          goto updateinfo;
        }

        if (itemHit < 13) {
          /********* small click *************/

          if (tier != 1) {
            updatetier(0);
            tier = 1;
            updatetier(1);
          }

        smallclick:

          keylevel[1] = itemHit - 1;
          GetDialogItem(spellwindow, itemHit, &itemType, &itemHandle, &itemRect);
          GetDialogItemText(itemHandle, myString);
          PtoCstr(myString); // Myriad
          if (strlen(myString)) // Myriad Confuse strlen<->StringWidth
          {
            if (nopower) {
              GetDialogItem(spellwindow, 48, &itemType, &itemHandle, &itemRect);
              pict(129, itemRect);
              GetDialogItem(spellwindow, 30, &itemType, &itemHandle, &buttonrect);
              downbutton(FALSE);
              nopower = FALSE;
            }

            GetDialogItem(spellwindow, itemHit, &itemType, &itemHandle, &buttonrect);
            buttonrect.left -= 2;
            buttonrect.right += 2;
            buttonrect.top--;

            loadspell(tempcaste, castlevel, itemHit - 1);

            if ((showdescript) && (describe)) {
              GetIndString(myString, -((1000 * c[charselectnew].spellcastertype) + castlevel), itemHit);
              GetDialogItem(describe, 2, &itemType, &itemHandle, &itemRect);
              SetDialogItemText(itemHandle, myString);
            }

            if (((incombat) && (spellinfo.incombat)) || ((!incombat) && (spellinfo.incamp)) || (scribing == 2) || (encountflag)) {
              if (!shhh)
                sound(141);

              shhh = FALSE;
              GetDialogItem(spellwindow, castnum + 1, &itemType, &itemHandle, &itemRect);
              GetDialogItemText(itemHandle, myString);

              if (oldred)
                RGBForeColor(&lightgrey);
              else
                ForeColor(yellowColor);
              oldred = FALSE;
              if (castnum + 1 != itemHit)
                MyrPascalDiStr(castnum + 1, myString);

              castnum = itemHit - 1;
              DrawPicture(non, &light);
              GetDialogItem(spellwindow, 17, &itemType, &itemHandle, &light);
              light.top += 17 * (castnum) + 1;
              light.left += 2;
              light.right -= 2;
              light.bottom = light.top + 9;
              downbutton(FALSE);
              ForeColor(yellowColor);
              DrawPicture(on, &light);
              RGBForeColor(&cyancolor);

              GetIndString(myString, (1000 * (tempcaste + 1)) + castlevel, itemHit);
              MyrPascalDiStr(itemHit, myString);
              ForeColor(yellowColor);
            } else {
            advanceit:
              itemHit += smallclickdirection;
              if ((itemHit > 0) && (itemHit < 13))
                goto smallclick;
              else {
                if (smallclickdirection == -1)
                  goto back;
                smallclickdirection *= -1;
                goto smallclick;
              }
            }

            DialogNum(41, abs(spellinfo.cost * scribing));
            DialogNum(42, abs(powerlevel * spellinfo.cost * scribing));

            upbutton(FALSE);
            if (abs(spellinfo.cost * powerlevel * scribing) > c[charselectnew].spellpoints)
              goto maxout;
          } else
            goto advanceit;

        updateinfo:

          type = abs(spellinfo.damagetype - 1);
          DrawPicture(non, &typerect);

          if ((type > -1) && (type < 6)) {

            if ((type > -1) && (type < 2)) {
              typerect.top = 293 + 19 * type;
              typerect.left = 134;
            }

            if ((type > 1) && (type < 4)) {
              typerect.top = 255 + 19 * type;
              typerect.left = 209;
            }

            if ((type > 3) && (type < 6)) {
              typerect.top = 217 + 19 * type;
              typerect.left = 273;
            }

            typerect.right = typerect.left + 9;
            typerect.bottom = typerect.top + 9;

            DrawPicture(on, &typerect);
          }

        bypass:
          spellinfoupdate();

          if (spellinfo.cost < 0) {
            spellinfo.cost = abs(spellinfo.cost);
            nopower = TRUE;
            itemHit = 30;
            goto maxpower2;
          }
        }
      }
    } else {

      switch (gTheEvent.what) {
        case keyDown:
        dokey:
          key = gTheEvent.message;

          oldkeylevel[tier] = keylevel[tier];
          whichset = skipdefault = 0;

          switch (key & charCodeMask) {

            case '1': /**** spell 1 ****/;
              whichset = 1;
              break;

            case '2': /**** spell 2 ****/;
              whichset = 2;
              break;

            case '3': /**** spell 3 ****/;
              whichset = 3;
              break;

            case '4': /**** spell 4 ****/;
              whichset = 4;
              break;

            case '5': /**** spell 5 ****/;
              whichset = 5;
              break;

            case '6': /**** spell 6 ****/;
              whichset = 6;
              break;

            case '7': /**** spell 7 ****/;
              whichset = 7;
              break;

            case '8': /**** spell 8 ****/;
              whichset = 8;
              break;

            case '9': /**** spell 9 ****/;
              whichset = 9;
              break;

            case '0': /**** spell 10 ****/;
              whichset = 10;
              break;

            case 'a': /**** abort key ****/;
            case 'A': /**** abort key ****/;
              itemHit = 38;
              goto abortspell;
              break;

            case 'c': /**** cast key ****/;
            case 'C': /**** cast key ****/;
            case 13: /**** cast key ****/;
              itemHit = 37;
              goto castspellnow;
              break;

            case 0x1e:
              keylevel[tier]--;
              smallclickdirection = -1;
              break;

            case 0x1f:
              keylevel[tier]++;
              smallclickdirection = 1;
              break;

            case 0x1c:
              updatetier(0);
              tier--;
              break;

            case 0x1d:
              updatetier(0);
              tier++;
              break;
          }

          if (whichset) /***** set special spells *****/
          {
            if (gTheEvent.modifiers & cmdKey) /*Myriad : ** set ***/
            {
              sound(144);
              castcaste = tempcaste;
              definespells[charselectnew][whichset - 1][0] = castcaste;
              definespells[charselectnew][whichset - 1][1] = castlevel;
              definespells[charselectnew][whichset - 1][2] = castnum;
              definespells[charselectnew][whichset - 1][3] = powerlevel;
            } else {
              skipdefault = TRUE;
              sound(145); /**** get ****/
              castcaste = definespells[charselectnew][whichset - 1][0];
              castlevel = definespells[charselectnew][whichset - 1][1];
              castnum = definespells[charselectnew][whichset - 1][2];
              powerlevel = definespells[charselectnew][whichset - 1][3];
              DrawDialog(spellwindow);
              goto wayback;
            }
          }

          if ((tier < 0) || (tier > 2)) {
            if (tier < 0)
              tier++;
            else
              tier--;
          }

          sound(130);
          updatetier(1);

          if (((keylevel[tier] < 0) || (keylevel[tier] > 6)) && (tier == 0)) {
            if (keylevel[tier] < 0)
              keylevel[tier] = 0;
            else
              keylevel[tier] = 6;
          }

          if (((keylevel[tier] < 1) || (keylevel[tier] > 7)) && (tier == 2)) {
            if (keylevel[tier] < 1)
              keylevel[tier] = 1;
            else
              keylevel[tier] = 7;
          }

          if ((tier == 1) && ((keylevel[tier] < 0) || (keylevel[tier] > 11))) {
            if (keylevel[tier] < 0)
              keylevel[tier] = 0;
            else
              keylevel[tier] = 11;
          }

          if (oldkeylevel[tier] != keylevel[tier]) {
            switch (tier) {
              case 0: /****** spell level ***/
                itemHit = 23 + keylevel[tier];
                goto bigclick;
                break;

              case 1: /****** spell ***/
                itemHit = 1 + keylevel[tier];
                goto smallclick;
                break;

              case 2: /****** power level ***/
                itemHit = 29 + keylevel[tier];
                goto maxpower;
                break;
            }
          }
          break;
      }
    }
  }

cast:

  if ((spellinfo.incamp < 0) && (!encountflag) && (scribing != 2)) {
    warn(91);
    DrawDialog(spellwindow);
    goto wayback;
  }

  if ((incombat) && (!spellinfo.incombat) && (!encountflag)) {
    warn(23);
    DrawDialog(spellwindow);
    goto wayback;
  } else if (!incombat) {
    if ((!spellinfo.incamp) && (scribing != 2) && (!encountflag)) {
      warn(22);
      DrawDialog(spellwindow);
      goto wayback;
    }
  }

  castcaste = tempcaste;

out:

  DisposeDialog(spellwindow);
  out();

  if (encountflag) {
    for (loop = 0; loop < 8; loop++)
      track[loop] = 0;
    track[charselectnew] = 1;
  }

  if (castnum > -1) {
    c[charselectnew].spellpoints -= abs(spellinfo.cost * powerlevel * scribing);
    lastspell[charselectnew][0] = castlevel;
    lastspell[charselectnew][1] = castnum;
    oldpowerlevel[charselectnew] = powerlevel;
    select[charselectnew] = TRUE;
  }

  spellwindow = NIL;

  if (describe)
    DisposeDialog(describe);
  if ((1101 + (castcaste * 1000) + castlevel * 100 + castnum) > 1100) {
    memoryspell = TRUE;
    if (incombat) {
      c[charup].spellscast++;
      c[charup].spellsofar++;
    }
  } else
    memoryspell = FALSE;
  return (1101 + (castcaste * 1000) + castlevel * 100 + castnum);
}

/******************* updatetier ****************/
void updatetier(short mode) {

  GetDialogItem(spellwindow, 46 + tier, &itemType, &itemHandle, &tierrect);
  tierrect.bottom = tierrect.top + 18;
  tierrect.right = tierrect.left + 18;

  if (tier == 1)
    OffsetRect(&tierrect, 87, 0);

  ploticon3(155 + mode, tierrect);
}
