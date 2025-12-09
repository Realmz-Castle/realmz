#include "prototypes.h"
#include "variables.h"

/************************** threed *********************/
void threed(long id, short gox, short goy, short dir) {
  long checktime;
  short cursor, oldcursor, wall, cancelkey, a, reply = FALSE;
  Boolean forward, skiptime = TRUE;
  WindowPtr whichWindow;
  Boolean bringbacktag, goodkey;

  static long x, y;
  static short deltax = 0;
  static short deltay = 0;

  editon = bringbacktag = needdungeonupdate = 0;
  indung = TRUE;

  UpdateWindow(FALSE);

  if (id < 0)
    goto update;

  loadland(id, TRUE);

  tagnew = 0;
  tagold = -1;

  floorx = gox;
  floory = goy;
  head = dir;

update:

  for (t = -1; t < 2; t++) /**** map area ****/
  {
    for (tt = -1; tt < 2; tt++) {
      MyrBitClrShort(&field[t + floory][tt + floorx], 8);
    }
  }

move:

  checktime = TickCount();

  updatewalls(floorx - 10, floory - 10);

  if (!skiptime) {
    if (checkforsecret(FALSE))
      updatewalls(floorx - 10, floory - 10);
  }

  if (bringbacktag)
    goto bringback;

  if (!skiptime)
    sound(130);

  for (;;) {
  startover:

    if (StillDown()) {
      delay(10);
      GetMouse(&point);
      if (StillDown())
        goto doitagainsam;
    }

    key = goodkey = skiptime = forward = 0;

    if (gTheEvent.modifiers & alphaLock)
      warn(21);

    SystemTask();

    a = GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
    DoCorrectBugMADRepeat();
#endif
    MyrCheckMemory(2);

    if (gTheEvent.what == autoKey)
      goto dokey;

    if (IsDialogEvent(&gTheEvent)) {
      a = DialogSelect(&gTheEvent, &dummy, &itemHit);
    } else {

      point = gTheEvent.where;
      GlobalToLocal(&(point));

    doitagainsam:

      if (!hide) {
        if (!PtInRect(point, &lookrect)) {
          SetCCursor(sword);
          tagold = oldcursor = 20;
          cursor = 0;
        } else if (viewtype == 1) {
          cursor = 128; //*** default to sword in case something goes ape.

          itemRect = AHEAD_RECT;
          if (PtInRect(point, &itemRect))
            cursor = 165;

          itemRect = BACK_RECT;
          if (PtInRect(point, &itemRect))
            cursor = 166;

          itemRect = TURN_LEFT_RECT;
          if (PtInRect(point, &itemRect))
            cursor = 167;

          itemRect = TURN_RIGHT_RECT;
          if (PtInRect(point, &itemRect))
            cursor = 168;

          if (cursor != oldcursor) {
            oldcursor = cursor;
            nokeys = GetCCursor(cursor);

            SetCCursor(nokeys);
            DisposeCCursor(nokeys);
          }

        } else {
          /**** 2D cursors ****/
          cursor = 142;

          if ((point.h >= point.v) && (point.h <= 320 + (leftshift / 2) - point.v))
            cursor = 139;

          if ((point.h <= point.v) && (point.v <= 320 + (leftshift / 2) - point.h))
            cursor = 141;

          if ((point.v >= 320 + (downshift / 2) - point.h) && (point.v >= point.h))
            cursor = 145;

          if ((point.h >= 320 + (downshift / 2) - point.v) && (point.v <= point.h))
            cursor = 143;

          if (cursor != oldcursor) {
            oldcursor = cursor;
            nokeys = GetCCursor(cursor);

            SetCCursor(nokeys);
            DisposeCCursor(nokeys);
          }
        }
      }

      tickcheck();

    jumpin:
      switch (gTheEvent.what) {
        case updateEvt:

          BeginUpdate(screen);
          EndUpdate(screen);

          BeginUpdate(look);
          EndUpdate(look);

          BeginUpdate(gWindow);
          EndUpdate(gWindow);

          break;

        case osEvt:
          if ((gTheEvent.message >> 24) == suspendResumeMessage) {
            if (hide) {
              compactheap();
              needdungeonupdate = TRUE;
              hide = FALSE;
              updatemain(FALSE, 0);
              FlushEvents(everyEvent, 0);
            } else
              hide = TRUE;
          }
          break;

        case mouseDown:

          GetMouse(&point);
          GlobalToLocal(&(point));

          thePart = FindWindow(gTheEvent.where, &whichWindow);

          theControl = NIL;

          if (cursor > 20) /*** mouse movement ***/
          {
            switch (cursor) {
              case 165:
                goto goup;
                break;

              case 166:
                goto godown;
                break;

              case 167:
                deltax = deltay = 0;
                goto goleft;
                break;

              case 168:
                deltax = deltay = 0;
                goto goright;
                break;
                /**** above = 3D, below = 2D ******/
              case 139: /*** up ***/
                head = 1;
                deltax = 0;
                deltay = -1;
                goto goup;
                break;

              case 145: /*** down ***/
                head = 3;
                deltax = 0;
                deltay = 1;
                goto goup;
                break;

              case 141: /*** left ***/
                head = 4;
                deltax = -1;
                deltay = 0;
                goto goup;
                break;

              case 143: /*** right ***/
                head = 2;
                deltax = 1;
                deltay = 0;
                goto goup;
                break;
            }
          }

          switch (thePart) {
            case inMenuBar:
              compactheap();
              menuChoice = MenuSelect(gTheEvent.where);
            gomenu:
              HandleMenuChoice();
              skiptime = TRUE;
              SetPort(GetWindowPort(look));
              ForeColor(blackColor);
              BackColor(whiteColor);
              SetPort(GetWindowPort(screen));
              if (revertgame)
                return;
              if (!indung)
                return; /***** take out ****/
              goto update;
              break;

            case inSysWindow:
              SystemClick(&gTheEvent, whichWindow);
              break;

            case inContent:

              point = gTheEvent.where;
              GlobalToLocal(&(point));

              thePart = FindControl(point, screen, &theControl);

              reply = FALSE;

            goback:

              reply = buttonchoice(reply);

              if (revertgame)
                return;

              if ((theControl == tradebut) && (charnum))
                UpdateWindow(FALSE);
              if (theControl == shopbut)
                UpdateWindow(FALSE);
              if (theControl == itemsbut)
                UpdateWindow(FALSE);
              if (theControl == swapbut)
                UpdateWindow(FALSE);

              if (!indung)
                return; /***** leave dungeon ***/

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

              FlushEvents(everyEvent, 0);

              skiptime = TRUE;
              SetPort(GetWindowPort(look));
              ForeColor(blackColor);
              BackColor(whiteColor);
              SetPort(GetWindowPort(screen));
              goto update;
              break;
          }
          break;

        case autoKey:
          goto dokey;
          break;

        case keyDown:

        dokey:

          ObscureCursor();

          key = gTheEvent.message & charCodeMask;
          FlushEvents(everyEvent, 0);

          if (gTheEvent.modifiers & cmdKey) {
            commandkey = BitAnd(gTheEvent.message, charCodeMask);
            menuChoice = MenuKey(commandkey);
            goto gomenu;
          }

          deltax = deltay = 0;

          theControl = NIL;

          switch (key) {
            case ' ': /*** space  (Switch view mode if possible)   *****/

              if ((multiview) || (partycondition[4])) {
                viewtype *= -1;
                if (viewtype == 1) { /* 3D */
                  UpdateWindow(FALSE);
                  goto startover;
                } else
                  SelectWindow(look);
              } else {
                warn(95);
                updatewalls(floorx - 10, floory - 10);
              }

              goto update;
              break;

            case 'p': /***** p   scroll    *****/
              if ((!inspell) && (checkfortype(charselectnew, 13, TRUE)))
                theControl = viewspellsbut;
              break;
            case 'k': /***** k   create scroll    *****/
              if ((!inspell) && (checkfortype(charselectnew, 13, TRUE))) {
                if ((incamp) && (c[charselectnew].stamina > 0))
                  theControl = overviewbut;
              }
              break;
            case 'c': /***** c   Camp    *****/
              theControl = campbut;
              break;
            case 'i': /***** i   items    *****/
              theControl = itemsbut;
              break;
            case 'm': /***** m   money    *****/
              theControl = swapbut;
              break;
            case 's': /***** s   spells    *****/
              theControl = castspellsbut;
              break;
            case 't': /***** t   trade    *****/
              if (charnum)
                theControl = tradebut;
              break;
            case 'h': /***** h   heal    *****/
              theControl = barbut;
              break;
            case 'r': /***** r   rest    *****/
              if (incamp == TRUE)
                theControl = rest;
              break;
            case 'e': /***** e   encounter    *****/
              if ((!incamp) && (!shopavail) && (!templeavail))
                theControl = shopbut;
              break;
            case 'a': /***** a   area search    *****/
              if (!incamp)
                theControl = overviewbut;
              break;
            case 'g': /***** g   go shop    *****/
              if ((!incamp) && ((shopavail) || (templeavail)))
                theControl = shopbut;
              break;

            case '8': /***** up ********/
            case 0x1e:
            goup:
              reRender = forward = TRUE;

              switch (head) {
                case 1:
                  deltay = -1;
                  break;

                case 2:
                  deltax = 1;
                  break;

                case 3:
                  deltay = 1;
                  break;

                case 4:
                  deltax = -1;
                  break;
              }

              break;

            case '2': /***** down ********/
            case 0x1f:
            godown:

              reRender = TRUE;

              switch (head) {
                case 1:
                  deltay = 1;
                  break;

                case 2:
                  deltax = -1;
                  break;

                case 3:
                  deltay = -1;
                  break;

                case 4:
                  deltax = 1;
                  break;
              }
              break;

            case '6': /***** right ********/

            goright:
              head++;
              reRender = TRUE;
              break;

            case 0x1d: /***** right ********/
              goto goright;
              reRender = TRUE;
              break;

            case '4': /***** left ********/

            goleft:
              head--;
              reRender = TRUE;
              break;

            case 0x1c: /***** left ********/
              goto goleft;
              reRender = TRUE;
              break;
          }

          if (head > 4)
            head = 1;
          if (head < 1)
            head = 4;

          if (theControl != NIL)
            goto goback;

        makemove:

          if ((!deltax) && (!deltay))
            goto update;

          if (fat > 134) {
            warn(54);
            goto startover;
          }

          checkmoneypool();
          goodkey = TRUE;
          if ((incamp) || (templeavail) || (shopavail)) {
            incamp = templeavail = shopavail = inshop = currentshop = 0;
            updatecontrols();
            if (!skiptime)
              timeclick(2, TRUE);
            if (revertgame)
              return;
            if (!indung)
              return;
          }
          wall = field[floory + deltay][floorx + deltax];
          if ((wall) && (!MyrBitTstShort((void*)(&wall), 14)) && (!MyrBitTstShort((void*)(&wall), 13))) {

            if ((MyrBitTstShort((void*)(&wall), 8 - head)) && (forward)) /**** looking for secret area can pass****/
            {
              sound(85);
              MyrBitSetShort(&field[floory + deltay][floorx + deltax], 9);
              goto check1;
            }

            if (BitAnd(wall, 3840)) /**** looking for secret area cant pass****/
            {
              flashmessage((StringPtr) "Something prevents you from passing through in that direction.", 30, 375, 0, 5000);
            }

            if ((MyrBitTstShort((void*)(&wall), 10)) || (MyrBitTstShort((void*)(&wall), 3)))
              goto check1; /**** note / encounter ****/

            if (!MyrBitTstShort((void*)(&wall), 15))
              goto check1;
            if (Rand(100) < 50)
              sound(10121);
            else
              sound(10123);
          } else {
          check1:
            if (!skiptime)
              timeclick(1, TRUE);
            if (revertgame)
              return;
            if (!indung)
              return;

            if (BitAnd(wall, 6)) /**** some kind of door ****/
            {
              if ((!MyrBitTstShort(&wall, 2)) && (Rand(10) < 4))
                sound(134); /**** squeeky door ****/
            }

            floorx += deltax;
            floory += deltay;
          }
          while (TickCount() < checktime + 15) {
          }

          cancelkey = key;
          key = 0;

          if (MyrBitTstShort(&wall, 3)) /**** encounter ****/
          {
            bringbacktag = TRUE;
            goto move;
          bringback:
            bringbacktag = FALSE;
            if (newland(floorx, floory, 0, 0, 0)) {
              if (revertgame)
                return;
              if (!indung)
                return; /***** leave dungeon   Need in both places ***/
              deltax *= -1;
              deltay *= -1;
              if (needdungeonupdate)
                UpdateWindow(FALSE);
              goto makemove; /***** leave dungeon   Need in both places ***/
            }
            if (revertgame)
              return;
            if (needdungeonupdate)
              UpdateWindow(FALSE);
            if (!indung)
              return; /***** leave dungeon ***/
          }

          if (goodkey)
            goto update;
          break;
      }
      skiptime = TRUE;
    }
  }
}

