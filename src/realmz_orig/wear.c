#include "variables.h"

/*************************** wear ************************/
short wear(short character, short itemnum, short play) {
  short t;
  Boolean correct;
  short holdtype;
  Rect box2;

  if ((incombat) && (character != q[up]) && (play)) {
    warn(10);
    return (0);
  }
  box2.right = 640;
  box2.bottom = 460;
  box2.left = box2.right - 356;
  box2.top = box2.bottom - 119;

  reply = FALSE;
  loaditem(c[character].items[itemnum].id);

  item.type = holdtype = abs(item.type);

  if ((incombat) && (c[character].condition[25]))
    play = FALSE;

  if (!canuse(c[character].items[itemnum].id, c[character].race, c[character].caste)) {
    if (play)
      warn(29);
    goto out;
  }

  if ((item.cost < 0) && (allowunique)) /**** check for unique item *****/
  {
    for (t = 0; t <= charnum; t++) {
      if (t != character) {
        if (checkforitem(item.itemid, FALSE, t)) {
          if (play)
            warn(116);
          goto out;
        }
      }
    }
  }

  if (item.type > 19) {
    if (play)
      warn(48);
    goto out;
  }

  if (item.type > 1) {
    if (c[character].armor[item.type]) {

      correct = FALSE;
      for (t = 0; t <= c[character].numitems; t++) {
        loaditem(c[character].items[t].id);
        if ((abs(item.type) == holdtype) && (c[character].items[t].equip))
          correct = TRUE;
      }

      if (correct) {
        if (play)
          warn(9);
        goto out;
      } else {
        loaditem(c[character].items[itemnum].id);
        item.type = holdtype = abs(item.type);
        c[character].armor[item.type] = 0;
        if (item.nohands)
          c[character].nohands = 0;
      }
    }
  }

  switch (item.type) {
    case 2: /***************** melee weapon ****************/
      if ((c[character].nohands < 2) && (!c[character].armor[2])) {
        if (item.nohands + c[character].nohands > 2) {
          if (play)
            warn(8);
          goto out;
        }
        c[character].armor[2] = item.itemid;
        c[character].weaponsound = item.sound;
        c[character].nohands += item.nohands;
        if (c[character].items[itemnum].ident)
          c[character].weaponnum = 1;
        else
          c[character].weaponnum = 0;
        reply = TRUE;

        c[character].toggle = FALSE;
      }
      break;

    case 15: /***************** missile weapon ****************/
      if (!c[character].armor[15]) {
        if (MyrBitTstLong(&item.itemcat[0], 12)) /**** look for normal bow and require quiver ****/
        {
          if (!c[character].armor[10]) {
            if (play)
              warn(24);
            goto out;
          }
        }

        c[character].armor[15] = item.itemid;
        if (c[character].items[itemnum].ident)
          c[character].missilenum = 1;
        else
          c[character].missilenum = 0;
        c[character].toggle = TRUE;
        reply = TRUE;
      }
      break;

    case 3: /*******   Shield  **************/

      if (c[character].nohands + item.nohands > 2) {
        if (play)
          warn(8);
        goto out;
      }

      c[character].armor[3] = item.itemid;
      c[character].nohands++;
      reply = TRUE;

      break;

    case 0: /*******   Ring  **************/

      if ((c[character].armor[0] == 0) || (c[character].armor[1] == 0)) {
        reply = TRUE;
        if (c[character].armor[0] == 0)
          c[character].armor[0] = item.itemid; /*** rings *****/
        else
          c[character].armor[1] = item.itemid;

        if (c[character].armor[0] == c[character].armor[1]) {
          c[character].armor[1] = 0;
          reply = FALSE;
          if (play)
            warn(31);
          goto out;
        }
      } else {
        if (play)
          warn(9);
        goto out;
      }
      break;

    default:
      if (item.type < 20)
        c[character].armor[item.type] = item.itemid;
      reply = TRUE;
      break;
  }

  if (reply) {
    c[character].items[itemnum].equip = TRUE;
    if (item.iscurse) {
      c[character].items[itemnum].ident = TRUE;
      warn(7);
    }
    characterr = c[character];
    c[character].magst += item.st;

    if (c[character].ac + item.ac < 0)
      c[character].ac = 0;
    else
      c[character].ac += item.ac;
    c[character].maglu += item.lu;
    c[character].magres += item.magres;
    c[character].spellpointsmax += item.spellpoints;
    c[character].spellpoints += item.spellpoints;
    c[character].damage += item.damage;
    if (c[character].damage > 110)
      c[character].damage = 110;
    c[character].movebonus += item.movement;

    movecalc(character);

    if ((item.sp1 > 59) && (item.sp1 < 100))
      c[character].condition[item.sp2] = 0; /**** neutralize condition ***/
    if (item.sp1 == 122) /******** item adds attacks **********/
    {
      c[character].attackbonus += item.sp2;
    }

    if ((item.sp1 > 19) && (item.sp1 < 60)) /******* adds condition *****/
    {
      if (c[character].condition[item.sp1 - 20] > -1)
        c[character].condition[item.sp1 - 20] = 0;
      c[character].condition[item.sp1 - 20] += item.sp2; /**** make condition[sp1-20] = sp2 ***/
    }

    if (item.sp3) /******* adds special ability *****/
    {
      if (item.sp3 < 0)
        c[character].special[abs(item.sp3) - 1] += item.sp5;
      else if ((item.sp3 < 16) && (item.sp3 > 0))
        c[character].spec[item.sp3 - 1] += item.sp5;
      else
        partycondition[item.sp3 - 30] -= abs(item.sp5);
    }

    if (item.sp4) {
      if (item.sp4 < 0)
        c[character].special[abs(item.sp4) - 1] += item.sp5;
      else if ((item.sp4 < 16) && (item.sp4 > 0))
        c[character].spec[item.sp4 - 1] += item.sp5;
      else
        partycondition[item.sp4 - 30] -= abs(item.sp5);
    }
  }

  if (reply)
    if ((item.sound) && (play))
      sound(600 + item.sound);
out:

  select[character] = reply;
  return (reply);
}
