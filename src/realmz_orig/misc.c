#include "prototypes.h"
#include "realmzbuild.h"
#include "variables.h"

/****************************** doreg5 **********************************/
short doreg5(void) {
#if development
  if (divineref)
    return (1);
  else
    return (0);
#else
  if ((MyrBitTstLong(&serial, 6 + divine)) || (!serial))
    return (FALSE);
  else if ((!MyrBitTstLong(&serial, 6 + divine)) && (serial))
    return (TRUE);
  exit(0);
  return (0);
#endif
}

/****************************** doreg4 **********************************/
short doreg4(void) {
#if development
  if (divineref)
    return (1);
  else
    return (0);
#else
  if ((MyrBitTstLong(&serial, 6 + divine)) || (!serial))
    return (FALSE);
  else if ((!MyrBitTstLong(&serial, 6 + divine)) && (serial))
    return (TRUE);
  exit(0);
#endif
  return (NIL);
}

/******************************** drawspell **************************/
void drawspell(void) {
  if ((spellinfo.targettype == 5) || (!spellrange))
    return;
  centerfield(5 + (2 * screensize), 5 + screensize); // Fantasoft 7.1
  SetPort(GetWindowPort(look));
  MoveTo(pos[charup][0] * 32 + 16, pos[charup][1] * 32 + 16);
  LineTo(oldspellx * 32 + 16, oldspelly * 32 + 16);
  MyrCopyScreen((CGrafPtr)gmaps, (CGrafPtr)look, &bigspellrect, &showspell, 1); // Fantasoft 7.1
}

/************************ movecalc ******************************/
void movecalc(short charselect) {
  float temp, top, bottom;
  short movement;

  struct character character;
  switch (charselect) {
    case -1:
      character = characterl;
      break;

    case -2:
      character = characterr;
      break;

    default:
      character = c[charselect];
      break;
  }

  movement = racebasemove[character.race - 1];

  if (character.load < 0)
    character.load = 0;

  character.loadmax = (character.st + character.magst) * (character.st + character.magst) * 20;
  if (character.loadmax < 500)
    character.loadmax = 500;
  top = character.load;
  bottom = character.loadmax;
  temp = 1 + (((bottom - top) / bottom) * movement);
  if (character.condition[2])
    temp -= character.condition[2]; /**** tangled ***/
  if (character.condition[6])
    temp /= 2; /**** slow ****/
  if (character.condition[23])
    temp *= 2; /******* haste *****/
  if (temp < 2)
    temp = 2;
  temp += character.movebonus;
  character.movementmax = temp;

  switch (charselect) {
    case -1:
      characterl = character;
      break;

    case -2:
      characterr = character;
      break;

    default:
      c[charselect] = character;
      break;
  }
}

/********************* mainscreeninit ***********************/
void mainscreeninit(short mode, short dostart) {
  Rect r;
  EnableItem(gBeast, 0);

  if (!mode) {
    if (screen != NIL)
      DisposeWindow(screen);
    screen = GetNewWindow(128, NIL, (WindowPtr)-1L);
    MoveWindow(screen, GlobalLeft, GlobalTop, FALSE);
    SetPort(GetWindowPort(screen));
    BackPixPat(base);
    ErasePortRect();
    TextMode(1);
    TextFace(bold);
    TextSize(18);
    TextFont(font);

    /***** ShowWindow(screen);
    ShowWindow(gWindow);
    ShowWindow(look);

        BringToFront(screen);
        BringToFront(look);
        BringToFront(gWindow);*****/

    rest = GetNewControl(160, screen);
    GetControlBounds(rest, &r);
    MoveControl(rest, r.left + leftshift, r.top + downshift);

    search = GetNewControl(142, screen);
    GetControlBounds(search, &r);
    MoveControl(search, r.left + leftshift, r.top + downshift);

    movelook = GetNewControl(1000 + screensize, screen);

    swapbut = GetNewControl(157, screen);
    GetControlBounds(swapbut, &r);
    MoveControl(swapbut, r.left + leftshift, r.top + downshift);

    campbut = GetNewControl(154, screen);
    GetControlBounds(campbut, &r);
    MoveControl(campbut, r.left + leftshift, r.top + downshift);

    itemsbut = GetNewControl(155, screen);
    GetControlBounds(itemsbut, &r);
    MoveControl(itemsbut, r.left + leftshift, r.top + downshift);

    tradebut = GetNewControl(156, screen);
    GetControlBounds(tradebut, &r);
    MoveControl(tradebut, r.left + leftshift, r.top + downshift);

    barbut = GetNewControl(172, screen);
    GetControlBounds(barbut, &r);
    MoveControl(barbut, r.left + leftshift, r.top + downshift);

    shopbut = GetNewControl(158, screen);
    GetControlBounds(shopbut, &r);
    MoveControl(shopbut, r.left + leftshift, r.top + downshift);

    viewspellsbut = GetNewControl(451, screen);
    GetControlBounds(viewspellsbut, &r);
    MoveControl(viewspellsbut, r.left + leftshift, r.top + downshift);

    castspellsbut = GetNewControl(450, screen);
    GetControlBounds(castspellsbut, &r);
    MoveControl(castspellsbut, r.left + leftshift, r.top + downshift);

    overviewbut = GetNewControl(159, screen);
    GetControlBounds(overviewbut, &r);
    MoveControl(overviewbut, r.left + leftshift, r.top + downshift);

    charmainbut = GetNewControl(500, screen);
    GetControlBounds(charmainbut, &r);
    MoveControl(charmainbut, r.left + leftshift, r.top);

    attacks = GetNewControl(166, screen);
    GetControlBounds(attacks, &r);
    MoveControl(attacks, r.left, r.top + downshift);

    monsterbut = GetNewControl(167, screen);
    GetControlBounds(monsterbut, &r);
    MoveControl(monsterbut, r.left, r.top + downshift);

    combatitem = GetNewControl(149, screen);
    GetControlBounds(combatitem, &r);
    MoveControl(combatitem, r.left, r.top + downshift);

    showitems = GetNewControl(164, screen);
    GetControlBounds(showitems, &r);
    MoveControl(showitems, r.left, r.top + downshift);

    condition = GetNewControl(165, screen);
    GetControlBounds(condition, &r);
    MoveControl(condition, r.left, r.top + downshift);

    showitembut = GetNewControl(168, screen);
    GetControlBounds(showitembut, &r);
    MoveControl(showitembut, r.left + leftshift, r.top + downshift);

    torch = GetNewControl(175, screen); /**** turn must come AFTER torch ****/
    GetControlBounds(torch, &r);
    MoveControl(torch, r.left + leftshift, r.top + downshift);

    turn = GetNewControl(147, screen);
    GetControlBounds(turn, &r);
    MoveControl(turn, r.left + leftshift, r.top + downshift);

    // HideControl(turn); // Myriad
    showconditionbut = GetNewControl(169, screen);
    GetControlBounds(showconditionbut, &r);
    MoveControl(showconditionbut, r.left + leftshift, r.top + downshift);

    melee = GetNewControl(177, screen);
    GetControlBounds(melee, &r);
    MoveControl(melee, r.left, r.top + downshift);

    for (t = 0; t < 6; t++) {
      autoone[t] = GetNewControl(190 + t, screen);
      GetControlBounds(autoone[t], &r);
      MoveControl(autoone[t], r.left + leftshift, r.top);
    }

    if (gWindow != NIL)
      DisposeWindow(gWindow);
    gWindow = GetNewWindow(131, NIL, (WindowPtr)-1L);
    MoveWindow(gWindow, GlobalLeft, GlobalTop, FALSE);

    if (look != NIL)
      DisposeWindow(look);
    look = GetNewWindow(131, NIL, (WindowPtr)-1L);
    MoveWindow(look, GlobalLeft, GlobalTop, FALSE);

    ShowWindow(screen);
    ShowWindow(gWindow);
    ShowWindow(look);

    // BringToFront(screen);
    BringToFront(look);
    BringToFront(gWindow);
  }

  SetPort(GetWindowPort(look));
  BackPixPat(base);
  ForeColor(blackColor);
  BackColor(whiteColor);

  PenMode(2);
  PenSize(2, 2);
  TextFace(bold);
  TextSize(12);

  if (!mode) {
    DisposeDialog(background);
    background = NIL;
  }

  DrawMenuBar();

  if (dostart) {
    updatemain(FALSE, -1);
    newland(0L, 0L, 1, globalmacro[0], 0);
  }

  mainscreen(0);
}

/************************************ getexp **************************/
void getexp(short level, short casteid, short who) {
  FILE* fp = NULL;
  long value;

  loadprofile(0, casteid); /************* load in caste profile to get vicotory points required *********/

  if (level > 30)
    level = 30;
  value = -(caste.victory[level - 1]);
  if (who == -1)
    characterl.exp = value;
  else
    c[who].exp += value;
}