/************************************** render *******************************/
void Render(void) {
  Rect source;

  SetRect(&source, 0, 0, 256, 192);
  MyrCopyScreen((CGrafPtr)gFloorWorld, (CGrafPtr)gBackWorld, &source, &source, srcCopy); // Myriad

  // vwalls 5th row ****************************************
  // *******************************************************

  if (gPlayer.vWall[0]) {
    MyrCopyScreen((CGrafPtr)gVWallsWorld, (CGrafPtr)gBackWorld, &VERTSOURCE[gPlayer.vWall[0] - 1][0], &VERTDEST[0], transparent); // Myriad
  }
  if (gPlayer.vWall[1]) {
    MyrCopyScreen((CGrafPtr)gVWallsWorld, (CGrafPtr)gBackWorld, &VERTSOURCE[gPlayer.vWall[1] - 1][0], &VERTDEST[1], transparent); // Myriad
  }

  // 4th row **********************************************
  // *******************************************************

  for (t = 0; t < 5; t++) {
    if (gPlayer.hWall[t]) {
      MyrCopyScreen((CGrafPtr)gHWallsWorld, (CGrafPtr)gBackWorld, &WALL4[gPlayer.hWall[t] - 1], &WALLDEST[t], transparent); // Myriad
    }
  }

  if (gPlayer.pillar[0]) {
    MyrCopyScreen((CGrafPtr)gPillarsWorld, (CGrafPtr)gBackWorld, &PILLAR_RECT_SOURCE[3], &LEFT_PILLAR_RECT[3], transparent); // Myriad
  }
  if (gPlayer.pillar[1]) {
    MyrCopyScreen((CGrafPtr)gPillarsWorld, (CGrafPtr)gBackWorld, &PILLAR_RECT_SOURCE[3], &RIGHT_PILLAR_RECT[3], transparent); // Myriad
  }
  // vwalls 4th row ****************************************
  // *******************************************************

  if (gPlayer.vWall[2]) {
    MyrCopyScreen((CGrafPtr)gVWallsWorld, (CGrafPtr)gBackWorld, &VERTSOURCE[gPlayer.vWall[2] - 1][2], &VERTDEST[2], transparent); // Myriad
  }
  if (gPlayer.vWall[3]) {
    MyrCopyScreen((CGrafPtr)gVWallsWorld, (CGrafPtr)gBackWorld, &VERTSOURCE[gPlayer.vWall[3] - 1][3], &VERTDEST[3], transparent); // Myriad
  }

  for (t = 5; t < 8; t++) {
    if (gPlayer.hWall[t]) {
      MyrCopyScreen((CGrafPtr)gHWallsWorld, (CGrafPtr)gBackWorld, &WALL3[gPlayer.hWall[t] - 1], &WALLDEST[t], transparent); // Myriad
    }
  }

  // 3rd row **********************************************
  // *******************************************************

  if (gPlayer.pillar[2]) {
    MyrCopyScreen((CGrafPtr)gPillarsWorld, (CGrafPtr)gBackWorld, &PILLAR_RECT_SOURCE[2], &LEFT_PILLAR_RECT[2], transparent); // Myriad
  }
  if (gPlayer.pillar[3]) {
    MyrCopyScreen((CGrafPtr)gPillarsWorld, (CGrafPtr)gBackWorld, &PILLAR_RECT_SOURCE[2], &RIGHT_PILLAR_RECT[2], transparent); // Myriad
  }
  // vwalls 3rd row ****************************************
  // *******************************************************

  if (gPlayer.vWall[4]) {
    MyrCopyScreen((CGrafPtr)gVWallsWorld, (CGrafPtr)gBackWorld, &VERTSOURCE[gPlayer.vWall[4] - 1][4], &VERTDEST[4], transparent); // Myriad
  }
  if (gPlayer.vWall[5]) {
    MyrCopyScreen((CGrafPtr)gVWallsWorld, (CGrafPtr)gBackWorld, &VERTSOURCE[gPlayer.vWall[5] - 1][5], &VERTDEST[5], transparent); // Myriad
  }

  for (t = 8; t < 11; t++) {
    if (gPlayer.hWall[t]) {
      MyrCopyScreen((CGrafPtr)gHWallsWorld, (CGrafPtr)gBackWorld, &WALL2[gPlayer.hWall[t] - 1], &WALLDEST[t], transparent); // Myriad
    }
  }

  // 2nd row **********************************************
  // *******************************************************

  if (gPlayer.pillar[4]) {
    MyrCopyScreen((CGrafPtr)gPillarsWorld, (CGrafPtr)gBackWorld, &PILLAR_RECT_SOURCE[1], &LEFT_PILLAR_RECT[1], transparent); // Myriad
  }
  if (gPlayer.pillar[5]) {
    MyrCopyScreen((CGrafPtr)gPillarsWorld, (CGrafPtr)gBackWorld, &PILLAR_RECT_SOURCE[1], &RIGHT_PILLAR_RECT[1], transparent); // Myriad
  }
  // vwalls 2nd row ****************************************
  // *******************************************************

  if (gPlayer.vWall[6]) {
    MyrCopyScreen((CGrafPtr)gVWallsWorld, (CGrafPtr)gBackWorld, &VERTSOURCE[gPlayer.vWall[6] - 1][6], &VERTDEST[6], transparent); // Myriad
  }
  if (gPlayer.vWall[7]) {
    MyrCopyScreen((CGrafPtr)gVWallsWorld, (CGrafPtr)gBackWorld, &VERTSOURCE[gPlayer.vWall[7] - 1][7], &VERTDEST[7], transparent); // Myriad
  }
  for (t = 11; t < 14; t++) {
    if (gPlayer.hWall[t]) {
      MyrCopyScreen((CGrafPtr)gHWallsWorld, (CGrafPtr)gBackWorld, &WALL1[gPlayer.hWall[t] - 1], &WALLDEST[t], transparent); // Myriad
    }
  }

  // 1st row **********************************************
  // *******************************************************

  if (gPlayer.pillar[6]) {
    MyrCopyScreen((CGrafPtr)gPillarsWorld, (CGrafPtr)gBackWorld, &PILLAR_RECT_SOURCE[0], &LEFT_PILLAR_RECT[0], transparent); // Myriad
  }
  if (gPlayer.pillar[7]) {
    MyrCopyScreen((CGrafPtr)gPillarsWorld, (CGrafPtr)gBackWorld, &PILLAR_RECT_SOURCE[0], &RIGHT_PILLAR_RECT[0], transparent); // Myriad
  }
  // vwalls 1st row ****************************************
  // *******************************************************

  if (gPlayer.vWall[8]) {
    MyrCopyScreen((CGrafPtr)gVWallsWorld, (CGrafPtr)gBackWorld, &VERTSOURCE[gPlayer.vWall[8] - 1][8], &VERTDEST[8], transparent); // Myriad
  }
  if (gPlayer.vWall[9]) {
    MyrCopyScreen((CGrafPtr)gVWallsWorld, (CGrafPtr)gBackWorld, &VERTSOURCE[gPlayer.vWall[9] - 1][9], &VERTDEST[9], transparent); // Myriad
  }
}

