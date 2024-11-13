#include "prototypes.h"
#include "realmzbuild.h"
#include "variables.h"

/*************************** regdiv **************************/
short regdiv(void) {
  long noteid = 0;
  long firsttime, secondtime, regcode = 0;
  short primer1, primer2, primer3;
  short toolong = 180;
  DialogRef screen;

  sound(10107);
  screen = GetNewDialog(11177, 0L, (WindowPtr)-1L);
  SetPortDialogPort(screen);
  MoveWindow(GetDialogWindow(screen), GlobalLeft, GlobalTop, FALSE);
  DrawDialog(screen);

  for (;;) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);

    if (itemHit == 1) {
      GetDialogItem(screen, 5, &itemType, &itemHandle, &itemRect);
      GetDialogItemText(itemHandle, myString);
      StringToNum(myString, &showserial);
      GetDialogItem(screen, 6, &itemType, &itemHandle, &itemRect);
      GetDialogItemText(itemHandle, myString);
      StringToNum(myString, &noteid);

      if (!StringWidth(myString))
        exit(0);

      GetDialogItem(screen, 4, &itemType, &itemHandle, &itemRect);
      GetDialogItemText(itemHandle, (StringPtr)&gotword); /*** puts name in gotword ***/
      for (tt = 1; tt <= gotword[0]; tt++)
        gotword[tt] = tolower(gotword[tt]); /**** convert to all lower case ******/
      firsttime = TickCount();

      regcode = serial = showserial;
      NumToString(regcode, myString); // Fantasoft 7.1  Use NumToString vs. MyrNumToString  This is only in the Mac version so it is o.k.
                                      // And it need to be in pascal for it to work right vs. the decoder.

      primer1 = 32 - (gotword[0] % 3);
      primer2 = 31 - (gotword[0] % 6);
      primer3 = 30 - (gotword[0] % 9);

      /**** do bit flip for reg name ******/

      for (tt = 1; tt <= gotword[0]; tt++) /**** reg name ******/
      {
        if (MyrBitTstLong(&regcode, gotword[tt] % primer1)) {
          MyrBitClrLong(&regcode, gotword[tt] % primer2 + 1);
        } else {
          MyrBitSetLong(&regcode, gotword[tt] % primer3 + 2);
        }
      }

      /**** do bit flip for s/n ******/

      for (tt = 1; tt < myString[0]; tt++) /***** do bit flip for s/n ****/
      {
        if (MyrBitTstLong(&regcode, myString[tt] % primer3 + 3)) {
          MyrBitClrLong(&regcode, myString[tt] % primer2 + 2);
        } else {
          MyrBitSetLong(&regcode, myString[tt] % primer1 + 1);
        }
      }

      secondtime = TickCount();

      if (regcode == noteid) {
        FlushEvents(everyEvent, 0);
        GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
        DoCorrectBugMADRepeat();
#endif
        MyrCheckMemory(2);

        MyrBitClrLong(&serial, 6 + divine);

        updatescenarioavail();

        GetDialogItem(screen, 4, &itemType, &itemHandle, &itemRect);
        GetDialogItemText(itemHandle, (StringPtr)&Name_String); /**** loads name for saving to pref *****/

        if (!StringWidth(Name_String))
          exit(0);

        savepref();

        SetPortDialogPort(background);
        BackColor(blackColor);
        gCurrent = background;
        MyrCDiStr(2, (StringPtr) "");
        MyrCDiStr(3, (StringPtr) "");
        DisposeDialog(screen);
        screen = NIL;
        DrawDialog(background);
        flashmessage((StringPtr) "Copy down your Serial Number and Registration Code.  You will need them to re-register future versions.", 50, 150, 0, 1101);
        flashmessage((StringPtr) "We are really serious. You will NEED those codes. Copy them down and put them under your pillow.", 50, 150, 0, 10141);
        return (temp);
      } else
        exit(0);
    }
  }
}

