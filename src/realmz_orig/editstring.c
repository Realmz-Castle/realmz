#include "prototypes.h"
#include "variables.h"

/****************************** downbutton2 *********************************/
void downbutton2(void) {
  Handle itemHandle;
  Rect itemRect;
  Point where;

  GetDialogItem(gCurrent, itemHit, &itemType, &itemHandle, &itemRect);

  if (itemType == userItem) {
    BitMap* src = GetPortBitMapForCopyBits(GetQDGlobalsThePort());
    where.v = itemRect.top;
    where.h = itemRect.left;

    LocalToGlobal(&where);

    OffsetRect(&itemRect, where.v, where.h);

    CopyBits(src, src, &itemRect, &itemRect, 4, NIL);

    delay(3);
    CopyBits(src, src, &itemRect, &itemRect, 4, NIL);
  }
}

/********************* makestring ****************/
void makestring(short id) {
  FILE* fp = NULL;
  if (stringindex < 0) {
    getfilename("Data OD");
    if ((fp = MyrFopen(filename, "r+b")) == NULL)
      scratch(10);
    GetDialogItem(gCurrent, id, &itemType, &itemHandle, &itemRect);
    GetIndString(myString, 3, 1);
    GetDialogItemText(itemHandle, myString);
    fseek(fp, abs(stringindex) * 25, SEEK_SET);
    fwrite(myString, 25, 1, fp);
    fclose(fp);
  } else {
    getfilename("Data SD2");
    if ((fp = MyrFopen(filename, "r+b")) == NULL)
      scratch(11);
    GetDialogItem(gCurrent, id, &itemType, &itemHandle, &itemRect);
    GetIndString(myString, 3, 1);
    GetDialogItemText(itemHandle, myString);
    fseek(fp, stringindex * sizeof myString, SEEK_SET);
    fwrite(myString, sizeof myString, 1, fp);
    fclose(fp);
  }
}

/******************************** getstring **********************/
void getstring(void) {
  FILE* fp = NULL;
  if (stringindex < 0) {
    GetIndString(myString, 3, 1);
    getfilename("Data OD");
    if ((fp = MyrFopen(filename, "rb")) == NULL)
      scratch(12);
    fseek(fp, abs(stringindex) * 25, SEEK_SET);
    fread(myString, 25, 1, fp);

    PtoCstr(myString); // Fantasoft v7.1
    myString[24] = 0; // Fantasoft v7.1
    CtoPstr((Ptr)myString); // Fantasoft v7.1
    fclose(fp);
  } else {
    getfilename("Data SD2");
    if ((fp = MyrFopen(filename, "rb")) == NULL)
      scratch(13);
    fseek(fp, stringindex * sizeof myString, SEEK_SET);
    fread(myString, sizeof myString, 1, fp);
    fclose(fp);
  }
}

