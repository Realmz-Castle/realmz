#include "prototypes.h"
#include "realmzbuild.h"
#include "variables.h"

#ifdef PC
#define MAGIC 'RLZ0'
static FILE* fPref;

/**************** openpref ********************
Myriad : PC side
Open a preference file in the preferences folder
=> 0 ouverture en lecture, 1 en ecriture
6 divinity
10 effacement
*/
void EraseFilePref(char* nom);
FILE* OpenFilePref(char* nom, char* attr);

short openpref(short mode) {
  Str255 pref_file_name;

  GetIndString(pref_file_name, STR_LIST_ID, PREF_STR_INDEX + divine);
  // Pre file name
  if (mode == 0) {
    fPref = OpenFilePref(pref_file_name, "rb");
  }
  if (mode == 1) {
    fPref = OpenFilePref(pref_file_name, (StringPtr) "wb");
  }
  if (mode == 6) /**** Get Divinity User Name. ****/
  {
    fPref = OpenFilePref("Divinity Prefs", (StringPtr) "ab+");
    fseek(fPref, 0L, SEEK_SET);
  }
  if (mode == 10) /**** Erase old Realmz Prefs. ****/
  {
    EraseFilePref("Realmz Preferences");
  }
  return (0);
}
static void closepref(void) {
  if (fPref)
    fclose(fPref);
  fPref = NULL;
}

/************************** savepref *************/
void savepref(void) {
  Handle new_data_handle;
  long magic;

  openpref(1);
  if (fPref) {
    new_data_handle = NewHandleClear(sizeof(PrefRecord));
    (**(PrefHandle)new_data_handle).delayspeed = delayspeed;
    (**(PrefHandle)new_data_handle).oldspeed = oldspeed;
    (**(PrefHandle)new_data_handle).oldvolume = musicvolume;
    (**(PrefHandle)new_data_handle).volume = volume;
    (**(PrefHandle)new_data_handle).defaultspell = defaultspell;
    (**(PrefHandle)new_data_handle).showcast = showcast;
    (**(PrefHandle)new_data_handle).usehashmarks = usehashmarks;
    (**(PrefHandle)new_data_handle).numchannel = numchannel;
    (**(PrefHandle)new_data_handle).lastgame = lastgame;
    (**(PrefHandle)new_data_handle).horseicon = horseicon;
    (**(PrefHandle)new_data_handle).dropitemprotection = dropitemprotection;
    (**(PrefHandle)new_data_handle).forgettreasure = forgettreasure;
    (**(PrefHandle)new_data_handle).fasttrade = fasttrade;
    (**(PrefHandle)new_data_handle).autocash = autocash;
    (**(PrefHandle)new_data_handle).autojoin = autojoin;
    (**(PrefHandle)new_data_handle).autoid = autoid;
    (**(PrefHandle)new_data_handle).usedefaultfont = usedefaultfont;
    (**(PrefHandle)new_data_handle).colormenus = colormenus;
    (**(PrefHandle)new_data_handle).showcaste = showcaste;
    (**(PrefHandle)new_data_handle).reducesound = reducesound;
    (**(PrefHandle)new_data_handle).showdescript = showdescript;
    (**(PrefHandle)new_data_handle).quickshow = quickshow;
    (**(PrefHandle)new_data_handle).hidedesktop = hidedesktop;
    (**(PrefHandle)new_data_handle).manualbandage = manualbandage;
    (**(PrefHandle)new_data_handle).showbleedmessage = showbleedmessage;
    (**(PrefHandle)new_data_handle).shownextroundmessage = shownextroundmessage;
    (**(PrefHandle)new_data_handle).auto256 = auto256;
    (**(PrefHandle)new_data_handle).iteminfo = iteminfo;
    (**(PrefHandle)new_data_handle).autoweapswitch = autoweapswitch;
    (**(PrefHandle)new_data_handle).nomusic = nomusic;
    (**(PrefHandle)new_data_handle).usenpc = usenpc;
    (**(PrefHandle)new_data_handle).castonfriends = castonfriends;
    (**(PrefHandle)new_data_handle).allowfumble = allowfumble;
    (**(PrefHandle)new_data_handle).allowunique = allowunique;
    (**(PrefHandle)new_data_handle).defaultfont = defaultfont;
    (**(PrefHandle)new_data_handle).serial = serial;
    (**(PrefHandle)new_data_handle).autonote = autonote;
    (**(PrefHandle)new_data_handle).portraitchoice = portraitchoice;
    (**(PrefHandle)new_data_handle).currentscenariohold = currentscenario;
    (**(PrefHandle)new_data_handle).blank3 = 0;
    (**(PrefHandle)new_data_handle).journalindex2 = 0;
    (**(PrefHandle)new_data_handle).blank5 = 0;
    (**(PrefHandle)new_data_handle).blank6 = 0;
    (**(PrefHandle)new_data_handle).blank7 = 0;
    (**(PrefHandle)new_data_handle).blank8 = 0;
    (**(PrefHandle)new_data_handle).blank9 = 0;
    (**(PrefHandle)new_data_handle).blank10 = 0;
    strcpy((**(PrefHandle)new_data_handle).name_str, Name_String);
    showserial = serial;
    MyrBitSetLong(&showserial, 6 + divine);

    magic = MAGIC;
    fwrite(&magic, sizeof(long), 1, fPref);
    fwrite(*new_data_handle, sizeof(PrefRecord), 1, fPref);

    closepref();
    DisposeHandle(new_data_handle);
  }
  return;
}

