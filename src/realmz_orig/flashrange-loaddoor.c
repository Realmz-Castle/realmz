#include "prototypes.h"
#include "variables.h"

/***************************************** flashrange ***********************/
void flashrange(short which, short who) {
  short range;

  if (which < 9)
    checkrange(who, pos[which][0], pos[which][1]);
  else
    checkrange(who, monpos[which - 10][0], monpos[which - 10][1]);

  range = currange;
  SetPort(GetWindowPort(screen));
  pict(156, infosmall);
  TextSize(24);
  ForeColor(yellowColor);
  MoveTo(425 + leftshift, 440 + downshift);
  MyrDrawCString("Range = ");
  ForeColor(whiteColor);

  string(range);
}

/****************************** loaddoor ****************************/
void loaddoor(long id, short index) {
  FILE* fp = NULL;

  if (indung) {
    if ((fp = MyrFopen(":Data Files:CD", "rb")) == NULL)
      scratch(56);
  } else if ((fp = MyrFopen(":Data Files:CL", "rb")) == NULL)
    scratch(57);

  fseek(fp, (id * (sizeof field + sizeof door + sizeof randlevel)) + (index * sizeof infodoor), SEEK_SET);
  fread(&infodoor, sizeof infodoor, 1, fp);
  CvtDoorToPc(&infodoor);

  fclose(fp);
}

/********************************** loaddoor2 ******************/
void loaddoor2(short id) {
  FILE* fp = NULL;
  struct door tempdoor;

  getfilename("Data ED3");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(58);
  fseek(fp, id * sizeof infodoor, SEEK_SET);
  fread(&tempdoor, sizeof tempdoor, 1, fp);
  CvtDoorToPc(&tempdoor);
  fclose(fp);
  tempdoor.landx = infodoor.landx;
  tempdoor.landy = infodoor.landy;
  tempdoor.percent = infodoor.percent;
  tempdoor.landid = infodoor.landid;
  tempdoor.doorid = infodoor.doorid;
  infodoor = tempdoor;
}