/************** warp ********************/
void warp(void) {
  WindowPtr oldptr;
  FILE *fp, *op, *fp2, *fp3;
  short tempx, tempy;
  long templong2;
  Rect temprect;
  Boolean setday = 0;

  if (!usercheck2()) {
    warn(132);
    return;
  }

  if (!indung)
    saveland(landlevel);
  else
    saveland(dunglevel);

  oldptr = FrontWindow();
  in();

  fat = 4;
  needupdate = TRUE;

  for (t = 0; t <= charnum; t++) {
    c[t].stamina = c[t].staminamax;
    c[t].spellpoints = c[t].spellpointsmax;
  }

  killparty = 0;

  getfilename("Global"); /******** always update global macros *****/
  if ((fp = fopen(filename, "rb")) != NULL) {
    fread(&globalmacro, sizeof globalmacro, 1, fp);
    CvtTabShortToPc(globalmacro, 30);
    fclose(fp);
  }

  gGeneration = GetNewDialog(-159, 0L, (WindowPtr)-1L);
  gCurrent = gGeneration;
  SetPortDialogPort(gGeneration);
  MoveWindow(GetDialogWindow(gGeneration), GlobalLeft + 320, GlobalTop, FALSE);
  ShowWindow(GetDialogWindow(gGeneration));
  DrawDialog(gGeneration);
  BeginUpdate(GetDialogWindow(gGeneration));
  EndUpdate(GetDialogWindow(gGeneration));

  if (indung) {
    DialogNumLong(5, dunglevel);
    DialogNumLong(3, floorx);
    DialogNumLong(4, floory);
  } else {
    DialogNumLong(5, landlevel);
    DialogNumLong(3, lookx + partyx);
    DialogNumLong(4, looky + partyy);
  }

  GetDialogItem(gGeneration, 9, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, indung);

  DialogNum(10, tyme.tm_yday);

  sound(607);

  DialogNum(14, itemnumid);

  DialogNum(31, npcid);

  getfilename("Data MD");
  fp = MyrFopen(filename, "rb");
  fseek(fp, npcid * sizeof monpick, SEEK_SET);
  /* !MYRIAD 12/10/99 Because fseek can be greater than the end of the file (and fread can fail)*/
  if (fread(&monpick, sizeof monpick, 1, fp) == 1)
    CvtMonsterToPc(&monpick);
  /* !MYRIAD 12/10/99 If not read, keeps the previous value of monpick */

  fclose(fp);

  iconhand = NIL;

  iconhand = GetCIcon(monpick.iconid);
  if (iconhand) {
    GetDialogItem(gCurrent, 28, &itemType, &itemHandle, &itemRect);
    EraseRect(&itemRect);
    if (monpick.size == 2)
      InsetRect(&itemRect, 0, 16);
    if (monpick.size == 1)
      InsetRect(&itemRect, 16, 0);
    if (monpick.size == 0)
      InsetRect(&itemRect, 16, 16);
    PlotCIcon(&itemRect, iconhand);
    DisposeCIcon(iconhand);
  }

  strcpy((StringPtr)gotword, monpick.monname);
  CtoPstr(gotword);
  MyrPascalDiStr(33, (StringPtr)gotword);

  DialogNum(12, questid);

  GetDialogItem(gGeneration, 11, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, quest[questid]);

  goto update;

  for (;;) {
  over:
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);

    downbutton2();

    if (itemHit == 10)
      setday = TRUE;

    if (itemHit == 9) /**** dungeon checkbox ****/
    {
      GetDialogItem(gGeneration, 9, &itemType, &itemHandle, &itemRect);
      indung = GetControlValue((ControlHandle)itemHandle);

      if (indung)
        indung = FALSE;
      else
        indung = TRUE;

      SetControlValue((ControlHandle)itemHandle, indung);
    }

    if (itemHit == 11) /**** quest checkbox ****/
    {
    updatequest:
      questid = GetDialogNum(12);

      if ((questid > -1) && (questid < 101)) {
        GetDialogItem(gGeneration, 11, &itemType, &itemHandle, &itemRect);
        SetControlValue((ControlHandle)itemHandle, quest[questid]);

        if (itemHit == 11) {
          if (quest[questid])
            quest[questid] = FALSE;
          else
            quest[questid] = TRUE;

          SetControlValue((ControlHandle)itemHandle, quest[questid]);
        }
      }
      // itemHit = 0;  Fantasoft v7.1
    }

    if (itemHit == 12)
      goto updatequest;

    if ((itemHit == 23) || (itemHit == 24)) {
      temp = GetDialogNum(26);
      if (itemHit == 23)
        temp--;
      else
        temp++;

      DialogNum(26, temp);

      goto updateicon;
    }

    if (itemHit == 27) /**** place icon in map *****/
    {
      tempx = GetDialogNum(21);
      tempy = GetDialogNum(20);

      templong = tempx;
      templong2 = tempy;

      field[templong + lookx][templong2 + looky] = GetDialogNum(26);
    }

    if ((itemHit == 26) || (itemHit == 20) || (itemHit == 21)) {
      BitMap* src = GetPortBitMapForCopyBits(gthePixels);
      BitMap* dst = GetPortBitMapForCopyBits(GetDialogPort(gCurrent));

    updateicon:

      centerpict();

      SelectWindow(GetDialogWindow(gCurrent));
      SetPortDialogPort(gCurrent);

      temp = GetDialogNum(26);

      tempx = GetDialogNum(21);
      tempy = GetDialogNum(20);

      itemRect.top = 32 * tempy;
      itemRect.left = 32 * tempx;
      itemRect.right = itemRect.left + 32;
      itemRect.bottom = itemRect.top + 32;

      fastplot(temp, itemRect, 0, 0); /***** 0 = look, 1 = buff ******/
      GetDialogItem(gCurrent, 22, &itemType, &itemHandle, &temprect);
      CopyBits(src, dst, &itemRect, &temprect, 0, NIL);

      gCurrent = gGeneration;
      SetPortDialogPort(gGeneration);
    }

    if (itemHit == 1) {
      sound(607);

      if (!indung) {
        lookx = GetDialogNum(3) - 5;
        looky = GetDialogNum(4) - 5;

        partyx = 5;
        partyy = 5;
      } else {
        floorx = GetDialogNum(3);
        floory = GetDialogNum(4);
      }

      temp = GetDialogNum(5);

      if (indung)
        dunglevel = temp;
      else
        landlevel = temp;

      if (setday) {
        tyme.tm_yday = GetDialogNum(10) - 1;
        tyme.tm_hour = 23;
        tyme.tm_min = 55;
      }

      if (indung)
        loadland(dunglevel, 1);
      else
        loadland(landlevel, 1);

      DisposeDialog(gGeneration);
      centerpict();
      return;
    }

    if (itemHit == 2) {
      sound(607);
      DisposeDialog(gGeneration);
      centerpict();
      return;
    }

    if (itemHit == 7) /***** land only ****/
    {
      sound(607);

      for (t = 0; t < 90; t++) {
        for (tt = 0; tt < 90; tt++)
          site[t][tt] = 0;
      }

      if ((op = MyrFopen(":Data Files:CLD", "w+b")) == NULL)
        scratch(14);
      getfilename("Data DD");
      if ((fp = MyrFopen(filename, "rb")) == NULL)
        scratch(15);
      getfilename("Data LD");
      if ((fp2 = MyrFopen(filename, "rb")) == NULL)
        scratch(16);
      getfilename("Data RD");
      if ((fp3 = MyrFopen(filename, "rb")) == NULL)
        scratch(17);
      while (fread(&door, sizeof door, 1, fp) == 1) {
        CvtTabDoorToPc(door, 100);
        sound(141);
        fread(&field, sizeof field, 1, fp2);
        CvtFieldToPc(&field);
        fread(&randlevel, sizeof randlevel, 1, fp3);
        CvtRandLevelToPc(&randlevel);

        CvtTabDoorToPc(door, 100);
        if (!fwrite(&door, sizeof door, 1, op))
          scratch(18);
        CvtTabDoorToPc(door, 100);

        CvtFieldToPc(&field);
        if (!fwrite(&field, sizeof field, 1, op))
          scratch(19);
        CvtFieldToPc(&field);

        CvtRandLevelToPc(&randlevel);
        if (!fwrite(&randlevel, sizeof randlevel, 1, op))
          scratch(20);
        CvtRandLevelToPc(&randlevel);

        /* MYRIAD : Boolean no conversion needed */
        if (!fwrite(&site, sizeof site, 1, op))
          scratch(20);
      }
      fclose(fp);
      fclose(fp2);
      fclose(fp3);
      fclose(op);
    }

    if (itemHit == 15) /**** give item *****/
    {
      itemnumid = GetDialogNum(14);
      for (t = 0; t < 20; t++)
        treasure.itemid[t] = itemnumid;
      booty(1);
      DisposeDialog(gGeneration);
      needupdate = TRUE;
      return;
    }

    if (itemHit == 32) /**** give Allies *****/
    {
      npcid = GetDialogNum(31);
      if (heldover < 20) {
        heldover++;
        monpick.stamina = monpick.staminamax = (monpick.hd * 6 + monpick.bonus);
        monpick.maxspellpoints = monpick.spellpoints;
        monpick.traiter = 0;
        holdover[heldover - 1] = monpick;
        sound(141);
        updatenpcmenu();
      } else
        SysBeep(20);
    }

    if ((itemHit == 18) || (itemHit == 19)) /*** change Item ****/
    {
      itemnumid = GetDialogNum(14);
      if (itemHit == 18)
        itemnumid--;
      else
        itemnumid++;

      DialogNum(14, itemnumid);
      goto update;
    }

    if ((itemHit == 29) || (itemHit == 30)) /*** change Allies ****/
    {
      npcid = GetDialogNum(31);
      if (itemHit == 29)
        npcid--;
      else
        npcid++;

      DialogNum(31, npcid);
      goto update2;
    }

    if (itemHit == 14) /**** check item *****/
    {
    update:

      itemnumid = GetDialogNum(14);
      loaditem(itemnumid);

      GetDialogItem(gCurrent, 16, &itemType, &itemHandle, &itemRect);

      iconhand = NIL;
      iconhand = GetCIcon(item.iconid);

      if (iconhand != NIL) {
        EraseRect(&itemRect);
        PlotCIcon(&itemRect, iconhand);
        GetIndString(myString, getselection(item.itemid) + 2, item.itemid - tempselection + 1);
        MyrPascalDiStr(17, myString);
        GetIndString(myString, getselection(item.itemid) + 1, item.itemid - tempselection + 1);
        MyrPascalDiStr(33, myString);
        DisposeCIcon(iconhand);
      } else {
        EraseRect(&itemRect);
        MyrCDiStr(17, (StringPtr) "");
      }
      itemHit = 0;
      goto over;
    }

    if (itemHit == 31) /**** check NPC *****/
    {
    update2:

      npcid = GetDialogNum(31);

      getfilename("Data MD");
      fp = MyrFopen(filename, "rb");
      fseek(fp, npcid * sizeof monpick, SEEK_SET);
      /* !MYRIAD 12/10/99 Because fseek can be greater than the end of the file (and fread can fail)*/
      if (fread(&monpick, sizeof monpick, 1, fp) == 1)
        CvtMonsterToPc(&monpick);
      /* !MYRIAD 12/10/99 If not read, keeps the previous value of monpick */
      fclose(fp);

      iconhand = NIL;

      iconhand = GetCIcon(monpick.iconid);
      if (iconhand) {
        GetDialogItem(gCurrent, 28, &itemType, &itemHandle, &itemRect);
        EraseRect(&itemRect);
        if (monpick.size == 2)
          InsetRect(&itemRect, 0, 16);
        if (monpick.size == 1)
          InsetRect(&itemRect, 16, 0);
        if (monpick.size == 0)
          InsetRect(&itemRect, 16, 16);
        PlotCIcon(&itemRect, iconhand);
        DisposeCIcon(iconhand);
      }

      strcpy((StringPtr)gotword, monpick.monname);
      CtoPstr(gotword);
      MyrPascalDiStr(33, (StringPtr)gotword);

      itemHit = 0;
      goto over;
    }

    if (itemHit == 6) {
      for (t = 0; t < 90; t++) {
        for (tt = 0; tt < 90; tt++)
          site[t][tt] = 0;
      }

      sound(607);
      if ((op = MyrFopen(":Data Files:CL", "w+b")) == NULL)
        scratch(21);
      getfilename("Data DD");
      if ((fp = MyrFopen(filename, "rb")) == NULL)
        scratch(22);
      getfilename("Data LD");
      if ((fp2 = MyrFopen(filename, "rb")) == NULL)
        scratch(23);
      getfilename("Data RD");
      if ((fp3 = MyrFopen(filename, "rb")) == NULL)
        scratch(25);
      while (fread(&door, sizeof door, 1, fp) == 1) {
        CvtTabDoorToPc(door, 100);
        sound(141);
        fread(&field, sizeof field, 1, fp2);
        CvtFieldToPc(&field);

        fread(&randlevel, sizeof randlevel, 1, fp3);
        CvtRandLevelToPc(&randlevel);

        CvtTabDoorToPc(door, 100);
        if (!fwrite(&door, sizeof door, 1, op))
          scratch(25);
        CvtTabDoorToPc(door, 100);

        CvtFieldToPc(&field);
        if (!fwrite(&field, sizeof field, 1, op))
          scratch(26);
        CvtFieldToPc(&field);

        CvtRandLevelToPc(&randlevel);
        if (!fwrite(&randlevel, sizeof randlevel, 1, op))
          scratch(27);
        CvtRandLevelToPc(&randlevel);

        if (!fwrite(&site, sizeof site, 1, op))
          scratch(27);
      }
      fclose(fp);
      fclose(fp2);
      fclose(fp3);
      fclose(op);

      if ((op = MyrFopen(":Data Files:CD", "w+b")) == NULL)
        scratch(28);
      getfilename("Data DDD");
      if ((fp = MyrFopen(filename, "rb")) == NULL)
        scratch(29);
      getfilename("Data DL");
      if ((fp2 = MyrFopen(filename, "rb")) == NULL)
        scratch(30);
      getfilename("Data RDD");
      if ((fp3 = MyrFopen(filename, "rb")) == NULL)
        scratch(31);
      while (fread(&door, sizeof door, 1, fp) == 1) {
        CvtTabDoorToPc(door, 100);
        sound(141);
        fread(&field, sizeof field, 1, fp2);
        CvtFieldToPc(&field);

        fread(&randlevel, sizeof randlevel, 1, fp3);
        CvtRandLevelToPc(&randlevel);

        CvtTabDoorToPc(door, 100);
        if (!fwrite(&door, sizeof door, 1, op))
          scratch(32);
        CvtTabDoorToPc(door, 100);

        CvtFieldToPc(&field);
        if (!fwrite(&field, sizeof field, 1, op))
          scratch(33);
        CvtFieldToPc(&field);

        CvtRandLevelToPc(&randlevel);
        if (!fwrite(&randlevel, sizeof randlevel, 1, op))
          scratch(34);
        CvtRandLevelToPc(&randlevel);
      }
      fclose(fp);
      fclose(fp2);
      fclose(fp3);
      fclose(op);

      getfilename("Data SD");
      if ((fp = MyrFopen(filename, "rb")) == NULL)
        scratch(35); /********* shops ********/
      if ((op = MyrFopen(":Data Files:CS", "w+b")) == NULL)
        scratch(36);
      while (fread(&theshop, sizeof theshop, 1, fp) == 1) {
        // No conversion needed : i write the data read as is
        if (!fwrite(&theshop, sizeof theshop, 1, op))
          scratch(37);
      }
      fclose(fp);
      fclose(op);

      getfilename("Data TD2");
      if ((fp = MyrFopen(filename, "rb")) == NULL)
        scratch(38); /********* thief ********/
      if ((op = MyrFopen(":Data Files:CT", "w+b")) == NULL)
        scratch(39);
      while (fread(&thief, sizeof thief, 1, fp) == 1) {
        if (!fwrite(&thief, sizeof thief, 1, op))
          scratch(40);
      }
      fclose(fp);
      fclose(op);

      getfilename("Data TD3");
      if ((fp = MyrFopen(filename, "rb")) == NULL)
        scratch(41); /********* time encoutner ********/
      if ((op = MyrFopen(":Data Files:CTD3", "w+b")) == NULL)
        scratch(42);
      while (fread(&dotime, sizeof dotime, 1, fp) == 1) {
        if (!fwrite(&dotime, sizeof dotime, 1, op))
          scratch(43);
      }
      fclose(fp);
      fclose(op);

      getfilename("Data ED");
      if ((fp = MyrFopen(filename, "rb")) == NULL)
        scratch(44); /********* simple encounters ********/

      if ((op = MyrFopen(":Data Files:CE", "w+b")) == NULL)
        scratch(45);
      while (fread(&enc, sizeof enc, 1, fp) == 1) {
        for (tt = 0; tt < 4; tt++) {
          GetIndString(buffer[tt], 3, 1);
          fread(&buffer[tt], 80, 1, fp);
        }
        fwrite(&enc, sizeof enc, 1, op);
        for (tt = 0; tt < 4; tt++) {
          if (!fwrite(&buffer[tt], 80, 1, op))
            scratch(46);
        }
      }
      fclose(fp);
      fclose(op);

      getfilename("Data ED2");
      if ((fp = MyrFopen(filename, "rb")) == NULL)
        scratch(47); /********* complex encounters ********/
      if ((op = MyrFopen(":Data Files:CE2", "w+b")) == NULL)
        scratch(48);

      while (fread(&enc2, sizeof enc2, 1, fp)) {
        for (t = 0; t < 9; t++)
          fread(&buffer[t], 40, 1, fp);
        if (!fwrite(&enc2, sizeof enc2, 1, op))
          scratch(49);
        for (t = 0; t < 9; t++) {
          if (!fwrite(&buffer[t], 40, 1, op))
            scratch(50);
        }
      }
      fclose(fp);
      fclose(op);

      if (indung)
        loadland(dunglevel, 1);
      else
        loadland(landlevel, 1);

      DisposeDialog(gGeneration);
      gCurrent = GetDialogFromWindow(oldptr);
      out();
      return;
    }
  }
}

