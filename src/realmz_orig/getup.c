#include "prototypes.h"
#include "variables.h"

/**************** getup *******************/
void getup(short skip) {
  float top, bottom, percent = 0;
  short t, tt, ttt;
  short showbleed;

  cleartarget();
  infocombat = usescroll = memoryspell = itemused = FALSE;
  canundo = TRUE;

  cleanup = cycle = physical = FALSE;

  if (killparty > charnum)
    partyloss(0);

  reply = FALSE;
  for (t = 0; t <= charnum; t++)
    if ((c[t].inbattle) && (!c[t].traiter))
      reply = TRUE;
  if (!reply) {
    killmon = numenemy;
    coward = TRUE;
    booty(2); /********** skip treasure scene **********/
    return;
  }

startover:

  if (!skip) {
    if (q[up] > -1) {
      if (q[up] > 9) {
        if (monster[q[up]].guarding)
          showresults(q[up], -5, 0); /*** guarding ***/
      } else if (c[q[up]].guarding)
        showresults(q[up], -5, 0); /*** guarding ***/
    }
  }

  nospell = didattack = poss = key = inspell = 0;

  if ((q[up] < 9) && (q[up] > -1)) {
    updatelight(q[up], 1);
    if (c[q[up]].condition[25] > 0)
      c[q[up]].condition[25] = 0;
  }

  if (numchannel < 2)
    for (t = 0; t <= numchannel; t++)
      quiet(t);

getnew:
  up++;

  for (t = 0; t < 60; t++) {
    if (que[t].duration) {
      if (up == que[t].phase)
        que[t].duration--;
    }
  }

  if (killmon >= numenemy)
    return;
  for (t = 0; t < 60; t++)
    collidecheck[t] = 0;

  emptyque(); /***** empty macro que ****/

  aimindex = up;
  if (up > maxloopminus) {
    combatround++;

    if (battle.battlemacro < 0) /***** activate battle macro ****/
    {
      battlemacro = TRUE;
      loaddoor2(abs(battle.battlemacro));
      newland(0L, 0L, -1, 0, 0);
      battlemacro = FALSE;
      if (killmon == numenemy)
        return; /*** encountered code 100, bail right away **/
    }

    if (blankround++ > 8) {
      killmon = numenemy;
      coward = TRUE;
      goto getnew;
    }

    up = aimindex = 0;

    reduce();

    for (t = 0; t <= charnum; t++) {
      updatelight(t, 0);
      for (tt = 0; tt < 60; tt++)
        collidecheck[tt] = 0;
      if (c[t].stamina > 0)
        collide(t, FALSE);
      c[t].beenattacked = FALSE;
      if (c[t].bleeding) {
        if (c[t].stamina > -10) {
          c[t].stamina--;
          if (c[t].stamina < -9) {
            c[t].bleeding = 0;
            c[t].knockouts--;
            c[t].deaths++;
            updatepictbox(t, 1, 0);
            sound(132);
            flashmessage((StringPtr) "Your friend has bled to death!", 30, 100, 180, 10122);
          } else {
            if (Rand(100) < 50)
              sound(10121);
            else
              sound(10123);
          }
        }
        updatechar(t, 3);
      }
    }

    for (t = 0; t < maxmon; t++) {
      for (tt = 0; tt < 40; tt++)
        if (monster[t].condition[tt] > 0)
          monster[t].condition[tt]--;
      monster[t].beenattacked = FALSE;

      monster[t].attacknum = 0;

      if ((monster[t].condition[10]) && (monster[t].stamina > 0) && (monster[t].stamina < monster[t].staminamax)) /***** Regenerate ****/
      {
        damage = abs(monster[t].condition[10]);
        monster[t].stamina += damage;
        if (monster[t].stamina > monster[t].staminamax) {
          damage -= (monster[t].stamina - monster[t].stamina);
          monster[t].stamina = monster[t].staminamax;
        }
        regenerate = TRUE;
        if (damage)
          showresults(t + 10, 0, 0); /********* regenerate for ***************/
        regenerate = FALSE;
      }

      if ((monster[t].condition[28]) && (!monster[t].condition[25]) && (monster[t].stamina > 0)) /***** disease ****/
      {
        damage = abs(monster[t].condition[28]);
        monster[t].stamina -= damage;
        showresults(t + 10, 29, 0); /********* diseased for ***************/
        if (monster[t].stamina < 1)
          killbody(t + 10, 0);
      }

      if ((monster[t].condition[9]) && (!monster[t].condition[25]) && (monster[t].stamina > 0)) /***** poison ****/
      {
        damage = abs(monster[t].condition[9]);
        monster[t].stamina -= damage;
        poisoned = TRUE;
        showresults(t + 10, 0, 0); /********* poisoned for ***************/
        poisoned = FALSE;
        if (monster[t].stamina < 1)
          killbody(t + 10, 0);
      }

      if (monster[t].condition[34]) /****** power suck *******/
      {
        monster[t].spellpoints -= monster[t].condition[34];
        if (monster[t].spellpoints < 0)
          monster[t].spellpoints = 0;
      }

      if ((monster[t].condition[33]) && (monster[t].type[0])) /******* power absorbe ********/
      {
        monster[t].spellpoints += monster[t].condition[34];
      }
      for (tt = 0; tt < 60; tt++)
        collidecheck[tt] = 0;
      if (monster[t].stamina > 0)
        collide(t + 10, FALSE);
      monster[t].beenattacked = FALSE;
    }

    if (!showbleedmessage) {
      showbleed = FALSE;

      for (t = 0; t <= charnum; t++)
        if (c[t].bleeding)
          showbleed = TRUE;

      if (showbleed) {
        if (Rand(100) < 50)
          sound(10121);
        else
          sound(10123);
        flashmessage((StringPtr) "Your friend is bleeding and will die if you do not bandage the wounds!", 30, 100, 120, 10122);
      }
    }

    if (shownextroundmessage)
      flashmessage((StringPtr) "New Combat Round.", 30, 100, 120, 139);
  }

  if (q[up] < 0)
    goto getnew;

  if (q[up] < 9) {
    if (!c[q[up]].inbattle)
      goto getnew; /*** not in battle ******/
    if (c[q[up]].condition[1])
      goto getnew; /*** helpless ******/
    if (c[q[up]].traiter)
      c[q[up]].condition[25] = poss = TRUE; /*** set character to auto because he is charmed ***/
  } else if (q[up] > -1) {
    if (monster[q[up] - 10].stamina < 1)
      goto getnew; /*** not in battle ******/
    if (monster[q[up] - 10].condition[1])
      goto getnew; /**** helpless ***/
  }

  if ((q[up] < 9) && (q[up] > -1)) {
    monsterturn = 0;
    charup = q[up];

    undox = pos[charup][0] + fieldx; /**** getundo coordinates ****/
    undoy = pos[charup][1] + fieldy;

    if (c[charup].condition[0]) /**** running ****/
    {
      if (!c[charup].condition[25])
        c[charup].condition[25] = TRUE;
    }

    if (c[charup].condition[25] > -1) {
      if (c[charup].condition[29]) /*********** confused **************/
      {
        temp = Rand(100);
        if (temp < 40) {
          if ((Rand(2) == 1) && (!c[charup].traiter)) {
            c[charup].traiter++;
            if (!c[charup].condition[25])
              c[charup].condition[25] = TRUE;
            numenemy++;
            killparty++;
          } else if ((Rand(2) == 2) && (c[charup].traiter)) {
            c[charup].traiter = 0;
            numenemy--;
            killparty--;
          }
        } else if (temp > 60)
          c[charup].condition[0]++;
        else {
          c[charup].condition[1]++;
          goto getnew;
        }
      }
    }
    movecalc(charup);
    c[charup].movement = c[charup].movementmax;
    c[charup].spellsofar = 0;
    combatupdate2(charup);
    updatecontrols();

    if (c[charup].attacks > 0)
      c[charup].attacks = 1;
    if (c[charup].attacks < 0)
      c[charup].attacks = 0;
    c[charup].attacks += c[charup].normattacks + c[charup].attackbonus;

    if (c[charup].condition[23])
      c[charup].attacks += 4; /****** haste ****/

    if (!inwindow(charup))
      centerfield(pos[charup][0], pos[charup][1]);
    else
      centerfield(5 + (2 * screensize), 5 + screensize);

    if (!c[charup].condition[25])
      sound(138);
  } else {
    monsterturn = 1;
    monsterup = q[up] - 10;
    monster[monsterup].attacknum = 0;

    if (monster[monsterup].condition[29]) /******* confusion *****/
    {
      temp = Rand(100);
      if (temp < 40) {
        if ((Rand(2) == 1) && (!monster[monsterup].traiter)) {
          monster[monsterup].traiter++;
          numenemy++;
        } else if ((Rand(2) == 1) && (monster[monsterup].traiter)) {
          monster[monsterup].traiter = 0;
          numenemy--;
        }
      } else if (temp > 60)
        monster[monsterup].condition[0]++;
      else {
        monster[monsterup].condition[1]++;
        goto getnew;
      }
    }

    monster[monsterup].movement = monster[monsterup].movementmax;
    monster[monsterup].movement -= monster[monsterup].condition[2]; /** tangled **/
    if (monster[monsterup].condition[6])
      monster[monsterup].movement /= 2; /** slow **/

    if (monster[monsterup].condition[23]) /***** Haste***/
    {

      monster[monsterup].movement *= 2;
      monster[monsterup].bonusattack = 2;

      for (ttt = 1; ttt < (monster[monsterup].noofattacks + monster[monsterup].bonusattack); ttt++) {
        if (!monster[monsterup].attacks[ttt][0]) {
          monster[monsterup].attacks[ttt][0] = monster[monsterup].attacks[0][0];
          monster[monsterup].attacks[ttt][1] = monster[monsterup].attacks[0][1];
          monster[monsterup].attacks[ttt][2] = monster[monsterup].attacks[0][2];
          monster[monsterup].attacks[ttt][3] = monster[monsterup].attacks[0][3];
        }
      }
    } else
      monster[monsterup].bonusattack = 0;

    combatupdate2(q[up]);

    if (!inwindow(q[up]))
      centerfield(monpos[monsterup][0], monpos[monsterup][1]);
    else
      centerfield(5 + (2 * screensize), 5 + screensize);

    top = monster[monsterup].stamina;
    bottom = monster[monsterup].stamina;
    percent = top / bottom * 100;

    if (percent < monster[monsterup].surrenderpercent) {
      if (monster[monsterup].surrenderpercent == 101)
        showresults(monsterup + 10, -8, 0); /********** flees in panic *******/
      else
        showresults(monsterup + 10, -7, 0); /************* surrenders **************/
      monster[monsterup].stamina = 0;
      bodyground(q[up], 0);
      bodyfield(q[up]);
      if (monster[monsterup].traiter)
        killmon++;
      goto startover;
    }

    if (percent < monster[monsterup].runpercent) {
      monster[monsterup].condition[0] = -1;
      showresults(monsterup + 10, -6, 0); /************* run away **************/
    }
  }
  canundo = TRUE;
  FlushEvents(everyEvent, 0);
}
