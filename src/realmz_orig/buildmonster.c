#include "prototypes.h"
#include "variables.h"

/****************************** buildmonster **********************/
void buildmonster(short traiter) {
  short tempt;
  float tempfloat, multiply = 0;

  monpick.attacknum = 0;
  monpick.stamina = monpick.bonus;
  for (tempt = 0; tempt < monpick.hd; tempt++)
    monpick.stamina += Rand(8);
  monpick.staminamax = monpick.stamina;
  if (traiter > -1)
    monpick.traiter = traiter;
  monpick.lr = monpick.up = 0;
  monpick.ac += (Rand(3) - 2); /**** randomate the ac a bit ***/
  monpick.dx += (Rand(3) - 2); /**** randomate dx a bit ***/
  if ((monpick.dx) < 1)
    monpick.dx = 1;

  temp = monpick.spellpoints / 10;
  monpick.spellpoints += randrange(-temp, temp); /**** randomate spellpoints a bit ***/

  /************ update for difficulty level ********/
  if ((monpick.magres < 99) && (monpick.magres > 9))
    monpick.magres += (3 * howhard); // Fantasoft v7.0
  if (monpick.magres < 99)
    monpick.magres += (3 * howhard);
  if (monpick.magres < 0)
    monpick.magres = 0;
  for (tempt = 0; tempt < 6; tempt++)
    monpick.save[tempt] += 7 * howhard;
  monpick.ac -= (2 * howhard);
  monpick.dx += howhard;

  multiply = (1 + (howhard * .33));

  tempfloat = monpick.spellpoints;
  tempfloat *= multiply;

  monpick.spellpoints = tempfloat;

  monpick.maxspellpoints = monpick.spellpoints;

  tempfloat = monpick.stamina;
  tempfloat *= multiply;

  monpick.stamina = tempfloat;

  if (monpick.stamina < 1)
    monpick.stamina = 1;

  monpick.stamina += (tyme.tm_yday / (180 - (30 * howhard)));

  monpick.staminamax = monpick.stamina;

  if (monpick.weapon < 0) /****** get random weapon *******/
  {
    monpick.weapon = monpick.items[0] = combatsetup(monpick.weapon, 0, 0);
  }
}
