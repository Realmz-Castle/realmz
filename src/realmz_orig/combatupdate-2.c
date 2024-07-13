#include "prototypes.h"
#include "variables.h"

/************************* combatupdate ***********************/
void combatupdate(short body) {
  short temp, tempmax, tempstamina, tempstaminamax;
  SetPort(GetWindowPort(screen));
  TextFont(font);
  TextSize(15);
  TextMode(1);
  RGBBackColor(&greycolor);

  if (body < 0)
    return;

  if (body < 6) {
    temp = c[body].movement;
    tempmax = c[body].movementmax;
    tempstamina = c[body].stamina;
    tempstaminamax = c[body].staminamax;
  } else {
    temp = monster[body - 10].movement;
    tempmax = monster[body - 10].movementmax;
    tempstamina = monster[body - 10].stamina;
    tempstaminamax = monster[body - 10].staminamax;
  }
  box.top = 354 + downshift;
  box.left = 167;
  box.right = box.left + 35;
  box.bottom = box.top + 15;
  EraseRect(&box);
  if (temp < tempmax)
    ForeColor(whiteColor);
  else
    ForeColor(yellowColor);
  MoveTo(169, 366 + downshift);

  if (temp < 0)
    temp = 0;
  string(temp);

  box.top = 329 + downshift;
  box.bottom = box.top + 15;
  EraseRect(&box);
  if (tempstamina < tempstaminamax)
    ForeColor(whiteColor);
  else
    ForeColor(yellowColor);
  MoveTo(169, 341 + downshift);

  string(tempstamina);
}
/************************ combatupdate2 ***********************/
void combatupdate2(short body) {
  Rect iconrect;
  SetPort(GetWindowPort(screen));
  TextFont(font);
  RGBBackColor(&greycolor);
  lastshown = body;

  if (body < 0)
    return;

  if (!infocombat) {
    pict(153 - (8 * inspell), buttons);
    pict(156 + 18 * inspell, infosmall);
    number();
  } else if (!inspell) {
    flashrange(body, charup);
    if (body < 9) {
      if (!cansee(pos[charup][0], pos[charup][1], pos[body][0], pos[body][1]))
        MyrDrawCString(" Blocked");
    } else {
      if (!cansee(pos[charup][0], pos[charup][1], monpos[body - 10][0], monpos[body - 10][1]))
        MyrDrawCString(" Blocked");
    }
  }

  pict(166, info);

  if (screensize) {
    iconrect = info;
    iconrect.left = iconrect.right;
    iconrect.right += leftshift;
    pict(164, iconrect);

    iconrect.top = 300;
    iconrect.bottom = 300 + downshift;
    iconrect.left = 320 + leftshift;
    iconrect.right = 800;
    EraseRect(&iconrect);

    if (body > 9) //*** show monster immune/vulnerabilties.
    {
      itemRect.left = 335;
      itemRect.right = itemRect.left + 9;
      itemRect.top = 450;
      itemRect.bottom = itemRect.top + 9;

      for (t = 0; t < 6; t++) //**** immune
      {
        if (monster[body - 10].spellimmune[t])
          pict(150, itemRect);
        else
          pict(146, itemRect);

        itemRect.top += 17;
        itemRect.bottom += 17;
      }

      itemRect.left = 422;
      itemRect.right = itemRect.left + 9;
      itemRect.top = 467;
      itemRect.bottom = itemRect.top + 9;

      for (t = 0; t < 5; t++) //**** vuls
      {
        if (monster[body - 10].save[t] < 0)
          pict(150, itemRect);
        else
          pict(146, itemRect);

        itemRect.top += 17;
        itemRect.bottom += 17;
      }
    }
  }

  TextSize(15);

  iconrect.top = 344 + downshift;
  iconrect.left = 23;
  iconrect.right = iconrect.left + 32;
  iconrect.bottom = iconrect.top + 32;

  iconhand = NIL;

  if (body < 10) {
    iconhand = GetCIcon(c[body].iconid);
    if (body == q[up]) {
      if (c[body].armor[15]) {
        GetControlBounds(melee, &buttonrect);
        // buttonrect = *&(**(melee)).contrlRect;
        upbutton(TRUE);
      }

      if (!c[body].toggle)
        loaditem(c[body].armor[2]);
      else
        loaditem(c[body].armor[15]);

      if (item.sp2 > 1100) {
        for (t = 0; t < c[body].numitems; t++) {
          if (c[body].items[t].id == item.itemid)
            break;
        }

        if (c[body].items[t].charge) {
          if (!inspell) {
            GetControlBounds(combatitem, &buttonrect);
            // buttonrect = *&(**(combatitem)).contrlRect;
            upbutton(TRUE);
          }
        }
      }
    }
  } else {
    size = monster[body - 10].size;
    temp = monster[body - 10].iconid;

    if (monster[body - 10].lr == 1)
      temp += 308;
    iconhand = GetCIcon(temp);

    if ((size == 1) || (size == 3)) {
      iconrect.top -= 16;
      iconrect.bottom += 16;
    }
    if (size > 1) {
      iconrect.left -= 16;
      iconrect.right += 16;
    }
  }

  if (iconhand) {
    PlotCIcon(&iconrect, iconhand);
    DisposeCIcon(iconhand);
  }

  combatupdate(body);

  ForeColor(yellowColor);
  MoveTo(265, 341 + downshift);

  if (body < 10) {
    string(c[body].ac);
    MoveTo(169, 341 + downshift);
    if (c[body].stamina < c[body].staminamax)
      ForeColor(whiteColor);
    string(c[body].stamina);
    MoveTo(173, 390 + downshift);
    if (c[body].spellpoints < c[body].spellpointsmax)
      ForeColor(whiteColor);
    else
      ForeColor(yellowColor);
    string(c[body].spellpoints);
  } else {
    string(monster[body - 10].ac);
    MoveTo(169, 341 + downshift);
    if (monster[body - 10].stamina < monster[body - 10].staminamax)
      ForeColor(whiteColor);
    string(monster[body - 10].stamina);
    MoveTo(173, 390 + downshift);
    if (monster[body - 10].spellpoints < monster[body - 10].maxspellpoints)
      ForeColor(whiteColor);
    else
      ForeColor(yellowColor);
    string(monster[body - 10].spellpoints);
  }

  ForeColor(yellowColor);

  iconrect.top = 397 + downshift;
  iconrect.left = 7;
  iconrect.right = iconrect.left + 32;
  iconrect.bottom = iconrect.top + 32;

  MoveTo(115, 408 + downshift);
  TextSize(13);

  iconhand = NIL;

  if (body < 10) {
    if (c[body].toggle) /**** missile weapon ***/
    {
      if (c[body].armor[15]) {
        loaditem(c[body].armor[15]);
        reply = 0;
        if (c[body].missilenum)
          reply = 1;
        GetIndString(myString, getselection(item.itemid) + reply, item.itemid - tempselection + 1);
        iconhand = GetCIcon(item.iconid);
      } else {
        iconhand = GetCIcon(7002);
        GetIndString(myString, 3, 4);
        c[body].toggle = 0;
      }
    } else {
      if (c[body].armor[2]) /**** melee weapon ***/
      {
        loaditem(c[body].armor[2]);
        reply = 0;
        if (c[body].weaponnum)
          reply = 1;
        GetIndString(myString, getselection(item.itemid) + reply, item.itemid - tempselection + 1);
        iconhand = GetCIcon(item.iconid);
      } else {
        iconhand = GetCIcon(7002);
        GetIndString(myString, 3, 4);
      }
    }
    PtoCstr(myString);
    MyrDrawCString((Ptr)myString);
    if (iconhand) {
      PlotCIcon(&iconrect, iconhand);
      DisposeCIcon(iconhand);
    }

    MoveTo(120, 428 + downshift);
    MyrDrawCString(c[body].name);
  } else {
    if (monster[body - 10].weapon) {
      loaditem(monster[body - 10].weapon);
      GetIndString(myString, getselection(item.itemid), item.itemid - tempselection + 1);
      iconhand = GetCIcon(item.iconid);
    } else {
      iconhand = GetCIcon(7002);
      GetIndString(myString, 3, 4);
    }
    PtoCstr(myString);
    MyrDrawCString((Ptr)myString);

    if (iconhand) {
      PlotCIcon(&iconrect, iconhand);
      DisposeCIcon(iconhand);
    }

    strcpy((StringPtr)gotword, monster[body - 10].monname);

    MoveTo(120, 428 + downshift);
    MyrDrawCString(gotword);
  }
  MoveTo(73, 449 + downshift);

  if (body < 10) {
    if (c[body].weaponsound != 38) {
      if (c[body].toggle)
        temp = c[body].armor[15];
      else
        temp = c[body].armor[2];

      for (t = 0; t < c[body].numitems; t++) {
        if (c[body].items[t].id == temp)
          break;
      }

      if (c[body].items[t].charge < 0)
        MyrDrawCString("--");
      else
        string(c[body].items[t].charge);

    } else
      MyrDrawCString("--");
  } else {
    if (monster[body - 10].weapon)
      MyrDrawCString("??");
    else
      MyrDrawCString("--");
  }

  for (t = 0; t < 40; t++) {
    if (body < 10) {
      if (c[body].condition[t])
        goto updateit;
    } else if (monster[body - 10].condition[t]) {
    updateit:
      GetIndString(myString, 133, t + 1);
      MoveTo(120, 449 + downshift);
      PtoCstr(myString);
      MyrDrawCString((Ptr)myString);
      return;
    }
  }
}
