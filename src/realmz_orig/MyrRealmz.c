/***********************************************************************
                                        MyrRealmz.c
Functions for the MAC and the Windows Side
Myriad 1999 for Fantasoft
*************************************************************************/
#include "prototypes.h"
#include "stdarg.h"
#include "variables.h"

#include "files.h"
/* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
 * NOTE(danapplegate): Realmz calls MyrParamText MyrDrawCString, with string
 * literals of various lengths, then copies 255 bytes from that location, causing invalid program
 * memory access. To be safe, we limit the number of bytes copied to the length of the string.
 */

/***********************************************************************
                                        MyrCopyScreen
Copy a part of a port to another port
*************************************************************************/
void MyrCopyScreen(CGrafPtr portSource, CGrafPtr portDest, Rect* rectSource, Rect* rectDest, short mode) {

// if(mode==transparent)
//   AcamErreur("Transparent");
#ifdef PC
  if (qd.thePort != portDest) {
    rgnClip = NewRgn();
    rgnVis = NewRgn();
    CopyRgn(portDest->clipRgn, rgnClip);
    CopyRgn(portDest->visRgn, rgnVis);
    SetRectRgn(portDest->clipRgn, -32767, -32767, 32767, 32767);
    RectRgn(portDest->visRgn, &portDest->portBits.bounds);
    // SetRectRgn(portDest->visRgn,-32767,-32767,32767,32767);
    AcamGetPortOffscreen(&gbak);
    AcamSetPortOffscreen(portDest);
    CopyBits(&((GrafPtr)portSource)->portBits, &((GrafPtr)portDest)->portBits, rectSource, rectDest, mode, NIL);
    CopyRgn(rgnClip, portDest->clipRgn);
    CopyRgn(rgnVis, portDest->visRgn);
    DisposeRgn(rgnClip);
    DisposeRgn(rgnVis);
    AcamSetPortOffscreen(gbak);
  } else
#endif
    BitMap* src = GetPortBitMapForCopyBits(portSource);
  BitMap* dst = GetPortBitMapForCopyBits(portDest);
  CopyBits(src, dst, rectSource, rectDest, mode, NIL);
}

/***********************************************************************
                                        MyrCopyMask
Copy a part of a port to another port
*************************************************************************/
void MyrCopyMask(CGrafPtr portSource, CGrafPtr masque, CGrafPtr portDest, Rect* rectSource, Rect* rectMask, Rect* rectDest) {
#ifdef PC
  if (qd.thePort != portDest) {
    rgnClip = NewRgn();
    rgnVis = NewRgn();
    CopyRgn(portDest->clipRgn, rgnClip);
    CopyRgn(portDest->visRgn, rgnVis);
    SetRectRgn(portDest->clipRgn, -32767, -32767, 32767, 32767);
    SetRectRgn(portDest->visRgn, -32767, -32767, 32767, 32767);
    AcamGetPortOffscreen(&gbak);
    AcamSetPortOffscreen(portDest);
    CopyMask(&((GrafPtr)portSource)->portBits, &((GrafPtr)masque)->portBits, &((GrafPtr)portDest)->portBits, rectSource, rectMask, rectDest);
    CopyRgn(rgnClip, portDest->clipRgn);
    CopyRgn(rgnVis, portDest->visRgn);
    DisposeRgn(rgnClip);
    DisposeRgn(rgnVis);
    AcamSetPortOffscreen(gbak);
  } else
#endif
    BitMap* src = GetPortBitMapForCopyBits(portSource);
  BitMap* dst = GetPortBitMapForCopyBits(portDest);
  BitMap* msk = GetPortBitMapForCopyBits(masque);
  CopyMask(src, msk, dst, rectSource, rectMask, rectDest);
}

/***********************************************************************
                                        MyrCopyIconMask
Copy the icon mask to an offscreen
*************************************************************************/
void MyrCopyIconMask(CIconHandle icon, CGrafPtr portDest, Rect* rectDest) {

#ifdef PC
  if (qd.thePort != portDest) {
    rgnClip = NewRgn();
    rgnVis = NewRgn();
    CopyRgn(portDest->clipRgn, rgnClip);
    CopyRgn(portDest->visRgn, rgnVis);
    SetRectRgn(portDest->clipRgn, -32767, -32767, 32767, 32767);
    SetRectRgn(portDest->visRgn, -32767, -32767, 32767, 32767);
    AcamGetPortOffscreen(&gbak);
    AcamSetPortOffscreen(portDest);
    CopyBits(&(**(icon)).iconBMap, &((GrafPtr)portDest)->portBits, &(**(iconhand)).iconPMap.bounds, rectDest, 0, 0L);
    CopyRgn(rgnClip, portDest->clipRgn);
    CopyRgn(rgnVis, portDest->visRgn);
    DisposeRgn(rgnClip);
    DisposeRgn(rgnVis);
    AcamSetPortOffscreen(gbak);
  } else
#endif
    BitMap* dst = GetPortBitMapForCopyBits(portDest);
  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(fuzziqersoftware): The original code uses CopyBits here, but we only
   * support CopyBits between two CCGrafPorts, not with arbitrary BitMaps or
   * PixMaps. This appears to be the only place where Realmz calls CopyBits
   * with a BitMap, so we replace it with a similar call that does the thing
   * the original code intends.
   */
  // CopyBits(&(**(icon)).iconBMap, dst, &(**(iconhand)).iconPMap.bounds, rectDest, 0, 0L);
  GrafPtr orig_port;
  GetPort(&orig_port);
  SetPort(portDest);
  PlotCIconBitmap(rectDest, icon);
  SetPort(orig_port);
  /* *** END CHANGES *** */
}

