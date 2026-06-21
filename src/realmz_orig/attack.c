#include "prototypes.h"
#include "variables.h"

/* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
 * NOTE(danapplegate): A number of fixes have been made to this file to correct apparent
 * mismatches between character and item special attributes, item damage types, monster
 * types, and conditions. See PRs #191, #192, #194, #195 for details.
 */

/*************** attack (man to other) ********************/
short attack(short chare, short mon) {
  Boolean kill = 0;
  struct character character;
  struct monster monst;
  short att, specialdam;
  short t;
  char addcond, amountcond, whichcond;
  Boolean success;

  if (chare < 7)
    character = c[chare];
  if (mon > 9)
    monst = monster[mon - 10];

  SetPort(GetWindowPort(look));

  damage = success = att = specialdam = addcond = amountcond = whichcond = 0;
  didattack = physical = 1;

  blankround = canundo = 0;

  reply = FALSE;
  for (t = 0; t <= charnum; t++)
    if ((c[t].inbattle) && (!c[t].traiter))
      reply = TRUE;
  if (!reply) {
    killmon = numenemy;
    return (0);
  }

  drawbody(chare, 1, 0); /***** This is to change fancing and highlight player prior to attack **** 0 = look, 1 = buff ******/

  if (character.armor[2]) {
    loaditem(character.armor[2]);
    if (item.sp2 > 1100) {
      loadspell2(item.sp2);
      if (spellinfo.damagetype == 9) {
        if (character.condition[COND_ANIMATED]) {
          if (character.handtohand < 8) {
            if (toweap(chare)) {
              character = c[chare];
              loaditem(character.armor[2]);
              goto moveon;
            } else
              return (0);
          } else
            return (0);
        } else {
          if (q[up] == charup) {
            SetPort(GetWindowPort(look));
            ForeColor(blackColor);
            BackColor(whiteColor);
            warn(39);
            combatupdate2(charup);
            return (0);
          } else
            return (0);
        }
      }
    }
    damage += item.damage; /******* magic plus ****/
    /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
     * The weapon magic plus (item.damage) was added to player melee damage but
     * never to the to-hit roll. The monster path (attack2) and missile path
     * (resolvespell) both add 5 per point, and the character sheet attack bonus
     * already includes it (damage * 5), so player melee was the only path that
     * ignored it. Add the same term here. */
    att += 5 * item.damage; /******* magic plus to hit; matches attack2 and resolvespell ****/
    /* *** END CHANGES *** */
    if (item.sp1 == 121)
      att += 5 * item.damage; /******* double to hit weapon ****/

    if (item.sp1 == -10) /***** weapon inflicts condition on hit ******/
    {
      if (item.sp2 == 1) /***** DRVs.  ******/
      {
        if (!savevs(item.sp4, mon))
          goto autoadd;
      } else if (item.sp2 == 2) /***** % chance.  ******/
      {
        if (Rand(100) <= item.sp4)
          goto autoadd;
      } else /**** auto add condition ****/
      {
      autoadd:
        addcond = TRUE;
        amountcond = item.sp5;
        whichcond = item.sp3;
      }
    }
  }

moveon:
  att += (50 + character.tohit + 20 * behind);
  if (character.condition[COND_TANGLED])
    att -= abs(character.condition[COND_TANGLED]); /*** tangled ***/
  if (character.condition[COND_STRONG])
    att += 15; /**** strong ***/
  if (character.condition[COND_SLOW])
    att -= 15; /**** slow ***/
  if (character.condition[COND_CONFUSED])
    att -= 10; /**** confused ***/
  if (character.condition[COND_BLIND])
    att -= 15; /**** blind ***/
  if (character.condition[COND_MAGIC_AURA])
    att += 5; /**** bless ***/
  if (character.condition[COND_CURSED])
    att -= 5; /**** curse ***/
  if (character.condition[COND_HINDERED_ATTACKS])
    att -= abs(character.condition[COND_HINDERED_ATTACKS]); /*** Hinder atk ***/
  if (monst.type[4] && character.condition[COND_PROTECTION_FROM_EVIL])
    att += 10; /*** +10% for pro evil ***/
  att += Rand(character.lu + character.maglu); /*** luck ***/

  if (mon > 9) {
    for (t = 0; t < 8; t++) {
      if (monst.type[t]) {
        att += (5 * character.special[t]); /**** race hatred ****/
        damage += character.special[t];
      }
    }
  }

  /********* defence ************/

  if (mon > 9) {
    att -= monst.ac;
    att -= 2 * abs(monst.condition[COND_SHIELD_FROM_HITS]); /*** shield ***/
    if (monst.condition[COND_CONFUSED])
      att += 10; /*** confused enemy ***/
    if (monst.condition[COND_BLIND])
      att += 15; /*** blind enemy ***/
    if (monst.condition[COND_SLOW])
      att += 15; /*** slow enemy ***/
    if (monst.condition[COND_MAGIC_AURA])
      att -= 5; /*** blessed enemy ***/
    if (monst.condition[COND_CURSED])
      att += 5; /*** cursed enemy ***/
    if (monst.condition[COND_INVISIBLE])
      att -= 10; /*** invisible enemy ***/
    if (monst.condition[COND_TANGLED])
      att += abs(monst.condition[COND_TANGLED]); /*** tangled ***/
    if (monst.condition[COND_HINDERED_DEFENSE])
      att += abs(monst.condition[COND_HINDERED_DEFENSE]); /*** hinter defense ***/
    if (monst.condition[COND_DEFENSE_BONUS])
      att -= abs(monst.condition[COND_DEFENSE_BONUS]); /*** defense bonus ***/
    if (monst.condition[COND_PROTECTION_FROM_EVIL])
      att -= 10; /*** pro evil enemy***/
    att -= tyme.tm_yday / 120; /****** time bonus ****/
  } else {
    att -= c[mon].ac;
    att -= 2 * abs(c[mon].condition[COND_SHIELD_FROM_HITS]); /*** shield ***/
    if (c[mon].condition[COND_CONFUSED])
      att += 10; /*** confused MAN ***/
    if (c[mon].condition[COND_BLIND])
      att += 15; /*** blind MAN ***/
    if (c[mon].condition[COND_SLOW])
      att += 15; /*** slow MAN ***/
    if (c[mon].condition[COND_MAGIC_AURA])
      att -= 5; /*** blessed MAN ***/
    if (c[mon].condition[COND_CURSED])
      att += 5; /*** cursed MAN ***/
    if (c[mon].condition[COND_INVISIBLE])
      att -= 10; /*** invisible MAN ***/
    if (c[mon].condition[COND_TANGLED])
      att += abs(c[mon].condition[COND_TANGLED]); /*** tangled ***/
    if (c[mon].condition[COND_HINDERED_DEFENSE])
      att += abs(c[mon].condition[COND_HINDERED_DEFENSE]); /*** hinder defense ***/
    if (c[mon].condition[COND_DEFENSE_BONUS])
      att -= abs(c[mon].condition[COND_DEFENSE_BONUS]); /*** defense bonus ***/
  }

  /***SetPort(look);

  TextMode(0);

  MoveTo(20,50);
  string(att);****/

  if (att < 5)
    att = 5;

  if (Rand(100) <= att)
    success = TRUE;

  if (mon > 9) {
    if (monst.condition[COND_HELPLESS])
      success = TRUE;
  } else if (c[mon].condition[COND_HELPLESS])
    success = TRUE;

  if (allowfumble) {
    templong = (Rand(1000 + 100 * c[chare].level));

    if ((templong > 50 && templong < 60) && (c[chare].armor[2])) /**** fumble weapon ****/
    {
      short fumbleditem;
      /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
       * Guard fumbled item drops against inventory and fumble queue bounds.
       */
      for (fumloop = 0; (fumloop < c[chare].numitems) && (fumloop < 30); fumloop++) {
        if (c[chare].items[fumloop].id == c[chare].armor[2]) {
          if (fumtotal >= 20)
            goto donefumble;
          fumbleditem = c[chare].items[fumloop].id;
          if (removeitem(chare, fumloop, FALSE, FALSE)) {
            sound(-10121);
            sound(-10123);
            fumque[fumtotal++] = fumbleditem;
            dropitem(chare, fumbleditem, fumloop, TRUE, FALSE);
            c[chare].armor[2] = c[chare].weaponnum = 0;
            warn(114);
            success = FALSE;
            combatupdate2(chare);
            goto donefumble;
          }
        }
      }
    }
  }

donefumble:

  if ((mon > 9) && (monst.magtohit > 0)) {
    Rect r;
    if (((success) && ((monst.magtohit > item.damage) && (c[chare].armor[2]))) || ((!c[chare].armor[2]) && (monst.magtohit > c[chare].level / 8))) {
      success = damage = FALSE; /******* need higher + weapon to Hit ****/
      showresults(mon, -15, chare);
    restorecontrol:
      if (c[chare].condition[COND_ANIMATED] == 1)
        c[chare].condition[COND_ANIMATED] = poss = 0;
      GetControlBounds(campbut, &r);
      ploticon3(129, r);
      sound(141);
      combatupdate2(chare);
      if (doauto[chare])
        theControl = autoone[chare];
      checkfordoauto();
      return (0);
    }
  }

  if ((mon > 9) && (monst.dist == -1)) /**** require blunt weapon ****/
  {
    if ((success) && (item.blunt != -1) && (c[chare].armor[2])) {
      success = damage = FALSE;
      showresults(mon, -50, chare);
      goto restorecontrol;
    }
  }

  if ((mon > 9) && (monst.dist == -2)) /**** require sharp weapon ****/
  {
    if ((success) && (item.blunt != -2) && (c[chare].armor[2])) {
      success = damage = FALSE;
      showresults(mon, -51, chare);
      goto restorecontrol;
    }
  }

  if ((mon > 9) && (monst.dist != 0) && (monst.dist != -1) && (monst.dist != -2)) {
    if ((success) && (monst.dist != item.itemid - 1024)) {
      success = damage = FALSE; /******* need special weapon to Hit ****/
      showresults(mon, -45, chare);
      goto restorecontrol;
    }
  }

  if (item.sp1 == 120)
    success = TRUE; /********* auto hit weapon ************/

  if (success) {
    if ((mon > 9) && (monst.condition[COND_REFLECTING_ATTACKS]) && (Rand(100) < 34))
      mon = chare;
    else if (mon < 9) {
      if ((c[mon].condition[COND_REFLECTING_ATTACKS]) && (Rand(100) < 34))
        mon = chare;
    }

    if (character.condition[COND_STRONG])
      damage += 3; /****** strong *****/
    if (!character.armor[2])
      damage += Rand(character.handtohand);
    else {
      if (mon > 9) /***** hit monster ****/
      {
        monster[mon - 10].beenattacked = TRUE;

        if ((item.heat) && (!monst.spellimmune[1])) {
          specdamage = Rand(item.heat);
          if (monst.condition[COND_FIRE_PROTECTION])
            specdamage /= 2;
          if (savevs(1, chare))
            specdamage /= 2;
          specialdam += specdamage;
          if (specdamage)
            showresults(mon, -19, chare);
        }

        if ((item.cold) && (!monst.spellimmune[2])) {
          specdamage = Rand(item.cold);
          if (monst.condition[COND_COLD_PROTECTION])
            specdamage /= 2;
          if (savevs(2, chare))
            specdamage /= 2;
          specialdam += specdamage;
          if (specdamage)
            showresults(mon, -20, chare);
        }
        if ((item.electric) && (!monst.spellimmune[3])) {
          specdamage = Rand(item.electric);
          if (monst.condition[COND_ELECTRICAL_PROTECTION])
            specdamage /= 2;
          if (savevs(3, chare))
            specdamage /= 2;
          specialdam += specdamage;
          if (specdamage)
            showresults(mon, -21, chare);
        }
        if ((item.vsundead) && (monst.type[1]))
          damage += Rand(item.vsundead);
        if (item.vsdd && monst.type[2])
          damage += Rand(item.vsdd);
        if (item.vsevil && monst.type[4])
          damage += Rand(item.vsevil);
        damage += Rand(item.vssmall);
      } else /**** hit man ****/
      {
        if (item.heat) {
          specdamage = Rand(item.heat);
          if (savevs(1, mon))
            specdamage /= 2;
          if (c[mon].condition[COND_FIRE_PROTECTION])
            specdamage /= 2;
          specialdam += specdamage;
          if (specdamage)
            showresults(mon, -19, chare);
        }
        if (item.cold) {
          specdamage = Rand(item.cold);
          if (c[mon].condition[COND_COLD_PROTECTION])
            specdamage /= 2;
          if (savevs(2, mon))
            specdamage /= 2;
          specialdam += specdamage;
          if (specdamage)
            showresults(mon, -20, chare);
        }
        if (item.electric) {
          specdamage = Rand(item.electric);
          if (c[mon].condition[COND_ELECTRICAL_PROTECTION])
            specdamage /= 2;
          if (savevs(3, mon))
            specdamage /= 2;
          specialdam += specdamage;
          if (specdamage)
            showresults(mon, -21, chare);
        }
        damage += Rand(item.vssmall);
      }
    }

    damage += character.damage;
    damage += character.condition[COND_ATTACK_BONUS]; /***** attack bonus ****/

    if (Rand(100) <= character.spec[0]) {
      damage *= 3;
      behind = -1; /****** Sneack Attack ****/
    }

    if (Rand(100) <= character.spec[3]) {
      damage *= 2;
      behind = -2; /****** Major Wound ****/
    }

    if (damage < 0)
      damage = 0;

    if (mon > 9) {
      if (addcond) /**** weapon hit adds condition ******/
      {
        if (monster[mon - 10].condition[whichcond - 20] > -1)
          monster[mon - 10].condition[whichcond - 20] += amountcond;
        showresults(mon, -40, chare);
      }

      if (monst.condition[COND_HELPLESS])
        damage = monst.stamina;
      monster[mon - 10].beenattacked = TRUE;
      monster[mon - 10].stamina -= (damage + specialdam);
      c[chare].damagegiven += (damage + specialdam);
      c[chare].hitsgiven++;

      if (monster[mon - 10].stamina <= 0) {
        showresults(mon, 0, chare);
        c[chare].kills++;
        killbody(mon, 0);
        monsterturn = 0;
        kill = TRUE; /**** return kill ******/
      }
    } else {
      if (addcond) /**** weapon hit adds condition ******/
      {
        if (c[mon].condition[whichcond - 20] > -1)
          c[mon].condition[whichcond - 20] += amountcond;
        showresults(mon, -40, chare);
      }

      if (c[mon].condition[COND_HELPLESS])
        damage = c[mon].stamina;
      c[mon].beenattacked = TRUE;
      c[mon].stamina -= (damage + specialdam);
      c[mon].damagetaken += (damage + specialdam);
      if (c[mon].stamina < 1) {
        if (c[mon].stamina > -10)
          c[mon].bleeding = TRUE;
        killbody(mon, 0);
        showresults(mon, 0, chare);
        kill = TRUE; /**** return kill ******/
      }
    }
    if (!kill)
      showresults(mon, 0, chare);
  } else {
    showresults(mon, -1, chare); /**** miss ****/
    c[chare].imissed++;
  }
  return (kill);
}

