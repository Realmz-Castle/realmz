#if defined(REALMZ_CLASSIC)
#include <Printing.h>
#endif

#include "prototypes.h"
#include "realmzbuild.h"
#include "variables.h"
#define LINE_SPACING_TEXT 15

/****************************** doreg3 **********************************/
short doreg3(void) {
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
  return (FALSE);
}

/****************************** Copy_Part_Of_String **********************/
void Copy_Part_Of_String(Str255 dst, Str255 src, short start_char, short end_char) {
  strcpy(dst, &src[start_char]);
  dst[end_char - start_char + 1] = 0;
  /* Myriad
          short i,j;
          j = 1;

          for(i=start_char;i<= end_char;i++)
          {
           dst[j] = src[i];
           j++;
          }
          dst[0] = (end_char - start_char) + 1; myriad
  */
}

/****************************** printstat **********************/
void printstat(short dlog_item, short text_y) {
  Handle hand;
  short type;
  Rect box;
  Str255 dlog_string;
  short str_length;
  short start_char;
  short end_char;
  short i;
  short count;
  char the_chr;
  Str255 line_string;

  GetDialogItem(gCurrent, dlog_item, &type, &hand, &box);
  GetDialogItemText(hand, dlog_string);
  PtoCstr(dlog_string); // Myriad

  str_length = strlen(dlog_string);

  count = 0;
  start_char = 0;

  for (i = 0; i < str_length; i++) {
    ++count;
    the_chr = dlog_string[i];

    if (the_chr == 13) {
      end_char = i;
      Copy_Part_Of_String(line_string, dlog_string, start_char, end_char);
      start_char = end_char + 1;
      MoveTo(itemRect.left, text_y);
      MyrDrawCString((Ptr)line_string);
      text_y += LINE_SPACING_TEXT;
      count = 0;
    }
  }
  Copy_Part_Of_String(line_string, dlog_string, start_char, i - 1);
  MoveTo(itemRect.left, text_y);
  MyrDrawCString((Ptr)line_string);
}