/************* cleartarget *******************/
void cleartarget(void) {
  short t;
  pick = 0;
  for (t = 0; t < 7; t++)
    target[t][0] = target[t][1] = -1;
  for (t = 0; t < maxloop; t++)
    track[t] = 0;
}

/********************* pict **********************/
void pict(short id, Rect frame) {
  PicHandle picturehandle = NIL;

  picturehandle = GetPicture(id);
  if (picturehandle)
    DrawPicture(picturehandle, &frame);
}

/************************ string ********************/
void string(short number) {
  MyrNumToString(number, myString);
  MyrDrawCString((Ptr)myString);
}

/************************ loadextracode ********************/
void loadextracode(short id) {
  FILE* fp = NULL;
  getfilename("Data EDCD");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(118);
  fseek(fp, id * sizeof extracode, SEEK_SET);
  fread(&extracode, sizeof extracode, 1, fp);
  CvtTabShortToPc(&extracode, 5);
  fclose(fp);
}

/************************* tween ******************/
short tween(short tween, short left, short right) {
  short reply = FALSE;
  if ((tween > left) && (tween < right))
    return (TRUE);
  return (reply);
}

/************************* twixt ******************/
short twixt(short tween, short left, short right) {
  short reply = FALSE;
  if ((tween >= left) && (tween <= right))
    reply = TRUE;
  return (reply);
}

/**************************** randrange ************************/
short randrange(short low, short high) {
  return (Rand(high - low + 1) - 1 + low);
}

/************************** updateprep **********************/
void updateprep(void) {
  int leftsize, rightsize;

  leftsize = 380 + leftshift - (leftshift * forcesmall);
  rightsize = 640 + leftshift - (leftshift * forcesmall);

  SetRect(&itemRect, leftsize, 0, rightsize, 50);

  for (t = 0; t <= charnum; t++) {
    if (c[t].spellpointsmax)
      temp = 192;
    else
      temp = 189;
    pict(temp, itemRect);
    updatepictbox(t, 0, 0);
    itemRect.top += 50;
    itemRect.bottom += 50;

    if (doauto[t]) {
      GetControlBounds(autoone[t], &buttonrect);
      // buttonrect = *&(**(autoone[t])).contrlRect;
      downbutton(TRUE);
    }
  }

  BackPixPat(base);
  itemRect.bottom = 300;
  itemRect.left = 320;
  EraseRect(&itemRect);
  RGBBackColor(&greycolor);
}

/******************************** strength **********************/
void strength(short st) {
  temp = damage = 0;

  if (st < 4)
    temp = -20;
  else {
    switch (st) {
      case 4:
        temp = -15;
        damage = -1;
        break;

      case 5:
        temp = -10;
        damage = -1;
        break;

      case 6:
        temp = -5;
        break;

        /**********************/

      case 16:
        temp = 5;
        damage = 1;
        break;

      case 17:
        temp = 5;
        damage = 2;
        break;

      case 18:
        temp = 10;
        damage = 2;
        break;

      case 19:
        temp = 10;
        damage = 3;
        break;

      case 20:
        temp = 15;
        damage = 3;
        break;

      case 21:
        temp = 15;
        damage = 4;
        break;

      case 22:
        temp = 20;
        damage = 4;
        break;

      case 23:
        temp = 20;
        damage = 5;
        break;

      case 24:
        temp = 25;
        damage = 5;
        break;

      case 25:
        temp = 25;
        damage = 6;
        break;

      case 26:
        temp = 30;
        damage = 6;
        break;

      case 27:
        temp = 30;
        damage = 7;
        break;

      case 28:
        temp = 35;
        damage = 7;
        break;

      case 29:
        temp = 35;
        damage = 8;
        break;

      case 30:
        temp = 40;
        damage = 8;
        break;
    }
  }

  if (damage > caste.strength[1])
    damage = caste.strength[1]; /********** check for max damage bonus ************/
}

/******************* getcharrange *****************/
void getcharrange(short body) {
  short t;

  range[6] = 127;
  for (t = 0; t <= charnum; t++) {
    range[t] = 127;
    if (c[t].stamina > 0) {
      checkrange(body, pos[t][0], pos[t][1]);
      range[t] = currange;
      if (currange < range[6])
        range[6] = currange;
    }
  }
}

/********************* GetDialogNum **************************/
long GetDialogNum(short number) {
  GetDialogItem(gCurrent, number, &itemType, &itemHandle, &itemRect);
  GetDialogItemText(itemHandle, myString);
  StringToNum(myString, &tempvalue);
  return (tempvalue);
}

/************************* xy **********************/
void xy(short mode) {
  Rect itemRect;

  SetPort(GetWindowPort(screen));
  BackPixPat(base);
  RGBBackColor(&greycolor);
  itemRect.top = 302 + downshift;
  itemRect.bottom = itemRect.top + 16;

  itemRect.left = 350 + leftshift;
  itemRect.right = itemRect.left + 25;

  ForeColor(yellowColor);

  TextFont(font);
  TextSize(16);
  TextMode(1);

  EraseRect(&itemRect);

  itemRect.left += 50;
  itemRect.right += 50;
  EraseRect(&itemRect);

  MoveTo(352 + leftshift, 314 + downshift);

  if (mode) {
    ForeColor(whiteColor);
    MyrNumToString(themap.startx, myString);
    MyrDrawCString((Ptr)myString);
    MoveTo(406 + leftshift, 314 + downshift);
    MyrNumToString(themap.starty, myString);
    MyrDrawCString((Ptr)myString);
  } else {
    if (indung) {
      MyrNumToString(floorx, myString);
      if (xydisplayflag)
        MyrDrawCString("?");
      else
        MyrDrawCString((Ptr)myString);
      MoveTo(406 + leftshift, 314 + downshift);
      MyrNumToString(floory, myString);
      if (xydisplayflag)
        MyrDrawCString("?");
      else
        MyrDrawCString((Ptr)myString);
    } else {
      MyrNumToString(lookx + partyx, myString);
      if (xydisplayflag)
        MyrDrawCString("?");
      else
        MyrDrawCString((Ptr)myString);
      MoveTo(406 + leftshift, 314 + downshift);
      MyrNumToString(looky + partyy, myString);
      if (xydisplayflag)
        MyrDrawCString("?");
      else
        MyrDrawCString((Ptr)myString);
    }
  }
}

/********************* scratch **************************/
void scratch(short location) {
  DialogRef scratch;
  char loc[255];
  Str255 hold;

  if ((!nofade) && (!nologo))
    fadeinout(25, fadein); /******** fade to black ******/

  strcpy(loc, (StringPtr) "");

  if (!MyrBitTstLong(&serial, 6 + divine))
    strcat(loc, (StringPtr) "G");
  if (quest[127] == 66)
    strcat(loc, (StringPtr) "S");

  if ((fp = MyrFopen(":Character Editor", "rb")) != NULL) {
    fclose(fp);
    strcat(loc, (StringPtr) "P");
  } else if ((fp = MyrFopen(":Character Editor copy", "rb")) != NULL) {
    fclose(fp);
    strcat(loc, (StringPtr) "P");
  } else if ((fp = MyrFopen(":Character Editor 6.0", "rb")) != NULL) {
    fclose(fp);
    strcat(loc, (StringPtr) "P");
  }

  strcat(loc, (StringPtr) " EL");
  MyrNumToString(abs(location), hold);
  // PtoCstr(hold);
  strcat(loc, hold);
  strcat(loc, (StringPtr) " CS");

  MyrNumToString((currentscenario - 9), hold);
  // PtoCstr(hold);
  strcat(loc, hold);
  strcat(loc, (StringPtr) " "); /***** get game version ******/
  GetVersStr(1, Appl_Rsrc_Fork_Ref_Num);
  PtoCstr((StringPtr)theString);
  strcat(loc, theString);
  CtoPstr((Ptr)theString);

  strcat(loc, (StringPtr) " - ");

  showserial = serial;

#if divine
  MyrBitClrLong(&showserial, 7); /******* use bit 7 for Divine Right *********/
#else
  MyrBitClrLong(&showserial, 6 + divine);
#endif

  MyrNumToString(showserial, hold);
  // PtoCstr(hold);
  strcat(loc, hold);

  strcat(loc, (StringPtr) " - ");
  PtoCstr((StringPtr)registrationname);
  strcat(loc, registrationname);
  CtoPstr(registrationname);
  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(danapplegate): MyrParamText expects to operate on cstrings, but here loc is
   * converted to a pascal string before being passed to it. Removing the CtoPstr call
   * keeps it a cstring.
   */
  // CtoPstr(loc);

  sound(10141);

  if (lowHD)
    MyrParamText((Ptr)loc, (Ptr) "Note: You are low on hard drive space and this is likely the cause.  Free up some more space and try again.", (Ptr) "", (Ptr) "");
  else
    ParamText((StringPtr)loc, (StringPtr) "", (StringPtr) "", (StringPtr) "");

  scratch = GetNewDialog(168, NIL, (WindowPtr)-1L);
  SetPortDialogPort(scratch);
  BackPixPat(base);
  ForeColor(yellowColor);
  FlushEvents(everyEvent, 0);
  ModalDialog(0L, &itemHit);
  exit(0);
}

