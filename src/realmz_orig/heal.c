#include "prototypes.h"
#include "variables.h"

/********************************** heal/ hurt ****************************/
void heal(short who, short howmuch, short show) {
  Boolean canrevive = FALSE;
  Boolean dead = FALSE;
  Boolean ok = TRUE;

  if ((who < 0) || (who > charnum))
    return;

  if (show) {
    spellinfo.spelllook2 = 15;
    spelleffect(who, 0);
  }

  if (c[who].condition[26])
    return;

  if (c[who].stamina < 1)
    ok = FALSE;
  if (c[who].stamina < -9)
    dead = TRUE;
  if ((c[who].stamina < 1) && (!dead))
    canrevive = TRUE;

  if (!dead)
    c[who].stamina += howmuch;
  if (c[who].stamina < -10)
    c[who].stamina = -10;
  if ((c[who].stamina > 0) && (canrevive))
    killparty--;
  if (c[who].stamina > c[who].staminamax)
    c[who].stamina = c[who].staminamax;

  if ((c[who].stamina < -9) && (!dead) && (!ok)) {
    updatecharshort(who, TRUE);
    sound(132);
  }

  if ((c[who].stamina < 1) && (ok)) {
    show = FALSE;
    killbody(who, 0);
    if ((!incombat) && (!encountflag))
      centerpict();
  } else if ((c[who].stamina > 0) && (!ok)) {
    updatecharshort(who, TRUE);
  } else
    updatecharshort(who, FALSE);

  if (killparty > charnum)
    partyloss(0);
  if (initems)
    select[who] = -1;
}