void getpref(void) {
  Handle data_handle;
  long magic = 0L;

  openpref(0);

  // File existe, I read it
  if (fPref) {
    fread(&magic, sizeof(long), 1, fPref);
    if (magic == MAGIC) {
      data_handle = NewHandleClear(sizeof(PrefRecord));
      fread(*data_handle, sizeof(PrefRecord), 1, fPref);
    }
    closepref();
  }
  if (magic != MAGIC) {
    // No pref file get default
    data_handle = GetResource('PRFN', PREF_RES_ID);
    DetachResource(data_handle);
    CvtShortToPc(&(**(PrefHandle)data_handle).delayspeed);
    CvtShortToPc(&(**(PrefHandle)data_handle).oldspeed);
    CvtShortToPc(&(**(PrefHandle)data_handle).oldvolume);
    CvtShortToPc(&(**(PrefHandle)data_handle).volume);
    CvtShortToPc(&(**(PrefHandle)data_handle).defaultfont);
  }

  delayspeed = (**(PrefHandle)data_handle).delayspeed;
  oldspeed = (**(PrefHandle)data_handle).oldspeed;
  musicvolume = (**(PrefHandle)data_handle).oldvolume;
  volume = (**(PrefHandle)data_handle).volume;
  defaultspell = (**(PrefHandle)data_handle).defaultspell;
  showcast = (**(PrefHandle)data_handle).showcast;
  usehashmarks = (**(PrefHandle)data_handle).usehashmarks;
  numchannel = (**(PrefHandle)data_handle).numchannel;
  lastgame = (**(PrefHandle)data_handle).lastgame;
  horseicon = (**(PrefHandle)data_handle).horseicon;
  dropitemprotection = (**(PrefHandle)data_handle).dropitemprotection;
  forgettreasure = (**(PrefHandle)data_handle).forgettreasure;
  fasttrade = (**(PrefHandle)data_handle).fasttrade;
  autocash = (**(PrefHandle)data_handle).autocash;
  autojoin = (**(PrefHandle)data_handle).autojoin;
  autoid = (**(PrefHandle)data_handle).autoid;
  usedefaultfont = (**(PrefHandle)data_handle).usedefaultfont;
  colormenus = (**(PrefHandle)data_handle).colormenus;
  showcaste = (**(PrefHandle)data_handle).showcaste;
  reducesound = (**(PrefHandle)data_handle).reducesound;
  showdescript = (**(PrefHandle)data_handle).showdescript;
  quickshow = (**(PrefHandle)data_handle).quickshow;
  hidedesktop = (**(PrefHandle)data_handle).hidedesktop;
  manualbandage = (**(PrefHandle)data_handle).manualbandage;
  showbleedmessage = (**(PrefHandle)data_handle).showbleedmessage;
  shownextroundmessage = (**(PrefHandle)data_handle).shownextroundmessage;
  auto256 = (**(PrefHandle)data_handle).auto256;
  iteminfo = (**(PrefHandle)data_handle).iteminfo;
  autoweapswitch = (**(PrefHandle)data_handle).autoweapswitch;
  nomusic = (**(PrefHandle)data_handle).nomusic;
  usenpc = (**(PrefHandle)data_handle).usenpc;
  castonfriends = (**(PrefHandle)data_handle).castonfriends;
  allowfumble = (**(PrefHandle)data_handle).allowfumble;
  allowunique = (**(PrefHandle)data_handle).allowunique;
  defaultfont = (**(PrefHandle)data_handle).defaultfont;
  defaultfont = 100;
  serial = (**(PrefHandle)data_handle).serial;

  strcpy(Name_String, (**(PrefHandle)data_handle).name_str);

  autonote = (**(PrefHandle)data_handle).autonote;
  portraitchoice = (**(PrefHandle)data_handle).portraitchoice;
  currentscenariohold = (**(PrefHandle)data_handle).currentscenariohold;
  blank3 = (**(PrefHandle)data_handle).blank3;
  journalindex2 = (**(PrefHandle)data_handle).journalindex2;
  blank5 = (**(PrefHandle)data_handle).blank5;
  blank6 = (**(PrefHandle)data_handle).blank6;
  blank7 = (**(PrefHandle)data_handle).blank7;
  blank8 = (**(PrefHandle)data_handle).blank8;
  blank9 = (**(PrefHandle)data_handle).blank9;
  blank10 = (**(PrefHandle)data_handle).blank10;

#if !divine
  if ((!serial) || (!MyrBitTstLong(&serial, 8))) {
    serial = Rand(21987); /***** generate new serial number ***/
    serial *= Rand(666);
    serial += Rand(32000);
    MyrBitSetLong(&serial, 8);
    MyrBitSetLong(&serial, 6 + divine);
    appnum = serial;

    PtoCstr((StringPtr)myString);

    if ((fp = MyrFopen((Ptr)myString, "r+b")) != NIL) {
      CvtLongToPc(&appnum);
      fwrite(&appnum, sizeof appnum, 1, fp);
      CvtLongToPc(&appnum);
      fclose(fp);
    }
  }
#endif

  showserial = serial;
  MyrBitSetLong(&showserial, 6 + divine);

  DisposeHandle(data_handle);
}

