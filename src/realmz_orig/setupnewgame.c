#include "prototypes.h"
#include "realmzbuild.h"
#include "variables.h"

/***************** setupnewgame *******************/
void setupnewgame(void) {
  FILE* fp = NULL;
  FILE* op = NULL;
  FILE* fp2 = NULL;
  FILE* fp3 = NULL;
  short temp;

  t = CountResources('RLMZ') - 1;

  if (currentscenario < 20) {
    if ((currentscenario - 5 != t) && (t)) {
      warn(92);
      scratch(-502);
    }
  }

  DrawDialog(background);
  gCurrent = background;
  SetPortDialogPort(background);
  TextFont(font);
  TextSize(32);
  BackColor(blackColor);
  ForeColor(yellowColor);
  MyrCDiStr(2, (StringPtr) "Building.....Please wait.");

  EnableItem(gScenario, 0);
  EnableItem(gParty, 0);
  DisableItem(gParty, 5);

  for (t = 0; t < 90; t++) {
    for (tt = 0; tt < 90; tt++)
      site[t][tt] = 0;
  }

  lowHD = FALSE;

  GetVInfo(0, myString, &temp, &templong);

  if (templong < 512000)
    lowHD = TRUE;

  nummon = numfoes = 0;
  for (t = 0; t < 10; t++) {
    partycondition[t] = 0;
    map[t] = 0;
    map[t + 10] = 0;
  }
  map[0] = TRUE;
  journalindex2 = 1; /**** reset journal to 0 notes *****/
  loaddark(0);

  updatemapmenu();

  if ((fp = MyrFopen(":Data Files:DN", "w+b")) == NULL)
    scratch2(5);
  fclose(fp);
  if ((fp = MyrFopen(":Data Files:WN", "w+b")) == NULL)
    scratch(157);
  fclose(fp);

  if ((op = MyrFopen(":Data Files:CL", "w+b")) == NULL)
    scratch(158); /********** land data ****/
  getfilename("Data DD");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(159);
  getfilename("Data LD");
  if ((fp2 = MyrFopen(filename, "rb")) == NULL)
    scratch(160);
  getfilename("Data RD");
  if ((fp3 = MyrFopen(filename, "rb")) == NULL)
    scratch(161);
  while (fread(&door, sizeof door, 1, fp) == 1) {
    fread(&field, sizeof field, 1, fp2);
    fread(&randlevel, sizeof randlevel, 1, fp3);
    if (!fwrite(&door, sizeof door, 1, op))
      scratch(162);
    if (!fwrite(&field, sizeof field, 1, op))
      scratch(163);
    if (!fwrite(&randlevel, sizeof randlevel, 1, op))
      scratch(164);
    if (!fwrite(&site, sizeof site, 1, op))
      scratch(164);
  }
  fclose(fp);
  fclose(fp2);
  fclose(fp3);
  fclose(op);

  if ((op = MyrFopen(":Data Files:CD", "w+b")) == NULL)
    scratch(165); /*************** dungeons ******/
  getfilename("Data DDD");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(166);
  getfilename("Data DL");
  if ((fp2 = MyrFopen(filename, "rb")) == NULL)
    scratch(167);
  getfilename("Data RDD");
  if ((fp3 = MyrFopen(filename, "rb")) == NULL)
    scratch(168);
  while (fread(&door, sizeof door, 1, fp) == 1) {
    fread(&field, sizeof field, 1, fp2);
    fread(&randlevel, sizeof randlevel, 1, fp3);
    if (!fwrite(&door, sizeof door, 1, op))
      scratch(169);
    if (!fwrite(&field, sizeof field, 1, op))
      scratch(170);
    if (!fwrite(&randlevel, sizeof randlevel, 1, op))
      scratch(171);
  }
  fclose(fp);
  fclose(fp2);
  fclose(fp3);
  fclose(op);

  getfilename("Data SD");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(172); /********* shops ********/
  if ((op = MyrFopen(":Data Files:CS", "w+b")) == NULL)
    scratch(173);
  while (fread(&theshop, sizeof theshop, 1, fp) == 1) {
    if (!fwrite(&theshop, sizeof theshop, 1, op))
      scratch(174);
  }
  fclose(fp);
  fclose(op);

  getfilename("Data TD2");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(175); /********* thief ********/
  if ((op = MyrFopen(":Data Files:CT", "w+b")) == NULL)
    scratch(176);
  while (fread(&thief, sizeof thief, 1, fp) == 1) {
    if (!fwrite(&thief, sizeof thief, 1, op))
      scratch(177);
  }
  fclose(fp);
  fclose(op);

  getfilename("Data TD3");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(178); /********* time encounters ********/
  if ((op = MyrFopen(":Data Files:CTD3", "w+b")) == NULL)
    scratch(179);
  while (fread(&dotime, sizeof dotime, 1, fp) == 1) {
    if (!fwrite(&dotime, sizeof dotime, 1, op))
      scratch(180);
  }
  fclose(fp);
  fclose(op);

  getfilename("Data ED");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(181); /********* simple encounters ********/
  if ((op = MyrFopen(":Data Files:CE", "w+b")) == NULL)
    scratch(182);
  while (fread(&enc, sizeof enc, 1, fp) == 1) {
    for (tt = 0; tt < 4; tt++) {
      GetIndString(buffer[tt], 3, 1);
      fread(&buffer[tt], 80, 1, fp);
    }
    if (!fwrite(&enc, sizeof enc, 1, op))
      scratch(183);
    for (tt = 0; tt < 4; tt++) {
      if (!fwrite(&buffer[tt], 80, 1, op))
        scratch(184);
    }
  }
  fclose(fp);
  fclose(op);

  getfilename("Data ED2");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(185); /********* complex encounters ********/
  if ((op = MyrFopen(":Data Files:CE2", "w+b")) == NULL)
    scratch(186);

  while (fread(&enc2, sizeof enc2, 1, fp)) {
    for (t = 0; t < 9; t++)
      fread(&buffer[t], 40, 1, fp);
    if (!fwrite(&enc2, sizeof enc2, 1, op))
      scratch(187);
    for (t = 0; t < 9; t++) {
      if (!fwrite(&buffer[t], 40, 1, op))
        scratch(188);
    }
  }
  fclose(fp);
  fclose(op);

  for (t = 0; t < 6; t++) {
    lastspell[t][0] = 0;
    lastspell[t][1] = 0;
    c[t].inbattle = TRUE;
  }

  lowHD = FALSE;

  GetMenuItemText(gGame, currentscenario, myString);
  PtoCstr(myString);
  getfilename((Ptr)myString);
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(189);
  // fread(&reclevel,sizeof reclevel,5,fp); Illegal
  fread(&reclevel, sizeof(long), 1, fp); // Myriad
  CvtLongToPc(&reclevel);
  fread(&maxlevel, sizeof(long), 1, fp); // Myriad
  CvtLongToPc(&maxlevel);
  fread(&landlevel, sizeof(long), 1, fp); // Myriad
  CvtLongToPc(&landlevel);
  fread(&lookx, sizeof(long), 1, fp); // Myriad
  CvtLongToPc(&lookx);
  fread(&looky, sizeof(long), 1, fp); // Myriad
  CvtLongToPc(&looky);

  fread(&codeseg1, 20, 1, fp);
  fread(&codeseg2, 20, 1, fp);
  fclose(fp);

  for (t = 0; t < 128; t++) {
    if (t <= templong)
      quest[t] = 0;
    else
      quest[t] = -1;
  }

  quest[0] = -1;

  updatemonstermenu(currentscenario);

  fat = 4;

  moneypool[0] = moneypool[1] = moneypool[2] = 0;
  campavail = canpriestturn = canencounter = TRUE;
  spellcasting = partyx = inboat = partyy = canshop = shopavail = inspell = incamp = incombat = killparty = editon = indung = heldover = cancamp = FALSE;

  loadland(landlevel, 1);
  BackColor(blackColor);
  ForeColor(yellowColor);
  MyrCDiStr(2, (StringPtr) "");
  BackColor(whiteColor);
  ForeColor(blackColor);

#if !development
  if (!usercheck2())
    DisableItem(gOptions, 0);
  else
    EnableItem(gOptions, 0);
#endif
}