/********************* scratch2 **************************/
void scratch2(short location) {
  short templocation;
  DialogRef scratch;
  char loc[255];
  Str255 hold;

  if ((!nofade) && (!nologo))
    fadeinout(25, fadein); /******** fade to black ******/

  strcpy(loc, (StringPtr) "");

  templocation = 1000 + abs(location);

  if (!MyrBitTstLong(&serial, 6 + divine))
    strcat(loc, (StringPtr) "G");
  if (quest[127] == 66)
    strcat(loc, (StringPtr) "S");
  if ((fp = MyrFopen(":Character Editor", "rb")) != NULL) {
    fclose(fp);
    strcat(loc, (StringPtr) "P");
  } else if ((fp = MyrFopen(":Character Editor Copy", "rb")) != NULL) {
    fclose(fp);
    strcat(loc, (StringPtr) "P");
  }

  strcat(loc, (StringPtr) " EL");
  MyrNumToString(templocation, hold);
  // PtoCstr(hold);
  strcat(loc, hold);
  strcat(loc, (StringPtr) " - ");

  showserial = serial;

#if divine
  MyrBitClrLong(&showserial, 7); /******* use bit 7 for Divine Right *********/
#else
  MyrBitClrLong(&showserial, 6 + divine);
#endif

  MyrNumToString(showserial, hold);
  // PtoCstr(hold);
  strcat(loc, hold);

  GetVersStr(1, Appl_Rsrc_Fork_Ref_Num);
  strcat(loc, (StringPtr) " - "); /***** get game version ******/
  PtoCstr((StringPtr)theString);
  strcat(loc, theString);
  CtoPstr((Ptr)theString);

  strcat(loc, (StringPtr) " ");
  PtoCstr((StringPtr)registrationname);
  strcat(loc, registrationname);
  CtoPstr(registrationname);

  CtoPstr(loc);

  GetIndString(myString, 5, templocation - 1000);

  sound(10141);
  ParamText((StringPtr)loc, myString, (StringPtr) "", (StringPtr) "");

  scratch = GetNewDialog(168, NIL, (WindowPtr)-1L);
  SetPortDialogPort(scratch);
  BackPixPat(base);
  ForeColor(yellowColor);
  FlushEvents(everyEvent, 0);
  ModalDialog(0L, &itemHit);
  exit(0);
}

/************************ savecharacter *********************/
void savecharacter(short who) {
  char charfilename[255];
  struct character tempchar, tempsave;
  FILE* fp;
  short t;

  if (who < 0) {
    setverify(-1);
    tempchar = characterl;
  } else {
    tempsave = c[who];

    for (t = c[who].numitems - 1; t > -1; t--) {
      loaditemnotype(c[who].items[t].id);
      if ((item.type == 25) || (item.type == 23) || (item.type < 0)) {
        item.type = abs(item.type);
        dropitem(who, c[who].items[t].id, t, FALSE, TRUE);
      }
    }
    setverify(who);
    tempchar = c[who];
    c[who] = tempsave;
  }

  minus(tempchar.name, 1);
  plus(tempchar.name, tempchar.level);

  if ((doreg()) || (tempchar.level < 4)) {
    strcpy(charfilename, ":Character Files:");
    strcat(charfilename, tempchar.name);

    if ((fp = MyrFopen(charfilename, "w+b")) == NULL)
      scratch(2);
    CvtCharacterToPc(&tempchar);
    fwrite(&tempchar, sizeof tempchar, 1, fp);
    CvtCharacterToPc(&tempchar);
    fclose(fp);

    loadprofile(0, tempchar.caste);
    switch (caste.casteclass) {
      case 1:
        setfileinfo("Warr", charfilename);
        break;

      case 2:
        setfileinfo("Rogu", charfilename);
        break;

      case 3:
        setfileinfo("Arch", charfilename);
        break;

      case 4:
        setfileinfo("Sorc", charfilename);
        break;

      case 5:
        setfileinfo("Prie", charfilename);
        break;

      case 6:
        setfileinfo("Entr", charfilename);
        break;

      case 7:
        setfileinfo("WaWi", charfilename);
        break;
    }
  }
}

/*************************** setfileinfo ***********/
void setfileinfo(char type[4], char filename[256]) {
  FSSpec fsp;
  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(danapplegate): Some invocations of setfileinfo pass a string literal
   * as the filename, which CtoPstr attempts to modify in-place, causing an invalid write
   * to program memory. Instead, we make a copy on the heap, modify that, and
   * eliminate the PtoCstr call that restores the filename at the end of function as well.
   */
  char* local_filename = strdup(filename);
  CtoPstr(local_filename);
  FSMakeFSSpec(0, 0, (unsigned char*)local_filename, &fsp);
  FSpGetFInfo(&fsp, &fileinfo);
  fileinfo.fdType = *(OSType*)type;
  fileinfo.fdCreator = 'RLMZ';
  FSpSetFInfo(&fsp, &fileinfo);
  free(local_filename);
  // PtoCstr((StringPtr)filename);
  /* *** END CHANGES *** */
}

/***************************** calcw ********************************/
void calcw(short who) {
  short t;

  short oldload = c[who].load;

  c[who].load = 0;
  c[who].load += c[who].money[0];
  c[who].load += c[who].money[1];
  c[who].load += (c[who].money[2] * 15);

  for (t = 0; t < c[who].numitems; t++) {
    loaditem(c[who].items[t].id);
    c[who].load += (item.wieght + c[who].items[t].charge * item.xcharge);
  }
}

