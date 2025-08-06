#include "prototypes.h"
#include "variables.h"

/************************ save game *********************/
void save(short mode) {
  FILE* op = NULL;
  FILE* fp = NULL;
  short choice, count, t = 0;
  short temp;
  char hold[80];
  long testlocation;
  short n;

  t = CountResources('RLMZ') - 1;

  if (currentscenario < 20) {
    if ((currentscenario - 5 != t) && (t)) {
      warn(92);
      scratch(-501);
    }
  }

  /************ check to see if they have enough space *********/

  lowHD = FALSE;

  GetVInfo(0, myString, &temp, &templong);

  if (templong < 512000)
    lowHD = TRUE;

  if (!indung)
    saveland(landlevel);
  else
    saveland(dunglevel);

  if (inshop)
    saveshop(1, (Ptr) "");

  in();
  choice = fileprep(mode);
  centerpict();
  if (!choice)
    return;

  strcpy((StringPtr)hold, (StringPtr) ":Save:Game ");

  flashmessage((StringPtr) "Saving Game ~ Please Wait.", 30, 100, -1, 0);

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
  strcat((StringPtr)filename, "Data D1");

  if ((fp = MyrFopen(filename, "w+b")) == NULL)
    scratch2(12);
  if ((op = MyrFopen(":Data Files:DN", "rb")) == NULL)
    scratch(136);

  while (fread(&note, sizeof note, 1, op) == 1) {
    fwrite(&note, sizeof note, 1, fp);
  }
  fclose(op);
  fclose(fp);

  setfileinfo("save", filename);

  strcpy((StringPtr)filename, (StringPtr)hold);
  strcat((StringPtr)filename, "Data C1");
  if ((fp = MyrFopen(filename, "w+b")) == NULL)
    scratch(137);
  if ((op = MyrFopen(":Data Files:WN", "rb")) == NULL)
    scratch(138);

  while (fread(&note, sizeof note, 1, op) == 1) {
    fwrite(&note, sizeof note, 1, fp);
  }
  fclose(op);
  fclose(fp);

  setfileinfo("save", filename);

  strcpy((StringPtr)filename, (StringPtr)hold);
  strcat((StringPtr)filename, "Data TD3"); /**************** time encounters **************/
  if ((fp = MyrFopen(filename, "w+b")) == NULL)
    scratch(139);
  if ((op = MyrFopen(":Data Files:CTD3", "rb")) == NULL)
    scratch(140);

  while (fread(&dotime, sizeof dotime, 1, op) == 1) {
    fwrite(&dotime, sizeof dotime, 1, fp);
  }
  fclose(op);
  fclose(fp);

  setfileinfo("save", filename);

  strcpy((StringPtr)filename, (StringPtr)hold);
  strcat((StringPtr)filename, "Data H1"); /**************** thief **************/
  if ((fp = MyrFopen(filename, "w+b")) == NULL)
    scratch(141);
  if ((op = MyrFopen(":Data Files:CT", "rb")) == NULL)
    scratch(142);

  while (fread(&thief, sizeof thief, 1, op) == 1) {
    fwrite(&thief, sizeof thief, 1, fp);
  }
  fclose(op);
  fclose(fp);

  setfileinfo("save", filename);

  strcpy((StringPtr)filename, (StringPtr)hold); /**************** simple encounters **************/
  strcat((StringPtr)filename, "Data F1");
  if ((fp = MyrFopen(filename, "w+b")) == NULL)
    scratch(143);
  if ((op = MyrFopen(":Data Files:CE", "rb")) == NULL)
    scratch(144);

  while (fread(&enc, sizeof enc, 1, op) == 1) {
    for (t = 0; t < 4; t++)
      fread(&buffer[t], 80, 1, op);
    fwrite(&enc, sizeof enc, 1, fp);
    for (t = 0; t < 4; t++)
      fwrite(&buffer[t], 80, 1, fp);
  }
  fclose(op);
  fclose(fp);

  setfileinfo("save", filename);

  strcpy((StringPtr)filename, (StringPtr)hold); /**************** complex encounters **************/
  strcat((StringPtr)filename, "Data G1");
  if ((fp = MyrFopen(filename, "w+b")) == NULL)
    scratch(145);
  if ((op = MyrFopen(":Data Files:CE2", "rb")) == NULL)
    scratch(146);

  while (fread(&enc2, sizeof enc2, 1, op) == 1) {
    for (t = 0; t < 9; t++)
      fread(&buffer[t], 40, 1, op);
    fwrite(&enc2, sizeof enc2, 1, fp);
    for (t = 0; t < 9; t++)
      fwrite(&buffer[t], 40, 1, fp);
  }
  fclose(op);
  fclose(fp);

  setfileinfo("save", filename);

  /********************** save location so people dont hack position **********/
  testlocation = partyx + partyy + lookx + looky + floorx + floory + landlevel;

  for (t = 0; t <= charnum; t++) {
    setverify(t);
  }

  strcpy((StringPtr)filename, (StringPtr)hold);
  strcat((StringPtr)filename, "Data I1");
  if ((fp = MyrFopen(filename, "w+b")) == NULL)
    scratch(147);

// Fantasoft v7.0 Begin   I don't want the PC and Mac saved games to be compatible so I switched the save order of
// Some important data to keep them from being compatible
#ifdef PC
  CvtShortToPc(&currentscenario);
  fwrite(&currentscenario, sizeof currentscenario, 1, fp);
  CvtShortToPc(&currentscenario);

  CvtShortToPc(&howhard);
  fwrite(&howhard, sizeof howhard, 1, fp);
  CvtShortToPc(&howhard);

  CvtShortToPc(&fat);
  fwrite(&fat, sizeof fat, 1, fp);
  CvtShortToPc(&fat);

  CvtTabShortToPc(&partycondition, 10);
  fwrite(&partycondition, sizeof partycondition, 1, fp);
  CvtTabShortToPc(&partycondition, 10);

  CvtFloatToPc(&percent);
  fwrite(&percent, sizeof percent, 1, fp);
  CvtFloatToPc(&percent);

  CvtFloatToPc(&hardpercent);
  fwrite(&hardpercent, sizeof hardpercent, 1, fp);
  CvtFloatToPc(&hardpercent);

  CvtTabCharacterToPc(&c, 6);
  fwrite(&c, sizeof c, 1, fp);
  CvtTabCharacterToPc(&c, 6);
#else
  CvtTabCharacterToPc(&c, 6);
  fwrite(&c, sizeof c, 1, fp);
  CvtTabCharacterToPc(&c, 6);

  CvtShortToPc(&currentscenario);
  fwrite(&currentscenario, sizeof currentscenario, 1, fp);
  CvtShortToPc(&currentscenario);

  CvtShortToPc(&howhard);
  fwrite(&howhard, sizeof howhard, 1, fp);
  CvtShortToPc(&howhard);

  CvtShortToPc(&fat);
  fwrite(&fat, sizeof fat, 1, fp);
  CvtShortToPc(&fat);

  CvtTabShortToPc(&partycondition, 10);
  fwrite(&partycondition, sizeof partycondition, 1, fp);
  CvtTabShortToPc(&partycondition, 10);

  CvtFloatToPc(&percent);
  fwrite(&percent, sizeof percent, 1, fp);
  CvtFloatToPc(&percent);

  CvtFloatToPc(&hardpercent);
  fwrite(&hardpercent, sizeof hardpercent, 1, fp);
  CvtFloatToPc(&hardpercent);
#endif
  // Fantasoft v7.0 END

  fwrite(&deltax, sizeof(char), 1, fp);
  fwrite(&deltay, sizeof(char), 1, fp);
  fwrite(&scenarioname, sizeof(char), 256, fp);
  fwrite(&killparty, sizeof(char), 1, fp);
  fwrite(&charnum, sizeof(char), 1, fp);
  fwrite(&head, sizeof(char), 1, fp);
  fwrite(&currentshop, sizeof(char), 1, fp);
  fwrite(&quest, sizeof(char), 128, fp);
  fwrite(&commandkey, sizeof(char), 1, fp);
  fwrite(&cl, sizeof(char), 1, fp);
  fwrite(&cr, sizeof(char), 1, fp);
  fwrite(&charselectnew, sizeof(char), 1, fp);
  fwrite(&map, sizeof(char), 20, fp);
  fwrite(&inscroll, sizeof(char), 1, fp);
  fwrite(&indung, sizeof(char), 1, fp);
  fwrite(&view, sizeof(char), 1, fp);
  fwrite(&editon, sizeof(char), 1, fp);
  fwrite(&incamp, sizeof(char), 1, fp);
  fwrite(&initems, sizeof(char), 1, fp);
  fwrite(&inswap, sizeof(char), 1, fp);
  fwrite(&inbooty, sizeof(char), 1, fp);
  fwrite(&shopavail, sizeof(char), 1, fp);
  fwrite(&campavail, sizeof(char), 1, fp);
  fwrite(&intemple, sizeof(char), 1, fp);
  fwrite(&inshop, sizeof(char), 1, fp);
  fwrite(&swapavail, sizeof(char), 1, fp);
  fwrite(&templeavail, sizeof(char), 1, fp);
  fwrite(&tradeavail, sizeof(char), 1, fp);
  fwrite(&canshop, sizeof(char), 1, fp);
  fwrite(&shopequip, sizeof(char), 1, fp);
  fwrite(&lastcaste, sizeof(char), 1, fp);
  fwrite(lastspell, sizeof(char), 6 * 2, fp);
  fwrite(&combatround, sizeof(char), 1, fp);
  fwrite(&lastpix, sizeof(char), 1, fp); // Myriad : Dummy !!!!

  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * See note in main.c about sizeof(long) vs. sizeof(int32_t). */

  CvtLongToPc(&x);
  fwrite(&x, sizeof(int32_t), 1, fp);
  CvtLongToPc(&x);

  CvtLongToPc(&y);
  fwrite(&y, sizeof(int32_t), 1, fp);
  CvtLongToPc(&y);

  CvtLongToPc(&wallx);
  fwrite(&wallx, sizeof(int32_t), 1, fp);
  CvtLongToPc(&wallx);

  CvtLongToPc(&wally);
  fwrite(&wally, sizeof(int32_t), 1, fp);
  CvtLongToPc(&wally);

  CvtLongToPc(&dunglevel);
  fwrite(&dunglevel, sizeof(int32_t), 1, fp);
  CvtLongToPc(&dunglevel);

  CvtLongToPc(&partyx);
  fwrite(&partyx, sizeof(int32_t), 1, fp);
  CvtLongToPc(&partyx);

  CvtLongToPc(&partyy);
  fwrite(&partyy, sizeof(int32_t), 1, fp);
  CvtLongToPc(&partyy);

  CvtLongToPc(&reclevel);
  fwrite(&reclevel, sizeof(int32_t), 1, fp);
  CvtLongToPc(&reclevel);

  CvtLongToPc(&maxlevel);
  fwrite(&maxlevel, sizeof(int32_t), 1, fp);
  CvtLongToPc(&maxlevel);

  CvtLongToPc(&landlevel);
  fwrite(&landlevel, sizeof(int32_t), 1, fp);
  CvtLongToPc(&landlevel);

  CvtLongToPc(&lookx);
  fwrite(&lookx, sizeof(int32_t), 1, fp);
  CvtLongToPc(&lookx);

  CvtLongToPc(&looky);
  fwrite(&looky, sizeof(int32_t), 1, fp);
  CvtLongToPc(&looky);

  CvtLongToPc(&fieldx);
  fwrite(&fieldx, sizeof(int32_t), 1, fp);
  CvtLongToPc(&fieldx);

  CvtLongToPc(&fieldy);
  fwrite(&fieldy, sizeof(int32_t), 1, fp);
  CvtLongToPc(&fieldy);

  CvtLongToPc(&floorx);
  fwrite(&floorx, sizeof(int32_t), 1, fp);
  CvtLongToPc(&floorx);

  CvtLongToPc(&floory);
  fwrite(&floory, sizeof(int32_t), 1, fp);
  CvtLongToPc(&floory);

  CvtTabLongToPc(&moneypool, 3);
  fwrite(&moneypool, sizeof(int32_t), 3, fp);
  CvtTabLongToPc(&moneypool, 3);

  // Myriad The tm struct is 9xshort on macintosh
  n = tyme.tm_sec;
  CvtShortToPc(&n);
  fwrite(&n, sizeof(short), 1, fp);
  n = tyme.tm_min;
  CvtShortToPc(&n);
  fwrite(&n, sizeof(short), 1, fp);
  n = tyme.tm_hour;
  CvtShortToPc(&n);
  fwrite(&n, sizeof(short), 1, fp);
  n = tyme.tm_mday;
  CvtShortToPc(&n);
  fwrite(&n, sizeof(short), 1, fp);
  n = tyme.tm_mon;
  CvtShortToPc(&n);
  fwrite(&n, sizeof(short), 1, fp);
  n = tyme.tm_year;
  CvtShortToPc(&n);
  fwrite(&n, sizeof(short), 1, fp);
  n = tyme.tm_wday;
  CvtShortToPc(&n);
  fwrite(&n, sizeof(short), 1, fp);
  n = tyme.tm_yday;
  CvtShortToPc(&n);
  fwrite(&n, sizeof(short), 1, fp);
  n = tyme.tm_isdst;
  CvtShortToPc(&n);
  fwrite(&n, sizeof(short), 1, fp);

  // fwrite(&multiview,sizeof multiview,30,fp);   Fantasoft v7.1b illegal, broke up below

  fwrite(&multiview, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fwrite(&updatedir, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fwrite(&monsterset, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fwrite(&bankavailable, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side

  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * See note in main.c about sizeof(long) vs. sizeof(int32_t). */
  fwrite(&bank, sizeof(int32_t), 3, fp); //*** fantasoft v7.1b  broke it up so work on PC side

  fwrite(&templecost, sizeof(short), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fwrite(&inboat, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fwrite(&boatright, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fwrite(&canencounter, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fwrite(&xydisplayflag, sizeof(char), 1, fp); //*** fantasoft v7.1b  broke it up so work on PC side
  fwrite(&blanksavegamevariables, sizeof(char), 8, fp); //*** fantasoft v7.1b  broke it up so work on PC side

  CvtTabMonsterToPc(&holdover, 20);
  fwrite(&holdover, sizeof holdover, 1, fp);
  CvtTabMonsterToPc(&holdover, 20);

  CvtShortToPc(&heldover);
  fwrite(&heldover, sizeof heldover, 1, fp);
  CvtShortToPc(&heldover);

  CvtShortToPc(&deduction);
  fwrite(&deduction, sizeof deduction, 1, fp);
  CvtShortToPc(&deduction);

  CvtLongToPc(&showserial);
  fwrite(&showserial, sizeof showserial, 1, fp);
  CvtLongToPc(&showserial);

  fwrite(&canpriestturn, sizeof canpriestturn, 1, fp);

  CvtTabShortToPc(&musictoggle, 20);
  fwrite(&musictoggle, sizeof musictoggle, 1, fp);
  CvtTabShortToPc(&musictoggle, 20);

  fwrite(&doauto, sizeof doauto, 1, fp);

  CvtTabShortToPc(&definespells, 6 * 10 * 4);
  fwrite(&definespells, sizeof definespells, 1, fp);
  CvtTabShortToPc(&definespells, 6 * 10 * 4);

  fwrite(&notes, sizeof notes, 1, fp);

  CvtTabShortToPc(&cancamp, 10); // Myriad ????
  fwrite(&cancamp, sizeof cancamp, 10, fp);
  CvtTabShortToPc(&cancamp, 10); // Myriad ????

  CvtTabItemToPc(&storage, 6);
  fwrite(&storage, sizeof storage, 1, fp);
  CvtTabItemToPc(&storage, 6);

  CvtTabLongToPc(&wealthstore, 3);
  fwrite(&wealthstore, sizeof wealthstore, 1, fp);
  CvtTabLongToPc(&wealthstore, 3);

  fwrite(&registrationname, sizeof registrationname, 1, fp);

  CvtLongToPc(&testlocation);
  fwrite(&testlocation, sizeof testlocation, 1, fp);
  CvtLongToPc(&testlocation);

  fwrite(&spellcasting, sizeof(Boolean), 1, fp); //*** fantasoft v7.1b  Added as was missing in previous version.
  fwrite(&spellcharging, sizeof(Boolean), 1, fp); //*** fantasoft v7.1b  Added as was missing in previous version.
  fwrite(&monstercasting, sizeof(Boolean), 1, fp); //*** fantasoft v7.1b  Added as was missing in previous version.
  fwrite(&spareboolean, sizeof(Boolean), 1, fp); //*** fantasoft v7.1b  Added as was missing in previous version.

  fclose(fp);

  setfileinfo("save", filename);

  strcpy((StringPtr)filename, (StringPtr)hold); /******* dung ** dungeons ** dungeon  door data *************/
  strcat((StringPtr)filename, "Data B1");
  if ((op = MyrFopen(filename, "w+b")) == NULL)
    scratch(148);
  if ((fp = MyrFopen(":Data Files:CD", "rb")) == NULL)
    scratch(149);
  count = 0;
  while ((fread(&door, sizeof door, 1, fp) == 1) && (count++ < 20)) {
    fread(&field, sizeof field, 1, fp);
    fread(&randlevel, sizeof randlevel, 1, fp);
    fwrite(&door, sizeof door, 1, op);
    fwrite(&field, sizeof field, 1, op);
    fwrite(&randlevel, sizeof randlevel, 1, op);
  }

pushon:

  fclose(fp);
  fclose(op);

  setfileinfo("save", filename);

  strcpy((StringPtr)filename, (StringPtr)hold); /********** rand level **** land data ** door data ******/
  strcat((StringPtr)filename, "Data A1");
  if ((fp = MyrFopen(":Data Files:CL", "rb")) == NULL)
    scratch(150);
  if ((op = MyrFopen(filename, "w+b")) == NULL)
    scratch(151);
  while (fread(&door, sizeof door, 1, fp) == 1) {
    fread(&field, sizeof field, 1, fp);
    fread(&randlevel, sizeof randlevel, 1, fp);
    if (!indung)
      fread(&site, sizeof site, 1, fp);
    fwrite(&door, sizeof door, 1, op);
    fwrite(&field, sizeof field, 1, op);
    fwrite(&randlevel, sizeof randlevel, 1, op);
    if (!indung) {
      fwrite(&site, sizeof site, 1, op);
    }
  }
  fclose(fp);
  fclose(op);

  setfileinfo("save", filename);

  if (indung)
    loadland(dunglevel, 1);
  else
    loadland(landlevel, 0);

  strcpy((StringPtr)filename, (StringPtr)hold);
  strcat((StringPtr)filename, "Data E1");
  saveshop(2, filename); /*************** replace old saved game shop file ************/

  flashmessage((StringPtr) "", 30, 100, -1, 0);
  lowHD = FALSE;
}

/****************************** saveland ****************************/
void saveland(long id) {
  FILE* fp = NULL;
  if (id > 20)
    scratch2(8);
  if (indung) {
    if ((fp = MyrFopen(":Data Files:CD", "r+b")) == NULL)
      scratch(152);
  }

  /**************<<<<<<<<<< pirates will get ERROR 153B >>>>>>>>>>***************/

  else if ((fp = MyrFopen(":Data Files:CL", "r+b")) == NULL)
    scratch(153);

  if (!indung)
    fseek(fp, id * (sizeof field + sizeof door + sizeof randlevel + sizeof site), SEEK_SET);
  else
    fseek(fp, id * (sizeof field + sizeof door + sizeof randlevel), SEEK_SET);

  CvtTabDoorToPc(door, 100);
  fwrite(&door, sizeof door, 1, fp);
  CvtTabDoorToPc(door, 100);
  CvtFieldToPc(&field);
  fwrite(&field, sizeof field, 1, fp);
  CvtFieldToPc(&field);
  CvtRandLevelToPc(&randlevel);
  fwrite(&randlevel, sizeof randlevel, 1, fp);
  CvtRandLevelToPc(&randlevel);
  if (!indung)
    fwrite(&site, sizeof site, 1, fp);
  fclose(fp);
}

/************************* direction ***********************/
short direction(float dx, float dy) {
  short temp;
  if ((dx > 0) && (dy < 0))
    temp = 1;
  if ((dx > 0) && (dy == 0))
    temp = 2;
  if ((dx > 0) && (dy > 0))
    temp = 3;
  if ((dx == 0) && (dy > 0))
    temp = 4;
  if ((dx < 0) && (dy > 0))
    temp = 5;
  if ((dx < 0) && (dy == 0))
    temp = 6;
  if ((dx < 0) && (dy < 0))
    temp = 7;
  if ((dx == 0) && (dy < 0))
    temp = 0;
  return (temp);
}

/*************************** order *****************************/
void order(void) {
  struct character c1[6];
  short t, tt;
  textbox(3, 33, 0, 0, textrect);
  getchoice(charnum + 1, 1, 1);
  for (t = 0; t <= charnum; t++) {
    c1[t] = c[t];
    for (tt = 0; tt < 10; tt++)
      definespells[t][tt][0] = definespells[t][tt][1] = definespells[t][tt][2] = definespells[t][tt][3] = 0;
  }
  for (t = 0; t <= charnum; t++)
    c[track[t] - 1] = c1[t];

  for (tt = 0; tt <= charnum; tt++) {
    for (t = 0; t < 10; t++) {
      definespells[tt][t][0] = c[tt].definespells[t][0];
      definespells[tt][t][1] = c[tt].definespells[t][1];
      definespells[tt][t][2] = c[tt].definespells[t][2];
      definespells[tt][t][3] = c[tt].definespells[t][3];
    }
  }

  SetPort(GetWindowPort(screen));
  EraseRect(&textrect);
  shortupdate(0);
  updatecontrols();
}

/************** seeshop *****************************************/
short seeshop(short mode) {
  WindowPtr whichWindow;
  short a;

  if (bankavailable) {
    moneypool[0] += bank[0];
    moneypool[1] += bank[1];
    moneypool[2] += bank[2];
    bank[0] = bank[1] = bank[2] = 0;
  }
  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(danapplegate): This function clearly expects itemswindow to be initialized
   * as it calls Showitems, which calls quickinfo, but it doesn't seem like it's been
   * initialized by this point. Adding initialization here and cleanup below.
   */
  itemswindow = GetNewWindow(129, 0L, (WindowPtr)-1L);

  if (!mode)
    Showitems(0); /**** initialize ******/
  SetPort(GetWindowPort(gshop));

  for (;;) {
    switch (didattack) {
      case 1:
        ploticon3(130, buttonrect);
        break;

      case 4:
        itemRect.top = 0;
        itemRect.bottom = 387;
        itemRect.left = 263 + (leftshift / 2);
        itemRect.right = itemRect.left + 114;

        pict(170, itemRect);

        HideControl(charitemsvert);
        HideControl(shopitemsvert);
        ShowControl(charitemsvert);
        ShowControl(shopitemsvert);
        break;
    }
    didattack = 0;

    if (gTheEvent.modifiers & alphaLock)
      warn(21);

    SystemTask();
    a = GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
    DoCorrectBugMADRepeat();
#endif
    MyrCheckMemory(2);

    if (IsDialogEvent(&gTheEvent)) {
      a = DialogSelect(&gTheEvent, &dummy, &itemHit);
    } else {

      switch (gTheEvent.what) {
        case (updateEvt):
          BeginUpdate(gshop);
          EndUpdate(gshop);
          if (mat) {
            BeginUpdate(mat);
            EndUpdate(mat);
          }
          break;

        case (mouseDown):

          thePart = FindWindow(gTheEvent.where, &whichWindow);
          switch (thePart) {
            case inMenuBar:
              compactheap();
              menuChoice = MenuSelect(gTheEvent.where);
              HiliteMenu(0);
              break;

            case inContent:
              reply = choice(0);
              if (reply) {
                DisposeWindow(itemswindow);
                itemswindow = NIL;
                return (reply);
              }
              break;
          }
          break;

        case (activateEvt):
          break;

        case (mouseUp):
          break;

        case (diskEvt):
          if (HiWord(gTheEvent.message) != noErr) /* if MountVol had err */
          { /* then initialize disk */
            SysBeep(NIL); /* beep */
            SetPt(&point, 30, 40); /* position */
            DIBadMount(point, gTheEvent.message); /* initialize */
          }
          break;

        case (networkEvt):
          break;

        case (driverEvt):
          break;

        case (app1Evt):
          break;

        case (app2Evt):
          break;

        case (app3Evt):
          break;

        case (keyUp):
          break;

        case keyDown:
          t = gTheEvent.message;
          theControl = NIL;
          reply = choice(0);
          if (reply) {
            DisposeWindow(itemswindow);
            itemswindow = NIL;
            return (reply);
          }
          break;

        case (autoKey):
          break;

        case osEvt:
          if ((gTheEvent.message >> 24) == suspendResumeMessage) {
            if (hide) {
              compactheap();
              hide = FALSE;
              Showitems(1);
              FlushEvents(everyEvent, 0);
            } else
              hide = TRUE;
          }
          break;
      }
    }
  }

  DisposeWindow(itemswindow);
  itemswindow = NIL;
  /* *** END CHANGES *** */
}