// Macintosh side
#else

/************************** savepref *************/
void savepref(void) {
  short pref_ref_num;
  Handle old_data_handle;
  Handle new_data_handle;
  PrefRecord* prefs;
  short res_ID;
  ResType res_type;
  Str255 res_name;
  short res_attributes;
  StringPtr source_str;
  Size byte_count;
  short oldresfile;

  oldresfile = CurResFile();

  pref_ref_num = openpref(0);

  UseResFile(pref_ref_num);

  new_data_handle = NewHandleClear(sizeof(PrefRecord));
  HLock(new_data_handle);
  prefs = *(PrefRecord**)new_data_handle;

  prefs->delayspeed = delayspeed;
  prefs->oldspeed = oldspeed;
  prefs->oldvolume = musicvolume;
  prefs->volume = volume;
  prefs->defaultspell = defaultspell;
  prefs->showcast = showcast;
  prefs->usehashmarks = usehashmarks;
  prefs->numchannel = numchannel;
  prefs->lastgame = lastgame;
  prefs->horseicon = horseicon;
  prefs->dropitemprotection = dropitemprotection;
  prefs->forgettreasure = forgettreasure;
  prefs->fasttrade = fasttrade;
  prefs->autocash = autocash;
  prefs->autojoin = autojoin;
  prefs->autoid = autoid;
  prefs->usedefaultfont = usedefaultfont;
  prefs->colormenus = colormenus;
  prefs->showcaste = showcaste;
  prefs->reducesound = reducesound;
  prefs->showdescript = showdescript;
  prefs->quickshow = quickshow;
  prefs->hidedesktop = hidedesktop;
  prefs->manualbandage = manualbandage;
  prefs->showbleedmessage = showbleedmessage;
  prefs->shownextroundmessage = shownextroundmessage;
  prefs->auto256 = auto256;
  prefs->iteminfo = iteminfo;
  prefs->autoweapswitch = autoweapswitch;
  prefs->nomusic = nomusic;
  prefs->usenpc = usenpc;
  prefs->castonfriends = castonfriends;
  prefs->allowfumble = allowfumble;
  prefs->allowunique = allowunique;
  prefs->defaultfont = defaultfont;
  prefs->serial = serial;
  prefs->autonote = autonote;
  prefs->portraitchoice = portraitchoice;
  prefs->currentscenariohold = currentscenario;
  prefs->blank3 = 0;
  prefs->journalindex2 = 0;
  prefs->blank5 = 0;
  prefs->blank6 = 0;
  prefs->blank7 = 0;
  prefs->blank8 = 0;
  prefs->blank9 = 0;
  prefs->blank10 = 0;

  showserial = serial;
  MyrBitSetLong(&showserial, 6 + divine);

  source_str = Name_String;
  byte_count = Name_String[0] + 1;
  BlockMoveData(source_str, prefs->name_str, byte_count);

  old_data_handle = Get1Resource('PRFN', PREF_RES_ID);

  GetResInfo(old_data_handle, &res_ID, &res_type, res_name);

  res_attributes = GetResAttrs(old_data_handle);
  RemoveResource(old_data_handle);

  CvtPrefsToPc(prefs);

  AddResource(new_data_handle, res_type, res_ID, res_name);
  WriteResource(new_data_handle);

  HUnlock(new_data_handle);

  ReleaseResource(new_data_handle);

  CloseResFile(pref_ref_num);

  UseResFile(oldresfile);
}

