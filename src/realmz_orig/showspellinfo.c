#include "prototypes.h"
#include "variables.h"

/***************** showspellinfo ********************/
void showspellinfo(void) {
  short tempint;

  spellrect2 = spellrect;
  tempint = 10000;
  switch (spellinfo.targettype) {
    case 5:
      if (spellinfo.size)
        tempint = 9999 + spellinfo.size;
      else
        tempint = 10215;
      break;

    case 9:
      tempint = 10211;
      break;

    case 10:
      tempint = 10210;
      break;

    case 11:
      tempint = 10212;
      break;

    case 6:
      tempint = 10213;
      break;

    case 4:
      tempint = 9999 + powerlevel;
      break;

    case 3:
      tempint = 9999 + spellinfo.size;
      break;

    case 7:
      tempint = 10214;
      break;
  }
  if (spellinfo.special == 58)
    tempint = 10013 + spellinfo.size;

  SetPort(GetWindowPort(screen));

  BackColor(whiteColor);
  iconhand = NIL;
  iconhand = GetCIcon(tempint);
  if (iconhand) {
    PlotCIcon(&spellrect, iconhand);
    // CopyBits(&(**(iconhand)).iconBMap,&((GrafPtr) gmaps)->portBits,&(**(iconhand)).iconPMap.bounds,&bigspellrect,0,0L);
    MyrCopyIconMask(iconhand, gmaps, &bigspellrect); // Myriad
    DisposeCIcon(iconhand);
  }

  if (spellinfo.range1 + spellinfo.range2 > 0) {
    /***** need to see ****/
    itemRect = spellrect;
    itemRect.bottom -= 32;
    itemRect.right -= 32;
    OffsetRect(&itemRect, 0, -5);
    iconhand = GetCIcon(14022);
    PlotCIcon(&itemRect, iconhand);
    DisposeCIcon(iconhand);
  }

  ForeColor(yellowColor);
  RGBBackColor(&greycolor);
  TextSize(14);
  MoveTo(369 + leftshift, 402 + downshift);

  itemused = 0;
  GetIndString(myString, 1000 * (castcaste + 1) + castlevel, castnum + 1);
  PtoCstr(myString);
  MyrDrawCString((Ptr)myString);
  TextMode(1);
  TextSize(16);
  ForeColor(redColor);
  MoveTo(548 + leftshift, 385 + downshift);
  string(powerlevel);
}

/**************** spellinfoupdate ************************/
void spellinfoupdate(void) {
  short tempint;

  ForeColor(yellowColor);
  DialogNum(13, spellinfo.powerdam1 * powerlevel + spellinfo.damage1);
  DialogNum(50, spellinfo.powerdam2 * powerlevel + spellinfo.damage2);

  DialogNum(14, abs(spellinfo.powerdur1 * powerlevel + spellinfo.duration1));
  DialogNum(15, abs(spellinfo.powerdur2 * powerlevel + spellinfo.duration2));

  DialogNum(18, abs(spellinfo.range1 + spellinfo.range2 * powerlevel));

  tempint = 1;
  if (spellinfo.targettype < 1)
    tempint = powerlevel;

  DialogNum(19, tempint);

  if (spellinfo.damagetype < 1) {
    MyrCDiStr(52, (StringPtr) "Vs");
    MyrCDiStr(53, (StringPtr) "");
  } else {
    if ((spellinfo.cannot == 1) || (spellinfo.cannot > 2))
      MyrCDiStr(52, (StringPtr) "No");
    else if (!spellinfo.resistadjust)
      MyrCDiStr(52, (StringPtr) "Yes");
    else
      DialogNum(52, powerlevel * spellinfo.resistadjust);
  }

  if (spellinfo.cannot > 1)
    MyrCDiStr(53, (StringPtr) "No");
  else if ((!spellinfo.saveadjust) && (!spellinfo.savebonus))
    MyrCDiStr(53, (StringPtr) "Yes");
  else
    DialogNum(53, spellinfo.savebonus + powerlevel * spellinfo.saveadjust);

  RGBForeColor(&cyancolor);
  TextSize(26);
  TextFont(font);
  DialogNum(54, powerlevel);
  TextFont(defaultfont);
  TextSize(12);
  ForeColor(yellowColor);

  tempint = 10000;
  switch (spellinfo.targettype) {
    case 5:
      if (spellinfo.size)
        tempint = 9999 + spellinfo.size;
      else
        tempint = 10215;
      break;

    case 9:
      tempint = 10211;
      break;

    case 10:
      tempint = 10210;
      break;

    case 11:
      tempint = 10212;
      break;

    case 6:
      tempint = 10213;
      break;

    case 4:
      tempint = 9999 + powerlevel;
      break;

    case 3:
      tempint = 9999 + spellinfo.size;
      break;

    case 7:
      tempint = 10214;
      break;
  }
  GetDialogItem(gCurrent, 16, &itemType, &itemHandle, &itemRect);
  ploticon3(tempint, itemRect);

  if ((incombat) && (spellinfo.range1 + spellinfo.range2 > 0)) { /***** need to see ****/
    itemRect.bottom -= 32;
    itemRect.right -= 32;
    OffsetRect(&itemRect, 0, -5);
    iconhand = GetCIcon(14022);
    PlotCIcon(&itemRect, iconhand);
    DisposeCIcon(iconhand);
  }
}