/******************************* ScrollProc  ***************************/
pascal void ScrollProc(ControlHandle dummycontrol, short theCode) {
  short charge;
  short scroll;
  Rect rect;
  Boolean showshop = FALSE;

  dummycontrol = NIL;

  curControlValue = GetControlValue(theControl);
  maxControlValue = GetControlMaximum(theControl);
  minControlValue = GetControlMinimum(theControl);

  rect.left = 390 + (leftshift / 2);
  if (theControl == charitemsvert)
    rect.left = 10;

  rect.right = rect.left + 250 + (leftshift / 2);

  switch (theCode) {
    case kControlPageDownPart:
      rect.top = 16;
      rect.bottom = 386;
      for (tt = 0; tt < 9; tt++) {
        charge = 0;
        delay(2); // Slows it down on fast machines.
        if (curControlValue < maxControlValue) {
          curControlValue++;
          ScrollRect(&rect, 0, -40, 0L);
          icon.top = 336;
          if (shop == TRUE) {
            tempid = theshop.id[shopselection + curControlValue + 8];
            if (tempid < 2000) {
              loaditem(tempid);
              showshop = TRUE;
              charge = item.charge;
            }
            lg = TRUE;
          }

          if ((shop == FALSE) && (cr > -1)) {
            tempid = characterr.items[curControlValue + 8].id;
            lg = characterr.items[curControlValue + 8].ident;
            charge = characterr.items[curControlValue + 8].charge;
            characterr.items[curControlValue + 8].charge;
            shopequip = characterr.items[curControlValue + 8].equip;
          }

          if (theControl == charitemsvert) {
            tempid = characterl.items[curControlValue + 8].id;
            lg = characterl.items[curControlValue + 8].ident;
            charge = characterl.items[curControlValue + 8].charge;
            shopequip = characterl.items[curControlValue + 8].equip;
            showshop = FALSE;
          }

          if (skip == TRUE) {
            tempid = characterl.items[characterl.numitems - 1].id;
            lg = characterl.items[curControlValue - 1].ident;
            charge = characterl.items[curControlValue - 1].charge;
            shopequip = characterl.items[curControlValue - 1].equip;
          }
          ploticon(tempid, showshop);
          if (tempid)
            showcharge2(charge, lg);

          shopequip = FALSE;
        }
      }
      break;

    case kControlDownButtonPart:
      rect.top = 16;
      rect.bottom = 386;
      if ((curControlValue < maxControlValue) || (skip == TRUE)) {
        curControlValue++;
        scroll = TRUE;
        if ((theControl == charitemsvert) && (characterl.numitems < 10))
          scroll = FALSE;
        if ((theControl == shopitemsvert) && (characterr.numitems < 9) && (cr != -1))
          scroll = FALSE;

        if (scroll == TRUE)
          for (t = 0; t < 4; t++)
            ScrollRect(&rect, 0, -10, 0L);
        delay(2); // Slows it down on fast machines.
        icon.top = 336;
        if (theControl == shopitemsvert) {
          if (cr == -1) {
            tempid = theshop.id[shopselection + curControlValue + 8];
            if (tempid < 2000) {
              showshop = TRUE;
              loaditem(tempid);
              charge = item.charge;
            }
            lg = TRUE;
          } else {
            icon.top = 16 + 40 * characterr.numitems;
            if (icon.top > 336)
              icon.top = 336;
            tempid = characterr.items[curControlValue + 8].id;
            lg = characterr.items[curControlValue + 8].ident;
            charge = characterr.items[curControlValue + 8].charge;
            shopequip = characterr.items[curControlValue + 8].equip;
          }
        } else {
          icon.top = 40 * characterl.numitems - 24;
          if (icon.top > 336)
            icon.top = 336;
          tempid = characterl.items[curControlValue + 8].id;
          lg = characterl.items[curControlValue + 8].ident;
          charge = characterl.items[curControlValue + 8].charge;
          shopequip = characterl.items[curControlValue + 8].equip;

          if (skip == TRUE) {
            tempid = characterl.items[characterl.numitems - 1].id;
            lg = characterl.items[characterl.numitems - 1].ident;
            charge = characterl.items[characterl.numitems - 1].charge;
          }
        }
        ploticon(tempid, showshop);
        if (tempid)
          showcharge2(charge, lg);

        shopequip = FALSE;
      }

      break;
    case kControlUpButtonPart:
      rect.top = 6;
      rect.bottom = 368;
      if ((curControlValue > minControlValue) || (skip == TRUE)) {
        curControlValue--;
        for (t = 0; t < 4; t++)
          ScrollRect(&rect, 0, 10, 0L);
        delay(2); // Slows it down on fast machines.
        icon.top = 16;

        if (shop == TRUE) {
          tempid = theshop.id[shopselection + curControlValue];
          if (tempid < 2000) {
            showshop = TRUE;
            loaditem(tempid);
            charge = item.charge;
          }
          lg = TRUE;
        }

        if ((shop == FALSE) && (cr > -1)) {
          tempid = characterr.items[curControlValue].id;
          lg = characterr.items[curControlValue].ident;
          charge = characterr.items[curControlValue].charge;
          shopequip = characterr.items[curControlValue].equip;
        }

        if (theControl == charitemsvert) {
          tempid = characterl.items[curControlValue].id;
          lg = characterl.items[curControlValue].ident;
          charge = characterl.items[curControlValue].charge;
          shopequip = characterl.items[curControlValue].equip;
        }
        ploticon(tempid, showshop);
        if (tempid)
          showcharge2(charge, lg);

        shopequip = FALSE;
      }
      break;

    case kControlPageUpPart:
      rect.top = 6;
      rect.bottom = 368;
      for (tt = 0; tt < 9; tt++) {
        charge = 0;
        delay(2); // Slows it down on fast machines.
        if (curControlValue > minControlValue) {
          curControlValue--;
          ScrollRect(&rect, 0, 40, 0L);
          icon.top = 16;
          if (shop == TRUE) {
            tempid = theshop.id[shopselection + curControlValue];
            if (tempid < 2000) {
              showshop = TRUE;
              loaditem(tempid);
              charge = item.charge;
            }
            lg = TRUE;
          }

          if ((shop == FALSE) && (cr > -1)) {
            tempid = characterr.items[curControlValue].id;
            lg = characterr.items[curControlValue].ident;
            charge = characterr.items[curControlValue].charge;
            shopequip = characterr.items[curControlValue].equip;
          }

          if (theControl == charitemsvert) {
            tempid = characterl.items[curControlValue].id;
            lg = characterl.items[curControlValue].ident;
            charge = characterl.items[curControlValue].charge;
            shopequip = characterl.items[curControlValue].equip;
            showshop = FALSE;
          }
          ploticon(tempid, showshop);
          if (tempid)
            showcharge2(charge, lg);

          shopequip = FALSE;
        }
      }
      break;
  }
  SetControlValue(theControl, curControlValue);
}

/***************** updateshop ********************************/
void updateshop(void) {
  Rect itemRect;

  SetPort(GetWindowPort(gshop));
  TextSize(16);
  TextMode(1);
  TextFace(0);
  BackPixPat(base);
  RGBForeColor(&goldcolor);

  itemRect.left = 460 + (leftshift / 2);
  itemRect.right = itemRect.left + 45;

  if (moneypool[0] < 1)
    moneypool[0] = 0;
  if (moneypool[1] < 1)
    moneypool[1] = 0;
  if (moneypool[2] < 1)
    moneypool[2] = 0;

  itemRect.top = 395;
  itemRect.bottom = itemRect.top + 15;
  EraseRect(&itemRect);

  MoveTo(462 + (leftshift / 2), 405);
  string(characterl.money[0]);

  itemRect.top = 414;
  itemRect.bottom = itemRect.top + 15;
  EraseRect(&itemRect);

  MoveTo(462 + (leftshift / 2), 426);
  MyrNumToString(moneypool[0], myString);
  MyrDrawCString((Ptr)myString);

  BackPixPat(whitepat);
  BackColor(whiteColor);
}

/********* eraseshopstats ***********/
void eraseshopstats(short mode) {
  Rect itemRect;
  TextFace(0);

  BackPixPat(base);
  itemRect.top = 414;
  itemRect.bottom = itemRect.top + 15;
  itemRect.left = 106 + (leftshift / 2);
  itemRect.right = itemRect.left + 45;

  if (mode) {
    EraseRect(&itemRect);

    OffsetRect(&itemRect, 52, 0);
    EraseRect(&itemRect);

    OffsetRect(&itemRect, 52, 0);
    EraseRect(&itemRect);
  } else {
    OffsetRect(&itemRect, 104, 0);
    EraseRect(&itemRect);
  }
  BackPixPat(whitepat);
}

/********* eraseshopname ***********/
void eraseshopname(short mode) {
  Rect itemRect;
  TextFace(0);

  BackPixPat(base);
  itemRect.top = 437;
  itemRect.bottom = itemRect.top + 18;

  if (!mode) {
    itemRect.left = 68 + (leftshift / 2);
    itemRect.right = itemRect.left + 188;
    EraseRect(&itemRect);
    MoveTo(71 + (leftshift / 2), 449);
  } else {
    itemRect.left = 420 + (leftshift / 2);
    itemRect.right = itemRect.left + 195;
    EraseRect(&itemRect);
    MoveTo(423 + (leftshift / 2), 449);
  }
}

/**************************** shortupdate ****************/
void shortupdate(short mode) {
  needupdate = FALSE;
  updateprep();
  for (t = 0; t <= charnum; t++)
    updatechar(t, mode);
}
/**************************** selectupdate ****************/
void selectupdate(void) {
  for (t = 0; t <= charnum; t++) {
    if (select[t]) {
      if (select[t] > 0)
        updatecharshort(t, 0);
      else
        updatecharshort(t, 1);
      select[t] = 0;
    }
  }
}

/*************** center *********************/
void center(short mon) {
  if (mon > 9) {
    mon -= 10;
    if ((!twixt(monpos[mon][0], 1, 8)) || (!twixt(monpos[mon][1], 1, 8)))
      centerfield(monpos[mon][0], monpos[mon][1]);
  } else if ((mon > -1) && (mon <= charnum)) {
    if ((!twixt(pos[mon][0], 1, 8)) || (!twixt(pos[mon][1], 1, 8)))
      centerfield(pos[mon][0], pos[mon][1]);
  }
}

/************************ savevs ***********************/
short savevs(short which, short who) {
  short temp, t, special = 0;
  short spelladjust = 0;

  temp = Rand(100);

  if (inspell) {
    if (spellinfo.cannot > 1)
      return (FALSE);
    spelladjust = (powerlevel * spellinfo.saveadjust) + spellinfo.savebonus;
  }

  if ((who > -1) && (who <= charnum)) {
    if ((which == 0) && (partycondition[7]))
      spelladjust += 50;
    if (temp <= c[who].save[which] + spelladjust)
      return (TRUE);
  } else if (who > 9) {
    if (which == 7) /**** special ****/
    {
      for (t = 0; t < 6; t++)
        special += monster[who - 10].save[t];
      special /= 6;
      if (temp <= special + spelladjust)
        return (TRUE);
    } else {
      if ((temp <= monster[who - 10].save[which] + spelladjust) || (monster[who - 10].spellimmune[which]))
        return (TRUE);
      if ((monster[who - 10].type[1]) && ((which == 5) || (which == 4) || (which == 0)))
        return (TRUE);
    }
  }
  return (FALSE);
}