/******************* getpref ***************/
void getpref(void) {
  short pref_ref_num;
  Handle data_handle;
  PrefRecord* prefs;
  StringPtr source_str;
  Size byte_count;
  short oldresfile;

  oldresfile = CurResFile();

  pref_ref_num = openpref(0);

  UseResFile(pref_ref_num);

  data_handle = Get1Resource('PRFN', PREF_RES_ID);
  prefs = *(PrefHandle)data_handle;

  CvtPrefsToPc(prefs);

  delayspeed = prefs->delayspeed;
  oldspeed = prefs->oldspeed;
  musicvolume = prefs->oldvolume;
  volume = prefs->volume;
  defaultspell = prefs->defaultspell;
  showcast = prefs->showcast;
  usehashmarks = prefs->usehashmarks;
  numchannel = prefs->numchannel;
  lastgame = prefs->lastgame;
  horseicon = prefs->horseicon;
  dropitemprotection = prefs->dropitemprotection;
  forgettreasure = prefs->forgettreasure;
  fasttrade = prefs->fasttrade;
  autocash = prefs->autocash;
  autojoin = prefs->autojoin;
  autoid = prefs->autoid;
  usedefaultfont = prefs->usedefaultfont;
  colormenus = prefs->colormenus;
  showcaste = prefs->showcaste;
  reducesound = prefs->reducesound;
  showdescript = prefs->showdescript;
  quickshow = prefs->quickshow;
  hidedesktop = prefs->hidedesktop;
  manualbandage = prefs->manualbandage;
  showbleedmessage = prefs->showbleedmessage;
  shownextroundmessage = prefs->shownextroundmessage;
  auto256 = prefs->auto256;
  iteminfo = prefs->iteminfo;
  autoweapswitch = prefs->autoweapswitch;
  nomusic = prefs->nomusic;
  usenpc = prefs->usenpc;
  castonfriends = prefs->castonfriends;
  allowfumble = prefs->allowfumble;
  allowunique = prefs->allowunique;
  defaultfont = prefs->defaultfont;
  serial = prefs->serial;

  source_str = prefs->name_str;
  byte_count = prefs->name_str[0] + 1;
  BlockMoveData(source_str, Name_String, byte_count);

  autonote = prefs->autonote;
  portraitchoice = prefs->portraitchoice;
  currentscenariohold = prefs->currentscenariohold;
  blank3 = prefs->blank3;
  journalindex2 = prefs->journalindex2;
  blank5 = prefs->blank5;
  blank6 = prefs->blank6;
  blank7 = prefs->blank7;
  blank8 = prefs->blank8;
  blank9 = prefs->blank9;
  blank10 = prefs->blank10;

#if !divine
  if ((!serial) || (!MyrBitTstLong(&serial, 8))) {
    serial = Rand(21987); /***** generate new serial number ***/
    serial *= Rand(666);
    serial += Rand(32000);
    MyrBitSetLong(&serial, 8);
    MyrBitSetLong(&serial, 6 + divine);
    appnum = serial;

    PtoCstr((StringPtr)myString);

    if ((fp = MyrFopen((Ptr)myString, "r+b")) != NIL) {
      CvtLongToPc(&appnum);
      fwrite(&appnum, sizeof appnum, 1, fp);
      CvtLongToPc(&appnum);
      fclose(fp);
    }
  }
#endif

  showserial = serial;
  MyrBitSetLong(&showserial, 6 + divine);

  CloseResFile(pref_ref_num);
  UseResFile(oldresfile);
}

