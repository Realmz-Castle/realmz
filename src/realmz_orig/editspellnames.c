#include "prototypes.h"
#include "variables.h"

/********************** SetIndString ***************/
OSErr SetIndString(StringPtr theStr, short resID, short strIndex) {
  Handle theRes; /* handle pointing to STR# resource */
  short numStrings; /* number of strings in STR# */
  short ourString; /* counter to index up to strIndex */
  char* resStr; /* string pointer to STR# string to replace */
  long oldSize; /* size of STR# resource before call */
  long newSize; /* size of STR# resource after call */
  unsigned long offset; /* resource offset to str to replace*/

  /* make sure resource exists */

  MyrStrCtoP((Ptr)theStr); // Myriad

  theRes = GetResource('STR#', resID);

  if (ResError() != noErr)
    return ResError();

  if (!theRes || !(*theRes))
    return resNotFound;

  HLock(theRes);
  HNoPurge(theRes);

  /* get # of strings in STR# */

  BlockMove(*theRes, &numStrings, sizeof(short));

#ifdef PC // Myriad: Set to PC mode
  numStrings = wintel2moto(numStrings);
#endif

  if (strIndex > numStrings) {
    return resNotFound;
  }

  // MyrStrCtoP(theStr);//Myriad // Fantasoft 7.1  Not needed

  offset = sizeof(short); /* get a pointer to the string to replace */
  resStr = (char*)*theRes + sizeof(short);

  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(jpetrie): The original implementation of the remainder of this function used "str[0]" indexing to get the
   * size of a variety of Pascal strings. Since string types are signed now, each of those lines have been updated to
   * cast to uint8_t to ensure that longer strings don't result in bugs because their length byte was interpreted as a
   * negative value.
   */
  for (ourString = 1; ourString < strIndex; ourString++) {
    offset += 1 + (uint8_t)resStr[0];
    resStr += 1 + (uint8_t)resStr[0];
  }

  oldSize = GetHandleSize(theRes); /* grow/shrink resource handle to make room for new string */
  newSize = oldSize - (uint8_t)resStr[0] + (uint8_t)theStr[0];
  HUnlock(theRes);
  SetHandleSize(theRes, newSize);

  if (MemError() != noErr) {
    ReleaseResource(theRes);
    return -1;
  }

  HLock(theRes);
  resStr = *theRes + offset;

  /* move old data forward/backward to make room */
  BlockMove(resStr + (uint8_t)resStr[0] + 1, resStr + (uint8_t)theStr[0] + 1, oldSize - offset - (uint8_t)resStr[0] - 1);

  /* move new data in */
  BlockMove(theStr, resStr, (uint8_t)theStr[0] + 1);

  /* write resource out */

  ChangedResource(theRes);
  WriteResource(theRes);
  HPurge(theRes);
  ReleaseResource(theRes);

  MyrStrPtoC((Ptr)theStr); // Myriad

  return ResError();
}

