#include "prototypes.h"
#include "realmzbuild.h"
#include "variables.h"

/************************ load *********************/
short load(void) {
  FILE* op = NULL;
  FILE* fp = NULL;
  long savedserial = 0;
  long testlocation, templong;
  WindowPtr splash = NIL;
  short t, choice, count;
  short tempVolRef;
  PicHandle picturehandle;
  short n;
  char bigbadbug; // Myriad

  Boolean showing = FALSE;
  char hold[256];

  strcpy((StringPtr)hold, (StringPtr) ":Save:Game ");

  choice = fileprep(1);
  if (background) {
    DrawDialog(background);
    gCurrent = background;
  }

  if (!choice)
    return (0);

  lowHD = FALSE;

  GetVInfo(0, myString, &tempVolRef, &templong);

  if (templong < 512000)
    lowHD = TRUE;

  switch (choice) {
    case 4:
      strcat((StringPtr)hold, (StringPtr) "A:");
      break;
    case 5:
      strcat((StringPtr)hold, (StringPtr) "B:");
      break;
    case 6:
      strcat((StringPtr)hold, (StringPtr) "C:");
      break;
    case 7:
      strcat((StringPtr)hold, (StringPtr) "D:");
      break;
    case 8:
      strcat((StringPtr)hold, (StringPtr) "E:");
      break;
    case 9:
      strcat((StringPtr)hold, (StringPtr) "F:");
      break;
    case 10:
      strcat((StringPtr)hold, (StringPtr) "G:");
      break;
    case 11:
      strcat((StringPtr)hold, (StringPtr) "H:");
      break;
    case 12:
      strcat((StringPtr)hold, (StringPtr) "I:");
      break;
    case 13:
      strcat((StringPtr)hold, (StringPtr) "J:");
      break;
  }
  strcpy((StringPtr)filename, (StringPtr)hold);
  strcat((StringPtr)filename, "Data I1");
  if ((fp = MyrFopen(filename, "rb")) == NULL) {
    warn(11);
    return (0);
  }

  fclose(fp);

  if (look == NIL) {
    gCurrent = background;
    SetPortDialogPort(background);
    TextFont(font);
    TextSize(32);
    BackColor(blackColor);
    ForeColor(yellowColor);
    MyrCDiStr(2, (StringPtr) "Loading......Please wait.");
  } else {
    if (!incombat)
      centerpict();
    else
      centerfield(5 + (2 * screensize), 5 + screensize);
    flashmessage((StringPtr) "Reverting to a previously saved game, please wait...", 25, 350 + downshift, -1, 0);
    showing = TRUE;
    partyloss(TRUE);
  }

  if (currentscenario < 7)
    currentscenario = 10;

  CheckItem(gGame, currentscenario, FALSE);

  heldover = 0;

  strcpy((StringPtr)filename, (StringPtr)hold);
  strcat((StringPtr)filename, "Data I1");
  if ((fp = MyrFopen(filename, "rb")) == NULL) {
    warn(11);
    return (0);
  }
  moneypool[0] = moneypool[1] = moneypool[2] = 0;

// Fantasoft v7.0 Begin   I don't want the PC and Mac saved games to be compatible so I switched the save order of
// Some important data to keep them from being compatible
#ifdef PC
  fread(&currentscenario, sizeof currentscenario, 1, fp);
  CvtShortToPc(&currentscenario);

  fread(&howhard, sizeof howhard, 1, fp);
  CvtShortToPc(&howhard);

  fread(&fat, sizeof fat, 1, fp);
  CvtShortToPc(&fat);

  fread(&partycondition, sizeof partycondition, 1, fp);
  CvtTabShortToPc(&partycondition, 10);

  fread(&percent, sizeof percent, 1, fp);
  CvtFloatToPc(&percent);

  fread(&hardpercent, sizeof hardpercent, 1, fp);
  CvtFloatToPc(&hardpercent);

  fread(&c, sizeof c, 1, fp);
  CvtTabCharacterToPc(&c, 6);
#else
  fread(&c, sizeof c, 1, fp);
  CvtTabCharacterToPc(&c, 6);

  fread(&currentscenario, sizeof currentscenario, 1, fp);
  CvtShortToPc(&currentscenario);

  fread(&howhard, sizeof howhard, 1, fp);
  CvtShortToPc(&howhard);

  fread(&fat, sizeof fat, 1, fp);
  CvtShortToPc(&fat);

  fread(&partycondition, sizeof partycondition, 1, fp);
  CvtTabShortToPc(&partycondition, 10);

  fread(&percent, sizeof percent, 1, fp);
  CvtFloatToPc(&percent);

  fread(&hardpercent, sizeof hardpercent, 1, fp);
  CvtFloatToPc(&hardpercent);
#endif
  // Fantasoft v7.0 END

  // fread(&deltax,sizeof(char),446,fp);

  fread(&deltax, sizeof(char), 1, fp);
  fread(&deltay, sizeof(char), 1, fp);
  fread(&scenarioname, sizeof(char), 256, fp);
  fread(&killparty, sizeof(char), 1, fp);
  fread(&charnum, sizeof(char), 1, fp);
  fread(&head, sizeof(char), 1, fp);
  fread(&currentshop, sizeof(char), 1, fp);
  fread(&quest, sizeof(char), 128, fp);
  fread(&commandkey, sizeof(char), 1, fp);
  fread(&cl, sizeof(char), 1, fp);
  fread(&cr, sizeof(char), 1, fp);
  fread(&charselectnew, sizeof(char), 1, fp);
  fread(&map, sizeof(char), 20, fp);
  fread(&inscroll, sizeof(char), 1, fp);
  fread(&indung, sizeof(char), 1, fp);
  fread(&view, sizeof(char), 1, fp);
  fread(&editon, sizeof(char), 1, fp);
  fread(&incamp, sizeof(char), 1, fp);
  fread(&initems, sizeof(char), 1, fp);
  fread(&inswap, sizeof(char), 1, fp);
  fread(&inbooty, sizeof(char), 1, fp);
  fread(&shopavail, sizeof(char), 1, fp);
  fread(&campavail, sizeof(char), 1, fp);
  fread(&intemple, sizeof(char), 1, fp);
  fread(&inshop, sizeof(char), 1, fp);
  fread(&swapavail, sizeof(char), 1, fp);
  fread(&templeavail, sizeof(char), 1, fp);
  fread(&tradeavail, sizeof(char), 1, fp);
  fread(&canshop, sizeof(char), 1, fp);
  fread(&shopequip, sizeof(char), 1, fp);
  fread(&lastcaste, sizeof(char), 1, fp);
  fread(lastspell, sizeof(char), 6 * 2, fp);
  fread(&combatround, sizeof(char), 1, fp);
  fread(&bigbadbug, sizeof(char), 1, fp); // Myriad : illegal access

  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * See note in main.c about sizeof(long) vs. sizeof(int32_t). */

  fread(&x, sizeof(int32_t), 1, fp);
  CvtLongToPc(&x);

  fread(&y, sizeof(int32_t), 1, fp);
  CvtLongToPc(&y);

  fread(&wallx, sizeof(int32_t), 1, fp);
  CvtLongToPc(&wallx);

  fread(&wally, sizeof(int32_t), 1, fp);
  CvtLongToPc(&wally);

  fread(&dunglevel, sizeof(int32_t), 1, fp);
  CvtLongToPc(&dunglevel);

  fread(&partyx, sizeof(int32_t), 1, fp);
  CvtLongToPc(&partyx);

  fread(&partyy, sizeof(int32_t), 1, fp);
  CvtLongToPc(&partyy);

  fread(&reclevel, sizeof(int32_t), 1, fp);
  CvtLongToPc(&reclevel);

  fread(&maxlevel, sizeof(int32_t), 1, fp);
  CvtLongToPc(&maxlevel);

  fread(&landlevel, sizeof(int32_t), 1, fp);
  CvtLongToPc(&landlevel);

  fread(&lookx, sizeof(int32_t), 1, fp);
  CvtLongToPc(&lookx);

  fread(&looky, sizeof(int32_t), 1, fp);
  CvtLongToPc(&looky);

  fread(&fieldx, sizeof(int32_t), 1, fp);
  CvtLongToPc(&fieldx);

  fread(&fieldy, sizeof(int32_t), 1, fp);
  CvtLongToPc(&fieldy);

  fread(&floorx, sizeof(int32_t), 1, fp);
  CvtLongToPc(&floorx);

  fread(&floory, sizeof(int32_t), 1, fp);
  CvtLongToPc(&floory);

  fread(&moneypool, sizeof(int32_t), 3, fp);
  CvtTabLongToPc(&moneypool, 3);

  // Myriad The tm struct is 9xshort on macintosh
  fread(&n, sizeof(short), 1, fp);
  CvtShortToPc(&n);
  tyme.tm_sec = n;
  fread(&n, sizeof(short), 1, fp);
  CvtShortToPc(&n);
  tyme.tm_min = n;
  fread(&n, sizeof(short), 1, fp);
  CvtShortToPc(&n);
  tyme.tm_hour = n;
  fread(&n, sizeof(short), 1, fp);
  CvtShortToPc(&n);
  tyme.tm_mday = n;
  fread(&n, sizeof(short), 1, fp);
  CvtShortToPc(&n);
  tyme.tm_mon = n;
  fread(&n, sizeof(short), 1, fp);
  CvtShortToPc(&n);
  tyme.tm_year = n;
  fread(&n, sizeof(short), 1, fp);
  CvtShortToPc(&n);
  tyme.tm_wday = n;
  fread(&n, sizeof(short), 1, fp);
  CvtShortToPc(&n);
  tyme.tm_yday = n;
  fread(&n, sizeof(short), 1, fp);
  CvtShortToPc(&n);
  tyme.tm_isdst = n;

  // fread(&multiview,sizeof multiview,30,fp);//*** fantasoft v7.1b  illegal on PC side

  fread(&multiview, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fread(&updatedir, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fread(&monsterset, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fread(&bankavailable, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side

  fread(&bank, sizeof(int32_t), 3, fp); //*** fantasoft v7.1b  broke it up so work on PC side

  fread(&templecost, sizeof(short), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fread(&inboat, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fread(&boatright, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fread(&canencounter, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fread(&xydisplayflag, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fread(&blanksavegamevariables, sizeof(char), 8, fp); //*** fantasoft v7.1b  broke it up so work on PC side

  fread(&holdover, sizeof holdover, 1, fp);
  CvtTabMonsterToPc(&holdover, 20);

  fread(&heldover, sizeof heldover, 1, fp);
  CvtShortToPc(&heldover);

  fread(&deduction, sizeof deduction, 1, fp);
  CvtShortToPc(&deduction);

  fread(&savedserial, sizeof savedserial, 1, fp);
  CvtLongToPc(&savedserial);

  fread(&canpriestturn, sizeof canpriestturn, 1, fp);

  fread(&musictoggle, sizeof musictoggle, 1, fp);
  CvtTabShortToPc(&musictoggle, 20);

  fread(&doauto, sizeof doauto, 1, fp);

  fread(&definespells, sizeof definespells, 1, fp);
  CvtTabShortToPc(&definespells, 6 * 10 * 4);

  fread(&notes, sizeof notes, 1, fp);

  fread(&cancamp, sizeof cancamp, 10, fp);
  CvtTabShortToPc(&cancamp, 10); // Myriad ????

  fread(&storage, sizeof storage, 1, fp);
  CvtTabItemToPc(&storage, 6);

  fread(&wealthstore, sizeof wealthstore, 1, fp);
  CvtTabLongToPc(&wealthstore, 3);

  fread(&registrationname, sizeof registrationname, 1, fp);
  fread(&testlocation, sizeof testlocation, 1, fp);
  CvtLongToPc(&testlocation);

  fwrite(&spellcasting, sizeof(Boolean), 1, fp); //*** fantasoft v7.1b  Added as was missing in previous version.
  fwrite(&spellcharging, sizeof(Boolean), 1, fp); //*** fantasoft v7.1b  Added as was missing in previous version.
  fwrite(&monstercasting, sizeof(Boolean), 1, fp); //*** fantasoft v7.1b  Added as was missing in previous version.
  fwrite(&spareboolean, sizeof(Boolean), 1, fp); //*** fantasoft v7.1b  Added as was missing in previous version.

  fclose(fp);

#if !winversion /******* corrects for old style mac format ****/
  for (t = 0; t <= charnum; t++) {
    characterl = c[t];
    checkforerrors();
    c[t] = characterl;
  }
#endif

#if development
  showserial = savedserial;
#endif

#if !divine
  if (testlocation != (partyx + partyy + lookx + looky + floorx + floory + landlevel))
    scratch2(6);
#endif

  GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
  DoCorrectBugMADRepeat();
#endif
  MyrCheckMemory(2);

#if !divine
  if (!doreg()) /*** check for PCs too high *****/
  {
    for (t = 0; t <= charnum; t++)
      if (c[t].level > 7)
        scratch(456);
  }
#endif

  for (t = 0; t < 20; t++) {
    switch (musictoggle[t]) {
      case 0:
        SetItemMark(musicmenu, t + 8, 0); //** do not play music in this area
        break;

      case 1:
        SetItemMark(musicmenu, t + 8, 19); //** play music in this area
        break;

      case 2:
        SetItemMark(musicmenu, t + 8, -41); //** Do not switch music in this area but keep playing current title
        break;
    }
  }

  viewtype = 1;

  if ((!doreg()) && (currentscenario > 10)) {
  needtoregister:

    CheckItem(gGame, 10, TRUE);
    currentscenario = 10;
    strcpy((StringPtr)scenarioname, (StringPtr) ":Scenarios:City of Bywater:");
    if (showing)
      flashmessage((StringPtr) "", 10, 10, -1, 0);
    warn(65);
    return (0);
  }

  GetMenuItemText(gGame, currentscenario, myString);
  PtoCstr(myString);
  strcpy((StringPtr)filename, (StringPtr)myString);

  if (!selectscenario((Ptr)myString, FALSE)) {

    /**** may be old format saved game, try shifting 5 and try again ****/
    /**** Not used in win version but check should not cause any problems either ****/
    currentscenario += 5;
    GetMenuItemText(gGame, currentscenario, myString);
    PtoCstr(myString);
    strcpy((StringPtr)filename, myString);

    if (!selectscenario((Ptr)myString, FALSE)) {
      CheckItem(gGame, 10, TRUE);
      strcpy((StringPtr)scenarioname, (StringPtr) ":Scenarios:City of Bywater:");
      currentscenario = 10;
      if (showing)
        flashmessage((StringPtr) "", 10, 10, -1, 0);
      warn(69);
      if (background != NIL)
        return (0);
      else
        quitgame();
    }
  }

  t = CountResources('RLMZ') - 1;

  if (currentscenario < topfantasoftsceanrio) // Fantasoft v7.1  changed it to use topfantasfotscenario varialbe
  {
    if ((currentscenario - 5 != t) && (t)) {
      warn(92);
      scratch(-502);
    }
  }

  picturehandle = NIL;
  picturehandle = GetPicture(32128);
  if (picturehandle) {
    splash = GetNewWindow(131, NIL, (WindowPtr)-1L);

    SizeWindow(splash, 320, 320, 0);

    if (look == NIL)
      MoveWindow(splash, GlobalLeft + 160, GlobalTop + 70, FALSE);
    else
      MoveWindow(splash, GlobalLeft + (leftshift / 2), GlobalTop + (downshift / 2), FALSE);

    ShowWindow(splash);

    itemRect.top = itemRect.left = 0;
    itemRect.right = itemRect.bottom = 320;

    SetPort(GetWindowPort(splash));
    DrawPicture(picturehandle, &itemRect);
  }

  CheckItem(gGame, currentscenario, TRUE);

  killparty = 0;
  for (t = 0; t <= charnum; t++)
    if ((c[t].stamina < 1) || (c[t].condition[25]))
      killparty++;

  if (partycondition[0])
    loaddark((partycondition[0] / 30) + 1);
  else
    loaddark(0);

  strcpy((StringPtr)filename, (StringPtr)hold); /******************** thief ******************/
  strcat((StringPtr)filename, "Data H1");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(66);
  op = NIL;
  if ((op = MyrFopen(":Data Files:CT", "w+b")) == NULL)
    scratch(67);

  while (fread(&thief, sizeof thief, 1, fp) == 1) {
    if (!fwrite(&thief, sizeof thief, 1, op))
      scratch(68);
  }
  fclose(op);
  fclose(fp);
  strcpy((StringPtr)filename, (StringPtr)hold); /******************** time encounters ******************/
  strcat((StringPtr)filename, "Data TD3");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(69);
  if ((op = MyrFopen(":Data Files:CTD3", "w+b")) == NULL)
    scratch(70);

  while (fread(&dotime, sizeof dotime, 1, fp) == 1) {
    if (!fwrite(&dotime, sizeof dotime, 1, op))
      scratch(71);
  }
  fclose(op);
  fclose(fp);

  strcpy((StringPtr)filename, (StringPtr)hold); /******************** simple encounters ******************/
  strcat((StringPtr)filename, "Data F1");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(72);
  if ((op = MyrFopen(":Data Files:CE", "w+b")) == NULL)
    scratch(73);

  while (fread(&enc, sizeof enc, 1, fp) == 1) {
    for (t = 0; t < 4; t++)
      fread(&buffer[t], 80, 1, fp);
    if (!fwrite(&enc, sizeof enc, 1, op))
      scratch(74);
    for (t = 0; t < 4; t++)
      if (!fwrite(&buffer[t], 80, 1, op))
        scratch(75);
  }
  fclose(op);
  fclose(fp);

  strcpy((StringPtr)filename, (StringPtr)hold); /******************** complex encounters ******************/
  strcat((StringPtr)filename, "Data G1");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(76);
  if ((op = MyrFopen(":Data Files:CE2", "w+b")) == NULL)
    scratch(77);

  while (fread(&enc2, sizeof enc2, 1, fp) == 1) {
    for (t = 0; t < 9; t++)
      fread(&buffer[t], 40, 1, fp);
    if (!fwrite(&enc2, sizeof enc2, 1, op))
      scratch(78);
    for (t = 0; t < 9; t++)
      if (!fwrite(&buffer[t], 40, 1, op))
        scratch(79);
  }
  fclose(op);
  fclose(fp);

  /**************************** Notes ******************/

  strcpy((StringPtr)filename, (StringPtr)hold);
  strcat((StringPtr)filename, "Data D1");
  if ((fp = MyrFopen(filename, "rb")) == NULL) {
    warn(129);
    goto skipnotes;
  }

  if ((op = MyrFopen(":Data Files:DN", "w+b")) == NULL)
    scratch(81);

  while (fread(&note, sizeof note, 1, fp) == 1) {
    if (!fwrite(&note, sizeof note, 1, op))
      scratch(82);
  }
  fclose(op);
  fclose(fp);

  strcpy((StringPtr)filename, (StringPtr)hold);
  strcat((StringPtr)filename, "Data C1");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(83);
  if ((op = MyrFopen(":Data Files:WN", "w+b")) == NULL)
    scratch(84);

  while (fread(&note, sizeof note, 1, fp) == 1) {
    if (!fwrite(&note, sizeof note, 1, op))
      scratch(85);
  }
  fclose(op);
  fclose(fp);

skipnotes:

  count = 0;

  strcpy((StringPtr)filename, (StringPtr)hold); /******* dung ** dungeons ** dungeon  door data *************/
  strcat((StringPtr)filename, "Data B1");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(86);
  if ((op = MyrFopen(":Data Files:CD", "w+b")) == NULL)
    scratch(87);
  while ((fread(&door, sizeof door, 1, fp) == 1) && (count < 20)) {
    count++;

    fread(&field, sizeof field, 1, fp);
    fread(&randlevel, sizeof randlevel, 1, fp);
    if (!fwrite(&door, sizeof door, 1, op))
      scratch(88);
    if (!fwrite(&field, sizeof field, 1, op))
      scratch(89);
    if (!fwrite(&randlevel, sizeof randlevel, 1, op))
      scratch(90);
  }
  fclose(fp);
  fclose(op);

  count = 0;

  strcpy((StringPtr)filename, (StringPtr)hold); /********** rand level **** land data ** door data ******/
  strcat((StringPtr)filename, "Data A1");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(91);
  if ((op = MyrFopen(":Data Files:CL", "w+b")) == NULL)
    scratch(92);
  while ((fread(&door, sizeof door, 1, fp) == 1) && (count < 20)) {
    count++;

    fread(&field, sizeof field, 1, fp);
    fread(&randlevel, sizeof randlevel, 1, fp);
    fread(&site, sizeof site, 1, fp);
    if (!fwrite(&door, sizeof door, 1, op))
      scratch(93);
    if (!fwrite(&field, sizeof field, 1, op))
      scratch(94);
    if (!fwrite(&randlevel, sizeof randlevel, 1, op))
      scratch(95);
    if (!fwrite(&site, sizeof site, 1, op))
      scratch(27);
  }
  fclose(fp);
  fclose(op);

  /************ restore saved shop ***************/
  strcpy((StringPtr)filename, (StringPtr)hold);
  strcat((StringPtr)filename, "Data E1");

  if ((op = MyrFopen(":Data Files:CS", "w+b")) == NULL)
    scratch(96);
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(97);

  while (fread(&theshop, sizeof theshop, 1, fp)) {
    if (!fwrite(&theshop, sizeof theshop, 1, op))
      scratch(98);
  }
  fclose(fp);
  fclose(op);

  lowHD = FALSE;

  charselectold = charselectnew = 0;

  if (indung)
    loadland(dunglevel, 1);
  else
    loadland(landlevel, 1);

  if (shopavail)
    loadshop(0);

  updatemonstermenu(currentscenario);
  updatemapmenu();
  updatenpcmenu();

  if (currentscenario > topfantasoftsceanrio) // Fantasoft v7.1  changed it to use topfantasfotscenario variable
  {
#if !development /************ display any contact info if needed to register *******/

    getfilename("Data CS"); /**** get codesegs from backup for use in demixing. ******/
    if ((fp = MyrFopen(filename, "rb")) == NULL)
      scratch(700);
    fseek(fp, 5 * sizeof reclevel, SEEK_SET);
    fread(&codeseg3, 20, 1, fp);
    fread(&codeseg4, 20, 1, fp);
    fclose(fp);

#if divine
    if (!usercheck2()) /********* dont bug them about thier own scenarios ****************/
    {
      if ((strlen(codeseg3)) || (strlen(codeseg4))) /********** requires registration *********/
      {
        if (quest[127] != 66) /********** is it already register? ************/
        {
          delay(120); /**** allow a few secs for splash screen ******/
          showcontactinfo();
        }
      }
    }
#else

    if ((strlen(codeseg3)) || (strlen(codeseg4))) /********** requires registration *********/
    {
      if (quest[127] != 66) /********** is it already register? ************/
      {
        delay(120); /**** allow a few secs for splash screen ******/
        showcontactinfo();
      }
    }
#endif
#endif
  }

  if (look == NIL) {
    SetPortDialogPort(background);
    BackColor(blackColor);
    ForeColor(yellowColor);
    gCurrent = background;
    MyrCDiStr(2, (StringPtr) "");
    BackColor(whiteColor);
    ForeColor(blackColor);

    if ((showing) && (background))
      flashmessage((StringPtr) "", 10, 10, -1, 0);
  } else if (showing)
    flashmessage((StringPtr) "", 25, 350, -1, 0);

  bandaid();

  if (splash) {
    DisposeWindow(splash);
    splash = NIL;
  }
  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(danapplegate): The gOptions menu resource (MENU 139) is not present in any version
   * of Realmz that we can find. It's possible that it was present in an early version of
   * the game, but the resource was removed before release and this code was not removed.
   * Rather than force our implementation of Menu Manager to handle this NULL case, we've
   * opted to remove any callsites which pass gOptions as a parameter.
   */
  // #if !development
  // #if divine
  //   if (!usercheck2())
  //     DisableItem(gOptions, 0);
  //   else
  //     EnableItem(gOptions, 0);
  // #endif
  // #endif
  /* *** END CHANGES *** */
  return (1);
}

/********************************** loadshop ******************/
void loadshop(short mode) {
  FILE* fp = NULL;

  if (mode)
    currentshop = mode;

  if ((fp = MyrFopen(":Data Files:CS", "rb")) == NULL)
    scratch(99);
  fseek(fp, currentshop * sizeof theshop, SEEK_SET);
  fread(&theshop, sizeof theshop, 1, fp);
  CvtShopToPc(&theshop);

  fclose(fp);
  if (!mode)
    shopavail = TRUE;
  if ((look) && (!mode)) {
    updatecontrols();
    sound(30005);
  }
}

/*********************** saveshop *********************/
void saveshop(short mode, char filename[256]) {
  FILE* fp = NULL;
  FILE* op;

  /****

   Mode:

     1 = update current game shops;
     2 = update saved game shops;
     3 = replace current game shop with saved shop/ resume saved game;****/

  if (mode == 1) /********** update current shop ***********/
  {
    if ((fp = MyrFopen(":Data Files:CS", "r+b")) == NULL)
      scratch(100);
    fseek(fp, currentshop * sizeof theshop, SEEK_SET);
    CvtShopToPc(&theshop);
    fwrite(&theshop, sizeof theshop, 1, fp);
    CvtShopToPc(&theshop);
    fclose(fp);
  }

  if (mode == 2) /*********** save game ******************/
  {
    if ((fp = MyrFopen(":Data Files:CS", "rb")) == NULL)
      scratch(101);
    if ((op = MyrFopen(filename, "w+b")) == NULL)
      scratch(102);
    while (fread(&theshop, sizeof theshop, 1, fp)) {
      if (!fwrite(&theshop, sizeof theshop, 1, op))
        scratch(103);
    }
    fclose(fp);
    fclose(op);

    setfileinfo("save", filename);

    if (shopavail)
      loadshop(0);
  }
}