#if !divine
/*************************** aboutrealmz **************************/
void aboutrealmz(void) {
  GrafPtr oldport;
  char myCString[255];
  short toolong = 180;
  long firsttime, secondtime, noteid = 0;
  short verifycode = 0;
  short randomcon[9] = {738, 752, 724, 741, 755, 790, 718, 713, 731};
  char str[255];

changed:

  sound(10107);
  GetPort(&oldport);
  if (look)
    in();

  savepref();
  getpref();

  showserial = serial;
  MyrBitClrLong(&showserial, 6 + divine);
  MyrNumToString(showserial, myString);
  CtoPstr((Ptr)myString);
  GetVersStr(1, Appl_Rsrc_Fork_Ref_Num); /*** load in version of Realmz ***/

  strcpy(str, (StringPtr) "Unregistered"); // Myriad
  CtoPstr(str); // Myriad

#ifdef PC
  if (MyrBitTstLong(&serial, 6 + divine))
    ParamText(myString, (Ptr)str, theString, (StringPtr) "\pP");
  else
    ParamText(myString, Name_String, theString, (StringPtr) "\pP");
#else
  if (MyrBitTstLong(&serial, 6 + divine))
    ParamText(myString, (StringPtr)str, theString, (StringPtr) "\pX");
  else
    ParamText(myString, Name_String, theString, (StringPtr) "\pX");
#endif

  about = GetNewDialog(166, 0L, (WindowPtr)-1L);

  SetPortDialogPort(about);
  BackPixPat(base);
  TextFont(defaultfont);
  gCurrent = about;
  ForeColor(yellowColor);
  MoveWindow(GetDialogWindow(about), GlobalLeft, GlobalTop, FALSE);
  ShowWindow(GetDialogWindow(about));
  ErasePortRect();
  DrawDialog(about);

  GetDialogItem(about, 3, &itemType, &itemHandle, &itemRect);
  SetDialogItemText(itemHandle, (StringPtr) "");

  temp = Rand(9) - 1;

  iconhand = GetCIcon(randomcon[temp]);
  itemRect.bottom = itemRect.top + (**iconhand).iconBMap.bounds.bottom;
  itemRect.right = itemRect.left + (**iconhand).iconBMap.bounds.bottom;
  PlotCIcon(&itemRect, iconhand);
  if (iconhand)
    DisposeCIcon(iconhand);

  sound(30000);
  delay(20);
  sound(10141);

  BeginUpdate(GetDialogWindow(about));
  EndUpdate(GetDialogWindow(about));
  FlushEvents(everyEvent, 0);
  goto goon;

  for (;;) {
  donothing:

    SystemTask();
    t = GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
    DoCorrectBugMADRepeat();
#endif
    MyrCheckMemory(2);

    if (IsDialogEvent(&gTheEvent)) {
      if (BitAnd(gTheEvent.message, charCodeMask) == 13) {
        FlushEvents(everyEvent, 0);
        gTheEvent.what = itemHit = 0;
        gTheEvent.message = 0L;
        gTheEvent.when = 0L;
      }

      t = DialogSelect(&gTheEvent, &dummy, &itemHit);

      if (itemHit == 13) /**** print reg form *****/
      {
        orderform();
        itemHit = 0;
        DisposeDialog(about);
        about = NIL;
        goto changed;
      }
      if (itemHit == 11) {
        firsttime = TickCount();
        GetDialogItem(about, 16, &itemType, &itemHandle, &itemRect);
        GetDialogItemText(itemHandle, myString);
        StringToNum(myString, &noteid);

        if (!StringWidth(myString))
          goto donothing;

        if (noteid == 911) {
        showmoviestuff:
          MyrBitSetLong(&serial, 8);
          MyrBitSetLong(&serial, 6 + divine);
          savepref();
          flashmessage((StringPtr) "If you have registered your copy with Fantasoft, contact us to get your new codes for this version.", 50, 150, 0, 1101);
          flashmessage((StringPtr) "Read the document 'Re-Registration Request' that is in your Realmz folder for details.", 50, 150, 0, 1100);
          goto goon;
        }

        if ((noteid > 0) && (noteid < 700)) /****** enter old serial number ****/
        {

          flashmessage((StringPtr) "Enter the SERIAL NUMBER, (Not registration code) of your old copy.", 30, 40, -1, 5000);
          getword();

          flashmessage((StringPtr) "", 30, 40, -1, 0);

          StringToNum((StringPtr)gotword, &templong);

          if (noteid != templong % 666) {
            flashmessage((StringPtr) "Wrong...", 50, 150, 0, 5000);
            goto showmoviestuff;
          }

          setfree(templong);

          if (!MyrBitTstLong(&templong, 8)) {
            warn(77);
            DisposeDialog(about);
            about = NIL;
            goto changed;
          }

          FlushEvents(everyEvent, 0);
          GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
          DoCorrectBugMADRepeat();
#endif
          MyrCheckMemory(2);

          serial = templong;
          MyrBitSetLong(&serial, 8);
          MyrBitSetLong(&serial, 6 + divine);

          showserial = serial;

          secondtime = TickCount();

          strcpy(myCString, myString);

          if ((fp = MyrFopen(myCString, "r+b")) != NIL) {
            CvtLongToPc(&showserial);
            fwrite(&showserial, sizeof showserial, 1, fp);
            CvtLongToPc(&showserial);

            fclose(fp);
          }

          savepref();
          getpref();
          flashmessage((StringPtr) "Resetting this copy to the old serial number.  You may now register with your old registration codes.", 30, 40, 0, 5000);
          flashmessage((StringPtr) "Realmz will now quit.  Run again to register.", 30, 40, 0, 5000);
          exit(0);
        }

        Name(1);

        templong = showserial;

#ifdef PC
        templong /= 24;
        templong += 48;
        templong *= 14;
        templong -= 128; //*** I don't want the codes for Realmz Mac - PC to be the same
#else

        templong /= 24;
        templong += 24;
        templong *= 15;
        templong -= 256;
#endif

        if (noteid == templong) {
          flashmessage((StringPtr) "Wrong...", 50, 150, 0, 5000);
          goto showmoviestuff;
        }

        templong += 100;

        if (noteid == templong) {
          FlushEvents(everyEvent, 0);
          GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
          DoCorrectBugMADRepeat();
#endif
          MyrCheckMemory(2);

          MyrBitClrLong(&serial, 6 + divine);
          updatescenarioavail();

          savepref();

          flashmessage((StringPtr) "Copy down your Serial Number and Registration Codes.  You will need them to re-register future versions.", 50, 150, 0, 1101);
          flashmessage((StringPtr) "We are really serious. You will NEED those codes. Copy them down and put them under your pillow.", 50, 150, 0, 1100);

          goto goon;
        } else {
          GetIndString(myString, 3, 1);
          GetIndString(myString, 6002, 2);
          flashmessage(myString, 50, 150, 400, 6000);
          DisposeDialog(about);
          about = NIL;
          goto changed;
        }
      }
      if (itemHit == 12)
        goto goon;
    }
  }

goon:

  SetPort(oldport);
  if (!look) {
    SetPortDialogPort(background);
    BackColor(blackColor);
    gCurrent = background;
    MyrPascalDiStr(2, (StringPtr) "");
    MyrPascalDiStr(3, (StringPtr) "");
    DisposeDialog(about);
    about = NIL;
    DrawDialog(background);
  } else {
    DisposeDialog(about);
    about = NIL;
    if (!incombat)
      updatemain(FALSE, -1);
  }
  FlushEvents(everyEvent, 0);
}
#else
#endif