/*************** attack2 (monster to other) ******************/
short attack2(short mon, short chare, short attacknum) {
  struct character character;
  struct monster monst;
  short att, specialdam, tempage, didage;
  float tempfloat;
  short t, addcond, amountcond, whichcond;
  Boolean success, kill;

  if (chare < 6)
    character = c[chare];
  if (mon > 9)
    monst = monster[mon - 10];

  SetPort(GetWindowPort(look));

  blankround = canundo = specialdam = specdamage = damage = success = att = addcond = amountcond = whichcond = kill = 0;
  didattack = physical = 1;

  reply = FALSE;
  for (t = 0; t <= charnum; t++)
    if ((c[t].inbattle) && (!c[t].traiter))
      reply = TRUE;

  if (!reply) {
    killmon = numenemy;
    return (0);
  }

trynewweapon:

  if (monst.weapon) {
    loaditem(monst.weapon);
    if (item.sp2 > 1100) {
      loadspell2(item.sp2);
      if (spellinfo.damagetype == 9) { /***** no attack with missile weap ***/
        monster[monsterup].weapon = monst.weapon = monster[monsterup].items[0];
        goto trynewweapon;
      }
    }
    att += 5 * item.damage;
    if (item.sp1 == 121)
      att += 5 * item.damage; /******* double to hit weapon ****/
    if (item.sp1 == 120)
      success = TRUE; /********** auto hit weapon *****/

    if (item.sp1 == -10) /***** weapon inflicts condition on hit ******/
    {
      if (item.sp2 == 1) /***** DRVs.  ******/
      {
        if (!savevs(item.sp4, chare))
          goto autoadd;
      } else if (item.sp2 == 2) /***** % chance.  ******/
      {
        if (Rand(100) <= item.sp4)
          goto autoadd;
      } else /**** auto add condition ****/
      {
      autoadd:
        addcond = TRUE;
        amountcond = item.sp5;
        whichcond = item.sp3;
      }
    }

  } else
    item.blunt = 0; /***** erase blunt/sharp if not using a weapon ***/

  att += 50 + (5 * monst.hd) + (5 * monst.damplus) + 20 * behind;
  if (monst.condition[COND_CONFUSED])
    att -= 10; /*** confused ***/
  if (monst.condition[COND_BLIND])
    att -= 15; /*** blind ***/
  if (monst.condition[COND_PROTECTION_FROM_EVIL])
    att += 10; /*** pro evil ***/
  if (monst.condition[COND_STRONG])
    att += 15; /*** strong ***/
  if (monst.condition[COND_TANGLED])
    att -= abs(monst.condition[COND_TANGLED]); /*** tangle ***/
  if (monst.condition[COND_MAGIC_AURA])
    att += 5; /*** bless ***/
  if (monst.condition[COND_CURSED])
    att -= 5; /*** curse ***/
  if (monst.condition[COND_SLOW])
    att -= 15; /*** slow ***/
  if (monst.condition[COND_HINDERED_ATTACKS])
    att -= abs(monst.condition[COND_HINDERED_ATTACKS]); /*** hinder attack ***/
  att += tyme.tm_yday / 70; /****** time bonus ****/

  if (chare < 9) {
    if ((pos[chare][0] > monpos[mon - 10][0]) && (monst.lr == 0)) {
      monster[mon - 10].lr = 1;
      bodyground(mon, 0);
      drawbody(mon, 0, 0);
    } else if ((pos[chare][0] < monpos[mon - 10][0]) && (monst.lr == 1)) {
      monster[mon - 10].lr = 0;
      bodyground(mon, 0);
      drawbody(mon, 0, 0);
    }

    if (character.condition[COND_INVISIBLE])
      att -= 10; /*** invisible ***/
    if (character.condition[COND_SLOW])
      att += 15; /*** slow ***/
    if (character.condition[COND_MAGIC_AURA])
      att -= 5; /*** bless ***/
    if (character.condition[COND_BLIND])
      att += 15; /*** blind ***/
    if (character.condition[COND_CONFUSED])
      att += 10; /*** confused ***/
    if (character.condition[COND_CURSED])
      att += 5; /*** curse ***/
    if (character.condition[COND_TANGLED])
      att += abs(character.condition[COND_TANGLED]); /*** tangle ***/
    if (character.condition[COND_HINDERED_DEFENSE])
      att += abs(character.condition[COND_HINDERED_DEFENSE]); /*** hinder defense ***/
    if (character.condition[COND_DEFENSE_BONUS])
      att -= abs(character.condition[COND_DEFENSE_BONUS]); /*** defense bonus ***/
    att -= Rand(character.lu + character.maglu); /*** luck ***/
    att -= character.ac;
    att -= 2 * abs(character.condition[COND_SHIELD_FROM_HITS]); /*** shield ***/
    if (monst.type[4] && character.condition[COND_PROTECTION_FROM_EVIL])
      att -= 10; /*** +10% for pro evil ***/
  } else {
    if ((monpos[chare - 10][0] > monpos[mon - 10][0]) && (monst.lr == 0)) {
      monster[mon - 10].lr = 1;
      bodyground(mon, 0);
      drawbody(mon, 0, 0);
    } else if ((monpos[chare - 10][0] < monpos[mon - 10][0]) && (monst.lr == 1)) {
      monster[mon - 10].lr = 0;
      bodyground(mon, 0);
      drawbody(mon, 0, 0);
    }

    if (monster[chare - 10].condition[COND_SLOW])
      att += 15; /*** slow ***/
    if (monster[chare - 10].condition[COND_MAGIC_AURA])
      att -= 5; /*** bless ***/
    if (monster[chare - 10].condition[COND_CURSED])
      att += 5; /*** curse ***/
    if (monster[chare - 10].condition[COND_BLIND])
      att += 15; /*** blind ***/
    if (monster[chare - 10].condition[COND_CONFUSED])
      att += 10; /*** confused ***/
    if (monster[chare - 10].condition[COND_INVISIBLE])
      att -= 10; /*** invisible ***/
    if (monster[chare - 10].condition[COND_TANGLED])
      att += abs(monster[chare - 10].condition[COND_TANGLED]); /*** tangle ***/
    if (monster[chare - 10].condition[COND_HINDERED_DEFENSE])
      att += abs(monster[chare - 10].condition[COND_HINDERED_DEFENSE]); /*** hinder defense ***/
    if (monster[chare - 10].condition[COND_DEFENSE_BONUS])
      att -= abs(monster[chare - 10].condition[COND_DEFENSE_BONUS]); /*** defense bonus ***/
    att -= monster[chare - 10].ac;
    att -= 2 * abs(monster[chare - 10].condition[COND_SHIELD_FROM_HITS]); /*** shield ***/
    if (monst.type[4] && monster[chare - 10].condition[COND_PROTECTION_FROM_EVIL])
      att -= 10; /*** +10% for pro evil ***/
  }

  if (att < 10)
    att = 10;

  if (Rand(100) <= att)
    success = TRUE;

  if (allowfumble) {
    templong = Rand(600 + 50 * monster[mon - 10].hd);

    if (templong > 20 && templong < 35) /***** fumble weapon *****/
    {
      if (monster[mon - 10].weapon) {
        monster[mon - 10].weapon = 0;
        combatupdate2(mon);
        sound(-10121);
        sound(-655);
        showresults(mon, -46, mon);
        success = FALSE;
      }
    }
  }

  if ((success) && (chare > 9)) {
    if ((monster[chare - 10].dist != 0) && (monster[chare - 10].dist != -1) && (monster[chare - 10].dist != -2) && (monster[chare - 10].dist != item.itemid - 1024)) {
      success = damage = FALSE; /******* need special weapon to Hit ****/
      showresults(mon, -45, chare);
      return (0);
    }
  }

  if ((success) && (chare > 9)) {
    if (monster[chare - 10].dist == -1) {
      if ((item.blunt != -1) && (monst.weapon)) {

        success = damage = FALSE; /******* need blunt weapon to Hit ****/
        showresults(mon, -50, chare);

        if (monst.weapon) {
          monster[mon - 10].weapon = monst.weapon = 0;
          combatupdate2(mon);
          showresults(mon, -52, mon);
          success = FALSE;
          return (0);
        }
      }
    }
  }

  if ((success) && (chare > 9)) {
    if (monster[chare - 10].dist == -2) {
      if ((item.blunt != -2) && (monst.weapon)) {
        success = damage = FALSE; /******* need sharp weapon to Hit ****/
        showresults(mon, -51, chare);

        if (monst.weapon) {
          monster[mon - 10].weapon = monst.weapon = 0;
          combatupdate2(mon);
          showresults(mon, -52, mon);
          success = FALSE;
          return (0);
        }
      }
    }
  }

  if (chare < 9) {
    if ((c[chare].condition[COND_REFLECTING_ATTACKS]) && (Rand(100) < 34) && (!monster[mon - 10].dist))
      chare = mon;
  } else if (chare > 9) {
    if ((monster[chare - 10].condition[COND_REFLECTING_ATTACKS]) && (Rand(100) < 34) && (!monster[mon - 10].dist))
      chare = mon;
  }

  if (chare < 9) {
    if (character.condition[COND_HELPLESS]) {
      damage = character.stamina + Rand(10);
      goto doit;
    }
  } else if (monster[chare - 10].condition[COND_HELPLESS]) {
    damage = monster[chare - 10].stamina;
    goto doit;
  }

  if (success) {
    damage += monst.damplus;
    if (monst.condition[COND_STRONG])
      damage += 3; /***** strong ****/
    damage += monst.condition[COND_ATTACK_BONUS]; /***** attack bonus ****/
    if (!monst.weapon) {
      if (monst.attacks[attacknum][0])
        damage += randrange(monst.attacks[attacknum][0], monst.attacks[attacknum][1]);
      else
        damage += randrange(monst.attacks[0][0], monst.attacks[0][1]);
    } else {
      damage += item.damage;

      if (chare > 9) {
        monster[chare - 10].beenattacked = TRUE;

        damage += Rand(item.vssmall);

        if ((item.heat) && (!monster[chare - 10].spellimmune[1])) {
          specdamage = Rand(item.heat);
          if (savevs(1, chare))
            specdamage /= 2;
          if (monster[chare - 10].condition[COND_FIRE_PROTECTION])
            specdamage /= 2;
          specialdam += specdamage;
          if (specdamage)
            showresults(chare, -19, mon);
        }
        if ((item.cold) && (!monster[chare - 10].spellimmune[2])) {
          specdamage = Rand(item.cold);
          if (savevs(2, chare))
            specdamage /= 2;
          if (monster[chare - 10].condition[COND_COLD_PROTECTION])
            specdamage /= 2;
          specialdam += specdamage;
          if (specdamage)
            showresults(chare, -20, mon);
        }
        if ((item.electric) && (!monster[chare - 10].spellimmune[3])) {
          specdamage = Rand(item.electric);
          if (savevs(3, chare))
            specdamage /= 2;
          if (monster[chare - 10].condition[COND_ELECTRICAL_PROTECTION])
            specdamage /= 2;
          if (specdamage)
            specialdam += specdamage;
          if (specdamage)
            showresults(chare, -21, mon);
        }
        if ((item.vsundead) && (monster[chare - 10].type[1])) {
          specdamage = Rand(item.vsundead);
          specialdam += specdamage;
        }
        if ((item.vsdd) && (monster[chare - 10].type[2])) {
          specdamage = Rand(item.vsdd);
          specialdam += specdamage;
        }
        if ((item.vsevil) && (monster[chare - 10].type[4])) {
          specdamage = Rand(item.vsevil);
          specialdam += specdamage;
        }
      } else {
        damage += Rand(item.vssmall);

        if (item.heat) {
          specdamage = Rand(item.heat);
          if (savevs(1, chare))
            specdamage /= 2;
          if (c[chare].condition[COND_FIRE_PROTECTION])
            specdamage /= 2;
          specialdam += specdamage;
          if (specdamage)
            showresults(chare, -19, mon);
        }
        if (item.cold) {
          specdamage = Rand(item.cold);
          if (savevs(2, chare))
            specdamage /= 2;
          if (c[chare].condition[COND_COLD_PROTECTION])
            specdamage /= 2;
          specialdam += specdamage;
          if (specdamage)
            showresults(chare, -20, mon);
        }
        if (item.electric) {
          specdamage = Rand(item.electric);
          if (savevs(3, chare))
            specdamage /= 2;
          if (c[chare].condition[COND_ELECTRICAL_PROTECTION])
            specdamage /= 2;
          specialdam += specdamage;
          if (specdamage)
            showresults(chare, -21, mon);
        }
      }
    }

  doit:

    t = monst.attacks[attacknum][3];

    if (t) /***** special attack *********/
    {
      specdamage = randrange(monst.hd / 2, monst.hd);
      if (specdamage < 1)
        specdamage = 1;

      if (chare < 9) {
        switch (t) {
          /** *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
           * NOTE(danapplegate): Each special attack condition check below originally had the form:
           *
           *   if ((c[chare]condition[COND_RUNS_AWAY] += abs(specdamage)) < 30)
           *     c[chare]condition[COND_RUNS_AWAY] += abs(specdamage);
           *
           * This appears to have been intended to only apply damage if adding the damage to the condition
           * duration would not exceed 30 rounds. However, note that the compound assignment operator
           * is used in the if clause and the body, apparently inadvertently applying the condition
           * twice. This makes monster attacks that impose a condition twice as effective as the spell
           * data would imply.
           *
           * If we were to fix it by changing it to read:
           *
           *   it ((c[chare]condition[COND_RUNS_AWAY] + abs(specdamage)) < 30)
           *
           * that would fix the double application of damage, but would have the strange result that more
           * powerful monsters that deal large amounts of special damage would have a lower chance of
           * inflicting any damage at all. As an alternative approach, we modify the logic to only apply
           * the attack if the starting condition duration is less than 30.
           */
          case 1: /**** fear ****/
            if ((!savevs(5, chare)) && (character.condition[COND_RUNS_AWAY] > -1)) {
              if (c[chare].condition[COND_RUNS_AWAY] < 30)
                c[chare].condition[COND_RUNS_AWAY] += abs(specdamage);
              sound(630);
              showresults(chare, -33, mon); /****** special attack ******/
            }
            break;

          case 2: /**** paralyze ****/
            if ((!savevs(5, chare)) && (character.condition[COND_HELPLESS] > -1)) {
              if (c[chare].condition[COND_HELPLESS] < 30)
                c[chare].condition[COND_HELPLESS] += abs(specdamage);
              sound(630);
              showresults(chare, -32, mon); /****** special attack ******/
            }
            break;

          case 3: /**** curse ****/
            if ((!savevs(7, chare)) && (character.condition[COND_CURSED] > -1)) {
              if (c[chare].condition[COND_CURSED] < 30)
                c[chare].condition[COND_CURSED] += abs(specdamage);
              sound(630);
              showresults(chare, -31, mon); /****** special attack ******/
            }
            break;

          case 4: /**** stupid ****/
            if ((!savevs(5, chare)) && (character.condition[COND_STUPID] > -1)) {
              if (c[chare].condition[COND_STUPID] < 30)
                c[chare].condition[COND_STUPID] += abs(specdamage);
              sound(630);
              showresults(chare, -30, mon); /****** special attack ******/
            }
            break;

          case 5: /**** entangle ****/
            if ((!savevs(7, chare)) && (character.condition[COND_SLOW] > -1)) {
              if (c[chare].condition[COND_SLOW] < 30)
                c[chare].condition[COND_SLOW] += abs(specdamage);
              sound(630);
              showresults(chare, -29, mon); /****** special attack ******/
            }
            break;

          case 6: /**** poison ****/
            if ((!savevs(4, chare)) && (character.condition[COND_POISONED] > -1)) {
              if (c[chare].condition[COND_POISONED] < 30)
                c[chare].condition[COND_POISONED] += abs(specdamage);
              sound(630);
              showresults(chare, -28, mon); /****** special attack ******/
            }
            break;

          case 7: /**** confuse ****/
            if ((!savevs(5, chare)) && (character.condition[COND_CONFUSED] > -1)) {
              if (c[chare].condition[COND_CONFUSED] < 30)
                c[chare].condition[COND_CONFUSED] += abs(specdamage);
              sound(630);
              showresults(chare, -27, mon); /****** special attack ******/
            }
            break;

          case 16: /**** disease ****/
            if ((!savevs(4, chare)) && (character.condition[COND_DISEASED] > -1)) {
              if (c[chare].condition[COND_DISEASED] < 30)
                c[chare].condition[COND_DISEASED] += abs(specdamage);
              sound(630);
              showresults(chare, -34, mon); /****** special attack ******/
            }
            break;
            /* *** END CHANGES *** */

          case 8: /**** drain spell points ****/
            if ((!savevs(6, chare)) && (c[chare].spellpoints)) {
              temp = monst.hd * 3;
              if (temp > c[chare].spellpoints)
                temp = c[chare].spellpoints;
              c[chare].spellpoints -= temp;
              monster[mon - 10].spellpoints += temp;
              showresults(chare, -17, mon); /****** special attack ******/
            }
            break;

          case 9: /**** drain victory ****/
            if (!savevs(5, chare)) {
              c[chare].exp -= (monst.staminamax * 20);
              sound(630);
              showresults(chare, -18, mon); /****** special attack ******/
            }
            break;

          case 10: /**** charm ****/
            if (!savevs(0, chare)) {
              if (!c[chare].traiter) {
                killparty++;
                numenemy++;
                showresults(chare, -26, mon); /****** special attack ******/
              }
              c[chare].traiter = monst.traiter;
            }
            break;

          case 11: /**** fire damage ****/
            specdamage = Rand(monst.attacks[attackloop][1]);
            if (savevs(1, chare))
              specdamage /= 2;
            if (c[chare].condition[COND_FIRE_PROTECTION])
              specdamage /= 2;
            specialdam += specdamage;
            if (specdamage)
              showresults(chare, -19, mon); /****** special attack ******/
            break;

          case 12: /**** cold damage ****/
            specdamage = Rand(monst.attacks[attackloop][1]);
            if (savevs(2, chare))
              specdamage /= 2;
            if (c[chare].condition[COND_COLD_PROTECTION])
              specdamage /= 2;
            specialdam += specdamage;
            if (specdamage)
              showresults(chare, -20, mon); /****** special attack ******/
            break;

          case 13: /**** shock damage ****/
            specdamage = Rand(monst.attacks[attackloop][1]);
            if (savevs(3, chare))
              specdamage /= 2;
            if (c[chare].condition[COND_ELECTRICAL_PROTECTION])
              specdamage /= 2;
            specialdam += specdamage;
            if (specdamage)
              showresults(chare, -21, mon); /****** special attack ******/
            break;

          case 14: /**** chemical damage ****/
            specdamage = Rand(monst.attacks[attackloop][1]);
            if (savevs(4, chare))
              specdamage /= 2;
            if (c[chare].condition[COND_CHEMICAL_PROTECTION])
              specdamage /= 2;
            specialdam += specdamage;
            if (specdamage)
              showresults(chare, -22, mon); /****** special attack ******/
            break;

          case 15: /**** mental damage ****/
            specdamage = Rand(monst.attacks[attackloop][1]);
            if (savevs(5, chare))
              specdamage /= 2;
            if (c[chare].condition[COND_MENTAL_PROTECTION])
              specdamage /= 2;
            specialdam += specdamage;
            if (specdamage)
              showresults(chare, -23, mon); /****** special attack ******/
            break;

          case 17: /**** age ****/
            if (!savevs(7, chare)) {
              loadprofile(c[chare].race, 0);
              showresults(chare, -48, mon); /****** special attack ******/
              tempage = monst.attacks[attackloop][1] * monst.hd;
              tempfloat = races.maxage * .01;
              tempfloat *= tempage;
              tempage = tempfloat;
              c[chare].age += tempage;
              characterl = c[chare];
              didage = age(c[chare].age, c[chare].race, c[chare].currentagegroup);
              c[chare] = characterl;

              if (didage != 0) {
                in();
                showageupdate(chare, c[chare].currentagegroup, 0);
                updatemain(1, -1);
                centerfield(5 + (2 * screensize), 5 + screensize);
                combatupdate2(q[up]);
                updatecontrols();
              }
            }
            break;

          case 18: /**** cause blindness ****/
            if (!savevs(7, chare)) {
              showresults(chare, -47, mon); /****** special attack ******/
              c[chare].condition[COND_BLIND] = -1;
            }
            break;

          case 19: /**** turn to stone ****/
            if (!savevs(7, chare)) {
              c[chare].stamina = 0;
              c[chare].condition[COND_TURNED_TO_STONE] = -1;
              showresults(chare, -49, mon); /****** special attack ******/
              goto forcekill;
            }
            break;
        }
      } else if (monster[chare - 10].magres > 100)
        goto whiff;
      else /****** affect monster ******/
      {
        switch (t) {
          case 1: /**** fear ****/
            if ((!savevs(5, chare)) && (monster[chare - 10].condition[COND_RUNS_AWAY] > -1)) {
              monster[chare - 10].condition[COND_RUNS_AWAY] += abs(specdamage);
              sound(630);
              showresults(chare, -33, mon); /****** special attack ******/
            }
            break;

          case 2: /**** paralyse ****/
            if ((!savevs(5, chare)) && (monster[chare - 10].condition[COND_HELPLESS] > -1)) {
              monster[chare - 10].condition[COND_HELPLESS] += abs(specdamage);
              sound(630);
              showresults(chare, -32, mon); /****** special attack ******/
            }
            break;

          case 3: /**** curse ****/
            if ((!savevs(7, chare)) && (monster[chare - 10].condition[COND_CURSED] > -1)) {
              monster[chare - 10].condition[COND_CURSED] += abs(specdamage);
              sound(630);
              showresults(chare, -31, mon); /****** special attack ******/
            }
            break;

          case 4: /**** stupid ****/
            if ((!savevs(5, chare)) && (monster[chare - 10].condition[COND_STUPID] > -1)) {
              monster[chare - 10].condition[COND_STUPID] += abs(specdamage);
              sound(630);
              showresults(chare, -30, mon); /****** special attack ******/
            }
            break;

          case 5: /**** entangle ****/
            if ((!savevs(7, chare)) && (monster[chare - 10].condition[COND_SLOW] > -1)) {
              monster[chare - 10].condition[COND_SLOW] += abs(specdamage);
              sound(630);
              showresults(chare, -29, mon); /****** special attack ******/
            }
            break;

          case 6: /**** poison ****/
            if ((!savevs(4, chare)) && (monster[chare - 10].condition[COND_POISONED] > -1)) {
              monster[chare - 10].condition[COND_POISONED] += abs(specdamage);
              sound(630);
              showresults(chare, -28, mon); /****** special attack ******/
            }
            break;

          case 7: /**** confuse ****/
            if ((!savevs(5, chare)) && (monster[chare - 10].condition[COND_CONFUSED] > -1)) {
              monster[chare - 10].condition[COND_CONFUSED] += abs(specdamage);
              sound(630);
              showresults(chare, -27, mon); /****** special attack ******/
            }
            break;

          case 16: /**** disease ****/
            if ((!savevs(4, chare)) && (monster[chare - 10].condition[COND_DISEASED] > -1)) {
              monster[chare - 10].condition[COND_DISEASED] += abs(specdamage);
              sound(684);
              showresults(chare, -34, mon); /****** special attack ******/
            }
            break;

          case 8: /**** drain spell points ****/
            if ((!savevs(6, chare)) && (monster[chare - 10].spellpoints)) {
              temp = monst.hd * 3;
              if (temp > monster[chare - 10].spellpoints)
                temp = monster[chare - 10].spellpoints;
              monster[chare - 10].spellpoints -= temp;
              monster[mon - 10].spellpoints += temp;
              showresults(chare, -17, mon); /****** special attack ******/
            }
            break;

          case 10: /**** charm ****/
            if (!savevs(0, chare)) {
              if ((!monst.traiter) && (monster[chare - 10].traiter))
                numenemy--;
              if ((monst.traiter) && (!monster[chare - 10].traiter))
                numenemy++;
              monster[chare - 10].traiter = monst.traiter;
              monster[mon - 10].target = -1;
              showresults(chare, -26, mon); /****** special attack ******/

              if (killmon >= numenemy)
                killbody(chare, 0); /*** charmed last guy, bail out!! ***/
            }
            break;

          case 11: /**** fire damage ****/
            specdamage = Rand(monst.attacks[attackloop][1]);
            if (savevs(1, chare))
              specdamage /= 2;
            if (monster[chare - 10].condition[COND_FIRE_PROTECTION])
              specdamage /= 2;
            specialdam += specdamage;
            if (specdamage)
              showresults(chare, -19, mon); /****** special attack ******/
            break;

          case 12: /**** cold damage ****/
            specdamage = Rand(monst.attacks[attackloop][1]);
            if (savevs(2, chare))
              specdamage /= 2;
            specialdam += specdamage;
            if (monster[chare - 10].condition[COND_COLD_PROTECTION])
              specdamage /= 2;
            if (specdamage)
              showresults(chare, -20, mon); /****** special attack ******/
            break;

          case 13: /**** shock damage ****/
            specdamage = Rand(monst.attacks[attackloop][1]);
            if (savevs(3, chare))
              specdamage /= 2;
            specialdam += specdamage;
            if (monster[chare - 10].condition[COND_ELECTRICAL_PROTECTION])
              specdamage /= 2;
            if (specdamage)
              showresults(chare, -21, mon); /****** special attack ******/
            break;

          case 14: /**** chemical damage ****/
            specdamage = Rand(monst.attacks[attackloop][1]);
            if (savevs(4, chare))
              specdamage /= 2;
            specialdam += specdamage;
            if (monster[chare - 10].condition[COND_CHEMICAL_PROTECTION])
              specdamage /= 2;
            if (specdamage)
              showresults(chare, -22, mon); /****** special attack ******/
            break;

          case 15: /**** mental damage ****/
            specdamage = Rand(monst.attacks[attackloop][1]);
            if (savevs(5, chare))
              specdamage /= 2;
            specialdam += specdamage;
            if (monster[chare - 10].condition[COND_MENTAL_PROTECTION])
              specdamage /= 2;
            if (specdamage)
              showresults(chare, -23, mon); /****** special attack ******/
            break;

          case 18: /**** Blinded ****/
            if (!savevs(7, chare)) {
              monster[chare - 10].condition[COND_BLIND] = -1;
              showresults(chare, -47, mon); /****** special attack ******/
            }
            break;

          case 19: /**** Stoned ****/
            if (!savevs(7, chare)) {
              monster[chare - 10].condition[COND_TURNED_TO_STONE] = TRUE;
              monster[chare - 10].stamina = 0;
              showresults(chare, -49, mon); /****** special attack ******/
              goto forcekill;
            }
            break;
        }
      }
    }

    if ((partycondition[PARTY_COND_DRAGON_HIDE]) && (damage > 1) && (chare < 9)) /**** barkskin & other *****/
    {
      damage -= 5;
      if (damage < 1)
        damage = 1;
      sound(-694);
    }

    if (chare < 9) {
      if (addcond) /**** weapon hit adds condition ******/
      {
        if (c[chare].condition[whichcond - 20] > -1)
          c[chare].condition[whichcond - 20] += amountcond;
        showresults(chare, -40, mon);
      }

      c[chare].damagetaken += (specialdam + damage);
      c[chare].hitstaken++;
      c[chare].stamina -= specialdam;
      c[chare].stamina -= damage;
      if ((c[chare].stamina < 1) && (c[chare].stamina > -10))
        c[chare].bleeding = TRUE;
      c[chare].beenattacked = TRUE;
    } else {

      if (addcond) /**** weapon hit adds condition ******/
      {
        if (monster[chare - 10].condition[whichcond - 20] > -1)
          monster[chare - 10].condition[whichcond - 20] += amountcond;
        showresults(chare, -40, mon);
      }

      monster[chare - 10].beenattacked = TRUE;
      monster[chare - 10].stamina -= damage;
      monster[chare - 10].stamina -= specialdam;
    }

    showresults(chare, 0, mon); /*** damage ***/
  } else {
  whiff:
    if (chare < 6) {
      c[chare].umissed++;
    }
    success = damage = specdamage = FALSE;
    showresults(chare, -1, mon); /*** miss ***/
  }

forcekill:

  if (chare < 9) {
    if (c[chare].stamina <= 0) {
      kill = TRUE;
      killbody(chare, 0);
    }
  } else if (monster[chare - 10].stamina <= 0) {
    kill = TRUE;
    killbody(chare, 0);
  }

  return (kill);
}