/******************* updatewalls ****************************/
void updatewalls(long x, long y) {
  Rect testRect, dest;
  long t, tt;

  if (Rand(100) < 10) {
    if ((!doreg3()) && (currentscenario > 10))
      scratch(452);
  }

  if (viewtype == 1) {
    if (FrontWindow() != gWindow)
      UpdateWindow(FALSE);
    else {
      CGrafPtr qdp;
      SetPort(GetWindowPort(gWindow));
      ForeColor(blackColor);
      BackColor(whiteColor);

      SetViewVariables();
      Render();
      SetRect(&testRect, 0, 0, 256, 192);
      SetRect(&dest, 32 + leftshift / 2, 30 + downshift / 2, 288 + leftshift / 2, 222 + downshift / 2);

      qdp = GetQDGlobalsThePort();
      MyrCopyScreen((CGrafPtr)gBackWorld, qdp, &testRect, &dest, srcCopy); // Myriad

      updatecompass();
    }
  } else {
    SetPort(GetWindowPort(look));
    if (FrontWindow() != look)
      SelectWindow(look);

    MyrCopyScreen((CGrafPtr)gthePixels, (CGrafPtr)gbuff2, &tiny[15], &lookrect, srcCopy); // Myriad

    box.top = -16;
    box.bottom = 0;

    for (t = y; t < y + 20; t++) {
      box.top += 16;
      box.bottom += 16;
      box.left = -16;
      box.right = 0;
      for (tt = x; tt < x + 20; tt++) {
        box.left += 16;
        box.right += 16;
        if ((t > -1) && (t < 90) && (tt > -1) && (tt < 90))
          plotwall(field[t][tt]);
      }
    }

    if (!editon) {
      box.top = box.left = 160;
      box.right = box.bottom = 176;
      MyrCopyScreen((CGrafPtr)gthePixels, (CGrafPtr)gbuff2, &tiny[head + 15], &box, transparent); // Myriad
    }

    {
      CGrafPtr qdp = GetQDGlobalsThePort();
      MyrCopyScreen((CGrafPtr)gbuff2, (CGrafPtr)qdp, &lookrect, &lookrect, srcCopy); // Myriad
    }
  }
  xy(0);
}

/******************** plotwall *********************/
void plotwall(short id) {
  char t, stop = 7;

  if (!editon) {
    if (MyrBitTstShort(&id, 8))
      return;
  } else
    stop += 5;

  for (t = 0; t < stop; t++)
    if (MyrBitTstShort(&id, 15 - t)) {
      MyrCopyScreen((CGrafPtr)gthePixels, (CGrafPtr)gbuff2, &tiny[t], &box, transparent); // Myriad
    }
}
