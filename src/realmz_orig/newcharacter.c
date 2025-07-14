#include "prototypes.h"
#include "variables.h"

/************************* pin *********************/
short pin(short data, short low, short high) {
  if (data < low)
    data = low;
  if (data > high)
    data = high;
  return (data);
}

/******************************* addinitialitems *****************************/
void addinitialitems(void) {
  short t, index = 0;

  for (t = 0; t < 20; t++) {
    if (caste.startitems[t]) {
      characterl.numitems++;
      loaditem(caste.startitems[t]);

      if ((characterl.load + item.charge * item.xcharge + item.wieght) <= characterl.loadmax) {
        characterl.items[index].id = caste.startitems[t];
        characterl.items[index].ident = TRUE;
        characterl.items[index].equip = FALSE;
        characterl.items[index].charge = item.charge;
        characterl.load += (item.charge * item.xcharge) + item.wieght;
        index++;
      }
    }
  }

  c[0] = characterl;
  for (t = 0; t < c[0].numitems; t++)
    wear(0, t, 0);
  characterl = c[0];
  characterl.money[0] = caste.startmoney;
  characterl.load += caste.startmoney;
}

/***************************** newcharacter ********************************/
void NewCharacter(short level) {
  DialogRef aging;
  short t, tempdice, temp2, offset = 0;
  short tempstat1, tempstat2, tempstat3, tempstat4;

  loadprofile(characterl.race, characterl.caste);

  for (t = 0; t < 7; t++) {
    characterl.nspells[t] = 0;
    for (tt = 0; tt < 12; tt++) {
      characterl.cspells[t][tt] = 0;
    }
  }

  for (t = 0; t < 20; t++)
    characterl.armor[t] = 0;
  for (t = 0; t < 40; t++)
    characterl.condition[t] = races.conditions[t];
  for (t = 0; t < 40; t++) {
    if (caste.conditions[t] == 1)
      characterl.condition[t] = -1;
  }
  for (t = 0; t < 30; t++)
    characterl.items[t].charge = characterl.items[t].id = characterl.items[t].ident = characterl.items[t].equip = 0;

  characterl.weaponsound = (30 + characterl.gender * 8);
  characterl.prestigepenelty = (10 * (level * level));

  gGeneration = GetNewDialog(128, 0L, (WindowPtr)-1L);
  MoveWindow(GetDialogWindow(gGeneration), GlobalLeft, GlobalTop, FALSE);
  ShowWindow(GetDialogWindow(gGeneration));
  gCurrent = gGeneration;
  SetPortDialogPort(gGeneration);
  BackPixPat(base);
  ErasePortRect();
  DrawDialog(gGeneration);
  BeginUpdate(GetDialogWindow(gGeneration));
  EndUpdate(GetDialogWindow(gGeneration));

  GetDialogItem(gGeneration, 68, &itemType, &itemHandle, &itemRect);
  plotportrait(characterl.pictid, itemRect, characterl.caste, -1);

  GetDialogItem(gGeneration, 74, &itemType, &itemHandle, &itemRect);
  ploticon3(characterl.iconid, itemRect);

waynew:

  for (t = 0; t < 15; t++)
    characterl.spec[t] = 0;
  for (t = 0; t < 12; t++)
    characterl.special[t] = 0;
  for (t = 0; t < 8; t++)
    characterl.save[t] = 50 + races.drvbonus[t] + caste.drvbonus[t];
  for (t = 0; t < 8; t++)
    characterl.special[t] = races.plusminustohit[t];

  characterl.movebonus = characterl.magres = characterl.missile = characterl.nohands = characterl.numitems = characterl.twohand = characterl.nspells[0] = 0;
  characterl.inbattle = characterl.level = 1;
  characterl.damagetaken = characterl.damagegiven = characterl.hitsgiven = characterl.hitstaken = characterl.imissed = characterl.umissed = 0;
  characterl.kills = characterl.deaths = characterl.knockouts = characterl.spellscast = characterl.destroyed = characterl.turns = 0;
  characterl.normattacks = characterl.attacks = races.numofattacks[0] + caste.bonusattacks;
  getexp(level + 1, characterl.caste, -1);
  characterl.handtohand = caste.hand2hand[0];

  for (;;) {
    characterl.maglu = characterl.magst = characterl.ac = 0; //***** Fantasoft v7.1
    characterl.tohit = caste.tohit[0];
    characterl.damage = characterl.money[0] = characterl.money[1] = characterl.money[2] = characterl.load = 0;

    for (t = 0; t < 7; t++) {
      offset = 0;
      if (t == 6)
        offset = -1;

      temp2 = randrange(1, 18) + races.attbonus[offset + t] + caste.attbonus[offset + t];
      temp2 = pin(temp2, caste.minmax[(offset + t) * 2], caste.minmax[1 + (offset + t) * 2]);
      temp2 = pin(temp2, races.minmax[(offset + t) * 2], races.minmax[1 + (offset + t) * 2]);

      switch (t) {
        case 0: /**** brawn *****/
          characterl.st = temp2;
          break;

        case 1: /**** knowledge *****/
          characterl.in = temp2;
          break;

        case 2: /**** judgement *****/
          characterl.wi = temp2;
          break;

        case 3: /**** agility *****/
          characterl.de = temp2;
          break;

        case 4: /**** vitality *****/
          characterl.co = temp2;
          break;

        case 5: /**** luck *****/
          characterl.lu = temp2;
          break;
      }
    }

    if (characterl.st > 25) {
      warn(111);
      exit(0);
    }

    if (characterl.gender == 2) {
      characterl.st--;
      characterl.de++;
      characterl.wi++;
    } else {
      characterl.st++;
      characterl.de--;
    }
    characterl.currentagegroup = 0;
    applyage(characterl.race, 1, 1); /*********** applies initial Youth age parameters ***/
    if (caste.minimumagegroup > 1)
      applyage(characterl.race, 2, 1); /*********** applies initial Young age parameters ***/
    if (caste.minimumagegroup > 2)
      applyage(characterl.race, 3, 1); /*********** applies initial Adult age parameters ***/
    if (caste.minimumagegroup > 3)
      applyage(characterl.race, 4, 1); /*********** applies initial Prime age parameters ***/
    if (caste.minimumagegroup > 4)
      applyage(characterl.race, 5, 1); /*********** applies initial Senior age parameters ***/

    for (t = 0; t < 7; t++) {
      offset = 0;
      if (t == 6)
        offset = -1;

      tempstat1 = caste.minmax[(offset + t) * 2];
      tempstat2 = caste.minmax[1 + (offset + t) * 2];
      tempstat3 = races.minmax[(offset + t) * 2];
      tempstat4 = races.minmax[1 + (offset + t) * 2];
      switch (t) {
        case 0: /**** brawn *****/
          characterl.st = pin(characterl.st, tempstat1, tempstat2);
          characterl.st = pin(characterl.st, tempstat3, tempstat4);
          break;

        case 1: /**** knowledge *****/
          characterl.in = pin(characterl.in, tempstat1, tempstat2);
          characterl.in = pin(characterl.in, tempstat3, tempstat4);
          break;

        case 2: /**** judgement *****/
          characterl.wi = pin(characterl.wi, tempstat1, tempstat2);
          characterl.wi = pin(characterl.wi, tempstat3, tempstat4);
          break;

        case 3: /**** agility *****/
          characterl.de = pin(characterl.de, tempstat1, tempstat2);
          characterl.de = pin(characterl.de, tempstat3, tempstat4);
          break;

        case 4: /**** vitality *****/
          characterl.co = pin(characterl.co, tempstat1, tempstat2);
          characterl.co = pin(characterl.co, tempstat3, tempstat4);
          break;

        case 5: /**** luck *****/
          characterl.lu = pin(characterl.lu, tempstat1, tempstat2);
          characterl.lu = pin(characterl.lu, tempstat3, tempstat4);
          break;
      }
    }

    if (Rand(100) > 80)
      characterl.special[randrange(0, 7)]++;
    if (Rand(100) > 90)
      characterl.special[randrange(0, 7)]++;
    if (Rand(100) > 95)
      characterl.special[randrange(0, 7)]++;

    characterl.version = -3; /********* -3 is the first version with the new casts ***/

    tempdice = caste.stamina[0];

    characterl.staminamax = Rand(tempdice);

    temp = 0;
    if (characterl.co > 16) {
      temp = characterl.co - 16;
      if (temp > caste.maxstaminabonus)
        temp = caste.maxstaminabonus;
    }
    characterl.staminamax += temp;

    strength(characterl.st);

    characterl.damage += damage;
    characterl.tohit += temp;

    characterl.magres = (characterl.in + characterl.wi) / 10;

    characterl.magres *= caste.magres; /************ caste.magres must be 1 or higher *********/

    characterl.ac = 0;
    if (characterl.de > 14)
      characterl.ac -= 2 * (14 - characterl.de);

    characterl.dodge = 2 * characterl.de + caste.dodge[0];

    characterl.age = randrange(races.agerange[caste.minimumagegroup - 1][0], races.agerange[caste.minimumagegroup - 1][1]);
    characterl.magres += races.magres;
    characterl.twohand = races.twohand + caste.twohand;
    characterl.missile = (races.missile + caste.missile[0]);

    if (!caste.canusemissile)
      characterl.missile = 0;

    characterl.movementmax = races.basemove + caste.movebonus;

    for (t = 0; t < 8; t++) {
      if (characterl.save[t] < -99)
        characterl.save[t] = -99;
      if (characterl.save[t] > 120)
        characterl.save[t] = 120;
    }

    characterl.age *= 365;
    characterl.spellcastertype = characterl.spellpoints = characterl.spellpointsmax = 0;

    /*********** set spellcastertype for level up for higher starters ************/
    if (caste.spellcasters[0][1]) {
      characterl.spellcastertype = 1;
      if ((level + 1) >= caste.spellcasters[0][1])
        characterl.spellpointsmax = 4 + characterl.in + (Rand(characterl.wi)); /******* scorcerer ***/
    }
    if (caste.spellcasters[1][1]) {
      characterl.spellcastertype = 2;
      if ((level + 1) >= caste.spellcasters[1][1])
        characterl.spellpointsmax = 4 + characterl.wi + (Rand(characterl.in)); /******* preist ***/
    }
    if (caste.spellcasters[2][1]) {
      characterl.spellcastertype = 3;
      if ((level + 1) >= caste.spellcasters[2][1])
        characterl.spellpointsmax = 10 + Rand(characterl.wi + characterl.in); /******* enchanter ***/
    }

    movecalc(-1);
    characterl.stamina = characterl.staminamax;
    characterl.spellpoints = characterl.spellpointsmax;

    updatespec(1); /****** 1 = Initialize Spec *******/

    c[0] = characterl; /********** do levelup for advanced characters ***********/
    if (level)
      for (t = 0; t < level; t++)
        levelup(0, TRUE);
    characterl = c[0];

  backup:

    ShowStats(0);

    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(gGeneration, itemHit, &itemType, &itemHandle, &buttonrect);

    sound(130);

    if ((itemHit == 17) || (itemHit == 1)) {
      ploticon3(133, buttonrect);
      return;
    }

    if (itemHit == 4) {
      downbutton(FALSE);
      showspecial();
      SetPortDialogPort(gGeneration);
      gCurrent = gGeneration;
      goto backup;
    }

    if (itemHit == 70) /******** view aging **********/
    {
      sound(3000);
      loadprofile(c[charselectnew].race, 0);
      aging = GetNewDialog(191, 0L, (WindowPtr)-1L);
      SetPortDialogPort(aging);
      gCurrent = aging;
      MoveWindow(GetDialogWindow(aging), GlobalLeft, GlobalTop, FALSE);
      ShowWindow(GetDialogWindow(aging));
      ErasePortRect();

      TextFace(bold);
      TextFont(font);
      ForeColor(yellowColor);
      BackPixPat(base);
      TextSize(18);

      DrawDialog(aging);
      BeginUpdate(GetDialogWindow(aging));
      EndUpdate(GetDialogWindow(aging));

      MyrCDiStr(7, (StringPtr) "Youth");
      MyrCDiStr(8, (StringPtr) "Young");
      MyrCDiStr(9, (StringPtr) "Prime");
      MyrCDiStr(10, (StringPtr) "Adult");
      MyrCDiStr(11, (StringPtr) "Senior");

      for (t = 0; t < 5; t++) {
        temp = c[charselectnew].age / 365;
        if (twixt(temp, races.agerange[t][0], races.agerange[t][1]))
          ForeColor(whiteColor);
        else
          ForeColor(yellowColor);
        TextSize(16);
        DialogNum(12 + t * 17, races.agerange[t][0]);
        DialogNum(13 + t * 17, races.agerange[t][1]);

        TextSize(18);
        for (tt = 0; tt < 15; tt++)
          DialogNumNZ(14 + (t * 17) + tt, races.agechange[t][tt]);
      }

      FlushEvents(everyEvent, 0);
      ModalDialog(0L, &itemHit);
      DisposeDialog(aging);
      SetPortDialogPort(gGeneration);
      ErasePortRect();
      DrawDialog(gGeneration);
      GetDialogItem(gGeneration, 68, &itemType, &itemHandle, &itemRect);
      plotportrait(characterl.pictid, itemRect, characterl.caste, -1);

      GetDialogItem(gGeneration, 74, &itemType, &itemHandle, &itemRect);
      ploticon3(characterl.iconid, itemRect);
      BeginUpdate(GetDialogWindow(gGeneration));
      EndUpdate(GetDialogWindow(gGeneration));
      goto backup;
    }

    if (itemHit != 18)
      goto backup;
    else {
      ploticon3(133, buttonrect);
      ploticon3(134, buttonrect);
      goto waynew;
    }
  }
}

