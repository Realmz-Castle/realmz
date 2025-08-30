#include "prototypes.h"
#include "realmzbuild.h"
#include "variables.h"

/**************** textbox ************************/
void textbox(short class, short index, short click, short different, Rect newrect) {
  FILE* fp = NULL;
  GrafPtr oldport;
  TEHandle messagetext;
  Rect userect;

  if (index == 0)
    return;

  if (class == -1) {
    if (index < 0)
      click = 0;
    else
      click = 1;
  }

  GetPort(&oldport);

  if (different)
    userect = newrect;
  else {
    userect = textrect;
    SetPort(GetWindowPort(screen));
  }
  ForeColor(yellowColor);
  SetCCursor(sword);
  if (class == -1) {
    GetIndString(myString, 3, 1);
    getfilename("Data SD2");
    if ((fp = MyrFopen(filename, "r")) == NULL)
      scratch(200);
    fseek(fp, abs(index) * sizeof myString, SEEK_SET);
    fread(myString, sizeof myString, 1, fp);
    MyrStrPtoC((Ptr)myString); // Myriad
    fclose(fp);
    stringindex = abs(index);
  } else {
    GetIndString(myString, 3, 1);
    GetIndString(myString, class, index);
  }
  TextFont(defaultfont);
  TextSize(12);
  TextFace(0);
  TextMode(1);
  BackPixPat(base);
  messagetext = TENew(&userect, &userect);
  TESetText(myString, 255, messagetext);
  MyrDump("textbox :[%s]\n", myString);
  TESetSelect(0, 1, messagetext);
  EraseRect(&userect);
  if (!different)
    userect.bottom = 333;
#ifndef PC // Myriad
  TEDelete(messagetext);
#endif
  if (!different)
    userect.bottom = 448 + downshift;
  TEUpdate(&userect, messagetext);
  TEDispose(messagetext);
  if ((!inspell) && (!inbooty))
    sound(5000);

  if ((divine) || (development)) {
    SetPort(GetWindowPort(look));
    TextSize(16);
    TextMode(0);
    TextFont(genevafont);
    MoveTo(20, 20);
    DrawString("\p                       ");
    MoveTo(20, 20);
    DrawString("\pString ID ");
    NumToString(index, myString);
    DrawString(myString);
    TextFont(defaultfont);
    TextSize(12);
    TextFace(0);
    TextMode(1);
    BackPixPat(base);
    SetPort(GetWindowPort(screen));
  }

  if (click) {
  backup:
    if (autonote)
      SetCCursor(mouse);
    else
      SetCCursor(notecursor);
    delay(10);
    gTheEvent.what = gTheEvent.modifiers = gTheEvent.message = 0;
    FlushEvents(everyEvent, 0);

    for (;;) {
      WaitNextEvent(everyEvent, &gTheEvent, 0L, NIL);
#ifdef PC // Myriad
      DoCorrectBugMADRepeat();
#endif

      if ((gTheEvent.what == mouseDown) || (gTheEvent.what == keyDown)) {
        if ((class == -1) && (((gTheEvent.message & charCodeMask) == 'n') || (autonote))) /**** n key = take note ***/
        {
          notes[abs(index)] = TRUE;
          journalindex2 = abs(index);
          if (!autonote)
            flashmessage((StringPtr) "Note Taken.", 30, 355, 20, 145);
        }
        EraseRect(&userect);
        SetCCursor(sword);
        goto out;
      }
    }
  }
out:

  FlushEvents(everyEvent, 0);
  different = 0;
  TextFace(bold);
  TextFont(font);
  SetPort(oldport);
}