/************************ savevsattr ***********************/
short savevsattr(short which, short who) {
  short adjust;

  if ((who < 0) || (who > charnum))
    return (TRUE);

  switch (which) {
    case 0: /***** Brawn ***/
      adjust = c[who].st + c[who].magst;
      break;

    case 1: /***** Knowledge ***/
      adjust = c[who].in;
      break;

    case 2: /***** Judgment ***/
      adjust = c[who].wi;
      break;

    case 3: /***** Agility ***/
      adjust = c[who].de;
      break;

    case 4: /***** Vitality ***/
      adjust = c[who].co + c[who].magco;
      break;

    case 6: /***** luck ***/
      adjust = c[who].lu + c[who].maglu;
      break;
  }
  adjust *= 4;

  if (Rand(100) < adjust)
    return (TRUE);
  return (FALSE);
}

/***************************** adddelscen ********************************/
void adddelscen(short mode) {
  DialogRef namewindow;
  Str255 teststring;

  if (mode)
    MyrParamText((Ptr) "Enter the name of the scenario you wish to install and click OK.", (Ptr) "", (Ptr) "", (Ptr) "");
  else
    MyrParamText((Ptr) "Enter the name of the scenario you wish to uninstall and click OK.", (Ptr) "", (Ptr) "", (Ptr) "");

  namewindow = GetNewDialog(405, 0L, (WindowPtr)-1L);
  gCurrent = namewindow;
  SetPortDialogPort(namewindow);
  BackPixPat(base);
  TextFont(defaultfont);
  ForeColor(yellowColor);
  gStop = 0;
  MoveWindow(GetDialogWindow(namewindow), GlobalLeft + 132, GlobalTop + 125, FALSE);
  ShowWindow(GetDialogWindow(namewindow));
  ErasePortRect();

  sound(30005);
  DrawDialog(namewindow);
  FlushEvents(everyEvent, 0);

  for (;;) {
    SystemTask();
    t = GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
    DoCorrectBugMADRepeat();
#endif
    MyrCheckMemory(2);

    if (IsDialogEvent(&gTheEvent)) {
      if (gTheEvent.what == keyDown) {
        if (BitAnd(gTheEvent.message, charCodeMask) == 13) {
          itemHit = 1;
          FlushEvents(everyEvent, 0);
          gTheEvent.what = 0;
          gTheEvent.message = 0L;
          gTheEvent.when = 0L;
          goto pushkey;
        }
      }

      t = DialogSelect(&gTheEvent, &dummy, &itemHit);
    pushkey:

      if (itemHit == 1) {
        GetIndString(teststring, 3, 1);
        GetDialogItem(namewindow, 4, &itemType, &itemHandle, &itemRect);
        GetDialogItemText(itemHandle, teststring);
        temp = StringWidth(teststring);

        if (!temp)
          goto out;

        if (mode) /***** Add Divine Scenario ************/
        {
          for (t = 1; t < 101; t++) {
            GetIndString(myString, 3, 1);
            GetIndString(myString, -6003 - divine, t);
            temp = StringWidth(myString);
            if (!temp) {
              GetDialogItem(namewindow, 4, &itemType, &itemHandle, &itemRect);
              GetDialogItemText(itemHandle, myString);
              strcpy(filename, myString);
              PtoCstr((StringPtr)filename);
              if (selectscenario(filename, 0)) {
                GetDialogItemText(itemHandle, myString);
                SetIndString(myString, -6003 - divine, t);
                AppendMenu(gGame, myString);
                flashmessage((StringPtr) "Scenario has been added.", 135, 80, 180, 5000);
              } else {
                flashmessage((StringPtr) "Could not locate valid scenairo files.", 135, 80, 0, 5000);
                flashmessage((StringPtr) "Be sure to install the scenario into the scenarios folder first.", 135, 80, 0, 5000);
              }
              goto out;
            }
          }
          flashmessage((StringPtr) "Maximum of 100 scenarios can be installed. You will have to remove one before you can add another.", 135, 132, 0, 6000);
        } else /***** Delete Divine Scenario ************/
        {
          for (t = 1; t < 101; t++) {
            GetIndString(myString, 3, 1);
            GetIndString(myString, -6003 - divine, t);
            GetDialogItem(namewindow, 4, &itemType, &itemHandle, &itemRect);
            GetDialogItemText(itemHandle, teststring);

            PtoCstr(myString);
            PtoCstr(teststring);

            if (!strcmp(myString, teststring)) {
              SetIndString((StringPtr) "", -6003 - divine, t);
              flashmessage((StringPtr) "Scenario has been removed, Note: It will still appear in the menu until the next time you play.", 135, 132, 0, 6000);
              goto out;
            }
          }
          flashmessage((StringPtr) "There is no scenario by that name so nothing has changed.", 135, 80, 0, 6000);
        }
      out:
        DisposeDialog(namewindow);
        updatescenarioavail();
        return;
      }
    }
  }
}

/***************************** getfilename **********************/
void getfilename(char tempfilename[256]) {
  strcpy(filename, scenarioname);
  strcat(filename, tempfilename);
}

/******************** in *****************/
void in(void) {
  Rect blitbox;

  if (look == NIL)
    return;

  blitbox = lookrect;
  blitbox.right += 320;
  blitbox.left += 320;

  if (screensize) {
    blitbox.left += leftshift;
    blitbox.right = 800;
  }

  okout = TRUE;

  SetPort(GetWindowPort(screen));
  ForeColor(blackColor);
  BackColor(whiteColor);

  {
    BitMap* src = GetPortBitMapForCopyBits(GetWindowPort(screen));
    BitMap* dst = GetPortBitMapForCopyBits(gbuff);
    CopyBits(src, dst, &blitbox, &lookrect, 0, NIL);
  }

  RGBBackColor(&greycolor);
}

/******************** out *****************/
void out(void) {
  Rect blitbox;

  if (look == NIL)
    return;

  if (!okout) {
    needupdate = okout = TRUE;
    shortupdate(0);
    in();
  }

  blitbox = lookrect;
  blitbox.right += 320;
  blitbox.left += 320;

  if (screensize) {
    blitbox.left += leftshift;
    blitbox.right = 800;
  }

  SetPort(GetWindowPort(screen));
  ForeColor(blackColor);
  BackColor(whiteColor);

  {
    BitMap* src = GetPortBitMapForCopyBits(gbuff);
    BitMap* dst = GetPortBitMapForCopyBits(GetWindowPort(screen));
    CopyBits(src, dst, &lookrect, &blitbox, 0, NIL);
  }

  RGBBackColor(&greycolor);
}

/******************************** downbutton *********************/
void downbutton(short newport) {
  GrafPtr oldport;
  GetPort(&oldport);
  if (newport)
    SetPort(GetWindowPort(screen));

  PenPixPat(dark);
  MoveTo(buttonrect.left, buttonrect.bottom);
  LineTo(buttonrect.left, buttonrect.top);
  LineTo(buttonrect.right, buttonrect.top);

  PenPixPat(light);
  LineTo(buttonrect.right, buttonrect.bottom);
  LineTo(buttonrect.left, buttonrect.bottom);
  SetPort(oldport);
}

/**************** checkfordoauto *********************/
void checkfordoauto(void) {
  short t, tt, count = 0;

  for (t = 0; t < 6; t++) {
    if (theControl == autoone[t]) {
      GetControlBounds(autoone[t], &buttonrect);
      // buttonrect = *&(**(autoone[t])).contrlRect;
      if (doauto[t]) {
        sound(139);
        doauto[t] = FALSE;
        updatechar(t, 3);
        flashmessage((StringPtr) "Manual Mode.", 30, 50, 60, 139);
      } else {
        sound(147);
        doauto[t] = TRUE;
        updatechar(t, 3);
        downbutton(TRUE);
        flashmessage((StringPtr) "Auto Mode.", 30, 50, 60, 139);
        if ((incombat) && (t == q[up]))
          theControl = campbut; /********* lets rock!!!!!! YAHHHHHHHHHHH!!!!  ************/
        for (tt = 0; tt <= charnum; tt++)
          if (doauto[tt])
            count++;
      }
    }
  }
  if (count > charnum)
    warn(107); /**** let them know how to get out of ALL AUTO ****/
}

/****************************** doreg **********************************/
short doreg(void) {
#if development
  if (divineref)
    return (1);
  else
    return (0);
#else
  if ((MyrBitTstLong(&serial, 6 + divine)) || (!serial))
    return (FALSE);
  else if ((!MyrBitTstLong(&serial, 6 + divine)) && (serial))
    return (TRUE);
  exit(0);
#endif
  return (0);
}

/****************************** DialogNumNZ ********************************/
void DialogNumNZ(short theItem, short theNum) {
  short temp;

  GetDialogItem(gCurrent, theItem, &itemType, &itemHandle, &itemRect);
  if (!theNum) {
    SetDialogItemText(itemHandle, (StringPtr) "");
    return;
  }

  if (theNum > 0) {
    temp = -theNum;
    MyrNumToString(temp, myString);
    myString[0] = 43;
  } else {
    MyrNumToString(theNum, myString);
  }
  CtoPstr((Ptr)myString);
  SetDialogItemText(itemHandle, myString);
}

