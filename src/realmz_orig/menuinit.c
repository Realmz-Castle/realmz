#include "prototypes.h"
#include "realmzbuild.h"
#include "variables.h"

/***************************** MenuInit ********************************/
void MenuInit(void) {
  short menucounter;
  short t = 0;
  Str255 holdversion;
  FILE* fp = NULL;

  copywright = GetNewMBar(129);
  SetMenuBar(copywright);
  copy = GetMenuHandle(133);

  strcpy(holdversion, theString);

  myMenuBar = GetNewMBar(128);
  SetMenuBar(myMenuBar);
  gScenario = GetMenuHandle(200);
  gApple = GetMenuHandle(128);
  gFile = GetMenuHandle(129);
  gGame = GetMenuHandle(130);
  gOptions = GetMenuHandle(139);
  gParty = GetMenuHandle(136);
  gBeast = GetMenuHandle(132);
  prefer = GetMenuHandle(137);
  gNPC = GetMenuHandle(146);
  musicmenu = GetMenuHandle(145);

  gSound = GetMenu(135);
  InsertMenu(gSound, -1);

  gSpeed = GetMenu(134);
  InsertMenu(gSpeed, -1);

  for (t = 1; t < 22; t++) /******** fill in maps menu with blank titles *****/
  {
    MyrAppendMenu(gScenario, (Ptr) "------------");
    DisableItem(gScenario, t + 3);
  }

  if (doreg()) {
    for (menucounter = 1; menucounter < 100; menucounter++) /******** add divinity scenario names to scenario menu *****/
    {
      GetIndString(myString, 3, 1);
      GetIndString(myString, -6003 - divine, menucounter);
      if (StringWidth(myString)) {
        AppendMenu(gGame, myString);
        DisableItem(gGame, menucounter + 23);
      }
    }

    setfree(serial); /*** check for pirated code numbers ****/
  } else if (!seenit) {
    aboutrealmz();
    seenit = 1;
  }

  updatescenarioavail();

  DisableItem(gScenario, 0);
  SetItemMark(gSpeed, oldspeed, 19);
  SetItemMark(gSound, volume + 2, 19);
  SetItemMark(gSound, musicvolume + 12, 19);

  for (t = 0; t < 20; t++)
    SetItemMark(musicmenu, t + 8, 19);

  CheckItem(musicmenu, 1, 1 - nomusic);

  if (!divine) {
    currentscenario = 10;
    if (currentscenariohold > 10)
      currentscenario = currentscenariohold;
    CheckItem(gGame, currentscenario, 1);
  } else {
    currentscenario = 7;
    if (currentscenariohold > 20)
      currentscenario = currentscenariohold;
    CheckItem(gGame, currentscenario, 1);
  }

  // Display versions of Realmz, Items, Spells, Castes, Races, Character editor
  //***************************************************************************

  GetMenuItemText(prefer, 10, myString); /***** Realmz *********/
  PtoCstr(myString);
  GetVersStr(1, Appl_Rsrc_Fork_Ref_Num); /*** load in version of Realmz ***/
  PtoCstr(theString);
  strcat(myString, theString);
  CtoPstr((Ptr)myString);
  SetMenuItemText(prefer, 10, myString);

  //***************************************************************************

  itemrefnum = MyrOpenResFile((Ptr) "\p:Data Files:Data ID"); /* open items resource for item names strings ****/
  GetMenuItemText(prefer, 11, myString); /***** Items  *********/
  GetVersStr(5, 0);
  PtoCstr(myString);
  PtoCstr(theString);
  if (itemrefnum != -1)
    strcat(myString, theString);
  else
    strcat(myString, (StringPtr) "Unknown");
  CtoPstr((Ptr)myString);
  SetMenuItemText(prefer, 11, myString);

  //***************************************************************************

  temp = MyrOpenResFile((Ptr) "\p:Data Files:Data S"); /* open spells resource for item names strings ****/
  GetMenuItemText(prefer, 12, myString); /***** spells  *********/
  GetVersStr(6, 0);
  PtoCstr(myString);
  PtoCstr(theString);
  if (temp != -1)
    strcat(myString, theString);
  else
    strcat(myString, (StringPtr) "Unknown");
  CtoPstr((Ptr)myString);
  SetMenuItemText(prefer, 12, myString);
  if (temp != -1)
    CloseResFile(temp);

  //***************************************************************************

  temp = MyrOpenResFile((Ptr) "\p:Data Files:Data Race"); /* open races resource  ****/
  GetMenuItemText(prefer, 13, myString); /***** races  *********/
  GetVersStr(7, 0);
  PtoCstr(myString);
  PtoCstr(theString);
  if (temp != -1)
    strcat(myString, theString);
  else
    strcat(myString, (StringPtr) "Unknown");
  CtoPstr((Ptr)myString);
  SetMenuItemText(prefer, 13, myString);
  if (temp != -1)
    CloseResFile(temp);

  //***************************************************************************

  temp = MyrOpenResFile((Ptr) "\p:Data Files:Data Caste"); /* open castes resource ****/
  GetMenuItemText(prefer, 14, myString); /***** castes  *********/
  GetVersStr(8, 0);
  PtoCstr(myString);
  PtoCstr(theString);
  if (temp != -1)
    strcat(myString, theString);
  else
    strcat(myString, (StringPtr) "Unknown");
  CtoPstr((Ptr)myString);
  SetMenuItemText(prefer, 14, myString);
  if (temp != -1)
    CloseResFile(temp);

  //***************************************************************************

  GetMenuItemText(prefer, 15, myString); /***** The Family Jewels  *********/
  GetVersStr(9, jewelsrefnum);
  PtoCstr(myString);
  PtoCstr(theString);
  strcat(myString, theString);
  CtoPstr((Ptr)myString);
  SetMenuItemText(prefer, 15, myString);

  GetMenuItemText(prefer, 16, myString); /***** PC Editor *********/
  PtoCstr(myString);
  temp = -1;

  if (temp == -1)
    temp = MyrOpenResFile((Ptr) "\p:Character Editor 10.0 DEMO");
  if (temp == -1)
    temp = MyrOpenResFile((Ptr) "\p:Character Editor 10.1 DEMO");
  if (temp == -1)
    temp = MyrOpenResFile((Ptr) "\p:Character Editor 10.0.1 DEMO");

  if (temp != -1) {
    GetVersStr(1, temp);
    if (temp > 0)
      CloseResFile(temp);
    temp = -1;
    PtoCstr(theString);
    strcat(myString, theString);
  } else
    strcat(myString, (StringPtr) "Unknown");

  CtoPstr((Ptr)myString);
  SetMenuItemText(prefer, 16, myString);
}

