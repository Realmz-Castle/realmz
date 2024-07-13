#include "variables.h"

/**************** killbody *******************/
void killbody(short mon, short performque) {
  struct door holddoor;
  Rect iconrect;
  short temp, t, tempstamina, escape = 0;

  if (incombat) {
    if (performque)
      goto doquenow;

    if (mon > 9) {
      tempstamina = monster[mon - 10].stamina;
      monster[mon - 10].stamina = 1;
      center(mon);
      monster[mon - 10].stamina = tempstamina;
      size = monster[mon - 10].size;
      iconrect.top = monpos[mon - 10][1] * 32;
      iconrect.left = monpos[mon - 10][0] * 32;
      if (monster[mon - 10].traiter)
        killmon++;
      monster[mon - 10].movement = 0;
      for (t = 0; t < 40; t++)
        monster[mon - 10].condition[t] = 0;
    } else if ((mon > -1) && (mon <= charnum)) {
      temp = c[mon].stamina;
      c[mon].stamina = 1;
      iconrect.top = pos[mon][1] * 32;
      iconrect.left = pos[mon][0] * 32;
      c[mon].stamina = temp;
      size = c[mon].inbattle = doauto[mon] = 0;
      if (temp > -10) {
        c[mon].bleeding = TRUE;
        c[mon].knockouts++;
      } else
        c[mon].deaths++;
      c[mon].position = -1; /**** turn light red ****/
      updatelight(mon, FALSE);

      if (c[mon].traiter)
        killmon++;
      else if ((!c[mon].traiter) && (c[mon].condition[25] < 1))
        killparty++;

      c[mon].condition[25] = c[mon].attacks = 0;
      for (t = 0; t < 40; t++)
        if (c[mon].condition[t] > 0)
          c[mon].condition[t] = 0;
    } else
      return;

    iconrect.bottom = iconrect.top + 32;
    iconrect.right = iconrect.left + 32;

    if ((size == 1) || (size == 3))
      iconrect.top -= 32;
    if (size > 1)
      iconrect.left -= 32;

    SetPort(GetWindowPort(look));

    iconhand = NIL;

    iconhand = GetCIcon(2015 + size);
    if (iconhand) {
      PlotCIcon(&iconrect, iconhand);
      DisposeCIcon(iconhand);
    }

    for (t = 1; t < maxloop; t++)
      if (q[t] == mon)
        q[t] = -1;
    if (mon < 9) {
      if (c[mon].stamina > -10)
        showresults(mon, -11, 0);
      else
        showresults(mon, -2, 0); /********** -2 = Killed **************/
    } else {
      if (monster[mon - 10].type[1])
        showresults(mon, -14, 0); /****** destroyed *****/
      else if (monster[mon - 10].stamina > -10)
        showresults(mon, -11, 0);
      else
        showresults(mon, -2, 0); /********** -2 = Killed **************/

      if (monster[mon - 10].todoondeath) /***** Monster Macro *****/
      {
        if ((macro) || (inspell)) {
          todoque[todoqueindex].monsterpostion = mon - 10;
          todoque[todoqueindex].frommonid = monster[mon - 10].name;
          todoque[todoqueindex].traiter = monster[mon - 10].traiter;
          todoque[todoqueindex].doorid = monster[mon - 10].todoondeath;
          todoque[todoqueindex].x = monpos[mon - 10][0] + fieldx;
          todoque[todoqueindex].y = monpos[mon - 10][1] + fieldy;
          todoqueindex++;
          goto skipmacro;
        }

      doquenow:

        macro = TRUE;

        for (t = 0; t < 8; t++) /**** save old door codes ****/
        {
          holddoor.code[t] = infodoor.code[t];
          holddoor.id[t] = infodoor.id[t];
        }

        for (t = 0; t < 5; t++)
          holdcode[t] = extracode[t]; // Fantasoft v7.1 **** save old E-Codes ****/

        /*** perform macro codes here ****/

        if (!performque) {
          loaddoor2(monster[mon - 10].todoondeath); /**** load codes to be executed ****/
          bodyground(mon, 0);
          bodyfield(mon);
          macromonster = mon - 10;
          newland(0L, 0L, -1, monster[mon - 10].todoondeath, mon);
          macromonster = 0;
          monster[mon - 10].traiter = 0;
        } else {
          loaddoor2(todoque[performque - 1].doorid); /**** load qued macro codes to be executed ****/
          if (todoque[performque - 1].monsterpostion)
            macromonster = todoque[performque - 1].monsterpostion;
          newland(0L, 0L, -1, todoque[performque - 1].doorid, -(performque));
        }

        if (q[up] != -1)
          combatupdate2(q[up]);

        /*** return from macro codes here ****/

        for (t = 0; t < 8; t++) /**** restore old door codes ****/
        {
          infodoor.code[t] = holddoor.code[t];
          infodoor.id[t] = holddoor.id[t];
        }

        for (t = 0; t < 5; t++)
          extracode[t] = holdcode[t]; // Fantasoft v7.1 **** restore old E-Codes ****/

        macro = physical = FALSE;
        goto skipfield;
      }
    }

  skipmacro:

    bodyground(mon, 0);
    bodyfield(mon);

  skipfield:

    pict(207, spellrect);
  } else {
    if (incombat)
      spelleffect(mon, 0);
    killparty++;
    sound(132);
    updatecharshort(mon, TRUE);
    MoveTo(30, 400);
    if (c[mon].stamina < -9)
      MyrDrawCString("Death!");
    else
      MyrDrawCString("Unconscious!");
    delay(0);
    if (mon == charselectnew)
      updatecontrols();
    EraseRect(&textrect);
  }

  if (killparty > charnum)
    partyloss(0);

  if (incombat) {
    if (!performque) {
      for (t = 0; t <= charnum; t++) {
        if ((c[t].position == -1) || (c[t].stamina < 1) || (c[t].traiter) || (c[t].condition[25] < 0))
          escape++;
      }

      if (escape > charnum) {
        killmon = numenemy;
        coward = TRUE;
        getup(FALSE);
      }
    }
    for (t = 0; t <= charnum; t++)
      if (c[t].target == mon)
        c[t].target = -1;
    for (t = 0; t < nummon; t++)
      if (monster[t].target == mon)
        monster[t].target = -1;
  }
}
