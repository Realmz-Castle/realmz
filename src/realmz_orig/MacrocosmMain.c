#include "prototypes.h"
#include "variables.h"

/**************************** updatecompass ************************/
void updatecompass(void) {
  Rect testRect;
  short temphead;

  testRect.top = 251 + downshift / 2;
  testRect.bottom = testRect.top + 69;
  testRect.left = 93 + leftshift / 2;
  testRect.right = testRect.left + 133;

  temphead = head * updatedir;

  switch (temphead) {
    case 1:
      pict(10134, testRect);
      break;

    case 2:
      pict(10135, testRect);
      break;

    case 3:
      pict(10136, testRect);
      break;

    case 4:
      pict(10137, testRect);
      break;

    default:
      pict(10138, testRect);
      break;
  }
  ForeColor(blackColor);
}

/************************* UpdateWindow *******************/
void UpdateWindow(short titleonly) {
  Rect source, dest;
  RGBColor color;

  if ((incombat) || (viewtype != 1))
    return;

  if (FrontWindow() != gWindow)
    SelectWindow(gWindow);

  SetPort(GetWindowPort(gWindow));
  ForeColor(blackColor);
  BackColor(whiteColor);

  PenPixPat(gNeutral);

  if (titleonly)
    SetRect(&dest, 40 + leftshift / 2, 12 + downshift / 2, 275 + leftshift / 2, 30 + downshift / 2);
  else
    SetRect(&dest, 0, 0, 320 + leftshift, 320 + downshift);

  PaintRect(&dest);

  PenPixPat(gHilite);
  MoveTo(28 + leftshift / 2, 225 + downshift / 2);
  LineTo(28 + leftshift / 2, 10 + downshift / 2);
  LineTo(291 + leftshift / 2, 10 + downshift / 2);
  MoveTo(32 + leftshift / 2, 222 + downshift / 2);
  LineTo(288 + leftshift / 2, 222 + downshift / 2);
  LineTo(288 + leftshift / 2, 30 + downshift / 2);

  PenPixPat(gShadow);
  MoveTo(29 + leftshift / 2, 225 + downshift / 2);
  LineTo(291 + leftshift / 2, 225 + downshift / 2);
  LineTo(291 + leftshift / 2, 11 + downshift / 2);
  MoveTo(31 + leftshift / 2, 222 + downshift / 2);
  LineTo(31 + leftshift / 2, 29 + downshift / 2);
  LineTo(288 + leftshift / 2, 29 + downshift / 2);

  TextFont(0);
  TextSize(12);
  color.red = 39384; // title
  color.green = 36384;
  color.blue = 39384;
  RGBForeColor(&color);

  if ((multiview) || (partycondition[4])) {
    MoveTo(81 + (leftshift / 2), 24 + downshift / 2);
    MyrDrawCString("Space bar to toggle view.");
  } else {
    MoveTo(81 + (leftshift / 2), 24 + downshift / 2);
    MyrDrawCString("       3D View Only.");
  }

  color.red = 12384; // title
  color.green = 10384;
  color.blue = 12384;
  RGBForeColor(&color);

  if ((multiview) || (partycondition[4])) {
    MoveTo(81 + (leftshift / 2), 24 + downshift / 2);
    MyrDrawCString("Space bar to toggle view.");
  } else {
    MoveTo(81 + (leftshift / 2), 24 + downshift / 2);
    MyrDrawCString("       3D View Only.");
  }

  ForeColor(blackColor);
  BackColor(whiteColor);

  if (titleonly)
    return;

  SetViewVariables();
  Render();
  SetRect(&source, 0, 0, 256, 192);
  SetRect(&dest, 32 + leftshift / 2, 30 + downshift / 2, 288 + leftshift / 2, 222 + downshift / 2);

  {
    BitMap* src = GetPortBitMapForCopyBits(gBackWorld);
    BitMap* dst = GetPortBitMapForCopyBits(GetQDGlobalsThePort());
    CopyBits(src, dst, &source, &dest, srcCopy, NIL);
  }
  updatecompass();
  updatefat(TRUE, 0, FALSE);
  BeginUpdate(gWindow);
  EndUpdate(gWindow);
  needdungeonupdate = FALSE;
}

