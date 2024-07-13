#include "prototypes.h"
#include "variables.h"

#include "files.h"

/*************************** partyselect *************************/
void partyselect(short mode) {
  short filepick, oldcharpick, newcharpick, t, index, level, totallevel;
  float total, required;
  Boolean danger = FALSE;
  Boolean hasrestrictions = FALSE;
  FILE* fp = NULL;
  char name[30];
  // SFTypeList types;
  Point where;

  forcesmall = TRUE;

  // strcpy(types,(StringPtr)"APPL");//*** not used but there to avoid the compile error ***/

  SetCCursor(sword);

  oldcharpick = newcharpick = index = filepick = totallevel = required = 0;
  needdungeonupdate = TRUE;

  party = GetNewDialog(145, 0L, (WindowPtr)-1L);
  if (!reducesound)
    sound(10004);
  SetPortDialogPort(party);
  ForeColor(yellowColor);
  MoveWindow(GetDialogWindow(party), GlobalLeft + (leftshift / 2), GlobalTop + (downshift / 2), FALSE);
  ShowWindow(GetDialogWindow(party));

  if ((fp = MyrFopen(":Character Files:Data CD", "rb")) == NULL) {
    if ((fp = MyrFopen(":Character Files:Data CD", "w+b")) == NULL)
      scratch2(10);
    fclose(fp);
    setfileinfo((Ptr) "CDat", (Ptr) ":Character Files:Data CD");
    flashmessage((StringPtr) "Your 'Data CD' file is damaged or missing. You need to IMPORT your characters.", 15, 60, 0, 6000);
    flashmessage((StringPtr) "To IMPORT your characters just click 'IMPORT' and locate your characters files.", 15, 60, 0, 6000);
  } else
    fclose(fp);
  GetMenuItemText(gGame, currentscenario, myString);
  PtoCstr(myString);
  getfilename((Ptr)myString);
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(130);
  fread(&reclevel, sizeof(long), 1, fp);
  CvtLongToPc(&reclevel);
  fread(&maxlevel, sizeof(long), 1, fp);
  CvtLongToPc(&maxlevel);
  fclose(fp);

  if (doreg())
    maxlevel = 999;

  charselectnew = 0;

  for (t = 0; t <= charnum; t++)
    c[t].inbattle = TRUE;

bigupdate:

  SetPortDialogPort(party);
  BackPixPat(base);
  TextFont(font);
  ForeColor(yellowColor);
  TextSize(17);
  TextFace(bold);
  gCurrent = party;
  ErasePortRect();
  DrawDialog(party);

  for (t = 85; t < 88; t++) {
    GetDialogItem(party, t, &itemType, &itemHandle, &buttonrect);
    downbutton(FALSE);
  }
  GetDialogItem(party, 67, &itemType, &itemHandle, &buttonrect);
  downbutton(FALSE);
  GetDialogItem(party, 69, &itemType, &itemHandle, &buttonrect);
  downbutton(FALSE);
  GetDialogItem(party, 60, &itemType, &itemHandle, &buttonrect);
  downbutton(FALSE);

  for (t = 80; t < 85; t++) {
    GetDialogItem(party, t, &itemType, &itemHandle, &itemRect);
    SetDialogItemText(itemHandle, (StringPtr) "");
    pict(179, itemRect);
  }

  for (t = 54; t < 57; t++) {
    GetDialogItem(party, t, &itemType, &itemHandle, &itemRect);
    SetDialogItemText(itemHandle, (StringPtr) "");
    pict(179, itemRect);
  }
update:

  SetPortDialogPort(party);
  ForeColor(yellowColor);
  RGBBackColor(&greycolor);

  if ((mode) || (charnum > 1))
    MyrCDiStr(89, (StringPtr) "");

  shortupdate(1);

shortupdate:

  gCurrent = party;
  SetPortDialogPort(party);
  TextMode(1);
  TextFont(font);

  GetDialogItem(party, 86, &itemType, &itemHandle, &buttonrect);
  downbutton(FALSE);
  GetDialogItem(party, 87, &itemType, &itemHandle, &buttonrect);
  downbutton(FALSE);

  ForeColor(yellowColor);

  MyrCDiStr(35, (StringPtr) "");
  pict(160, itemRect);
  if (charnum > 4) {
    ploticon3(2019, itemRect);
    charnum = 5;
  }

  GetDialogItem(party, 82 + howhard, &itemType, &itemHandle, &itemRect);
  InsetRect(&itemRect, 2, 2);
  DrawPicture(on, &itemRect);

  GetDialogItem(party, 55 + monsterset, &itemType, &itemHandle, &itemRect);
  InsetRect(&itemRect, 2, 2);
  DrawPicture(on, &itemRect);

  GetDialogItem(party, newcharpick + 28, &itemType, &itemHandle, &itemRect);
  DrawPicture(marker, &itemRect);

  totallevel = 0;
  for (t = 0; t <= charnum; t++) {
    DialogNum(61 + t, c[t].level);
    totallevel += c[t].level;
    minus(c[t].name, 1);
  }

  DialogNum(75, totallevel);
  DialogNumLong(74, reclevel);

  if (maxlevel == 999)
    MyrCDiStr(77, (StringPtr) "None");
  else
    DialogNumLong(77, maxlevel);

  total = totallevel;
  required = reclevel;

  hardpercent = 1 + (howhard * .33);

  if (total) {
    percent = (hardpercent * (required / total)) * 100;

    if (percent < 20)
      percent = 20;
    if (percent > 250)
      percent = 250;

    temp = percent;
    DialogNum(72, temp);
  } else
    MyrCDiStr(72, (StringPtr) "");

  percent /= 100;

  if ((fp = MyrFopen(":Character Files:Data CD", "rb")) == NULL)
    scratch(131);
  fseek(fp, index * 32, SEEK_SET);
  for (t = 2; t < 14; t++) {
    if (t == filepick)
      ForeColor(yellowColor);
    else
      ForeColor(cyanColor);
    fread(&name, 30, 1, fp);
    fread(&level, sizeof level, 1, fp);
    CvtShortToPc(&level);
    CtoPstr(name);
    if (!feof(fp)) {
      MyrPascalDiStr(t, (StringPtr)name);
      if (StringWidth((StringPtr)name))
        DialogNum(t + 40, level);
      else
        MyrCDiStr(t + 40, (StringPtr) "");
    } else {
      MyrCDiStr(t, (StringPtr) "");
      MyrCDiStr(t + 40, (StringPtr) "");
    }
  }
  fclose(fp);

  ForeColor(yellowColor);

  getfilename("Data RI");
  if ((fp = MyrFopen(filename, "rb")) != NULL) {
    fread(&restrictinfo, sizeof restrictinfo, 1, fp);
    CvtRestrictionInfoToPc(&restrictinfo);
    fclose(fp);
    hasrestrictions = TRUE;
    GetDialogItem(party, 21, &itemType, &itemHandle, &itemRect);
    pict(216, itemRect);
  }

  for (;;) {
  over:
    gCurrent = party;
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(party, itemHit, &itemType, &itemHandle, &buttonrect);

    if ((itemHit == 21) && (hasrestrictions)) {
      viewrestrictions();
      goto bigupdate;
    }

    if ((itemHit > 79) && (itemHit < 85)) /**** difficulty level *****/
    {

      if ((itemHit > 83) && (!doreg())) {
        SetPortDialogPort(party);
        ForeColor(blackColor);
        BackColor(whiteColor);
        warn(103);
        goto update;
      }

      sound(144);
      GetDialogItem(party, 82 + howhard, &itemType, &itemHandle, &itemRect);
      InsetRect(&itemRect, 2, 2);
      DrawPicture(non, &itemRect);

      howhard = itemHit - 82;
      goto shortupdate;
    }

    if ((itemHit > 53) && (itemHit < 57)) /***** monster set ******/
    {
      if ((itemHit == 56) || (itemHit == 54)) {
        if (!doreg()) {
          SetPortDialogPort(party);
          ForeColor(blackColor);
          BackColor(whiteColor);
          warn(103);
          goto update;
        }
      }

      sound(144);
      GetDialogItem(party, 55 + monsterset, &itemType, &itemHandle, &itemRect);
      InsetRect(&itemRect, 2, 2);
      DrawPicture(non, &itemRect);

      monsterset = itemHit - 55;

      GetDialogItem(party, 55 + monsterset, &itemType, &itemHandle, &itemRect);
      InsetRect(&itemRect, 2, 2);
      DrawPicture(on, &itemRect);

      savepref();
    }

    if ((itemHit > 37) && (itemHit < 40)) {
      ploticon3(129, buttonrect);
      sound(141);
      index -= 5;
      if (itemHit == 39)
        index += 10;
      if (index < 0)
        index = 0;
      ploticon3(130, buttonrect);
      goto update;
    }

    if ((itemHit == 37) && (filepick)) /****** erase char from disk ******/
    {
      ploticon3(129, buttonrect);
      GetDialogItem(party, filepick, &itemType, &itemHandle, &itemRect);
      GetDialogItemText(itemHandle, myString);
      ploticon3(130, buttonrect);
      if (!StringWidth(myString))
        goto update;

      if (question(3)) {
        GetDialogItem(party, filepick, &itemType, &itemHandle, &itemRect);
        GetDialogItemText(itemHandle, myString);
        PtoCstr(myString);
        minus((Ptr)myString, 0); /****** erase char from disk ******/
        SetPortDialogPort(party);
      }
      filepick = 0;
      goto update;
    }

    if (itemHit == 88) /****** import ******/
    {
/* File loading not ready under carbon yet */
#ifdef CARBON
    /* UGH, GOTOs */
    charerror:
      SetPortDialogPort(party);
      ForeColor(blackColor);
      BackColor(whiteColor);
      warn(38);
      goto bigupdate;

#else

      SFReply SFReplyRecord;
      SFTypeList types = {'APPL', 0, 0, 0};
      char** file_types = {NULL};
      // char *filename = dialog_open_file(file_types, "Please select the character file to import:");
      SFGetFile(where, myString, NIL, -1, types, NIL, &SFReplyRecord);

      if (!SFReplyRecord.good)
        goto over;
      if (filename == NULL)
        goto over;

      GetFInfo(SFReplyRecord.fName, SFReplyRecord.vRefNum, &fileinfo);

      ploticon3(129, buttonrect);

      PtoCstr((StringPtr)gotword);
      PtoCstr(SFReplyRecord.fName);
      strcpy((StringPtr)gotword, SFReplyRecord.fName);

      strcpy(filename, ":Character Files:");
      strncat(filename, gotword, 30);

      ploticon3(130, buttonrect);

      minus((Ptr)SFReplyRecord.fName, 1);

      if ((fp = MyrFopen(filename, "rb")) == NULL) {
      charerror:
        SetPortDialogPort(party);
        ForeColor(blackColor);
        BackColor(whiteColor);
        warn(38);
        goto bigupdate;
      } else {
        fread(&characterl, sizeof characterl, 1, fp);
        CvtCharacterToPc(&characterl);
        fclose(fp);

        if (!checkforerrors())
          plus(characterl.name, characterl.level);
      }
      goto bigupdate;
#endif /* !CARBON */
    }

    if (itemHit == 34) /*********** done button *************/
    {
      ploticon3(129, buttonrect);
      if (charnum > -1)
        sound(26260);
      for (t = 0; t <= charnum; t++)
        plus(c[t].name, c[t].level);

      for (tt = 0; tt <= charnum; tt++) {
        for (t = 0; t < 10; t++) {
          definespells[tt][t][0] = c[tt].definespells[t][0];
          definespells[tt][t][1] = c[tt].definespells[t][1];
          definespells[tt][t][2] = c[tt].definespells[t][2];
          definespells[tt][t][3] = c[tt].definespells[t][3];
        }
      }

      bandaid();
      DisposeDialog(party);
      forcesmall = FALSE;
      return;
    }

    if (itemHit < 14) {
      sound(145);
      GetDialogItem(party, itemHit, &itemType, &itemHandle, &itemRect);
      GetDialogItemText(itemHandle, myString);
      PtoCstr(myString);
      strcpy(filename, ":Character Files:");
      strncat(filename, myString, 30);

      if ((fp = MyrFopen(filename, "rb")) == NULL) {
        SetPortDialogPort(party);
        ForeColor(blackColor);
        BackColor(whiteColor);
        warn(38);
        warn(143);
        GetDialogItem(party, filepick, &itemType, &itemHandle, &itemRect);
        GetDialogItemText(itemHandle, myString);
        PtoCstr(myString);
        minus((Ptr)myString, 1);
        goto bigupdate;
      }

      fread(&characterl, sizeof characterl, 1, fp);
      CvtCharacterToPc(&characterl);
      fclose(fp);

      if (itemHit == filepick) {
        fp = MyrFopen(filename, "rb");
        characterr = c[0];
        fread(&c[0], sizeof c[0], 1, fp);
        CvtCharacterToPc(&c[0]);

        fclose(fp);
        viewcharacter(0, 1);
        filepick = 0;
        c[0] = characterr;
        goto bigupdate;
      }

      if (!filepick)
        filepick = itemHit;
      ForeColor(cyanColor);
      GetDialogItem(party, filepick, &itemType, &itemHandle, &itemRect);
      GetDialogItemText(itemHandle, myString);
      MyrPascalDiStr(filepick, myString);
      GetDialogItem(party, filepick + 40, &itemType, &itemHandle, &itemRect);
      GetDialogItemText(itemHandle, myString);
      MyrPascalDiStr(filepick + 40, myString);

      ForeColor(yellowColor);
      GetDialogItem(party, itemHit + 40, &itemType, &itemHandle, &itemRect);
      GetDialogItemText(itemHandle, myString);
      MyrPascalDiStr(itemHit + 40, myString);
      GetDialogItem(party, itemHit, &itemType, &itemHandle, &itemRect);
      GetDialogItemText(itemHandle, myString);
      MyrPascalDiStr(itemHit, myString);

      ForeColor(blackColor);
      PtoCstr(myString);
      strcpy(name, myString);
      filepick = itemHit;

      temp = characterl.level;
      SetCCursor(sword);
      if ((temp > restrictinfo.maxlevel) && (restrictinfo.maxlevel)) /******** too high a level character *********/
      {
        SetCCursor(stop);
      }

      if ((charnum + 1 >= restrictinfo.maxpc) && (restrictinfo.maxpc)) /*************** too many characters *******/
      {
        SetCCursor(stop);
      }

      if (restrictinfo.canrace[characterl.race - 1]) /*************** banned race *******/
      {
        SetCCursor(stop);
      }

      if (restrictinfo.cancaste[characterl.caste - 1]) /*************** banned caste *******/
      {
        SetCCursor(stop);
      }
    }

    if ((itemHit > 21) && (itemHit < 23 + charnum)) {
      ploticon3(129, buttonrect);
      sound(141);
      newcharpick = charselectnew = itemHit - 22;
      GetDialogItem(party, oldcharpick + 28, &itemType, &itemHandle, &itemRect);
      DrawPicture(grey, &itemRect);
      GetDialogItem(party, newcharpick + 28, &itemType, &itemHandle, &itemRect);
      DrawPicture(marker, &itemRect);
      if (newcharpick == oldcharpick) {
        ForeColor(yellowColor);
        viewcharacter(newcharpick, 0);
        filepick = 0;
        goto bigupdate;
      }
      oldcharpick = newcharpick;
      ploticon3(130, buttonrect);
    }

    if ((itemHit == 36) && (charnum > -1)) /********** drop from party ****/
    {
      charselectnew = 0;
      if ((mode) && (!charnum)) {
        SetPortDialogPort(party);
        ForeColor(blackColor);
        BackColor(whiteColor);
        warn(67);
        SetPortDialogPort(party);
        ForeColor(yellowColor);
        RGBBackColor(&greycolor);
      } else {
        danger = FALSE;
        for (t = 0; t < c[newcharpick].numitems; t++) {
          loaditem(c[newcharpick].items[t].id);
          if ((item.type == 25) || (item.type == 23) || (item.type < 0))
            danger = TRUE;
        }

        if (danger) {
          SetPortDialogPort(party);
          ForeColor(blackColor);
          BackColor(whiteColor);
          warn(100);
          SetPortDialogPort(party);
          ForeColor(yellowColor);
          RGBBackColor(&greycolor);

          gCurrent = party;
          if (question(13))
            danger = FALSE;
          else
            goto bigupdate;

          GetDialogItem(party, 13, &itemType, &itemHandle, &buttonrect);
        }

        if (!danger) {
          ploticon3(129, buttonrect);
          sound(663);
          plus(c[newcharpick].name, c[newcharpick].level);
          ForeColor(yellowColor);
          savecharacter(newcharpick);

          MyrCDiStr(61 + charnum, (StringPtr) "");

          if ((c[newcharpick].stamina < 1) || (c[newcharpick].condition[25]))
            killparty--;

#if CHECK_ILLEGAL_ACCESS > 0
          if (newcharpick < 0 || newcharpick >= 6 || charnum < 0 || charnum >= 6)
            AcamErreur("party select bad index");
#endif
          for (t = newcharpick; t < charnum; t++)
            c[t] = c[t + 1];

          GetDialogItem(party, newcharpick + 22, &itemType, &itemHandle, &itemRect);

          itemRect.top = newcharpick * 50;
          itemRect.bottom = 300;
          itemRect.left -= 10;
          itemRect.right = 640;

          SetPortDialogPort(party);
          ScrollRect(&itemRect, 0, -50, 0L);
          delay(10); // Fantasoft 7.1  Slows it down a bit on fast machines.

          itemRect.left = 319;
          itemRect.right = 640;
          itemRect.top = 250;
          itemRect.bottom = 300;
          EraseRect(&itemRect);

          charnum--;
          oldcharpick = filepick = 0;
          if (newcharpick > charnum) {
            newcharpick--;
            if (newcharpick < 0)
              newcharpick++;
          }
          ploticon3(130, buttonrect);
          goto bigupdate;
        }
      }
    }

    if ((itemHit == 35) && (charnum < 5) && (filepick)) /**** add to party ****/
    {
      temp = GetDialogNum(filepick + 40);

      if ((total + temp > maxlevel) && (!mode)) {
        ForeColor(blackColor);
        BackColor(whiteColor);
        warn(64);
        goto update;
      }

      if (hasrestrictions) {
        GetDialogItem(party, filepick, &itemType, &itemHandle, &itemRect); /******** load in for restiction testing ********/
        GetDialogItemText(itemHandle, myString);
        if (StringWidth(myString)) {
          PtoCstr(myString);
          strcpy(filename, ":Character Files:");
          strcat((StringPtr)filename, myString);
          if ((fp = MyrFopen(filename, "rb")) != NULL) {
            fread(&characterl, sizeof characterl, 1, fp);
            CvtCharacterToPc(&characterl);
            fclose(fp);
          } else
            goto charerror;
        }

        if ((temp > restrictinfo.maxlevel) && (restrictinfo.maxlevel)) /******** too high a level character *********/
        {
          ForeColor(blackColor);
          BackColor(whiteColor);
          warn(134);
          viewrestrictions();
          goto bigupdate;
        }

        if ((charnum + 1 >= restrictinfo.maxpc) && (restrictinfo.maxpc)) /*************** too many characters *******/
        {
          ForeColor(blackColor);
          BackColor(whiteColor);
          warn(135);
          viewrestrictions();
          goto bigupdate;
        }

        if (restrictinfo.canrace[characterl.race - 1]) /*************** banned race *******/
        {
          ForeColor(blackColor);
          BackColor(whiteColor);
          warn(136);
          viewrestrictions();
          goto bigupdate;
        }

        if (restrictinfo.cancaste[characterl.caste - 1]) /*************** banned caste *******/
        {
          ForeColor(blackColor);
          BackColor(whiteColor);
          warn(137);
          viewrestrictions();
          goto bigupdate;
        }
      }

      ploticon3(129, buttonrect);
      GetDialogItem(party, filepick, &itemType, &itemHandle, &itemRect);
      GetDialogItemText(itemHandle, myString);

      if (StringWidth(myString)) {
        PtoCstr(myString);
        sound(662);
        strcpy(filename, ":Character Files:");
        strcat((StringPtr)filename, myString);
        charnum++;
        if ((fp = MyrFopen(filename, "rb")) == NULL) {
          charnum--;
          ForeColor(blackColor);
          BackColor(whiteColor);
          warn(38);
          GetDialogItem(party, filepick, &itemType, &itemHandle, &itemRect);
          GetDialogItemText(itemHandle, myString);
          PtoCstr(myString);
          minus((Ptr)myString, 1);
          goto update;
        }
        fread(&characterl, sizeof characterl, 1, fp);
        CvtCharacterToPc(&characterl);
        fclose(fp);

        checkforerrors(); /**** clean up name and spells data from old version ***/

        if (characterl.version > -3) {
          charnum--;
          ForeColor(blackColor);
          BackColor(whiteColor);
          warn(73);
          goto update;
        }

        if ((characterl.level > 3) && (!doreg())) {
          charnum--;
          ForeColor(blackColor);
          BackColor(whiteColor);
          warn(76);
          goto update;
        }

        c[charnum] = characterl;

        minus(c[charnum].name, 1);
        filepick = 0;
        GetDialogItem(party, 22 + charnum, &itemType, &itemHandle, &itemRect);
        itemRect.left -= 10;
        pict(1, itemRect);
        updatechar(charnum, 5);
        ploticon3(130, buttonrect);

        if ((c[charnum].stamina < 1) || (c[charnum].condition[25]))
          killparty++;
        goto shortupdate;
      }
      ploticon3(130, buttonrect);
    }
  }
}