/************************************ editportraits *********************/
short editportraits(short fromid, short mode) {
  short t, choice;
  RgnHandle theRgn;
  Rect slop;

  SetRect(&slop, -32000, -32000, 32000, 32000);
  theRgn = NewRgn();

  GetDialogItem(gCurrent, itemHit, &itemType, &itemHandle, &itemRect);
  RectRgn(theRgn, &itemRect);

  point.h = itemRect.left + 25;
  point.v = itemRect.top + 25;
  while (Button()) {
    DragGrayRgn(theRgn, point, &slop, &slop, 0, NIL);
  }

  GetMouse(&point);

  for (t = 3; t < 64; t++) {
    GetDialogItem(gCurrent, t, &itemType, &itemHandle, &itemRect);
    if (PtInRect(point, &itemRect)) {
      choice = t - 2;
      goto madechoice;
    }
  }
  goto nochoice;

madechoice:

  if (!mode)
    copyresource(tempport, portraitrefnum, 'cicn', choice + 256, fromid);
  else
    copyresource(temptact, tacticalrefnum, 'cicn', choice + 8999, fromid);

nochoice:

  if (choice > 0) {
    if (!mode)
      return (choice + 257);
    else
      return (choice + 8999);
  } else
    return (0);
  return (0);
}

/******************** copyresource *****************/
void copyresource(short fromrefnum, short torefnum, ResType res_type, short toid, short fromid) {
  Handle app_handle;
  short the_ID;
  short currentresfile;
  ResType the_type;
  Str255 res_name;
  short res_attributes;
  Handle test_handle;

  currentresfile = CurResFile();
  UseResFile(fromrefnum);

  app_handle = Get1Resource(res_type, fromid);
  GetResInfo(app_handle, &the_ID, &the_type, res_name);
  res_attributes = GetResAttrs(app_handle);

  DetachResource(app_handle);

  UseResFile(torefnum);

  test_handle = Get1Resource(res_type, toid);

  if (test_handle != NIL) {
    removeresource(torefnum, res_type, toid);
  }

  AddResource(app_handle, res_type, toid, res_name);
  SetResAttrs(app_handle, res_attributes);
  ChangedResource(app_handle);
  WriteResource(app_handle);
  ReleaseResource(app_handle);
  UseResFile(currentresfile);
}

/******************* quitgame ***************/
void quitgame(void) {
  DoFreeBeforeQuit();
  ExitToShell();
}

/******************** removeresource *****************/
void removeresource(short fromrefnum, ResType res_type, short fromid) {
  short currentresfile;
  Handle app_handle = NIL;
  short the_ID;
  ResType the_type;
  Str255 res_name;

  currentresfile = CurResFile();
  UseResFile(fromrefnum);

  app_handle = Get1Resource(res_type, fromid);

  if (app_handle != NIL) {
    GetResInfo(app_handle, &the_ID, &the_type, res_name);
    RemoveResource(app_handle);
    DisposeHandle(app_handle);
    UpdateResFile(fromrefnum);
  } else
    SysBeep(20);

  UseResFile(currentresfile);
}

/********************** loadprofile ****************************/
void loadprofile(short raceid, short casteid) {
  if (raceid) {
    openrace(); /***************** load in race profile ************/
    fseek(fp, (raceid - 1) * sizeof races, SEEK_SET);
    fread(&races, sizeof races, 1, fp);
    CvtRaceToPc(&races);
    fclose(fp);
  }

  if (casteid) {
    opencaste(); /***************** load in caste profile ************/
    fseek(fp, (casteid - 1) * sizeof caste, SEEK_SET);
    fread(&caste, sizeof caste, 1, fp);
    CvtCasteToPc(&caste);
    fclose(fp);
  }
}

/***************** bandaid *******************/
void bandaid(void) {
  Boolean showmessage1 = FALSE;
  Boolean showmessage2 = FALSE;
  short t, tt;

  if (charnum < 0)
    return;

  updatemapmenu(); /**** this has to be here to load in scenario items info ****/

  for (t = 0; t <= charnum; t++) {
    checkverify(t);
    loadprofile(c[t].race, c[t].caste);
    temp = races.specialability[13] + (caste.specialability[1][13] * (c[t].level - 1)) + caste.specialability[0][13];

    if (temp > c[t].spec[13])
      c[t].spec[13] = temp;

    for (tt = c[t].numitems - 1; tt > -1; tt--) /***** check for bogus items ******/
    {
      if (c[t].items[tt].id) {
        loaditem(c[t].items[tt].id);
        if (!item.iconid) {
          showmessage1 = TRUE;
          dropitem(t, c[t].items[tt].id, tt, FALSE, TRUE);
        }
      }
    }

    /********** make sure non spellcasters are o.k. ************/
    if ((caste.spellcasters[0][1] + caste.spellcasters[1][1] + caste.spellcasters[2][1]) == 0) {
      c[t].spellpoints = c[t].spellpointsmax = c[t].spellcastertype = 0;
    }

    if (c[t].version > -3)
      showmessage2 = TRUE; /******* look for old style PC and bail out ******/
  }

  if (showmessage1)
    flashmessage((StringPtr) "Some of these characters have invalid items.  Those items have been deleted. (Click)", 50, 40, 0, 5000);
  if (showmessage2) {
    flashmessage((StringPtr) "Some of these characters are of the old format and are not compatible with this version. (Click)", 50, 40, 0, 6000);
    flashmessage((StringPtr) "You will have to create new characters.  This program must now quit. (Click)", 50, 40, 0, 6000);
    exit(0);
  }
}

/***************************** setverify *******************************/
void setverify(short who) {
  short t;
  switch (who) {
    case -1:
      characterl.verify1 = characterl.level + characterl.staminamax + characterl.attackbonus;
      characterl.verify2 = characterl.tohit + characterl.spellpointsmax + characterl.st + characterl.in;
      characterl.verify3 = characterl.ac + characterl.staminamax + characterl.de + characterl.staminamax + characterl.magres;
      break;

    default:
      c[who].verify1 = c[who].level + c[who].staminamax + c[who].attackbonus;
      c[who].verify2 = c[who].tohit + c[who].spellpointsmax + c[who].st + c[who].in;
      c[who].verify3 = c[who].ac + c[who].staminamax + c[who].de + c[who].staminamax + c[who].magres;

      /************** set characters default spells **************/

      for (t = 0; t < 10; t++) {
        c[who].definespells[t][0] = definespells[who][t][0];
        c[who].definespells[t][1] = definespells[who][t][1];
        c[who].definespells[t][2] = definespells[who][t][2];
        c[who].definespells[t][3] = definespells[who][t][3];
      }
      break;
  }
}

/***************************** checkverify *******************************/
short checkverify(short who) {
  short reply = 0;
  if (c[who].verify1 != (c[who].level + c[who].staminamax + c[who].attackbonus))
    reply = TRUE;
  if (c[who].verify2 != (c[who].tohit + c[who].spellpointsmax + c[who].st + c[who].in))
    reply = TRUE;
  if (c[who].verify3 != (c[who].ac + c[who].staminamax + c[who].de + c[who].staminamax + c[who].magres))
    reply = TRUE;
  return (reply);
}

/**************************** updatemapmenu *****************/
void updatemapmenu(void) {
  short t;
  FILE* fp = NULL;

  for (t = 1; t < 21; t++) {
    if (map[t - 1])
      GetIndString(myString, -102, t);
    else
      GetIndString(myString, -101, t);

    SetMenuItemText(gScenario, t + 3, myString);
    if (map[t - 1])
      EnableItem(gScenario, t + 3);
    else
      DisableItem(gScenario, t + 3);
  }

  getfilename("Data NI");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(104);
  fread(&allsupply, sizeof allsupply, 1, fp);
  CvtTabItemAttrToPc(&allsupply, 200);
  fclose(fp);

  if (quickshow)
    CheckItem(prefer, 7, 1);
  else
    CheckItem(prefer, 7, 0);
}