/*************************** createpref ********************/
short createpref(FSSpec pref_FSSpec, short mode) {
  short file_ref_num;
  Handle app_handle;
  short res_ID;
  ResType res_type;
  Str255 res_name;
  short res_attributes;
  short oldresfile;

  oldresfile = CurResFile();
  UseResFile(Appl_Rsrc_Fork_Ref_Num);
  app_handle = Get1Resource('PRFN', PREF_RES_ID);

  GetResInfo(app_handle, &res_ID, &res_type, res_name);
  res_attributes = GetResAttrs(app_handle);
  DetachResource(app_handle);

  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(fuzziqersoftware): The following is not part of the original
   * implementation. It appears the template PRFN resource in realmz.rsrc is
   * too small, leading to an out-of-bounds memory access when we try to use
   * it later. To deal with this, we manually extend it with zeroes it if
   * it's too small before writing it to the new preferences file. */
  size_t orig_app_handle_size = GetHandleSize(app_handle);
  if (orig_app_handle_size < sizeof(PrefRecord)) {
    SetHandleSize(app_handle, sizeof(PrefRecord));
    memset(*app_handle + orig_app_handle_size, 0, sizeof(PrefRecord) - orig_app_handle_size);
  }
  /* *** END CHANGES *** */

  if (!mode)
    FSpCreateResFile(&pref_FSSpec, 'RLMZ', 'PREF', smSystemScript);
  else
    FSpCreateResFile(&pref_FSSpec, 'MACS', 'pref', smSystemScript);

  file_ref_num = FSpOpenResFile(&pref_FSSpec, fsCurPerm);
  UseResFile(file_ref_num);

  AddResource(app_handle, res_type, res_ID, res_name);
  SetResAttrs(app_handle, res_attributes);
  ChangedResource(app_handle);
  WriteResource(app_handle);
  ReleaseResource(app_handle);

  UseResFile(oldresfile);
  return (file_ref_num);
}

/**************** openpref *********************/
short openpref(short mode) {
  Str255 pref_file_name;
  short vol_ref;
  long dir_ID;
  FSSpec pref_FSSpec;
  short file_ref_num;
  short dummyrefnum = 0;

  GetIndString(pref_file_name, STR_LIST_ID, PREF_STR_INDEX + divine);

  FindFolder(kOnSystemDisk, kPreferencesFolderType,
      kDontCreateFolder, &vol_ref, &dir_ID);

  if (!mode) {
    FSMakeFSSpec(vol_ref, dir_ID, pref_file_name, &pref_FSSpec);
    file_ref_num = FSpOpenResFile(&pref_FSSpec, fsCurPerm);
  } else if (mode == 6) /**** Get Divinity User Name. ****/
  {
    FSMakeFSSpec(vol_ref, dir_ID, "\pDivinity Prefs", &pref_FSSpec);
    file_ref_num = FSpOpenResFile(&pref_FSSpec, fsCurPerm);
    return (file_ref_num);
  }
  if (mode == 10) /**** Erase old Realmz Prefs. ****/
  {
    FSMakeFSSpec(vol_ref, dir_ID, "\pRealmz Preferences", &pref_FSSpec);
    FSpDelete(&pref_FSSpec);
    return (file_ref_num);
  }

  if ((file_ref_num == -1) && ((!mode) || (mode == 3)))
    file_ref_num = createpref(pref_FSSpec, mode);

  if (mode)
    return (dummyrefnum);

  return (file_ref_num);
}
#endif