/***********************************************************************
                                        MyrStrPtoC
Convert a Pascal string to PC
(Do Nothing on Macintosh Side)
*************************************************************************/
void MyrStrPtoC(char* str) {
#ifdef PC
  short taille;

  if (str && *str) {
    taille = *(unsigned char*)str;
    BlockMove(str + 1, str, taille);
    str[taille] = 0;
  }
#endif
}

/***********************************************************************
                                        MyrStrCtoP
Convert a C string to pascal
(Do Nothing on Macintosh Side)
*************************************************************************/
void MyrStrCtoP(char* str) {
#ifdef PC
  short taille;

  if (str && *str) {
    taille = strlen(str);
    BlockMove(str, str + 1, taille);
    str[0] = taille;
  }
#endif
}

/***********************************************************************
                                        MyrParamText
Do a param text with C strings
=> up to four C string
*************************************************************************/
void MyrParamText(Ptr p1, Ptr p2, Ptr p3, Ptr p4) {
  char c1[256];
  char c2[256];
  char c3[256];
  char c4[256];

  strcpy(c1, (StringPtr) "");
  strcpy(c2, (StringPtr) "");
  strcpy(c3, (StringPtr) "");
  strcpy(c4, (StringPtr) "");
  if (p1)
    BlockMove(p1, c1, strlen((const char*)p1) + 1);
  if (p2)
    BlockMove(p2, c2, strlen((const char*)p2) + 1);
  if (p3)
    BlockMove(p3, c3, strlen((const char*)p3) + 1);
  if (p4)
    BlockMove(p4, c4, strlen((const char*)p4) + 1);
  CtoPstr(c1);
  CtoPstr(c2);
  CtoPstr(c3);
  CtoPstr(c4);

  ParamText((StringPtr)c1, (StringPtr)c2, (StringPtr)c3, (StringPtr)c4);
  return;
}

/***********************************************************************
                                        MyrAppendMenu
Add a C string to a menu
=> menu handle, C string
*************************************************************************/
void MyrAppendMenu(MenuHandle menu, Ptr str) {
  char chaine[256];

  strcpy(chaine, str);
  CtoPstr(chaine);
  AppendMenu(menu, (StringPtr)chaine);
  return;
}

/******************************************************************
                    MyrFopen
Ouvre une fichier C en convertissant le path
******************************************************************/
FILE* MyrFopen(char* nomMac, char* type) {
  FILE* f;
#ifdef PC
  char nom[255];
  // char chaine[255];

  f = fopen(CvtPathMacToPc(nomMac, nom), type);
  if (f == NULL) {
    // char path[512];
    // SaisitChemin(NULL,path,NULL);
    // sprintf(chaine,(StringPtr)"Myriad fopen : '%s%s' not found %d",path,nom,errno);
    // AcamErreur(chaine);
  }
#else

  f = mac_fopen(nomMac, type);

#endif
  return (f);
}

/******************************************************************
                    MyrOpenResFile
******************************************************************/
short MyrOpenResFile(char* nomMac) {
  short id;
  FSSpec fsp;
  FSMakeFSSpec(0, 0, nomMac, &fsp);
#ifdef PC
  char nom[255];
  char chaine[255];

  CvtPathMacToPc(nomMac, nom);
  // strcat(nom,(StringPtr)".rsr");
  id = OpenResFile(nom);
  if (id < 0) {
    sprintf(chaine, (StringPtr) "Myriad OpenResFile : %s not found", nom);
    // AcamErreur(chaine);
  }
#else
  // id=FSpOpenResFile(&fsp, fsRdWrShPerm);
  id = mac_OpenResFile(nomMac);
#endif
  return (id);
}

/******************************************************************
                    MyrRemove
******************************************************************/
short MyrRemove(char* nomMac) {
#ifdef PC
  char nom[255];

  CvtPathMacToPc(nomMac, nom);
  return (remove(nom));
#else
  return (remove(nomMac));
#endif
}

/*****************************************************************
                                MyrDrawCString
Draw a C String
Myriad 6-7-99
***********************************************/
void MyrDrawCString(Ptr strc) {
  char strp[256];

  BlockMove(strc, strp, strlen((const char*)strc) + 1);
  CtoPstr(strp);
  DrawString((StringPtr)strp);
  return;
}

/****************************** MyrPascalDiStr ********************************/
// Myriad :
// PC side : we receive a C string
// Mac Side : we receive a Pascal String
void MyrPascalDiStr(short theItem, Str255 thestring) {
  GetDialogItem(gCurrent, theItem, &itemType, &itemHandle, &itemRect);
  SetDialogItemText(itemHandle, thestring);
  return;
}

/****************************** MyrCDiStr ********************************/
// Myriad :
// PC side : we receive a C string
// Mac Side : we receive a C String
void MyrCDiStr(short theItem, Str255 thestring) {
  char str[256];

  strcpy(str, thestring);
  CtoPstr(str);
  GetDialogItem(gCurrent, theItem, &itemType, &itemHandle, &itemRect);
  SetDialogItemText(itemHandle, (StringPtr)str);
  return;
}

/****************************** MyrNumToString ********************************/
// Myriad : Convert a long value to a Cstring
void MyrNumToString(long value, Str255 thestring) {
  NumToString(value, thestring);
  PtoCstr(thestring);
  return;
}

/****************************** MyDump ********************************/
// Myriad : Dump values to file output
void MyrDump(char* format, ...) {
#if DUMP > 0
  va_list argptr;
  char chaine[512];
  FILE* f;

  va_start(argptr, format);
  vsprintf(chaine, format, argptr);

  MyrCheckMemory(2);
  f = fopen("Dump Realmz.txt", (StringPtr) "a+");
  fprintf(f, chaine);
  fclose(f);
#endif
  return;
}

/*EOF*/
