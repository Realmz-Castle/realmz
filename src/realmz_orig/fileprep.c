#include "prototypes.h"
#include "variables.h"

/************************************* fileprep ***********************/
short fileprep(short mode) {
  short oldchoice;
  Str255 descript;
  FILE* fp = NULL;
  char hold[80];
  short savedscenario;
  DialogRef savewindow;

  if (lastgame < 4)
    lastgame = 4;

  oldchoice = lastgame;
  needdungeonupdate = TRUE;

  if (mode == -1)
    return (lastgame);

  SetCCursor(sword);
  savewindow = GetNewDialog(139, 0L, (WindowPtr)-1L);
  SetPortDialogPort(savewindow);
  BackPixPat(base);
  TextFont(defaultfont);
  ForeColor(yellowColor);
  gCurrent = savewindow;

  MoveWindow(GetDialogWindow(savewindow), GlobalLeft + 40 + (leftshift / 2), GlobalTop + 20 + (downshift / 2), FALSE);
  ShowWindow(GetDialogWindow(savewindow));
  ErasePortRect();
  DrawDialog(savewindow);
  BeginUpdate(GetDialogWindow(savewindow));
  EndUpdate(GetDialogWindow(savewindow));

  if (!reducesound)
    sound(10004);
  gStop = FALSE;

  GetDialogItem(savewindow, 3, &itemType, &itemHandle, &itemRect);
  pict(185 + mode, itemRect);
  GetDialogItem(savewindow, lastgame, &itemType, &itemHandle, &itemRect);
  InsetRect(&itemRect, 2, 2);
  DrawPicture(on, &itemRect);

update:

  if ((fp = MyrFopen(":Save:Game Descriptions", "rb")) != NULL) {
    fseek(fp, ((oldchoice - 4) * sizeof descript) + 40, SEEK_SET);
    if (fread(&descript, sizeof descript, 1, fp)) {
      MyrStrPtoC((Ptr)descript);
      MyrPascalDiStr(41, descript);
    } else
      MyrCDiStr(41, (StringPtr) "No Game Description");
    fclose(fp);
  } else {
    MyrCDiStr(41, (StringPtr) "No Game Description");
    if ((fp = MyrFopen(":Save:Game Descriptions", "w+b")) != NULL) {
      fclose(fp);
      setfileinfo("save", ":Save:Game Descriptions");
      goto update;
    }
  }

  strcpy((StringPtr)hold, (StringPtr) ":Save:Game ");

  switch (oldchoice) {
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
    case 14:
      strcat((StringPtr)hold, (StringPtr) "K:");
      break;
  }
  strcpy((StringPtr)filename, (StringPtr)hold);
  strcat((StringPtr)filename, "Data I1");

  fp = NULL;

  if ((fp = MyrFopen(filename, "rb")) == NULL) {
    strcpy(myString, (StringPtr) "No saved game here.");
    CtoPstr((Ptr)myString);
  } else {
    fseek(fp, sizeof c, SEEK_SET);
    fread(&savedscenario, sizeof savedscenario, 1, fp);
    CvtShortToPc(&savedscenario);
    fclose(fp);
    GetMenuItemText(gGame, savedscenario, myString);
  }

  MyrPascalDiStr(43, myString);

  while (gStop == FALSE) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(savewindow, itemHit, &itemType, &itemHandle, &buttonrect);

    if (itemHit == 2) {
      ploticon3(129, buttonrect);
      sound(141);
      oldchoice = 0;
      goto cancel;
    }

    if (((itemHit > 2) && (itemHit < 14)) || (itemHit == 1)) {
      GetDialogItem(savewindow, 41, &itemType, &itemHandle, &itemRect);
      GetDialogItemText(itemHandle, descript);

      if ((fp = MyrFopen(":Save:Game Descriptions", "r+b")) != NULL) {
        fseek(fp, ((oldchoice - 4) * sizeof descript) + 40, SEEK_SET);
        MyrStrCtoP((Ptr)descript); // Myriad: write in pascal format
        fwrite(&descript, sizeof descript, 1, fp);
        fclose(fp);
      }
    }

    if (itemHit == 14) /**** erase files ****/
    {
      ploticon3(129, buttonrect);
      if (question(12)) {
        strcpy((StringPtr)filename, (StringPtr)hold);
        strcat((StringPtr)filename, "Data A1");
        MyrRemove(filename);

        strcpy((StringPtr)filename, (StringPtr)hold);
        strcat((StringPtr)filename, "Data B1");
        MyrRemove(filename);

        strcpy((StringPtr)filename, (StringPtr)hold);
        strcat((StringPtr)filename, "Data C1");
        MyrRemove(filename);

        strcpy((StringPtr)filename, (StringPtr)hold);
        strcat((StringPtr)filename, "Data D1");
        MyrRemove(filename);

        strcpy((StringPtr)filename, (StringPtr)hold);
        strcat((StringPtr)filename, "Data E1");
        MyrRemove(filename);

        strcpy((StringPtr)filename, (StringPtr)hold);
        strcat((StringPtr)filename, "Data F1");
        MyrRemove(filename);

        strcpy((StringPtr)filename, (StringPtr)hold);
        strcat((StringPtr)filename, "Data G1");
        MyrRemove(filename);

        strcpy((StringPtr)filename, (StringPtr)hold);
        strcat((StringPtr)filename, "Data H1");
        MyrRemove(filename);

        strcpy((StringPtr)filename, (StringPtr)hold);
        strcat((StringPtr)filename, "Data I1");
        MyrRemove(filename);

        strcpy((StringPtr)filename, (StringPtr)hold);
        strcat((StringPtr)filename, "Data TD3");
        MyrRemove(filename);
      }
      SetPortDialogPort(savewindow);
      gCurrent = savewindow;
      GetDialogItem(savewindow, 14, &itemType, &itemHandle, &buttonrect);
      ploticon3(130, buttonrect);
      goto update;
    }

    if ((itemHit > 3) && (itemHit < 14)) {
      GetDialogItem(savewindow, oldchoice, &itemType, &itemHandle, &itemRect);
      pict(179, itemRect);
      sound(-144);

      GetDialogItem(savewindow, itemHit, &itemType, &itemHandle, &itemRect);
      InsetRect(&itemRect, 2, 2);
      DrawPicture(on, &itemRect);
      oldchoice = itemHit;
      sound(145);
      goto update;
    }

    if ((itemHit == 3) || (itemHit == 1)) {
      GetDialogItem(savewindow, 3, &itemType, &itemHandle, &itemRect);
      ploticon3(129, itemRect);
      sound(141);
      goto out;
    }
  }
out:
  lastgame = oldchoice;
  savepref();
cancel:
  if (!reducesound)
    sound(3000);

  DisposeDialog(savewindow);

  if (FrontWindow() == GetDialogWindow(background)) {
    SetPortDialogPort(background);
    gCurrent = background;
    BackColor(blackColor);
  }

  itemHit = key = gTheEvent.message = 0;
  FlushEvents(everyEvent, 0);
  return (oldchoice);
}