/********************************* viewrestrictions ***********************/
void viewrestrictions(void) {
  DialogRef restrict;

  restrict = GetNewDialog(-7931, 0L, (WindowPtr)-1L);
  SetPortDialogPort(restrict);
  gCurrent = restrict;
  showzero = 0;

  DrawDialog(restrict);
  DrawControls(GetDialogWindow(restrict));

  MyrPascalDiStr(4, restrictinfo.description);
  DialogNum(65, restrictinfo.maxpc);
  DialogNum(66, restrictinfo.maxlevel);

  for (t = 0; t < 30; t++) {
    GetDialogItem(gCurrent, t + 5, &itemType, &itemHandle, &itemRect);
    SetControlValue((ControlHandle)itemHandle, restrictinfo.canrace[t]);

    GetDialogItem(gCurrent, t + 35, &itemType, &itemHandle, &itemRect);
    SetControlValue((ControlHandle)itemHandle, restrictinfo.cancaste[t]);

    GetIndString(myString, 129, t + 1); /******** show races ********/
    MyrPascalDiStr(t + 67, myString);

    GetIndString(myString, 131, t + 1); /******** show castes ********/
    MyrPascalDiStr(t + 97, myString);
  }

  BeginUpdate(GetDialogWindow(restrict));
  EndUpdate(GetDialogWindow(restrict));

  FlushEvents(everyEvent, 0);
  ModalDialog(0L, &itemHit);

  DisposeDialog(restrict);
}