/*********************** editstring **************************/
void editstring(void) {
  DialogRef string;

  string = GetNewDialog(-21, NIL, (WindowPtr)-1L);
  SetPortDialogPort(string);
  gCurrent = string;
  gStop = FALSE;

  ShowWindow(GetDialogWindow(string));

  DialogNum(6, stringindex);
  getstring();
  MyrPascalDiStr(2, myString);

  while (gStop == FALSE) {

    FlushEvents(everyEvent, 0);
    ModalDialog(NIL, &itemHit);

    if (itemHit == 9) /******** scrub ******/
    {
      makestring(2);
      stringindex = GetDialogNum(6);

      while (stringindex) {
        getstring();
        MyrPascalDiStr(2, myString);
        makestring(2);
        stringindex--;
        DialogNum(6, stringindex);
      }
    }

    if (itemHit == 5) {
      stringindex = GetDialogNum(6);
      getstring();
      MyrPascalDiStr(2, myString);
    }

    if (itemHit == 3) {
      GetIndString(myString, 3, 1);
      MyrPascalDiStr(2, myString);
      makestring(2);
    }

    if ((itemHit == 7) || (itemHit == 8)) {
      makestring(2);
      stringindex = GetDialogNum(6);
      if (itemHit == 8)
        stringindex++;
      else
        stringindex--;
      getstring();
      MyrPascalDiStr(2, myString);
      DialogNum(6, stringindex);
    }

    if (itemHit == 1) {
      makestring(2);
      DisposeDialog(string);
      if (!incombat)
        centerpict();
      return;
    }
  }
}