/****************************** Startlevel **********************************/
short Startlevel(void) {
  DialogRef gGeneration;

  short levelmatch[12] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 20, 25, 30};

  short oldhit = 0;
  Boolean first = TRUE;

  gGeneration = GetNewDialog(161, 0L, (WindowPtr)-1L);
  gStop = FALSE;

  SetPortDialogPort(gGeneration);
  BackPixPat(base);
  TextFont(defaultfont);
  ForeColor(yellowColor);

  MoveWindow(GetDialogWindow(gGeneration), GlobalLeft + 206, GlobalTop + 134, FALSE);
  ShowWindow(GetDialogWindow(gGeneration));
  ErasePortRect();

  while (gStop == FALSE) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);

    if ((itemHit > 2) && (itemHit < 15)) {
      sound(130);

      if ((itemHit > 3) && (!doreg())) {
        warn(103);
        warn(104);
      } else {
        GetDialogItem(gGeneration, itemHit, &itemType, &itemHandle, &buttonrect);
        downbutton(FALSE);
        characterl.level = itemHit - 3;
        if ((itemHit != oldhit) && (!first)) {
          GetDialogItem(gGeneration, oldhit, &itemType, &itemHandle, &buttonrect);
          upbutton(FALSE);
        }
        first = FALSE;
        oldhit = itemHit;
      }
    }

    if ((itemHit == 1) || (itemHit == 29)) {
      if (oldhit) {
        GetDialogItem(gGeneration, 29, &itemType, &itemHandle, &buttonrect);
        ploticon3(133, buttonrect);
        sound(6001);
        DisposeDialog(gGeneration);

        /***** update info to reflect level ****/
        minus(characterl.name, TRUE);
        plus(characterl.name, oldhit - 2);
        return (levelmatch[oldhit - 3] - 1);
      } else
        sound(6000);
    }
  }
  return (0);
}

/********************* memout **************************/
void memout(void) {
  warn(56);
  SetDepth(curGDev, currentDepth, 4, 8);
  exit(0);
}

/*************************** quiet ****************/
void quiet(short id) {
  SndCommand cmd;

  if (numchannel > -1) {
    cmd.cmd = quietCmd;
    cmd.param1 = 0;
    cmd.param2 = 0;

    SndDoImmediate(cool[id], &cmd);
    cmd.cmd = flushCmd;
    SndDoImmediate(cool[id], &cmd);
  }
}

/***************** sound *******************/
void sound(short id) {
  SndCommand theCommand;

  if ((!volume) || (numchannel < 0)) {
    if (id < 0)
      delay(20);
    return;
  }

  channel++;

  if (channel > numchannel)
    channel = 0;

  HUnlock(sndhandle[channel]);
  sndhandle[channel] = GetResource('snd ', abs(id));
  HLock(sndhandle[channel]);

  theCommand.cmd = flushCmd;
  theCommand.param1 = 0;
  theCommand.param2 = 0L;
  bug = SndDoImmediate(cool[channel], &theCommand);

  theCommand.cmd = quietCmd;
  theCommand.param1 = 0;
  theCommand.param2 = 0L;
  bug = SndDoImmediate(cool[channel], &theCommand);

  if (sndhandle != NIL) {
    if (id > 0)
      SndPlay(cool[channel], (SndListHandle)sndhandle[channel], TRUE);
    else
      SndPlay(cool[channel], (SndListHandle)sndhandle[channel], FALSE);
  }
}

/********************* updatemain **************************/
void updatemain(short center, short who) {
  Rect mainrect;

  SetMenuBar(myMenuBar);
  InsertMenu(gSound, -1);
  InsertMenu(gSpeed, -1);
  DrawMenuBar();

  updatemusic();

  if (screen) // Myriad, current port can be NULL
  {
    SetPort(GetWindowPort(screen));

    TextFont(font);

    if ((!who) || (needupdate))
      shortupdate(0);
    else {
      out();
      selectupdate();
    }

    if (!incombat) {
      if (!center)
        centerpict();
      updatefat(1, 0, 0);
      GetNextEvent(everyEvent, &gTheEvent);
      if (screensize) {
        mainrect.top = 320 + downshift;
        mainrect.bottom = 555;
        mainrect.left = 0;
        mainrect.right = 800;
        EraseRect(&mainrect);

        mainrect.top = 300;
        mainrect.bottom = 300 + downshift;
        mainrect.left = 320 + leftshift;
        mainrect.right = 800;
        EraseRect(&mainrect);

        mainrect.top = 321 + downshift;
        mainrect.left = 0;
        mainrect.bottom = 555;
        mainrect.right = 308 + leftshift;
        pict(203, mainrect);
      } else {
        mainrect.top = 321 + downshift;
        mainrect.left = 0;
        mainrect.bottom = 460 + 96;
        mainrect.right = 308;
        pict(203, mainrect);
      }

      mainrect.top = 321 + downshift;
      mainrect.left = 308 + leftshift;
      mainrect.bottom = 460 + 96;
      mainrect.right = 640 + leftshift;
      DrawPicture(mainpict, &mainrect);

      updatetorch();
      tickcheck();
      updatecontrols();
      timeclick(0, FALSE);
      if (revertgame)
        return;
    }
    SetPort(GetWindowPort(screen));
  }
}