/********************************* timeclick *******************/
short timeclick(unsigned char number, short checkforrandom) {
  FILE* fp = NULL;
  Boolean doday = FALSE;
  GrafPtr oldport;
  short t, timeloop, timeindex;
  short chunk, suprise;
  short scale = 5;
  Point test;
  char s[20];

  Boolean tag = FALSE;
  if (basescale[lastpix])
    scale = 1; /********* if indoor area, scale = 1 else = 5 ****/

  GetPort(&oldport);

  if (!number) {
    tyme.tm_min -= scale;
    tag = TRUE;
    number++;
  }

  for (timeloop = number; timeloop > 0; timeloop--) {
    tickcheck();
    tyme.tm_min += scale;

    if (tag)
      goto jump1;

    if (tyme.tm_min >= 60) {
      if (!tag)
        reduce();
      tyme.tm_min -= 60;
      tyme.tm_hour++;

      int enable_recomposite = WindowManager_SetEnableRecomposite(0);
      if (timeclick)
        updatefat(FALSE, 1, FALSE);

      for (t = 0; t <= charnum; t++) {
        if ((c[t].spellpoints < c[t].spellpointsmax) && (c[t].stamina > 0) && (!c[t].condition[25])) {
          chunk = c[t].level / 2;
          if (chunk < 1)
            chunk = 1;
          c[t].spellpoints += chunk;
          if (c[t].spellpoints > c[t].spellpointsmax)
            c[t].spellpoints = c[t].spellpointsmax;
          updatecharshort(t, FALSE);
        }
      }
      WindowManager_SetEnableRecomposite(enable_recomposite);

      for (t = 0; t < heldover; t++) /****** Allies Spell Points ******/
      {
        if (holdover[t].spellpoints < holdover[t].maxspellpoints) {
          chunk = holdover[t].hd / 2;
          if (chunk < 1)
            chunk = 1;
          holdover[t].spellpoints += chunk;
          if (holdover[t].spellpoints > holdover[t].maxspellpoints)
            holdover[t].spellpoints = holdover[t].maxspellpoints;
        }
      }

    jump1:
      if ((tyme.tm_hour == 12) || (tyme.tm_hour == 24)) {
        if (tyme.tm_hour == 24) {
          tyme.tm_yday++;
          tyme.tm_hour -= 24;
          timeindex = -1;

          for (t = 0; t <= charnum; t++) {
            c[t].age++;
            characterl = c[t];
            temp = age(c[t].age, c[t].race, c[t].currentagegroup);
            c[t] = characterl;
            if (temp != 0) {
              in();
              showageupdate(t, c[t].currentagegroup, 0);
              updatemain(1, -1);
              centerpict();
              updatecontrols();
            }
          }

        continueday:

          /**************** check for time encounter ******/

          if ((fp = MyrFopen(":Data Files:CTD3", "r+b")) == NULL)
            scratch(201);

          fseek(fp, (timeindex + 1) * sizeof dotime, SEEK_SET);
          doday = FALSE;
          do {
            fread(&dotime, sizeof dotime, 1, fp);
            CvtTimeEncounterToPc(&dotime);
            timeindex++;

            if (timeindex > 150)
              goto nottimetoday;

            if (dotime.day == tyme.tm_yday) {
              /**** struct timeencounter
             {
              short day,increment,percent,door;
              short reclevel,recrect,recx,recy,recitem,recquest;
              short stuff[10];
             };*****/

              dotime.day += dotime.increment;

              if (Rand(100) > dotime.percent)
                goto breakofday;
              if ((dotime.recitem > 0) && (!checkforitem(dotime.recitem, FALSE, -1)))
                goto breakofday;
              if ((dotime.recquest > -1) && (!quest[dotime.recquest]))
                goto breakofday; /**** need the quest ***/

              if (dotime.stuff[0] > 0) /**** need specific location ****/
              {
                if (((dotime.stuff[0] == 1) && (landlevel == dotime.reclevel) && (!indung)) || ((dotime.stuff[0] == 2) && (dunglevel == dotime.reclevel) && (indung))) {
                  doday = TRUE;
                  if (dotime.stuff[0] == 1) /*** land ****/
                  {
                    test.h = lookx + partyx + deltax;
                    test.v = looky + partyy + deltay;
                  } else {
                    test.h = floorx;
                    test.v = floory;
                  }

                  if (dotime.recrect > -1) /**** need rect ***/
                  {
                    if (!PtInRect(test, &randlevel.randrect[dotime.recrect]))
                      doday = FALSE;
                  }

                  if (dotime.recx > -1) /**** need x ***/
                  {
                    if (test.h != dotime.recx)
                      doday = FALSE;
                  }

                  if (dotime.recx > -1) /**** need y ***/
                  {
                    if (test.v != dotime.recy)
                      doday = FALSE;
                  }
                  if (doday)
                    goto breakofday;
                }
              } else {
                doday = TRUE;
                goto breakofday;
              }
            }
          } while (dotime.day);
        breakofday:

          fseek(fp, timeindex * sizeof dotime, SEEK_SET);
          fwrite(&dotime, sizeof dotime, 1, fp);
        nottimetoday:

          fclose(fp);
        }
        if (!tag) {
          for (t = 0; t <= charnum; t++) {
            if (c[t].stamina < c[t].staminamax) {
              chunk = c[t].level / 3;
              if (c[t].stamina > -10) {
                if (!checkforitem(877, TRUE, -1))
                  chunk /= 2; /***** who = -1 is anybody ****/
                if (chunk < 1)
                  chunk = 1;
                heal(t, chunk + c[t].condition[9], 0);
              }
            }
          }

          for (t = 0; t < heldover; t++) {
            if (holdover[t].stamina < holdover[t].staminamax) {
              holdover[t].stamina += ((holdover[t].hd / 4) + 1);
              if (holdover[t].stamina > holdover[t].staminamax)
                holdover[t].stamina = holdover[t].staminamax;
            }
          }
        }
      }
    }
  }

  int enable_recomposite = WindowManager_SetEnableRecomposite(0);

  SetPort(GetWindowPort(screen));
  BackPixPat(base);

  itemRect.top = 375 + downshift;
  itemRect.bottom = itemRect.top + 14;
  itemRect.left = 560 + leftshift;
  itemRect.right = itemRect.left + 70;
  EraseRect(&itemRect);

  TextMode(1);
  TextFont(defaultfont);
  TextSize(12);
  TextFace(bold);
  RGBBackColor(&greycolor);
  ForeColor(yellowColor);
  MoveTo(561 + leftshift, 386 + downshift);
  strftime(s, 20, "%I:%M %p", &tyme);
  // CtoPstr(s);
  MyrDrawCString(s);

  itemRect.top = 390 + downshift;
  itemRect.bottom = itemRect.top + 14;
  itemRect.left = 585 + leftshift;
  itemRect.right = itemRect.left + 38;
  EraseRect(&itemRect);

  MoveTo(585 + leftshift, 401 + downshift);
  string(tyme.tm_yday);

  WindowManager_SetEnableRecomposite(enable_recomposite);

  SetPort(oldport);
  TextFont(font);
  if (!indung) {
    test.h = lookx + partyx;
    test.v = looky + partyy;
  } else {
    test.h = floorx;
    test.v = floory;
  }

  if (doday) {
    newland(0L, 0L, 1, dotime.door, 0);
    goto continueday;
  }

  if ((!tag) && (checkforrandom)) {
    for (t = 19; t > -1; t--) {
      if ((twixt(test.h, randlevel.randrect[t].left, randlevel.randrect[t].right)) && (twixt(test.v, randlevel.randrect[t].top, randlevel.randrect[t].bottom))) {
        if (Rand(10000) <= randlevel.percent[t]) {
          for (tt = 0; tt < 3; tt++) {
            if (Rand(100) <= abs(randlevel.randdoorpercent[t][tt])) {
              if (randlevel.randdoorpercent[t][tt] > 0) {
                randlevel.randdoorpercent[t][tt] = 0;
                if (!indung)
                  saveland(landlevel);
                else
                  saveland(dunglevel);
              }
              incamp = intemple = inshop = shopavail = currentshop = templeavail = 0;
              moveparty(0);
              updatecontrols();
              reply = newland(0L, 0L, 1, randlevel.randdoor[t][tt], 0);
              deltax = deltay = 0;
              return (reply);
            }
          }
          if ((!partycondition[7]) && (canencounter)) {
            if (randlevel.battlerange[t][0]) {
              suprise = 0;
              if (Rand(100) < randlevel.option[t]) {
                sound(randlevel.sound[t]);
                textbox(-1, -(abs(randlevel.text[t])), FALSE, FALSE, textrect);
                reply = question2(0, 0);
                EraseRect(&textrect);
                if (!reply)
                  goto optnot;
                /**** good suprise *****/
                suprise = TRUE;
              } else if (Rand(100) < 10)
                suprise = -1; /**** bad suprise *****/

              battlenum = randrange(randlevel.battlerange[t][0], randlevel.battlerange[t][1]);
              combat(suprise, 0); /***** default of indoor 1 *****/
              if (revertgame)
                return (0);
              booty(0);
              lastpix = -1;
              if (indung)
                loadland(dunglevel, TRUE);
              else
                loadland(landlevel, TRUE);

              updatemain(FALSE, 0);
            }
          }
        }
      optnot:
        if (randlevel.only[t])
          return (0); /***** hit an only rect *****/
      }
    }
    return (0);
  }
  return (0);
}
