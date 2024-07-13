#include "prototypes.h"
#include "variables.h"

/***************************** beast ********************************/
void beast(short who, short mode, short specific) {
  FILE* fp;
  short index, t;
  Str255 des;
  Boolean winjump = 0;
  GrafPtr oldport;

  GetPort(&oldport);
  fp = NULL;

  SetCCursor(sword);
  if (mode)
    winjump++;

  for (t = 0; t <= numchannel; t++)
    quiet(t);
  if (!reducesound)
    sound(133);

  compactheap();

  count = CountMItems(gBeast);

  needdungeonupdate = TRUE;

  in();

  gGeneration = GetNewDialog(163 - 5 * winjump, 0L, (WindowPtr)-1L);
  gCurrent = gGeneration;
  SetPortDialogPort(gGeneration);
  BackPixPat(base);
  TextFont(defaultfont);
  ForeColor(yellowColor);

  if (winjump) {
    MoveWindow(GetDialogWindow(gGeneration), GlobalLeft + 142 + leftshift, GlobalTop + 40 + downshift, FALSE);
    ShowWindow(GetDialogWindow(gGeneration));
  } else {
    MoveWindow(GetDialogWindow(gGeneration), GlobalLeft + 80 + leftshift, GlobalTop + 333 + downshift, FALSE);
    ShowWindow(GetDialogWindow(gGeneration));
  }

  ErasePortRect();
  MyrCDiStr(42, (StringPtr) "");

  if (mode == 2)
    pict(154, itemRect);
  else
    pict(142, itemRect);

  if (mode == 1) {
    getfilename("Data MD");

    if (monsterset) {
      MyrNumToString(monsterset, myString);
      // PtoCstr(myString);
      strcat((StringPtr)filename, myString);
    }

    if ((fp = MyrFopen(filename, "rb")) == NULL) {

      if (monsterset) {
        monsterset = 0;
        warn(102);
        goto out;
      } else
        scratch(4);
    }
    if (specific)
      fseek(fp, specific * sizeof monpick, SEEK_SET);
    else
      fseek(fp, (menupos[who] - 1) * sizeof monpick, SEEK_SET);
    /* !MYRIAD 12/10/99 Because fseek can be greater than the end of the file (and fread can fail)*/
    if (fread(&monpick, sizeof monpick, 1, fp) == 1)
      CvtMonsterToPc(&monpick);
    /* !MYRIAD 12/10/99 If not read, keeps the previous value of monpick */
    fclose(fp);
    index = who;
  } else
    monpick = monster[who - 10];

  if (mode) {
    for (t = 29; t < 42; t++)
      MyrCDiStr(t, (StringPtr) "");
    for (t = 44; t < 49; t++)
      MyrCDiStr(t, (StringPtr) "");
  }

backup:

  strcpy((StringPtr)gotword, (StringPtr)monpick.monname);
  CtoPstr(gotword);

  MyrPascalDiStr(7, (StringPtr)gotword);

  /*** if (mode == 1)
  {
   getfilename("Data DES");
   if ((fp = MyrFopen(filename,"rb")) == NULL) scratch(5);
   if (specific) fseek(fp,specific * sizeof des,SEEK_SET);
    else fseek(fp,(menupos[index] -1) * sizeof des,SEEK_SET);
   fread(&des,sizeof des,1,fp);
   fclose(fp);
   MyrStrPtoC((Ptr)des);
   MyrCDiStr(28,des);
  }******/

  if (mode == 1) {
    getfilename("Data DES");
    if ((fp = MyrFopen(filename, "rb")) == NULL)
      scratch(5);
    if (specific)
      fseek(fp, specific * 256, SEEK_SET);
    else
      fseek(fp, (menupos[index] - 1) * 256, SEEK_SET);
    fread(&des, 256, 1, fp);
    fclose(fp);
    GetDialogItem(gCurrent, 28, &itemType, &itemHandle, &itemRect);
    SetDialogItemText(itemHandle, (StringPtr)des);
  }

  MyrCDiStr(6, (StringPtr) "");
  if (monpick.size < 2)
    InsetRect(&itemRect, 16, 0);
  if ((monpick.size == 0) || (monpick.size == 2))
    InsetRect(&itemRect, 0, 16);

  iconhand = NIL;
  iconhand = GetCIcon(monpick.iconid);
  if (iconhand) {
    PlotCIcon(&itemRect, iconhand);
    DisposeCIcon(iconhand);
  }

  if (monpick.weapon > 0) {
    loaditem(monpick.weapon);
    for (t = 11; t < 23; t++)
      MyrCDiStr(t, (StringPtr) "");
    MyrCDiStr(8, (StringPtr) "Weapon");
    MyrCDiStr(9, (StringPtr) "");
    MyrCDiStr(10, (StringPtr) "");
    DialogNum(18, item.vssmall + monpick.damplus);
    DialogNum(13, monpick.damplus + 1);
  } else if (monpick.weapon) {
    MyrCDiStr(8, (StringPtr) "    By");
    MyrCDiStr(9, (StringPtr) "Weapon");
    MyrCDiStr(10, (StringPtr) "   Type");
    for (t = 11; t < 23; t++)
      MyrCDiStr(t, (StringPtr) "");
  } else {
    for (t = 0; t < 5; t++) {
      if (monpick.attacks[t][0]) {
        DialogNum(t + 13, monpick.attacks[t][0]);
        DialogNum(t + 18, monpick.attacks[t][1]);
        GetIndString(myString, 128, monpick.attacks[t][2] - 31);
        MyrPascalDiStr(t + 8, myString);
      } else {
        MyrCDiStr(t + 13, (StringPtr) "");
        MyrCDiStr(t + 18, (StringPtr) "");
        MyrCDiStr(t + 8, (StringPtr) "");
      }
    }
  }

  MyrCDiStr(27, (StringPtr) "No Special Attacks");

  for (t = 0; t < 5; t++) {
    if (monpick.attacks[t][3]) {
      GetIndString(myString, 137, monpick.attacks[t][3] + 1);
      MyrPascalDiStr(27, myString);
    }
  }

  DialogNum(26, monpick.magres);
  DialogNum(43, monpick.hd);
  DialogNum(25, monpick.ac);
  DialogNum(24, monpick.movementmax);
  DialogNum(23, monpick.noofmagattacks);

  if (mode) {
    GetDialogItem(gGeneration, 29, &itemType, &itemHandle, &itemRect);
    for (t = 0; t < 6; t++) {
      if (monpick.type[t])
        DrawPicture(on, &itemRect);
      else
        DrawPicture(non, &itemRect);

      OffsetRect(&itemRect, 0, 17);
    }

    GetDialogItem(gGeneration, 36, &itemType, &itemHandle, &itemRect);

    for (t = 0; t < 6; t++) {
      if (monpick.spellimmune[t])
        DrawPicture(on, &itemRect);
      else
        DrawPicture(non, &itemRect);

      OffsetRect(&itemRect, 0, 17);
    }

    GetDialogItem(gGeneration, 44, &itemType, &itemHandle, &itemRect);

    for (t = 0; t < 5; t++) {
      if (monpick.save[t] < 0)
        DrawPicture(on, &itemRect);
      else
        DrawPicture(non, &itemRect);

      OffsetRect(&itemRect, 0, 17);
    }
  }

  for (;;) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(gGeneration, itemHit, &itemType, &itemHandle, &buttonrect);
    if (mode != 1)
      goto out;
    if ((itemHit == 1) || (itemHit == 4))
      goto out;
    if ((itemHit == 2) || (itemHit == 3)) {
      sound(141);
      ploticon3(135, buttonrect);
      if (itemHit == 3)
        index += 2;
      index--;

      if (index < 1)
        index = 1;
      if (index > count)
        index = count;

      getfilename("Data MD");

      if (monsterset) //*** fantasoft v7.1  Begin
      {
        MyrNumToString(monsterset, myString);
        strcat((StringPtr)filename, myString);
      } //*** fantasoft v7.1 END

      if ((fp = MyrFopen(filename, "rb")) == NULL) {
        monsterset = 0;
        warn(102);
        goto out;
      }
      if (specific)
        fseek(fp, specific * sizeof monpick, SEEK_SET);
      else
        fseek(fp, (menupos[index] - 1) * sizeof monpick, SEEK_SET);

      /* !MYRIAD 12/10/99 Because fseek can be greater than the end of the file (and fread can fail)*/
      if (fread(&monpick, sizeof monpick, 1, fp) == 1)
        CvtMonsterToPc(&monpick);
      /* !MYRIAD 12/10/99 If not read, keeps the previous value of monpick */
      fclose(fp);

      ploticon3(136, buttonrect);
      goto backup;
    }
  }
out:
  GetDialogItem(gGeneration, 4, &itemType, &itemHandle, &buttonrect);
  ploticon3(129, buttonrect);
  if (!reducesound)
    sound(133);
  SetPort(oldport);
  DisposeDialog(gGeneration);
  out();
}
