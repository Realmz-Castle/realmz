#include "prototypes.h"
#include "variables.h"

/********************************* showcontactinfo ***********************/
void showcontactinfo(void) {
  DialogRef contact;

  if (!loadcontact())
    return;

  contact = GetNewDialog(-7930, NIL, (WindowPtr)-1L);
  SetPortDialogPort(contact);
  gCurrent = contact;
  showzero = 0;

  DrawDialog(contact);
  showcontact();

  FlushEvents(everyEvent, 0);
  ModalDialog(0L, &itemHit);

  DisposeDialog(contact);
}

/************************ showcontact ****************/
void showcontact(void) {
  /************struct contactdata
  {
   Str255 scenarioname;
   Str255 version;
   Str255 date;
   Str255 authorsname;
   Str255 email;
   Str255 web;
   Str255 fee;
   Str255 payinfo[5];
   Str255 titles[5];
   Str255 description;
  };*******************/

  MyrPascalDiStr(2, contactinfo.scenarioname);
  MyrPascalDiStr(3, contactinfo.authorsname);
  MyrPascalDiStr(4, contactinfo.version);
  MyrPascalDiStr(5, contactinfo.date);
  MyrPascalDiStr(6, contactinfo.authorsname);
  if (!StringWidth(contactinfo.authorsname))
    MyrCDiStr(3, (StringPtr) "Fantasoft");
  if (!StringWidth(contactinfo.authorsname))
    MyrCDiStr(6, (StringPtr) "Fantasoft");

  MyrPascalDiStr(7, contactinfo.email);
  MyrPascalDiStr(8, contactinfo.web);
  MyrPascalDiStr(9, contactinfo.fee);

  MyrPascalDiStr(10, contactinfo.payinfo[0]);
  MyrPascalDiStr(11, contactinfo.payinfo[1]);
  MyrPascalDiStr(12, contactinfo.payinfo[2]);
  MyrPascalDiStr(13, contactinfo.payinfo[3]);

  MyrPascalDiStr(14, contactinfo.titles[0]);
  MyrPascalDiStr(15, contactinfo.titles[1]);
  MyrPascalDiStr(16, contactinfo.titles[2]);
  MyrPascalDiStr(17, contactinfo.titles[3]);
  MyrPascalDiStr(18, contactinfo.titles[4]);

  MyrPascalDiStr(19, contactinfo.description);
}

/************************ loadcontact ****************/
short loadcontact(void) {
  FILE* fp;

  getfilename("Data CI");
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    return (0);
  fread(&contactinfo, sizeof contactinfo, 1, fp);
  CvtContactToPc(&contactinfo);
  fclose(fp);
  return (1);
}