/***************************** preference ********************************/
void preference(void) {
  DialogRef preferwindow;
  Rect buttonrect, itemRect;
  Handle itemHandle;
  short saveportraitchoice = portraitchoice;

  if ((background != NIL) && ((FrontWindow() == look) || (FrontWindow() == gWindow)) && (look != NIL))
    in();

  preferwindow = GetNewDialog(167, 0L, (WindowPtr)-1L);

refresh:

  SetPortDialogPort(preferwindow);
  ForeColor(yellowColor);
  BackPixPat(base);
  TextFont(defaultfont);
  TextFace(bold);

  needdungeonupdate = TRUE;

  MoveWindow(GetDialogWindow(preferwindow), GlobalLeft, GlobalTop, FALSE);
  ShowWindow(GetDialogWindow(preferwindow));
  ErasePortRect();
  DrawDialog(preferwindow);
  BeginUpdate(GetDialogWindow(preferwindow));
  EndUpdate(GetDialogWindow(preferwindow));
  gCurrent = preferwindow;

  GetDialogItem(preferwindow, 5, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, defaultspell);

  GetDialogItem(preferwindow, 6, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, showcast);

  GetDialogItem(preferwindow, 7, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, usehashmarks);

  GetDialogItem(preferwindow, 34, &itemType, &itemHandle, &itemRect);
  MyrCDiStr(34, (StringPtr) "");
  ploticon3(9000 + horseicon, itemRect);

  GetDialogItem(preferwindow, 9, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, dropitemprotection);

  GetDialogItem(preferwindow, 10, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, forgettreasure);

  GetDialogItem(preferwindow, 11, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, fasttrade);

  GetDialogItem(preferwindow, 12, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, autocash);

  GetDialogItem(preferwindow, 13, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, autojoin);

  GetDialogItem(preferwindow, 14, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, autoid);

  GetDialogItem(preferwindow, 15, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, usedefaultfont);

  GetDialogItem(preferwindow, 16, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, colormenus);

  GetDialogItem(preferwindow, 17, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, showcaste);

  GetDialogItem(preferwindow, 18, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, reducesound);

  GetDialogItem(preferwindow, 19, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, showdescript);

  GetDialogItem(preferwindow, 20, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, quickshow);

  GetDialogItem(preferwindow, 21, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, hidedesktop);

  GetDialogItem(preferwindow, 22, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, manualbandage);

  GetDialogItem(preferwindow, 23, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, showbleedmessage);

  GetDialogItem(preferwindow, 24, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, shownextroundmessage);

  GetDialogItem(preferwindow, 25, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, auto256);

  GetDialogItem(preferwindow, 26, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, iteminfo);

  GetDialogItem(preferwindow, 27, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, autoweapswitch);

  GetDialogItem(preferwindow, 28, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, nomusic);

  GetDialogItem(preferwindow, 29, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, usenpc);

  GetDialogItem(preferwindow, 30, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, castonfriends);

  GetDialogItem(preferwindow, 31, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, allowfumble);

  GetDialogItem(preferwindow, 32, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, allowunique);

  GetDialogItem(preferwindow, 33, &itemType, &itemHandle, &itemRect);
  SetControlValue((ControlHandle)itemHandle, autonote);

// Myriad : Some Items have no sense on the PC side
#ifdef PC
  HideDialogItem(preferwindow, 15); // use default font
  HideDialogItem(preferwindow, 54);
  HideDialogItem(preferwindow, 16); // colors menus
  HideDialogItem(preferwindow, 55); // colors menus
  HideDialogItem(preferwindow, 21); // hidedesktop
  HideDialogItem(preferwindow, 49); // hidedesktop
  HideDialogItem(preferwindow, 25); // switch 256 colors
  HideDialogItem(preferwindow, 46); // switch 256 colors
#endif
backup:

  SetPortDialogPort(preferwindow);
  ForeColor(yellowColor);

  GetDialogItem(preferwindow, 15, &itemType, &itemHandle, &itemRect);
  usedefaultfont = GetControlValue((ControlHandle)itemHandle);

  temp = defaultfont;

  if (usedefaultfont)
    defaultfont = ID_FONT_DEFAULT;
  else
    defaultfont = 3;

  TextFont(defaultfont);

  if (temp != defaultfont) {
    DrawDialog(preferwindow);
    GetDialogItem(preferwindow, 34, &itemType, &itemHandle, &itemRect);
    MyrCDiStr(34, (StringPtr) "");
    ploticon3(9000 + horseicon, itemRect);
  }

  GetDialogItem(preferwindow, 5, &itemType, &itemHandle, &itemRect);
  defaultspell = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 6, &itemType, &itemHandle, &itemRect);
  showcast = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 7, &itemType, &itemHandle, &itemRect);
  usehashmarks = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 9, &itemType, &itemHandle, &itemRect);
  dropitemprotection = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 10, &itemType, &itemHandle, &itemRect);
  forgettreasure = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 11, &itemType, &itemHandle, &itemRect);
  fasttrade = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 12, &itemType, &itemHandle, &itemRect);
  autocash = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 13, &itemType, &itemHandle, &itemRect);
  autojoin = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 14, &itemType, &itemHandle, &itemRect);
  autoid = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 16, &itemType, &itemHandle, &itemRect);
  colormenus = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 17, &itemType, &itemHandle, &itemRect);
  showcaste = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 18, &itemType, &itemHandle, &itemRect);
  reducesound = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 19, &itemType, &itemHandle, &itemRect);
  showdescript = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 20, &itemType, &itemHandle, &itemRect);
  quickshow = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 21, &itemType, &itemHandle, &itemRect);
  hidedesktop = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 22, &itemType, &itemHandle, &itemRect);
  manualbandage = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 23, &itemType, &itemHandle, &itemRect);
  showbleedmessage = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 24, &itemType, &itemHandle, &itemRect);
  shownextroundmessage = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 25, &itemType, &itemHandle, &itemRect);
  auto256 = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 26, &itemType, &itemHandle, &itemRect);
  iteminfo = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 27, &itemType, &itemHandle, &itemRect);
  autoweapswitch = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 28, &itemType, &itemHandle, &itemRect);
  nomusic = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 29, &itemType, &itemHandle, &itemRect);
  usenpc = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 31, &itemType, &itemHandle, &itemRect);
  allowfumble = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 32, &itemType, &itemHandle, &itemRect);
  allowunique = GetControlValue((ControlHandle)itemHandle);

  GetDialogItem(preferwindow, 33, &itemType, &itemHandle, &itemRect);
  autonote = GetControlValue((ControlHandle)itemHandle);

  for (;;) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(preferwindow, itemHit, &itemType, &itemHandle, &itemRect);

    if (itemHit < 3) {
      GetDialogItem(preferwindow, 2, &itemType, &itemHandle, &buttonrect);
      ploticon3(129, buttonrect);
      sound(141);

      DisposeDialog(preferwindow);
      savepref();

      if (FrontWindow() == GetDialogWindow(background))
        DrawDialog(background);
      else
        updatemain(FALSE, -1);
      goto out;
    }

