#include "prototypes.h"
#include "variables.h"

/************************************ showcondition2 **********************/
void showcondition2(short who, short type) {
  short index = 0;
  MenuHandle popup;

  who -= 10;

  if (who < 0)
    return;

  for (t = 0; t <= numchannel; t++)
    quiet(t);
  compactheap();

  sound(609);
  popup = GetMenu(131);
  InsertMenu(popup, -1);

  strcpy((StringPtr)gotword, monster[who].monname);
  CtoPstr(gotword);
  AppendMenu(popup, (StringPtr)gotword);
  index++;
  SetItemIcon(popup, index, monster[who].iconid);
  switch (type) {
    case 0: /***** condition *****/
      for (t = 0; t < 40; t++) {
        if (monster[who].condition[t]) {
          index++;
          GetIndString(myString, 133, t + 1);
          AppendMenu(popup, myString);
        }
      }
      break;

    case 1: /****** items *****/
      for (t = 0; t < 6; t++) {
        if (monster[who].items[t]) {
          index++;
          loaditem(monster[who].items[t]);
          GetIndString(myString, getselection(item.itemid), item.itemid - tempselection + 1);
          AppendMenu(popup, myString);
          if (monster[who].items[t] < 0)
            CheckItem(popup, index, TRUE);
        }
      }
      break;
  }
  itemHit = PopUpMenuSelect(popup, 40, 400, 0);
  DeleteMenu(131);
  sound(607);
  ReleaseResource((Handle)popup);
}
