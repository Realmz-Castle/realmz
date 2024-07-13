#include "prototypes.h"
#include "variables.h"

/******************* thiefenc **********************/
short thiefenc(void) {
  FILE* fp = NULL;
  char type, try = 0;
  Boolean first = 0;
  Boolean beenthere = 0;
  DialogRef templewindow;

  SetCCursor(sword);
  needdungeonupdate = TRUE;

  SetRect(&buttonrect, 0, 0, 0, 0);

  if ((fp = MyrFopen(":Data Files:CT", "rb")) == NULL)
    scratch(51);
  fseek(fp, enc2.thiefsuccess * sizeof thief, SEEK_SET);
  fread(&thief, sizeof thief, 1, fp);
  CvtThiefToPc(&thief);
  fclose(fp);

  templewindow = GetNewDialog(162, 0L, (WindowPtr)-1L);
  MoveWindow(GetDialogWindow(templewindow), GlobalLeft + 7 + (leftshift / 2), GlobalTop + 7 + (downshift / 2), FALSE);
  ShowWindow(GetDialogWindow(templewindow));
  SetPortDialogPort(templewindow);
  gCurrent = templewindow;
  BackPixPat(base);
  sound(6001);

  in();

  if (Rand(100) < 10) {
    if ((!doreg4()) && (currentscenario > 10))
      scratch(452);
  }

bigupdate:

  if (thief.sound[0])
    sound(thief.sound[0]);
  textbox(-1, -(abs(thief.prompt[0])), 0, 0, textrect);

  charselectnew--;

  goto tryover;

update:

  beenthere = TRUE;

  SelectWindow(GetDialogWindow(templewindow));
  SetPortDialogPort(templewindow);
  gCurrent = templewindow;
  TextFont(defaultfont);
  ForeColor(yellowColor);
  TextFont(font);
  TextSize(14);
  TextFace(bold);
  ErasePortRect();
  MyrCDiStr(2, (StringPtr) "");
  DrawDialog(templewindow);

update2:

  GetDialogItem(templewindow, 2, &itemType, &itemHandle, &itemRect);
  pict(0, itemRect);
  GetDialogItem(templewindow, 7, &itemType, &itemHandle, &itemRect);
  plotportrait(c[charselectnew].pictid, itemRect, c[charselectnew].caste, -1);
  ForeColor(yellowColor);
  CtoPstr((Ptr)c[charselectnew].name);
  MyrPascalDiStr(41, (StringPtr)c[charselectnew].name);
  PtoCstr((StringPtr)c[charselectnew].name);

  for (t = 24; t < 32; t++) {
    GetDialogItem(templewindow, t, &itemType, &itemHandle, &itemRect);
    GetDialogItem(templewindow, t - 16, &itemType, &itemHandle, &buttonrect);
    GetIndString(myString, 1201, t - 18);
    if ((c[charselectnew].spec[t - 19] + thief.modifer[t - 24] > 0) && (thief.type[t - 24]) && (c[charselectnew].spec[t - 19])) {
      upbutton(FALSE);
      if (!first)
        ForeColor(yellowColor);

    } else
      downbutton(FALSE);

    MyrPascalDiStr(t, myString);
    if (c[charselectnew].spec[t - 19])
      DialogNum(t + 8, c[charselectnew].spec[t - 19] + thief.modifer[t - 24]);
    else
      DialogNum(t + 8, 0);

    GetDialogItem(templewindow, t - 8, &itemType, &itemHandle, &buttonrect);
    downbutton(FALSE);
  }

  first = TRUE;

  ForeColor(yellowColor);
  for (;;) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(templewindow, itemHit, &itemType, &itemHandle, &buttonrect);

    if ((itemHit > 7) && (itemHit < 16) && (c[charselectnew].spec[itemHit - 3] > 0) && (thief.type[itemHit - 8])) {
      downbutton(FALSE);
      sound(10129);
      type = itemHit - 8;
      thief.type[type] = FALSE;

      if ((type == 4) && (thief.type[9]))
        thief.type[4] = TRUE; /*** reset bendbars on trap triggger ***/

      if ((type != 2) && (type != 6) && (type != 7) && (type != 4)) {
        if (Rand(100) <= (c[charselectnew].spec[itemHit - 3] + thief.modifer[itemHit - 8])) {
          sound(thief.sounds[type]);
          textbox(-1, thief.texts[type], 0, 0, textrect);
          if ((type == 1) && (thief.type[9])) {
            thief.type[2] = TRUE; /**** detect trap sets disarm trap ******/
            first = FALSE;
          }

          if (thief.codes[type]) {
            reply = thief.codes[type];
            goto out;
          } else
            goto update;
        } else if (type == 1)
          textbox(-1, thief.textf[type], 0, 0, textrect); /**** failed to detect trap ****/
        else
          goto dotrap;

        if (thief.codef[type]) {
          reply = thief.codef[type];
          goto out;
        } else
          goto update;
      } else /***** pick lock/ pocket / disarm trap ******/
      {
        if ((thief.type[9]) && (type != 2))
          goto sprungtrap;

        if (picklock(charselectnew, type)) /**** success ****/
        {
          sound(thief.sounds[type]);
          textbox(-1, thief.texts[type], 0, 0, textrect);
          if (type == 2)
            thief.type[9] = 0; /**** disarm trap *****/
          if (thief.codes[type]) {
            reply = thief.codes[type];
            if (reply != 4)
              c[charselectnew].exp += (300 * thief.tumblers);
            goto out;
          }
        } else /***** failed *****/
        {
        dotrap:
          sound(thief.soundf[type]);
          textbox(-1, thief.textf[type], 0, 0, textrect);

          if (thief.type[9]) /***** trap is set and triggered ****/
          {
          sprungtrap:
            textbox(3, 55, TRUE, 0, textrect);
            thief.type[9] = thief.type[1] = 0;
            if (thief.lowdamage) {
              if (thief.type[8]) {
                sound(thief.prompt[1]);
                temp = randrange(thief.lowdamage, thief.highdamage);
                heal(charselectnew, -temp, TRUE);
                SetPortDialogPort(templewindow);
                gCurrent = templewindow;
              } else
                for (t = 0; t <= charnum; t++) {
                  sound(thief.prompt[1]);
                  temp = randrange(thief.lowdamage, thief.highdamage);
                  heal(t, -temp, TRUE);
                  SetPortDialogPort(templewindow);
                  gCurrent = templewindow;
                }
            }

            if (thief.spell) {
              if (thief.type[8])
                track[charselectnew] = 1;
              else
                for (t = 0; t <= charnum; t++)
                  track[t] = 1;

              loadspell2(thief.spell);
              powerlevel = thief.prompt[2];
              resolvespell();
            }
            thief.type[6] = TRUE;

            if (c[charselectnew].stamina < 1) /**** trap killed thief ***/
            {
              itemHit = 4;
              goto tryover;
            }
            goto update;
          }
        }
        if (thief.codef[type]) {
          reply = thief.codef[type];
          goto out;
        } else
          goto update;
      }
    }

    if ((itemHit == 5) || (itemHit == 1)) {
      GetDialogItem(templewindow, 5, &itemType, &itemHandle, &buttonrect);
      sound(141);
      ploticon3(129, buttonrect);
      reply = 0;

    out:

      if (enc2.thiefsuccess) {
        if ((fp = MyrFopen(":Data Files:CT", "r+b")) == NULL)
          scratch(52);
        fseek(fp, enc2.thiefsuccess * sizeof thief, SEEK_SET);
        CvtThiefToPc(&thief);
        fwrite(&thief, sizeof thief, 1, fp);
        CvtThiefToPc(&thief);
        fclose(fp);
      }

      DisposeDialog(templewindow);
      centerpict();
      return (reply);
    }

    if ((itemHit == 3) || (itemHit == 4)) {
      ploticon3(135, buttonrect);
      sound(141);

    tryover:
      GetDialogItem(templewindow, itemHit, &itemType, &itemHandle, &buttonrect);
      charselectnew++;
      if (itemHit == 3)
        charselectnew -= 2;
      if (charselectnew > charnum)
        charselectnew = 0;
      if (charselectnew < 0)
        charselectnew = charnum;
      if ((c[charselectnew].stamina < 1) || (c[charselectnew].condition[25]))
        goto tryover;
      ploticon3(136, buttonrect);
      if (beenthere)
        goto update2;
      goto update;
    }

    if (itemHit == 2) {
      ploticon3(129, buttonrect);
      viewcharacter(charselectnew, 0);
      updatemain(FALSE, -1);
      DrawDialog(templewindow);
      in();
      goto update;
    }
  }
}

