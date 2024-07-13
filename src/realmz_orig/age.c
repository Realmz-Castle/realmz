#include "prototypes.h"
#include "variables.h"

/******************** age ***********************/
short age(long age, short raceid, short currentagegroup) {
  short nextyear, t;
  loadprofile(raceid, 0);

  nextyear = age / 365;

  for (t = 0; t < 5; t++) {
    if (twixt(nextyear, races.agerange[t][0], races.agerange[t][1])) {
      if ((t + 1) != (currentagegroup)) {
        if (((t + 1) > (currentagegroup)) && (currentagegroup < 5)) {
          characterl.currentagegroup++;
          applyage(raceid, characterl.currentagegroup, 1);
          return (1);
        }
        if (((t + 1) < (currentagegroup)) && (currentagegroup > 1)) {
          applyage(raceid, characterl.currentagegroup, -1); /****** erase current age group change because we are getting younger ****/
          characterl.currentagegroup--;
          return (-1);
        }
      }
    }
  }
  return (NIL);
}

/************************** showageupdate **************************/
void showageupdate(short who, short agegroup, short backup) {
  DialogRef ageupdate;
  short tt;

  ageupdate = GetNewDialog(192, 0L, (WindowPtr)-1L);
  gCurrent = ageupdate;
  SetPortDialogPort(ageupdate);
  BackPixPat(base);
  ForeColor(yellowColor);
  TextFont(defaultfont);

  MoveWindow(GetDialogWindow(ageupdate), GlobalLeft + leftshift / 2, GlobalTop + downshift / 2, FALSE);
  ShowWindow(GetDialogWindow(ageupdate));
  ErasePortRect();
  DrawDialog(ageupdate);

  switch (agegroup) {
    case 1:
      MyrCDiStr(10, (StringPtr) "Youth");
      MyrCDiStr(14, (StringPtr) "Youth");
      break;

    case 2:
      MyrCDiStr(10, (StringPtr) "Young");
      MyrCDiStr(14, (StringPtr) "Young");
      break;

    case 3:
      MyrCDiStr(10, (StringPtr) "Prime");
      MyrCDiStr(14, (StringPtr) "Prime");
      break;

    case 4:
      MyrCDiStr(10, (StringPtr) "Adult");
      MyrCDiStr(14, (StringPtr) "Adult");
      break;

    case 5:
      MyrCDiStr(10, (StringPtr) "Senior");
      MyrCDiStr(14, (StringPtr) "Senior");
      break;
  }

  DialogNum(15, races.agerange[agegroup - 1][0]);
  DialogNum(16, races.agerange[agegroup - 1][1]);

  GetIndString(myString, 129, c[who].race);
  MyrPascalDiStr(32, myString);

  GetDialogItem(ageupdate, 5, &itemType, &itemHandle, &itemRect);
  plotportrait(c[who].pictid, itemRect, c[who].caste, who);

  GetDialogItem(ageupdate, 7, &itemType, &itemHandle, &itemRect);
  ploticon3(c[who].iconid, itemRect);

  TextFace(bold);
  TextFont(font);
  ForeColor(yellowColor);
  BackPixPat(base);
  TextSize(18);

  loadprofile(c[who].race, 0);

  CtoPstr(c[who].name);
  MyrPascalDiStr(13, (StringPtr)c[who].name);
  PtoCstr((StringPtr)c[who].name);

  if (backup)
    for (tt = 0; tt < 15; tt++)
      DialogNumNZ(tt + 17, -(races.agechange[agegroup][tt]));
  else
    for (tt = 0; tt < 15; tt++)
      DialogNumNZ(tt + 17, races.agechange[agegroup - 1][tt]);

  BeginUpdate(GetDialogWindow(ageupdate));
  EndUpdate(GetDialogWindow(ageupdate));

  sound(3002);

  for (;;) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(ageupdate, itemHit, &itemType, &itemHandle, &buttonrect);

    if (itemHit) {
      DisposeDialog(ageupdate);
      return;
    }
  }
}

/******************** applyage ***********************/
void applyage(short raceid, short agegroup, short direction) {
  short t;

  loadprofile(raceid, 0);

  characterl.currentagegroup = agegroup;

  strength(characterl.st);
  characterl.tohit -= temp;
  characterl.damage -= damage;

  characterl.st += direction * races.agechange[agegroup - 1][0];

  strength(characterl.st);
  characterl.tohit += temp;
  characterl.damage += damage;

  characterl.in += direction * races.agechange[agegroup - 1][1];
  characterl.wi += direction * races.agechange[agegroup - 1][2];
  characterl.de += direction * races.agechange[agegroup - 1][3];
  characterl.co += direction * races.agechange[agegroup - 1][4];
  characterl.lu += direction * races.agechange[agegroup - 1][5];

  characterl.magres += direction * races.agechange[agegroup - 1][6];
  characterl.movementmax += direction * races.agechange[agegroup - 1][7];

  if (characterl.movementmax < 2)
    characterl.movementmax = 2;

  for (t = 0; t < 7; t++)
    characterl.save[t] += direction * races.agechange[agegroup - 1][8 + t];
}
