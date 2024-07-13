#include "prototypes.h"
#include "variables.h"

/********************** resolvespell *************************/
void resolvespell(void) {
  struct monster monst;
  short special;
  short spellclass, damagetype, t, misslecount = 1;
  float tempfloat, tempfloat2;
  Boolean doesresist = 0;
  char* attr;

  adjdam = damage = duration = 0;

  special = abs(spellinfo.special);
  damagetype = abs(spellinfo.damagetype);

  spellclass = spellinfo.spellclass;
  duration = randrange(spellinfo.duration1, spellinfo.duration2);

  if (incombat) {
    if ((q[up] < 6) && (memoryspell == TRUE) && (charup < 6) && (q[up] > -1) && (!monsterturn)) {
      definespells[charup][9][0] = castcaste;
      definespells[charup][9][1] = castlevel;
      definespells[charup][9][2] = castnum;
      definespells[charup][9][3] = powerlevel;
    } else if ((!incombat) && (memoryspell == TRUE)) {
      definespells[charselectnew][9][0] = castcaste;
      definespells[charselectnew][9][1] = castlevel;
      definespells[charselectnew][9][2] = castnum;
      definespells[charselectnew][9][3] = powerlevel;
      memoryspell = FALSE;
    }
  }

  if (spellinfo.powerdur1)
    for (t = 0; t < powerlevel; t++)
      duration += randrange(spellinfo.powerdur1, spellinfo.powerdur2);

  damage = randrange(spellinfo.damage1, spellinfo.damage2);

  if (damagetype == 9) /********** look to add magic plus for magical missile weapon itself ********/
  {
    damage += item.damage;
    spellinfo.tohitbonus += (5 * item.damage);
  }

  if ((q[up] < 6) && (q[up] > -1) && (!monsterturn)) {
    if (spellinfo.spellclass == 9) /**** Missile Bonus Damage ****/
    {
      loadprofile(0, c[charup].caste);
      if (caste.getsmissilebonus)
        damage += randrange(1, (c[charup].level / 2));
    }
  }

  if (spellinfo.powerdam1) {
    for (t = 0; t < powerlevel; t++) {
      damage += randrange(spellinfo.powerdam1, spellinfo.powerdam2);
    }
  }

  adjdam = damage;

  if (special == 68) /***** reduce fatigue *****/
  {
    fat = 1;
    updatefat(TRUE, 0, TRUE);
    return;
  }

  if (special == 48) /**** identify ****/
  {
    sound(647);
    for (t = 0; t <= charnum; t++)
      if (track[t]) {
        for (tt = 0; tt <= c[t].numitems; tt++)
          c[t].items[tt].ident = TRUE;
      }
  }

  if (spellinfo.targettype == 7) {
    if (special == 50) /**** light *****/
    {
      sound(601);
      if (powerlevel * 30 > partycondition[0]) {
        CGrafPtr thePort = GetQDGlobalsThePort();
        partycondition[0] = (30 * powerlevel) - 1;
        if ((thePort == GetWindowPort(screen)) ||
            (thePort == GetWindowPort(look))) {
          RGBBackColor(&greycolor);
          GetControlBounds(torch, &buttonrect);
          // buttonrect = *&(**(torch)).contrlRect;
          downbutton(TRUE);
          InsetRect(&buttonrect, 1, 1);
          EraseRect(&buttonrect);
          InsetRect(&buttonrect, -1, -1);
          updatetorch();
        }
        loaddark(powerlevel - 1);
        if (FrontWindow() == look)
          centerpict();
      }
      inspell = 0;
      {
        CGrafPtr thePort = GetQDGlobalsThePort();
        if ((thePort == GetWindowPort(screen)) && (checkforitem(805, FALSE, -1)))
          upbutton(TRUE);
      }
      return;
    } else {
      if (duration > partycondition[special])
        partycondition[special] = duration;
      inspell = 0;
      if (special == 4) {
        if (indung) {
          warn(96); /**** wizard eye ****/
          UpdateWindow(FALSE);
        }
      }
      return;
    }
  }

  if (special == 66) /****** increase attirbute *****/
  {
    if (!spellinfo.size)
      spellinfo.size = Rand(5);
    for (t = 0; t <= charnum; t++)
      if (track[t]) {
        loadprofile(0, c[t].caste);
        attr = &c[t].st;
        attr += (spellinfo.size - 1);
        if (*attr < 25) {
          *attr += 1;

          switch (spellinfo.size) {
            case 1: /***** Brawn ****/
              c[t].st--;

              strength(c[t].st);
              c[t].damage -= damage;
              c[t].tohit -= temp;

              c[t].st++;
              strength(c[t].st);
              c[t].damage += damage;
              c[t].tohit += temp;

              break;

            case 2: /***** Knowledge ****/
              if (c[t].in > 15)
                c[t].magres += caste.magres;
              break;

            case 3: /***** Judgment ****/
              if (c[t].wi > 15)
                c[t].magres += caste.magres;
              break;

            case 4: /***** Agility ****/
              if (c[t].de > 14)
                c[t].ac += 2;
              break;

            case 5: /***** Vitality ****/
              if (c[t].co > 18)
                for (tt = 0; tt < 8; tt++)
                  c[t].save[tt] += 5;
              break;

            case 10: /***** Stamina ****/
              c[t].staminamax += Rand(8);
              break;

            case 11: /***** Spell Points ****/
              if (c[t].spellpointsmax > 0)
                c[t].spellpointsmax += Rand(20);
              break;
          }
        } else
          warn(79);
      }
  }

  if (special == 57) /****** heal ******/
  {
    adjdam = damage = -damage;
    special = 0;
  }

  if (special == 56) /******* phase *******/
  {
    if (canundo) {
      showresults(charup, special, q[up]);
      sound(699);
      spelleffect(charup, 0);
      bodyground(charup, 0);
      bodyfield(charup);
      pos[charup][0] = target[0][0] - fieldx;
      pos[charup][1] = target[0][1] - fieldy;
      centerfield(pos[charup][0], pos[charup][1]);
      temp = field[pos[charup][0] + fieldx][pos[charup][1] + fieldy];
      charunder[charup] = temp;

      if (temp > 999) {
        if (mapstats[temp - 1000].solid)
          goto phaseintosolid;
      }

      if (temp < 999) /******* blinked into a solid object/creature *****/
      {
      phaseintosolid:
        field[pos[charup][0] + fieldx][pos[charup][1] + fieldy] = charup;
        sound(658);
        spelleffect(charup, 0);
        sound(631);
        warn(123);
        c[charup].stamina = -10;
        killbody(charup, 0);
        return;
      }

      field[pos[charup][0] + fieldx][pos[charup][1] + fieldy] = charup;
      sound(658);
      spelleffect(charup, 0);

      if (!spellinfo.size)
        c[charup].attacks = 0;
      c[charup].beenattacked = TRUE;
      return;
    } else {
      warn(87);
      return;
    }
  }

  if (special == 58) /********* summon **************/
  {
    if (nummon > (maxmon - 1)) {
      cleartarget();
      targetnum = 0;
      loop = 6;
      flashmessage((StringPtr) "Sorry, this battle has reached it's limit of monsters.", 50, 100, 0, 6000);
      inspell = 0;
      combatupdate2(charup);
      return;
    } else {
      spelllist(0, special);
      return;
    }
  }

  if ((spellinfo.targettype == 9) && (incombat))
    flashmessage((StringPtr) "This spell affects all those friendly to the caster.", 305, 322, 60, 6001);
  else if ((spellinfo.targettype == 10) && (incombat))
    flashmessage((StringPtr) "This spell affects all enemies of the caster.", 305, 322, 60, 6001);

  usequickshow = FALSE;

  if ((quickshow) && ((spellinfo.targettype == 3) || (spellinfo.targettype == 4) || (spellinfo.targettype > 8))) {
    if ((!collideflag) && (incombat)) {
      usequickshow = TRUE;
      sound(600 + spellinfo.sound2);
      if (incombat)
        spelleffect(temp, TRUE);
    }
  }

  for (t = 0; t < maxloop; t++) {
    if (track[t] > 0) {
    doagain:
      damage = adjdam;
      if ((t < 9) && (incombat)) {
        if ((t != q[up]) && (c[t].spellpointsmax) && (c[t].condition[35]) && (q[up] > 9)) {
          c[t].spellpoints += curControlValue;
          if (c[t].spellpoints > c[t].spellpointsmax)
            c[t].spellpoints = c[t].spellpointsmax;
        }
      } else if (incombat) {
        monst = monster[t - 10];
        if ((t != q[up]) && (monster[t - 10].spellpoints) && (monster[t - 10].condition[35]) && (q[up] < 9))
          monster[t - 10].spellpoints += curControlValue;
      }

      doesresist = resist(t);
      if ((collideflag) && (collidecheck[collideflag]))
        goto out;

      if ((!doesresist) || (!incombat)) {
        if (t < 9) /**** check resistances ****/
        {
          if ((damagetype < 8) && (damagetype)) {
            if ((savevs(damagetype, t)) && (spellinfo.cannot < 2)) {
              if (damage)
                damage /= 2;
              else {
                showresults(t, -24, q[up]); /**** saved Vs. *****/
                goto out;
              }
            }
            if ((damage) && (c[t].condition[10 + damagetype]))
              damage /= 2;
            collidecheck[collideflag] = TRUE;
          }
        } else if ((damagetype < 8) && (damagetype)) {
          if (savevs(damagetype, t)) {
            if (damage)
              damage /= 2;
            else {
              showresults(t, -24, q[up]); /**** saved Vs. *****/
              goto out;
            }
          }

          if ((damage) && (monst.condition[10 + damagetype]))
            damage /= 2;

          collidecheck[collideflag] = TRUE;

          if ((spellclass < 6) && (spellclass > -1)) {
            if (monst.spellimmune[spellclass]) {
            immune:
              damage = 0;
              showresults(t, -4, q[up]);
              continue;
            }
          }

          if (damagetype > 0) {
            if (monst.save[damagetype - 1] < 0) /*** vulnerability ***/
            {
              temp = abs(monst.save[damagetype - 1]);
              tempfloat = temp;
              tempfloat /= 100;
              tempfloat++;

              tempfloat2 = ((long)damage * tempfloat);

              damage = ((short)tempfloat2);
            }
          }
        }

        if ((special < 41) && (special)) {
          if (special == 28)
            damage = adjdam = duration; /*** disease ***/

          if (t > 9) {
            if (abs(monster[t - 10].condition[special - 1] + duration) < 125) {
              if (monster[t - 10].condition[special - 1] > -1)
                monster[t - 10].condition[special - 1] += duration;
            }
          } else if ((c[t].condition[special - 1] + duration) < 100) {
            if (c[t].condition[special - 1] > -1)
              c[t].condition[special - 1] += duration;
          }
        } else if (special > 99) {
          if (t > 9)
            monster[t - 10].condition[special - 101] = 0;
          else {
            c[t].condition[special - 101] = 0;
            updatechar(t, 3);
          }
        }

        if (spelllist(t, special)) {
          if ((!adjdam) && (special)) {
            sound(600 + spellinfo.sound2);
            showresults(t, abs(special), q[up]);
            if (!usequickshow)
              spelleffect(t, 0);
          }
        }

        if (special == 49)
          adjdam = damage;

        if ((adjdam) && (!damage) && (special != 60) && (special != 59))
          damage = 1;

        if (t < 9) {
          if ((adjdam) && (c[t].stamina > -10)) {
            sound(600 + spellinfo.sound2);
            if (damage < 0)
              heal(t, abs(damage), FALSE);
            else {
              if ((c[t].stamina < 0) && (damage > 0))
                continue; /***** dont do damage to unconsious characters *****/
              c[t].stamina -= damage;
              select[t] = -1;
              if (damage > 0)
                c[t].beenattacked = TRUE;
              if ((c[t].stamina <= 0) && (c[t].stamina + damage > 0)) /**** make sure he just went under ***/
              {
                killbody(t, 0);
              } else {
                showresults(t, abs(special), q[up]);
                if (incombat) {
                  if (!usequickshow)
                    spelleffect(t, 0);
                }
                if (!incombat) {
                  spelleffect(t, 0);
                  delay(0);
                  SetPort(GetWindowPort(screen));
                  EraseRect(&textrect);
                }
              }
            }
          }
        } else if ((adjdam) && (t > 9)) {
          sound(600 + spellinfo.sound2);
          monster[t - 10].beenattacked = TRUE;
          monster[t - 10].stamina -= damage;
          if ((q[up] < 6) && (q[up] > -1))
            c[charup].damagegiven += damage;
          if ((q[up] < 6) && (q[up] > -1) && (spellclass == 9))
            c[charup].hitsgiven++;

          if (monster[t - 10].stamina <= 0) {
            if ((q[up] < 6) && (q[up] > -1))
              c[charup].kills++;
            killbody(t, 0);
          } else {
            if (damage)
              showresults(t, 0, q[up]);
            if (!usequickshow)
              spelleffect(t, 0);
            if ((spellclass == 9) && (misslecount++ < spellinfo.fixedtargetnum)) {
              if (killmon == numenemy)
                return;
              goto doagain;
            }
          }
        }
      } else {
        if (spellinfo.spellclass == 9) {
          if ((q[up] < 6) && (q[up] > -1))
            c[charup].imissed++;
          sound(652); /*** Pro Jo Miss sound ***/
          showresults(t, -10, q[up]);
        } else if (doesresist == 1) {
          sound(133); /*** resist sound ***/
          showresults(t, -3, q[up]); /********* magic resistance *******/
        }
      }
    out:
      if (incombat) {
        if ((inspell) && (spellinfo.targettype > 8))
          delay(5);
        else
          delay(0);
      }
    }
  }
}