/**************** encounter **	simple *****************************/
short encounter(short id, short mode) {
  FILE* fp = NULL;
  DialogRef win;

  compactheap();

  needdungeonupdate = TRUE;

  SetCCursor(sword);
  if (mode) /**** all new encounter ***/
  {
    sound(20005);
    if ((fp = MyrFopen(":Data Files Data:CE", "rb")) == NULL)
      scratch(53);
    fseek(fp, id * (sizeof enc + 320), SEEK_SET);
    fread(&enc, sizeof enc, 1, fp);
    CvtEncountToPc(&enc);
    for (t = 0; t < 4; t++) {
      GetIndString(buffer[t], 3, 1);
      fread(&buffer[t], 80, 1, fp);
      MyrStrPtoC((Ptr)&buffer[t]); // myriad
    }
    fclose(fp);
  }

  /***** check to see if choices have been eliminated *****/
  for (t = 0; t < 4; t++)
    if (!enc.choiceresult[t])
      GetIndString(buffer[t], 3, 1);

  ParamText(buffer[0], buffer[1], buffer[2], buffer[3]);
  textbox(-1, -(abs(enc.prompt)), 0, 0, textrect);

  in();
  SetCCursor(sword);
  win = GetNewDialog(147, 0L, (WindowPtr)-1L);
  SetPortDialogPort(win);
  TextFont(defaultfont);
  ForeColor(yellowColor);
  if (!enc.canbackout)
    SizeWindow(GetDialogWindow(win), 259, 267, 0);
  else
    SizeWindow(GetDialogWindow(win), 259, 320, 0);
  MoveWindow(GetDialogWindow(win), GlobalLeft + 30, GlobalTop, FALSE);
  ShowWindow(GetDialogWindow(win));
  BackPixPat(base);
  ErasePortRect();

  FlushEvents(everyEvent, 0);
  delay(15);
over:

  SelectWindow(GetDialogWindow(win));
  SetPortDialogPort(win);
  FlushEvents(everyEvent, 0);
  ModalDialog(0L, &itemHit);

  if (itemHit == 9)
    reply = 0;
  else if (enc.choiceresult[itemHit - 1])
    reply = itemHit;
  else {
    sound(6000);
    goto over;
  }

  DisposeDialog(win);
  SetCCursor(sword);
  centerpict();
  EraseRect(&textrect);
  if (reply)
    return (enc.choiceresult[reply - 1]);
  else
    return (0);
}

