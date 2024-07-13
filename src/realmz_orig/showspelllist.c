#include "prototypes.h"
#include "variables.h"

/************************** showspelllist ****************/
void showspelllist(void) {
  register short t, class;

  class = c[charselectnew].spellcastertype - 1;

  for (t = 0; t < 12; t++) {
    GetDialogItem(gCurrent, t + 1, &itemType, &itemHandle, &buttonrect);
    buttonrect.left -= 2;
    buttonrect.right += 2;
    buttonrect.top--;
    if (c[charselectnew].cspells[castlevel][t]) {
      loadspell(class, castlevel, t);
      GetIndString(myString, 1000 * (class + 1) + castlevel, t + 1);
      if (((incombat) && (spellinfo.incombat)) || ((!incombat) && (spellinfo.incamp)) || (scribing == 2) || (encountflag)) {
        upbutton(FALSE);
        ForeColor(yellowColor);
        MyrPascalDiStr(t + 1, myString);
      } else {
        downbutton(FALSE);
        MyrPascalDiStr(t + 1, myString);
      }
    } else {
      downbutton(FALSE);
      MyrCDiStr(t + 1, (StringPtr) "");
    }
  }
}
