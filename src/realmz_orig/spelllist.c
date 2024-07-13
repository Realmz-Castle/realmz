#include "prototypes.h"
#include "variables.h"

/*************************** spelllist ***********************************/
short spelllist(short who, short special) {
  FILE* fp = NULL;
  short too, sumnum, temptemp, temploop, loop1, loop2, t, tempage;
  Boolean done, verge;
  float tempfloat, multiply = 0;

  done = sumnum = verge = 0;

  if (!special)
    return (0);
  if (who < 0)
    return (0);
  if (up < 0)
    up = 0;

  if (who <= charnum) {
    if ((c[who].condition[30]) && (Rand(100) < 34))
      who = q[up];
  }

  if (who < 0)
    return (0);
  if (twixt(who, charnum + 1, 9))
    return (0);

  if ((special == 59) || (special == 60)) /***** power surge/drain ******/
  {
    if (special == 60)
      damage = -(damage);
    if (who < 9) {
      c[who].spellpoints += damage;
      damage = 0;
      if (c[who].spellpoints < 0)
        c[who].spellpoints = 0;
      if (c[who].spellpoints > c[who].spellpointsmax)
        c[who].spellpoints = c[who].spellpointsmax;
      updatecharshort(who, FALSE);
    } else {
      monster[who - 10].spellpoints += damage;
      if (monster[who - 10].spellpoints < 0)
        monster[who - 10].spellpoints = 0;
      damage = 0;
    }
    return (0);
  }

  if ((special == 67) && (allowfumble)) /**** fumble ****/
  {
    if (!savevs(7, who)) /***** DRVs. special ******/
    {
      if (who < 9) {
        if ((c[who].armor[2]) || (c[who].armor[15])) {
          temp = c[who].armor[2];
          if (!temp)
            temp = c[who].armor[15];
          for (fumloop = 0; fumloop <= c[who].numitems; fumloop++) {
            if (c[who].items[fumloop].id == temp) {
              if (removeitem(who, fumloop, FALSE, FALSE)) {
                sound(-10121);
                sound(-10123);
                fumque[fumtotal++] = c[who].items[fumloop].id;
                dropitem(who, c[who].items[fumloop].id, fumloop, TRUE, TRUE);
                c[who].armor[2] = c[who].weaponnum = 0;
                goto donefumble;
              }
            }
          }
        }
      } else if (monster[who - 10].weapon) {
        monster[who - 10].weapon = 0;
        sound(-10121);
        sound(-10123);
        sound(655);
      }
    } else
      return (0);
  }

donefumble:

  if (special == 63) /**** detect magic ****/
  {
    if (who > 9) {
      for (t = 0; t < 6; t++) {
        temp = monster[who - 10].items[t];
        if (temp)
          loaditem(temp);
        if (item.ismagical)
          monster[who - 10].items[t] = -(temp);
      }
    }
  }

  if ((special == 49) || (special == 27)) /********** death magic/Flesh to Stone **********/
  {
    if (who < 9) {
      damage = adjdam = 10 + c[who].stamina;
      if (special == 27)
        c[who].condition[26] = -1;
    } else
      damage = adjdam = 10 + monster[who - 10].stamina;
  }

  if ((special == 64) && (!c[who].condition[26]) && ((c[who].stamina < -9) || (c[who].condition[25]))) /**** ressurrect *****/
  {
    c[who].condition[25] = 0;
    c[who].stamina = -9;
    c[who].spec[2] -= 2;
    needupdate = TRUE;
    updatechar(who, 3);
  }

  if ((special == 53) || (special == 54)) /***** Soul Bind *****/
  {
    if (who < 9) {
      c[who].condition[1] += duration;
      c[who].movement = c[who].attacks = 0;
      updatechar(who, 3);
    } else {
      monster[who - 10].condition[1] += duration;
      monster[who - 10].movement = 0;
    }
  }

  if (special == 61) /***** dispel Magic **  leave broke apart ***/
  {
    if (who < 9) {
      for (t = 0; t < 40; t++) {
        if (c[who].condition[t] > 0)
          c[who].condition[t] = 0;
        if (c[who].traiter) {
          c[who].traiter = 0;
          killparty--;
          numenemy--;
        }
      }
      updatechar(who, 3);
    } else {
      for (t = 0; t < 40; t++) {
        if (monster[who - 10].condition[t] > 0)
          monster[who - 10].condition[t] = 0;
      }
    }
  }

  if (special == 62) /************** remove curse *****************/
  {
    c[who].condition[3] = 0;
    for (t = 0; t < c[who].numitems; t++) {
      loaditem(c[who].items[t].id);
      if ((item.iscurse) && (c[who].items[t].equip))
        removeitem(who, t, FALSE, TRUE);
    }
  }

  if (special == 46) /********** polymorth monster **********/
  {
    if (who > 9) {
      bodyground(who, 0);
      monpick = monster[who - 10];

    onemoretime:

      getfilename("Data MD");

      if (monsterset) {
        MyrNumToString(monsterset, myString);
        // PtoCstr(myString);
        strcat((StringPtr)filename, myString);
      }

      if ((fp = MyrFopen(filename, "rb")) == NULL) {
        monsterset = 0;
        warn(102);
        goto onemoretime;
      }

      do {
        fseek(fp, Rand(200) * sizeof monpick, SEEK_SET);
        fread(&monster[who - 10], sizeof monpick, 1, fp);
      } while ((monpick.size != monster[who - 10].size) || (!monster[who - 10].hd) || (monster[who - 10].cansum != 1));

      fclose(fp); /***** close monster file ******/

      for (t = 0; t < 40; t++)
        if (monpick.condition[t] > -1)
          monster[who - 10].condition[t] = monpick.condition[t];
      monster[who - 10].dx += (Rand(3) - 2);
      monster[who - 10].stamina = monster[who - 10].bonus;
      for (temploop = 0; temploop < monster[who - 10].hd; temploop++)
        monster[who - 10].stamina += Rand(8);
      monster[who - 10].staminamax = monster[who - 10].stamina;
      monster[who - 10].weapon = monster[monsterup].attacknum = 0;
      monster[who - 10].traiter = monpick.traiter;
      monster[who - 10].up = monster[who - 10].lr = 0;
      monster[who - 10].underneath[0][0] = monpick.underneath[0][0];
      monster[who - 10].underneath[1][0] = monpick.underneath[1][0];
      monster[who - 10].underneath[0][1] = monpick.underneath[0][1];
      monster[who - 10].underneath[1][1] = monpick.underneath[1][1];
      monster[who - 10].money[0] = monster[who - 10].money[1] = monster[who - 10].money[2] = 0;
      monster[who - 10].ac += (Rand(3) - 2); /**** randomate the ac a bit ***/
      monster[who - 10].dx += (Rand(3) - 2); /**** randomate dx a bit ***/
      if ((monster[who - 10].dx) < 1)
        monster[who - 10].dx = 1;

      monster[who - 10].magres += (3 * howhard);
      for (loop2 = 0; loop2 < 6; loop2++)
        monster[who - 10].save[loop2] += 7 * howhard;
      monster[who - 10].ac -= (2 * howhard);
      monster[who - 10].dx += howhard;

      multiply = (1 + (howhard * .33));

      tempfloat = monster[who - 10].spellpoints;
      tempfloat *= multiply;

      monster[who - 10].spellpoints = tempfloat;

      tempfloat = monster[who - 10].stamina;
      tempfloat *= multiply;

      monster[who - 10].stamina = tempfloat;

      if (monster[who - 10].stamina < 1)
        monster[who - 10].stamina = 1;

      monster[who - 10].stamina += (tyme.tm_yday / (180 - (30 * howhard)));

      monster[who - 10].staminamax = monster[who - 10].stamina;

      if (monster[who - 10].weapon < 0) /****** get random weapon *******/
      {
        monster[who - 10].weapon = monster[who - 10].items[0] = combatsetup(monster[who - 10].weapon, 0, 0);
      }
    }
  }

  if (special == 58) /***** summon monster *****/
  {
    while (done == FALSE) {
      for (loop1 = nummon; loop1 < maxmon; loop1++)
        if (monster[loop1].stamina <= 0)
          sumnum = 1;

      /****** make sure you dont summont 2 to the same spot or overlap ***/
      if (field[target[loop][0]][target[loop][1]] <= maxloop)
        sumnum = 0;
      if ((field[target[loop][0]][target[loop][1] - 1] <= maxloop) && ((monpick.size == 1) || (monpick.size == 3)))
        sumnum = 0;
      if ((field[target[loop][0] - 1][target[loop][1]] <= maxloop) && (monpick.size > 1))
        sumnum = 0;
      if ((field[target[loop][0] - 1][target[loop][1] - 1] <= maxloop) && (monpick.size == 3))
        sumnum = 0;

      if (sumnum) {
        sound(spellinfo.sound2 + 600);
        for (loop1 = nummon; loop1 < maxmon; loop1++) {
          if (monster[loop1].stamina < 1) {
            nummon++;
            monster[loop1] = monpick;
            monster[loop1].target = -1;
            monpos[loop1][0] = target[loop][0] - fieldx;
            monpos[loop1][1] = target[loop][1] - fieldy;
            for (loop2 = maxloopminus; loop2 > 0; loop2--) {
              if (q[loop2] == -1) {
                q[loop2] = loop1 + 10;
                showresults(loop1 + 10, -12, 0); /***** show summoned ****/
                break;
              }
            }
            monster[loop1].stamina = monster[loop1].bonus;
            for (loop2 = 0; loop2 < monster[loop1].hd; loop2++)
              monster[loop1].stamina += Rand(8);
            monster[loop1].staminamax = monster[loop1].stamina;
            monster[loop1].traiter = c[charup].traiter;
            monster[loop1].lr = monster[loop1].up = 0;
            monster[loop1].ac += (Rand(3) - 2); /**** randomate the ac a bit ***/
            monster[loop1].dx += (Rand(3) - 2); /**** randomate dx a bit ***/
            if ((monster[loop1].dx) < 1)
              monster[loop1].dx = 1;

            temp = monster[loop1].spellpoints / 10;
            monster[loop1].spellpoints += randrange(-temp, temp); /**** randomate spellpoints a bit ***/

            /************ update for difficulty level ********/

            if ((monster[loop1].magres < 99) && (monster[loop1].magres > 9))
              monster[loop1].magres += (3 * howhard); // Fantasoft v7.0
            for (loop2 = 0; loop2 < 6; loop2++)
              monster[loop1].save[loop2] += 7 * howhard;
            monster[loop1].ac -= (2 * howhard);
            monster[loop1].dx += howhard;

            multiply = (1 + (howhard * .33));

            tempfloat = monster[loop1].spellpoints;
            tempfloat *= multiply;

            monster[loop1].spellpoints = tempfloat;

            monster[loop1].maxspellpoints = monster[loop1].spellpoints;

            tempfloat = monster[loop1].stamina;
            tempfloat *= multiply;

            monster[loop1].stamina = tempfloat;

            if (monster[loop1].stamina < 1)
              monster[loop1].stamina = 1;

            monster[loop1].stamina += (tyme.tm_yday / (180 - (30 * howhard)));

            monster[loop1].staminamax = monster[loop1].stamina;

            if (monster[loop1].weapon < 0) /****** get random weapon *******/
            {
              monster[loop1].weapon = monster[loop1].items[0] = combatsetup(monster[loop1].weapon, 0, 0);
            }

            placemonster(monpos[loop1][0], monpos[loop1][1], loop1);
            spelleffect(loop1 + 10, 0);
            return (0);
          }
        }
      }
      sound(133);
      showresults(q[up], -9, 0); /************* spell failed ******************/
      return (0);
    }
  }

  if (special == 90) {
    temptemp = (5 * powerlevel) + (3 * c[charup].level);
    if (((monster[who - 10].type[1]) || (monster[who - 10].type[2])) && (monster[who - 10].traiter) && (monster[who - 10].cansum != 255) && (monster[who - 10].stamina > 0)) {
      tempid = 100 - temptemp + 5 * monster[who - 10].hd; /**** need to roll above ****/

      if (tempid < 25)
        tempid = 25;

      temp = Rand(100);
      temp -= tempid; /***** diff of roll **********/
      if (temp > 0) {
        if (temp < 30) {
          undead--;
          monster[who - 10].stamina = 0;
          killbody(who, 0);
        } else {
          numenemy--;
          undead--;
          sound(630);
          showresults(who, -13, 0); /********** turned *******/
          monster[who - 10].traiter = c[charup].traiter; /*** make friendly ****/
          spelleffect(who, 0);
        }
      } else
        return (0);
    }
  }

  if ((special == 51) || (special == 52)) /****** charm ****/
  {
    if (q[up] < 9)
      too = c[charup].traiter;
    else
      too = monster[monsterup].traiter;

    if (!too) /**** charm to good ****/
    {
      if (who > 9) {
        if (monster[who - 10].traiter) {
          monster[who - 10].target = -1;
          monster[who - 10].traiter = 0;
          numenemy--;
        }
      } else if (c[who].traiter) {
        c[who].traiter = 0;
        killparty--;
        numenemy--;
      }
    } else /*** charm to evil ***/
    {
      if (who < 9) {
        if (c[who].traiter != too) {
          if (!c[who].traiter) {
            numenemy++;
            killparty++;
          }
          c[who].target = -1;
          c[who].traiter = too;
          updatechar(who, 3);
        }
      } else {
        if (!monster[who - 10].traiter)
          numenemy++;
        monster[who - 10].traiter = too;
        monster[who - 10].target = -1;
      }
    }
  }

  if (special == 2) {
    if (who < 9) {
      c[who].movement = 0;
      updatechar(who, 3);
    } else
      monster[who - 10].movement = 0;
  }

  if ((special == 7) || (special == 3)) /***** slow * entangle ***/
  {
    if (who < 9) {
      c[who].movement = c[who].movement / 2;
      updatechar(who, 3);
    } else
      monster[who - 10].movement = monster[who - 10].movement / 2;
  }

  if ((special == 26) && (c[who].stamina < -9)) /***** animate ****/
  {
    c[who].condition[25] = -1;
    c[who].stamina = c[who].staminamax / 4;
    updatechar(who, 3);
  }

  if (special == 10) /******* poison *****/
  {
    if (who < 9) {
      if (c[who].condition[25] < 0)
        c[who].condition[9] = 0;
      updatechar(who, 3);
    } else if ((monster[who - 10].spellimmune[5]) || (monster[who - 10].condition[25]))
      monster[who - 10].condition[9] = 0;
  }

  if ((special == 24) || (special == 91)) /******* haste/age *****/
  {
    /************* lets age them a bit *******************/
    if (who <= charnum) {
      loadprofile(c[who].race, 0);
      if (special == 24)
        tempage = powerlevel * 30;
      else
        tempage = duration * 30;
      tempfloat = races.maxage * .01;
      tempfloat *= tempage;
      tempage = tempfloat;
      c[who].age += tempage;
      characterl = c[who];
      temp = age(c[who].age, c[who].race, c[who].currentagegroup);
      c[who] = characterl;

      if (temp) {
        if (FrontWindow() == look)
          in();
        showageupdate(who, c[who].currentagegroup, 0);
        if (FrontWindow() == look)
          updatemain(0, -1);
        else if (FrontWindow() == GetDialogWindow(charstat))
          cleanage = TRUE;
      }
    }
  }

  if (special == 92) /******* youth effect *****/
  {
    /************* lets youth them a bit *******************/
    if (who < 9) {
      loadprofile(c[who].race, 0);
      tempage = duration * 30;
      tempfloat = races.maxage * .01;
      tempfloat *= tempage;
      tempage = tempfloat;
      c[who].age -= tempage;

      if (c[who].age < 3650)
        c[who].age = 3650;

      temp = Rand(3);
      c[who].staminamax -= temp;
      if (c[who].staminamax < 1)
        c[who].staminamax = 1;

      c[who].stamina -= temp;
      if (c[who].stamina < 1)
        c[who].stamina = 1;

      characterl = c[who];
      temp = age(c[who].age, c[who].race, c[who].currentagegroup);
      c[who] = characterl;

      if (temp) {
        if (FrontWindow() == look)
          in();
        showageupdate(who, c[who].currentagegroup, 1); /********* 1 = show back up stats for getting younger *********/

        if (FrontWindow() == look)
          updatemain(0, -1);
        else if (FrontWindow() == GetDialogWindow(charstat))
          cleanage = TRUE;
      }
    }
  }
  return (1);
}
