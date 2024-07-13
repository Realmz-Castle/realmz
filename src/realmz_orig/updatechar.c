#include "prototypes.h"
#include "variables.h"

/************************** updatechar *****************************/
void updatechar(short who, short mode) {
  GrafPtr oldport;
  Rect rect;
  Boolean forcesmallshift = 0;
  short ydist;
  short temp;

  forcesmallshift = forcesmall * screensize;

  if ((who < 0) || (who > charnum))
    return;

  GetPort(&oldport);

  if ((!mode) || (mode == 3))
    SetPort(GetWindowPort(screen));
  TextMode(1);
  TextFont(font);
  TextFace(bold);

  rect.left = 380 + leftshift - (leftshift * forcesmallshift);
  rect.right = 640 + leftshift - (leftshift * forcesmallshift);
  rect.top = who * 50;
  rect.bottom = rect.top + 50;

  switch (mode) {
    case 2: {
      rect.left -= 60;
      EraseRect(&rect);
      return;
    } break;

    case 5:
      goto forpartyselect;
      break;

    case 3:

    forpartyselect:

      if (c[who].spellpointsmax)
        temp = 192;
      else
        temp = 189;
      pict(temp, rect);

      break;

    case 4:
      pict(184, rect); /***** do copy bits for booty *******/
      return;
      break;
  }

  ydist = rect.top + 41;
  if (mode)
    updatepictbox(who, 1, 0); /******* no initial box ******/
  ForeColor(yellowColor);
  TextSize(15);
  MoveTo(420 + leftshift - (leftshift * forcesmallshift), ydist - 22);
  MyrDrawCString(c[who].name);
  TextSize(17);

  if (c[who].ac > -1)
    MoveTo(605 + leftshift - (leftshift * forcesmallshift), ydist - 22);
  else
    MoveTo(600 + leftshift - (leftshift * forcesmallshift), ydist - 22);

  string(c[who].ac);

  MoveTo(450 + leftshift - (leftshift * forcesmallshift), ydist);

  if (c[who].staminamax > 999)
    TextSize(12);
  string(c[who].staminamax);

  MyrNumToString(c[who].stamina, myString);
  MoveTo((444 - TextWidth(myString, 0, strlen(myString))) + leftshift - (leftshift * forcesmallshift), ydist);
  if (c[who].stamina < c[who].staminamax)
    ForeColor(whiteColor);

  MyrDrawCString((Ptr)myString);

  TextSize(17);

  RGBForeColor(&cyancolor);

  if (c[who].spellpointsmax) {
    MoveTo(604 + leftshift - (leftshift * forcesmallshift), ydist);
    if (c[who].spellpointsmax > 999)
      TextSize(12);

    string(c[who].spellpointsmax);
    if (c[who].spellpoints < c[who].spellpointsmax)
      ForeColor(whiteColor);
    MyrNumToString(c[who].spellpoints, myString);
    MoveTo((593 - TextWidth(myString, 0, strlen(myString))) + leftshift - (leftshift * forcesmallshift), ydist);
    MyrDrawCString((Ptr)myString);
    TextSize(17);
  } else {
    MoveTo(598 + leftshift - (leftshift * forcesmallshift), ydist);
    temp = c[who].normattacks + c[who].attackbonus;
    if (c[who].condition[23])
      temp *= 2;
    if (c[who].condition[6])
      temp /= 2;
    switch (temp) {
      case 0:
        MyrDrawCString("0   1");
        break;
      case 1:
        MyrDrawCString("1   2");
        break;
      case 2:
        MyrDrawCString("1   1");
        break;
      case 3:
        MyrDrawCString("3   2");
        break;
      case 4:
        MyrDrawCString("2   1");
        break;
      case 5:
        MyrDrawCString("5   2");
        break;
      case 6:
        MyrDrawCString("3   1");
        break;
      case 7:
        MyrDrawCString("7   2");
        break;
      case 8:
        MyrDrawCString("4   1");
        break;
      case 9:
        MyrDrawCString("9   2");
        break;
      case 10:
        MyrDrawCString("5   1");
        break;
      case 11:
        MoveTo(596 + leftshift - (leftshift * forcesmallshift), ydist);
        MyrDrawCString("11  2");
        break;
      case 12:
        MyrDrawCString("6   1");
        break;
      case 13:
        MyrDrawCString("13  2");
        break;
      case 14:
        MyrDrawCString("7   1");
        break;
      case 15:
        MyrDrawCString("15  2");
        break;
      case 16:
        MyrDrawCString("8   1");
        break;
      case 17:
        MyrDrawCString("17  2");
        break;
      case 18:
        MyrDrawCString("9   1");
        break;
      case 19:
        MyrDrawCString("19  2");
        break;
      default:
        MyrDrawCString(">  10");
        break;
    }
  }

  if (incombat)
    updatelight(who, 0);

  SetPort(oldport);
}