/********************** mainscreen ************************/
void mainscreen(short who) {
  WindowPtr whichWindow;
  short a;
  char commandkey;
  short reply = 0;

  SetMenuBar(myMenuBar);
  InsertMenu(gSound, -1);
  InsertMenu(gSpeed, -1);
  DrawMenuBar();

over:

  if (indung) {
    updatemain(TRUE, 0);
    SetPort(GetWindowPort(look));
    BackPixPat(base);
    ForeColor(blackColor);
    BackColor(whiteColor);
    threed(-1L, 0, 0, 0);
    in();
    who = -1;
    if (revertgame) {
      revertgame = FALSE;
      goto over;
    }
  }

  updatemain(FALSE, who);
  FlushEvents(everyEvent, 0);

  for (;;) {
    if (!hide) {
      GetMouse(&point);
      if (!PtInRect(point, &lookrect)) {
        SetCCursor(sword);
        tagold = 20;
      } else
        updatearrow(1);
    }

    key = 0;

    if (gTheEvent.modifiers & alphaLock)
      warn(21);

    tickcheck();
    SystemTask();
    a = GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
    DoCorrectBugMADRepeat();
#endif
    MyrCheckMemory(2);

    if (IsDialogEvent(&gTheEvent)) {
      a = DialogSelect(&gTheEvent, &dummy, &itemHit);

      if (gTheEvent.what == keyDown)
        goto dokey;
    } else {

      switch (gTheEvent.what) {
        case (updateEvt):
          BeginUpdate(look);
          EndUpdate(look);
          BeginUpdate(gWindow);
          EndUpdate(gWindow);
          BeginUpdate(screen);
          EndUpdate(screen);
          if (mat) {
            BeginUpdate(mat);
            EndUpdate(mat);
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

        case (autoKey):
          goto dokey;
          break;

        case (keyDown):
        dokey:

          if (gTheEvent.modifiers & cmdKey) {
            key = gTheEvent.message;
            scanCode = (gTheEvent.message >> 8) & 0xFF;
            checkkeypad(1);
            if (!fastspell) {
              commandkey = BitAnd(gTheEvent.message, charCodeMask);
              menuChoice = MenuKey(commandkey);
              HandleMenuChoice();
              theControl = NIL;
              if (revertgame) {
                revertgame = FALSE;
                goto over;
              }
            }
          } else {
            key = gTheEvent.message;
            scanCode = (gTheEvent.message >> 8) & 0xFF;
            theControl = NIL;
            checkkeypad(0);
            checkkeypad(1);
          }

          reply = 0;

        goback2:

          reply = buttonchoice(reply);

          if (revertgame) {
            revertgame = FALSE;
            goto over;
          }

          switch (reply) {
            case -2:
              if (shopavail)
                theControl = shopbut;
              else
                theControl = tradebut;
              goto goback2;
              break;

            case -1:
              theControl = itemsbut;
              goto goback2;
              break;

            case -3:
              updatemain(FALSE, -1);
              break;
          }
          break;

        case (mouseDown):

          thePart = FindWindow(gTheEvent.where, &whichWindow);
          switch (thePart) {
            case inMenuBar:
              compactheap();
              menuChoice = MenuSelect(gTheEvent.where);
              HandleMenuChoice();
              theControl = NIL;
              if (revertgame) {
                revertgame = FALSE;
                goto over;
              }
              break;

            case inContent:
              point = gTheEvent.where;
              GlobalToLocal(&(point));
              thePart = FindControl(point, screen, &theControl);
              FlushEvents(everyEvent, 0);
              reply = FALSE;

            goback:

              reply = buttonchoice(reply);

              if (revertgame) {
                revertgame = FALSE;
                goto over;
              }

              switch (reply) {
                case -2:
                  if (shopavail)
                    theControl = shopbut;
                  else
                    theControl = tradebut;
                  goto goback;
                  break;

                case -1:
                  theControl = itemsbut;
                  goto goback;
                  break;

                case -3:
                  updatemain(FALSE, -1);
                  break;
              }
              break;
          }
          break;

        case (osEvt):

          if ((gTheEvent.message >> 24) == suspendResumeMessage) {
            if (hide) {
              compactheap();
              hide = FALSE;
              updatemain(FALSE, 0);
              FlushEvents(everyEvent, 0);
            } else
              hide = TRUE;
          }
          break;
      }
    }
  }
}

/***************************** openrace ***************************/
void openrace(void) {
  getfilename("Data Race"); //*** only 3rd party scenariso can use custom races/caste data

  if (currentscenario < 20)
    fp = MyrFopen(":Data Files:Data Race", "rb"); //******** open standard Race file
  else if ((fp = MyrFopen(filename, "rb")) == NULL) //******** open custom Race file
  {
    fp = MyrFopen(":Data Files:Data Race", "rb"); //******** open standard Race file
  }
}

/***************************** opencaste ***************************/
void opencaste(void) {
  getfilename("Data Caste"); //*** only 3rd party scenariso can use custom races/caste data

  if (currentscenario < 20)
    fp = MyrFopen(":Data Files:Data Caste", "rb"); //******** open standard caste file
  else if ((fp = MyrFopen(filename, "rb")) == NULL) //******** open custom caste file
  {
    fp = MyrFopen(":Data Files:Data Caste", "rb"); //******** open standard caste file
  }
}

/**************************** usercheck2 ********************/
short usercheck2(void) {
#if !divine /********* only divine right needs to check for the user name ****/
  return (1);
#else

  Str255 scenname;

  getdivineuser();

  GetMenuItemText(gGame, currentscenario, myString); /*** load in creator for scenario ****/
  PtoCstr(myString);
  getfilename((Ptr)myString);
  if ((fp = MyrFopen(filename, "rb")) != NULL) {
    fseek(fp, (5 * sizeof reclevel) + (40), SEEK_SET);
    fread(&scenname, sizeof scenname, 1, fp);
    fclose(fp);
  } else
    strcpy(scenname, (StringPtr) "");

#if development /****** development version always has access ****/
  return (TRUE);
#endif

  PtoCstr(scenname);
  PtoCstr(codename);

  if (!strlen(scenname))
    return (TRUE); /*** allow them to warp in tutorial ****/
  if (!strcmp(codename, scenname))
    return (TRUE); /********** Valid person to warp ****/
#endif
  return (FALSE);
}

/******************* getdivineuser ***************/
void getdivineuser(void) {
  short pref_ref_num;
  Handle data_handle;
  StringPtr source_str;
  Size byte_count;
  short oldresfile;

  oldresfile = CurResFile();

  pref_ref_num = openpref(6);

  UseResFile(pref_ref_num);

  data_handle = NIL;
  data_handle = Get1Resource('TDLY', 129);
  if (data_handle == NIL) {
    flashmessage((StringPtr) "Please run and register your copy of Divinity first.", 50, 50, 0, 6000);
    exit(0); /*** they have no biz using Divine Right ***/
  }

  source_str = (**(DivinePrefHandle)data_handle).codename;
  byte_count = (**(DivinePrefHandle)data_handle).codename[0] + 1;
  BlockMoveData(source_str, codename, byte_count);

  CloseResFile(pref_ref_num);
  UseResFile(oldresfile);
}

/************************ updatescenarioavail *****************/
void updatescenarioavail(void) {
  short t;
  short start, stop;

  start = 7;
  stop = topfantasoftsceanrio; //  Fantasoft v7.1
  if (divine) {
    start = topfantasoftsceanrio + 1; /*** v7.1 ****/
    ;
    stop = 50;
  }

  for (t = start; t < stop; t++) {
    GetIndString(myString, 3, 1);
    GetMenuItemText(gGame, t, myString);
    PtoCstr(myString); // Myriad
    // if (StringWidth(myString)) Myriad
    if (strlen(myString)) {
      if (selectscenario((Ptr)myString, 0)) {
        EnableItem(gGame, t);
        GetMenuItemText(gGame, t, myString); /***** scenarios version  *********/
        GetVersStr(2, 0);
        PtoCstr(myString);
        PtoCstr(theString);
        strcat(myString, (StringPtr) " - ");
        if (temp != -1)
          strcat(myString, theString);
        else
          strcat(myString, (StringPtr) "Unknown");
        CtoPstr((Ptr)myString);
        AppendMenu(prefer, myString);
      }
    }
  }

  GetMenuItemText(gGame, 10 - 3 * divine, myString);
  PtoCstr(myString);
  selectscenario((Ptr)myString, 0);
}

/***************************** selectscenario **********************/
short selectscenario(char tempfilename[256], short mode) {
  FILE* fp = NULL;
  short t;
  char filename[256];
  char returnname[256];

  if ((!doreg()) && (!seenit)) {
    if (FrontWindow() == GetDialogWindow(background)) {
      gCurrent = background;
      SetPortDialogPort(background);
      BackColor(blackColor);
      MyrCDiStr(2, (StringPtr) "");
      MyrCDiStr(3, (StringPtr) "");
    }
    aboutrealmz();
    return (0);
  }
  if (!doreg() && (currentscenario > 10)) {
    if (mode)
      warn(65);
    return (0);
  }

  strcpy(filename, ":Scenarios:");
  strcat((StringPtr)filename, tempfilename);
  strcpy(scenarioname, filename);
  strcpy(returnname, filename);
  strcat(scenarioname, (StringPtr) ":");
  strcat((StringPtr)filename, ":");
  strcat(returnname, (StringPtr) ":Scenario");
  strcat((StringPtr)filename, tempfilename);

  if ((fp = MyrFopen(filename, "rb")) != NULL) {
    fclose(fp);
    if (refnum > 0) {
      CloseResFile(refnum);
      refnum = -1;
    }
    CtoPstr(returnname);

    refnum = MyrOpenResFile((Ptr)returnname);

    t = CountResources('RLMZ') - 1;

#if divine
    if ((t > 1) && (mode)) {
      warn(141);
      return (0);
    }

    if ((!usercheck2()) && (mode)) {
      warn(142);
      return (0);
    }
#endif

    if ((currentscenario > topfantasoftsceanrio) && (!t)) // Fantasoft v7.1
    {
      getpref();
      if (MyrBitTstLong(&serial, 6 + divine)) /******* not registerd, no 3rd party scenarios ***/
      {
        warn(101);
        return (NIL);
      } else {
        GetMenuItemText(gGame, currentscenario, myString); /**** get codesegs ******/
        PtoCstr(myString);
        getfilename((Ptr)myString);
        if ((fp = MyrFopen(filename, "rb")) == NULL)
          scratch(700);
        fseek(fp, 5 * sizeof reclevel, SEEK_SET);
        fread(&codeseg1, 20, 1, fp);
        fread(&codeseg2, 20, 1, fp);
        fclose(fp);
      }
    }

    strcpy(filename, scenarioname); /************ load in custom solid data ***********/
    strcat((StringPtr)filename, "Data Solids");

    if ((fp = MyrFopen(filename, "rb")) == NULL) {
      fp = MyrFopen(filename, "w+b");
      for (t = 0; t < 1024; t++)
        solids[t] = 0;
      fwrite(&solids, sizeof solids, 1, fp);
    } else
      fread(&solids, sizeof solids, 1, fp);
    fclose(fp);

    if (customspellresnum > 0) //  Fantasoft v7.1 Begin
    {
      CloseResFile(customspellresnum);
      customspellresnum = -1;
    } //  Fantasoft v7.1 End

    strcpy(filename, scenarioname); /************ load in custom spells data ***********/
    strcat((StringPtr)filename, "Data Spell");

    if ((fp = MyrFopen(filename, "rb")) != NULL) {
      fread(&spelldata[4], sizeof spellinfo * 105, 1, fp);
      CvtTabSpellToPc(&spelldata[4], 105);
      fclose(fp);
      CtoPstr(filename);
      customspellresnum = MyrOpenResFile((StringPtr)filename);
    }

    return (TRUE);
  } else {
    if (mode)
      warn(61);
    return (FALSE);
  }
  return (TRUE);
}

/**************** updateshopwings *************/
void updateshopwings(int cl, int cr) {
  Rect itemRect;
  struct character character[2];

  if (!screensize)
    return;

  if (cl == -2)
    character[0] = characterl;
  else
    character[0] = c[cl];
  if (cr == -2)
    character[1] = characterr;
  else if (cr != -1)
    character[1] = c[cr];

  itemRect.top = 387;
  itemRect.bottom = itemRect.top + 73;
  TextFont(font);
  TextFace(0);
  TextMode(1);
  TextSize(12);

  ForeColor(yellowColor);
  itemRect.left = 0;
  itemRect.right = 80;
  pict(217, itemRect);
  MoveTo(50, 446);
  string(character[0].numitems);
  MoveTo(10, 426);
  string(character[0].load);
  MoveTo(42, 426);
  string(character[0].loadmax);

  itemRect.left = 720;
  itemRect.right = 800;
  pict(217, itemRect);

  if (cr != -1) {
    MoveTo(768, 446);
    string(character[1].numitems);
    MoveTo(730, 426);
    string(character[1].load);
    MoveTo(762, 426);
    string(character[1].loadmax);
  }

  return;
}
