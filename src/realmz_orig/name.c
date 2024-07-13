#include "prototypes.h"
#include "variables.h"

/***************************** Name ********************************/
short Name(short mode) {
  FILE* fp = NULL;
  short reply = TRUE;
  char name[256];
  DialogRef namewindow;

  namewindow = GetNewDialog(132, 0L, (WindowPtr)-1L);
  gCurrent = namewindow;
tryagain:
  SetPortDialogPort(namewindow);
  BackPixPat(base);
  TextFont(defaultfont);
  ForeColor(yellowColor);
  gStop = 0;
  MoveWindow(GetDialogWindow(namewindow), GlobalLeft + 132, GlobalTop + 125, FALSE);
  ShowWindow(GetDialogWindow(namewindow));
  ErasePortRect();

  sound(30005);

  GetDialogItem(namewindow, 2, &itemType, &itemHandle, &itemRect);
  SetDialogItemText(itemHandle, (StringPtr) "");

over:
  DrawDialog(namewindow);

  while (!gStop) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    if ((itemHit == 1) || (itemHit == 4)) {
      GetDialogItem(namewindow, 4, &itemType, &itemHandle, &buttonrect);
      GetDialogItem(namewindow, 2, &itemType, &itemHandle, &itemRect);
      if (!mode)
        GetDialogItemText(itemHandle, (StringPtr)&name);
      else {
        GetDialogItemText(itemHandle, myString);
        GetDialogItemText(itemHandle, (StringPtr)&Name_String);
      }
      ploticon3(133, buttonrect);

      if (mode) {
        PtoCstr(myString);
        if (!strlen(myString)) {
          warn(70);
          SetPortDialogPort(namewindow);
          ForeColor(yellowColor);
          MyrCDiStr(2, (StringPtr) "");
          goto over;
        } else {
          GetDialogItemText(itemHandle, (StringPtr)&gotword);
          goto out;
        }
      }

      if (StringWidth((StringPtr)name)) {
        PtoCstr((StringPtr)name);
        strcpy(characterl.name, name);

        TextFont(font);
        TextSize(15);
        CtoPstr(characterl.name);
        temp = StringWidth((StringPtr)characterl.name);

        if (temp > 133) {
          warn(44);
          goto over;
        }
        PtoCstr((StringPtr)characterl.name);

        if (strlen(name) > 28) {
          warn(44);
          goto over;
        }
        strcpy(filename, ":Character Files:");
        strncat(filename, name, 30);

        if ((fp = MyrFopen(filename, "rb")) != NULL) {
          fclose(fp);

          flashmessage((StringPtr) "If you play a saved game which has a character by this name, this one may change to match that one.", 100, 200, 0, 6000);
          if (question(4)) {
            sound(6001);
            DrawDialog(namewindow);
            goto out;
          } else
            goto tryagain;
        } else {
          if ((fp = MyrFopen(":Character Files:Data CD", "r+b")) == NULL) {
            if ((fp = MyrFopen(":Character Files:Data CD", "w+b")) == NULL)
              scratch2(10);
          }

          for (t = 0; t < 5000; t++) {
            strcpy(name, (StringPtr) "");
            fread(&name, 30, 1, fp);
            fread(&temp, 2, 1, fp); /***** level placeholder ****/
            if (!strcmp(name, (StringPtr) "")) {
              gStop = 1;
              sound(6001);
              fseek(fp, t * 32, SEEK_SET);
              fwrite(characterl.name, 30, 1, fp);
              CvtCharacterToPc(&characterl);
              fwrite(&characterl.level, 2, 1, fp); /***** level placeholder ****/
              CvtCharacterToPc(&characterl);
              fclose(fp);
              setfileinfo("CDat", (Ptr) ":Character Files:Data CD");
              goto out;
            }
          }
        }
      } else {
        sound(6000);
        if (!question(11))
          goto over;
        reply = FALSE;
        goto out;
      }
    }
  }
out:
  DisposeDialog(namewindow);
  return (reply);
}
