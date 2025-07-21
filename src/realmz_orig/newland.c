#include "prototypes.h"
#include "realmzbuild.h"
#include "variables.h"

short regscen_pc(void);
short regscen_pc_custom(void);
extern Boolean tagger;

/******************************* newland ****************************/
short newland(long x, long y, short mode, short modecode, short data) {
  struct door holddoorforbattle;
  short ttt, lr, ud, start, stop, posx, posy;
  static short holddoorposforbattle;
  float multiply = 0;
  Rect goodrect;
  short oldview, thieftry = 0;
  short encountid, fumloop, loop, id, t, tt, tempt, encid, enc2id;
  short code, temp, temp1, temp2, temp4, itemcount, branchdoorcode, seconddoor = 0;
  FILE* fp = NULL;
  Boolean fastsound, dosound, remove, tempencountflag, noseconddoor = 0;
  struct door tempdoor, holddoor;
  struct encount tempenc;
  PicHandle picture = NIL;
  int tempholdovername; //** fantasoft v7.1

  memoryspell = collideflag = 0;

  if (mode == -1)
    goto startover; /***** monster macro *****/

  enctry = encountflag = remove = FALSE;

  if (mode == 1) {
    infodoor.landx = lookx + partyx;
    infodoor.landy = looky + partyy;
    infodoor.landid = landlevel;

    loaddoor2(modecode);
    goto startover;
  }

  if (mode == 2) /****** trigger simple encounter *****/
  {
    infodoor.landx = lookx + partyx;
    infodoor.landy = looky + partyy;
    infodoor.landid = landlevel;

    for (t = 0; t < 8; t++)
      infodoor.code[t] = infodoor.id[t] = 0;

    infodoor.code[0] = 4;
    infodoor.id[0] = modecode;
    goto startover;
  }

  if (mode == 3) /****** trigger complex encounter *****/
  {
    infodoor.landx = lookx + partyx;
    infodoor.landy = looky + partyy;
    infodoor.landid = landlevel;

    for (t = 0; t < 8; t++)
      infodoor.code[t] = infodoor.id[t] = 0;

    infodoor.code[0] = 5;
    infodoor.id[0] = modecode;
    goto startover;
  }

recheck:

  if (!indung)
    doorid = landlevel * 10000 + x + y * 100;
  else
    doorid = dunglevel * 10000 + x + y * 100;

  for (t = 0; t < 100; t++) {
    if (door[t].doorid == doorid) {
      infodoor = door[t];
      doornum = t;
      goto moveon;
    }
  }
  return (FALSE);

moveon:

  if (infodoor.percent < 1)
    return (FALSE); /****** disabled door ****/
  if (Rand(100) > infodoor.percent)
    return (FALSE);

startover:

  for (t = 0; t < 8; t++) {

  startcode:
    code = infodoor.code[t];
    id = infodoor.id[t];
    MyrDump("newland (%d,%d)\n", code, id);

    if ((code < 0) && (code != -14) && (code != -23)) {
      gosub = TRUE;
      code *= -1; //***** bring it back in line with Positive Code For Switch
    } else if (stackindex == 0)
      gosub = FALSE;

    switch (code) {

      case 111: //***** return from GOSUB
        if (stackindex > 0) /*** get last AP from stack and continue scripts ****/
        {
          t = 1 + pulloffstack();
          goto startcode;
        }
        break;

      case 112: //***** POP Stack
        if (stackindex > 0)
          stackindex--; //*** just decrease the stack index
        break;

      case 76: /***** Increment � Decrement Quest Value *****/

        loadextracode(id);
        if (extracode[0] < 100) {
          temp = quest[extracode[0]];
          temp += extracode[1];
          temp = pin(temp, -127, 127);
          quest[extracode[0]] = temp;
        }

        if (extracode[3]) /**** looks to have an autobranch ***/
        {
          if (quest[extracode[0]] >= extracode[3]) {
            switch (extracode[2]) {
              case 1: //**** X-AP
                if (gosub)
                  pushonstack(infodoor, t);
                loaddoor2(extracode[4]);
                t = -1;
                goto startover;
                break;

              case 2: //**** Simple
                if (gosub)
                  pushonstack(infodoor, t);
                id = extracode[4];
                goto type1new;
                break;

              case 3: //**** Complex
                if (gosub)
                  pushonstack(infodoor, t);
                id = extracode[4];
                goto type2new;
                break;
            }
          }
        }

        break;

      case 77: /***** Branch on Quest Value *****/

        loadextracode(id);
        if (quest[extracode[0]] >= extracode[1])
          reply = TRUE;
        else
          reply = FALSE;

        if ((!reply && extracode[3]) || (reply && extracode[4])) {

          switch (extracode[2]) {
            case 0: //**** X-AP
              if (gosub)
                pushonstack(infodoor, t);
              loaddoor2(extracode[3 + reply]);
              t = -1;
              goto startover;
              break;

            case 1: //**** Simple
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[3 + reply];
              goto type1new;
              break;

            case 2: //**** Complex
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[3 + reply];
              goto type2new;
              break;
          }
        }

        break;

      case 78: /***** Branch on Tile Parameters *****/

        loadextracode(id);

        temp = field[partyx + lookx][partyy + looky];
        MyrBitClrShort(&temp, 2); //*** clear path marker ****/
        MyrBitClrShort(&temp, 1); //*** clear note marker ****/

        if (temp > 999)
          temp -= 1000; //*** wipe out door marker
        if (temp > 999)
          temp -= 1000;
        if (temp > 999)
          temp -= 1000;
        if (temp < -999)
          temp += 1000;
        if (temp < -999)
          temp += 1000;
        if (temp < -999)
          temp += 1000;

        holdstatbase = mapstats[temp];
        reply = FALSE;

        switch (extracode[0]) /**** test for parameter *****/
        {
          case 1:
            if (holdstatbase.shore)
              reply = TRUE;
            break;

          case 2:
            if (holdstatbase.needboad)
              reply = TRUE;
            break;

          case 3:
            if (holdstatbase.ispath)
              reply = TRUE;
            break;

          case 4:
            if (holdstatbase.los)
              reply = TRUE;
            break;

          case 5:
            if (holdstatbase.flyfloat)
              reply = TRUE;
            break;

          case 6:
            if (holdstatbase.forest)
              reply = TRUE;
            break;

          case 7: /*** specific tile id ****/
            if (temp == extracode[1])
              reply = TRUE;
            break;
        }

        if ((!reply && extracode[3]) || (reply && extracode[4])) {

          switch (extracode[2]) {
            case 0: //**** X-AP
              if (gosub)
                pushonstack(infodoor, t);
              loaddoor2(extracode[3 + reply]);
              t = -1;
              goto startover;
              break;

            case 1: //**** Simple
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[3 + reply];
              goto type1new;
              break;

            case 2: //**** Complex
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[3 + reply];
              goto type2new;
              break;
          }
        }

        break;

      case 119: /***** Revive NPC / Party after combat *****/

        if (killparty > charnum) {
          for (loop = 0; loop <= charnum; loop++) {
            c[loop].stamina = 1;
            c[loop].condition[25] = 0;
            killparty = 0;
          }
          return (-1);
        } else /**** NPC revival ******/
        {
          monster[macromonster].stamina = 1;
          monster[macromonster].traiter = 0;
        }

        break;

      case 120: /***** Alter NPC or Monster During Combat *****/

        loadextracode(id);
        temp = extracode[2]; /********* how many to alter **********/

        for (tt = 0; tt < nummon; tt++) {
          if (temp) {
            if (monster[tt].name == extracode[1]) {
              if (((monster[tt].cansum == -1) && (extracode[0] == 1)) || ((monster[tt].cansum != -1) && (extracode[0] == 2))) {

                if (extracode[3] != -1) /********** alter icon *********/
                {
                  monster[tt].iconid = extracode[3];
                  temp--;
                } else if ((extracode[4] != -1) || (monster[tt].traiter != extracode[4])) /********** alter traitor *********/
                {
                  monster[tt].traiter = extracode[4];
                  numenemy--;
                  temp--;
                }
              }
            }
          }
        }

        centerfield(5 + (2 * screensize), 5 + screensize);

        break;

      case 121: /***** de-animate lower level undead *****/

        if (incombat) {
          loadextracode(id);

          for (tt = 0; tt < maxmon; tt++) {
            if ((monster[tt].type[1]) && (!monster[tt].type[5]) && (monster[tt].stamina > 0)) {
              destroymonster(tt);
            }
          }
        }
        break;

      case 122: /***** cause fumble *****/

        loadextracode(id);

        if ((incombat) && (!inspell) && (allowfumble) && (physical) && (q[up] < 10)) // Fantasoft v7.1
        {
          if (extracode[1])
            sound(extracode[1]);
          if (extracode[0]) /**** text message ***/
          {
            SetRect(&itemRect, 0, 321, 308, 460);
            SetPort(GetWindowPort(screen));
            pict(203, itemRect);
            textbox(-1, extracode[0], 0, 0, textrect);
          }

          if ((q[up] < 9) && (q[up] > -1)) {
            if (c[charup].armor[2]) /**** fumble weapon ****/
            {
              for (fumloop = 0; fumloop <= c[charup].numitems; fumloop++) {
                if (c[charup].items[fumloop].id == c[charup].armor[2]) {
                  if (removeitem(charup, fumloop, FALSE, FALSE)) {
                    sound(-10121);
                    sound(-10123);
                    fumque[fumtotal++] = c[charup].items[fumloop].id;
                    dropitem(charup, c[charup].items[fumloop].id, fumloop, TRUE, FALSE);
                    c[charup].armor[2] = c[charup].weaponnum = 0;
                    combatupdate2(charup);
                    goto donefumble;
                  }
                }
              }
            }
            return (0);
          } else if (monster[monsterup].weapon) {
            monster[monsterup].weapon = 0;
            sound(-10121);
            sound(-655);
            combatupdate2(monsterup);
            showresults(monsterup, -46, monsterup);
          } else
            return (0);
        }

      donefumble:

        break;

      case 123: /***** cause Route *****/

        if (incombat) {
          loadextracode(id);
          for (ttt = 0; ttt < 5; ttt++) {
            if (extracode[ttt]) {
              for (tt = 0; tt < maxmon; tt++) {
                if (monsternameid[tt] == extracode[ttt]) {
                  if (!doingque) {
                    if (monster[tt].traiter == monster[macromonster].traiter) {
                      monster[tt].condition[0] = -1;
                      monster[tt].surrenderpercent = 50;
                    }
                  } else if (monster[tt].traiter == todoque[abs(data) - 1].traiter) {
                    monster[tt].condition[0] = -1;
                    monster[tt].surrenderpercent = 50;
                  }
                }
              }
            }
          }
        }
        break;

      case 124: /***** Spawn *****/

        if (incombat) {
          loadextracode(id);
          spellinfo.spelllook2 = 5;
          if (nummon < maxmon) {
            goodrect.top = goodrect.left = 7;
            goodrect.right = goodrect.bottom = 83;
            getfilename("Data MD");

            if (monsterset) {
              MyrNumToString(monsterset, myString);
              // PtoCstr(myString);
              strcat((StringPtr)filename, myString);
            }

            if (extracode[2] < 0)
              extracode[2] = Rand(abs(extracode[2]));

            fp = MyrFopen(filename, "rb");

            for (tt = 0; tt < extracode[2]; tt++) {
              if (nummon < maxmon) {
                fseek(fp, extracode[1] * sizeof monpick, SEEK_SET);
                /* !MYRIAD 12/10/99 Because fseek can be greater than the end of the file (and fread can fail)*/
                if (fread(&monpick, sizeof monpick, 1, fp) == 1)
                  CvtMonsterToPc(&monpick);
                /* !MYRIAD 12/10/99 If not read, keeps the previous value of monpick */
                monpick.target = start = -1;
                stop = 1;
                size = monpick.size;
                monpick.attacknum = 0;

                switch (extracode[4]) {
                  case 0: //*** default to old style rules
                    if (doingque)
                      buildmonster(todoque[abs(data) - 1].traiter);
                    else if (!battlemacro)
                      buildmonster(monster[macromonster].traiter);
                    else
                      buildmonster(-1);
                    break;

                  default:
                    buildmonster(extracode[4]);
                    break;
                }

                monster[nummon] = monpick;
                monsternameid[nummon] = extracode[1];

              tryagain3:

                if (doingque) {
                  posx = todoque[abs(data) - 1].x - fieldx;
                  posy = todoque[abs(data) - 1].y - fieldy;
                } else {
                  posx = monpos[macromonster][0];
                  posy = monpos[macromonster][1];
                }
                for (ud = start; ud < stop; ud++) {
                  for (lr = start; lr < stop; lr++) {
                    point.h = lr + fieldx + posx;
                    point.v = ud + fieldy + posy;

                    if (PtInRect(point, &goodrect)) {

                      monpos[nummon][0] = lr + posx;
                      monpos[nummon][1] = ud + posy;

                      placemonster(monpos[nummon][0], monpos[nummon][1], nummon);

                      if (monster[nummon].underneath[1][1] > 999) {
                        if (((mapstats[monster[nummon].underneath[1][1] - 1000].solid) && (!monster[nummon].size)) || (mapstats[monster[nummon].underneath[1][1] - 1000].solid == 2))
                          goto yep2;
                      } else
                        goto yep2;

                      if ((size == 1) || (size == 3)) {
                        if (monster[nummon].underneath[1][0] > 999) {
                          if (mapstats[monster[nummon].underneath[1][0] - 1000].solid > 1)
                            goto yep2;
                        } else
                          goto yep2;
                      }

                      if (size == 3) {
                        if (monster[nummon].underneath[0][0] > 999) {
                          if (mapstats[monster[nummon].underneath[0][0] - 1000].solid > 1)
                            goto yep2;
                        } else
                          goto yep2;
                      }

                      if (size > 1) {
                        if (monster[nummon].underneath[0][1] > 999) {
                          if (mapstats[monster[nummon].underneath[0][1] - 1000].solid > 1)
                            goto yep2;
                        } else
                          goto yep2;
                      }

                      goto good3;

                    yep2:
                      bodyfield(nummon + 10);
                    }
                  }
                }

                start--;
                stop++;

                goto tryagain3;

              good3:
                if (extracode[3])
                  sound(extracode[3]);
                showresults(nummon + 10, -12, nummon + 10);
                spelleffect(nummon + 10, 0);
                if (monster[nummon].traiter)
                  numenemy++;

                for (temp = maxloopminus; temp > 0; temp--) {
                  if (q[temp] == -1) {
                    q[temp] = nummon + 10;
                    goto gotit;
                  }
                }
              gotit:
                nummon++;
              }
            }
            fclose(fp);
          }
        }
        break;

      case 125: /***** destroy related monsters *****/

        if (incombat) {
          loadextracode(id);
          if (!extracode[1])
            extracode[1] = 100;
          temp = 0;
          for (tt = 0; tt < maxmon; tt++) {
            if (((monster[tt].traiter) || (extracode[4])) && (monster[tt].name == extracode[0]) && (temp < extracode[1]) && (monster[tt].stamina > 0)) {
              temp++;
              destroymonster(tt);
            }
          }
        }
        break;

      case 126: /***** Battle combatround macro *****/

        if (battle.battlemacro > 0)
          return (0); /***** not a valid code, stuck in the old days ****/

        loadextracode(id);

        if (extracode[0] == 1) {
          if (Rand(100) > extracode[1])
            return (0);
        } else if ((!extracode[0]) && ((combatround - 1) != extracode[1]))
          return (0);

        if (extracode[2] != 1)
          battle.battlemacro = 0; //** fantasoft v7.1 /**** single time macro ****/

        /***** activate standard macro *****/

        if (extracode[2] == 2)
          loaddoor2(randrange(extracode[3], extracode[4])); /**** branch to random macro *****/
        else
          loaddoor2(extracode[3]);

        t = -1;
        goto startover;
        break;

      case 127: /***** Continue if Monster Present *****/

        reply = FALSE;
        for (tt = 0; tt < nummon; tt++) {
          if ((monster[tt].name == id) && (monster[tt].stamina > 0))
            reply = TRUE;
        }

        if (!reply)
          return (0);

        break;

      case 108: /***** Alter Selected Character *****/

        loadextracode(id);

        for (tt = 0; tt <= charnum; tt++) {
          if (track[tt]) {
            switch (extracode[0]) {
              case 1: //****** alter attacks/round
                c[tt].attackbonus += extracode[1];
                if (c[tt].attackbonus < 0)
                  c[tt].attackbonus = 0;
                break;

              case 2: //****** alter spells/round
                if (c[tt].maxspellsattacks) {
                  c[tt].maxspellsattacks += extracode[1];
                  if (c[tt].maxspellsattacks < 1)
                    c[tt].maxspellsattacks = 1;
                }
                break;

              case 3: //****** alter movement
                c[tt].movementmax += extracode[1];
                if (c[tt].movementmax < 3)
                  c[tt].movementmax = 3;
                break;

              case 4: //****** alter damage
                c[tt].damage += extracode[1];
                if (c[tt].damage < 0)
                  c[tt].damage = 0;
                break;

              case 5: //****** alter spellpoints
                if (c[tt].spellpointsmax) {
                  c[tt].spellpointsmax += extracode[1];
                  if (c[tt].spellpointsmax < 0)
                    c[tt].spellpointsmax = 0;
                }
                break;

              case 6: //****** alter hand 2 hand
                if (c[tt].handtohand) {
                  c[tt].handtohand += extracode[1];
                  if (c[tt].handtohand < 1)
                    c[tt].handtohand = 1;
                }
                break;

              case 7: //****** alter stamina
                c[tt].staminamax += extracode[1];
                if (c[tt].staminamax < 2)
                  c[tt].staminamax = 2;
                break;

              case 8: //****** alter AR
                c[tt].ac += extracode[1];
                if (c[tt].ac < 0)
                  c[tt].ac = 0;
                break;

              case 9: //****** alter ToHit
                c[tt].tohit += extracode[1];
                if (c[tt].tohit < 2)
                  c[tt].tohit = 2;
                break;

              case 10: //****** alter ProJo ToHit
                c[tt].missile += extracode[1];
                if (c[tt].missile < 2)
                  c[tt].missile = 2;
                break;

              case 11: //****** alter Magic Resistance
                c[tt].magres += extracode[1];
                if (c[tt].magres < 0)
                  c[tt].magres = 0;
                break;

              case 12: //****** alter Prestigue
                c[tt].prestigepenelty += -(extracode[1]);
                break;
            }
          }
          updatechar(tt, 3);
        }
        break;

      case 107: /************ improved selective battle ***********/

        for (tt = 0; tt < 8; tt++) /**** save old door codes ****/
        {
          holddoorforbattle.code[tt] = infodoor.code[tt];
          holddoorforbattle.id[tt] = infodoor.id[tt];
        }
        holddoorposforbattle = t;

        loadextracode(id);
        if (extracode[2])
          sound(extracode[2]);
        if (extracode[3])
          textbox(-1, extracode[3], 0, 0, textrect);

        if (!extracode[1])
          battlenum = extracode[0];
        else
          battlenum = randrange(extracode[0], extracode[1]);

        branchdoorcode = extracode[4];

        combat(0, 1);

        if (revertgame)
          return (0);

        lastpix = -1;
        if (indung)
          loadland(dunglevel, TRUE);
        else
          loadland(landlevel, TRUE);

        needupdate = FALSE;

        for (tt = 0; tt < 8; tt++) /**** restore old door codes ****/
        {
          infodoor.code[tt] = holddoorforbattle.code[tt];
          infodoor.id[tt] = holddoorforbattle.id[tt];
        }
        t = holddoorposforbattle;

        if (!coward)
          booty(0);
        else {
          loaddoor2(branchdoorcode);
          t = -1;
        }
        for (tt = 0; tt <= charnum; tt++)
          c[t].inbattle = TRUE;
        updatemain(FALSE, 0);

        if (coward)
          goto startover;
        break;

      case 100: /***** End Battle *****/

        holddoorposforbattle = 8;

        killmon = numenemy;
        extracode[4] = 5;
        goto battledone;

        break;

      case 101: /***** Back Up Party *****/

        if (!indung) {
          centerpict();
          moveparty(-1);
          return (TRUE);
        }

        break;

      case 102: /***** Level Up Picked Characters *****/

        for (loop = 0; loop <= charnum; loop++) {
          if (track[loop])
            c[loop].exp = 1;
        }

        for (tt = 0; tt <= charnum; tt++)
          c[t].inbattle = TRUE;
        totalexp = 1;
        nummon = incombat = 0;
        booty(0);
        updatemain(FALSE, -1 + needupdate);

        break;

      case 103: /***** test/set Boat/Camp Status *****/

        loadextracode(id);
        if (((extracode[0] == 1) && (!inboat)) || ((extracode[0] == 2) && (inboat))) {
          t = 8;
        }

        if (((extracode[1] == 1) && (!incamp)) || ((extracode[1] == 2) && (incamp))) {
          t = 8;
        }

        if (extracode[2] == 1) {
          inboat = TRUE;
          centerpict();
        }

        if (extracode[2] == 2) {
          inboat = FALSE;
          centerpict();
        }

        break;

      case 104: /***** Set Encounter Status *****/
        canencounter = id;
        break;

      case 105: /***** Activate - Disable Allies *****/
        suspendallies = id;
        break;

      case 106: /***** Set Darkland Status *****/
        loadextracode(id);

        if ((extracode[1]) && (randlevel.isdark == extracode[0] - 1))
          return (0); /*** discontinue codes ***/

        randlevel.isdark = extracode[0] - 1;
        centerpict();
        break;

      case 99: /***** get scenario registration *****/

        if (currentscenario > topfantasoftsceanrio) //*** v7.1 Begin   both mac and PC codes for custom scenario use same scheme
        {
          if (!regscen_pc_custom())
            moveparty(-1);
          return (0);
        }

#ifdef PC
        if (currentscenario > topfantasoftsceanrio) {
          if (!regscen_pc_custom())
            moveparty(-1);
          return (0);
        } else if (!regscen_pc()) {
          moveparty(-1);
          return (0);
        }
#else
        if (!regscen()) {
          moveparty(-1);
          return (0);
        }
#endif
        //*** v7.1 End
        break;

      case 98: /****** require registered game ********/

#if !development
        if (!doreg()) {
          flashmessage((StringPtr) "Sorry, you cannot enter this area until you have registered.", 40, 50, 0, 10105);
          moveparty(-1);
          if (id)
            exit(0);
          return (0);
        }
#endif
        break;

      case 97: /****** Allow Full Map ********/

        if (!multiview)
          warn(96);
        multiview = TRUE;

        if (viewtype == 1)
          UpdateWindow(TRUE);

        break;

      case 96: /****** Require 3D Map ********/

        if (multiview)
          warn(97);

        multiview = FALSE;
        if (viewtype == -1) {
          viewtype = 1;
          UpdateWindow(FALSE);
        } else {
          viewtype = 1;
          UpdateWindow(TRUE);
        }

        break;

      case 95: /****** Change Look Diretion ********/

        head = id;

        if ((head < 1) || (head > 4))
          head = Rand(4);

        updatewalls(floorx - 10, floory - 10);

        break;

      case 94: /****** Turn Compass Off ********/

        if (updatedir)
          warn(99);

        updatedir = 0;

        updatewalls(floorx - 10, floory - 10);

        break;

      case 93: /****** Turn Compass on ********/

        if (!updatedir)
          warn(98);

        updatedir = 1;

        updatewalls(floorx - 10, floory - 10);

        break;

      case 92: /****** Alter Size of Random Rect ********/

        loadextracode(id);

        temp = indung;

        if (indung)
          saveland(dunglevel);
        else
          saveland(landlevel);

        templong = extracode[0];

        indung = extracode[2];

        loadland(templong, 0);
        temp1 = extracode[1];

        randlevel.percent[temp1] += extracode[3];

        temp2 = extracode[4];

        loadextracode(id + 1); /**** next code has additional info ****/

#ifdef PC
        if (temp1 < 0 || temp1 >= 20) {
          AcamErreur("Rand rect overload");
          temp1 = 0;
        }
#endif
        switch (temp2) {
          case -1: /**** no change called for ***/

            break;

          case 0: /**** set absolute size ***/
            randlevel.randrect[temp1].left = extracode[0];
            randlevel.randrect[temp1].right = extracode[1];
            randlevel.randrect[temp1].top = extracode[2];
            randlevel.randrect[temp1].bottom = extracode[3];
            break;

          case 1: /**** offset rect ***/
            randlevel.randrect[temp1].left += extracode[0];
            randlevel.randrect[temp1].right += extracode[0];
            randlevel.randrect[temp1].top += extracode[1];
            randlevel.randrect[temp1].bottom += extracode[1];
            break;

          case 2: /**** warp rect ***/
            randlevel.randrect[temp1].left += extracode[0];
            randlevel.randrect[temp1].right += extracode[1];
            randlevel.randrect[temp1].top += extracode[2];
            randlevel.randrect[temp1].bottom += extracode[3];
            break;
        }

        saveland(templong);

        indung = temp;

        if (!indung)
          loadland(landlevel, 0);
        else
          loadland(dunglevel, 0);

        break;

      case 91: /***** Drop all equipment ****/
        for (loop = 0; loop <= charnum; loop++) {
          itemcount = -1;
          while (c[loop].numitems) {
            itemcount++;
            removeitem(loop, itemcount, FALSE, TRUE);
            dropitem(loop, c[loop].items[1].id, 0, 141, TRUE);
          }
        }

        shortupdate(0);
        break;

      case 90: /***** Take away Victory ****/
        loadextracode(id);
        switch (extracode[1]) {
          case 1: /*** take from picked ****/
            for (tt = 0; tt <= charnum; tt++) {
              if (track[t])
                c[tt].exp -= extracode[0];
            }
            break;

          case 2: /*** Spread the loss around ****/
            for (tt = 0; tt <= charnum; tt++) {
              c[tt].exp -= (extracode[0] / (charnum + 1));
            }
            break;

          default: /*** take from each ****/
            for (tt = 0; tt <= charnum; tt++) {
              c[tt].exp -= extracode[0];
            }
            break;
        }
        break;

      case 89: /************ add Allies to party ***********/

      addnpc:

        if (heldover < 20) {
          getfilename("Data MD");

          if (monsterset) {
            MyrNumToString(monsterset, myString);
            // PtoCstr(myString);
            strcat((StringPtr)filename, myString);
          }

          if ((fp = MyrFopen(filename, "rb")) == NULL) {

            if (monsterset) {
              monsterset = 0;
              warn(102);
              scratch(-4);
            } else
              scratch(-5);
          }
          fseek(fp, id * sizeof monpick, SEEK_SET);
          /* !MYRIAD 12/10/99 Because fseek can be greater than the end of the file (and fread can fail)*/
          if (fread(&holdover[heldover], sizeof monpick, 1, fp) == 1)
            CvtMonsterToPc(&holdover[heldover]);
          /* !MYRIAD 12/10/99 If not read, keeps the previous value of monpick */
          fclose(fp);

          holdover[heldover].stamina = holdover[heldover].bonus;
          for (temp = 0; temp < holdover[heldover].hd; temp++)
            holdover[heldover].stamina += Rand(8);
          holdover[heldover].staminamax = holdover[heldover].stamina;
          holdover[heldover].traiter = 0;
          holdover[heldover].lr = holdover[heldover].up = 0;
          holdover[heldover].maxspellpoints = holdover[heldover].spellpoints;

          holdover[heldover].magres += (3 * howhard);
          for (temp = 0; temp < 6; temp++)
            holdover[heldover].save[temp] += 7 * howhard;
          holdover[heldover].ac -= (2 * howhard);
          holdover[heldover].dx += howhard;

          holdover[heldover].staminamax = holdover[heldover].stamina;

          if (holdover[heldover].weapon < 0) /****** get random weapon *******/
          {
            holdover[heldover].weapon = holdover[heldover].items[0] = combatsetup(holdover[heldover].weapon, 0, 0);
          }

          heldover++;
        } else
          warn(108);

        updatenpcmenu();

        break;

      case 88: /************ drop Allies from party ***********/

        for (temp = 0; temp < 19; temp++) {
        checkforduplicates:
          if (holdover[temp].name == id) {
            for (temp2 = temp; temp2 < 19; temp2++) {
              holdover[temp2] = holdover[temp2 + 1];
            }
            heldover--;
            holdover[19] = blankmonster;
            goto checkforduplicates;
          }
        }

        updatenpcmenu();

        break;

      case 87: /************ Branch On Allies In Party ***********/

        loadextracode(id);

        reply = FALSE;

        tempholdovername = extracode[0]; //**** fantasoft v7.1

        for (temp = 0; temp < heldover; temp++)
          if (holdover[temp].name == tempholdovername)
            reply = TRUE; //**** fantasoft v7.1

        if (reply) /**** Allies present ********/
        {
          switch (extracode[1]) {
            case 0:
              if (gosub)
                pushonstack(infodoor, t);
              loaddoor2(extracode[3]);
              t = -1;
              goto startover;
              break;

            case 1:
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[3];
              goto type1new;
              break;

            case 2:
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[3];
              goto type2new;
              break;
          }
        } else {
          if (!extracode[2]) {
            switch (extracode[1]) /****** Allies not present *****/
            {
              case 0:
                if (gosub)
                  pushonstack(infodoor, t);
                loaddoor2(extracode[4]);
                t = -1;
                goto startover;
                break;

              case 1:
                if (gosub)
                  pushonstack(infodoor, t);
                id = extracode[4];
                goto type1new;
                break;

              case 2:
                if (gosub)
                  pushonstack(infodoor, t);
                id = extracode[4];
                goto type2new;
                break;
            }
          } else if (extracode[2] == 2) /******* display string and exit ********/
          {
            textbox(-1, extracode[4], 0, 0, textrect);
            goto getout;
          }
        }
        break;

      case 86: /************ Branch on Misc. ***********/

        reply = FALSE;

        loadextracode(id);

        switch (extracode[0]) {
          case 0: /****** class present *********/
            for (tt = 0; tt <= charnum; tt++) {
              if (extracode[1] < 0) {
                if ((c[tt].caste == abs(extracode[1])) && (track[tt]))
                  reply = TRUE;

              } else if (c[tt].caste == extracode[1])
                reply = TRUE;
            }
            break;

          case 1: /**** race present ****/
            for (tt = 0; tt <= charnum; tt++) {
              if (extracode[1] < 0) {
                if ((c[tt].race == abs(extracode[1])) && (track[tt]))
                  reply = TRUE;

              } else if (c[tt].race == extracode[1])
                reply = TRUE;
            }
            break;

          case 2: /**** gender present ****/
            for (tt = 0; tt <= charnum; tt++) {
              if (extracode[1] < 0) {
                if ((c[tt].gender == abs(extracode[1])) && (track[tt]))
                  reply = TRUE;

              } else if (c[tt].gender == extracode[1])
                reply = TRUE;
            }
            break;

          case 3: /**** inboat ****/
            if (inboat)
              reply = TRUE;
            break;

          case 4: /**** incamp ****/
            if (incamp)
              reply = TRUE;
            break;

          case 5: /**** caste class present ****/
            for (tt = 0; tt <= charnum; tt++) {
              loadprofile(0, c[tt].caste);
              if (extracode[1] < 0) {
                if ((caste.casteclass == abs(extracode[1])) && (track[tt]))
                  reply = TRUE;
              } else if (caste.casteclass == extracode[1])
                reply = TRUE;
            }
            break;

          case 6: /**** race class present ****/
            for (tt = 0; tt <= charnum; tt++) {
              loadprofile(c[tt].race, 0);

              if (extracode[1] < 0) {
                if ((MyrBitTstShort(&races.descriptors, abs(extracode[1]) - 1)) && (track[tt]))
                  reply = TRUE;
              } else if (MyrBitTstShort(&races.descriptors, extracode[1] - 1))
                reply = TRUE;
            }
            break;

          case 7: /**** total party level check ****/

            temp = 0;
            for (tt = 0; tt <= charnum; tt++) {
              temp += c[tt].level;
            }

            if (temp > extracode[1])
              reply = TRUE;
            break;

          case 8: /**** picked PC level check ****/

            temp = 0;
            for (tt = 0; tt <= charnum; tt++) {
              if (track[tt])
                temp += c[tt].level;
            }

            if (temp > extracode[1])
              reply = TRUE;
            break;
        }

        id = FALSE;

        if (reply == TRUE)
          id = extracode[3];
        else if (extracode[4])
          id = extracode[4];

        if (id) {
          switch (extracode[2]) {
            case 0:
              if (gosub)
                pushonstack(infodoor, t);
              loaddoor2(id);
              t = -1;
              goto startover;
              break;

            case 1:
              if (gosub)
                pushonstack(infodoor, t);
              goto type1new;
              break;

            case 2:
              if (gosub)
                pushonstack(infodoor, t);
              goto type2new;
              break;
          }
        }

        break;

      case 85: /************ Branch To Random Door ***********/

        loadextracode(id);

        if (extracode[3])
          sound(extracode[3]);
        if (extracode[4])
          textbox(-1, extracode[4], 0, 0, textrect);

        switch (extracode[0]) {
          case 0: /****** branch to door *********/
            if (gosub)
              pushonstack(infodoor, t);
            loaddoor2(randrange(extracode[1], extracode[2]));
            goto startover;
            break;

          case 1: /**** go simple ****/
            if (gosub)
              pushonstack(infodoor, t);
            id = randrange(extracode[1], extracode[2]);
            goto type1new;
            break;

          case 2: /**** go complex ****/
            if (gosub)
              pushonstack(infodoor, t);
            id = randrange(extracode[1], extracode[2]);
            goto type2branch;
            break;
        }

        break;

      case 84: /************ check for registered scenario ***********/
#if !development
        if (quest[127] != 66) {
          GetIndString(myString, 3, 109);
          flashmessage(myString, 40, 50, 0, 10105);
          exit(0);
        }
#endif
        break;

      case 83: /************ Turn Priest Turning on ***********/

        canpriestturn = TRUE;
        flashmessage((StringPtr) "You regain your ability to turn undead and nether spawn.", 34, 50, 0, 20004);

        break;

      case 82: /************ Turn Priest Turning Off ***********/

        canpriestturn = FALSE;
        flashmessage((StringPtr) "You may not use your ability to turn undead or nether spawn.", 34, 50, 0, 10105);

        break;

      case 81: /******* Branch on PC Condition *******/

        loadextracode(id);

        reply = TRUE;

        switch (extracode[1]) {
          case 0: /**** check whole party ****/
            for (tt = 0; tt <= charnum; tt++)
              if (!c[tt].condition[extracode[0]])
                reply = FALSE;
            break;

          case -1:
            for (tt = 0; tt <= charnum; tt++) {
              if (track[tt]) {
                if (!c[tt].condition[extracode[0]])
                  reply = FALSE;
              }
            }
            break;

          default:
            if (!c[extracode[1]].condition[extracode[0]])
              reply = FALSE;
            break;
        }

        if (reply == TRUE) /**** pc does have condition ****/
        {
          if (gosub)
            pushonstack(infodoor, t);
          loaddoor2(extracode[3]);
          t = -1;
        } else {
          if (gosub)
            pushonstack(infodoor, t);
          loaddoor2(extracode[4]);
          t = -1;
        }
        break;

      case 1: /***** text ****/

        if (mode == -1) /**** monster macro ***/
        {
          itemRect.left = 0;
          itemRect.right = 308 + leftshift;
          itemRect.bottom = 460 + downshift;
          itemRect.top = 321 + downshift;

          SetPort(GetWindowPort(screen));
          pict(203, itemRect);
        }

        textbox(-1, id, 0, 0, textrect);

        break;

      case 2: /***** battle ****/

        for (tt = 0; tt < 8; tt++) /**** save old door codes ****/
        {
          holddoorforbattle.code[tt] = infodoor.code[tt];
          holddoorforbattle.id[tt] = infodoor.id[tt];
        }
        holddoorposforbattle = t;

        loadextracode(id);
        if (extracode[2])
          sound(extracode[2]);
        if (extracode[3])
          textbox(-1, extracode[3], 0, 0, textrect);

        if (!extracode[1])
          battlenum = extracode[0];
        else
          battlenum = randrange(abs(extracode[0]), abs(extracode[1]));

        if (extracode[0] < 0)
          battlenum = -(abs(battlenum)); //** fantasoft v7.1
        //** above line accounts for - battle id in a Range of IDs and allows surprise

        if (extracode[4] == 10)
          revivepartyflag = TRUE;
        else
          revivepartyflag = FALSE;

        combat(0, 0);

        if (revivepartyflag) // Fantasoft v7.1
        {
          if (killparty > charnum) {
            for (loop = 0; loop <= charnum; loop++) {
              c[loop].stamina = 1;
              c[loop].condition[25] = 0;
              killparty = 0;
            }
          }

          if (!indung)
            loadland(landlevel, 1);
          else
            loadland(dunglevel, 1);

          updatemain(TRUE, 0);
          centerpict();

          loaddoor2(extracode[2]);
          t = -1;
          goto startover;
        }

        if (revertgame)
          return (0);

      battledone:
        booty(extracode[4]); /**** 0 = normal, 5 = exp only ****/
        for (tt = 0; tt <= charnum; tt++)
          c[t].inbattle = TRUE;
        lastpix = -1;
        if (indung)
          loadland(dunglevel, TRUE);
        else
          loadland(landlevel, TRUE);

        needupdate = FALSE;

        for (tt = 0; tt < 8; tt++) /**** restore old door codes ****/
        {
          infodoor.code[tt] = holddoorforbattle.code[tt];
          infodoor.id[tt] = holddoorforbattle.id[tt];
        }
        t = holddoorposforbattle;

        updatemain(FALSE, 0);
        break;

      case 3: /***** choice ****/
        loadextracode(id);
        reply = question2(extracode[3], extracode[4]);
        SetPort(GetWindowPort(screen));
        EraseRect(&textrect);
        if (((reply) && (!extracode[0])) || ((!reply) && (extracode[0]))) {
          if (!extracode[1]) {
            if (!indung)
              moveparty(-1);
            return (TRUE);
          } else {
            switch (extracode[1]) {
              case 1: /****** branch to door *********/
                if (gosub)
                  pushonstack(infodoor, t);
                loaddoor2(extracode[2]);
                goto startover;
                break;

              case 2: /****** within simple *****/
                for (t = 0; t < 8; t++) {
                  infodoor.code[t] = enc.code[extracode[2]][t];
                  infodoor.id[t] = enc.id[extracode[2]][t];
                }
                goto startover;
                break;

              case 3: /****** within complex *****/
                for (t = 0; t < 8; t++) {
                  infodoor.code[t] = enc2.code[extracode[2]][t];
                  infodoor.id[t] = enc2.id[extracode[2]][t];
                }
                goto startover;
                break;

              case 4: /****** eliminate option ****/
                t = 9;
                break;
            }
          }
        }
        break;

      case 4: /******************* simple encounter *********************/

        tempdoor = holddoor = infodoor;
        temp4 = t;

      type1new:

        encountid = id;

        if ((fp = MyrFopen(":Data Files:CE", "rb")) == NULL)
          scratch(124);
        fseek(fp, id * (sizeof enc + 320), SEEK_SET);
        fread(&enc, sizeof enc, 1, fp);
        CvtEncountToPc(&enc);
        for (t = 0; t < 4; t++) {
          GetIndString(buffer[t], 3, 1);
          fread(&buffer[t], 80, 1, fp);
          MyrStrPtoC((Ptr)&buffer[t]); // myriad
        }
        fclose(fp);
        encid = id;
        enctry = enc.maxtimes;

      type1:

        sound(20005);

        encountflag = 1;

      type1comeback:

        if (enc.choiceresult[0] == -4)
          reply = 4; /********* auto goto result 4 ****/
        else {
          reply = encounter(encid, FALSE);
        }

        SetPort(GetWindowPort(screen));
        EraseRect(&textrect);
        if (!reply) /***** cancel ******/
        {
          encountflag = 0;
          centerpict();
          saveencounter(encountid, 1);
          if (!mode)
            door[doornum] = infodoor = holddoor; /**** dont do random door ***/
          if ((!indung) && (!seemless))
            moveparty(-1);
          return (TRUE);
        } else {
          tempdoor = infodoor;
          tempt = t;
          for (tt = 0; tt < 8; tt++) {
            infodoor.code[tt] = enc.code[reply - 1][tt];
            infodoor.id[tt] = enc.id[reply - 1][tt];
          }
          t = -1;
          encountflag = 1;
          goto startover;
        }
        break;

      case 5: /*************** complex encounter *****************/

      type2branch:

        needdungeonupdate = TRUE;

        tempdoor = holddoor = infodoor;
        temp4 = t;

      type2new:

        encountid = id;

        if ((fp = MyrFopen(":Data Files:CE2", "rb")) == NULL)
          scratch(125);
        fseek(fp, (id * (sizeof enc2 + 360)), SEEK_SET);
        fread(&enc2, sizeof enc2, 1, fp);
        CvtEncount2ToPc(&enc2);

        for (t = 0; t < 9; t++) {
          GetIndString(buffer[t], 3, 1);
          fread(&buffer[t], 40, 1, fp);
          MyrStrPtoC((Ptr)&buffer[t]); // myriad
        }
        fclose(fp);

        enctry = enc2.maxtimes;
        enc2id = id;

      type2:

        encountflag = 2;

        reply = encounter2();

        updatepictbox(5, FALSE, 0);

        SetPort(GetWindowPort(screen));
        EraseRect(&textrect);

        if (reply == -1)
          return (-1); /*** activate door item ******/

        if (!reply) /***** cancel ******/
        {
          encountflag = 0;
          if (!mode)
            door[doornum] = infodoor = holddoor; /**** dont do random door ***/

          saveencounter(encountid, 2);
          if (!indung)
            moveparty(-1);
          return (TRUE);
        } else {
          tempt = t;
          encountflag = 2;

          if ((reply == 4) && (enctry == 1) && (enc2.maxtimes > 1))
            reply = 3; /**** change to timeout ***/
          for (tt = 0; tt < 8; tt++) {
            infodoor.code[tt] = enc2.code[reply - 1][tt];
            infodoor.id[tt] = enc2.id[reply - 1][tt];
          }
          t = -1;
          goto startover;
        }
        break;

      case 6: /******************* load shop ********************/
        currentshop = abs(id);
        templeavail = TRUE;
        loadshop(0);

        //** lets remove any restrictions to this shop

        acceptlowrange1 = 0;
        accepthighrange1 = 0;

        acceptlowrange2 = 0;
        accepthighrange2 = 0;

        if (id < 0) {
          characterl = c[charselectnew];
          cl = charselectnew;
          cr = -1;
          numitems = 200;

        loopback:

          music(8); /**** shop music ***/

          reply = seeshop(0);

          if (reply == -1) {
            items();
            goto loopback;
          }
          updatemain(0, 0);
        }
        break;

      case 7: /********** Action Data Data With X-AP Data ********/

        loadextracode(id);
        tempdoor = infodoor;

        switch (extracode[0]) {
          case -1: /********* Replace Simple Result Code ************/
            loaddoor2(extracode[2]);
            if ((fp = MyrFopen(":Data Files:CE", "r+b")) == NULL)
              scratch(853);
            fseek(fp, extracode[1] * (sizeof enc + 320), SEEK_SET);
            fread(&enc, sizeof enc, 1, fp);
            CvtEncountToPc(&enc);
            for (tt = 0; tt < 8; tt++) {
              enc.code[extracode[4]][tt] = infodoor.code[tt];
              enc.id[extracode[4]][tt] = infodoor.id[tt];
            }
            fseek(fp, extracode[1] * (sizeof enc + 320), SEEK_SET);
            CvtEncountToPc(&enc);
            fwrite(&enc, sizeof enc, 1, fp);
            CvtEncountToPc(&enc);
            fclose(fp);
            infodoor = tempdoor;
            break;

          case -2: /********* Replace Complex Result Code ************/
            loaddoor2(extracode[2]);
            if ((fp = MyrFopen(":Data Files:CE2", "r+b")) == NULL)
              scratch(125);
            fseek(fp, (extracode[1] * (sizeof enc2 + 360)), SEEK_SET);
            fread(&enc2, sizeof enc2, 1, fp);
            CvtEncount2ToPc(&enc2);
            for (tt = 0; tt < 8; tt++) {
              enc2.code[extracode[4]][tt] = infodoor.code[tt];
              enc2.id[extracode[4]][tt] = infodoor.id[tt];
            }
            fseek(fp, extracode[1] * (sizeof enc + 320), SEEK_SET);
            CvtEncount2ToPc(&enc);
            fwrite(&enc2, sizeof enc2, 1, fp);
            CvtEncount2ToPc(&enc);
            fclose(fp);
            infodoor = tempdoor;
            break;

          default:
            if (indung)
              saveland(dunglevel);
            else
              saveland(landlevel);

            temp2 = indung;

            if (extracode[3]) /**** other than default level ****/
            {
              indung = extracode[3] - 1;
            }

            templong = extracode[0];
            loadland(templong, 0);

            loaddoor2(extracode[2]);

            for (tt = 0; tt < 8; tt++) {
              door[extracode[1]].code[tt] = infodoor.code[tt];
              door[extracode[1]].id[tt] = infodoor.id[tt];
            }
            saveland(templong);

            indung = temp2;

            if (indung)
              loadland(dunglevel, 0);
            else
              loadland(landlevel, 0);
            break;
        }

        infodoor = tempdoor;

        break;

      case 8: /********** Same As Other Door ********/
        for (tt = 0; tt < 8; tt++) {
          infodoor.code[tt] = door[id].code[tt];
          infodoor.id[tt] = door[id].id[tt];
        }
        t = 0;
        goto moveon;
        break;

      case 9: /****** Play Sound ********/
        sound(id);
        break;

      case 10: /***** give treasure ****/
        getfilename("Data TD");
        if ((fp = MyrFopen(filename, "rb")) == NULL)
          scratch(126);
        fseek(fp, id * sizeof treasure, SEEK_SET);
        fread(&treasure, sizeof treasure, 1, fp);
        CvtTreasureToPc(&treasure);
        fclose(fp);
        nummon = incombat = 0;
        booty(1);
        updatemain(FALSE, -1 + needupdate);
        break;

      case 11: /***** give exp ******/
        for (tt = 0; tt <= charnum; tt++)
          c[t].inbattle = TRUE;
        totalexp = id;
        nummon = incombat = 0;
        booty(0);
        updatemain(FALSE, -1 + needupdate);
        break;

      case 12: /***** new land icon ******/
        loadextracode(id);
        if (indung)
          saveland(dunglevel);
        else
          saveland(landlevel);
        templong = extracode[0];
        temp = indung;

        indung = extracode[4];

        loadland(templong, 0);

        if (indung)
          field[extracode[2]][extracode[1]] = extracode[3];
        else
          field[extracode[1]][extracode[2]] = extracode[3];

        saveland(templong);

        indung = temp;

        if (indung)
          loadland(dunglevel, 0);
        else
          loadland(landlevel, 0);
        centerpict();
        break;

      case 13: /******* enable / disable door *****/
        loadextracode(id);
        tempdoor = infodoor;
        if (indung)
          saveland(dunglevel);
        else
          saveland(landlevel);

        templong = extracode[0];

        temp = indung;

        if (extracode[3] < 0)
          indung = TRUE;
        else if (extracode[3] > 0)
          indung = FALSE;

        loadland(templong, 0);
        if (extracode[1])
          door[extracode[1]].percent = extracode[2];

        if (extracode[3]) {
          for (tt = extracode[3]; tt <= extracode[4]; tt++) {
            door[tt].percent = extracode[2];
          }
        }

        saveland(templong);

        indung = temp;

        if (indung)
          loadland(dunglevel, 0);
        else
          loadland(landlevel, 0);
        infodoor = tempdoor;
        break;

      case 14: /******* pick characters *****/
        if (id < 0)
          fastsound = 1;
        else
          fastsound = 0;
        getchoice(abs(id) - 1, 0, fastsound);
        break;

      case -14: /******* pick inverse characters *****/
        if (id < 0)
          fastsound = 1;
        else
          fastsound = 0;
        getchoice(abs(id) - 1, 0, fastsound);
        for (tt = 0; tt <= charnum; tt++) {
          if (track[tt])
            track[tt] = FALSE;
          else
            track[tt] = TRUE;
        }
        break;

      case 15: /***** give damage/heal to picked characters ****/
        loadextracode(id);
        spellinfo.spelllook2 = 15;
        for (tt = 0; tt <= charnum; tt++)
          if (track[tt]) {
            if (extracode[3])
              sound(extracode[3]);
            heal(tt, extracode[0] * randrange(extracode[1], extracode[2]), 1);
          }
        if (extracode[4])
          textbox(-1, abs(extracode[4]), 0, 0, textrect);
        break;

      case 16: /***** give damage/heal to party****/
        loadextracode(id);
        spellinfo.spelllook2 = 15;
        for (tt = 0; tt <= charnum; tt++) {
          if (extracode[3])
            sound(extracode[3]);
          heal(tt, extracode[0] * randrange(extracode[1], extracode[2]), 1);
        }
        if (extracode[4])
          textbox(-1, abs(extracode[4]), 0, 0, textrect);
        break;

      case 17: /***** cast spell on picked ****/
        loadextracode(id);
        loadspell2(extracode[0]);
        powerlevel = extracode[1];
        spellinfo.saveadjust += extracode[2];
        if (extracode[3])
          spellinfo.cannot = 3; /**** force affect ****/

        if (mode == -1) /***** monster macro ****/
        {
          if (data > 1) {
            target[0][0] = monpos[data - 10][0] + fieldx;
            target[0][1] = monpos[data - 10][1] + fieldy;
          } else {
            target[0][0] = todoque[abs(data) - 1].x;
            target[0][1] = todoque[abs(data) - 1].y;
          }
          spelltargets(0, -1, FALSE, 0);
        } else {
          inspell = TRUE;
          resolvespell();
          inspell = FALSE;
        }
        break;

      case 18: /***** cast spell on party ****/
        loadextracode(id);
        for (tt = 0; tt < maxloop; tt++)
          track[tt] = 0;
        for (tt = 0; tt <= charnum; tt++)
          track[tt] = 1;
        loadspell2(extracode[0]);
        powerlevel = extracode[1];
        spellinfo.saveadjust += extracode[2];
        if (extracode[3])
          spellinfo.cannot = 3; /**** force affect ****/
        resolvespell();
        break;

      case 19: /***** Display Random String ****/
        loadextracode(id);
        textbox(-1, randrange(extracode[0], extracode[1]), 0, 0, textrect);
        break;

      case 20: /***** Teleport ****/
        loadextracode(id);
      teleport:

        if (indung)
          saveland(dunglevel);
        else
          saveland(landlevel);

        if (indung) {
          if (extracode[0] > -1)
            dunglevel = extracode[0];
          if (extracode[1] > -1)
            x = floorx = extracode[1];
          if (extracode[2] > -1)
            y = floory = extracode[2];
          infodoor.landid = dunglevel;
          loadland(dunglevel, 0);
        } else {
          if (extracode[0] > -1)
            landlevel = extracode[0];
          if (extracode[1] > -1)
            x = lookx = extracode[1];
          if (extracode[2] > -1)
            y = looky = extracode[2];
          partyx = partyy = 0;
          infodoor.landid = landlevel;
          loadland(landlevel, 1);
        }

        infodoor.landx = x;
        infodoor.landy = y;

        centerpict();

        if (extracode[3])
          sound(extracode[3]);
        if (extracode[4])
          textbox(-1, extracode[4], 0, 0, textrect);

        if (code == 45)
          goto endteleport; /***** teleport only ***/

        infodoor.landid = landlevel;
        seconddoor = TRUE;
        if (abs(field[x][y]) > 999)
          goto recheck;

      endteleport:

        break;

      case 21: /***** Branch on item Possession ****/
        loadextracode(id);
        if (checkforitem(extracode[0], FALSE, -1)) {
          switch (extracode[1]) {
            case 0:
              if (gosub)
                pushonstack(infodoor, t);
              loaddoor2(extracode[3]);
              t = -1;
              goto startover;
              break;

            case 1:
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[3];
              goto type1new;
              break;

            case 2:
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[3];
              goto type2new;
              break;
          }
        } else {
          if (!extracode[2]) {
            switch (extracode[1]) /****** not possesed *****/
            {
              case 0:
                if (gosub)
                  pushonstack(infodoor, t);
                loaddoor2(extracode[4]);
                t = -1;
                goto startover;
                break;

              case 1:
                if (gosub)
                  pushonstack(infodoor, t);
                id = extracode[4];
                goto type1new;
                break;

              case 2:
                if (gosub)
                  pushonstack(infodoor, t);
                id = extracode[4];
                goto type2new;
                break;
            }
          } else if (extracode[2] == 2) /******* display string and exit ********/
          {
            textbox(-1, extracode[4], 0, 0, textrect);
            goto getout;
          }
        }
        break;

      case 67: /***** Branch on item charges ****/
        loadextracode(id);
        if (checkforcharge(extracode[0]) >= extracode[2]) {
          switch (extracode[1]) {
            case 0:
              if (gosub)
                pushonstack(infodoor, t);
              loaddoor2(extracode[3]);
              t = -1;
              goto startover;
              break;

            case 1:
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[3];
              if (id != -1)
                goto type1new;
              break;

            case 2:
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[3];
              if (id != -1)
                goto type2new;
              break;
          }
        } else {
          switch (extracode[1]) {
            case 0:
              if (gosub)
                pushonstack(infodoor, t);
              loaddoor2(extracode[4]);
              t = -1;
              goto startover;
              break;

            case 1:
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[4];
              goto type1new;
              break;

            case 2:
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[4];
              goto type2new;
              break;
          }
        }
        break;

      case 68: /***** Alter Party Fatigue ****/
        loadextracode(id);
        switch (extracode[0]) {
          case 1:
            fat = 135;
            if (FrontWindow() == look)
              updatefat(TRUE, 0, TRUE);
            break;

          case 2:
            fat = 4;
            if (FrontWindow() == look)
              updatefat(TRUE, 0, TRUE);
            break;

          case 3:
            fat = fat * (extracode[2] / 100);
            if (FrontWindow() == look)
              updatefat(TRUE, 0, TRUE);
            break;
        }
        break;

      case 69: /***** Set Spell Casting / Charging Flags ****/

        if (id) {
          loadextracode(id);
          spellcasting = extracode[0];
          monstercasting = extracode[1];
          spellcharging = extracode[2];
        }
        break;

      case 71: /***** Disable & Enable X: Y: Display ****/
        xydisplayflag = id;
        updatefat(TRUE, 0, TRUE);
        xy(0);
        break;

      case 70: /***** Save and Restore Party Postion ****/
        loadextracode(id);

        switch (extracode[0]) {
          case 1: /*********** save current postion ******/
            if (!indung) {
              savedlandlevel = landlevel;
              savedpostion[0] = partyx + lookx;
              savedpostion[1] = partyy + looky;
            } else {
              savedlandlevel = dunglevel;
              savedpostion[0] = partyx + wallx;
              savedpostion[1] = partyy + wally;
            }

            savedlandtype = indung;

            break;

          case 2: /*********** restore last saved postion ******/

            if (indung)
              saveland(dunglevel);
            else
              saveland(landlevel);

            if (savedlandtype == TRUE) /********* restore to inside a dungeon *********/
            {
              indung = TRUE;
              dunglevel = savedlandlevel;
              wallx = savedpostion[0] - 5;
              partyx = 5;
              wally = savedpostion[1] - 5;
              partyy = 5;
            } else {
              indung = FALSE;
              landlevel = savedlandlevel;
              lookx = savedpostion[0] - 5;
              partyx = 5;
              looky = savedpostion[1] - 5;
              partyy = 5;
            }

            if (indung)
              loadland(dunglevel, 0);
            else
              loadland(landlevel, 1);

            infodoor.landid = savedlandlevel; /****** keeps an AP from moving them after the restore ****/
            infodoor.landx = savedpostion[0];
            infodoor.landy = savedpostion[1];

            centerpict();

            break;
        }
        break;

      case 23: /***** alter rand rect info (Land) ****/
        loadextracode(id);

        temp = indung;

        if (indung)
          saveland(dunglevel);
        else
          saveland(landlevel);

        templong = extracode[0];

        indung = FALSE;

        loadland(templong, 0);
        temp1 = extracode[1];
        randlevel.percent[temp1] = extracode[2];
        if (extracode[3] > -1)
          randlevel.battlerange[temp1][0] = extracode[3];
        if (extracode[4] > -1)
          randlevel.battlerange[temp1][1] = extracode[4];
        saveland(templong);

        indung = temp;

        if (!indung)
          loadland(landlevel, 0);
        else
          loadland(dunglevel, 0);
        break;

      case -23: /***** alter rand rect info (dungeon) ****/
        loadextracode(id);

        temp = indung;

        if (indung)
          saveland(dunglevel);
        else
          saveland(landlevel);

        templong = extracode[0];

        indung = TRUE;

        loadland(templong, 0);
        temp1 = extracode[1];
        randlevel.percent[temp1] = extracode[2];
        if (extracode[3] > -1)
          randlevel.battlerange[temp1][0] = extracode[3];
        if (extracode[4] > -1)
          randlevel.battlerange[temp1][1] = extracode[4];
        saveland(templong);

        indung = temp;

        if (!indung)
          loadland(landlevel, 0);
        else
          loadland(dunglevel, 0);
        break;

      case 24: /******* Keep Codes *******/
        encountflag = 0;
        goto keepcodes;
        break;

      case 25: /******* Remove Door X-Y *******/

      blowout:
        gosub = stackindex = 0;

        remove = TRUE;
        encountflag = 0;
        temp1 = x;
        temp2 = y;
        break;

      case 26: /******* Get Click *******/
        flashmessage((StringPtr) "Click Mouse", 350, 100, 0, 30005);
        break;

      case 27: /******* Show Pict *******/

        SetPort(GetWindowPort(look));
        picture = NIL;
        picture = GetPicture(id);
        if (picture) {
          itemRect = (**picture).picFrame;
          /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
           * NOTE(fuzziqersoftware): GetPicture decodes the PICT internally and
           * writes a header with picFrame in native byte order, so we don't
           * need to byteswap the rect here using rintel2moto.
           */
          // rintel2moto(&itemRect);
          /* *** END CHANGES *** */
          OffsetRect(&itemRect, -itemRect.left, -itemRect.top);
          if ((itemRect.bottom < (320 + downshift)) || (itemRect.right < (320 + leftshift))) {
            itemRect.top = ((320 + downshift) - itemRect.bottom) / 2;
            itemRect.left = ((320 + leftshift) - itemRect.right) / 2;
            itemRect.bottom += itemRect.top;
            itemRect.right += itemRect.left;
          }
          EraseRect(&lookrect);
          if (picture)
            DrawPicture(picture, &itemRect);
        }
        break;

      case 28: /******* Center Screen *******/
        centerpict();
        break;

      case 66: /******* Disable/Enable Camping *******/

        temp = cancamp;
        if (id)
          cancamp = 1;
        else
          cancamp = 0;

        if (temp != id) {
          if (!id)
            flashmessage((StringPtr) "You may now camp again.", 50, 50, 0, 6001);
          else
            flashmessage((StringPtr) "You may not camp at the present time.", 50, 50, 0, 6001);
        }
        break;

      case 29: /******* Give/Display Map *******/
        map[abs(id)] = TRUE;
        updatemapmenu();
        CheckItem(gScenario, abs(id) + 4, 1);
        if (id > -1)
          flashmessage((StringPtr) "You gain a map, to view the map use Maps/Notes in the Menu.", 30, 355, 0, 30005);
        else
          showmap(abs(id));
        break;

      case 30: /******* Set Picked on Check Vs. Special Ability or Attribute *******/
        loadextracode(id);
        if (extracode[2] == 1)
          for (temp = 0; temp <= charnum; temp++)
            track[temp] = TRUE;
        if (extracode[2] == 2)
          for (temp = 0; temp <= charnum; temp++)
            if (c[temp].stamina > 0)
              track[temp] = TRUE;
        for (loop = 0; loop <= charnum; loop++)
          if (track[loop]) {
            track[loop] = FALSE; /***** clear track before test ****/
            if (extracode[3]) /************** check vs. attribute ***********/
            {
              temp = (Rand(25) - extracode[1]);
              switch (abs(extracode[0])) {
                case 0: /*** brawn ****/
                  if (temp < c[t].st)
                    track[loop] = TRUE;
                  break;

                case 1: /*** intelligence ****/
                  if (temp < c[t].in)
                    track[loop] = TRUE;
                  break;

                case 2: /*** wisdom ****/
                  if (temp < c[t].wi)
                    track[loop] = TRUE;
                  break;

                case 3: /*** dexterity ****/
                  if (temp < c[t].de)
                    track[loop] = TRUE;
                  break;

                case 4: /*** consitition ****/
                  if (temp < c[t].co)
                    track[loop] = TRUE;
                  break;

                case 5: /*** Not Used ****/
                  break;

                case 6: /*** luck ****/
                  if (temp < c[t].lu)
                    track[loop] = TRUE;
                  break;
              }
              if (extracode[0] < 0) /*** reverse pick *****/
              {
                if (track[loop])
                  track[loop] = 0;
                else
                  track[loop] = TRUE;
              }
            } else /************** check vs. special ability ***********/
            {
              if (Rand(100) > c[t].spec[abs(extracode[0])] + extracode[1])
                track[loop] = FALSE;
              else
                track[loop] = TRUE;
            }
          }

        if (extracode[0] < 0) /**** set on fail if desired *****/
        {
          for (loop = 0; loop <= charnum; loop++) {
            if (track[loop])
              track[loop] = FALSE;
            else
              track[loop] = TRUE;
          }
        }
        break;

      case 31: /******* Branch on Check Vs. Ability *******/
        getchoice(0, 0, fastsound); /**** pick 1 character ****/
        loadextracode(id);
        for (temp = 0; temp <= charnum; temp++)
          if (track[temp])
            track[0] = temp;
        if (extracode[2]) {
          temp = (Rand(25) - extracode[1]);
          switch (extracode[0]) {
            case 0: /*** brawn ****/
              if (temp < c[track[0]].st)
                goto goodcheck;
              else
                goto badcheck;
              break;

            case 1: /*** intelligence ****/
              if (temp < c[track[0]].in)
                goto goodcheck;
              else
                goto badcheck;
              break;

            case 2: /*** wisdom ****/
              if (temp < c[track[0]].wi)
                goto goodcheck;
              else
                goto badcheck;
              break;

            case 3: /*** dexterity ****/
              if (temp < c[track[0]].de)
                goto goodcheck;
              else
                goto badcheck;
              break;

            case 4: /*** consitition ****/
              if (temp < c[track[0]].co)
                goto goodcheck;
              else
                goto badcheck;
              break;

            case 5: /*** Not Used ****/
              break;

            case 6: /*** luck ****/
              if (temp < c[track[0]].lu)
                goto goodcheck;
              else
                goto badcheck;
              break;
          }
        } else {
          if (Rand(100) <= c[track[0]].spec[extracode[0]] + extracode[1]) {
          goodcheck:
            if (gosub)
              pushonstack(infodoor, t);
            loaddoor2(extracode[3]);
            t = -1;
          } else {
          badcheck:
            if (gosub)
              pushonstack(infodoor, t);
            loaddoor2(extracode[4]);
            t = -1;
          }
        }
        break;

      case 32: /**************** offer temple *************/
        templeavail = TRUE;
        templecost = id;
        sound(10105);
        updatecontrols();
        break;

      case 33: /**************** take gold *************/
        loadextracode(id);

        if (extracode[0] > 0)
          reply = takegold(extracode[0], 0);
        else
          reply = takegold(abs(extracode[0]), 1);

        if ((!reply) && (extracode[1] == -1))
          t = 6;

        goto branch;
        break;

      case 34: /**************** break encounter loop *************/
        infodoor = holddoor;
        t = temp4 + 1;
        encountflag = 0;
        goto startcode;
        break;

      case 35: /**************** eliminate simple enc option *************/
        enc.choiceresult[id - 1] = 0;
        saveencounter(encountid, 1);
        tempdoor = infodoor = holddoor;
        goto type1comeback;
        break;

      case 36: /********** Store / Give Equipment **************/

        if ((id) && (!storeditems)) /********** take parties equipment *************/
        {
          storeditems = TRUE;

          pool();

          wealthstore[0] = moneypool[0];
          wealthstore[1] = moneypool[1];
          wealthstore[2] = moneypool[2];

          moneypool[0] = 0;
          moneypool[1] = 0;
          moneypool[2] = 0;

          for (tt = 0; tt <= charnum; tt++) {
            for (loop = 0; loop < c[tt].numitems; loop++) {
              storage[tt].items[loop].id = c[tt].items[loop].id;
              storage[tt].items[loop].equip = c[tt].items[loop].equip;
              storage[tt].items[loop].charge = c[tt].items[loop].charge;
              storage[tt].items[loop].ident = c[tt].items[loop].ident;
            }
            for (loop = 0; loop < 30; loop++)
              dropitem(tt, c[tt].items[0].id, 0, 0, TRUE);
            for (loop = 0; loop < 30; loop++)
              c[tt].items[loop] = blank100;
            c[tt].numitems = 0;
          }
        } else if ((!id) && (storeditems)) /********** give parties equipment back *************/
        {
          moneypool[0] += wealthstore[0];
          moneypool[1] += wealthstore[1];
          moneypool[2] += wealthstore[2];

          share();

          wealthstore[0] = wealthstore[1] = wealthstore[2] = 0;

          temp4 = 0;
          for (tt = 0; tt <= charnum; tt++) {
            c[tt].numitems = 0;
            for (loop = 0; loop < 30; loop++) {
              if (storage[tt].items[loop].id) {
                if (c[tt].items[loop].id)
                  storageitems[temp4++] = c[tt].items[loop].id;
                c[tt].numitems++;
                c[tt].items[loop].id = storage[tt].items[loop].id;
                c[tt].items[loop].equip = storage[tt].items[loop].equip;
                c[tt].items[loop].charge = storage[tt].items[loop].charge;
                c[tt].items[loop].ident = storage[tt].items[loop].ident;
                if (c[tt].items[loop].equip)
                  wear(tt, loop, 0);
                storage[tt].items[loop].id = storage[tt].items[loop].equip = storage[tt].items[loop].ident = storage[tt].items[loop].charge = 0;
              }
            }
          }
          storeditems = FALSE;
          if (temp4) {
            flashmessage((StringPtr) "Here is your recovered wealth and extra items.", 50, 50, 0, 6000);
            booty(10);
            updatemain(FALSE, -1);
          }
        }

        break;

      case 37: /********************* dungeon move *****/
        loadextracode(id);
        theControl = NIL;

        if (indung)
          saveland(dunglevel);
        else
          saveland(landlevel);

        if (!extracode[0]) {
          dunglevel = extracode[1];
          loadland(dunglevel, 1);

          head = abs(extracode[4]);
          multiview = TRUE;
          if (extracode[4] < 0) {
            multiview = FALSE;
            viewtype = TRUE;
          }

          threed(dunglevel, extracode[2], extracode[3], head);
          return (1);
        } else {
          indung = partyx = partyy = 0;
          landlevel = extracode[1];
          loadland(landlevel, 1);
          x = lookx = extracode[2];
          y = looky = extracode[3];

          SelectWindow(look);
          SetPort(GetWindowPort(look));
          centerpict();
          in();
          return (1);
        }
        break;

      case 38: /********* branch on possesion II *****/
        loadextracode(id);
        reply = checkforitem(extracode[0], FALSE, -1); /***** who = -1 is anybody ****/

      branch:

        if (extracode[1] == 2)
          goto forcebranch; /******** force branch ********/
        if (((extracode[1] == 0) && (!reply)) || ((extracode[1] == 1) && (reply))) {
          /********** branch *********/
        forcebranch:
          switch (extracode[2]) {
            case -1: /**** dropout ****/
              t = 7;
              goto startcode;
              break;

            case 0: /****** extra door *****/
              id = extracode[3];
              goto jumptoextradoor;
              break;

            case 1: /****** within simple *****/
              for (t = 0; t < 8; t++) {
                infodoor.code[t] = enc.code[extracode[3]][t];
                infodoor.id[t] = enc.id[extracode[3]][t];
              }
              t = extracode[4] - 1;
              break;

            case 2: /****** within complex *****/
              for (t = 0; t < 8; t++) {
                infodoor.code[t] = enc2.code[extracode[3]][t];
                infodoor.id[t] = enc2.id[extracode[3]][t];
              }
              t = extracode[4] - 1;
              break;

            case 3: /****** exit script and keep codes *****/
              encountflag = 0;
              goto keepcodes;
              break;
          }
        }
        break;

      case 39: /********** Extend Door Codes ********/
      jumptoextradoor:
        loaddoor2(id);
        t = -1;
        goto startover;
        break;

      case 40: /**************** Branch on party condition else continue *************/
        loadextracode(id);

        reply = extracode[0];
        temp = partycondition[extracode[3]];

        if (((reply == 2) && (!temp)) || ((reply == 1) && (temp))) {
          switch (extracode[1]) {
            case 1: /**** go door ****/
              if (gosub)
                pushonstack(infodoor, t);
              loaddoor2(extracode[2]);
              t = -1;
              goto startover;
              break;

            case 2: /**** go simple ****/
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[2];
              goto type1new;
              break;

            case 3: /**** go complex ****/
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[2];
              goto type2branch;
              break;

            default: /**** no branch ****/
              t = 8;
              break;
          }
        }
        break;

      case 41: /**************** eliminate simple enc option *************/
        loadextracode(id);
        if ((fp = MyrFopen(":Data Files:CE", "r+b")) == NULL)
          scratch(127);
        fseek(fp, extracode[0] * (sizeof enc + 320), SEEK_SET);
        fread(&tempenc, sizeof enc, 1, fp);
        CvtEncountToPc(&tempenc);
        tempenc.choiceresult[extracode[1] - 1] = 0;
        fseek(fp, extracode[0] * (sizeof enc + 320), SEEK_SET);
        CvtEncountToPc(&tempenc);
        fwrite(&tempenc, sizeof tempenc, 1, fp);
        CvtEncountToPc(&tempenc);
        fclose(fp);
        break;

      case 42: /**************** Branch on % Chance *************/
        loadextracode(id);

        if (Rand(100) <= extracode[0]) {
          if (extracode[1] == -2)
            t = 99; /**** dropout & erase ****/
          if (extracode[1] == 2)
            goto keepcodes; /**** dropout & keep codes ****/
          if (extracode[1] == 1)
            goto forcebranch;
        }

        break;

      case 58: /**************** Branch difficulty level *************/
        loadextracode(id);

        if (howhard >= extracode[0]) {
          if (extracode[1] == -2)
            t = 99; /**** dropout & erase ****/
          if (extracode[1] == 2)
            goto keepcodes; /**** dropout & keep codes ****/
          if (extracode[1] == 1)
            goto forcebranch;
        }

        break;

      case 44:
        for (tt = 0; tt < 7; tt++)
          enc2.code[id - 1][tt] = enc2.id[id - 1][tt] = 0;
        enc2.code[id - 1][7] = 24;
        break;

      case 43: /**************** give condition *************/
        loadextracode(id);

        for (tt = 0; tt <= charnum; tt++) {
          if (c[tt].condition[extracode[1]] > 0)
            c[tt].condition[extracode[1]] = 0;
          dosound = FALSE;
          switch (extracode[0]) {
            case 0: /****** party ****/
              dosound = TRUE;
              break;

            case 1: /****** picked ****/
              if (track[tt])
                dosound = TRUE;
              break;

            case 2: /****** alive ****/
              if (c[tt].stamina > -10)
                dosound = TRUE;
              break;
          }

          if (dosound) {
            if (c[tt].condition[extracode[1]] > 0)
              c[tt].condition[extracode[1]] = 0;
            c[tt].condition[extracode[1]] += extracode[2];
            sound(extracode[3]);
            spelleffect(tt, 0);
            showresults(tt, extracode[1] + 1, 0);
          }
        }
        break;

      case 45: /***** Teleport only ****/
        loadextracode(id);
        noseconddoor = TRUE;
        goto teleport;
        break;

      case 46: /********** branch on quest ***********/
        loadextracode(id);

        smallreply = quest[extracode[0]];

        if (extracode[1] == 2) /******** force branch ********/
        {
          if (gosub)
            pushonstack(infodoor, t); //***** Fantasoft v7.1 BEGIN
          goto forcebranch;
        }

        if (((extracode[1] == 1) && (smallreply)) || ((!extracode[1]) && (!smallreply))) {
          if (gosub)
            pushonstack(infodoor, t);
          goto forcebranch;
        } //***** Fantasoft v7.1 End

        break;

      case 72: /********** branch on range of quest ***********/
        loadextracode(id);

        smallreply = TRUE;

        for (tt = extracode[0]; tt <= extracode[1]; tt++)
          if (!quest[tt])
            smallreply = FALSE;

        if (smallreply) {
          switch (extracode[3]) {
            case 0: // branch to X-AP
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[4];
              loaddoor2(id);
              t = -1;
              goto startover;
              break;

            case 1: // branch to simple encounter
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[4];
              goto type1new;
              break;

            case 2: // branch to complex encounter
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[4];
              goto type2new;
              break;
          }
        }

        break;

      case 73: /********** Load Shop & Restrict Items ***********/

        loadextracode(id);
        id = extracode[0];

        currentshop = abs(id);
        templeavail = TRUE;
        loadshop(0);

        //** lets set up the restrictions to this shop

        acceptlowrange1 = extracode[1];
        accepthighrange1 = extracode[2];

        acceptlowrange2 = extracode[3];
        accepthighrange2 = extracode[4];

        //*********************************************

        if (id < 0) {
          characterl = c[charselectnew];
          cl = charselectnew;
          cr = -1;
          numitems = 200;

        loopback2:

          music(8); /**** shop music ***/

          reply = seeshop(0);

          if (reply == -1) {
            items();
            goto loopback2;
          }
          updatemain(0, 0);
        }

        break;

      case 74: /********** Take / Give Spell Points To Picked Characters ***********/

        loadextracode(id);

        for (tt = 0; tt <= charnum; tt++) {
          if ((track[tt]) && (c[tt].spellpointsmax)) {
            needupdate = TRUE;
            if (extracode[3])
              sound(extracode[1]);
            temp = abs(extracode[0]);

            for (loop = 0; loop < temp; loop++) {
              temp2 = randrange(extracode[1], extracode[2]);
            }

            if (extracode[0] < 0)
              temp2 *= -1;

            c[tt].spellpoints += temp2;

            if (c[tt].spellpoints < 0)
              c[tt].spellpoints = 0;
            if (c[tt].spellpoints > c[tt].spellpointsmax)
              c[tt].spellpoints = c[tt].spellpointsmax;
          }
        }
        if (needupdate)
          updatemain(0, 0);

        if (extracode[4])
          textbox(-1, extracode[4], 0, 0, textrect); /**** text message ***/
        break;

      case 75: /********** Branch on Spell Points ***********/

        loadextracode(id);
        smallreply = FALSE;

        switch (extracode[0]) {
          case 1: // check picked only

            for (tt = 0; tt <= charnum; tt++) {
              if (track[tt]) {
                if (c[tt].spellpoints >= extracode[1])
                  smallreply = TRUE;
              }
            }

            break;

          case 2: // check all alive

            for (tt = 0; tt <= charnum; tt++) {
              if ((c[tt].stamina > 0) && (!c[tt].condition[25])) {
                if (c[tt].spellpoints >= extracode[1])
                  smallreply = TRUE;
              }
            }

            break;
        }

        if (smallreply) {
          switch (extracode[3]) {
            case 0: // branch to X-AP
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[4];
              loaddoor2(id);
              t = -1;
              goto startover;
              break;

            case 1: // branch to Simple Enc
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[4];
              goto type1new;
              break;

            case 2: // branch to Complex Enc
              if (gosub)
                pushonstack(infodoor, t);
              id = extracode[4];
              goto type2new;
              break;
          }
        } else if (extracode[2] == 1)
          goto keepcodes;

        break;

      case 47: /************ setquest ***********/

        if (id > 0)
          quest[id] = TRUE;
        else
          quest[abs(id)] = FALSE;
        break;

      case 48: /************ selective combat ***********/
        loadextracode(id);

        if (extracode[2])
          sound(extracode[2]);
        if (extracode[3])
          textbox(-1, extracode[3], 0, 0, textrect);

        if (extracode[1])
          battlenum = randrange(extracode[0], extracode[1]);
        else
          battlenum = extracode[0];

        tempencountflag = encountflag;
        combat(0, 1);

        if (revertgame)
          return (0);

        encountflag = tempencountflag;

        reply = FALSE;
        for (tt = 0; tt <= charnum; tt++)
          if (c[tt].inbattle)
            reply = TRUE;

        if (reply) {
          booty(0);

          if (extracode[4]) {
            getfilename("Data TD");
            if ((fp = MyrFopen(filename, "rb")) == NULL)
              scratch(128);
            fseek(fp, extracode[4] * sizeof treasure, SEEK_SET);
            fread(&treasure, sizeof treasure, 1, fp);
            CvtTreasureToPc(&treasure);
            fclose(fp);
            nummon = incombat = 0;
            booty(1);
          }
        } else {
          flashmessage((StringPtr) "There is nobody left to collect any treasure.", 100, 100, 180, 6000);
          booty(2);
        }

        for (tt = 0; tt <= charnum; tt++)
          c[t].inbattle = TRUE;
        lastpix = -1;
        if (indung)
          loadland(dunglevel, TRUE);
        else
          loadland(landlevel, TRUE);
        updatemain(FALSE, 0);
        break;

      case 49: /***** bank ******/

        bankavailable = TRUE;
        sound(128);
        updatecontrols();
        warn(106);

        break;

      case 50: /************ Pick on Race/Caste/Gender ***********/
        loadextracode(id);
        for (tt = 0; tt < 6; tt++)
          track[tt] = 0;

        for (tt = 0; tt <= charnum; tt++) {
          if (((extracode[4]) && (c[tt].stamina > 0)) || (!extracode[4])) {
            switch (extracode[0]) {
              case 0: /**** Race ****/
                if (c[tt].race == extracode[2])
                  track[tt] = TRUE;

                break;

              case 1: /**** Gender ****/
                if (c[tt].gender == extracode[1])
                  track[tt] = TRUE;
                break;

              case 2: /**** Caste ****/
                if (c[tt].caste == extracode[2])
                  track[tt] = TRUE;
                break;

              case 3: /**** Race Class ****/
                loadprofile(c[tt].race, 0);

                if (MyrBitTstShort(&races.descriptors, extracode[2] - 1))
                  track[tt] = TRUE;
                break;

              case 4: /**** Caste Class ****/
                loadprofile(0, c[tt].caste);
                if (caste.casteclass == extracode[2])
                  track[tt] = TRUE;
                break;
            }
          }
        }
        break;

      case 51: /************ alter shop ***********/
        loadextracode(id);
        loadshop(extracode[0]);
        theshop.inflation += extracode[1];
        for (tt = 0; tt < 1000; tt++) {
          if (theshop.id[tt] == extracode[2]) {
            theshop.num[tt] += extracode[3];
            if (theshop.num[tt] < 0)
              theshop.num[tt] = 0;
          }
        }
        saveshop(1, (Ptr) "");
        break;

      case 52: /************ pick on pos/move/item/%/current selected ***********/
        loadextracode(id);
        if (extracode[2] != 2)
          for (tt = 0; tt < 6; tt++)
            track[tt] = 0;
        for (tt = 0; tt < 6; tt++)
          track[tt] = 0;

        for (tt = 0; tt <= charnum; tt++) {
          if ((((extracode[2] == 1) && (c[tt].stamina > 0)) || (!extracode[2])) || ((extracode[2] == 2) && (track[tt]))) {
            switch (extracode[0]) {
              case 0: /**** movement ****/
                if (c[tt].movementmax < extracode[1])
                  track[tt] = TRUE;
                break;

              case 1: /**** position ****/
                if (tt + 1 < extracode[1])
                  track[tt] = TRUE;
                break;

              case 2: /**** possess item id ****/
                if (checkforitem(extracode[1], FALSE, tt))
                  track[tt] = TRUE;
                break;

              case 3: /**** % ****/
                if (Rand(100) <= extracode[1])
                  track[tt] = TRUE;
                break;

              case 4: /**** DRVs. Attribute, ie, Brawn, Agility ****/
                if (!savevsattr(extracode[1], tt))
                  track[tt] = TRUE;
                break;

              case 5: /**** DRVs. Spell Type, ie, Chemical, Electrical ****/
                if (!savevs(extracode[1], tt))
                  track[tt] = TRUE;
                break;

              case 6: /**** pick current selected PC ****/
                track[charselectnew] = TRUE;
                break;

              case 7: /**** item is worn ****/
                if (checkforitem(extracode[1], FALSE, tt)) {
                  for (loop = 0; loop <= c[tt].numitems; loop++) {
                    if ((c[tt].items[loop].id == extracode[1]) && (c[tt].items[loop].equip))
                      track[tt] = TRUE;
                  }
                }
                break;

              case 8: /**** Exact Position ****/
                track[extracode[1] - 1] = TRUE;
                break;
            }
          }
        }
        break;

      case 53: /************ pick on caste ***********/
        loadextracode(id);
        for (tt = 0; tt < 6; tt++)
          track[tt] = 0;

        for (tt = 0; tt <= charnum; tt++) {
          if ((((extracode[2] == 1) && (c[tt].stamina > 0)) || (!extracode[2])) || ((extracode[2] == 2) && (track[tt]))) {
            if (c[tt].caste == extracode[0])
              track[tt] = TRUE;

            if (extracode[1] == 1) /*** fighter types ***/
            {
              if ((c[tt].caste == 1) || (c[tt].caste == 3) || (c[tt].caste == 4))
                track[tt] = TRUE;
            }

            if (extracode[1] == 2) /*** magical types ***/
            {
              if ((c[tt].caste == 3) || (c[tt].caste == 6) || (c[tt].caste == 7) || (c[tt].caste == 8))
                track[tt] = TRUE;
            }

            if (extracode[1] == 3) /*** thief/monk ***/
            {
              if ((c[tt].caste == 2) || (c[tt].caste == 5))
                track[tt] = TRUE;
            }
          }
        }
        break;

      case 54: /*************** alter time encounter ****************/
        loadextracode(id);

        if ((fp = MyrFopen(":Data Files:CTD3", "r+b")) == NULL)
          scratch(129);
        fseek(fp, extracode[0] * sizeof dotime, SEEK_SET);
        fread(&dotime, sizeof dotime, 1, fp);
        CvtTimeEncounterToPc(&dotime);
        if (extracode[1] > -1)
          dotime.percent = extracode[1];
        if (extracode[2] > -1)
          dotime.increment = extracode[2];
        if (extracode[3])
          dotime.day = tyme.tm_yday;
        if (extracode[4] > -1)
          dotime.day += extracode[4];

        fseek(fp, extracode[0] * sizeof dotime, SEEK_SET);
        CvtTimeEncounterToPc(&dotime);
        fwrite(&dotime, sizeof dotime, 1, fp);
        CvtTimeEncounterToPc(&dotime);
        fclose(fp);

        break;

      case 55: /*************** Branch on Picked ****************/

        loadextracode(id);

        reply = FALSE;

        switch (extracode[0]) {
          case 0: /***** Success if ANY picked *****/
            for (t = 0; t <= charnum; t++)
              if (track[t])
                reply = TRUE;
            break;

          case 1: /***** Success if Specific Picked *****/
            if (track[0])
              reply = TRUE;
            break;

          case 2: /***** Success if Specific Picked *****/
            if (track[0])
              reply = TRUE;
            break;

          case 3: /***** Success if Specific Picked *****/
            if (track[0])
              reply = TRUE;
            break;

          case 4: /***** Success if Specific Picked *****/
            if (track[0])
              reply = TRUE;
            break;

          case 5: /***** Success if Specific Picked *****/
            if (track[0])
              reply = TRUE;
            break;

          case 6: /***** Success if Specific Picked *****/
            if (track[0])
              reply = TRUE;
            break;

          default: /***** Success if X Number Picked *****/
            for (t = 0; t <= charnum; t++)
              if (track[t])
                reply++;
            if (reply == abs(extracode[0]))
              reply = TRUE;
            else
              reply = FALSE;
            break;
        }

        if (reply) {
          if (gosub)
            pushonstack(infodoor, t);
          loaddoor2(extracode[3]);
          t = -1;
        } else {
          if (extracode[1] == 1) {
            if (gosub)
              pushonstack(infodoor, t);
            loaddoor2(extracode[4]);
            t = -1;
          }
          if (extracode[1] == 2) {
            textbox(-1, extracode[4], 0, 0, textrect);
            goto getout;
          } else {
            t = 99; /**** discontinue codes ****/
          }
        }

        break;

      case 56: /*************** Branch on Battle Outcome ****************/

        for (tt = 0; tt < 8; tt++) /**** save old door codes ****/
        {
          holddoorforbattle.code[tt] = infodoor.code[tt];
          holddoorforbattle.id[tt] = infodoor.id[tt];
        }
        holddoorposforbattle = t;

        loadextracode(id);
        if (extracode[3])
          sound(extracode[3]);
        if (extracode[4])
          textbox(-1, extracode[4], 0, 0, textrect);

        if (!extracode[1])
          battlenum = extracode[0];
        else
          battlenum = randrange(extracode[0], extracode[1]);

        branchdoorcode = extracode[2];

        combat(0, 0);

        if (revertgame)
          return (0);

        lastpix = -1;
        if (indung)
          loadland(dunglevel, TRUE);
        else
          loadland(landlevel, TRUE);

        needupdate = FALSE;

        for (tt = 0; tt < 8; tt++) /**** restore old door codes ****/
        {
          infodoor.code[tt] = holddoorforbattle.code[tt];
          infodoor.id[tt] = holddoorforbattle.id[tt];
        }
        t = holddoorposforbattle;

        if (!coward)
          booty(0);
        else if (branchdoorcode == -1) {
          updatemain(FALSE, 0);
          warn(118);
          sound(26260);
          warn(124);
          for (temp = 0; temp <= charnum; temp++)
            c[temp].exp -= 2000 * c[temp].level;
          deltax = lastdeltax;
          deltay = lastdeltay;
          moveparty(-1);
          return (TRUE);
        } else {
          if (gosub)
            pushonstack(infodoor, t);
          loaddoor2(branchdoorcode);
          t = -1;
        }
        for (tt = 0; tt <= charnum; tt++)
          c[t].inbattle = TRUE;
        updatemain(FALSE, 0);
        break;

      case 57: /*************** Change Land Look ****************/

        loadextracode(id);

        if (!indung) {
          temp = landlevel;
          saveland(landlevel);

          loadland((long)extracode[2], 1);

          randlevel.landlook = extracode[0];
          randlevel.isdark = extracode[1];

          saveland(extracode[2]);
          landlevel = temp;
          loadland((long)landlevel, TRUE);
          centerpict();
        } else {
          temp = dunglevel;
          saveland(dunglevel);
          indung = FALSE;

          loadland((long)extracode[2], 1);

          randlevel.isdark = extracode[1];
          randlevel.landlook = extracode[0];
          saveland(extracode[2]);

          indung = TRUE;
          dunglevel = temp;
          loadland(dunglevel, 0);
        }
        break;

      case 63: /*************** Alter Game Time ****************/
        loadextracode(id);

        switch (extracode[0]) {
          case 1: /************ set clock **********/
            if (extracode[1] != -1)
              tyme.tm_yday = extracode[1];
            if (extracode[2] != -1)
              tyme.tm_hour = extracode[2];
            if (extracode[3] != -1)
              tyme.tm_min = extracode[3];
            break;

          case 2: /************ offset clock **********/
            tyme.tm_yday += extracode[1];
            tyme.tm_hour += extracode[2];
            if (tyme.tm_hour > 23) {
              tyme.tm_yday++;
              tyme.tm_hour -= 24;
            }
            tyme.tm_min += extracode[3];
            if (tyme.tm_min > 59) {
              tyme.tm_hour++;
              tyme.tm_min -= 60;
            }
            break;
        }

        timeclick(0, 0);

        break;

      case 64: /*************** Branch On Game Time ****************/

        loadextracode(id);
        if ((tyme.tm_yday <= extracode[0]) || (extracode[0] == -1)) /**** sofar so good *****/
        {
          if ((tyme.tm_hour <= extracode[1]) || (extracode[1] == -1)) /**** sofar so good *****/
          {
            if (gosub)
              pushonstack(infodoor, t);
            loaddoor2(extracode[3]);
            t = -1;
            goto startover;
          } else
            goto toolate;
        } else {
        toolate:
          if (gosub)
            pushonstack(infodoor, t);
          loaddoor2(extracode[4]);
          t = -1;
          goto startover;
        }

        break;

      case 65: /*************** Award Random Item(s) ****************/
        loadextracode(id);

        if (extracode[0] < 0)
          temp4 = Rand(abs(extracode[0]));
        else
          temp4 = extracode[0];

        for (loop = 0; loop < 20; loop++)
          treasure.itemid[loop] = 0;
        treasure.exp = treasure.gold = treasure.gems = treasure.jewelry = 0;

        for (loop = 0; loop < temp4; loop++) {
          treasure.itemid[loop] = randrange(extracode[1], extracode[2]);
        }

        nummon = incombat = 0;
        booty(1);
        updatemain(FALSE, -1 + needupdate);
        break;

      case 59: /***** Branch on Tile ID ****/
        loadextracode(id);
        temp4 = field[partyx + lookx][partyy + looky];

        if (temp4 > 999)
          temp4 -= 1000;
        if (temp4 > 999)
          temp4 -= 1000;
        if (temp4 > 999)
          temp4 -= 1000;
        if (temp4 < -999)
          temp4 += 1000;
        if (temp4 < -999)
          temp4 += 1000;
        if (temp4 < -999)
          temp4 += 1000;

        if (extracode[1] == -2)
          t = 99; /**** dropout & erase ****/
        if (extracode[1] == 2)
          goto keepcodes; /**** dropout & keep codes ****/
        if (extracode[1] == 1)
          goto forcebranch;

        break;

      case 60: /*************** Alter party money **************/
        loadextracode(id);

        for (tt = 0; tt <= charnum; tt++) {
          if ((!extracode[1]) || (extracode[1] && track[tt])) {
            c[tt].load -= c[tt].money[extracode[0] - 1];
            if (extracode[0] == 3)
              c[tt].load -= 14 * c[tt].money[2];
            c[tt].money[extracode[0] - 1] = 0;
          }
        }

        break;

      case 62: /*************** Display Scrolling Text **************/

        oldview = viewtype;
        movie(id, 129, 0);
        SetPort(GetWindowPort(look));
        ForeColor(blackColor);
        BackColor(whiteColor);
        viewtype = oldview;
        if (indung) {
          if (viewtype == 1)
            UpdateWindow(FALSE);
        }
        updatemain(FALSE, -1);
        break;

      case 61: /*************** shift party level, X: or Y: **************/
        loadextracode(id);

        if (indung) {
          // dunglevel = extracode[0];

          if (extracode[3]) {
            if (Rand(100) < 50)
              temp = randrange(1, extracode[1]);
            else
              temp = -(randrange(1, extracode[1]));
            floorx += temp;
          } else
            floorx += extracode[1];

          if (extracode[3]) {
            if (Rand(100) < 50)
              temp = randrange(1, extracode[2]);
            else
              temp = -(randrange(1, extracode[2]));
            floory += temp;
          } else
            floory += extracode[2];

          x = floorx;
          y = floory;
          infodoor.landx = floorx;
          infodoor.landy = floory;
        } else {
          if (extracode[3]) {
            if (Rand(100) < 50)
              temp = randrange(1, extracode[1]);
            else
              temp = -(randrange(1, extracode[1]));
            partyx += temp;
          } else
            partyx += extracode[1];

          if (extracode[3]) {
            if (Rand(100) < 50)
              temp = randrange(1, extracode[2]);
            else
              temp = -(randrange(1, extracode[2]));
            partyy += temp;
          } else
            partyy += extracode[2];
        }

        centerpict();

        if (indung) {
          infodoor.landx = floorx + partyx;
          infodoor.landy = floory + partyy;
        } else {
          infodoor.landx = lookx + partyx;
          infodoor.landy = looky + partyy;
        }

        break;

      case 22: /***** Alter Item Status ****/
        loadextracode(id);
        temp = 0;
        for (tt = 0; tt <= charnum; tt++) {
          for (ttt = 0; ttt < c[tt].numitems; ttt++) {
            if ((c[tt].items[ttt].id == extracode[0]) && (temp < extracode[1])) {
              temp++;
              switch (extracode[2]) {
                case 1:
                  dropitem(tt, c[tt].items[ttt].id, ttt, TRUE, TRUE);
                  ttt--;
                  break;

                case 2:
                  c[tt].items[ttt].charge += extracode[3];
                  break;

                case 3:
                  loaditem(extracode[4]);
                  fastsound = 0;
                  if (c[tt].items[ttt].equip) {
                    fastsound = TRUE;
                    removeitem(tt, ttt, FALSE, TRUE);
                  }
                  c[tt].items[ttt].id = extracode[4];
                  c[tt].items[ttt].charge = item.charge;
                  c[tt].items[ttt].ident = 0;
                  if (fastsound)
                    wear(tt, ttt, 0);
                  break;
              }
            }
          }
        }
        break;

      out:
        reply = 0;
    }
  }

  if (mode == -1)
    return (0); /**** monster macro, dont do anything fancy, just return to killbody ***/
  ;

  if ((!mode) && (!encountflag))
    door[doornum].percent = infodoor.percent = -1; /**** eliminate door ****/

keepcodes:

  if ((encountflag == 2) && (enctry-- > 1))
    goto type2;
  if ((encountflag == 1) && (enctry-- > 1))
    goto type1;

  encountflag = 0;

  if (!incombat) {
    if ((infodoor.landid != landlevel) && (!seconddoor)) {
      if (!indung) {
        saveland(landlevel);
        landlevel = infodoor.landid;
        loadland(landlevel, 1);
        goto updatepostion;
      } else {
        saveland(dunglevel);
        dunglevel = infodoor.landid;
        loadland(dunglevel, 0);
      }
    }

    if ((((infodoor.landx != (partyx + lookx)) || (infodoor.landy != (partyy + looky)))) && (!seconddoor)) {
    updatepostion:
      lookx = x = infodoor.landx;
      looky = y = infodoor.landy;
      if (remove) {
        infodoor.landx = temp1;
        infodoor.landy = temp2;
#if CHECK_ILLEGAL_ACCESS > 0
        if (doornum < 0 || doornum >= 100)
          AcamErreur("newland door errror");
#endif
        door[doornum] = infodoor;
      }
      partyx = partyy = deltax = deltay = 0;
      centerpict();
      if ((abs(field[partyx + lookx][partyy + looky]) > 999) && (!noseconddoor)) {
        seconddoor = TRUE;
        goto recheck;
      }
    }
  }

getout:
  delay(10);
  FlushEvents(everyEvent, 0);
  deltax = deltay = 0;
  return (0);
}