#if !divine

    if (itemHit == 32) {
      if (!doreg()) /**** unique ****/
      {
        GetIndString(myString, 6001, itemHit - 4);
        MyrPascalDiStr(37, myString);
        allowunique = TRUE;
        ForeColor(blackColor);
        BackColor(whiteColor);
        warn(117);
        goto backup;
      }

      if (incombat) {
        ForeColor(blackColor);
        BackColor(whiteColor);
        warn(119);
        goto backup;
      }
    }

    if (itemHit == 31) /**** fumbling ****/
    {
      if (!doreg()) {
        GetIndString(myString, 6001, itemHit - 4);
        MyrPascalDiStr(37, myString);
        allowfumble = TRUE;
        ForeColor(blackColor);
        BackColor(whiteColor);
        warn(117);
        goto backup;
      }

      if (incombat) {
        ForeColor(blackColor);
        BackColor(whiteColor);
        warn(119);
        goto backup;
      }
    }
#endif

    if ((itemHit != 8) && (itemHit < 66)) {
      if (GetControlValue((ControlHandle)itemHandle))
        SetControlValue((ControlHandle)itemHandle, FALSE);
      else
        SetControlValue((ControlHandle)itemHandle, TRUE);

      GetIndString(myString, 6001, itemHit - 4);
      MyrPascalDiStr(37, myString);
    } else if (itemHit < 66) /************ pick new indoor icon ***************/
    {
      sound(141);
      MyrParamText((Ptr) "Select New Indoor Party Icon", (Ptr) "", (Ptr) "", (Ptr) "");
      iconpicture(2);
      goto refresh;
    }

    goto backup;
  }

out:

  if (quickshow)
    CheckItem(prefer, 7, 1);
  else
    CheckItem(prefer, 7, 0);
// Myriad no desktop on the pc side
#ifndef PC
  if (hidedesktop) {
    if (mat == NIL) {
      mat = GetNewWindow(132, 0L, screen);
      SizeWindow(mat, width, depth, FALSE);
    }
  } else {
    if (mat)
      DisposeWindow(mat);
    mat = NIL;
  }
#endif
}