/****************** editspellnames ******************************/
void editspellnames(void) {
  short spelllevel, oldlevel, oldspell = 10;
  short oldhit, t, classindex = 1;
  short spellselectpoints = 0;
  DialogRef spellwindow;

  if (usecustomnames == -1) {
    warn(126);
    return;
  }

  sound(3000);

  oldlevel = 0;

  spellwindow = GetNewDialog(130, 0L, (WindowPtr)-1L);
  MoveWindow(GetDialogWindow(spellwindow), GlobalLeft + 1, GlobalTop + 1, FALSE);
  ShowWindow(GetDialogWindow(spellwindow));
  SetPortDialogPort(spellwindow);
  BackPixPat(base);
  ErasePortRect();
  gCurrent = spellwindow;
  ForeColor(yellowColor);

  DrawDialog(spellwindow);
  BeginUpdate(GetDialogWindow(spellwindow));
  EndUpdate(GetDialogWindow(spellwindow));

  ForeColor(yellowColor);
  TextSize(16);
  GetIndString(myString, 131, classindex + 5);
  MyrPascalDiStr(33, myString);

  TextSize(12);

  spelllevel = 0;
  oldhit = 10;

  GetDialogItem(spellwindow, 9, &itemType, &itemHandle, &itemRect);
  pict(130, itemRect);
  GetDialogItem(spellwindow, 2, &itemType, &itemHandle, &buttonrect);
  downbutton(FALSE);

  ForeColor(yellowColor);
  for (t = 0; t < 12; t++) {
    GetDialogItem(spellwindow, 10 + t, &itemType, &itemHandle, &buttonrect);
    buttonrect.left -= 2;
    buttonrect.top--;
    upbutton(FALSE);
    GetIndString(myString, 1000 * (classindex) + spelllevel, t + 1);
    MyrPascalDiStr(10 + t, myString);
  }
  GetIndString(myString, -(1000 * classindex + spelllevel), 1);
  MyrPascalDiStr(39, myString);
  RGBForeColor(&cyancolor);
  GetDialogItem(spellwindow, 10, &itemType, &itemHandle, &buttonrect);
  GetDialogItemText(itemHandle, myString);
  buttonrect.left -= 2;
  buttonrect.top--;
  downbutton(FALSE);
  MyrPascalDiStr(10, myString);

  for (;;) {
  over:
    SetPortDialogPort(spellwindow);
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(spellwindow, itemHit, &itemType, &itemHandle, &buttonrect);

    if ((itemHit == 1) || (itemHit == 32)) {
      /* Myriad, modified almost :o) on request 08/11/99 */
      GetDialogItem(spellwindow, oldspell, &itemType, &itemHandle, &buttonrect);
      GetDialogItemText(itemHandle, myString);
      bug = SetIndString(myString, 1000 * classindex + spelllevel, oldspell - 9);

      ploticon3(129, buttonrect);
      goto out;
    }

    if ((itemHit == 24) || (itemHit == 25)) {
      /* Myriad, modified on request 08/11/99 */
      GetDialogItem(spellwindow, oldspell, &itemType, &itemHandle, &buttonrect);
      GetDialogItemText(itemHandle, myString);
      bug = SetIndString(myString, 1000 * classindex + spelllevel, oldspell - 9);

      ploticon3(135, buttonrect);
      sound(141);
      if (itemHit == 24)
        classindex--;
      else
        classindex++;

      if (classindex < 1)
        classindex = 3;
      if (classindex > 3)
        classindex = 1;

      ForeColor(yellowColor);
      TextSize(16);
      GetIndString(myString, 131, classindex + 5);
      MyrPascalDiStr(33, myString);

      TextSize(12);
      ploticon3(136, buttonrect);

      goto updatelist;
    }

    if ((itemHit < 9) && (itemHit > 1)) {

      GetDialogItem(spellwindow, 9, &itemType, &itemHandle, &itemRect);
      pict(130, itemRect);

      downbutton(FALSE);
      sound(137);

      GetDialogItem(spellwindow, oldspell, &itemType, &itemHandle, &buttonrect);
      GetDialogItemText(itemHandle, myString);
      bug = SetIndString(myString, 1000 * classindex + spelllevel, oldspell - 9);
      MyrPascalDiStr(oldspell, myString);

      spelllevel = itemHit - 2;

    updatelist:

      ForeColor(yellowColor);
      for (t = 0; t < 12; t++) {
        GetDialogItem(spellwindow, 10 + t, &itemType, &itemHandle, &buttonrect);
        buttonrect.left -= 2;
        buttonrect.top--;
        upbutton(FALSE);

        GetIndString(myString, 1000 * classindex + spelllevel, t + 1);
        MyrPascalDiStr(10 + t, myString);
      }
      GetIndString(myString, -(1000 * classindex + spelllevel), oldspell - 9);
      MyrPascalDiStr(39, myString);
      RGBForeColor(&cyancolor);
      GetDialogItem(spellwindow, oldspell, &itemType, &itemHandle, &buttonrect);
      GetDialogItemText(itemHandle, myString);
      buttonrect.left -= 2;
      buttonrect.top--;
      downbutton(FALSE);
      MyrPascalDiStr(oldspell, myString);
    }

    if (((itemHit > 9) && (itemHit < 22)) && (itemHit != oldspell)) {
      TextSize(12);
      ForeColor(yellowColor);

      GetDialogItem(spellwindow, oldspell, &itemType, &itemHandle, &buttonrect);
      buttonrect.left -= 2;
      buttonrect.top--;
      upbutton(FALSE);
      GetDialogItemText(itemHandle, myString);
      bug = SetIndString(myString, 1000 * classindex + spelllevel, oldspell - 9);
      MyrPascalDiStr(oldspell, myString);

      oldspell = itemHit;
      GetDialogItem(spellwindow, itemHit, &itemType, &itemHandle, &buttonrect);
      buttonrect.left -= 2;
      buttonrect.top--;
      downbutton(FALSE);

      GetIndString(myString, -(1000 * classindex + spelllevel), itemHit - 9);
      MyrPascalDiStr(39, myString);

      RGBForeColor(&cyancolor);
      GetDialogItemText(itemHandle, myString);
      GetIndString(myString, 1000 * classindex + spelllevel, itemHit - 9);
      MyrPascalDiStr(itemHit, myString);
      sound(144);
    }
  }
out:
  DisposeDialog(spellwindow);
}

