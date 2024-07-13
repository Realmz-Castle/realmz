#include "prototypes.h"
#include "variables.h"

/********************************** getnumspells ***********************************/
short getnumspells(short spellcastertype, short casteid, short level) {
  short spellselectpoints = 0;
  short t;
  short baselevel = 1;

  loadprofile(0, casteid);

  baselevel = caste.spellcasters[0][1] + caste.spellcasters[1][1] + caste.spellcasters[2][1];

  baselevel--;

  level -= baselevel;

  if (level < 1)
    return (0);

  switch (spellcastertype) {
    default: /********** Sorcerer / Enchanter **************/
      if (characterl.in > 15)
        spellselectpoints += (level * (characterl.in - 15));
      break;

    case 2: /********** Priest **************/
      if (characterl.wi > 15)
        spellselectpoints += (level * (characterl.wi - 15));
      break;
  }

  spellselectpoints += (3 * level);
  for (t = 1; t < level; t++)
    spellselectpoints += t;

out:

  return (spellselectpoints);
}
