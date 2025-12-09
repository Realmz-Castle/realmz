#include "prototypes.h"
#include "variables.h"

extern Boolean tagger;

/****************** combat **************************/
void combat(short suprise, short mode) {
  FILE* fp = NULL;
  WindowPtr whichWindow;
  short temp, intellloop, a, numoftarg, holdtargx, holdtargy, t, oldnumoftarg = 0;
  short seex, seey, fromx, fromy, cantseeindex, spellpick, targetindex, commandkey, pass = 0;
  short targetnew, spellloop, targetsleft;
  Rect targetrect;
  Boolean didcast, sliding = FALSE;

  DisableItem(gScenario, 0);
  DisableItem(gParty, 0);
  DrawMenuBar();

  compactheap();

  music(11); /**** battle music ***/

  blankround = encountflag = 0;

  if (battlenum < 0)
    suprise = -1; /****** force suprise battle on party *****/

  getfilename("Data BD");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(7);
  fseek(fp, abs(battlenum) * sizeof battle, SEEK_SET);
  fread(&battle, sizeof battle, 1, fp);
  CvtBattleToPc(&battle);
  fclose(fp);

  if (battle.messagebefore)
    textbox(-1, battle.messagebefore, 0, 0, textrect);

  textbox(3, 53, 0, 0, textrect); /**** building map message ***/
  incombat = combatround = 1;
  for (t = 0; t <= charnum; t++)
    if (c[t].stamina < 1)
      updatepictbox(t, TRUE, 0);

  switch (suprise) {
    case 1:
      flashmessage((StringPtr) "You have suprise on your side!            You move first.  (Click Mouse)", 30, 100, 0, 6001);
      break;

    case -1:
      flashmessage((StringPtr) "They have suprised you!                   They move first. (Click Mouse)", 30, 100, 0, 6001);
      break;
  }

  if (!indung)
    saveland(landlevel);
  else
    saveland(dunglevel);

  sound(10049);

  updatefat(FALSE, FALSE, TRUE);

  combatmap();

  combatsetup(0, suprise, mode);

  // NOTE(fuzziqersoftware): It seems that the window order isn't correctly
  // updated when a battle is started from 3D dungeon view. This obviously
  // worked in the original builds, so it's likely a deficiency of some sort in
  // our Window Manager implementation. This appears to be the easiest way to
  // work around the issue.
  if (indung && (viewtype == 1)) {
    SelectWindow(look);
  }

  while (incombat) {
    music(11); /**** battle music ***/

    if (killmon >= numenemy) /**** no enemy left/ won battle ****/
    {
      incombat = 0;
      if (!indung)
        loadland(landlevel, 1);
      else
        loadland(dunglevel, 1);
      return;
    }

    GetMouse(&point);
    key = 0;

    if (!hide) {
      if (!PtInRect(point, &lookrect)) {
        SetCCursor(sword);
        tagold = 20;
      } else
        updatearrow(0);
    }

    if (monsterturn) {
      SystemTask();
      if (Button()) {
        GetMouse(&gTheEvent.where);
        LocalToGlobal(&gTheEvent.where);
        thePart = FindWindow(gTheEvent.where, &whichWindow);

        switch (thePart) {
          case inMenuBar:
            switch (thePart) {
              case inMenuBar:
                compactheap();
                menuChoice = MenuSelect(gTheEvent.where);
                HandleMenuChoice();
                if (revertgame)
                  return;
                break;
            }
            break;
        }
      }

      didcast = FALSE;
      monster[monsterup].guarding = TRUE;

      // if (monster[monsterup].condition[0]) goto startmove;/**** start retreat ***/
      if (monster[monsterup].condition[0])
        goto startretreat; /**** start retreat ***/

      if (Rand(100) <= monster[monsterup].misslepercent) {
        if (!checkforenemy(1)) {
          monster[monsterup].weapon = monster[monsterup].items[1];
          loaditem(monster[monsterup].weapon);
          loadspell2(item.sp2);
          goto missle;
        } else
          monster[monsterup].weapon = monster[monsterup].items[0];
      }

      if (Rand(100) <= monster[monsterup].castpercent) {
      tryspell:

        if ((!monster[monsterup].condition[5]) && (!monster[monsterup].condition[29]) && (!monster[monsterup].condition[39]) && (!monster[monsterup].condition[1]) && (monstercasting == 0)) {
          if (!monster[monsterup].beenattacked) {

            for (spellloop = 0; spellloop < monster[monsterup].noofmagattacks; spellloop++) {
              for (t = 0; t < 30; t++) {
                spellpick = randrange(0, 9);
                if (monster[monsterup].spells[spellpick] > 1100) {
                  loadspell2(monster[monsterup].spells[spellpick]);
                  goto good;
                }
              }
              goto nospell;

            good:

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
                inspell = didcast = TRUE;
                combatupdate2(monsterup + 10);
                showspellinfo();
                resolvespell();
                goto nextspell;
              }

              if (spellinfo.targettype == 9) /*** all friendly ***/
              {
                for (t = 0; t <= charnum; t++)
                  if ((c[t].traiter == monster[monsterup].traiter) && (c[t].inbattle))
                    track[t] = TRUE;
                for (t = 0; t < nummon; t++)
                  if ((monster[t].traiter == monster[monsterup].traiter) && (monster[t].stamina > 0))
                    track[t + 10] = TRUE;
                inspell = didcast = TRUE;
                combatupdate2(monsterup + 10);
                showspellinfo();
                monster[monsterup].guarding = 0;
                resolvespell();
                goto nextspell;
              }

              if (spellinfo.targettype == 10) /*** all enemy ***/
              {
                for (t = 0; t <= charnum; t++)
                  if ((c[t].traiter != monster[monsterup].traiter) && (c[t].inbattle))
                    track[t] = TRUE;
                for (t = 0; t < nummon; t++)
                  if ((monster[t].traiter != monster[monsterup].traiter) && (monster[t].stamina > 0))
                    track[t + 10] = TRUE;
                inspell = didcast = TRUE;
                combatupdate2(monsterup + 10);
                showspellinfo();
                monster[monsterup].guarding = 0;
                resolvespell();
                goto nextspell;
              }

            missle:
              targetindex = targetsleft = 0;
              cleartarget();

              powerlevel = Rand(7);

              spellrange = abs(spellinfo.range1 + spellinfo.range2 * powerlevel);

              targetsleft = getrange(monsterup + 10, monster[monsterup].traiter, spellinfo.cannot); /**** canresist = 4 means target friends ***/

              if (!targetsleft)
                goto alloutofrange;

              if (!spellinfo.targettype)
                powerlevel = Rand(targetsleft);
              if (powerlevel > 7)
                powerlevel = 7;

            lowerlevel:

              if (spellinfo.cost * powerlevel > monster[monsterup].spellpoints) {
                powerlevel--;
                if (!powerlevel)
                  goto alloutofrange;
                goto lowerlevel;
              }
              targetnum = powerlevel;
              targetsleft = targetnum;

              if (spellinfo.targettype > 0)
                targetsleft = targetnum = 1;

              if (monster[monsterup].size)
                spellrange++;
              if (monster[monsterup].size == 3)
                spellrange++;

              if ((spellinfo.targettype == 3) && (spellinfo.size == 10) && (range[maxloop + 1] < spellrange)) /**** wall of ?? ****/
              {
                for (t = 0; t < 100; t++) {
                  temp = Rand(10 + nummon) - 1;
                  if (range[temp] <= spellrange) {
                    if (temp < 9) {
                      target[0][0] = pos[temp][0] + fieldx;
                      target[0][1] = pos[temp][1] + fieldy;
                      goto gocast;
                    } else {
                      target[0][0] = monpos[temp - 10][0] + fieldx;
                      target[0][1] = monpos[temp - 10][1] + fieldy;
                      goto gocast;
                    }
                  }
                }
                goto alloutofrange;
              }

              if ((spellinfo.targettype > 1) && (range[maxloop + 1] < spellrange /***-spellinfo.targettype/6***/)) {
                for (intellloop = 0; intellloop < maxloop; intellloop++) {
                  numoftarg = 0;

                  temp = intellloop;

                  if (range[temp] <= spellrange) {
                    if (temp < 9) {
                      target[0][0] = pos[temp][0] + fieldx;
                      target[0][1] = pos[temp][1] + fieldy;
                      numoftarg = spelltargets(0, -1, TRUE, 0); /******* get number of targets for intelligent cast ****/

                      if (numoftarg > oldnumoftarg) {
                        oldnumoftarg = numoftarg;
                        holdtargx = target[0][0];
                        holdtargy = target[0][1];
                      }
                    } else {
                      target[0][0] = monpos[temp - 10][0] + fieldx;
                      target[0][1] = monpos[temp - 10][1] + fieldy;
                      numoftarg = spelltargets(0, -1, TRUE, 0); /******* get number of targets for intelligent cast ****/
                      if (numoftarg > oldnumoftarg) {
                        oldnumoftarg = numoftarg;
                        holdtargx = target[0][0];
                        holdtargy = target[0][1];
                      }
                    }
                  }
                }

                if (oldnumoftarg) {
                  target[0][0] = holdtargx;
                  target[0][1] = holdtargy;
                  oldnumoftarg = 0;
                  goto gocast;
                } else if ((powerlevel > 1) && (spellinfo.targettype == 4)) {
                  powerlevel--;
                  goto lowerlevel;
                }

                goto alloutofrange;
              }

              if ((spellinfo.targettype < 2) && (range[maxloop + 1] <= spellrange)) {
                targetnum = 0;
                while (targetsleft) {
                passagain:
                  pass++;
                  if (pass > 100) {
                    pass = 0;
                    if (targetnum)
                      goto gocast;
                    goto alloutofrange;
                  }
                  targetnew = Rand(10 + nummon) - 1;

                  if (range[targetnew] > spellrange)
                    goto passagain; //**** is target out of range?

                  if (spellinfo.range1 + spellinfo.range2 > 0) //***** does spell require LOS?
                  {
                    fromx = monpos[monsterup][0];
                    fromy = monpos[monsterup][1];

                    if (targetnew < 10) {
                      seex = pos[targetnew][0];
                      seey = pos[targetnew][1];
                    } else {
                      seex = monpos[targetnew - 10][0];
                      seey = monpos[targetnew - 10][1];
                    }
                    if (!cansee(fromx, fromy, seex, seey))
                      goto passagain; //**** is target in LOS?
                  }

                  for (t = 0; t < 7; t++)
                    if (target[t][0] == targetnew)
                      goto passagain;

                  target[targetindex][0] = targetnew;
                  if (targetnew < 9)
                    target[targetindex++][1] = 1;
                  else
                    target[targetindex++][1] = 2;
                  targetsleft--;
                  targetnum++;
                }
              } else {
              alloutofrange:
                goto nospell;
              }
            gocast:
              monster[monsterup].movement = monster[monsterup].guarding = 0;
              monster[monsterup].spellpoints -= spellinfo.cost * powerlevel;
              inspell = TRUE;
              combatupdate2(monsterup + 10);

              if (spellinfo.spellclass == 9) {
                finddxdy(monsterup + 10, target[0][0]);
                powerlevel = 1;
              }
              showtargets(0);
              showspellinfo();
              if (spellinfo.canrotate)
                rotate = (Rand(4) - 1);
              cast(targetnum, monsterup + 10);
            nextspell:
              didcast = TRUE;
            }
          nospell:
            nospell++;
            cleartarget();
            if (didcast)
              goto monsterdone;
          } else
            goto startmove;
        } else
          goto startmove;
      }

      if (!monster[monsterup].condition[1])
        checkforenemy(0);

    startmove:

      cantseeindex = 0;

      if ((numenemy - killmon) <= 0)
        goto monsterdone;

      while ((monster[monsterup].movement > 0) && (monster[monsterup].attacknum < (monster[monsterup].noofattacks + monster[monsterup].bonusattack)) && (!monster[monsterup].condition[1])) {
        if ((numenemy - killmon) <= 0)
          goto monsterdone;

        if (monster[monsterup].target < -1)
          monster[monsterup].target = -1;
        temp = monster[monsterup].target;

        if (temp == -1)
          goto newtarget;

        if (temp < 9) {
          if ((!c[temp].inbattle) || (c[temp].traiter == monster[monsterup].traiter))
            goto newtarget;
        } else if ((monster[temp - 10].stamina < 1) || (monster[monsterup].traiter == monster[temp - 10].traiter)) {
        newtarget:

          if (sliding == TRUE)
            goto newtargetslide;

          temp = Rand(10 + nummon) - 1;

        newtargetslide:

          if ((temp > charnum) && (temp < 10)) {
            if (sliding)
              monster[monsterup].target = temp = cantseeindex++;
            goto newtarget;
          }

          if (temp < 10) {
            if (c[temp].traiter == monster[monsterup].traiter) {
              if (sliding)
                monster[monsterup].target = temp = cantseeindex++;
              goto newtarget;
            }
            if (!c[temp].inbattle) {
              if (sliding)
                monster[monsterup].target = temp = cantseeindex++;
              goto newtarget;
            }
          } else {
            if (monster[temp - 10].traiter == monster[monsterup].traiter) {
              if (sliding)
                monster[monsterup].target = temp = cantseeindex++;
              goto newtarget;
            }
            if (monster[temp - 10].stamina < 1) {
              if (sliding)
                monster[monsterup].target = temp = cantseeindex++;
              goto newtarget;
            }
          }
          monster[monsterup].target = temp;
        }

        fromx = monpos[monsterup][0];
        fromy = monpos[monsterup][1];

        if (monster[monsterup].target < 10) {
          seex = pos[temp][0];
          seey = pos[temp][1];
        } else {
          seex = monpos[temp - 10][0];
          seey = monpos[temp - 10][1];
        }

        if ((!cansee(fromx, fromy, seex, seey)) && (cantseeindex < 110)) /*** can monster see target? ****/
        {
          monster[monsterup].target = temp = cantseeindex++; /*** Begin sliding down characters to attack ****/
          sliding = TRUE;
          if (cantseeindex > 110) {
            monster[monsterup].target = -1;
          }
          goto newtargetslide;
        } else
          sliding = FALSE;

        if (monster[monsterup].target > 100)
          monster[monsterup].target = -1;
        if (monster[monsterup].target < -1)
          monster[monsterup].target = -1;

      startretreat:

        if (!movemonster()) {
          checkforenemy(0);
          if ((nospell < 2) && (monster[monsterup].castpercent) && (!didattack)) {
            nospell += 2;
            monster[monsterup].movement = 0;
            if (!monster[monsterup].beenattacked) {
              if ((monster[monsterup].condition[5]) || (monster[monsterup].condition[29]) || (monster[monsterup].condition[39]) || (monster[monsterup].condition[1]))
                goto monsterdone;
              goto tryspell;
            }
          }
          goto monsterdone;
        }

        if (monster[monsterup].condition[1])
          goto monsterdone;
        checkforenemy(0);
        combatupdate(q[up]);
      }

      if (monster[monsterup].condition[0])
        checkforenemy(0); /***** done retreating ***/
      if ((nospell < 2) && (monster[monsterup].castpercent) && (!didattack)) {
        nospell = 2;
        monster[monsterup].movement = 0;
        if (!monster[monsterup].beenattacked)
          goto tryspell;
      }
    monsterdone:
      sliding = cantseeindex = FALSE; /**** reset sliding monster stats ****/
      getup(FALSE);
    }

    if (((poss) || (c[charup].condition[25])) && (q[up] < 9))
      goto jumpposs;

    if ((q[up] < 9) && (q[up] > -1)) {
      if ((doauto[q[up]]) && (!c[q[up]].condition[0]) && (!c[q[up]].condition[26]) && (c[q[up]].stamina > 0) && (c[q[up]].inbattle)) {
        if (Button()) {
          doauto[q[up]] = FALSE;
          GetControlBounds(autoone[q[up]], &buttonrect);
          // buttonrect = *&(**(autoone[q[up]])).contrlRect;
          sound(141);
          upbutton(TRUE);
        } else {
          poss = TRUE;
          c[q[up]].condition[25] = 1;
        }
      }
    }

  backfromrange:
    if (gTheEvent.modifiers & alphaLock)
      warn(21);
    SystemTask();
    a = GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
    DoCorrectBugMADRepeat();
#endif
    MyrCheckMemory(2);

    if (IsDialogEvent(&gTheEvent)) {
      a = DialogSelect(&gTheEvent, &dummy, &itemHit);

      if (gTheEvent.what == keyDown)
        goto dokey;
    } else {

      switch (gTheEvent.what) {
        case (updateEvt):
          BeginUpdate(look);
          EndUpdate(look);
          BeginUpdate(gWindow);
          EndUpdate(gWindow);
          BeginUpdate(screen);
          EndUpdate(screen);
          if (mat) {
            BeginUpdate(mat);
            EndUpdate(mat);
          }
          break;

        case (autoKey):
          goto dokey;
          break;

        case keyDown:

        dokey:

          if (gTheEvent.modifiers & cmdKey) {
            commandkey = BitAnd(gTheEvent.message, charCodeMask);
            menuChoice = MenuKey(commandkey);
            compactheap();
            HandleMenuChoice();
            if (revertgame)
              return;
          }

          if (gTheEvent.modifiers & optionKey) /****** shift map *******/
          {
            key = gTheEvent.message;
            scanCode = (gTheEvent.message >> 8) & 0xFF;
            checkkeypad(0);

            centerfield(5 + 2 * deltax, 5 + 2 * deltay);
            key = deltax = deltay = 0;
            theControl = NIL;
            continue;
          }

          theControl = NIL;
          key = gTheEvent.message & charCodeMask;
          scanCode = (gTheEvent.message >> 8) & 0xFF;
          checkkeypad(0);
          SetPort(GetWindowPort(look));
        gotkey:
          whichset = 0;

          // Fantasoft v7.0 Begin
          // I broke the switch for checking the keyboard into two portions.  The first does a switch
          // on scancode to detect the numbers 1-0 as hit by the number keypad, the second does a switch
          // on key to detect the number 1-0 as hit on the keyboard.

          if (!monsterturn) {
            switch (scanCode) {
              case 0x12: /**** spell 1 *** '1' */;
                whichset = 1;
                break;

              case 0x13: /**** spell 2 *** '2' */;
                whichset = 2;
                break;

              case 0x14: /**** spell 3 *** '3' */;
                whichset = 3;
                break;

              case 0x15: /**** spell 4 *** '4' */;
                whichset = 4;
                break;

              case 0x17: /**** spell 5 *** '5' */;
                whichset = 5;
                break;

              case 0x16: /**** spell 6 *** '6' */;
                whichset = 6;
                break;

              case 0x1a: /**** spell 7 *** '7' */;
                whichset = 7;
                break;

              case 0x1c: /**** spell 8 *** '8' */;
                whichset = 8;
                break;

              case 0x19: /**** spell 9 *** '9' */;
                whichset = 9;
                break;

              case 0x1d: /**** spell 10 *** '0' */;
                whichset = 10;
                break;
            }

            switch (key) {

                // Fantasoft v7.0 End

              case 'i': /***** I   items    *****/
                theControl = itemsbut;
                break;

              case 'a': /***** A   Auto Character Move    *****/
                theControl = campbut;
                break;

              case 'l': /****** L   use scroll *********/

                theControl = viewspellsbut;
                goto jumpposs;

                break;

              case 's': /****** S   cast spell *********/

                theControl = castspellsbut;
                goto jumpposs;

                break;

              case 'u': /********** u ** undo *******************/
                buttonrect.top = 366 + downshift;
                buttonrect.bottom = buttonrect.top + 18;
                buttonrect.left = 422 + leftshift;
                buttonrect.right = buttonrect.left + 44;
                downbutton(TRUE);

                if (canundo) {
                  if ((!c[charup].condition[1]) && (!c[charup].traiter) && (!c[charup].condition[29])) {
                    sound(664);
                    c[charup].attacks -= (c[charup].normattacks + c[charup].attackbonus);
                    drawbody(charup, TRUE, 0);
                    bodyfield(charup);
                    pos[charup][0] = undox - fieldx;
                    pos[charup][1] = undoy - fieldy;
                    charunder[charup] = field[undox][undoy];
                    field[undox][undoy] = charup;
                    SetPort(GetWindowPort(look));
                    up--;
                    SetPort(GetWindowPort(screen));
                    getup(FALSE);
                  }
                } else /**** cant undo ****/
                {
                  warn(82);
                  upbutton(TRUE);
                }
                break;

              case 'e': /********** e ** escape *******************/
                buttonrect.top = 366 + downshift;
                buttonrect.bottom = buttonrect.top + 18;
                buttonrect.left = 364 + leftshift;
                buttonrect.right = buttonrect.left + 54;
                downbutton(TRUE);
                getrange(charup, 0, FALSE);
                if (range[maxloop + 1] < 10) /***** still too close ****/
                {
                  warn(81);
                  upbutton(TRUE);
                } else {
                  if ((c[charup].condition[1]) || (c[charup].traiter) || (c[charup].condition[29]) || (c[charup].condition[2]) || (c[charup].condition[6])) { /***** bad condition ****/
                    warn(83);
                    upbutton(TRUE);
                  } else if (question3((StringPtr) "Embrace Cowardice", (StringPtr) "Stay and Fight") == 2) /**** ask if they want to flee from battle ****/
                  {
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
                  }
                }
                break;

              case 'g': /********** g ** guard *******************/
                buttonrect.top = 326 + downshift;
                buttonrect.bottom = buttonrect.top + 18;
                buttonrect.left = 364 + leftshift;
                buttonrect.right = buttonrect.left + 49;
                if (Rand(100) < 50)
                  sound(10121);
                else
                  sound(10123);
                downbutton(TRUE);
                c[charup].guarding = TRUE;
                getup(FALSE);
                break;

              case 'b': /********** b ** bandage *******************/
                buttonrect.top = 386 + downshift;
                buttonrect.bottom = buttonrect.top + 18;
                buttonrect.left = 364 + leftshift;
                buttonrect.right = buttonrect.left + 60;
                downbutton(TRUE);

                if (!canundo) /**** cant bandage ****/
                {
                  warn(84);
                  upbutton(TRUE);
                } else {
                  flashmessage((StringPtr) "Select character to bandage.", 30, 100, -1, 10105);
                  SetPort(GetWindowPort(screen));
                  getchoice(0, 0, TRUE);
                  SetPort(GetWindowPort(look));
                  flashmessage((StringPtr) "", 30, 100, -1, 0);
                  SetPort(GetWindowPort(screen));
                  for (t = 0; t <= charnum; t++) {
                    if (track[t]) {
                      c[t].bleeding = FALSE;
                      updatepictbox(t, TRUE, 0);
                    }
                  }
                  getup(FALSE);
                }
                break;

              case 'r': /************** r ** reveal friendly **********/
                buttonrect.top = 346 + downshift;
                buttonrect.bottom = buttonrect.top + 18;
                buttonrect.left = 364 + leftshift;
                buttonrect.right = buttonrect.left + 102;
                downbutton(TRUE);
                showrange(0);
                centerfield(5 + (2 * screensize), 5 + screensize);
                FlushEvents(everyEvent, 0);
                upbutton(TRUE);
                goto backfromrange;
                break;

              case 'd': /********** d ** delay *******************/

                if (c[charup].movement == c[charup].movementmax) {
                  c[charup].attacks -= (c[charup].normattacks + c[charup].attackbonus);
                  buttonrect.top = 326 + downshift;
                  buttonrect.bottom = buttonrect.top + 18;
                  buttonrect.left = 417 + leftshift;
                  buttonrect.right = buttonrect.left + 49;
                  downbutton(TRUE);
                  temp = q[up];
                  for (ttt = up; ttt < maxloop; ttt++) {
                    q[ttt] = q[ttt + 1];
                    if ((q[ttt] <= charnum) && (q[ttt] > -1))
                      c[q[ttt]].position--;
                  }
                  q[maxloopminus] = temp;
                  c[temp].position = maxloopminus;
                  up--;
                  getup(TRUE);
                } else {
                  warn(59);
                  SetPort(GetWindowPort(screen));
                  RGBBackColor(&greycolor);
                }
                break;

              case 'm': /************ m ** center on cursor ******************/
                buttonrect.top = 346 + downshift;
                buttonrect.bottom = buttonrect.top + 18;
                buttonrect.left = 470 + leftshift;
                buttonrect.right = buttonrect.left + 50;
                downbutton(TRUE);
                centerfield((point.h) / 32, (point.v) / 32);
                upbutton(TRUE);
                break;

              case 'n': /************ n ** center on next ******************/
                buttonrect.top = 386 + downshift;
                buttonrect.bottom = buttonrect.top + 18;
                buttonrect.left = 428 + leftshift;
                buttonrect.right = buttonrect.left + 38;
                downbutton(TRUE);
                targetrect = buttonrect;
                centerstage(1);
                buttonrect = targetrect;
                upbutton(TRUE);
                break;

              case 'p': /************ p ** center on previous ******************/
                buttonrect.top = 386 + downshift;
                buttonrect.bottom = buttonrect.top + 18;
                buttonrect.left = 470 + leftshift;
                buttonrect.right = buttonrect.left + 50;
                targetrect = buttonrect;
                downbutton(TRUE);
                centerstage(-1);
                buttonrect = targetrect;
                upbutton(TRUE);
                break;

              case 'c': /**************** center on charup **************/
                buttonrect.top = 366 + downshift;
                buttonrect.bottom = buttonrect.top + 18;
                buttonrect.left = 470 + leftshift;
                buttonrect.right = buttonrect.left + 50;
                targetrect = buttonrect;
                sound(147);
                downbutton(TRUE);
                aimindex = up;
                centerstage(0);
                buttonrect = targetrect;
                upbutton(TRUE);
                break;

              case 'f': /**** finish ***/
                buttonrect.top = 326 + downshift;
                buttonrect.bottom = buttonrect.top + 18;
                buttonrect.left = 470 + leftshift;
                buttonrect.right = buttonrect.left + 50;
                downbutton(TRUE);
                c[charup].movement = c[charup].guarding = 0;
                getup(FALSE);
                break;

              case 'w': /**** weapon ***/
                buttonrect.top = 326 + downshift;
                buttonrect.bottom = buttonrect.top + 18;
                buttonrect.left = 522 + leftshift;
                buttonrect.right = buttonrect.left + 44;
                downbutton(TRUE);
                theControl = melee;
                break;

              case 't': /**** target ***/
                buttonrect.top = 346 + downshift;
                buttonrect.bottom = buttonrect.top + 18;
                buttonrect.left = 522 + leftshift;
                buttonrect.right = buttonrect.left + 44;
                downbutton(TRUE);
                theControl = combatitem;
                break;
            }
          }

          if (whichset) /********* fast spells *********/
          {
            if (charup < 6) {
              castcaste = definespells[charup][whichset - 1][0];
              castlevel = definespells[charup][whichset - 1][1];
              castnum = definespells[charup][whichset - 1][2];
              powerlevel = definespells[charup][whichset - 1][3];
              loadspell(castcaste, castlevel, castnum);
              c[charup].spellscast++;
              memoryspell = TRUE;
            }

            if (gTheEvent.modifiers & cmdKey) /*** use ***/
            {
              if (abs(spellinfo.cost * powerlevel) >= c[charup].spellpoints)
                theControl = castspellsbut;
              else {

                if ((incombat) && (!spellinfo.incombat))
                  sound(143);
                else if ((!incombat) && (!spellinfo.incamp))
                  sound(143);
                else if (!powerlevel)
                  sound(143);
                else {
                  c[charup].spellpoints -= abs(spellinfo.cost * powerlevel);
                  lastspell[charup][0] = castlevel;
                  lastspell[charup][1] = castnum;
                  oldpowerlevel[charup] = powerlevel;
                  select[charup] = fastspell = TRUE;
                  theControl = castspellsbut;
                }
              }
            } else /***** display name only *********/
            {
              SetPort(GetWindowPort(screen));
              MoveTo(330 + leftshift, 315 + downshift);
              TextMode(1);
              ForeColor(yellowColor);
              itemRect.left = 324 + leftshift;
              itemRect.right = 640 + leftshift;
              itemRect.top = 300 + downshift;
              itemRect.bottom = 320 + downshift;
              EraseRect(&itemRect);
              MoveTo(330 + leftshift, 315 + downshift);
              GetIndString(myString, 1000 * (castcaste + 1) + castlevel, castnum + 1);
              if (powerlevel) {
                PtoCstr(myString);
                MyrDrawCString((Ptr)myString);
                MyrDrawCString("  X ");
                MyrNumToString(powerlevel, myString);
                MyrDrawCString((Ptr)myString);
              } else {
                MyrDrawCString("Undefined Spell");
                MoveTo(330 + leftshift, 315 + downshift);
                sound(-143);
              }
            }
          }

          if ((theControl) && (q[up] < 9)) {
            if (killparty > charnum)
              partyloss(0);
            combatchoice();
            if (c[charup].attacks < 2)
              getup(FALSE);
          }
          break;

        case mouseDown:

          if ((gTheEvent.modifiers & cmdKey) && (PtInRect(point, &lookrect))) {
            combatinfo(charup);
            goto backfromrange;
          }

          thePart = FindWindow(gTheEvent.where, &whichWindow);
          switch (thePart) {
            case inMenuBar:
              compactheap();
              menuChoice = MenuSelect(gTheEvent.where);
              HandleMenuChoice();
              if (revertgame)
                return;
              break;

            case inContent:
              if ((q[up] < 9) && (q[up] > -1)) {
                point = gTheEvent.where;
                GlobalToLocal(&(point));
                thePart = FindControl(point, screen, &theControl);
                if (PtInRect(point, &buttons)) {
                  key = 0;
                  /************* turn **********************/
                  buttonrect.top = 367 + downshift;
                  buttonrect.bottom = buttonrect.top + 34;
                  buttonrect.left = 522 + leftshift;
                  buttonrect.right = buttonrect.left + 34;
                  if (PtInRect(point, &buttonrect))
                    theControl = turn;

                  /************ g ** guard ******************/
                  buttonrect.top = 326 + downshift;
                  buttonrect.bottom = buttonrect.top + 18;
                  buttonrect.left = 364 + leftshift;
                  buttonrect.right = buttonrect.left + 49;
                  if (PtInRect(point, &buttonrect))
                    key = 'g';

                  /************* f *** finish ****************/
                  buttonrect.left = 470 + leftshift;
                  buttonrect.right = buttonrect.left + 50 + downshift;
                  if (PtInRect(point, &buttonrect))
                    key = 'f';

                  /********** d ** delay *********************/
                  buttonrect.left = 417 + leftshift;
                  buttonrect.right = buttonrect.left + 49;
                  if (PtInRect(point, &buttonrect))
                    key = 'd';

                  /************ r ** reveal friendly ***********/
                  buttonrect.top = 346 + downshift;
                  buttonrect.bottom = buttonrect.top + 18;
                  buttonrect.left = 364 + leftshift;
                  buttonrect.right = buttonrect.left + 102;
                  if (PtInRect(point, &buttonrect))
                    key = 'r';

                  /*** n *** center on next ***/
                  buttonrect.top = 386 + downshift;
                  buttonrect.bottom = buttonrect.top + 18;
                  buttonrect.left = 428 + leftshift;
                  buttonrect.right = buttonrect.left + 38;
                  if (PtInRect(point, &buttonrect))
                    key = 'n';

                  /************ b ** bandage ******************/
                  buttonrect.top = 386 + downshift;
                  buttonrect.bottom = buttonrect.top + 18;
                  buttonrect.left = 364 + leftshift;
                  buttonrect.right = buttonrect.left + 60;
                  if (PtInRect(point, &buttonrect))
                    key = 'b';

                  /*** p *** center on previous ************/
                  buttonrect.left = 470 + leftshift;
                  buttonrect.right = buttonrect.left + 50;
                  if (PtInRect(point, &buttonrect))
                    key = 'p';

                  /************ c ** center ******************/
                  buttonrect.top = 366 + downshift;
                  buttonrect.bottom = buttonrect.top + 18;
                  if (PtInRect(point, &buttonrect))
                    key = 'c';

                  /************ e ** escape ******************/
                  buttonrect.left = 364 + leftshift;
                  buttonrect.right = buttonrect.left + 54;
                  if (PtInRect(point, &buttonrect))
                    key = 'e';

                  /************ u ** undo ******************/
                  buttonrect.left = 422 + leftshift;
                  buttonrect.right = buttonrect.left + 44;
                  if (PtInRect(point, &buttonrect))
                    key = 'u';

                  /************ t ** target ******************/
                  buttonrect.top = 346 + downshift;
                  buttonrect.bottom = buttonrect.top + 18;
                  buttonrect.left = 522 + leftshift;
                  buttonrect.right = buttonrect.left + 44;
                  if (PtInRect(point, &buttonrect))
                    key = 't';

                  /************ w ** weapon ******************/
                  buttonrect.top = 326 + downshift;
                  buttonrect.bottom = buttonrect.top + 18;
                  buttonrect.left = 522 + leftshift;
                  buttonrect.right = buttonrect.left + 44;
                  if (PtInRect(point, &buttonrect))
                    key = 'w';

                  if (key)
                    goto gotkey;
                }

              jumpposs:
                combatchoice();
                if (!incombat)
                  return;

                emptyque(); /***** empty macro que ****/
                if (c[charup].attacks < 2)
                  getup(FALSE);
                else if (!inwindow(charup))
                  centerstage(0);
              }
              break;
          }
          break;

        case osEvt:
          if ((gTheEvent.message >> 24) == suspendResumeMessage) {
            if (hide) {
              compactheap();
              hide = FALSE;
              updatemain(TRUE, 0);
              combatupdate2(charup);
              centerfield(pos[charup][0], pos[charup][1]);
              updatecontrols();
              FlushEvents(everyEvent, 0);
            } else
              hide = TRUE;
          }
          break;
      }
    }
  }
}
