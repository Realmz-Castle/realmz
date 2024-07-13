#include "prototypes.h"
#include "variables.h"

/*************************** loadspell2 ************************/
void loadspell2(short id) {

  if (id < 1101)
    return;

  castcaste = id / 1000;
  id -= 1000 * castcaste;
  castcaste--;
  castlevel = id / 100;
  id -= 100 * castlevel;
  castlevel--;
  castnum = id - 1;

  spellinfo = spelldata[castcaste][castlevel][castnum];
}

/*************************** loadspell ************************/
void loadspell(short castcaste, short castlevel, short castnum) {
  spellinfo = spelldata[castcaste][castlevel][castnum];
}
