#include "prototypes.h"
#include "variables.h"

/*************************** combatinfo *******************/
void combatinfo(short tempold) {
  short temp;
  CCrsrHandle shield;

  shield = GetCCursor(132);
  infocombat = TRUE;

  SetCCursor(sword);
  while (Button()) {
    SetPort(GetWindowPort(look));
    GetMouse(&point);

    temp = abs(field[fieldx + (point.h) / 32][fieldy + (point.v) / 32]);

    if ((temp <= maxloop) && (temp != tempold) && (temp > -1)) {
      bodyground(tempold, 0);
      drawbody(tempold, inspell, 0);
      drawbody(temp, 1, 0);

      if (temp < 9) {
        if (c[temp].guarding)
          SetCCursor(shield);
        else
          SetCCursor(sword);
      } else {
        if (monster[temp - 10].guarding)
          SetCCursor(shield);
        else
          SetCCursor(sword);
      }
      combatupdate2(temp);
      tempold = temp;
    }
  }
  bodyground(tempold, 0);
  drawbody(tempold, 0, 0);
  if (inspell)
    combatupdate2(charup); /*** snaps back to char ****/
  infocombat = FALSE;
  FlushEvents(everyEvent, 0);

  DisposeCCursor(shield);
}

/************************* checkdelay ****************/
short checkdelay(short scale) {
  if (TickCount() > oldtime + 2 * oldspeed * scale) {
    oldtime = TickCount();
    return (TRUE);
  }
  return (FALSE);
}

