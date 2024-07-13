#include "prototypes.h"
#include "variables.h"

/****************** items ************************************/
short items(void) {
  Boolean updatelist = FALSE;
  Boolean doit = FALSE;
  Boolean usecharge;
  long mouseuptime = 0;
  Point dclick;
  short temp, ts, ti, a, count, charge;
  char itemselectold, loop;
  WindowPtr whichWindow;
  ControlHandle tempdummy, castident, describe, join, split, identify, done, pick, trade, checkcontrol, uparrow, down, drop, charinfo, use;
  Rect checkrect, shadebox, box2, box4, charbox;
  char first = -1;
  char backvalue = 0;
  Boolean equip, play, ident;
  Rect r;

  SetMenuBar(copywright);
  DrawMenuBar();

  box.left = 10;
  box.right = 284 + leftshift;

  SetCCursor(sword);
  for (t = 0; t <= numchannel; t++)
    quiet(t);

  compactheap();

  initems = TRUE;
  cleanage = 0;

  if (!incombat)
    music(6); /**** items music ***/

  for (loop = 1; loop < 8; loop++)
    track[loop] = 0; /**** for encounter use ******/

  itemswindow = GetNewWindow(129, 0L, (WindowPtr)-1L);
#ifdef PC
  AcamWindowRgbBackColor(itemswindow, 0xFFFF, 0xFFFF, 0xFFFF); // Myriad
#endif
  SetCCursor(sword);
  SetPort(GetWindowPort(itemswindow));
  ErasePortRect();
  TextFont(font);
  TextFace(NIL);
  TextSize(15);
  TextMode(0);

  itemselectnew = itemselectold = 0;
  tempdummy = GetNewControl(196, itemswindow);
  GetControlBounds(tempdummy, &r);
  MoveControl(tempdummy, r.left + leftshift, r.top);

  use = GetNewControl(152, itemswindow);
  GetControlBounds(use, &r);
  MoveControl(use, r.left + leftshift, r.top);

  identify = GetNewControl(143, itemswindow);
  GetControlBounds(identify, &r);
  MoveControl(identify, r.left + leftshift, r.top);

  drop = GetNewControl(148, itemswindow);
  GetControlBounds(drop, &r);
  MoveControl(drop, r.left + leftshift, r.top);

  describe = GetNewControl(140, itemswindow);
  GetControlBounds(describe, &r);
  MoveControl(describe, r.left + leftshift, r.top);

  uparrow = GetNewControl(150, itemswindow);
  GetControlBounds(uparrow, &r);
  MoveControl(uparrow, r.left + leftshift, r.top);

  down = GetNewControl(151, itemswindow);
  GetControlBounds(down, &r);
  MoveControl(down, r.left + leftshift, r.top);

  checkcontrol = GetNewControl(145, itemswindow);
  GetControlBounds(checkcontrol, &r);
  MoveControl(checkcontrol, r.left + leftshift, r.top);

  charinfo = GetNewControl(146, itemswindow);
  GetControlBounds(charinfo, &r);
  MoveControl(charinfo, r.left + leftshift, r.top);

  pick = GetNewControl(162, itemswindow); //***** this needs to be resized, not moved.
  GetControlBounds(pick, &r);
  SizeControl(pick, (r.right - r.left) + leftshift, r.bottom - r.top);

  done = GetNewControl(161, itemswindow);
  GetControlBounds(done, &r);
  MoveControl(done, r.left + leftshift, r.top);

  trade = GetNewControl(163, itemswindow);
  GetControlBounds(trade, &r);
  MoveControl(trade, r.left + leftshift, r.top);

  castident = GetNewControl(176, itemswindow);
  GetControlBounds(castident, &r);
  MoveControl(castident, r.left + leftshift, r.top);

  box2.right = 640 + leftshift;
  box2.bottom = 460;
  box2.left = box2.right - 356;
  box2.top = box2.bottom - 119;

  box3.left = 0;
  box3.top = 11;
  box3.bottom = 51;
  box3.right = 8;

  charbox.left = 417 + leftshift;
  charbox.top = 395;
  charbox.right = 449 + leftshift;
  charbox.bottom = 427;

  shadebox = charbox;
  InsetRect(&shadebox, -9, -9);

  incr = itemused = 0;

  MoveWindow(itemswindow, GlobalLeft, GlobalTop, FALSE);
  ShowWindow(itemswindow);

  charstat = GetNewDialog(143, 0L, (WindowPtr)-1L);

  if (!reducesound)
    sound(20001);
  MoveWindow(GetDialogWindow(charstat), GlobalLeft + 285 + leftshift, GlobalTop, FALSE);
  ShowWindow(GetDialogWindow(charstat));
  SetPortDialogPort(charstat);
  BackPixPat(base);
  ErasePortRect();
  DrawDialog(charstat);

  SetPort(GetWindowPort(itemswindow));
  DrawPicture(marker, &box3);

  compactitems(charselectnew);
  theControl = tempdummy;
  updateitems(0, 10);

  join = GetNewControl(170, itemswindow);
  GetControlBounds(join, &r);
  MoveControl(join, r.left + leftshift, r.top);

  split = GetNewControl(171, itemswindow);
  GetControlBounds(split, &r);
  MoveControl(split, r.left + leftshift, r.top);

backup:

  SetPort(GetWindowPort(itemswindow));
  TextMode(0);
  ForeColor(yellowColor);
  RGBBackColor(&greycolor);
  pict(6, box2);

  if (screensize)
    quickinfo(charselectnew, itemselectnew + incr, c[charselectnew].items[itemselectnew + incr].id, 0);

  plotportrait(c[charselectnew].pictid, charbox, c[charselectnew].caste, charselectnew);
  ForeColor(yellowColor);
  if (c[charselectnew].stamina < 1) {
    ploticon3(2019, shadebox);
    if (c[charselectnew].stamina < -9)
      ploticon3(2015, charbox);
  }

  if (incombat) {
    GetControlBounds(trade, &itemRect);
    // itemRect = *&(**(trade)).contrlRect;
    InsetRect(&itemRect, 9, 9);
    ploticon3(0, itemRect);
  }

  buttonrect.top = 386;
  buttonrect.left = 353 + leftshift;
  buttonrect.right = buttonrect.left + 22;
  buttonrect.bottom = buttonrect.top + 22;

  for (t = 0; t <= charnum; t++) {
    c[t].condition[1] = 0; /******** remove HELPLESS condition *****/
    if (t == 3)
      OffsetRect(&buttonrect, 24, -72);

    iconhand = NIL;
    iconhand = GetCIcon(c[t].pictid);

    if (iconhand) {
      PlotCIcon(&buttonrect, iconhand);
      DisposeCIcon(iconhand);
    }
    if (t == charselectnew)
      ploticon3(145, buttonrect);

    OffsetRect(&buttonrect, 0, 24);
  }

  TextMode(1);
  MoveTo(475 + leftshift, 450);
  MyrDrawCString((Ptr)c[charselectnew].name);
  BackColor(whiteColor);
  gCurrent = charstat;
  SetPortDialogPort(charstat);

  TextFont(font);
  gCurrent = charstat;
  TextSize(17);
  ForeColor(yellowColor);
  GetIndString(myString, 131, c[charselectnew].caste);
  MyrPascalDiStr(23, myString);
  GetIndString(myString, 129, c[charselectnew].race);
  MyrPascalDiStr(24, myString);

  DialogNumLong(21, c[charselectnew].exp);

  if (c[charselectnew].gender == 1)
    MyrCDiStr(27, (StringPtr) "Male");
  else
    MyrCDiStr(27, (StringPtr) "Female");

  calcw(charselectnew);

  updatecharinfo();

  for (;;) {
  tryagain:

    itemused = doit = FALSE;
    SetPort(GetWindowPort(itemswindow));

    if (gTheEvent.modifiers & alphaLock)
      warn(21);

    SystemTask();
    a = GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
    DoCorrectBugMADRepeat();
#endif
    MyrCheckMemory(2);

    if (IsDialogEvent(&gTheEvent)) {
      if (DialogSelect(&gTheEvent, &dummy, &itemHit)) {

        GetDialogItem(charstat, itemHit, &itemType, &itemHandle, &buttonrect);
        SetPortDialogPort(charstat);
        switch (itemHit) {
          case 34:
            downbutton(FALSE);
            SetPort(GetWindowPort(itemswindow));
            temp = showcondition(charselectnew, charselectnew, 1, 0, charselectnew);
            SetPortDialogPort(charstat);
            upbutton(FALSE);
            theControl = tempdummy; /****** trustme, leave in ******/
            updateitems(temp, temp);
            if (temp > -1)
              updatecharinfo();
            break;

          case 22:
            downbutton(FALSE);
            SetPort(GetWindowPort(itemswindow));
            showcondition(charselectnew, charselectnew, 0, 0, charselectnew);
            SetPortDialogPort(charstat);
            upbutton(FALSE);
            break;
        }
        SetPort(GetWindowPort(itemswindow));
      }
    }

    switch (gTheEvent.what) {
      case osEvt:
        if ((gTheEvent.message >> 24) == suspendResumeMessage) {
          if (hide) {
            compactheap();
            hide = FALSE;
            theControl = tempdummy;
            updateitems(0, 10);
            FlushEvents(everyEvent, 0);
            goto backup;
          } else
            hide = TRUE;
        }
        break;

      case (updateEvt):
        BeginUpdate(itemswindow);
        EndUpdate(itemswindow);
        BeginUpdate(GetDialogWindow(charstat));
        EndUpdate(GetDialogWindow(charstat));
        if (mat) {
          BeginUpdate(mat);
          EndUpdate(mat);
        }
        break;

      case mouseUp:
        mouseuptime = gTheEvent.when + GetDblTime();
        dclick = gTheEvent.where;
        GlobalToLocal(&(dclick));
        break;

      case (diskEvt):
        if (HiWord(gTheEvent.message) != noErr) /* if MountVol had err	*/
        { /* then initialize disk	*/
          SysBeep(NIL); /* beep			*/
          SetPt(&point, 30, 40); /* position		*/
          DIBadMount(point, gTheEvent.message); /* initialize	*/
        }
        goto tryagain;
        break;

      case keyDown:

        if (gTheEvent.modifiers & cmdKey) {
          commandkey = BitAnd(gTheEvent.message, charCodeMask);
          menuChoice = MenuKey(commandkey);
        }

      dokey:
        switch ((gTheEvent.message & charCodeMask)) {
          case 'j':
          case 'J':
            theControl = join;
            goto checkcontrols;
            break;

          case 0x1e: /*** up arrow ***/
          case 0x38: /*** up arrow ***/
          up:
            itemselectold = itemselectnew;
            itemselectnew--;
            goto forcepick;
            break;

          case 0x1f: /*** down arrow ***/
          case 0x32:
          down:
            itemselectnew++;
            itemselectold = itemselectnew;
            goto forcepick;
            break;

          case 13: /**** return ***/
          case 3: /**** return ***/
          case ' ':
          pick:
            theControl = pick;
            doit = TRUE;
            itemselectold = itemselectnew;
            goto forcepick;
            break;

          case 's': /*** Show Item ***/
          case 'S': /*** Show Item ***/
            theControl = describe;
            goto checkcontrols;
            break;

          case 'd': /*** Drop Item ***/
          case 'D': /*** Drop Item ***/
            theControl = drop;
            goto checkcontrols;
            break;

          case 'c': /*** Cast Ident ***/
          case 'C': /*** Cast Ident ***/
            theControl = castident;
            goto checkcontrols;
            break;

          case 'p': /*** Pay Ident ***/
          case 'P': /*** Pay Ident ***/
            theControl = identify;
            goto checkcontrols;
            break;

          case 'u': /*** Use Item ***/
          case 'U': /*** Use Item ***/
            theControl = use;
            goto checkcontrols;
            break;

          case 't': /*** trade ***/
          case 'T': /*** trade ***/
            theControl = trade;
            goto checkcontrols;
            break;

          default:
            goto tryagain;
            break;
        }
        break;

      case (autoKey):
        goto dokey;
        break;

      case (mouseDown):

        thePart = FindWindow(gTheEvent.where, &whichWindow);

        switch (thePart) {
          case inMenuBar:
            if ((gTheEvent.modifiers & cmdKey) && (!gotegg)) {
              gotegg = TRUE;

              MyrAppendMenu(copy, (Ptr) "The Packers Rock!");
            }
            compactheap();
            menuChoice = MenuSelect(gTheEvent.where);
            HiliteMenu(0);
            goto tryagain;
            break;

          case inContent:

            point = gTheEvent.where;
            GlobalToLocal(&(point));

            if (gTheEvent.modifiers & optionKey) {
              itemselectnew = (point.v - 11) / 40;
              itemselectold = itemselectnew;
              theControl = describe;
              EraseRect(&box3);
              box3.top = 11 + itemselectnew * 40;
              box3.bottom = box3.top + 40;
              DrawPicture(marker, &box3);
              goto getinfo;
            }

            if ((gTheEvent.modifiers & cmdKey) && (gTheEvent.where.h < (240 + leftshift))) {
              itemselectnew = (point.v - 11) / 40;
              itemselectold = itemselectnew;
              goto dopick;
            }

            if ((gTheEvent.when <= mouseuptime) && (gTheEvent.where.h < (240 + leftshift))) {
              templong = DeltaPoint(dclick, point);

              if (abs(HiWord(templong)) + abs(LoWord(templong)) < 5) {
              dopick:
                theControl = pick;
                itemselectold = itemselectnew;
                doit = TRUE;
                goto forcepick;
              }
            }

            thePart = FindControl(point, itemswindow, &theControl);
            break;

          default:
            goto tryagain;
            break;
        }

      checkcontrols:

        icon.left = 10;
        icon.right = icon.left + 32;
        box.left = 10;
        box.right = 284 + leftshift;
        ti = c[charselectnew].items[itemselectnew + incr].id;
        ts = getselection(ti);
        ti -= ts;

        if (theControl == split) {
          ident = c[charselectnew].items[itemselectnew + incr].ident;
          charge = c[charselectnew].items[itemselectnew + incr].charge;
          temp = c[charselectnew].numitems;

          loaditem(c[charselectnew].items[itemselectnew + incr].id);

          if (temp < 30) {
            if ((charge < 2) || (!item.xcharge)) {
              SetPort(GetWindowPort(itemswindow));
              warn(42);
              SetPort(GetWindowPort(itemswindow));
              goto backup;
            }

            GetControlBounds(split, &r);
            ploticon3(133, r);
            sound(678);
            c[charselectnew].items[itemselectnew + incr].charge = charge / 2;
            c[charselectnew].items[temp].id = c[charselectnew].items[itemselectnew + incr].id;
            c[charselectnew].items[temp].equip = 0;
            c[charselectnew].items[temp].ident = ident;
            c[charselectnew].items[temp].charge = charge / 2;
            if (c[charselectnew].items[itemselectnew + incr].charge + c[charselectnew].items[temp].charge < charge)
              c[charselectnew].items[itemselectnew + incr].charge++;
            c[charselectnew].numitems++;
            theControl = tempdummy;
            updateitems(itemselectnew, 10);
          } else {
            SetPort(GetWindowPort(itemswindow));
            warn(43);
            SetPort(GetWindowPort(itemswindow));
            goto backup;
          }
          GetControlBounds(split, &r);
          ploticon3(134, r);
        }

        if (theControl == join) {
          loaditem(ti + ts);
          temp = first = equip = ident = 0;
          if (item.xcharge) {
            GetControlBounds(join, &r);
            ploticon3(133, r);
            for (count = first; count < 30; count++) {
              if (c[charselectnew].items[count].id == (ti + ts)) {
                sound(663);
                if (c[charselectnew].items[count].equip)
                  equip = TRUE;
                if (c[charselectnew].items[count].ident)
                  ident = TRUE;
                temp += c[charselectnew].items[count].charge;
                if (first) {
                  c[charselectnew].load += (item.wieght + (c[charselectnew].items[count].charge * item.xcharge));
                  dropitem(charselectnew, c[charselectnew].items[count].id, count, 0, FALSE);
                  count--;
                } else
                  first = count + 1;
              }
            }
            c[charselectnew].items[first - 1].equip = equip;
            c[charselectnew].items[first - 1].ident = ident;
            c[charselectnew].items[first - 1].charge = temp;
            compactitems(charselectnew);
            theControl = tempdummy;
            updateitems(0, 10);
            GetControlBounds(join, &r);
            ploticon3(134, r);
          } else {
            SetPort(GetWindowPort(itemswindow));
            warn(41);
            SetPort(GetWindowPort(itemswindow));
            goto backup;
          }
        }
        GetControlBounds(theControl, &r);
        if ((theControl == use) && (c[charselectnew].numitems)) {
          memoryspell = FALSE;
          if ((incombat) && (charselectnew != charup)) {
            SetPort(GetWindowPort(itemswindow));
            warn(10);
            SetPort(GetWindowPort(itemswindow));
            goto backup;
          }
          ploticon3(129, r);

          itemused = ti + ts;
          loaditem(itemused);

          if ((item.type == 25) && (item.sp2 < 1100) && (!encountflag)) {
            warn(88);
            warn(89);
          }

          usecharge = FALSE;

          if (c[charselectnew].items[itemselectnew + incr].charge > 0)
            usecharge = TRUE;
          if ((abs(item.type) == 23) || (item.sp1 == -23)) /**** door item *****/
          {
            needdungeonupdate = TRUE;
            if ((incombat) && (item.sp1 != -23)) {
              SetPort(GetWindowPort(itemswindow));
              warn(52);
              SetPort(GetWindowPort(itemswindow));
              goto backup;
            }

            if (c[charselectnew].items[itemselectnew + incr].charge > 0) {
              c[charselectnew].items[itemselectnew + incr].charge--;
            }

            itemused = -(item.sp5 + 100);

            c[charselectnew].load -= item.xcharge;

            if ((!c[charselectnew].items[itemselectnew + incr].charge) && (item.drop)) {
              backvalue = 3;
              play = 1;
              goto dropitemsfast;
            }

            goto getout;
          }

          if ((item.sp2 > 1100) && ((usecharge) || (item.charge < 0))) {
            if (!canuse(-1, c[charselectnew].race, c[charselectnew].caste) && (!encountflag)) {
              SetPort(GetWindowPort(itemswindow));
              warn(29);
              SetPort(GetWindowPort(itemswindow));
              itemused = 0;
              goto backup;
            }

            if ((item.sp2 > 1100) && ((usecharge) || (item.charge < 0)))
              loadspell2(item.sp2);

            if (spellinfo.spellclass == 9) {
              if (!incombat)
                goto tryagain;
              if (checkforenemy(1)) {
                SetPort(GetWindowPort(itemswindow));
                warn(39);
                SetPort(GetWindowPort(itemswindow));
                itemused = 0;
                goto backup;
              }
            }
          }

          if ((item.sp2 > 1100) && ((usecharge) || (item.charge < 0))) {
            loadspell2(item.sp2);
            powerlevel = abs(item.sp1);
            if (powerlevel == 8)
              powerlevel = Rand(7);

            if ((incombat) && (!spellinfo.incombat)) {
              SetPort(GetWindowPort(itemswindow));
              warn(23);
              SetPort(GetWindowPort(itemswindow));
              itemused = 0;
              goto backup;
            } else if ((!spellinfo.incamp) && (!incombat) && (!encountflag)) {
              SetPort(GetWindowPort(itemswindow));
              warn(22);
              SetPort(GetWindowPort(itemswindow));
              itemused = 0;
              goto backup;
            }
          }

          GetControlBounds(use, &r);
          ploticon3(134, r);

          if (((c[charselectnew].items[itemselectnew + incr].charge > 0) || (c[charselectnew].items[itemselectnew + incr].charge == -1)) && (item.sp2 > 1100)) {
            if (c[charselectnew].items[itemselectnew + incr].charge > 0) {
              c[charselectnew].items[itemselectnew + incr].charge--;
              c[charselectnew].load -= item.xcharge;
            }

            if ((!c[charselectnew].items[itemselectnew + incr].charge) && (item.drop)) {
              backvalue = play = 1;
              goto dropitemsfast;
            } else {
              theControl = tempdummy;
              updateitems(itemselectnew, itemselectnew);
            }
          }

        bringback1:
          loaditem(itemused);

          if (item.sound)
            sound(item.sound + 600);

          cleartarget();
          temp = 0;

          if ((item.sp2 > 1100) && ((usecharge) || (item.charge < 0))) {
            loadspell2(item.sp2);

            if (incombat) {
              inspell = TRUE;
              goto getout;
            } else {
            campcast:
              if ((spellinfo.targettype > 2) || (spellinfo.targettype == 7))
                temp = charnum;

              if (spellinfo.targettype == 5) {
                track[charselectnew] = 1;
                goto pushon;
              }

              if (!spellinfo.targettype)
                temp = powerlevel - 1;

              if (temp < charnum) {
                SelectWindow(itemswindow);
                flashmessage((StringPtr) "Select targets.                                   Hit 'A' to abort spell.                        Hit space to cast early.", 40, 100, -1, 6001);
                SetPort(GetWindowPort(itemswindow));
                shortupdate(1);
                getchoice(temp, 1, 1);
                flashmessage((StringPtr) "", 40, 100, -1, 0);
                SetPort(GetWindowPort(itemswindow));
              } else
                for (t = 0; t <= charnum; t++)
                  track[t] = 1;
            pushon:
              resolvespell();

              if (spellinfo.special == 48) {
                SetPort(GetWindowPort(itemswindow));
                BackColor(whiteColor);
                theControl = tempdummy;
                updateitems(0, 10);
              }

              if (encountflag)
                goto getout;

              if (FrontWindow() != GetDialogWindow(charstat)) {
                SelectWindow(GetDialogWindow(charstat));
                SetPortDialogPort(charstat);
                RGBForeColor(&greycolor);
                DrawDialog(charstat);
              }

              updatecharinfo();
              SetPort(GetWindowPort(itemswindow));
              BackColor(whiteColor);
              inspell = itemused = 0;

              if (cleanage) {
                cleanage = 0;
                updateitems(0, 10);
                goto backup;
              }

              if (select[charselectnew] < 0)
                goto backup;
              goto tryagain;
            }
          }

          if (encountflag)
            goto getout;

          if (abs(item.type) == 13) /***** scrollcase ****/
          {
            if (!c[charselectnew].armor[13]) {
              SetPort(GetWindowPort(itemswindow));
              warn(72);
              SetPort(GetWindowPort(itemswindow));
              goto backup;
            }

            if (incombat)
              charselectnew = q[up];

            if (!getscroll()) {
              itemused = 0;
              goto backup;
            }

            if (incombat)
              goto getout;

            SetPort(GetWindowPort(itemswindow));
            TextMode(0);
            ForeColor(yellowColor);
            RGBBackColor(&greycolor);
            pict(6, box2);
            plotportrait(c[charselectnew].pictid, charbox, c[charselectnew].caste, charselectnew);
            MoveTo(475 + leftshift, 450);
            ForeColor(yellowColor);
            MyrDrawCString((Ptr)c[charselectnew].name);
            BackColor(whiteColor);

            goto campcast;
          }

          /********* nothing else to do with is so lets just wear/remove it **********/
          goto pick;
        }
      bustout:

        if ((theControl == down) && (incr < 30)) {
          sound(141);

        onedown:
          SetPort(GetWindowPort(itemswindow));
          GetControlBounds(down, &r);
          ploticon3(133, r);
          box.top = 10;
          box.bottom = 458;
          box.left = 10;
          box.right = 284 + leftshift;
          EraseRect(&box3);
          TextSize(15);
          do {
            if (incr + 11 < c[charselectnew].numitems) {
              incr++;
              for (t = 0; t < 4; t++) {
                ScrollRect(&box, 0, -10, NIL);
              }
              delay(2); // Fantasoft 7.1  Slows it down a bit on fast machines.
              theControl = tempdummy;
              updateitems(10, 10);
              quickinfo(charselectnew, itemselectnew + incr, c[charselectnew].items[itemselectnew + incr].id, 0);

            } else {
              sound(6000);
              DrawPicture(marker, &box3);
              break;
            }
          } while (Button());

          if (gTheEvent.what == keyDown) {
            box3.top = 411;
            box3.bottom = 451;
          }

          DrawPicture(marker, &box3);

          GetControlBounds(down, &r);
          ploticon3(134, r);
        }

        GetControlBounds(theControl, &r);
        if (theControl == uparrow) {
          sound(141);

        oneup:
          SetPort(GetWindowPort(itemswindow));
          ploticon3(133, r);
          box.top = 0;
          box.bottom = 450;
          box.left = 10;
          box.right = 284 + leftshift;
          EraseRect(&box3);
          TextSize(15);
          do {

            if (incr) {
              incr--;
              for (t = 0; t < 4; t++)
                ScrollRect(&box, 0, 10, 0L);
              delay(2); // Fantasoft 7.1  Slows it down a bit on fast machines.
              theControl = tempdummy;
              updateitems(0, 0);
              quickinfo(charselectnew, itemselectnew + incr, c[charselectnew].items[itemselectnew + incr].id, 0);
            } else {
              sound(6000);
              DrawPicture(marker, &box3);
              break;
            }
          } while (Button());

          if (gTheEvent.what == keyDown) {
            box3.top = 11;
            box3.bottom = 51;
          }

          DrawPicture(marker, &box3);

          ploticon3(134, r);
        }

        if (theControl == charinfo) {
          ploticon3(129, r);
          sound(141);
          cl = charselectnew;
          viewcharacter(charselectnew, 0);
          SetPort(GetWindowPort(itemswindow));
          compactitems(charselectnew);
          theControl = tempdummy;
          updateitems(0, 10);
          charselectnew = cl;
          goto backup;
        }

        if (((theControl == identify) || (theControl == castident) || (theControl == describe)) && (c[charselectnew].numitems)) {
          if (!updatelist) {
            updatelist = TRUE;
            updatecanlist();
          }

        getinfo:

          ploticon3(129, r);

          if (theControl != describe) {
            if (theControl == identify) {
              if ((shopavail) || (templeavail)) {
                if (takegold(20, 0)) {
                  c[charselectnew].items[itemselectnew + incr].ident = TRUE;
                  sound(683);
                  goto moveon;
                } else
                  goto backup;
              } else {
                SetPort(GetWindowPort(itemswindow));
                warn(57);
                SetPort(GetWindowPort(itemswindow));
                goto backup;
              }
            } else {
              for (t = 0; t <= charnum; t++) {
                if ((c[t].spellpoints > 24) && (cancast(t, 0)) && (c[t].canidentify)) {
                  c[t].spellpoints -= 25;
                  for (tt = 0; tt < c[charselectnew].numitems; tt++)
                    c[charselectnew].items[tt].ident = TRUE;
                  theControl = tempdummy;
                  updateitems(0, 10);
                  select[t] = TRUE;
                  sound(683);
                  goto moveon;
                }
              }
              SetPort(GetWindowPort(itemswindow));
              warn(45);
              SetPort(GetWindowPort(itemswindow));
              goto backup;
            }
          }

          if (!reducesound)
            sound(30005);
        moveon:

          updateitems(itemselectnew, itemselectnew);

          if (theControl != describe)
            goto backup;

          showiteminfo(c[charselectnew].items[itemselectnew + incr].id, c[charselectnew].items[itemselectnew + incr].ident, c[charselectnew].items[itemselectnew + incr].charge, c[charselectnew].items[itemselectnew + incr].equip);
          SetPort(GetWindowPort(itemswindow));
          BackPixPat(whitepat);
          DrawDialog(charstat);
          goto backup;
        }

        if (theControl == trade) {
          if (!charnum) {
            SetPort(GetWindowPort(itemswindow));
            warn(62);
            SetPort(GetWindowPort(itemswindow));
            goto backup;
          }
          initems = FALSE;

          if ((!incombat) && (!encountflag)) {
            ploticon3(129, r);
            characterl = c[charselectnew];
            cl = charselectnew;
            cr = cl + 1;
            if (cr > charnum)
              cr = 0;
            if (cr != -1)
              characterr = c[cr];
            numitems = characterr.numitems;
            DisposeWindow(itemswindow);
            itemswindow = NIL;
            DisposeDialog(charstat);
            return (-2); /***** goto shop ****/
          } else {
            SetPort(GetWindowPort(itemswindow));
            warn(6);
            SetPort(GetWindowPort(itemswindow));
            goto backup;
          }
        }

        if (theControl == drop) {
          if ((dropitemprotection) && (!gTheEvent.modifiers & cmdKey)) {
            SetPort(GetWindowPort(itemswindow));
            warn(75);
            SetPort(GetWindowPort(itemswindow));
            goto backup;
          }

          if ((incombat) && (charselectnew != charup)) {
            SetPort(GetWindowPort(itemswindow));
            warn(10);
            SetPort(GetWindowPort(itemswindow));
            goto backup;
          }
          if (c[charselectnew].items[itemselectnew + incr].equip) {
            SetPort(GetWindowPort(itemswindow));
            warn(5);
            SetPort(GetWindowPort(itemswindow));
            goto backup;
          }
        }

        if ((theControl == drop) && (c[charselectnew].numitems)) {
          if (!c[charselectnew].items[itemselectnew + incr].id) {
            incr = itemselectnew = 0;
            theControl = tempdummy;
            updateitems(0, 10);

            FlushEvents(everyEvent, 0);
            goto backup;
          }

          ploticon3(129, r);
          play = TRUE;
        dropitemsfast:
          dropitem(charselectnew, c[charselectnew].items[itemselectnew + incr].id, itemselectnew + incr, play, FALSE);
          SetPortDialogPort(charstat);
          ForeColor(yellowColor);
          TextSize(20);
          DialogNum(26, c[charselectnew].movementmax);
          TextSize(16);
          DialogNum(25, c[charselectnew].load);

          SetPort(GetWindowPort(itemswindow));
          TextSize(15);
          TextMode(0);

          for (t = 0; t < 16; t++) {
            for (tt = 0; tt < 3; tt++) {
              ForeColor(whiteColor);
              box4.top = itemselectnew * 40 + 15 + t;
              box4.left = 10 + t;
              box4.right = box4.left + 32 - t * 2;
              box4.bottom = box4.top + 32 - t * 2;
              FrameRect(&box4);
            }
          }
          ForeColor(blackColor);
          theControl = tempdummy;
          updateitems(itemselectnew, 10);
          ploticon3(130, r);

          if ((itemselectnew + incr == c[charselectnew].numitems) && (itemselectnew)) {
            itemselectnew--;
            EraseRect(&box3);
            box3.top = 11 + itemselectnew * 40;
            box3.bottom = box3.top + 40;
            DrawPicture(marker, &box3);
          }

          if (backvalue == 1) {
            theControl = NIL;
            backvalue = 0;
            goto bringback1;
          }
          if (backvalue == 3)
            goto getout;
        }

        if (theControl == checkcontrol) {
        change:
          sound(141);
          incr = 0;
          if (c[charselectnew].load < 0)
            c[charselectnew].load = 0;

          SetPort(GetWindowPort(itemswindow));
          TextSize(15);

          GetMouse(&point);

          compactitems(charselectnew);

          checkrect.top = 386;
          checkrect.left = 353 + leftshift;
          checkrect.right = checkrect.left + 22;
          checkrect.bottom = checkrect.top + 22;

          for (t = 0; t <= charnum; t++) {
            if (t == 3)
              OffsetRect(&checkrect, 24, -71);

            if (PtInRect(point, &checkrect)) {
              if (t != charselectnew) {
                compactheap();
                charselectnew = t;
                compactitems(charselectnew);
                theControl = tempdummy;
                updateitems(0, 10);
                goto backup;
              }
            }
            OffsetRect(&checkrect, 0, 24);
          }
        }

        if (theControl == done) {
          initems = FALSE;
          ploticon3(129, r);
          sound(141);
          compactitems(charselectnew);
          DisposeWindow(itemswindow);
          DisposeDialog(charstat);
          itemswindow = NIL;
          charstat = NIL;
          if (encountflag)
            return (0);
          charselectold = charselectnew;
          if (!incombat) {
            if (indung) {
              updatemain(TRUE, -1);
              threed(-1L, 0, 0, 0);
            }
          }
          return (-3);
        }

        if (theControl == pick) {
          itemselectold = itemselectnew;
          itemselectnew = (point.v - 11) / 40;

        forcepick:

          EraseRect(&box3);

          if (itemselectnew >= c[charselectnew].numitems) {
            itemselectnew = c[charselectnew].numitems - 1;
          }

          if (itemselectnew < 0) {
            itemselectnew = 0;
            if (incr) {
              theControl = uparrow;
              sound(83);
              goto oneup;
            }
          }

          if (itemselectnew > 10) {
            itemselectnew = 10;
            sound(83);
            theControl = down;
            goto onedown;
          }

          box3.top = 11 + itemselectnew * 40;
          box3.bottom = box3.top + 40;
          DrawPicture(marker, &box3);

          quickinfo(charselectnew, itemselectnew + incr, c[charselectnew].items[itemselectnew + incr].id, 0);

          if (doit) {
            if (c[charselectnew].items[itemselectnew + incr].equip) {
              if (removeitem(charselectnew, itemselectnew + incr, TRUE, FALSE)) {
                theControl = tempdummy;
                updateitems(itemselectnew, itemselectnew);
                goto backup;
              }
            } else if (wear(charselectnew, itemselectnew + incr, TRUE)) {
              theControl = tempdummy;
              c[charselectnew].items[itemselectnew + incr].equip = 1;
              updateitems(itemselectnew, itemselectnew);
              goto backup;
            }
            SetPort(GetWindowPort(itemswindow));
          } else
            sound(83);
        }
    }
  }

getout:
  if (encountflag)
    track[charselectnew] = 1; /***** for encounter use ****/
  DisposeDialog(charstat);
  DisposeWindow(itemswindow);
  itemswindow = NIL;
  out();
  initems = FALSE;
  return (itemused);
}