/****************************** orderform **********************/
#if !defined(REALMZ_CLASSIC)
void orderform(void) {}
#else
void orderform(void) {
  THPrint Print_Record;
  TPPrPort printer_port;
  long total = 0;
  Handle item1, item2;
  float exchange = 1;
  Boolean disk = FALSE;
  Boolean foreign = FALSE;
  Boolean set = FALSE;
  DialogRef regscreen, countryscreen;
  long showserial; //*** fantasoft v7.1

  regscreen = GetNewDialog(300, 0L, (WindowPtr)-1L);
  gCurrent = regscreen;
  MoveWindow(regscreen, GlobalLeft + 42, GlobalTop + 1, FALSE);
  ShowWindow(regscreen);

  SetPort(regscreen);

  gStop = reply = FALSE;

  GetVersStr(1, Appl_Rsrc_Fork_Ref_Num); /*** load in version of Realmz right away ***/

  showserial = serial; //*** fantasoft v7.1
  MyrBitClrLong(&showserial, 6 + divine); //*** fantasoft v7.1
  MyrPascalDiStr(34, theString); /*** Version ***/

  MyrNumToString(showserial, myString);
  MyrCDiStr(36, myString);

  SelectDialogItemText(regscreen, 3, 0, 250);

  SetPort(regscreen);
  SelectDialogItemText(regscreen, 3, 0, 250);

  DrawDialog(regscreen);

  if (!doreg()) {
    GetDialogItem(regscreen, 12, &itemType, &itemHandle, &itemRect);
    SetControlValue((ControlHandle)itemHandle, TRUE);
    DialogNum(33, 20);
  }

  BeginUpdate(regscreen);
  EndUpdate(regscreen);

  while (!gStop) {

  keepgoing:

    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);

    if (itemHit == 49) {
      GetDialogItem(regscreen, 53, &itemType, &itemHandle, &itemRect);
      if (GetControlValue((ControlHandle)itemHandle)) /*** charge needs no exchange rate ***/
      {
        warn(130);
        warn(131);
        goto update;
      }

      countryscreen = GetNewDialog(171, 0L, (WindowPtr)-1L);
      FlushEvents(everyEvent, 0);
      ModalDialog(0L, &itemHit);
      DisposeDialog(countryscreen);

      GetIndString(myString, 4, itemHit);
      MyrPascalDiStr(45, myString);
      switch (itemHit) {
        case 1:
          exchange = 1;
          break;

        case 2:
          exchange = 1.6;
          break;

        case 3:
          exchange = 38;
          break;

        case 4:
          exchange = .75;
          break;

        case 5:
          exchange = 1.50;
          break;

        case 6:
          exchange = 7.2;
          break;

        case 7:
          exchange = 6.3;
          break;

        case 8:
          exchange = 290;
          break;

        case 9:
          exchange = 1.90;
          break;

        case 10:
          exchange = 1850;
          break;

        case 11:
          exchange = 0;
          break;

        case 12:
          exchange = 2.10;
          break;

        case 13:
          exchange = 1.80;
          break;

        case 14:
          exchange = 7.70;
          break;

        case 15:
          exchange = 1.90;
          break;

        case 16:
          exchange = 8.10;
          break;

        case 17:
          exchange = 1.55;
          break;

        case 18:
          exchange = 35.00;
          break;

        case 19:
          exchange = 55;
          break;

        case 20:
          exchange = 1.0; /**** USA ****/
          break;
      }
      itemHit = 0;
      goto update;
    }

    if (itemHit == 39)
      goto out;

    if (itemHit == 2)
      reply = gStop = TRUE;

    GetDialogItem(regscreen, itemHit, &itemType, &itemHandle, &itemRect);

    if (itemType == 6) // Radio Buttons
    {
      GetDialogItem(regscreen, 53, &itemType, &itemHandle, &itemRect);
      SetControlValue((ControlHandle)itemHandle, 0);
      GetDialogItem(regscreen, 21, &itemType, &itemHandle, &itemRect);
      SetControlValue((ControlHandle)itemHandle, 0);
      GetDialogItem(regscreen, itemHit, &itemType, &itemHandle, &itemRect);
      SetControlValue((ControlHandle)itemHandle, 1);

      if (itemHit == 53) {
        warn(94);
        exchange = 1;
        goto update;
      }
    }

    if (itemType == 5) //  check box
    {

      GetDialogItem(regscreen, itemHit, &itemType, &itemHandle, &itemRect);
      set = GetControlValue((ControlHandle)itemHandle);
      if (set)
        set = 0;
      else
        set = 1;

      SetControlValue((ControlHandle)itemHandle, set);

    update:
      total = disk = temp = 0;

      GetDialogItem(regscreen, 53, &itemType, &itemHandle, &itemRect); /*** Credit elims X-Rate ****/
      if (GetControlValue((ControlHandle)itemHandle))
        exchange = 1.0; /*** Credit elims X-Rate ****/

      GetDialogItem(regscreen, 12, &itemType, &itemHandle, &itemRect); /**** Realmz ****/
      if (GetControlValue((ControlHandle)itemHandle)) {
        total += 20;
      } else if (!doreg()) {
        SetControlValue((ControlHandle)itemHandle, TRUE);
        total += 20;
      }

      GetDialogItem(regscreen, 13, &itemType, &itemHandle, &itemRect); /**** Prelude ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 0;

      GetDialogItem(regscreen, 14, &itemType, &itemHandle, &itemRect); /**** Assault ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 56, &itemType, &itemHandle, &itemRect); /**** Divinity ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 30;

      GetDialogItem(regscreen, 19, &itemType, &itemHandle, &itemRect); /**** Castle ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 52, &itemType, &itemHandle, &itemRect); /**** Destroy ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 20, &itemType, &itemHandle, &itemRect); /**** Grilochs ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 57, &itemType, &itemHandle, &itemRect); /**** Sword Lands ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 58, &itemType, &itemHandle, &itemRect); /**** Mithril Vault ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 59, &itemType, &itemHandle, &itemRect); /**** Twin Sands of Time ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      // Fantasoft v7.1 BEGIN
      GetDialogItem(regscreen, 60, &itemType, &itemHandle, &itemRect); /**** War in Sword Lands ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 61, &itemType, &itemHandle, &itemRect); /**** Half Truth ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 62, &itemType, &itemHandle, &itemRect); /**** Wrath of Mind Lords ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 63, &itemType, &itemHandle, &itemRect); /**** Future Use ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 64, &itemType, &itemHandle, &itemRect); /**** Future Use ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 65, &itemType, &itemHandle, &itemRect); /**** Future Use ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 66, &itemType, &itemHandle, &itemRect); /**** Future Use ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 67, &itemType, &itemHandle, &itemRect); /**** Future Use ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 68, &itemType, &itemHandle, &itemRect); /**** Future Use ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 69, &itemType, &itemHandle, &itemRect); /**** Future Use ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;

      GetDialogItem(regscreen, 70, &itemType, &itemHandle, &itemRect); /**** Future Use ****/
      if (GetControlValue((ControlHandle)itemHandle))
        total += 13;
      // Fantasoft v7.1 END

      GetDialogItem(regscreen, 15, &itemType, &itemHandle, &itemRect);
      if (GetControlValue((ControlHandle)itemHandle)) {
        total += 10;
        disk = TRUE;
      }

      GetDialogItem(regscreen, 16, &itemType, &itemHandle, &itemRect);
      if (GetControlValue((ControlHandle)itemHandle)) {
        total += 6;
        disk = TRUE;
      }

      GetDialogItem(regscreen, 17, &itemType, &itemHandle, &itemRect);
      if (GetControlValue((ControlHandle)itemHandle)) {
        total += 13;
        disk = TRUE;
      }

      GetDialogItem(regscreen, 45, &itemType, &itemHandle, &itemRect);
      if ((GetDialogNum(45) > 1) && (disk)) {
        total += 3;
        MyrCDiStr(26, (StringPtr) "   Overseas disk postage     $");
        DialogNum(25, 2);
      } else {
        MyrCDiStr(26, (StringPtr) "");
        MyrCDiStr(25, (StringPtr) "");
      }

      GetDialogItem(regscreen, 18, &itemType, &itemHandle, &itemRect); // tax
      if (GetControlValue((ControlHandle)itemHandle)) {
        temp = total * .055;
        DialogNum(32, temp);
        total *= 1.055;
        MyrCDiStr(27, (StringPtr) "Sales Tax");
        MyrCDiStr(29, (StringPtr) "$");
      } else {
        MyrCDiStr(27, (StringPtr) "");
        MyrCDiStr(29, (StringPtr) "");
        MyrCDiStr(32, (StringPtr) "");
      }

      if (exchange != 1.0) {
        MyrCDiStr(43, (StringPtr) "Exchange Rate");
        MyrCDiStr(44, (StringPtr) "x");

        MyrCDiStr(46, (StringPtr) "Sub Total");
        MyrCDiStr(47, (StringPtr) "$");
        MyrCDiStr(30, (StringPtr) "");
        DialogNumLong(48, total);
      } else {
        MyrCDiStr(43, (StringPtr) "");
        MyrCDiStr(44, (StringPtr) "");
        MyrCDiStr(45, (StringPtr) "");
        MyrCDiStr(30, (StringPtr) "$");

        MyrCDiStr(46, (StringPtr) "");
        MyrCDiStr(47, (StringPtr) "");
        MyrCDiStr(48, (StringPtr) "");
      }

      total *= exchange;

      DialogNumLong(33, total);
    }
  }

  GetDialogItem(regscreen, 53, &itemType, &item1, &itemRect);
  GetDialogItem(regscreen, 21, &itemType, &item2, &itemRect);

  if ((!GetControlValue((ControlHandle)item1)) && (!GetControlValue((ControlHandle)item2))) {
    warn(93);
    gStop = FALSE;
    goto keepgoing;
  }

  if (reply) {

    Print_Record = (THPrint)NewHandleClear(sizeof(TPrint));

    PrOpen();

    PrintDefault(Print_Record);

    PrStlDialog(Print_Record);

    if (PrJobDialog(Print_Record)) {

      printer_port = PrOpenDoc(Print_Record, NIL, NIL);

      PrOpenPage(printer_port, NIL);

      for (t = 3; t < 100; t++) {
        itemHandle = NIL;
        GetDialogItem(regscreen, t, &itemType, &itemHandle, &itemRect);
        if (itemHandle != NIL) {
          if ((itemType == 5) || (itemType == 6)) // controls
          {
            if (GetControlValue((ControlHandle)itemHandle)) /*** print only checked items ***/
            {
              GetControlTitle((ControlHandle)itemHandle, myString);
              MoveTo(itemRect.left, itemRect.top + 40);
              PtoCstr(myString); // Myriad
              MyrDrawCString((Ptr)myString); // Myriad
            }
          }

          if ((itemType == statText) || (itemType == editText)) {
            printstat(t, (itemRect.top + 40));
          }
        }
      }

      itemRect.top = 20;
      itemRect.left = 0;
      itemRect.right = itemRect.left + 277;
      itemRect.bottom = itemRect.top + 110;

      pict(164, itemRect);

      PrClosePage(printer_port);
      PrCloseDoc(printer_port);
    }

    PrClose();

  out:

    DisposeDialog(regscreen);
  } else
    DisposeDialog(regscreen);
}
#endif /* !CARBON */