/************************** combatchoice ***************************/
void combatchoice(void) {
  FILE* fp = NULL;
  FILE* pickfile;
  Boolean arrowused = FALSE;
  short start, stop, loopud, looplr, trypick = 0;
  short hit, temptarget, hit0, hit1, hit2, hit3, itemnum;
  float dx, dy;
  short targetx, targety, pass, temptemp, shottry;
  long oldtick;
  short t, tt, cost, spellindex, sumsize, sumsizelow, rotateindex = 0;
  Rect targetrect, goodrect;
  short spellxold, spellyold;
  short temp;
  Boolean bad, skipload, beenattacked;
  CCrsrHandle compassnew;
  Rect r;

  targetnum = numoftar = spellindex = size = cost = rotate = skipload = regenerate = poisoned = behind = 0;
  oldtick = TickCount();
  SetPort(GetWindowPort(screen));

  goodrect.top = goodrect.left = 7;
  goodrect.right = goodrect.bottom = 83;

  checkfordoauto();

  if (theControl == melee) {
  gomelee:

    GetControlBounds(melee, &buttonrect);
    // buttonrect = *&(**(melee)).contrlRect;
    downbutton(TRUE);
    sound(141);
    temp = c[charup].toggle;
    if (c[charup].toggle) {
      if (!c[charup].armor[2])
        c[charup].weaponsound = (30 + c[charup].gender * 8);
      else {
        loaditem(c[charup].armor[2]);
        c[charup].weaponsound = item.sound;
      }
      c[charup].toggle = 0;
    } else {
      if (c[charup].armor[15])
        c[charup].toggle = 1;
    }
    if (temp != c[charup].toggle)
      combatupdate2(charup);
    else
      sound(6000);
  }

  if (theControl == monsterbut) {
    GetControlBounds(monsterbut, &buttonrect);
    // buttonrect = *&(**(monsterbut)).contrlRect;
    in();
    downbutton(TRUE);
    if (lastshown > 9)
      beast(lastshown, 2, 0);
    else
      viewcharacter(lastshown, 0);
    out();
    centerstage(0);
    combatupdate2(lastshown);
    updatecontrols();
    FlushEvents(everyEvent, 0);
  }

  if (theControl == showitems) {
    GetControlBounds(showitems, &buttonrect);
    // buttonrect = *&(**(showitems)).contrlRect;
    downbutton(TRUE);
    SetPort(GetWindowPort(look));
    if (lastshown < 9)
      showcondition(lastshown, lastshown, 1, 0, lastshown);
    else
      showcondition2(lastshown, 1);
    upbutton(TRUE);
    FlushEvents(everyEvent, 0);
  }

  if (theControl == turn)
    goto forceturn;

  if (theControl == condition) {
    GetControlBounds(condition, &buttonrect);
    // buttonrect = *&(**(condition)).contrlRect;
    downbutton(TRUE);
    if (lastshown < 9)
      showcondition(lastshown, lastshown, 0, 0, lastshown);
    else
      showcondition2(lastshown, 0);
    upbutton(TRUE);
    FlushEvents(everyEvent, 0);
  }

  if (theControl == attacks) {
    if (lastshown > 9) {
      GetControlBounds(attacks, &buttonrect);
      // buttonrect = *&(**(attacks)).contrlRect;
      downbutton(TRUE);
      beast(lastshown, 0, 0);
      if (inwindow(charup))
        combatupdate2(lastshown);
    } else
      sound(6000);
    FlushEvents(everyEvent, 0);
  }

  if (theControl == campbut) /****** auto move *******/
  {
    GetControlBounds(campbut, &r);
    ploticon3(129, r);
    sound(141);

    if (!inwindow(charup)) {
      if (inspell)
        goto centercharupspell;
      else
        centerfield(pos[charup][0], pos[charup][1]);
    }
    if (!c[charup].condition[25])
      c[charup].condition[25] = TRUE;
    FlushEvents(everyEvent, 0);
  }

  if ((theControl == viewspellsbut) && (c[charup].armor[13])) {
    GetControlBounds(viewspellsbut, &r);
    ploticon3(129, r);
    if (incombat)
      charselectnew = q[up];
    if (getscroll()) {
      combatupdate2(charup);
      updatecontrols();
      skipload = TRUE; /***** skips center field *****/
      theControl = NIL;
      FlushEvents(everyEvent, 0);
      goto wand;
    } else {
      theControl = NIL;
      combatupdate2(charup);
      updatecontrols();
      FlushEvents(everyEvent, 0);
    }
  }

  if ((c[charup].condition[25]) && (!c[charup].traiter)) {
    theControl = movelook;

    if ((c[charup].condition[25] > 0) && (!c[charup].condition[0])) /***** only auto characters do this ****/
    {
      if (!manualbandage) /**** manual bandage only ***/
      {
        for (t = 0; t <= charnum; t++) /*** bandage bleeders ***/
        {
          if (c[t].bleeding) {
            flashmessage((StringPtr) "Bandaging wounded character.", 30, 100, 120, 10105);
            c[t].bleeding = FALSE;
            if (Rand(100) < 50)
              sound(10121);
            else
              sound(10123);
            SetPort(GetWindowPort(screen));
            updatepictbox(t, TRUE, 0);
            c[charup].attacks = 0;
            return;
          }
        }
      }

    forceturn:

      if (((undead) && (canpriestturn) && (!c[charup].traiter) && (!c[charup].hasturned)) && (c[charup].spec[13] > 0)) {
        buttonrect.left = 522 + leftshift;
        buttonrect.right = buttonrect.left + 33;
        buttonrect.top = 366 + downshift;
        buttonrect.bottom = buttonrect.top + 38;
        downbutton(TRUE);
        c[charup].hasturned = TRUE;

        temptemp = c[charup].spec[13];
        flashmessage((StringPtr) "Attempting to turn any undead or nether spawn.", 30, 100, 120, 659);

        for (t = 0; t < nummon; t++) {
          if (((monster[t].type[1]) || (monster[t].type[2])) && (monster[t].traiter) && (monster[t].cansum != 255) && (monster[t].stamina > 0)) {
            tempid = 100 - temptemp + 5 * monster[t].hd; /**** need to roll above ****/

            if (tempid < 25)
              tempid = 25;

            tempid += monster[t].magres;

            temp = Rand(100) - tempid; /***** diff of roll **********/

            if (temp > 0) {
              if (temp < 30) {
                undead--;
                c[charup].exp += (25 * monster[t].hd);
                monster[t].stamina = 0;
                killbody(t + 10, 0);
                c[charup].destroyed++;
              } else {
                numenemy--;
                undead--;
                sound(630);
                c[charup].exp += (50 * monster[t].hd);
                showresults(t + 10, -13, 0); /********** turned *******/
                monster[t].traiter = c[charup].traiter; /*** make friendly ****/
                spelleffect(t + 10, 0);
                c[charup].turns++;
              }
            }
          }
        }
        c[charup].attacks -= 2;
        updatecontrols();
        return;
      } /***** end turn *****/
    }
  }

  if (c[charup].condition[25]) {
    if (!tomissle(charup)) {
      temp = c[charup].toggle;
      if (c[charup].handtohand < 8)
        toweap(charup);
      if (c[charup].toggle != temp)
        combatupdate2(charup);
      theControl = movelook;
    } else {
      theControl = combatitem;
      if (lastshown != q[up]) {
        centerfield(pos[charup][0], pos[charup][1]);
        combatupdate2(charup);
      }
      c[charup].toggle = 1;
    }
  }

  if ((theControl == combatitem) && (lastshown == q[up])) {
    FlushEvents(everyEvent, 0);

    buttonrect.top = 346 + downshift;
    buttonrect.bottom = buttonrect.top + 18;
    buttonrect.left = 522 + leftshift;
    buttonrect.right = buttonrect.left + 44;
    downbutton(TRUE);

    if (!c[charup].toggle)
      loaditem(c[charup].armor[2]);
    else {
      if (c[charup].armor[10]) {
        loaditem(c[charup].armor[10]);
        if (item.sp2 > 1100)
          loadspell2(item.sp2);
        else
          loaditem(c[charup].armor[15]);
      } else
        loaditem(c[charup].armor[15]);
    }

    if (item.sp2 > 1100) {
      usescroll = TRUE;

      loadspell2(item.sp2);

      if ((item.sp2 > 1100) && (c[charup].toggle) && (item.charge != -1) && (item.type == 10))
        arrowused = TRUE;
      else if (c[charup].toggle)
        arrowused = FALSE;

      if ((spellinfo.spellclass == 9) || (spellinfo.damagetype == 9)) {
        if (checkforenemy(1)) {
          upbutton(TRUE);
          SetPort(GetWindowPort(look));
          ForeColor(blackColor);
          BackColor(whiteColor);
          warn(39);
          return;
        }
      }

      for (t = 0; t < c[charup].numitems; t++) {
        if (c[charup].items[t].id == item.itemid)
          break;
      }

      if (!c[charup].items[t].charge) {
        if (!c[charup].condition[25]) {
          upbutton(TRUE);
          SetPort(GetWindowPort(look));
          ForeColor(blackColor);
          BackColor(whiteColor);
          warn(85);
          inspell = 0;
          return;
        } else {
          inspell = 0;
          theControl = movelook;
          goto forcemove;
        }
      }

      if (!c[charup].toggle)
        itemused = c[charup].armor[2];
      else if (arrowused)
        itemused = c[charup].armor[10];
      else
        itemused = c[charup].armor[15];

      skipload = TRUE;
      theControl = itemsbut;
      powerlevel = abs(item.sp1);
      inspell = TRUE;

      if (powerlevel == 8)
        powerlevel = Rand(7);

      if (!arrowused) {
        for (itemnum = 0; itemnum < c[charup].numitems; itemnum++) {
          if ((c[charup].items[itemnum].id == c[charup].armor[2 + (13 * c[charup].toggle)]) && (c[charup].items[itemnum].equip))
            goto bustout;
        }
      } else {
        for (itemnum = 0; itemnum < c[charup].numitems; itemnum++) {
          if ((c[charup].items[itemnum].id == c[charup].armor[10]) && (c[charup].items[itemnum].equip))
            goto bustout;
        }
      }

    bustout:

      shottry = 0;

      if (c[charup].condition[25]) /****** possesed char using spell/missle weap ***/
      {
        spellrange = abs(spellinfo.range1 + spellinfo.range2 * powerlevel);

        if (!getrange(charup, c[charup].traiter, FALSE))
          goto notargets;

      tryover:

        temp = c[charup].target;
        if ((twixt(temp, 6, 9)) || (temp < 0))
          goto getnew;
        if (temp < 10) {
          if ((c[temp].traiter != c[charup].traiter) && (c[temp].inbattle))
            goto keepmoving;
          else
            goto getnew;
        } else if ((c[charup].traiter != monster[temp - 10].traiter) && (monster[temp - 10].stamina > 0)) {
        keepmoving:
          if ((twixt(temp, 6, 9)) || (temp < 0))
            goto getnew;
          c[charup].target = temp;

          target[0][0] = temp;

          if ((shottry++ > 50) && (cycle == TRUE)) {
          notargets:
            shottry = cycle = inspell = FALSE;
            target[0][1] = targetx = targety = 0;
            goto forcemove;
          } else if ((shottry > 20) && (!cycle)) {
            shottry = FALSE;
            target[0][1] = targetx = targety = 0;
            cycle = TRUE;
            c[charup].target = randrange(0, maxmon);
            goto getnew;
          }

          if (temp < 9) {
            target[0][1] = 1;
            targetx = pos[temp][0];
            targety = pos[temp][1];
          } else {
            target[0][1] = 2;
            targetx = monpos[temp - 10][0];
            targety = monpos[temp - 10][1];
          }

          if (range[temp] > spellrange)
            goto getnew;

          if (c[charup].items[itemnum].charge > 0) {
            c[charup].items[itemnum].charge--;
            c[charup].load -= item.xcharge;
          }

          if ((!c[charup].items[itemnum].charge) && (item.drop))
            dropitem(charup, itemused, itemnum, 1, FALSE);

          inspell = TRUE;
          combatupdate2(charup);
          showspellinfo();
          inspell = FALSE;
          finddxdy(charup, target[0][0]);
          targetnum = 1;
          c[charup].attacks -= 2;
          cast(targetnum, charup);
          if (c[charup].attacks > 1)
            combatupdate(charup);
          if (c[charup].stamina < 1)
            c[charup].attacks = 0;
          return;
        } else {
        getnew:
          if (!cycle)
            c[charup].target = randrange(0, maxloopminus);
          else {
            c[charup].target++;
            if (c[charup].target > maxloopminus)
              c[charup].target = 0;
          }

          goto tryover;
        }
      } else /***** reduce for non possesed using missile/wand ****/
      {
        if (c[charup].items[itemnum].charge > 0) {
          c[charup].items[itemnum].charge--;
          c[charup].load -= item.xcharge;
        }

        if ((!c[charup].items[itemnum].charge) && (item.drop))
          dropitem(charup, itemused, itemnum, 1, FALSE);
      }
    } else {
      sound(6000);
      return;
    }
  }
  if (theControl == itemsbut) {
    if (!skipload) {
      sound(141);
      GetControlBounds(itemsbut, &r);
      ploticon3(129, r);
      in();
      items();
      out();
      combatupdate2(charup);
      centerfield(pos[charup][0], pos[charup][1]);
      updatecontrols();
      SetMenuBar(myMenuBar);
      InsertMenu(gSound, -1);
      InsertMenu(gSpeed, -1);
      DrawMenuBar();
      FlushEvents(everyEvent, 0);
    }

    if ((itemused) && (inspell)) {
      combatupdate(lastshown);

      skipload = inspell = TRUE;
      combatupdate2(lastshown);
      goto wand;
    }
  }

  if (itemused < -99) /**** activate door items ****/
  {
    SetRect(&itemRect, 0, 321, 308, 460);
    SetPort(GetWindowPort(screen));
    pict(203, itemRect);
    in();
    newland(0L, 0L, 1, abs(itemused) - 100, 0);
    return;
  }

  if (theControl == castspellsbut) {
    if (!cancast(charup, 0)) {
      if ((fastspell) && (memoryspell == TRUE))
        c[charup].spellpoints += abs(spellinfo.cost * powerlevel);
      inspell = fastspell = 0;
      sound(143);
      return;
    }
    GetControlBounds(castspellsbut, &r);
    ploticon3(129, r);
    sound(141);
    cleartarget();

    if (fastspell) {
      inspell = TRUE;
      fastspell = FALSE;
      in();
    } else {
      scribing = 1;
      castspell();
      FlushEvents(everyEvent, 0);
    }

  wand:

    spellrange = abs(spellinfo.range1 + powerlevel * spellinfo.range2);

    if (!skipload) {
      updatemain(TRUE, -1);
      updatecharshort(charup, FALSE);
      combatupdate2(charup);
      updatecontrols();
      centerpict();
    }

    if (castnum < 0)
      return;

    if (spellinfo.special == 58) //**** summon monster, lets pick what they get right away
    {
      sumsize = 6 * spellinfo.size;
      sumsizelow = sumsize / 2 - 1;

      if (sumsize > 27)
        sumsize = 200; /*** max size cratures ***/

    onemoretime:

      getfilename("Data MD");

      if (monsterset) {
        MyrNumToString(monsterset, myString);
        strcat((StringPtr)filename, myString);
      }

    tryagain:

      if ((pickfile = MyrFopen(filename, "rb")) != NULL) {

        if (trypick++ > 100) {
          sumsize = 200;
          sumsizelow = 1;
        }

        if (trypick > 400) {
          flashmessage((StringPtr) "The gods deny your request puny mortal!", 30, 50, 0, 26260);
          inspell = FALSE;
          fclose(pickfile);
          return;
        }

        if (itemused) {
          loaditem(itemused);

          if (item.sp5) {
            templong = item.sp5;
            fseek(pickfile, templong * sizeof monpick, SEEK_SET);
            /* !MYRIAD 12/10/99 Because fseek can be greater than the end of the file (and fread can fail)*/
            if (fread(&monpick, sizeof monpick, 1, pickfile) == 1)
              CvtMonsterToPc(&monpick);
            /* !MYRIAD 12/10/99 If not read, keeps the previous value of monpick */
            fclose(pickfile);
            goto good;
          } else
            templong = randrange(0, 200); /********** setmonster for summons ********/
        } else
          templong = randrange(0, 200); /********** setmonster for summons ********/

        if (spellinfo.spellclass)
          templong = spellinfo.spellclass;

        fseek(pickfile, templong * sizeof monpick, SEEK_SET); /***** summon specific monster *********/
        /* !MYRIAD 12/10/99 Because fseek can be greater than the end of the file (and fread can fail)*/
        if (fread(&monpick, sizeof monpick, 1, pickfile) == 1)
          CvtMonsterToPc(&monpick);
        /* !MYRIAD 12/10/99 If not read, keeps the previous value of monpick */

        tempmonsternameid = templong; //*** used so critters over 256 route correctly and can be use in "if NPC present" checks.

        if (!spellinfo.spellclass) /***** check the summons of non specific monster *******/
        {
          if ((monpick.hd < sumsizelow) || (monpick.hd > sumsize) || (!monpick.hd) || (monpick.cansum != 1)) {
            fclose(pickfile);
            goto tryagain;
          }
        }
      } else {
        monsterset = 0;
        warn(102);
        goto onemoretime;
      }
    good:

      spellinfo.size = monpick.size + 1;
    }

    if (spellinfo.targettype < 1)
      numoftar = powerlevel;
    else
      numoftar = 1;

    showspellinfo();

    if ((spellrange == 0) && (spellinfo.targettype < 8))
      goto castonself;

    if (spellinfo.targettype == 5) /**** self ***/
    {
    castonself:
      target[0][0] = pos[charup][0] + fieldx;
      target[0][1] = pos[charup][1] + fieldy;
      targetnum = 1;
      spellx = pos[charup][0];
      spelly = pos[charup][1];
      inspell = TRUE;
      goto launch;
    }

    if (spellinfo.targettype > 8) {
      for (t = 0; t < maxloop; t++)
        track[t] = 0;

      if (spellinfo.targettype == 12) /*** Target Everybody ***/
      {
        for (t = 0; t <= charnum; t++)
          if (c[t].inbattle)
            track[t] = TRUE;
        for (t = 0; t < nummon; t++)
          if (monster[t].stamina > 0)
            track[t + 10] = TRUE;
      }

      if (spellinfo.targettype == 9) /**** all friendly ****/
      {
        for (t = 0; t <= charnum; t++)
          if ((!c[t].traiter) && (c[t].inbattle))
            track[t] = TRUE;
        for (t = 0; t < nummon; t++)
          if ((!monster[t].traiter) && (monster[t].stamina > 0))
            track[t + 10] = TRUE;
      } else /***** all enemy ****/
      {
        for (t = 0; t <= charnum; t++)
          if ((c[t].traiter) && (c[t].inbattle))
            track[t] = TRUE;
        for (t = 0; t < nummon; t++)
          if ((monster[t].traiter) && (monster[t].stamina > 0))
            track[t + 10] = TRUE;
      }
      inspell = TRUE;
      c[charup].movement -= 12;
      if (c[charup].movement < 0)
        c[charup].movement = 0;
      c[charup].attacks -= 2;
      resolvespell();
      cleartarget();
      inspell = FALSE;
      if (c[charup].attacks > 1) {
        combatupdate2(charup);
        updatecontrols();
      }
      if (c[charup].stamina < 1)
        c[charup].attacks = 0;
      return;
    }

    GetMouse(&point);

    oldspellx = spellx = (point.h) / 32;
    oldspelly = spelly = (point.v) / 32;

    showspell.top = spelly * 32 - 98;
    showspell.left = spellx * 32 - 98;
    showspell.right = showspell.left + 224;
    showspell.bottom = showspell.top + 224;

    showrange(0);

    drawspell();
    gDone = FALSE;

    while (gDone == FALSE) {
      SetPort(GetWindowPort(screen));
      GetMouse(&point);

      spellx = point.h / 32;
      spelly = point.v / 32;

      if (spellx > 9 + (screensize * 5))
        spellx = (9 + (screensize * 5));
      if (spelly > 9 + (screensize * 3))
        spelly = (9 + (screensize * 3));

      if (spellx < 0)
        spellx = 0;
      if (spelly < 0)
        spelly = 0;

    getnewtarget:
      updatespell(0);

    backfromrange:

      WaitNextEvent(everyEvent, &gTheEvent, 0L, NIL);
#ifdef PC // Myriad
      DoCorrectBugMADRepeat();
#endif

      if (TickCount() - oldtick > 7) {
        SetPort(GetWindowPort(screen));

        if (spellinfo.canrotate) {
          compassnew = GetCCursor(156 + rotateindex++);
          if (rotateindex > 7)
            rotateindex = 0;
          SetCCursor(compassnew);
          DisposeCCursor(compassnew);
        }

        ForeColor(blackColor);
        if ((spellinfo.spellclass != 9) || (!spellinfo.damagetype)) {
          iconhand = NIL;
          iconhand = GetCIcon(11992 + spellinfo.spelllook1 * 8 + spellindex++);
          if (spellindex > 7)
            spellindex = 0;
        } else {
          dx = spellx - pos[charup][0];
          dy = spelly - pos[charup][1];
          iconhand = NIL;
          iconhand = GetCIcon(11992 + spellinfo.spelllook1 * 8 + direction(dx, dy));
        }
        EraseRect(&spell);
        if (iconhand) {
          PlotCIcon(&spell, iconhand);
          DisposeCIcon(iconhand);
        }
        oldtick = TickCount();
      }

      switch (gTheEvent.what) {
        case osEvt:
          if ((gTheEvent.message >> 24) == suspendResumeMessage) {
            if (hide) {
              hide = FALSE;
              centerfield(pos[charup][0], pos[charup][1]);
              if (inspell)
                drawspell();
              updatemain(FALSE, 0);
              combatupdate2(charup);
              updatecontrols();
              SetRect(&buttonrect, 0, 0, 0, 0);
              FlushEvents(everyEvent, 0);
            } else
              hide = TRUE;
          }
          break;

        case mouseDown:
        testforinfo:
          if (gTheEvent.modifiers & cmdKey) {
            if (inspell)
              centerfield(5 + (2 * screensize), 5 + screensize); //   Fantasoft 7.1
            combatinfo(charup);
            compassnew = GetCCursor(147 + numoftar - targetnum);
            SetCCursor(compassnew);
            DisposeCCursor(compassnew);
            showtargets(0);
            if (inspell)
              drawspell();
            goto pass;
          }

          GetControlBounds(charmainbut, &r);
          if (PtInRect(point, &lookrect)) {
            centerfield((point.h) / 32, (point.v) / 32);
            drawspell();
          } else if (PtInRect(point, &buttons)) {
            key = 0;

            /************ left arrow ** decrease power level ******************/
            buttonrect.top = 395 + downshift;
            buttonrect.bottom = buttonrect.top + 10;
            buttonrect.left = 568 + leftshift;
            buttonrect.right = buttonrect.left + 31;
            if (PtInRect(point, &buttonrect))
              key = '-';

            /************ right arrow ** increase power level ******************/
            buttonrect.left = 603 + leftshift;
            buttonrect.right = buttonrect.left + 31;
            if (PtInRect(point, &buttonrect))
              key = '+';

            /************ a ** abort spell ******************/
            buttonrect.top = 327 + downshift;
            buttonrect.bottom = buttonrect.top + 18;
            buttonrect.left = 417 + leftshift;
            buttonrect.right = buttonrect.left + 49;
            if (PtInRect(point, &buttonrect))
              key = 'a';

            /************ n ** center on next ******************/
            oldspellx = oldspelly = spellx = spelly = 5;
            buttonrect.top = 369 + downshift;
            buttonrect.bottom = buttonrect.top + 18;
            buttonrect.left = 364 + leftshift;
            buttonrect.right = buttonrect.left + 49;
            if (PtInRect(point, &buttonrect))
              key = 'n';

            /************ p ** center on previous ******************/
            oldspellx = oldspelly = spellx = spelly = 5;
            buttonrect.left = 417 + leftshift;
            buttonrect.right = buttonrect.left + 49;
            if (PtInRect(point, &buttonrect))
              key = 'p';

            /********** c ** center on charup *******************/
            buttonrect.left = 471 + leftshift;
            buttonrect.right = buttonrect.left + 49;
            if (PtInRect(point, &buttonrect))
              key = 'c';

            /*** space **** cast spell ***/
            buttonrect.top = 327 + downshift;
            buttonrect.bottom = buttonrect.top + 39;
            if (PtInRect(point, &buttonrect))
              key = ' ';

            if (key) {
              centerfield(spellx, spelly);
              drawspell();
              goto gotkey;
            } else
              sound(143);
          } else if (PtInRect(point, &r)) {
            if (spellinfo.special == 57) {
              temptemp = point.v / 50;
              if ((c[temptemp].stamina > -10) && (c[temptemp].stamina < 1)) {
                track[temptemp] = TRUE;

                itemRect.top = temptemp * 50;
                itemRect.bottom = itemRect.top + 50;
                itemRect.left = 330;
                itemRect.right = itemRect.left + 50;

                ploticon3(129, itemRect);
                sound(-141);
                ploticon3(130, itemRect);

                inspell = 0;
                resolvespell();
                drawspell();
                centerstage(0);

                start = -1;
                stop = 1;

                if (c[temptemp].stamina > 0) /***** bring back ****/
                {
                tryagain2:

                  for (loopud = start; loopud < stop; loopud++) {
                    for (looplr = start; looplr < stop; looplr++) {
                      pos[temptemp][0] = temptemp - 3 * (abs(temptemp / 3)) + abs(temptemp / 3) + looplr + 5;
                      pos[temptemp][1] = (temptemp / 3) + loopud + 5;

                      point.h = pos[temptemp][0] + fieldx;
                      point.v = pos[temptemp][1] + fieldy;

                      if (PtInRect(point, &goodrect)) {
                        temp = field[pos[temptemp][0] + fieldx][pos[temptemp][1] + fieldy];
                        if (temp > 999) {
                          if (!mapstats[(temp - 1000)].solid) /********** place char in NON SOLID square **********/
                          {
                            charunder[temptemp] = temp;
                            field[pos[temptemp][0] + fieldx][pos[temptemp][1] + fieldy] = temptemp;
                            goto good2;
                          }
                        }
                      }
                    }
                  }
                  start--;
                  stop++;
                  goto tryagain2;

                good2:
                  sound(631);
                  if (c[temptemp].condition[6] > -1)
                    c[temptemp].condition[6] = 3;
                  c[temptemp].traiter = c[temptemp].bleeding = inspell = 0;
                  c[temptemp].target = -1;
                  c[temptemp].beenattacked = c[temptemp].inbattle = TRUE;

                  if (c[temptemp].condition[5] > -1)
                    c[temptemp].condition[5] = 2;

                  for (t = maxloopminus; t > 0; t--) {
                    if (q[t] == -1) {
                      q[t] = temptemp;
                      c[temptemp].position = t;
                      updatelight(temptemp, FALSE);
                      drawbody(temptemp, 0, 0);
                      getup(0);
                      centerstage(0);
                      return;
                    }
                  }
                } else {
                  getup(0);
                  return;
                }
              } else
                sound(143);
            } else
              sound(143);
          } else
            sound(143);
          break;

        case keyDown:
          key = gTheEvent.message & charCodeMask;
        gotkey:

          switch (key) {
            case 0x1e:
              centercursor();
              centerfield(5 + (screensize * 2), 4 + (screensize));
              drawspell();
              break;

            case '8':
              centercursor();
              centerfield(5 + (screensize * 2), 4 + (screensize));
              drawspell();
              break;

            case 0x1f:
              centercursor();
              centerfield(5 + (screensize * 2), 6 + (screensize));
              drawspell();
              break;

            case '2':
              centercursor();
              centerfield(5 + (screensize * 2), 6 + (screensize));
              drawspell();
              break;

            case 0x1c:
              centercursor();
              centerfield(4 + (screensize * 2), 5 + (screensize));
              drawspell();
              break;

            case '4':
              centercursor();
              centerfield(4 + (screensize * 2), 5 + (screensize));
              drawspell();
              break;

            case 0x1d:
              centercursor();
              centerfield(6 + (screensize * 2), 5 + (screensize));
              drawspell();
              break;

            case '6':
              centercursor();
              centerfield(6 + (screensize * 2), 5 + (screensize));
              drawspell();
              break;

            case '7':
              centercursor();
              centerfield(4 + (screensize * 2), 4 + (screensize));
              drawspell();
              break;

            case '9':
              centercursor();
              centerfield(6 + (screensize * 2), 4 + (screensize));
              drawspell();
              break;

            case '1':
              centercursor();
              centerfield(4 + (screensize * 2), 6 + (screensize));
              drawspell();
              break;

            case '3':
              centercursor();
              centerfield(6 + (screensize * 2), 6 + (screensize));
              drawspell();
              break;

            case '-': /**** decrease power level ***/
            dpl:
              buttonrect.top = 395 + downshift;
              buttonrect.bottom = buttonrect.top + 10;
              buttonrect.left = 568 + leftshift;
              buttonrect.right = buttonrect.left + 31;
              if ((!usescroll) && (powerlevel > 1) && (numoftar > targetnum)) {
                downbutton(TRUE);
                sound(-130);
                powerlevel--;

                if (!spellinfo.targettype)
                  numoftar--;

                spellrange = abs(spellinfo.range1 + spellinfo.range2 * powerlevel);

                updatespell(TRUE);

                compassnew = GetCCursor(147 + numoftar - targetnum);
                SetCCursor(compassnew);
                DisposeCCursor(compassnew);

                if (memoryspell == TRUE)
                  c[charup].spellpoints += spellinfo.cost;
                updatecharshort(charup, FALSE);
                delay(5);
                combatupdate2(charup);
                showspellinfo();
                updatespell(TRUE);
                drawspell();
              } else
                sound(143);

              break;

            case '+': /**** increase power level ***/
            ipl:
              buttonrect.top = 395 + downshift;
              buttonrect.bottom = buttonrect.top + 10;
              buttonrect.left = 603 + leftshift;
              buttonrect.right = buttonrect.left + 31;
              if ((!usescroll) && (powerlevel < 7) && (memoryspell == TRUE) && (c[charup].spellpoints >= spellinfo.cost)) {
                downbutton(TRUE);
                sound(-130);
                delay(5);
                powerlevel++;

                if (!spellinfo.targettype)
                  numoftar++;

                spellrange = abs(spellinfo.range1 + spellinfo.range2 * powerlevel);

                compassnew = GetCCursor(147 + numoftar - targetnum);
                SetCCursor(compassnew);
                DisposeCCursor(compassnew);

                if (memoryspell == TRUE)
                  c[charup].spellpoints -= spellinfo.cost;
                updatecharshort(charup, FALSE);
                combatupdate2(charup);
                showspellinfo();
                updatespell(TRUE);
                drawspell();
              } else
                sound(143);

              break;

            case '=': /**** increase power level ***/
              goto ipl;
              break;

            case 'a': /************ a ** abort spell ******************/
              buttonrect.top = 327 + downshift;
              buttonrect.bottom = buttonrect.top + 18;
              buttonrect.left = 417 + leftshift;
              buttonrect.right = buttonrect.left + 49;
              downbutton(TRUE);
              inspell = infocombat = FALSE;
              gDone = TRUE;
              SetCCursor(sword);
              c[charup].movement -= 3;
              if ((memoryspell) && (charup < 6))
                c[charup].spellsofar--;
              cleartarget();
              if (c[charup].movement < 0)
                c[charup].movement = 0;
              if ((!usescroll) && (memoryspell == TRUE))
                c[charup].spellpoints += (.66 * (spellinfo.cost * powerlevel));
              usescroll = memoryspell = FALSE;
              combatupdate2(charup);
              updatecharshort(charup, FALSE);
              updatecontrols();
              centerstage(0);
              return;
              break;

            case 'r': /************** r ** reveal friendly **********/
              buttonrect.top = 346 + downshift;
              buttonrect.bottom = buttonrect.top + 18;
              buttonrect.left = 364 + leftshift;
              buttonrect.right = buttonrect.left + 102;
              downbutton(TRUE);
              centerfield(5 + (2 * screensize), 5 + screensize);
              showrange(0);
              drawspell();
              upbutton(TRUE);
              goto backfromrange;
              break;

            case 'n': /************ n ** center on next ******************/
              buttonrect.top = 369 + downshift;
              buttonrect.bottom = buttonrect.top + 18;
              buttonrect.left = 364 + leftshift;
              buttonrect.right = buttonrect.left + 49;
              downbutton(TRUE);
              targetrect = buttonrect;
              centerstage(1);
              drawspell();
              buttonrect = targetrect;
              upbutton(TRUE);
              break;

            case 'p': /************ p ** center on previous ******************/
              buttonrect.top = 369 + downshift;
              buttonrect.bottom = buttonrect.top + 18;
              buttonrect.left = 417 + leftshift;
              buttonrect.right = buttonrect.left + 49;
              downbutton(TRUE);
              targetrect = buttonrect;
              centerstage(-1);
              drawspell();
              buttonrect = targetrect;
              upbutton(TRUE);
              break;

            case 0x0d: /************** return  rotate spell *********************/
              if (spellinfo.canrotate) {
                if (rotate++ > 2)
                  rotate = 0;
                sound(130);
                drawspell();
                temp = 9999 + spellinfo.size + rotate;
                SetPort(GetWindowPort(screen));
                BackColor(whiteColor);
                iconhand = NIL;
                iconhand = GetCIcon(temp);
                if (iconhand) {
                  PlotCIcon(&spellrect, iconhand);
                  MyrCopyIconMask(iconhand, gmaps, &bigspellrect); // Myriad
                  //   CopyBits(&(**(iconhand)).iconBMap,&((GrafPtr) gmaps)->portBits,&(**(iconhand)).iconPMap.bounds,&bigspellrect,0,0L);
                  DisposeCIcon(iconhand);
                }
                RGBBackColor(&greycolor);
                drawspell();
              }
              break;

            case 'c': /********** c ** center on charup *******************/
              buttonrect.top = 369 + downshift;
              buttonrect.bottom = buttonrect.top + 18;
              buttonrect.left = 471 + leftshift;
              buttonrect.right = buttonrect.left + 49;
              downbutton(TRUE);
              infocombat = TRUE;
              targetrect = buttonrect;

            centercharupspell:
              sound(147);
              aimindex = up;
              centerfield(pos[charup][0], pos[charup][1]);
              drawspell();
              combatupdate2(charup);
              buttonrect = targetrect;
              upbutton(TRUE);
              break;

            case ' ': /*** space ***/

            launch:
              buttonrect.top = 327 + downshift;
              buttonrect.bottom = buttonrect.top + 39;
              buttonrect.left = 471 + leftshift;
              buttonrect.right = buttonrect.left + 49;
              downbutton(TRUE);
              if (!targetnum) {
                combaterrors(1); /*** target error ***/
                goto pass;
              }
              if ((spellinfo.targettype > 1) && (!checkrange(charup, spellx, spelly))) {
                combaterrors(2);
                goto pass;
              }
            forcecast:
              gDone = TRUE;
              centerfield(5 + (2 * screensize), 5 + screensize); // Fantasoft 7.1
              c[charup].movement -= 12;
              if (c[charup].movement < 0)
                c[charup].movement = 0;
              c[charup].attacks -= 2;
              cast(targetnum, charup);
              cleartarget();
              if (c[charup].attacks > 1) {
                combatupdate2(charup);
                updatecontrols();
              }
              if (c[charup].stamina < 1)
                c[charup].attacks = 0;
              return;
              break;

            case 3: /***************** (enter = space ) *******************/
              goto launch;
              break;

            case 't': /***************** t ** target *******************/

              buttonrect.top = 327 + downshift;
              buttonrect.bottom = buttonrect.top + 18;
              buttonrect.left = 364 + leftshift;
              buttonrect.right = buttonrect.left + 49;
              downbutton(TRUE);

              if (checkrange(charup, spellx, spelly)) {
                temptarget = abs(field[fieldx + spellx][fieldy + spelly]);
                if ((spellinfo.range1 + spellinfo.range2) > 0) /**** can be blocked ***/
                {
                  if (!cansee(pos[charup][0], pos[charup][1], spellx, spelly)) {
                    combaterrors(3); /***** cant see target ****/
                    goto pass;
                  }
                }

                if (spellinfo.targettype > 1) {
                  target[0][0] = fieldx + spellx;
                  target[0][1] = fieldy + spelly;
                  targetnum++;
                  updatespell(1);
                  upbutton(TRUE);
                  goto launch;
                } else if (((temptarget <= maxloop) && (!spellinfo.size)) || ((temptarget > maxloop) && (spellinfo.size))) {
                  for (t = 0; t < 7; t++) {
                    if (((target[t][0] == temptarget) && (!spellinfo.size)) || ((spellinfo.size) && (target[t][0] == fieldx + spellx) && (target[t][1] == fieldy + spelly))) {
                      sound(144); /********* target off *************/

                      if (!spellinfo.size) {
                        bodyground(temptarget, 0);
                        drawbody(temptarget, 0, 0);
                      } else {
                        targetrect.top = 32 * spelly;
                        targetrect.left = 32 * spellx;
                        targetrect.bottom = targetrect.top + 32;
                        targetrect.right = targetrect.left + 32;
                        fastplot(temptarget, targetrect, 0, 0);
                        if ((monpick.size == 1) || (monpick.size == 3)) {
                          targetrect.top -= 32;
                          targetrect.bottom -= 32;
                          fastplot(field[fieldx + spellx][fieldy + spelly - 1], targetrect, 0, 0);
                          targetrect.top += 32;
                          targetrect.bottom += 32;
                        }
                        if (monpick.size > 1) {
                          targetrect.left -= 32;
                          targetrect.right -= 32;
                          fastplot(field[fieldx + spellx - 1][fieldy + spelly], targetrect, 0, 0);
                        }
                        if (monpick.size == 3) {
                          targetrect.top -= 32;
                          targetrect.bottom -= 32;
                          fastplot(field[fieldx + spellx - 1][fieldy + spelly - 1], targetrect, 0, 0);
                        }
                      }

                      for (tt = t; tt < 6; tt++) {
                        target[tt][0] = target[tt + 1][0];
                        target[tt][1] = target[tt + 1][1];
                      }
                      target[6][0] = -1;
                      target[6][1] = -1;

                      targetnum--;
                      updatespell(1);
                      drawspell();
                      upbutton(TRUE);
                      goto getnewtarget;
                    }
                  }
                  if ((((temptarget < 9) && (!spellinfo.size)) || ((spellinfo.size) && (temptarget > maxloopminus))) && (targetnum < numoftar)) {
                    if (spellinfo.spellclass == 9)
                      missileset(dx, dy);
                    if (!spellinfo.size) {
                      target[targetnum][0] = temptarget;
                      target[targetnum][1] = 1;
                    } else {
                      bad = FALSE;
                      hit0 = abs(field[fieldx + spellx][fieldy + spelly]);
                      hit1 = abs(field[fieldx + spellx - 1][fieldy + spelly - 1]);
                      hit2 = abs(field[fieldx + spellx][fieldy + spelly - 1]);
                      hit3 = abs(field[fieldx + spellx - 1][fieldy + spelly]);
                      size = monpick.size;

                      if (hit0 > 999) /************ not a valid spot to target ****/
                      {
                        if (mapstats[hit0 - 1000].solid)
                          bad = TRUE;
                      } else
                        bad = TRUE;

                      if ((size == 1) || (size == 3)) {
                        if (hit2 < 999)
                          bad = TRUE;
                        else if (mapstats[hit2 - 1000].solid > 1)
                          bad = TRUE;
                      }

                      if (size > 1) {
                        if (hit3 < 999)
                          bad = TRUE;
                        else if (mapstats[hit3 - 1000].solid > 1)
                          bad = TRUE;
                      }

                      if (size == 3) {
                        if (hit1 < 999)
                          bad = TRUE;
                        else if (mapstats[hit1 - 1000].solid > 1)
                          bad = TRUE;
                      }

                      if (!bad) {
                        target[targetnum][0] = fieldx + spellx;
                        target[targetnum][1] = fieldy + spelly;
                      } else
                        goto error;
                    }
                    sound(145); /************** target on ***********/
                    targetnum++;
                    updatespell(1);
                    targetrect.top = 32 * spelly;
                    targetrect.left = 32 * spellx;

                    if (spellinfo.size) /**** for summon monster ***/
                    {
                      if ((monpick.size == 1) || (monpick.size == 3))
                        targetrect.top -= 16;
                      if (monpick.size > 1)
                        targetrect.left -= 16;
                    }
                    targetrect.bottom = targetrect.top + 32;
                    targetrect.right = targetrect.left + 32;
                  } else if (targetnum < numoftar) {
                    if (spellinfo.spellclass == 9)
                      missileset(dx, dy);
                    sound(145); /************** target on ***********/
                    target[targetnum][0] = temptarget;
                    target[targetnum][1] = 2;
                    targetnum++;
                    updatespell(1);
                    size = 0;
                    if (temptarget > 9)
                      size = monster[temptarget - 10].size;
                    targetrect.top = 32 * monpos[temptarget - 10][1];
                    targetrect.left = 32 * monpos[temptarget - 10][0];

                    if ((size == 1) || (size == 3))
                      targetrect.top -= 16;
                    if (size > 1)
                      targetrect.left -= 16;
                    targetrect.bottom = targetrect.top + 32;
                    targetrect.right = targetrect.left + 32;
                  } else {
                    combaterrors(5); /*********** too many targets *********/
                    goto pass;
                  }
                } else {
                error:
                  combaterrors(4); /*********** not valid target *********/
                  goto pass;
                }
                iconhand = NIL;
                iconhand = GetCIcon(12032);
                if (iconhand) {
                  PlotCIcon(&targetrect, iconhand);
                  DisposeCIcon(iconhand);
                }
                upbutton(TRUE);
              } else
                combaterrors(2); /*********** out of range *********/
          }
          break;
      }
    pass:

      if ((spellx != spellxold) || (spelly != spellyold)) {
        showspell.top = spelly * 32 - 98;
        showspell.left = spellx * 32 - 98;
        showspell.right = showspell.left + 224;
        showspell.bottom = showspell.top + 224;
        oldspellx = spellx;
        oldspelly = spelly;

        drawspell();
        spellxold = spellx;
        spellyold = spelly;
      }
    }
  }

  if ((theControl == charmainbut) && (abs(point.v / 50) <= charnum)) {
    SetPort(GetWindowPort(screen));
    charselectnew = point.v / 50;

    buttonrect.top = charselectnew * 50;
    buttonrect.bottom = buttonrect.top + 50;
    buttonrect.left = 330 + leftshift;
    buttonrect.right = buttonrect.left + 50;
    ploticon3(129, buttonrect);

    if (charselectnew == charselectold) {
      ploticon3(130, buttonrect);
      in();
      viewcharacter(charselectnew, 0);
      updatemain(FALSE, -1);
      combatupdate2(charup);
      updatecontrols();
      SetRect(&buttonrect, 0, 0, 0, 0);
    } else
      sound(660);

    charselectold = charselectnew;

    FlushEvents(everyEvent, 0);

    if (c[charselectnew].inbattle) {
      centerfield(pos[charselectnew][0], pos[charselectnew][1]);
      for (t = 0; t < 12; t++) {
        bodyground(charselectnew, 0);
        drawbody(charselectnew, 0, 0);
      }
      SetPort(GetWindowPort(screen));
      ploticon3(130, buttonrect);
    } else {
      SetPort(GetWindowPort(screen));
      ploticon3(130, buttonrect);
    }
  }

  if (theControl == movelook) {
  forcemove:

    if (!inwindow(charup)) {
      if (inspell)
        goto centercharupspell;
      else {

        if ((pos[charup][0] + deltax + fieldx < 3) || (pos[charup][1] + deltay + fieldy < 3) || (pos[charup][0] + deltax + fieldx > 86) || (pos[charup][1] + deltay + fieldy > 86))
          goto canflee;

        key = 'c';
        checkkeypad(0);
        deltax = deltay = 0;
        return;
      }
    }

    if (lastshown != charup)
      combatupdate2(charup);

    if (!PtInRect(point, &lookrect)) {
      SetCCursor(sword);
      tagold = 20;
    } else
      updatearrow(0);

    if ((poss) || (c[charup].condition[25])) {
      if (checkforenemy(1))
        c[charup].target = enemy[0];
      else {
        if (c[charup].target < 0) {
        getnewt:
          temp = randrange(0, maxloopminus);
          if (twixt(temp, 6, 9))
            goto getnewt;

          if (temp < 10) {
            if ((c[temp].traiter != c[charup].traiter) && (c[temp].inbattle))
              c[charup].target = temp;
            else
              goto getnewt;
          } else if ((c[charup].traiter != monster[temp - 10].traiter) && (monster[temp - 10].stamina > 0))
            c[charup].target = temp;
          else
            goto getnewt;
        }
      }
      temptarget = c[charup].target;

      if (temptarget < 9) {
        if ((!c[temptarget].inbattle) || (c[temptarget].stamina < 1) || (c[charup].traiter == c[temptarget].traiter))
          goto getnewt;
        targetx = pos[temptarget][0];
        targety = pos[temptarget][1];
      } else {
        if ((monster[temptarget - 10].stamina < 1) || (monster[temptarget - 10].traiter == c[charup].traiter))
          goto getnewt;
        targetx = monpos[temptarget - 10][0];
        targety = monpos[temptarget - 10][1];
      }

      deltax = deltay = 0;
      if (targetx < pos[charup][0])
        deltax = -1;
      if (targetx > pos[charup][0])
        deltax = 1;
      if (targety < pos[charup][1])
        deltay = -1;
      if (targety > pos[charup][1])
        deltay = 1;

      if (c[charup].condition[0]) /**** running away ****/
      {
        deltax *= -1;
        deltay *= -1;
      }

      goto moveforward;
    }

    if (!key) {
      deltax = deltay = 0;

      if (point.v < pos[charup][1] * 32)
        deltay--;
      if (point.v > pos[charup][1] * 32 + 32)
        deltay++;
      if (point.h < pos[charup][0] * 32)
        deltax--;
      if (point.h > pos[charup][0] * 32 + 32)
        deltax++;
    }
    key = 0;

    if ((q[up] < 9) && (q[up] > -1)) {
      if ((pos[charup][0] + deltax + fieldx < 2) || (pos[charup][1] + deltay + fieldy < 2) || (pos[charup][0] + deltax + fieldx > 87) || (pos[charup][1] + deltay + fieldy > 87)) {
      canflee:
        deltax = deltay = 0;

        if ((pos[charup][0] + deltax + fieldx < 1) || (pos[charup][1] + deltay + fieldy < 1) || (pos[charup][0] + deltax + fieldx > 88) || (pos[charup][1] + deltay + fieldy > 88)) {
          flashmessage((StringPtr) "Sorry...This PCs position forces him to flee from battle.", 100, 100, 0, 6000);
          goto forceflee;
        }

        if (question3((StringPtr) "Embrace Cowardice", (StringPtr) "Stay and Fight") == 2) /**** ask if they want to flee battle ****/
        {
        forceflee:
          bodyground(charup, 0);
          bodyfield(charup);
          q[up] = c[charup].position = pos[charup][0] = pos[charup][1] = -1;
          c[charup].inbattle = 0;
          c[charup].prestigepenelty += 200;
          updatelight(charup, FALSE);
          reply = FALSE;
          for (t = 0; t <= charnum; t++)
            if ((c[t].inbattle) && (!c[t].traiter))
              reply = TRUE;
          if (!reply) {
            killmon = numenemy;
            coward = TRUE;
          }
          getup(FALSE);
          return;
        } else {
          if (c[charup].condition[25] > 0) {
            c[charup].condition[25] = poss = 0;
          }
        }
      }
    }

  moveforward:

    pass = hit = -1;
    beenattacked = c[charup].beenattacked;
    checkforenemy(0); /****** check for guarding enemy ***********/
    checkforenemy(1); /****** check for enemy before moving *****/

  loopposs:
    if (((poss) || (c[charup].condition[25])) && (hit > -1)) {
      if ((pass++ > 20) || (c[charup].movement < 1)) {
        deltax = deltay = 0;
        checkforenemy(0); /****** check for guarding enemy ***********/
        c[charup].guarding = TRUE;
        getup(FALSE);
        return;
      } else /********* possesed char juking *****************/
      {
        deltax = -2 + Rand(3);
        deltay = -2 + Rand(3);
      }
    }

    if (deltax < 0)
      c[charup].face = 1;
    else if (deltax > 0)
      c[charup].face = 0;

    hit = abs(field[pos[charup][0] + deltax + fieldx][pos[charup][1] + deltay + fieldy]);
    cost = getmovecost(hit);

    if (hit <= maxloop) {
      if (hit < 9) {
        if ((c[charup].traiter != c[hit].traiter))
          goto attackfriendhere;
      } else if (monster[hit - 10].traiter != c[charup].traiter) {
      attackfriendhere:

        if (!c[charup].toggle) {
          if (c[charup].armor[2])
            loaditem(c[charup].armor[2]);
        } else if (c[charup].armor[15])
          loaditem(c[charup].armor[15]);
        else
          goto noweapon;

        if (item.sp2 > 1100) {
          loadspell2(item.sp2);
          if ((spellinfo.spellclass == 9) && (spellinfo.damagetype == 9) && (c[charup].toggle)) {
            if (checkforenemy(1)) {
              if (poss) {
                if (c[charup].handtohand < 8)
                  toweap(charup);
                theControl = movelook;
                goto noweapon;
              } else {
                if (!autoweapswitch) {
                  SetPort(GetWindowPort(look));
                  ForeColor(blackColor);
                  BackColor(whiteColor);
                  warn(39);
                  return;
                } else {
                  key = 1;
                  goto gomelee;
                }
              }
            }
          }
        }
      noweapon:

        c[charup].movement -= 3;
        c[charup].attacks -= 2;
        if (c[charup].movement < 0)
          c[charup].movement = 0;
        combatupdate(charup);
        attack(charup, hit);
        if (c[charup].stamina < 1)
          c[charup].attacks = 0;
        SetPort(GetWindowPort(screen));
        pict(207, spellrect);
        return;
      }

      if (hit == q[up])
        return;

      if (hit > 9) {
        if ((!monster[hit - 10].size) && (c[charup].movement > 4))
          goto goodswap;
        else if ((!poss) && (!c[charup].condition[25]))
          goto askem;
        else if ((poss) || (c[charup].condition[25]))
          goto loopposs;
      } else if (c[charup].movement > 4) {
      goodswap:
        if ((poss) || (c[charup].condition[25]))
          goto autoswap;

        if (question3((StringPtr) "Swap Positions", (StringPtr) "Attack Friend") == 2) {
        autoswap:

          canundo = 0;

          if (hit < 10) /**** swap two PCs ****/
          {
            temp = pos[charup][0];
            pos[charup][0] = pos[hit][0];
            pos[hit][0] = temp;

            temp = pos[charup][1];
            pos[charup][1] = pos[hit][1];
            pos[hit][1] = temp;

            temp = charunder[charup];
            charunder[charup] = charunder[hit];
            charunder[hit] = temp;

            field[pos[charup][0] + fieldx][pos[charup][1] + fieldy] = charup;
            field[pos[hit][0] + fieldx][pos[hit][1] + fieldy] = hit;
          } else /**** swap with monster ****/
          {
            temp = pos[charup][0];
            pos[charup][0] = monpos[hit - 10][0];
            monpos[hit - 10][0] = temp;

            temp = pos[charup][1];
            pos[charup][1] = monpos[hit - 10][1];
            monpos[hit - 10][1] = temp;

            temp = charunder[charup];
            charunder[charup] = monster[hit - 10].underneath[1][1];
            monster[hit - 10].underneath[1][1] = temp;

            field[pos[charup][0] + fieldx][pos[charup][1] + fieldy] = charup;
            field[monpos[hit - 10][0] + fieldx][monpos[hit - 10][1] + fieldy] = hit;
          }
          sound(654);
          deltax = deltay = 0;
          centerfield(5 + (2 * screensize), 5 + screensize);
          c[charup].movement -= 5;
          combatupdate2(charup);
          return;
        } else if (itemHit == 1)
          return; /**** hit esc key ****/
        else
          goto wantstoattack;
      } else if ((!poss) && (!c[charup].condition[25])) {
      askem:
        if (question3((StringPtr) "Attack Your Friend", (StringPtr) "Do Not Attack") == 2) {
        wantstoattack:
          if (hit > 10) {
            if (!monster[hit - 10].traiter) {
              monster[hit - 10].traiter++;
              numenemy++;
            }
          }
          goto attackfriendhere;
        } else
          return;
      } else
        goto loopposs;
    }

    if (hit > 999) {
      if (mapstats[hit - 1000].solid) /**** hit solid****/
      {
        if ((!poss) && (!c[charup].condition[25]))
          sound(-129);
        else
          goto loopposs; /******* send poss char back to try again **************/
      } else
        goto keepchecking;
    } else {
    keepchecking:

      if (c[charup].movement - cost > -1) {
        if (checkforenemy(2)) {
          getup(FALSE);
          return;
        }
        c[charup].movement -= cost;
        if (c[charup].beenattacked != beenattacked)
          updatecontrols();
        bodyground(charup, 0);
        /******** tag field ********/
        field[pos[charup][0] + fieldx][pos[charup][1] + fieldy] = charunder[charup];
        charunder[charup] = field[pos[charup][0] + fieldx + deltax][pos[charup][1] + fieldy + deltay];
        field[pos[charup][0] + fieldx + deltax][pos[charup][1] + fieldy + deltay] = charup;

        pos[charup][0] += deltax;
        pos[charup][1] += deltay;
        sound((mapstats[charunder[charup] - 1000].sound));
        drawbody(charup, 0, 0);
        collide(charup, FALSE); /****** check for spell collision *****/
        if (c[charup].stamina < 1) {
          c[charup].movement = c[charup].attacks = 0;
          return;
        }
      } else /**** look for attack at end of move ****/
      {
        deltax = deltay = 0;
        if (checkforenemy(2)) {
          getup(FALSE);
          return;
        }
      }
    }

    combatupdate(charup);

    if ((pos[charup][0] < 2) || (pos[charup][0] > 7 + (screensize * 5)) || (pos[charup][1] < 2) || (pos[charup][1] > 7 + (screensize * 3))) {
      centerfield(pos[charup][0], pos[charup][1]);
    }

    if ((c[charup].movement - cost < 0) || ((deltax == 0) && (deltay == 0))) {
      deltax = deltay = 0;
      if ((poss) || (c[charup].condition[25]))
        goto loopposs;
      sound(-130);
    }

    if (checkforenemy(0))
      getup(FALSE);
  }
}
