#include "prototypes.h"
#include "variables.h"

/************************************ showcondition **********************/
short showcondition(short start, short stop, short type, short showonly, short who) {
  short t, tt, itemHit, index = 0;
  short startvalue = 0;
  short markresult;
  MenuHandle popup;
  Boolean update = 0;
  Boolean canequip = TRUE;
  char special[50];

  sound(609);

  if (gTheEvent.modifiers & cmdKey) {
    start = 0;
    stop = charnum;
    canequip = FALSE;
  }

  popup = GetMenu(131);
  InsertMenu(popup, -1);

  for (tt = start; tt <= stop; tt++) {
    if ((type != 4) && (type != 3)) {
      CtoPstr(c[tt].name);
      AppendMenu(popup, (StringPtr)c[tt].name);
      PtoCstr((StringPtr)c[tt].name);
      index++;
      SetItemIcon(popup, index, c[tt].pictid);
    }

    switch (type) {
      case 0: /***** condition *****/
        for (t = 0; t < 40; t++) {
          if (c[tt].condition[t]) {
            index++;
            GetIndString(myString, 133, t + 1);
            AppendMenu(popup, myString);
            if (c[tt].condition[t] > 0)
              SetItemMark(popup, index, -41);
            else
              SetItemMark(popup, index, 19);
          }
        }
        break;

      case 1: /****** items *****/
        for (t = 0; t < 30; t++) {
          if (c[tt].items[t].id) {
            index++;
            loaditem(c[tt].items[t].id);
            getselection(item.itemid);
            if ((item.iscurse) && (!c[tt].items[t].equip))
              GetIndString(myString, tempselection + c[tt].items[t].ident, item.iscurse + 1);
            else
              GetIndString(myString, tempselection + c[tt].items[t].ident, item.itemid - tempselection + 1);
            AppendMenu(popup, myString);
            if (c[tt].items[t].equip)
              SetItemMark(popup, index, 19);
          }
        }
        break;

      case 2: /****** special abilities *****/
        for (t = 0; t < 12; t++) {
          if (c[tt].special[t]) {
            strcpy(special, (StringPtr) "");
            index++;

            if (c[tt].special[t] > 0)
              strcpy(special, (StringPtr) " +");
            else
              strcpy(special, (StringPtr) " ");

            MyrNumToString(c[tt].special[t], myString);
            // PtoCstr(myString);
            strcat(special, myString);
            GetIndString(myString, 132, t + 1);
            PtoCstr(myString);
            strcat(special, myString);
            CtoPstr(special);
            AppendMenu(popup, (StringPtr)special);
          }
        }
        break;

      case 3: /****** usable items by this PC *****/
        loadprofile(c[who].race, c[who].caste);
        MyrAppendMenu(popup, (Ptr) "This character can use items of type:");
        SetItemIcon(popup, 1, c[who].pictid);
        startvalue = 1;
        for (t = 0; t <= 57; t++) {
          GetIndString(myString, 6, t + 1);
          if (StringWidth(myString)) {
            if ((MyrBitTstLong(&caste.itemtypes[0], t)) && (MyrBitTstLong(&races.itemtypes[0], t)))
              AppendMenu(popup, myString);

            if (MyrBitTstLong(&item.itemcat[0], t)) {
              startvalue = t + 2;
              SetItemIcon(popup, t + 2, 379);
            }
          }
        }
        break;

      case 4: /****** What PCs can use this item *****/
        startvalue = 1;
        gTheEvent.where.v = GlobalTop - 5;
        gTheEvent.where.h = GlobalLeft + 335;
        MyrAppendMenu(popup, (Ptr) "Characters Who Can Use This Item.");
        SetItemIcon(popup, 1, 379);
        for (tt = 0; tt <= charnum; tt++) {
          CtoPstr(c[tt].name);
          AppendMenu(popup, (StringPtr)c[tt].name);
          SetItemIcon(popup, tt + 2, c[tt].pictid);
          PtoCstr((StringPtr)c[tt].name);

          if (!canuse(item.itemid, c[tt].race, c[tt].caste))
            DisableItem(popup, tt + 2); /********* usable by this character? *************/
        }
        SetItemMark(popup, who + 2, 19);
        break;
    }
  }
  if ((type != 3) && (type != 4))
    itemHit = PopUpMenuSelect(popup, 50, 400, startvalue);
  else
    itemHit = PopUpMenuSelect(popup, gTheEvent.where.v, gTheEvent.where.h - 50, startvalue);
  GetItemMark(popup, itemHit, &markresult);
  DeleteMenu(131);
  ReleaseResource((Handle)popup);
  temp = c[start].armor[2];

  if ((type) && (itemHit > 1) && (canequip) && !(showonly)) {
    switch (markresult) {
      case 19:
        if (removeitem(start, itemHit - 2, TRUE, FALSE)) {
          updatechar(start, 3);
          update = TRUE;
        } else
          return (-1);
        break;

      case 0:
        if (wear(start, itemHit - 2, TRUE)) {
          c[start].items[itemHit - 2].equip = 1;
          updatechar(start, 3);
          update = TRUE;
        }
        break;
    }
  } else
    sound(607);

  if ((incombat) && (update)) {
    combatupdate2(start);
    updatecontrols();
  }
  return (itemHit - 2);
}

/***************************** popupchoice ********************************/
short popupchoice(short stringid) {
  short markresult, itemHit;
  MenuHandle popup;

  popup = GetMenu(131);
  InsertMenu(popup, -1);

  for (t = 1; t < 100; t++) {
    GetIndString(myString, stringid, t);
    AppendMenu(popup, myString);
  }

  itemHit = PopUpMenuSelect(popup, 50, 400, 0);
  GetItemMark(popup, theItem, &markresult);
  DeleteMenu(131);
  ReleaseResource((Handle)popup);
  return (itemHit);
}
