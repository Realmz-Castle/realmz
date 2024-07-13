#include "prototypes.h"
#include "variables.h"
#include <math.h>
// #include "fp.h"

/*************************** combatsetup **************************/
short combatsetup(short mode, short suprise, short combatmode) {
  FILE* fp = NULL;
  Boolean swi = TRUE;
  short temp, lr, ud, start, stop, looplr, loopud;
  Rect goodrect;
  short weapons[9][5][3] = {0, 50, 10, /***** swords ** -1 ***/
      51, 60, 20,
      61, 70, 71,
      71, 95, 75,
      96, 100, 24,

      0, 35, 65, /***** clubs ** -2 ***/
      36, 70, 37,
      71, 85, 125,
      85, 94, 137,
      95, 100, 138,

      0, 40, 120, /***** clubs & spears ** -3 ***/
      41, 60, 37,
      61, 90, 125,
      91, 95, 65,
      96, 100, 138,

      0, 40, 81, /***** axes ** -4 ***/
      41, 80, 92,
      81, 85, 81,
      86, 93, 92,
      94, 100, 136,

      0, 35, 81, /***** small swords & small axes ** -5 ***/
      36, 70, 75,
      71, 90, 81,
      91, 95, 75,
      96, 100, 136,

      0, 20, 37, /***** Clubs/Flails/Spear ** -6 ***/
      21, 40, 65,
      41, 60, 75,
      61, 80, 120,
      81, 100, 120,

      0, 15, 140, /***** Spear Halebard *** -7 **/
      16, 30, 142,
      31, 45, 120,
      46, 75, 140,
      76, 100, 120,

      0, 25, 92, /***** Axes & Spears *** -8 ***/
      26, 50, 81,
      51, 75, 120,
      76, 90, 120,
      91, 100, 120,

      0, 20, 1, /***** swords & dagger & cutlass & nunchucka *** -10 ***/
      21, 40, 31,
      41, 65, 75,
      66, 90, 44,
      91, 100, 31};

  float direction, degree;
  double basemonx, basemony;
  short monx, mony, weapon, roll, shiftlr, loop;

  for (t = 0; t < 20; t++)
    fumque[t] = 0;
  for (t = 0; t < 100; t++)
    monsternameid[t] = 0;

  fumtotal = doingque = quetodo = todoqueindex = charup = monsterturn = 0;

  if (mode) /***** return random weapon for summon *******/
  {
    roll = Rand(100);
    weapon = abs(mode) - 1;
    {
      for (temp = 0; temp < 5; temp++) {
        if (weapon < 0)
          weapon = 0;
        if (twixt(roll, weapons[weapon][temp][0], weapons[weapon][temp][1]))
          return (weapons[weapon][temp][2]);
      }
    }
    return (0);
  }

  incombat = TRUE;

  goodrect.top = goodrect.left = 7;
  goodrect.right = goodrect.bottom = 83;

  fieldx = 45 + mapshiftx;
  fieldy = 45 + mapshifty;

  getfilename("Data BD");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(8);
  fseek(fp, abs(battlenum) * sizeof battle, SEEK_SET);
  fread(&battle, sizeof battle, 1, fp);
  CvtBattleToPc(&battle);
  fclose(fp);

  messageafter = battle.messageafter;
  direction = Rand(360);
  degree = direction / 57.3;

  battle.dist = Rand(battle.dist);

  basemonx = (cos(degree) * battle.dist) - 5;
  basemony = (sin(degree) * battle.dist) - 5;

  monx = basemonx;
  mony = basemony;

  nummon = numenemy = undead = coward = 0;

  swi = FALSE;
  ud = 1;
  shiftlr = -1;

  if (((direction < 45) || (direction > 315)) || ((direction > 135) && (direction < 225)))
    swi = TRUE;
  if (direction < 180)
    ud = -1;
  if ((direction > 90) && (direction < 270))
    shiftlr = 1;

  for (t = 0; t <= charnum; t++) {
    start = -1;
    stop = 1;

    loadprofile(0, c[t].caste); /**************** update max spells attacks value ***********/

    c[t].spellsofar = c[t].guarding = 0;
    c[t].maxspellsattacks = caste.maxspellsattacks;

    c[t].toggle = c[t].bleeding = c[t].traiter = c[t].face = 0;

    if (c[t].armor[2]) {
      loaditem(c[t].armor[2]);
      c[t].weaponsound = item.sound;
    } else if (c[t].armor[15]) {
      c[t].toggle = 1;
      c[t].weaponsound = (30 + c[t].gender * 8);
    } else
      c[t].weaponsound = (30 + c[t].gender * 8);

  tryagain2:

    for (loopud = start; loopud < stop; loopud++) /******* place characters on map *****/
    {
      for (looplr = start; looplr < stop; looplr++) {

        if (!swi) {
          pos[t][0] = t - 3 * (abs(t / 3)) + abs(t / 3) * shiftlr + looplr;
          pos[t][1] = (t / 3) * ud + loopud;
        } else {
          pos[t][0] = (t / 3) * shiftlr + looplr;
          pos[t][1] = t - 3 * (abs(t / 3)) + abs(t / 3) * ud + loopud;
        }

        point.h = pos[t][0] + fieldx;
        point.v = pos[t][1] + fieldy;

        if (PtInRect(point, &goodrect)) {

          temp = field[pos[t][0] + fieldx][pos[t][1] + fieldy];

          if (temp > 999) {
            if (!mapstats[(temp - 1000)].solid) /********** place char in NON SOLID square **********/
            {
              charunder[t] = temp;
              field[pos[t][0] + fieldx][pos[t][1] + fieldy] = t;
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
    c[t].target = -1;
    c[t].attacks = c[t].hasturned = c[t].beenattacked = 0;
    if (c[t].stamina > 0) {
      movecalc(t);
      c[t].movement = c[t].movementmax;
    }
  }

  for (t = 0; t < maxmon; t++)
    monster[t].stamina = monster[t].underneath[0][0] = monster[t].underneath[1][0] = monster[t].underneath[0][1] = monster[t].underneath[1][1] = 0;

  if ((heldover) && (!suspendallies)) /******* add fodder *************/
  {
    for (t = 0; t < heldover; t++) {
      monster[nummon] = holdover[t];
      monster[nummon].target = start = -1;
      monster[nummon].attacknum = monster[nummon].guarding = 0;

      stop = 1;
      size = monster[nummon].size;

    tryagain3:

      for (ud = start; ud < stop; ud++) {
        for (lr = start; lr < stop; lr++) {
          point.h = lr + fieldx;
          point.v = ud + fieldy;

          if (PtInRect(point, &goodrect)) {

            monpos[nummon][0] = lr;
            monpos[nummon][1] = ud;

            placemonster(lr, ud, nummon);

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

      nummon++;
      temp = temp;
    }
  }

onemoretime:

  getfilename("Data MD");

  if (monsterset) {
    MyrNumToString(monsterset, myString);
    strcat((StringPtr)filename, myString);
  }

  if ((fp = MyrFopen(filename, "rb")) == NULL) {
    monsterset = 0;
    warn(102);
    goto onemoretime;
  }

  for (tt = 0; tt < 13; tt++) {
    for (t = 0; t < 13; t++) {
      if ((battle.battle[t][tt]) && (nummon < (maxmon))) {
        monster[nummon].magtohit = 0;

#if CHECK_ILLEGAL_ACCESS > 0
        // Check overload
        if (nummon >= 100 || nummon < 0)
          AcamErreur("combatsetup: nummon overload");
#endif
        fseek(fp, abs(battle.battle[t][tt]) * sizeof monster[0], SEEK_SET);
        /* !MYRIAD 12/10/99 Because fseek can be greater than the end of the file (and fread can fail)*/
        if (fread(&monster[nummon], sizeof(monster[0]), 1, fp) == 1)
          CvtMonsterToPc(&monster[nummon]);
        /* !MYRIAD 12/10/99 If not read, keeps the previous value of monpick */

        if (!monster[nummon].hd)
          goto skipmonster; /**** got an invalid monster ****/

        monster[nummon].target = start = -1;

        monster[nummon].attacknum = monster[nummon].guarding = 0;
        monsternameid[nummon] = battle.battle[t][tt];

        stop = 1;
        size = monster[nummon].size;

      tryagain:

        for (ud = start; ud < stop; ud++) {
          for (lr = start; lr < stop; lr++) {
            point.h = t + monx + lr + fieldx;
            point.v = tt + mony + ud + fieldy;

            if (PtInRect(point, &goodrect)) {

              monpos[nummon][0] = t + monx + lr;
              monpos[nummon][1] = tt + mony + ud;

              placemonster(t + monx + lr, tt + mony + ud, nummon);

              if (monster[nummon].underneath[1][1] > 999) {
                if (((mapstats[monster[nummon].underneath[1][1] - 1000].solid) && (!monster[nummon].size)) || (mapstats[monster[nummon].underneath[1][1] - 1000].solid == 2))
                  goto yep;
              } else
                goto yep;

              if ((size == 1) || (size == 3)) {
                if (monster[nummon].underneath[1][0] > 999) {
                  if (mapstats[monster[nummon].underneath[1][0] - 1000].solid > 1)
                    goto yep;
                } else
                  goto yep;
              }

              if (size == 3) {
                if (monster[nummon].underneath[0][0] > 999) {
                  if (mapstats[monster[nummon].underneath[0][0] - 1000].solid > 1)
                    goto yep;
                } else
                  goto yep;
              }

              if (size > 1) {
                if (monster[nummon].underneath[0][1] > 999) {
                  if (mapstats[monster[nummon].underneath[0][1] - 1000].solid > 1)
                    goto yep;
                } else
                  goto yep;
              }

              goto good;

            yep:

              bodyfield(nummon + 10);
            }
          }
        }

        start--;
        stop++;

        goto tryagain;

      good:

        monster[nummon].up = monster[nummon].lr = 0;

        monster[nummon].ac += Rand(3) - 2; /**** randomate the ac a bit ***/
        monster[nummon].dx += Rand(3) - 2; /**** randomate dx a bit ***/

        if ((monster[nummon].dx) < 1)
          monster[nummon].dx = 1;

        temp = monster[nummon].spellpoints / 10;
        monster[nummon].spellpoints += randrange(-temp, temp); /**** randomate spellpoints a bit ***/

        if (battle.battle[t][tt] < 0) {
          if (monster[nummon].traiter)
            monster[nummon].traiter = 0;
          else
            monster[nummon].traiter = 1;
        }
        monster[nummon].stamina = monster[nummon].bonus;
        for (loop = 0; loop < monster[nummon].hd; loop++)
          monster[nummon].stamina += Rand(8);
        weapon = monster[nummon].weapon;

        /************ update for difficulty level ********/

        if ((monster[nummon].magres < 99) && (monster[nummon].magres > 9))
          monster[nummon].magres += (3 * howhard); // Fantasoft v7.0
        for (loop = 0; loop < 6; loop++)
          monster[nummon].save[loop] += 10 * howhard;
        monster[nummon].ac -= (3 * howhard);
        monster[nummon].dx += howhard;
        monster[nummon].spellpoints *= (1 + (howhard * .40));

        monster[nummon].maxspellpoints = monster[nummon].spellpoints;

        monster[nummon].stamina *= (1 + (howhard * .40));
        if (monster[nummon].stamina < 1)
          monster[nummon].stamina = 1;

        monster[nummon].stamina += (tyme.tm_yday / (180 - (30 * howhard)));

        monster[nummon].staminamax = monster[nummon].stamina;

        if (weapon < 0) {
          roll = Rand(100);
          weapon = abs(weapon) - 1;
          {
            for (temp = 0; temp < 5; temp++) {
              if (twixt(roll, weapons[weapon][temp][0], weapons[weapon][temp][1]))
                monster[nummon].weapon = monster[nummon].items[0] = weapons[weapon][temp][2];
            }
          }
        }
        if (monster[nummon].traiter) {
          numenemy++;
          if ((monster[nummon].type[1]) || (monster[nummon].type[2]))
            undead++;
        }
        nummon++;
      }
    skipmonster:
      swi = 0;
    }
  }

  numfoes = numenemy;

  fclose(fp);

  for (t = 0; t < nummon; t++) {
    if (monpos[t][0] < pos[0][0])
      monster[t].lr = 1;
    else if (monpos[t][0] > pos[0][0])
      monster[t].lr = -1;
  }

  getbattleorder(suprise);

  up = 0;

  for (t = 0; t <= charnum; t++) {
    c[t].inbattle = TRUE;
    if ((c[t].stamina < 1) || ((combatmode) && (!track[t]))) {
      bodyfield(t);
      c[t].inbattle = FALSE;
      q[c[t].position] = -1;
      pos[t][0] = pos[t][1] = c[t].position = -1;
    }
    updatelight(t, FALSE);
  }

  if (!suspendallies)
    heldover = 0;

  DisableItem(gFile, 3);
  DisableItem(gFile, 4);

  getup(FALSE);
  return (0);
}