/***************************** editracecaste ********************************/
void editracecaste(void) {
  short mode = 0;
  DialogRef gGeneration;

  if (usecustomnames == -1) {
    warn(126);
    return;
  }

  sound(3000);

  gGeneration = GetNewDialog(172, 0L, (WindowPtr)-1L);
  gStop = FALSE;
  gCurrent = gGeneration;
  SetPortDialogPort(gGeneration);
  BackPixPat(base);
  ForeColor(yellowColor);

  MoveWindow(GetDialogWindow(gGeneration), GlobalLeft, GlobalTop, FALSE);
  ShowWindow(GetDialogWindow(gGeneration));
  ErasePortRect();
  DrawDialog(gGeneration);

update:

  if (!mode)
    MyrCDiStr(63, (StringPtr) "Current Race Names.");
  else
    MyrCDiStr(63, (StringPtr) "Current Caste Names.");

  GetDialogItem(gGeneration, 63, &itemType, &itemHandle, &buttonrect);
  downbutton(FALSE);
  GetDialogItem(gGeneration, 64, &itemType, &itemHandle, &buttonrect);
  downbutton(FALSE);

  GetDialogItem(gGeneration, 65, &itemType, &itemHandle, &buttonrect);
  upbutton(FALSE);

  for (t = 3; t < 33; t++) {
    GetDialogItem(gGeneration, t, &itemType, &itemHandle, &itemRect);
    GetIndString(myString, 129 + (2 * mode), t - 2);
    SetDialogItemText(itemHandle, myString);
  }

  BeginUpdate(GetDialogWindow(gGeneration));
  EndUpdate(GetDialogWindow(gGeneration));

  for (;;) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);

    if (itemHit == 65) {
      for (t = 3; t < 33; t++) {
        GetDialogItem(gGeneration, t, &itemType, &itemHandle, &itemRect);
        GetDialogItemText(itemHandle, myString);
        SetIndString(myString, 129 + (2 * mode), t - 2);
      }

      sound(141);
      if (mode)
        mode = 0;
      else
        mode = 1;

      goto update;
    }

    if ((itemHit == 1) || (itemHit == 2)) {
      for (t = 3; t < 33; t++) {
        GetDialogItem(gGeneration, t, &itemType, &itemHandle, &itemRect);
        GetDialogItemText(itemHandle, myString);
        SetIndString(myString, 129 + (2 * mode), t - 2);
      }

      DisposeDialog(gGeneration);
      return;
    }
  }
}