/**************** encounter2 *** Complex ****************************/
short encounter2(void) {
  FILE* fp = NULL;
  DialogRef win;
  long longvalue, longvalue2, longvalue3;
  short spell, t, knockchance, savedung, temp1, temp2;
  char reply, encsuccess, tag, goodword = TRUE;
  char word[256];

  encsuccess = tag = FALSE;
  SetCCursor(sword);

  in();

  win = GetNewDialog(153, 0L, (WindowPtr)-1L);
  if (!enc2.canbackout)
    SizeWindow(GetDialogWindow(win), 300, 50, 0);
  else
    SizeWindow(GetDialogWindow(win), 350, 50, 0);

  MoveWindow(GetDialogWindow(win), GlobalLeft - 1, GlobalTop + 270 + downshift, FALSE);
  ShowWindow(GetDialogWindow(win));
  SetPortDialogPort(win);
  BackPixPat(base);
  ErasePortRect();
  FlushEvents(everyEvent, 0);

backup:
  SelectWindow(GetDialogWindow(win));
  SetPortDialogPort(win);
  TextFont(defaultfont);
  DrawDialog(win);
  SetCCursor(sword);
  GetDialogItem(win, 3, &itemType, &itemHandle, &itemRect);
  pict(151, itemRect); /*********** scroll avail ***************/

  if ((!checkfortype(-1, 13, FALSE)) || (!enc2.spellid[0])) {
    InsetRect(&itemRect, 9, 9);
    ploticon3(0, itemRect);
  }

  GetDialogItem(win, 6, &itemType, &itemHandle, &itemRect);
  pict(201, itemRect); /*********** thief avail ***************/

  if (!enc2.thief) {
    InsetRect(&itemRect, 9, 9);
    ploticon3(0, itemRect);
  }

  GetDialogItem(win, 7, &itemType, &itemHandle, &itemRect);
  pict(143, itemRect); /**************** word avail ******************/

  if (!enc2.wordresult) {
    InsetRect(&itemRect, 9, 9);
    ploticon3(0, itemRect);
  }

  GetDialogItem(win, 5, &itemType, &itemHandle, &itemRect);
  pict(171, itemRect); /**************** action avail ******************/

  if (!enc2.choiceresult) {
    InsetRect(&itemRect, 9, 9);
    ploticon3(0, itemRect);
  }

  GetDialogItem(win, 2, &itemType, &itemHandle, &itemRect);
  pict(162, itemRect); /**************** spell avail ******************/

  if (!enc2.spellid[0]) {
    InsetRect(&itemRect, 9, 9);
    ploticon3(0, itemRect);
  }

  GetDialogItem(win, 4, &itemType, &itemHandle, &itemRect);
  pict(172, itemRect); /**************** item avail ******************/

  if (!enc2.itemid[0]) {
    InsetRect(&itemRect, 9, 9);
    ploticon3(0, itemRect);
  }

  textbox(-1, -(abs(enc2.prompt)), 0, 0, textrect);
  sound(20005);
  delay(15);

  SetPortDialogPort(win);
  ForeColor(yellowColor);
  encountflag = 2;

tryagain:
  SelectWindow(GetDialogWindow(win));
  SetPortDialogPort(win);
  FlushEvents(everyEvent, 0);
  ModalDialog(0L, &itemHit);
  MyrCheckMemory(2);
  GetDialogItem(win, itemHit, &itemType, &itemHandle, &buttonrect);
  ploticon3(129, buttonrect);

  if (itemHit == 7) /****** word *****/
  {
    if (!enc2.wordresult)
      goto nochoice;
    getword();
    sound(1101);

    if (!strlen(gotword)) {
      centerpict();
      goto backup;
    }

    PtoCstr((StringPtr)gotword);
    PtoCstr(buffer[8]);

    strcpy(word, buffer[8]);
    CtoPstr((Ptr)buffer[8]);

    for (t = 0; t < 40; t++) {
      if ((word[t] != 32) && (word[t])) {
        gotword[t] = tolower(gotword[t]);
        if (word[t] != gotword[t]) {
          goodword = FALSE;
          goto done;
        }
      } else
        goto done;
    }

  done:

    if (!strcmp(gotword, (StringPtr) "home")) /**** check for home word ****/
    {
      if (indung)
        saveland(dunglevel);
      else
        saveland(landlevel);

      indung = partyx = partyy = landlevel = 0;

      cancamp = 0;

      spellcasting = 0;
      monstercasting = 0;
      spellcharging = 0;

      GetMenuItemText(gGame, currentscenario, myString);
      PtoCstr(myString);
      getfilename((Ptr)myString);
      if ((fp = MyrFopen(filename, "rb")) == NULL)
        scratch(300);
      // fread(&reclevel,sizeof reclevel,5,fp); Illegal
      fread(&reclevel, sizeof(long), 1, fp); // Myriad
      CvtLongToPc(&reclevel);
      fread(&maxlevel, sizeof(long), 1, fp); // Myriad
      CvtLongToPc(&maxlevel);
      fread(&landlevel, sizeof(long), 1, fp); // Myriad
      CvtLongToPc(&landlevel);
      fread(&lookx, sizeof(long), 1, fp); // Myriad
      CvtLongToPc(&lookx);
      fread(&looky, sizeof(long), 1, fp); // Myriad
      CvtLongToPc(&looky);

      fclose(fp);

      x = y = 0;

      loadland(landlevel, 1);

      canpriestturn = TRUE;

      centerpict();
      in();
      goto out;
    }

    if (!strcmp(gotword, (StringPtr) "boat")) /**** check for home word ****/
    {
      flashmessage((StringPtr) "0 = Leave Boat", 20, 40, -1, 6000);

      getword();
      StringToNum((StringPtr)gotword, &templong);

      if (templong == 666)
        inboat = TRUE;
      else
        inboat = FALSE;

      centerpict();
      in();
      goto out;
    }

    if (!strcmp(gotword, (StringPtr) "turn")) /**** check for home word ****/
    {
      flashmessage((StringPtr) "Turning Enabled.", 20, 40, -1, 6000);

      canpriestturn = TRUE;
    }

    if (!strcmp(gotword, (StringPtr) "give item")) /**** check for give item word ****/
    {

      flashmessage((StringPtr) "Item ID Number:", 20, 40, -1, 6000);

      getword();

      for (t = 0; t < 20; t++)
        treasure.itemid[t] = 0;

      StringToNum((StringPtr)gotword, &templong);
      treasure.itemid[0] = templong;

      flashmessage((StringPtr) "", 20, 40, -1, 0);

      booty(1);

      updatemain(FALSE, -1);
      SetMenuBar(myMenuBar);
      InsertMenu(gSound, -1);
      InsertMenu(gSpeed, -1);
      DrawMenuBar();

      goto out;
    }

    if (!strcmp(gotword, (StringPtr) "camp")) /**** let them camp ****/
    {
      cancamp = 0;

      spellcasting = 0;
      monstercasting = 0;
      spellcharging = 0;

      updatemain(FALSE, -1);
      SetMenuBar(myMenuBar);
      InsertMenu(gSound, -1);
      InsertMenu(gSpeed, -1);
      DrawMenuBar();

      goto out;
    }

    if (!strcmp(gotword, (StringPtr) "change land")) /**** check for change land word ****/
    {
      if (indung)
        return (NIL);

      saveland(landlevel);

      flashmessage((StringPtr) "Land level: X: Y: ID/Bit:", 20, 40, -1, 6000);

      getword();

      StringToNum((StringPtr)gotword, &templong);

      getword();

      StringToNum((StringPtr)gotword, &longvalue);

      getword();

      StringToNum((StringPtr)gotword, &longvalue2);

      getword();

      StringToNum((StringPtr)gotword, &longvalue3);

      flashmessage((StringPtr) "", 20, 40, -1, 0);

      loadland(templong, 1);

      /***** do actual land change ***/

      temp = longvalue;
      temp1 = longvalue2;
      temp2 = longvalue3;

      if ((field[temp][temp1] < 999) && (field[temp][temp1] > 0))
        field[temp][temp1] = temp2;

      /***** done changing land ******/

      saveland(templong);

      loadland(landlevel, 1);

      centerpict();
      in();
      goto out;
    }

    if (!strcmp(gotword, (StringPtr) "reset door")) /**** check for Reset Door word ****/
    {
      if (indung)
        saveland(dunglevel);
      else
        saveland(landlevel);

      savedung = indung;

      flashmessage((StringPtr) "Land Level:   Door Number:", 20, 40, -1, 6000);

      getword();

      StringToNum((StringPtr)gotword, &templong);

      getword();

      StringToNum((StringPtr)gotword, &longvalue);

      flashmessage((StringPtr) "", 20, 40, -1, 0);

      if (templong < 1)
        indung = TRUE;
      else
        indung = FALSE;

      loadland(abs(templong) - 1, 1);

      /***** do the actual reset ******/
      temp = longvalue;

      if (templong < 0)
        getfilename("Data DDD");
      else
        getfilename("Data DD");

      fp = MyrFopen(filename, "rb");
      fseek(fp, ((templong - 1) * (sizeof field + sizeof door + sizeof randlevel)) + (temp * sizeof infodoor), SEEK_SET);
      fread(&infodoor, sizeof infodoor, 1, fp);
      CvtDoorToPc(&infodoor);
      fclose(fp);

      for (tt = 0; tt < 8; tt++) {
        door[temp].code[tt] = infodoor.code[tt];
        door[temp].id[tt] = infodoor.id[tt];
      }

      /***** done reseting door ******/

      if (temp > 0)
        saveland(templong - 1);
      else
        saveland(templong - 1);

      indung = savedung;

      loadland(landlevel, 1);

      centerpict();
      in();
      goto out;
    }

    if (!strcmp(gotword, (StringPtr) "activate door")) /**** check for activate door word ****/
    {
      if (indung)
        saveland(dunglevel);
      else
        saveland(landlevel);

      flashmessage((StringPtr) "Land Level:   Door Number:    New Percent:", 20, 40, -1, 6000);

      getword();

      StringToNum((StringPtr)gotword, &templong);

      getword();

      StringToNum((StringPtr)gotword, &longvalue);

      getword();

      StringToNum((StringPtr)gotword, &longvalue2);

      flashmessage((StringPtr) "", 20, 40, -1, 0);

      if (templong > 0)
        loadland(templong - 1, 1);
      else
        loadland(templong - 1, 1);

      door[longvalue].percent = longvalue2;

      if (temp > 0)
        saveland(templong - 1);
      else
        saveland(templong - 1);

      loadland(landlevel, 1);

      centerpict();
      in();
      goto out;
    }

    if (!strcmp(gotword, (StringPtr) "trigger time")) /**** check for trigger time word ****/
    {
      flashmessage((StringPtr) "Time Encounter:", 20, 40, -1, 6000);

      getword();

      StringToNum((StringPtr)gotword, &templong);

      flashmessage((StringPtr) "", 20, 40, -1, 0);

      fp = MyrFopen(":Data Files:CTD3", "r+b");
      fseek(fp, templong * sizeof dotime, SEEK_SET);
      fread(&dotime, sizeof dotime, 1, fp);
      CvtTimeEncounterToPc(&dotime);
      fclose(fp);

      dotime.day = tyme.tm_yday + 1;
      dotime.reclevel = dotime.recrect = dotime.recitem = dotime.recquest = dotime.recx = dotime.recy = -1;
      dotime.percent = 100;

      fp = MyrFopen(":Data Files:CTD3", "r+b");
      fseek(fp, templong * sizeof dotime, SEEK_SET);
      CvtTimeEncounterToPc(&dotime);
      fwrite(&dotime, sizeof dotime, 1, fp);
      CvtTimeEncounterToPc(&dotime);

      fclose(fp);

      centerpict();
      in();
      goto out;
    }

    if (!strcmp(gotword, (StringPtr) "trigger simple")) /**** check for trigger simple word ****/
    {
      flashmessage((StringPtr) "Simple Encounter:", 20, 40, -1, 6000);

      getword();

      StringToNum((StringPtr)gotword, &templong);

      flashmessage((StringPtr) "", 20, 40, -1, 0);

      temp = templong;

      newland(0L, 0L, 2, temp, 0);

      centerpict();
      in();
      goto out;
    }

    if (!strcmp(gotword, (StringPtr) "trigger complex")) /**** check for trigger complex word ****/
    {
      flashmessage((StringPtr) "Complex Encounter:", 20, 40, -1, 6000);

      getword();

      StringToNum((StringPtr)gotword, &templong);

      flashmessage((StringPtr) "", 20, 40, -1, 0);

      temp = templong;

      newland(0L, 0L, 3, temp, 0);

      centerpict();
      in();
      goto out;
    }

    if (!strcmp(gotword, (StringPtr) "set quest")) /**** check for set quest word ****/
    {

      flashmessage((StringPtr) "Quest ID:   1 = Set, 0 = Clear", 20, 40, -1, 6000);

      getword();

      StringToNum((StringPtr)gotword, &longvalue);

      getword();

      StringToNum((StringPtr)gotword, &longvalue2);

      flashmessage((StringPtr) "", 20, 40, -1, 0);

      if (longvalue == 127)
        longvalue2 = 0;

      quest[longvalue] = longvalue2;

      centerpict();
      in();
      goto out;
    }

    if (!strcmp(gotword, (StringPtr) "cast spell")) /**** check for set spell flag word ****/
    {

      spellcasting = 0;
      monstercasting = 0;
      spellcharging = 0;
    }

    if (!strcmp(gotword, (StringPtr) "give creature")) /**** check for give creature word ****/
    {

      flashmessage((StringPtr) "Creature ID:", 20, 40, -1, 6000);

      getword();

      StringToNum((StringPtr)gotword, &longvalue);

      flashmessage((StringPtr) "", 20, 40, -1, 0);

      temp = longvalue;

      getfilename("Data MD");
      fp = MyrFopen(filename, "rb");
      fseek(fp, temp * sizeof monpick, SEEK_SET);
      /* !MYRIAD 12/10/99 Because fseek can be greater than the end of the file (and fread can fail)*/
      if (fread(&monpick, sizeof monpick, 1, fp) == 1)
        CvtMonsterToPc(&monpick);
      /* !MYRIAD 12/10/99 If not read, keeps the previous value of monpick */

      fclose(fp);

      heldover++;
      monpick.stamina = monpick.staminamax = (monpick.hd * 6 + monpick.bonus);
      monpick.maxspellpoints = monpick.spellpoints;
      monpick.traiter = 0;
      holdover[heldover - 1] = monpick;

      updatenpcmenu();

      centerpict();
      in();
      goto out;
    }

    if (goodword)
      encsuccess = enc2.wordresult;
    else
      encsuccess = 4;

    goto out;
  }

  if (itemHit == 2) /****** cast spell *****/
  {
    if (!enc2.spellid[0])
      goto nochoice;

    scribing = 1;
    spell = castspell();
    centerpict();

  tryspell:

    updatemain(TRUE, -1);
  tryspell2:

    inspell = FALSE;
    if (spell < 1101)
      goto backup;

  resolve:

    updatecontrols();
    updatecharshort(charselectnew, FALSE);
    cleartarget();
    sound(spellinfo.sound2 + 600);

    if ((spellinfo.special == 65) || (spellinfo.special == 70)) {
      /******* 70 = knock, 65 = disarmtrap ******/
      if (enc2.thief) {
        if ((fp = MyrFopen(":Data Files:CT", "rb")) == NULL)
          scratch(54);
        fseek(fp, enc2.thiefsuccess * sizeof thief, SEEK_SET);
        fread(&thief, sizeof thief, 1, fp);
        CvtThiefToPc(&thief);
        fclose(fp);

        if ((thief.sound[2]) && (spellinfo.special == 65)) /*** disarmtrap ***/
        {
          if (Rand(100) < thief.sound[2] * powerlevel) {
            thief.type[9] = FALSE; /*** success ***/

            sound(thief.sounds[2]);
            textbox(-1, thief.texts[2], 0, 0, textrect);

            reply = thief.codes[2];
            encsuccess = reply;
            goto out;
          } else {
            reply = thief.codef[2];
            sound(thief.soundf[2]);
            textbox(-1, thief.textf[2], 0, 0, textrect);
            spellinfo.special = 70;
          }
        }

        if ((thief.sound[1]) && (spellinfo.special == 70)) /*** knock ***/
        {
          knockchance = thief.sound[1] * powerlevel;

          if (thief.type[9]) /**** set off trap ****/
          {
            textbox(3, 55, TRUE, 0, textrect);
            thief.type[9] = thief.type[1] = 0;
            if (thief.lowdamage) {
              if (thief.type[8]) {
                sound(thief.prompt[1]);
                temp = randrange(thief.lowdamage, thief.highdamage);
                heal(charselectnew, -temp, TRUE);
              } else
                for (t = 0; t <= charnum; t++) {
                  sound(thief.prompt[1]);
                  temp = randrange(thief.lowdamage, thief.highdamage);
                  heal(t, -temp, TRUE);
                }
            }

            if (thief.spell) {
              if (thief.type[8])
                track[charselectnew] = 1;
              else
                for (t = 0; t <= charnum; t++)
                  track[t] = 1;

              loadspell2(thief.spell);
              powerlevel = thief.prompt[2];
              resolvespell();
            }
            thief.type[6] = TRUE;
          }

          if (Rand(100) < thief.sound[2] * powerlevel) {
            thief.type[9] = FALSE; /*** success ***/

            sound(thief.sounds[6]);
            textbox(-1, thief.texts[6], 0, 0, textrect);

            reply = thief.codes[6];
            encsuccess = reply;
          } else {
            reply = thief.codef[6];
            sound(thief.soundf[6]);
            textbox(-1, thief.textf[6], 0, 0, textrect);
          }
          goto out;
        }
      }
    }

    if (spellinfo.targettype == 7) {
      resolvespell(); /**** party ****/
      tickcheck();
    }

    for (t = 0; t < 10; t++) {
      if (spell == enc2.spellid[t]) {
        encsuccess = enc2.spellresult[t];
        goto out;
      }

      if ((enc2.spellid[t]) && (enc2.spellid[t] < 7)) /**** react to class of spell ****/
      {
        loadspell2(spell);
        if (spellinfo.spellclass == enc2.spellid[t]) {
          encsuccess = enc2.spellresult[t];
          goto out;
        }
      }
    }
    encsuccess = 4; /***** default fail *****/
    goto out;
  }

  if (itemHit == 3) /***** use scroll *****/
  {
    if ((checkfortype(-1, 13, FALSE)) && (enc2.spellid[0])) {
      if (getscroll()) {
        spell = 1000 * castcaste + 100 * castlevel + castnum + 1101;
        loadspell2(spell);
        inspell = FALSE;
        goto tryspell;
      } else
        goto backup;
    } else {
    nochoice:
      sound(6000);
      goto backup;
    }
  }

  if (itemHit == 4) /****** Use Item *****/
  {
    if (!enc2.itemid[0])
      goto nochoice;

    in();
    itemused = items();

    updatemain(FALSE, -1);
    if (!itemused)
      goto backup;

    if (item.type == 20) {
      spell = item.sp2;
      loadspell2(item.sp2);
      goto tryspell2; /***** scroll **************/
    }

    if (item.type == 23) /****** activate door item  *****/
    {
      itemused = -(item.sp5 + 100);
      encsuccess = -1;
      goto out;
    }

    for (t = 0; t < 5; t++) {
      if (itemused == enc2.itemid[t]) {
        encsuccess = enc2.itemresult[t];
        goto out;
      }
    }
    encsuccess = 4; /***** default fail *****/
    goto out;
  }

  if (itemHit == 6) /***** Thief Action ****/
  {
    if (!enc2.thief)
      goto nochoice;

    reply = thiefenc();

    if (!reply)
      goto backup;
    else
      encsuccess = reply;
    goto out;
  }

  if (itemHit == 5) /****** Action *****/
  {
    if (!enc2.choiceresult)
      goto nochoice;
    reply = actionpicker();
    updatemain(TRUE, -1);
    if (!reply)
      goto backup;
    temp = 0;
    encsuccess = TRUE;

    for (t = 0; t < 8; t++)
      if (enc2.group[t])
        temp++;
    if (!temp) {
      encsuccess = 4;
      goto out;
    }

    for (t = 0; t < 8; t++) {
      if ((actiontaken[t]) && (!enc2.group[t]))
        encsuccess = 0;
      if ((!actiontaken[t]) && (enc2.group[t]))
        encsuccess = 0;
    }

    if (encsuccess)
      encsuccess = enc2.choiceresult;
    else
      encsuccess = 4; /***** default fail *****/
    goto out;
  }
out:
  SetCCursor(sword);
  DisposeDialog(win);
  updatefat(1, 0, 0);
  centerpict();
  return (encsuccess);
}