/****************************** HandleEvent ********************************/
void HandleEvent(void) {
  WindowPtr whichWindow;
  short a;

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

    if (gTheEvent.what == keyDown)
      goto dokey;
    if (gTheEvent.what == mouseDown)
      goto domouse;
  } else {
    switch (gTheEvent.what) {
      case (updateEvt):
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

      case keyDown:
      dokey:
        if (gTheEvent.modifiers & cmdKey) {
          commandkey = BitAnd(gTheEvent.message, charCodeMask);
          menuChoice = MenuKey(commandkey);
          HandleMenuChoice();
        }
        break;

      case (autoKey):
        break;

      case (mouseDown):
      domouse:

        thePart = FindWindow(gTheEvent.where, &whichWindow);
        switch (thePart) {
          case inMenuBar:
            menuChoice = MenuSelect(gTheEvent.where);
            HandleMenuChoice();
            if (revertgame)
              return;
            break;
        }
        break;

      case osEvt:
        if ((gTheEvent.message >> 24) == suspendResumeMessage) {
          if (hide) {
            compactheap();
            SetPortDialogPort(background);
            BackColor(blackColor);
            hide = FALSE;
            DrawDialog(background);
            TextFont(font);
            FlushEvents(everyEvent, 0);
          } else
            hide = TRUE;
        }
        break;
    }
  }
}

/***************************** CharacterGen ********************************/
void CharacterGen(void) {
  short temp;

  HiliteMenu(0);

  if (!Name(0)) {
    SetPortDialogPort(background);
    BackColor(blackColor);
    DrawDialog(background);
    return;
  }

  music(5); /*********** create music *********/
  temp = Startlevel();

  Gender();
  Caste(0);
  Race(0);
  skip = TRUE;
  charselectnew = 0;

  characterl.pictid = 251 + (races.defaulticonset * 6);
  portrait(1);

  characterl.iconid = 9000 - 257 + (characterl.pictid);
  iconpicture(1);

  NewCharacter(temp);

  if (characterl.spellpointsmax) {
    for (t = 0; t < 7; t++)
      for (tt = 0; tt < 12; tt++)
        characterl.cspells[t][tt] = 0;
    Spellselect();
    gCurrent = gGeneration;
    ShowStats(0);
  }

  addinitialitems();

  if (question(2))
    savecharacter(-1);
  else
    minus(characterl.name, 0);

  SetPortDialogPort(background);
  BackColor(blackColor);
  DisposeDialog(gGeneration);
  DrawDialog(background);
}
