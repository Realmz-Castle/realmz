#include "prototypes.h"
#include "variables.h"

/*************************** remove item **********************/
short removeitem(short character, short itemnum, short play, short force) {
  Rect box2;

  if ((incombat) && (character != q[up]) && (play)) {
    warn(10);
    return (0);
  }

  box2.right = 640;
  box2.bottom = 460;
  box2.left = box2.right - 356;
  box2.top = box2.bottom - 119;
  loaditem(c[character].items[itemnum].id);

  item.type = abs(item.type);

  if ((item.iscurse) && (!force)) {
    warn(7);
    return (0);
  }
  switch (item.type) {
    case 2: /***************** melee weapon ****************/
      c[character].armor[2] = 0;
      c[character].weaponsound = 38;
      c[character].nohands -= item.nohands;
      if (c[character].nohands < 0)
        c[character].nohands = 0;
      if (c[character].armor[15])
        c[character].toggle = TRUE;
      break;

    case 15: /***************** missile weapon ****************/
      c[character].armor[15] = 0;
      c[character].toggle = FALSE;

      break;

    case 10: /***************** quiver ****************/

      if (c[character].armor[15]) {
        if (play)
          warn(30);
        return (0);
      }

      c[character].armor[10] = 0;
      break;

    case 3: /*******   Shield  **************/

      c[character].nohands--;
      if (c[character].nohands < 0)
        c[character].nohands = 0;
      c[character].armor[3] = 0;

      break;

    case 0: /*******   Rings  **************/

      if (c[character].armor[0] == item.itemid)
        c[character].armor[0] = 0; /*** rings *****/
      else
        c[character].armor[1] = 0;

      break;

    default: /*******   default  **************/

      if (item.type < 20)
        c[character].armor[abs(item.type)] = 0;

      break;
  }

  if (item.sp1 == 122) /******** item adds attacks **********/
  {
    c[character].attackbonus -= item.sp2;
  }

  if (item.sp3) {
    if (item.sp3 < 0)
      c[character].special[abs(item.sp3) - 1] -= item.sp5;
    else if ((item.sp3 < 16) && (item.sp3 > 0))
      c[character].spec[item.sp3 - 1] -= item.sp5;
    else
      partycondition[item.sp3 - 30] = 0;
  }

  if (item.sp4) {
    if (item.sp4 < 0)
      c[character].special[abs(item.sp4) - 1] -= item.sp5;
    else if ((item.sp4 < 16) && (item.sp4 > 0))
      c[character].spec[item.sp4 - 1] -= item.sp5;
    else
      partycondition[item.sp4 - 30] = 0;
  }

  c[character].items[itemnum].equip = FALSE;
  c[character].magst -= item.st;
  c[character].maglu -= item.lu;
  c[character].magres -= item.magres;
  if (c[character].ac - item.ac > 0)
    c[character].ac -= item.ac;
  else
    c[character].ac = 0;
  c[character].spellpointsmax -= item.spellpoints;
  c[character].spellpoints -= item.spellpoints;
  c[character].damage -= item.damage;
  c[character].movebonus -= item.movement;
  movecalc(character);

  if ((item.sp1 > 59) && (item.sp1 < 99))
    c[character].condition[item.sp2] = 0; /**** neutralize sp2 condition ****/
  if ((item.sp1 > 19) && (item.sp1 < 60)) {
    if (c[character].condition[item.sp1 - 20] < 0)
      c[character].condition[item.sp1 - 20] -= item.sp2;
    if (c[character].condition[item.sp1 - 20] > 0)
      c[character].condition[item.sp1 - 20] = 0;
    /**** remove permanent condition ****/
  }

  if ((item.sound) && (play))
    sound(600 + item.sound);
  select[character] = TRUE;
  return (1);
}
