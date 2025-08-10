#include "prototypes.h"
#include "variables.h"

/********************************* fastplotmap ***************/
void fastplotmap(short id, Rect destrect) {
  FILE* fp = NULL;

  if (id < 0) {
    if (id < -999)
      id += 1000;
    if (id < -999)
      id += 1000;
    if (id < -999)
      id += 1000;

    fastplot(basetile[lastpix], destrect, 0, 0); /***** 0 = look, 1 = buff ******/
    ploticon3(id, destrect);
    return;
  }

  if (id > 999) {
    MyrBitClrShort(&id, 1);
    MyrBitClrShort(&id, 2);

    if (id > 999)
      id -= 1000;
    if (id > 999)
      id -= 1000;
    if (id > 999)
      id -= 1000;
  }

  if (id > 200) {
    fastplot(basetile[lastpix], destrect, 0, 0); /***** 0 = look, 1 = buff ******/
    return;
  }

  tempid = (id - 1) / 20;

  temp = id - (tempid * 20) - 1;

  itemRect.top = 32 * tempid;
  itemRect.left = 32 * temp;

  itemRect.right = itemRect.left + 32;
  itemRect.bottom = itemRect.top + 32;

  {
    BitMap* src = GetPortBitMapForCopyBits(gthePixels);
    BitMap* dst = GetPortBitMapForCopyBits(GetWindowPort(look));
    CopyBits(src, dst, &itemRect, &destrect, 0, NIL);
  }
}

/***************** showmap *******************/
void showmap(short mapnumber) {
  FILE* fp = NULL;
  DialogRef show;
  Boolean tag = FALSE;
  Rect temprect;
  short oldview, t, tt, temp, tempisdung;

  tempisdung = indung;
  oldview = viewtype;

  in();
  getfilename("Data MD2");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(115);
  fseek(fp, mapnumber * sizeof themap, SEEK_SET);
  fread(&themap, sizeof themap, 1, fp);
  CvtMapsToPc(&themap);
  fclose(fp);

  SelectWindow(look);

  viewtype = -1;

  if (themap.show < 0) {
    movie(themap.show, 129, 0);
    goto out;
  } else if (themap.pictid) {
    itemRect = lookrect;
    SetPort(GetWindowPort(look));
    if ((themap.rect[2]) || (themap.rect[3])) {
      itemRect.top = themap.rect[0];
      itemRect.left = themap.rect[1];
      itemRect.bottom = themap.rect[2];
      itemRect.right = themap.rect[3];
    }
    pict(themap.pictid, itemRect);
  } else {
    temp = 320 / themap.iconsize;
    if (temp * themap.iconsize < 320)
      temp++;

    if (!indung)
      saveland(landlevel);
    else
      saveland(dunglevel);

    templong = themap.level;
    indung = themap.isdungeon;
    loadland(templong, TRUE);

    xy(1);

    SetPort(GetWindowPort(look));

    ForeColor(blackColor);
    BackColor(whiteColor);

    temprect.top = temprect.left = 0;
    temprect.right = temprect.bottom = themap.iconsize;

    if (!indung) {
      /* *** CHANGED FROM ORIGINAL CODE ***
       * NOTE(fuzziqersoftware): Our window manager implementation recomposites
       * the entire window after each drawing call, which greatly amplifies the
       * work required to draw a map, especially if the map covers a large
       * area. To mitigate this, we disable recompositing temporarily and
       * manually recomposite after drawing all the map tiles. */
      WindowManager_SetEnableRecomposite(0);
      for (t = themap.starty; t < temp + themap.starty; t++) {
        for (tt = themap.startx; tt < themap.startx + temp; tt++) {
          fastplotmap(field[tt][t], temprect);
          OffsetRect(&temprect, themap.iconsize, 0);
        }
        OffsetRect(&temprect, -(themap.iconsize * temp), themap.iconsize);
      }
      WindowManager_SetEnableRecomposite(1);
    } else {
      SetPort(GetWindowPort(look));
      ForeColor(blackColor);
      BackColor(whiteColor);
      editon = TRUE;
      centerpict();
      xy(1);
      editon = FALSE;
      SetPort(GetWindowPort(look));
      tag = TRUE;
    }

    for (t = 0; t < 10; t++) {
      if (themap.icon[t][0]) {
        if (!themap.iconsize)
          themap.iconsize = 32;
        temp = themap.icon[t][2];
        temprect.top = temp * themap.iconsize;
        temprect.bottom = temprect.top + themap.iconsize;

        temp = themap.icon[t][1];
        temprect.left = temp * themap.iconsize;
        temprect.right = temprect.left + themap.iconsize;

        iconhand = NIL;
        iconhand = GetCIcon(themap.icon[t][0]);

        if ((themap.icon[t][0] == 137) || (139)) /*** let x and s be full size ***/
        {
          InsetRect(&temprect, -((32 - themap.iconsize) / 2), -((32 - themap.iconsize) / 2));
        }

        if (iconhand) {
          PlotCIcon(&temprect, iconhand);
          DisposeCIcon(iconhand);
        }
      }
    }
  }

  point.h = partyx + lookx;
  point.v = partyy + looky;

  itemRect.left = themap.startx - 1;
  itemRect.top = themap.starty - 1;
  itemRect.right = itemRect.left + 320 / themap.iconsize + 1;
  itemRect.bottom = itemRect.top + 320 / themap.iconsize + 1;

  if (themap.isdungeon == tempisdung) {
    if (((themap.isdungeon) && (themap.level == dunglevel)) || ((!themap.isdungeon) && (themap.level == landlevel))) {
      if (PtInRect(point, &itemRect)) /***** show you are here ****/
      {
        icon.left = (point.h - themap.startx) * themap.iconsize - 20;
        icon.top = (point.v - themap.starty) * themap.iconsize - 24;
        icon.right = icon.left + 64;
        icon.bottom = icon.top + 64;

        ploticon3(138, icon);
      }
    }
  }

  show = GetNewDialog(169, 0L, (WindowPtr)-1L);
  SetPortDialogPort(show);
  BackPixPat(base);
  TextFont(defaultfont);
  MoveWindow(GetDialogWindow(show), GlobalLeft - 1 + (leftshift / 2), GlobalTop + 321 + (downshift / 2), FALSE);
  ForeColor(yellowColor);
  gCurrent = show;
  ShowWindow(GetDialogWindow(show));
  ErasePortRect();
  TextMode(1);

  DrawDialog(show);
  MyrPascalDiStr(2, themap.note);

  indung = tempisdung;

  if (!indung)
    loadland(landlevel, TRUE);
  else
    loadland(dunglevel, TRUE);

  flashmessage((StringPtr) "Click Mouse", 350, 100, 0, 30005);

  xy(0);
  DisposeDialog(show);

out:

  SetPort(GetWindowPort(look));
  ForeColor(blackColor);
  BackColor(whiteColor);
  viewtype = oldview;
  if (indung) {
    if (viewtype == 1)
      UpdateWindow(FALSE);
  }
  updatemain(FALSE, -1);
}