/********************************** SetViewVariables ********************/
void SetViewVariables(void) {
  short h, v;
  short temphead = 1;

  v = floorx;
  h = floory;

  if (head > 2)
    temphead = 2;

  switch (temphead) {
    case 1: /**** north ****/
      hm1 = h - 1;
      hm2 = h - 2;
      hm3 = h - 3;
      hm4 = h - 4;

      hp1 = h + 1;
      hp2 = h + 2;
      hp3 = h + 3;
      hp4 = h + 4;

      vm1 = v - 1;
      vm2 = v - 2;
      vm3 = v - 3;
      vm4 = v - 4;

      vp1 = v + 1;
      vp2 = v + 2;
      vp3 = v + 3;
      vp4 = v + 4;
      break;

    case 2: /**** east ****/
      hp1 = h - 1;
      hp2 = h - 2;
      hp3 = h - 3;
      hp4 = h - 4;

      hm1 = h + 1;
      hm2 = h + 2;
      hm3 = h + 3;
      hm4 = h + 4;

      vp1 = v - 1;
      vp2 = v - 2;
      vp3 = v - 3;
      vp4 = v - 4;

      vm1 = v + 1;
      vm2 = v + 2;
      vm3 = v + 3;
      vm4 = v + 4;
      break;
  }

  temphead = 1;

  if (hp1 < 0)
    hp1 = 0;
  if (hp2 < 0)
    hp2 = 0;
  if (hp3 < 0)
    hp3 = 0;
  if (hp4 < 0)
    hp4 = 0;

  if (hm1 < 0)
    hm1 = 0;
  if (hm2 < 0)
    hm2 = 0;
  if (hm3 < 0)
    hm3 = 0;
  if (hm4 < 0)
    hm4 = 0;

  if (vp1 < 0)
    vp1 = 0;
  if (vp2 < 0)
    vp2 = 0;
  if (vp3 < 0)
    vp3 = 0;
  if (vp4 < 0)
    vp4 = 0;

  if (vm1 < 0)
    vm1 = 0;
  if (vm2 < 0)
    vm2 = 0;
  if (vm3 < 0)
    vm3 = 0;
  if (vm4 < 0)
    vm4 = 0;

  if ((head == 2) || (head == 4))
    temphead = 2;

  switch (temphead) {
    case 1: /*** north/south ****/

      viewdung[0] = field[hm4][vm2];
      viewdung[1] = field[hm4][vm1];
      viewdung[2] = field[hm4][v];
      viewdung[3] = field[hm4][vp1];
      viewdung[4] = field[hm4][vp2];

      viewdung[5] = field[hm3][vm1];
      viewdung[6] = field[hm3][v];
      viewdung[7] = field[hm3][vp1];

      viewdung[8] = field[hm2][vm1];
      viewdung[9] = field[hm2][v];
      viewdung[10] = field[hm2][vp1];

      viewdung[11] = field[hm1][vm1];
      viewdung[12] = field[hm1][v];
      viewdung[13] = field[hm1][vp1];

      for (t = 0; t < 14; t++) {
        gPlayer.hWall[t] = MyrBitTstShort(&viewdung[t], 15); /**** Wall ****/
        if (MyrBitTstShort(&viewdung[t], 14))
          gPlayer.hWall[t] = 4; /**** Door ****/
        if (MyrBitTstShort(&viewdung[t], 2))
          gPlayer.hWall[t] = 3; /**** Arch ****/
        if (MyrBitTstShort(&viewdung[t], 12))
          gPlayer.hWall[t] = 2; /**** Stair ****/
      }

      vviewdung[0] = viewdung[1];
      vviewdung[1] = viewdung[3];
      vviewdung[2] = viewdung[5];
      vviewdung[3] = viewdung[7];
      vviewdung[4] = viewdung[8];
      vviewdung[5] = viewdung[10];
      vviewdung[6] = viewdung[11];
      vviewdung[7] = viewdung[13];
      vviewdung[8] = field[h][vm1];
      vviewdung[9] = field[h][vp1];

      for (t = 0; t < 10; t++) {
        gPlayer.vWall[t] = MyrBitTstShort(&vviewdung[t], 15); /**** Wall ****/
        if (MyrBitTstShort(&vviewdung[t], 13))
          gPlayer.vWall[t] = 3; /**** verticle door ***/
        if (BitAnd(vviewdung[t], 8200))
          gPlayer.vWall[t] = 2; /**** arch stair door ***/
      }

      gPlayer.pillar[0] = MyrBitTstShort(&viewdung[1], 11) + MyrBitTstShort(&viewdung[2], 11) + MyrBitTstShort(&viewdung[5], 11) + MyrBitTstShort(&viewdung[6], 11);
      gPlayer.pillar[1] = MyrBitTstShort(&viewdung[3], 11) + MyrBitTstShort(&viewdung[2], 11) + MyrBitTstShort(&viewdung[7], 11) + MyrBitTstShort(&viewdung[6], 11);
      gPlayer.pillar[2] = MyrBitTstShort(&viewdung[5], 11) + MyrBitTstShort(&viewdung[6], 11) + MyrBitTstShort(&viewdung[8], 11) + MyrBitTstShort(&viewdung[9], 11);
      gPlayer.pillar[3] = MyrBitTstShort(&viewdung[7], 11) + MyrBitTstShort(&viewdung[6], 11) + MyrBitTstShort(&viewdung[10], 11) + MyrBitTstShort(&viewdung[9], 11);
      gPlayer.pillar[4] = MyrBitTstShort(&viewdung[8], 11) + MyrBitTstShort(&viewdung[9], 11) + MyrBitTstShort(&viewdung[11], 11) + MyrBitTstShort(&viewdung[12], 11);
      gPlayer.pillar[5] = MyrBitTstShort(&viewdung[10], 11) + MyrBitTstShort(&viewdung[9], 11) + MyrBitTstShort(&viewdung[13], 11) + MyrBitTstShort(&viewdung[12], 11);
      gPlayer.pillar[6] = MyrBitTstShort(&viewdung[11], 11) + MyrBitTstShort(&viewdung[12], 11) + MyrBitTstShort(&vviewdung[8], 11) + MyrBitTstShort(&field[h][v], 11);
      gPlayer.pillar[7] = MyrBitTstShort(&viewdung[13], 11) + MyrBitTstShort(&viewdung[12], 11) + MyrBitTstShort(&vviewdung[9], 11) + MyrBitTstShort(&field[h][v], 11);

      break;

    case 2: /**** east/west ****/

      viewdung[0] = field[hm2][vp4];
      viewdung[1] = field[hm1][vp4];
      viewdung[2] = field[h][vp4];
      viewdung[3] = field[hp1][vp4];
      viewdung[4] = field[hp2][vp4];

      viewdung[5] = field[hm1][vp3];
      viewdung[6] = field[h][vp3];
      viewdung[7] = field[hp1][vp3];

      viewdung[8] = field[hm1][vp2];
      viewdung[9] = field[h][vp2];
      viewdung[10] = field[hp1][vp2];

      viewdung[11] = field[hm1][vp1];
      viewdung[12] = field[h][vp1];
      viewdung[13] = field[hp1][vp1];

      for (t = 0; t < 14; t++) {
        gPlayer.hWall[t] = MyrBitTstShort(&viewdung[t], 15); /**** wall ****/
        if (MyrBitTstShort(&viewdung[t], 13))
          gPlayer.hWall[t] = 4; /**** horizontal door ***/
        if (MyrBitTstShort(&viewdung[t], 2))
          gPlayer.hWall[t] = 3; /**** horizontal arch ***/
        if (MyrBitTstShort(&viewdung[t], 12))
          gPlayer.hWall[t] = 2; /**** horizontal stair ***/
      }

      vviewdung[0] = viewdung[1];
      vviewdung[1] = viewdung[3];
      vviewdung[2] = viewdung[5];
      vviewdung[3] = viewdung[7];
      vviewdung[4] = viewdung[8];
      vviewdung[5] = viewdung[10];
      vviewdung[6] = viewdung[11];
      vviewdung[7] = viewdung[13];
      vviewdung[8] = field[hm1][v];
      vviewdung[9] = field[hp1][v];

      for (t = 0; t < 10; t++) {
        gPlayer.vWall[t] = MyrBitTstShort(&vviewdung[t], 15); /**** wall ****/
        if (MyrBitTstShort(&vviewdung[t], 14))
          gPlayer.vWall[t] = 3; /**** verticle door ***/
        if (BitAnd(vviewdung[t], 8200))
          gPlayer.vWall[t] = 2; /**** verticle arch/stair ***/
      }

      gPlayer.pillar[0] = MyrBitTstShort(&viewdung[1], 11) + MyrBitTstShort(&viewdung[2], 11) + MyrBitTstShort(&viewdung[5], 11) + MyrBitTstShort(&viewdung[6], 11);
      gPlayer.pillar[1] = MyrBitTstShort(&viewdung[3], 11) + MyrBitTstShort(&viewdung[2], 11) + MyrBitTstShort(&viewdung[7], 11) + MyrBitTstShort(&viewdung[6], 11);
      gPlayer.pillar[2] = MyrBitTstShort(&viewdung[5], 11) + MyrBitTstShort(&viewdung[6], 11) + MyrBitTstShort(&viewdung[8], 11) + MyrBitTstShort(&viewdung[9], 11);
      gPlayer.pillar[3] = MyrBitTstShort(&viewdung[7], 11) + MyrBitTstShort(&viewdung[6], 11) + MyrBitTstShort(&viewdung[10], 11) + MyrBitTstShort(&viewdung[9], 11);
      gPlayer.pillar[4] = MyrBitTstShort(&viewdung[8], 11) + MyrBitTstShort(&viewdung[9], 11) + MyrBitTstShort(&viewdung[11], 11) + MyrBitTstShort(&viewdung[12], 11);
      gPlayer.pillar[5] = MyrBitTstShort(&viewdung[10], 11) + MyrBitTstShort(&viewdung[9], 11) + MyrBitTstShort(&viewdung[13], 11) + MyrBitTstShort(&viewdung[12], 11);
      gPlayer.pillar[6] = MyrBitTstShort(&viewdung[11], 11) + MyrBitTstShort(&viewdung[12], 11) + MyrBitTstShort(&field[hm1][v], 11) + MyrBitTstShort(&field[h][v], 11);
      gPlayer.pillar[7] = MyrBitTstShort(&viewdung[13], 11) + MyrBitTstShort(&viewdung[12], 11) + MyrBitTstShort(&field[hp1][v], 11) + MyrBitTstShort(&field[h][v], 11);

      break;
  }
}
