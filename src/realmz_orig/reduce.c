#include "prototypes.h"
#include "variables.h"

/*************************** reduce conditions *******************/
void reduce(void) {
  short t, tt;
  Boolean verge, tag = FALSE;
  Boolean wizard = FALSE;
  Rect flamerect;

  if (partycondition[0] == 1)
    tag = TRUE;
  if (partycondition[4])
    wizard = TRUE;

  SetRect(&flamerect, 367, 330, 399, 362); /**** L T R B ****/

  for (t = 1; t < 10; t++) {
    if ((partycondition[t] == 1) && (t != 5)) {
      SetPort(GetWindowPort(screen));
      EraseRect(&flamerect);
    }
    if (t == 5)
      OffsetRect(&flamerect, -156, 40);
    OffsetRect(&flamerect, 39, 0);
  }

  for (t = 0; t < 10; t++)
    if (partycondition[t] > 0)
      partycondition[t]--;

  if (partycondition[0] > 0)
    partycondition[0]--;

  if (!incombat) {
    if ((!partycondition[4]) && (wizard)) /**** wizard eye *****/
    {
      if ((!multiview) && (indung)) {
        warn(95);
        viewtype = 1;
        UpdateWindow(FALSE);
      }
    }

    if (partycondition[0] / 30 != (partycondition[0] + 1) / 30) {
      loaddark(partycondition[0] / 30 + 1);
      centerpict();
    } else if (tag) {
      loaddark(0);
      centerpict();
      updatetorch();
    }
  }

  for (t = 0; t <= charnum; t++) {
    if ((c[t].condition[10]) && (c[t].stamina > -10) && (c[t].stamina < c[t].staminamax) && (!c[t].condition[25])) /***** Regenerate ****/
    {
      if (((incombat) && (c[t].stamina > 0)) || (!incombat)) {
        damage = abs(c[t].condition[10]);
        heal(t, damage, FALSE);
        regenerate = TRUE;
        if ((damage) && (incombat) && (c[t].inbattle))
          showresults(t, 0, 0); /********* regenerate for ***************/
        else
          updatecharshort(t, FALSE);
        regenerate = FALSE;
      }
    }

    if ((c[t].condition[28]) && (c[t].stamina > 0) && (c[t].condition[25] > -1)) /***** disease ****/
    {
      damage = abs(c[t].condition[28]);
      c[t].stamina -= damage;
      spellinfo.spelllook2 = 7;
      if (c[t].stamina < 1) {
        killbody(t, 0);
        flashmessage((StringPtr) "Disease takes its toll!", 30, 100, 100, 10122);
      } else if ((incombat) && (c[t].inbattle))
        showresults(t, 29, 0); /********* diseased ***************/
      else
        updatecharshort(t, FALSE);
    }

    if ((c[t].condition[9]) && (c[t].stamina > 0) && (c[t].condition[25] > -1)) /***** poison ****/
    {
      spellinfo.spelllook2 = 7;
      damage = abs(c[t].condition[9]);
      c[t].stamina -= damage;
      poisoned = TRUE;
      if (c[t].stamina < 1) {
        killbody(t, 0);
        flashmessage((StringPtr) "Poison takes its toll!", 30, 100, 100, 684);
      } else if ((incombat) && (c[t].inbattle))
        showresults(t, 0, 0); /********* poisoned for ***************/
      else
        updatecharshort(t, FALSE);
      poisoned = FALSE;
    }

    if ((c[t].condition[34]) && (c[t].stamina > 0)) /****** power suck *******/
    {
      c[t].spellpoints -= abs(c[t].condition[34]);
      if (c[t].spellpoints < 0)
        c[t].spellpoints = 0;
      updatecharshort(t, FALSE);
    }

    if ((c[t].condition[33]) && (c[t].spellpointsmax) && (c[t].stamina > 0)) /******* power absorbe ********/
    {
      c[t].spellpoints += abs(c[t].condition[33]);
      if (c[t].spellpoints > c[t].spellpointsmax)
        c[t].spellpoints = c[t].spellpointsmax;
      updatecharshort(t, FALSE);
    }

    if (c[t].condition[29]) {
      if (c[t].traiter) {
        c[t].traiter = 0;
        killparty--;
        numenemy--;
      }
    }

    verge = FALSE;
    for (tt = 0; tt < 40; tt++) /**** Party ****/
    {

      if (c[t].condition[tt] == 1)
        verge = TRUE;
      if (c[t].condition[tt] > 0)
        c[t].condition[tt]--;
    }
    if (verge)
      updatechar(t, 3);
  }

  if (!incombat) {
    for (t = 0; t < heldover; t++) {
      for (tt = 0; tt < 20; tt++) /**** Allies ****/
      {
        if (holdover[t].condition[tt] > 0)
          holdover[t].condition[tt]--;
      }
    }
  }
}