/**************************** updatemonstermenu *************/
void updatemonstermenu(short currentload) {
  static short lastload;
  short filecount;
  short numofitems, t, tt, temp;
  char name1[255], name2[255];
  FILE* fp = NULL;
  Boolean quickload;

updatenewfile:

  filecount = count = quickload = 0;

  numofitems = CountMItems(gBeast);

  getfilename("Data MENU");

  if ((fp = MyrFopen(filename, "rb")) == NULL)
    goto neednewfile;
  {
    if (!fread(&menupos, sizeof menupos, 1, fp)) {
      fclose(fp);
      goto neednewfile;
    }
    CvtTabShortToPc(&menupos, 251);
    quickload = TRUE;
    fclose(fp);
  }

neednewfile:

  if (lastload != currentload) {
    getfilename("Data MD");
    if ((fp = MyrFopen(filename, "rb")) == NULL)
      scratch(116);

    if (quickload) {
      for (t = 0; t < 250; t++) {
        if (menupos[t]) {
          count++;

          fseek(fp, (menupos[t] - 1) * sizeof monpick, SEEK_SET);
          /* !MYRIAD 12/10/99 Because fseek can be greater than the end of the file (and fread can fail)*/
          if (fread(&monpick, sizeof monpick, 1, fp) == 1)
            CvtMonsterToPc(&monpick);
          /* !MYRIAD 12/10/99 If not read, keeps the previous value of monpick */

          if ((monpick.hd) && (!monpick.notonmenu)) {
            if (monpick.hd == 255)
              goto finish;
            strcpy((StringPtr)gotword, monpick.monname);
            CtoPstr(gotword);

            if (numofitems < count)
              AppendMenu(gBeast, (StringPtr)gotword);
            else
              SetMenuItemText(gBeast, count, (StringPtr)gotword);
          }
        }
      }
    } else {
      flashmessage((StringPtr) "Loading and sorting bestiary.", 50, 70, -1, 0);

      while (fread(&monpick, sizeof monpick, 1, fp)) {
        filecount++;
        CvtMonsterToPc(&monpick);
        strcpy((StringPtr)gotword, (StringPtr) "");

        if (count > 250)
          goto finish;

        if ((monpick.hd) && (!monpick.notonmenu)) {
          if (monpick.hd == 255)
            goto finish;

          count++;
          menupos[count] = filecount;
          strcpy((StringPtr)gotword, monpick.monname);
          CtoPstr(gotword);
          if (numofitems < count)
            AppendMenu(gBeast, (StringPtr)gotword);
          else
            SetMenuItemText(gBeast, count, (StringPtr)gotword);
        }
      }
    }

  finish:

    fclose(fp);
    lastload = currentload;

    if (!quickload) {

      for (t = 1; t < count; t++) {
        for (tt = 1; tt < count; tt++) {
          GetMenuItemText(gBeast, tt, (StringPtr)name1);
          GetMenuItemText(gBeast, tt + 1, (StringPtr)name2);

          PtoCstr((StringPtr)name1);
          PtoCstr((StringPtr)name2);

          if ((strlen(name1)) && (strlen(name2))) {
            if (strcmp(name1, name2) > 0) {
              temp = menupos[tt];
              menupos[tt] = menupos[tt + 1];
              menupos[tt + 1] = temp;
              CtoPstr(name1);
              CtoPstr(name2);
              SetMenuItemText(gBeast, tt + 1, (StringPtr)name1);
              SetMenuItemText(gBeast, tt, (StringPtr)name2);
            }
          }
        }
      }
      getfilename("Data MENU");
      if ((fp = MyrFopen(filename, "w+b")) == NULL)
        scratch(117);
      CvtTabShortToPc(&menupos, 251);
      fwrite(&menupos, sizeof menupos, 1, fp);
      CvtTabShortToPc(&menupos, 251);
      fclose(fp);
      setfileinfo("scen", filename);
      flashmessage((StringPtr) "", 50, 70, -1, 0);
      lastload = -1;
      goto updatenewfile;
      lastload = -1;
    }
  }
}
