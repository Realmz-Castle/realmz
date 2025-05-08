#include "prototypes.h"
#include "variables.h"

extern Boolean tagger;

/****************** buttonchoice *************************/
short buttonchoice(short skipin) {
  short temp, hit, oldhit, t, tt; /**** dont use exter ****/
  Boolean didnpc, nosound, skip = 0;
  Boolean solidtile = 0;
  short reply = 0;
  Point test;
  Rect r;

  MyrCheckMemory(1);

  if (!indung) {
    if (theControl == movelook) {

      checkmoneypool();

      if ((fat > 134) && (!cancamp)) {
        warn(54);
        return (0);
      }

      tagold = 20;

      if (incamp) {
        incamp = 0;
        updatecontrols();
        moveparty(0);
        timeclick(5, FALSE);
        timeclick(10, TRUE);
        if (revertgame)
          return (0);
      }

      do {
        GetMouse(&point);
        if (PtInRect(point, &lookrect))
          updatearrow(1);
        else {
          SetCCursor(sword);
          tagold = 20;
        }

        tickcheck();
        if (checkdelay(1)) {
          if (!key) {
            deltax = deltay = 0;
            if (point.v < partyy * 32)
              deltay--;
            if (point.v > partyy * 32 + 32)
              deltay++;
            if (point.h < partyx * 32)
              deltax--;
            if (point.h > partyx * 32 + 32)
              deltax++;
            if ((!deltax) && (!deltay))
              return (0);
          }

          nosound = FALSE;
          if (deltax == -1)
            boatright = -1;
          else if (deltax == 1)
            boatright = 0;

          if (deltax > 0)
            face = 1;
          else if (deltax < 0)
            face = 0;

          hit = field[partyx + lookx + deltax][partyy + looky + deltay];
          oldhit = hit;

          if ((hit < 0) && (hit > -999))
            if (solids[-hit])
              solidtile = TRUE; /******** check for solid status ********/

          if (hit > 0) {
            MyrBitClrShort(&hit, 1); /**** removes note *****/
            MyrBitClrShort(&hit, 2); /**** removes path *****/
          } else
            nosound = TRUE;

          if ((hit > 2999) || (hit < -2999)) /**** undetected secret area ******/
          {
            if (hit > 2999)
              hit -= 3000;
            else
              hit += 3000;
          }

          hit = abs(hit);

          if (hit > 999) /**** some type of door ******/
          {
            temp = hit - 1000;
            if (temp > 999)
              temp -= 1000;
            if (temp > 999)
              temp -= 1000;

            /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
             * NOTE(fuzziqersoftware): This seems to be a bug in the original
             * code. Some tiles have IDs beyond 402, which is the last entry in
             * mapstats, and the original code liekly simply accessed memory
             * out of bounds on this array. Since it was a global with plenty
             * of memory after it, this probably didn't break on older systems.
             * TODO: Should we use 200 here instead of 402? Currently we just
             * limit to the size of the array, since that's closest to the
             * original code's behavior.
             */
            if (temp > 402)
              temp = basetile[lastpix];
            /* *** END CHANGES *** */

            fieldx = partyx + lookx;
            fieldy = partyy + looky;

            moveparty(1);

            if (!nosound)
              sound(mapstats[temp].sound);
            else {
              sound(82);
              nosound = FALSE;
            }

            timeclick(mapstats[temp].time, TRUE);
            if (revertgame)
              return (0);

            if (hit < 2999) /***** normal door or detected secret area ****/
            {

              newland(partyx + lookx, partyy + looky, 0, 0, 0);

              /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
               * NOTE(fuzziqersoftware): This check also accessed mapstats out
               * of bounds, similar to the above comment.
               */
              // if (hit > 999) {
              //   if (mapstats[hit - 1000].needboad == 1)
              //     goto doboat; /**** hit boat ****/
              // }
              // if (hit < 999) {
              //   if (mapstats[hit].needboad == 1)
              //     goto doboat; /**** hit boat ****/
              // }
              if (mapstats[temp].needboad == 1)
                goto doboat; /**** hit boat ****/
              /* *** END CHANGES *** */

              goto next;
            }

          } else if (deltax || deltay) {
            if ((hit > 200) || (oldhit < 0))
              hit = basetile[lastpix];

            if ((mapstats[hit].ispath) && (!mapstats[hit].solid)) /**** hit / mark path ****/
            {
              MyrBitSetShort(&field[partyx + lookx + deltax][partyy + looky + deltay], 2); /**** markes path *****/
            }

            if (inboat) {
              if ((mapstats[hit].solid) && (mapstats[hit].needboad != 2)) /**** hit solid in boat ****/
              {
                sound(-148);
                deltax = deltay = 0;
              }

              if (mapstats[hit].shore) /**** hit shore ****/
              {
                sound(-148);
                deltax = deltay = 0;
                hitland++;
                if (hitland > 2) {
                  hitland = 0;
                  if (question(14)) {
                    inboat = key = FALSE;
                    field[partyx + lookx][partyy + looky] = 147;
                  }
                }
              }
            } else {
              if (mapstats[hit].needboad == 1) /**** hit boat ****/
              {
              doboat:
                if (basescale[lastpix])
                  goto noboat; //*** fantasoft v7.1  NO boats indoors, bypass this code.
                if (question(15)) {
                  inboat = TRUE;
                  moveparty(1);
                  field[partyx + lookx][partyy + looky] = 60;
                  centerpict();
                }
              }
            }

          noboat: //*** fantasoft v7.1  NO boats indoors, bypass this code.

            if (deltax || deltay) {
              hitland = 0;
            }

            if ((!mapstats[hit].solid) && (!inboat) && (mapstats[hit].needboad != 2) && (!solidtile))
              moveparty(1);
            else if ((inboat) && (mapstats[hit].needboad == 2))
              moveparty(1);

            solidtile = FALSE;

            if (!nosound) {
              sound(mapstats[hit].sound);
              timeclick(mapstats[hit].time, TRUE);
            } else {
              sound(82);
              timeclick(3, TRUE);
            }
            if (revertgame)
              return (0);
          }
        next:

          key = 0;
          if (checkforsecret(FALSE))
            newland(partyx + lookx, partyy + looky, 0, TRUE, 0);
        }
      } while (((StillDown()) || (key)) && ((fat < 135) || (cancamp)));
    }
  }

  SetPort(GetWindowPort(screen));

  if (theControl == search) {
    sound(141);
    if (partycondition[5]) {
      partycondition[5] = 0;
      SetPort(GetWindowPort(screen));
      GetControlBounds(search, &r);
      ploticon3(128, r);
    } else
      partycondition[5] = -1;
  }

  if (theControl == torch) {
    if (checkforitem(805, TRUE, -1)) {
      loaditem(805);
      loadspell2(item.sp2);
      powerlevel = abs(item.sp1);
      sound(600 + spellinfo.sound2);
      resolvespell();
    }
  }

  if (theControl == barbut) /*********** heal ******************/
  {
    GetControlBounds(barbut, &r);
    ploticon3(129, r);
    if (fat > 134) {
      warn(54);
      ploticon3(130, r);
      return (0);
    }
    sound(10105);
    do {
      delay(2);
      timeclick(1 + 4 * indung, TRUE);
      if (revertgame)
        return (0);
      tickcheck();
      for (t = 0; t <= charnum; t++) {
        if (((c[t].spellcastertype == 1) || (c[t].spellcastertype == 2)) && cancast(t, 0)) {
          for (tt = 0; tt <= charnum; tt++) {
            if ((c[tt].stamina < c[tt].staminamax) && (c[tt].stamina > -10) && (!c[tt].condition[26]) && (c[t].spellpoints > 9)) {
              if ((c[t].canheal) && cancast(t, 0)) {
                c[t].spellpoints -= 10;
                updatecharshort(t, FALSE);
                heal(tt, Rand(8), FALSE);
              }
            }
          }

          for (tt = 0; tt < heldover; tt++) {
            if ((holdover[tt].stamina < holdover[tt].staminamax) && (holdover[tt].stamina > -10) && (c[t].spellpoints > 9)) {
              if ((c[t].canheal) && cancast(t, 0)) {
                c[t].spellpoints -= 10;
                heal(tt, Rand(8), FALSE);
                holdover[tt].stamina += Rand(8);
                if (holdover[tt].stamina > holdover[tt].staminamax)
                  holdover[tt].stamina = holdover[tt].staminamax;
              }
            }
          }
        }
      }
    } while ((Button()) && (fat < 135));
    ploticon3(130, r);
  }

  if (theControl == showitembut) {
    GetControlBounds(showitembut, &buttonrect);
    // buttonrect = *&(**(showitembut)).contrlRect;
    downbutton(TRUE);
    showcondition(charselectnew, charselectnew, 1, 0, charselectnew);
    upbutton(TRUE);
  }

  if (theControl == showconditionbut) {
    GetControlBounds(showconditionbut, &buttonrect);
    // buttonrect = *&(**(showconditionbut)).contrlRect;
    downbutton(TRUE);
    showcondition(charselectnew, charselectnew, 0, 0, charselectnew);
    upbutton(TRUE);
  }

  if (theControl == rest) {
    GetControlBounds(rest, &r);
    ploticon3(129, r);
    sound(6001);
    do {
      tickcheck();
      delay(1);
      updatefat(FALSE, -2, FALSE);
      timeclick(3, FALSE);
      timeclick(2, TRUE);
      if (revertgame)
        return (0);
    } while (StillDown());
    ploticon3(130, r);
  }

  if (theControl == campbut) {
    if (!indung)
      EraseRect(&textrect);
    GetControlBounds(campbut, &r);
    ploticon3(129, r);
    if (!incamp) {
      if (!cancamp) {
        sound(10001);
        music(9); /**** Camp music ***/
        incamp = TRUE;
        deltax = deltay = inshop = intemple = templeavail = shopavail = currentshop = 0;
        moveparty(0);
        ploticon3(130, r);
        timeclick(3, FALSE);
        timeclick(2, TRUE);
        if (revertgame)
          return (0);
        updatecontrols();
      } else {
        ploticon3(130, r);
        flashmessage((StringPtr) "You may not camp at the present time.", 50, 50, 0, 6001);
      }
    } else {
      sound(141);
      incamp = FALSE;
      moveparty(0);
      ploticon3(130, r);
      timeclick(2, FALSE);
      if (revertgame)
        return (0);
      updatecontrols();
    }
  }

  if (theControl == overviewbut) /*************** make scroll ****************/
  {
    GetControlBounds(overviewbut, &r);
    ploticon3(129, r);
    if ((incamp) && (c[charselectnew].stamina > 0)) {
      if (!c[charselectnew].spellcastertype)
        warn(15);
      else {
        sound(6001);
        if (checkfortype(charselectnew, 13, TRUE)) {
          makescroll();
          goto moveon;
        }
        warn(16); /****** no scroll in possetion ****/
      }
    } else if (!incamp) /****** random choice & search ******/
    {
      if (fat > 134) {
        warn(54);
        ploticon3(130, r);
        return (0);
      }

      sound(6001);
      do {
        delay(1);
        if (checkforsecret(TRUE))
          newland(partyx + lookx, partyy + looky, 0, TRUE, 0);
        if (revertgame)
          return (0);
        timeclick(1, TRUE);
        if (revertgame)
          return (0);
        tickcheck();
      } while ((Button()) && (fat < 135));
    }
    SetPort(GetWindowPort(screen));
    ploticon3(130, r);
  }

moveon:

  if (theControl == shopbut) {
    sound(141);
    GetControlBounds(shopbut, &r);
    if (shopavail) {
      if (globalmacro[4]) /*** activate global shop macro if any ****/
      {
        reply = newland(0L, 0L, 1, globalmacro[4], 0);
      }
      ploticon3(129, r);
      characterl = c[charselectnew];
      cl = charselectnew;
      cr = -1;
      numitems = 200;
      if (!skipin)
        in();
      music(8); /**** shop music ***/
      reply = seeshop(0);
      return (reply);
    } else if (templeavail) {

      if (globalmacro[5]) /*** activate global temple macro if any ****/
      {
        reply = newland(0L, 0L, 1, globalmacro[5], 0);
      }
      intemple = TRUE;
      ploticon3(129, r);
      in();
      temple();
      return (0);
    } else /**** check for seamless encounter ****/
    {
      ploticon3(129, r);
      if (!indung) {
        test.h = lookx + partyx;
        test.v = looky + partyy;
      } else {
        test.h = floorx;
        test.v = floory;
      }

      temp = 0;

      for (t = 19; t > -1; t--) {
        if ((PtInRect(test, &randlevel.randrect[t])) && (randlevel.percent[t] < 0)) {
          for (tt = 0; tt < 3; tt++) {
            if (Rand(100) <= abs(randlevel.randdoorpercent[t][tt])) {
              temp = randlevel.randdoor[t][tt];
              if (randlevel.randdoorpercent[t][tt] > 0)
                randlevel.randdoorpercent[t][tt] = 0;
            }
          }
        }
      }

      if (!indung)
        saveland(landlevel);
      else
        saveland(dunglevel);
      ploticon3(130, r);
      deltax = deltay = 0;
      seemless = needdungeonupdate = TRUE;
      reply = newland(0L, 0L, 1, temp, 0);
      seemless = FALSE;
      if (reply == -1)
        goto godooritem; /******** activate door item *****/
      return (0);
    }
  }

  if ((theControl == tradebut) && (charnum)) {
    sound(141);
    GetControlBounds(tradebut, &r);
    ploticon3(129, r);
    if (!skipin)
      in();
    characterl = c[charselectnew];
    cl = charselectnew;
    cr = cl + 1;
    if (cr > charnum)
      cr = 0;
    if (cr != -1)
      characterr = c[cr];
    numitems = characterr.numitems;
    reply = seeshop(0);
    return (reply);
  }

  if ((theControl == charmainbut) && (abs(point.v / 50) <= charnum)) {
    charselectnew = point.v / 50;
    itemRect.top = charselectnew * 50;
    itemRect.bottom = itemRect.top + 50;
    itemRect.left = 330;
    itemRect.right = itemRect.left + 50;
    ploticon3(129, itemRect);
    sound(141);

    if (charselectnew == charselectold) {
      ploticon3(130, itemRect);
      in();
      viewcharacter(charselectnew, 0);
      if ((indung) && (viewtype == 1))
        UpdateWindow(FALSE);
      updatemain(FALSE, -1);
    } else {
      updatecontrols();
      charselectnew = point.v / 50;
      itemRect.top = charselectnew * 50;
      itemRect.bottom = itemRect.top + 50;
      itemRect.left = 330;
      itemRect.right = itemRect.left + 50;
      ploticon3(130, itemRect);
    }
  }

  if (theControl == itemsbut) {
    sound(141);
    if (!skipin)
      in();
    GetControlBounds(itemsbut, &r);
    if (skipin != -1)
      ploticon3(129, r);

    reply = items();

    if (reply < -99) /**** activate door items ****/
    {
      updatemain(FALSE, -1);

    godooritem:

      newland(0L, 0L, 1, abs(itemused) - 100, 0);
      goto next;
    }

    if (reply < 0)
      return (reply); /**** goto shop ***/

    if (!indung)
      updatemain(FALSE, -1);
    else {
      updatemain(TRUE, -1);
      threed(-1L, 0, 0, 0);
    }

    return (0);
  }

  if (theControl == swapbut) {
    sound(141);
    in();
    GetControlBounds(swapbut, &r);
    ploticon3(129, r);
    swap();
    characterl = c[charselectnew];
    updatemain(FALSE, -1);
  }

  if (theControl == viewspellsbut) /*************** use scroll **************/
  {
    GetControlBounds(viewspellsbut, &r);
    ploticon3(129, r);
    if (getscroll()) {
      skip = TRUE;
      ploticon3(130, r);
      goto castincamp;
    }
    ploticon3(130, r);
    return (0);
  }

  if (theControl == castspellsbut) {
    GetControlBounds(castspellsbut, &r);
    if (fastspell) {
      ploticon3(129, r);
      goto castincamp;
    }

    sound(141);
    if (!cancast(charselectnew, 0))
      return (0);
    scribing = 1;
    if (!inspell) {
      ploticon3(129, r);
      castspell();
      centerpict();
    }

  castincamp:

    if (castnum > -1) {
      if (!skip)
        updatemain(TRUE, -1);
      updatecharshort(charselectnew, FALSE);
      cleartarget();
      temp = 0;
      if (spellinfo.targettype > 2)
        temp = charnum;
      if (!spellinfo.targettype)
        temp = powerlevel - 1;

      didnpc = FALSE;
      if (spellinfo.targettype == 5)
        track[charselectnew] = TRUE;
      else if (temp < charnum) {
        textbox(3, 47, 0, 0, textrect);
        if (!getchoice(temp, 0, TRUE))
          return (0);
      } else {
        for (t = 0; t <= charnum; t++)
          track[t] = TRUE;
        if ((temp > charnum) || (spellinfo.targettype == 3) || (spellinfo.targettype == 7) || (spellinfo.targettype == 9)) {

          for (t = 0; t < heldover; t++) {
            monster[t] = holdover[t];
            track[t + 10] = TRUE;
            didnpc = TRUE;
          }
        }
      }
      SetPort(GetWindowPort(screen));
      EraseRect(&textrect);
      sound(-(spellinfo.sound1 + 600));
      resolvespell();

      if (didnpc)
        for (t = 0; t < heldover; t++)
          holdover[t] = monster[t];

      if (spellinfo.targettype == 11)
        sound(spellinfo.sound2 + 600);
      delay(20);
      EraseRect(&textrect);
      inspell = fastspell = memoryspell = FALSE;
      updatecontrols();
    } else
      updatemain(TRUE, -1);
  }

  checkfordoauto();

  return (0);
}
