#include "prototypes.h"
#include "variables.h"

/****************************** loadland ****************************/
void loadland(long id, short mode) {
  short i;

  FILE* fp = NULL;

  if (indung) {
    if ((fp = MyrFopen(":Data Files:CD", "rb")) == NULL)
      scratch(106);
    fseek(fp, id * (sizeof field + sizeof door + sizeof randlevel), SEEK_SET);
  } else {
    if ((fp = MyrFopen(":Data Files:CL", "rb")) == NULL)
      scratch(107);
    fseek(fp, id * (sizeof field + sizeof door + sizeof randlevel + sizeof site), SEEK_SET);
  }

  fread(door, sizeof(door), 1, fp);
  CvtTabDoorToPc(door, 100);

  fread(&field, sizeof field, 1, fp);
  CvtFieldToPc(&field);
  fread(&randlevel, sizeof randlevel, 1, fp);
  CvtRandLevelToPc(&randlevel);
  if (!indung) {
    fread(&site, sizeof site, 1, fp);
  }
  fclose(fp);
  if ((mode) && (!indung))
    loadpixmap(randlevel.landlook);
}

/****************************** loadpixmap ****************************/
void loadpixmap(short id) {
  PicHandle picture;
  FILE* fp = NULL;
  Boolean usecustom = FALSE;
  Rect aire; // Myriad

  if (lastpix == id)
    return; /***** only load in new tile set if it is different than the last loaded ***/

retry:

  if ((id >= 6) && (id <= 8)) {
    getfilename(""); /******** loads in prefix to file path for custom land files ***/
    usecustom = TRUE;
  } else
    strcpy((StringPtr)filename, ":Data Files:"); /**** use standard graphics file ***/

  switch (id) {
    case 0:
      strcat((StringPtr)filename, "Data P BD");
      break;

    case 1:
      flashmessage((StringPtr) "Outdated files or scenario. Please udpate.", 50, 50, 0, 5000);
      break;

    case 2:
      flashmessage((StringPtr) "Outdated files or scenario. Please udpate.", 50, 50, 0, 5000);
      break;

    case 3:
      strcat((StringPtr)filename, "Data SUB BD");
      break;

    case 4:
      strcat((StringPtr)filename, "Data Castle BD");
      break;

    case 5:
      strcat((StringPtr)filename, "Data Desert BD");
      break;

    case 6:
      strcat((StringPtr)filename, "Data Custom 1");
      break;

    case 7:
      strcat((StringPtr)filename, "Data Custom 2");
      break;

    case 8:
      strcat((StringPtr)filename, "Data Custom 3");
      break;

    case 9:
      strcat((StringPtr)filename, "Data Swamp BD");
      break;

    case 10:
      strcat((StringPtr)filename, "Data Snow BD");
      break;
  }

  picture = NIL;
  picture = GetPicture(300 + id); /****** load tile set graphics into offscreen GWorld ***/

  if (picture != NIL) /*** check to see if there is a corresponding pict for the custom tile set ****/
  {
    SetRect(&itemRect, 0, 0, 640, 320);
    GetGWorld(&savedPort, &savedDevice); /********* get current drawing port info ****/

    SetGWorld(gthePixels, NIL);
    aire = ((**picture).picFrame);
    /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
     * NOTE(fuzziqersoftware): GetPicture decodes the PICT internally and
     * writes a header with picFrame in native byte order, so we don't need to
     * byteswap the rect here using rintel2moto.
     */
    // rintel2moto(&aire);
    /* *** END CHANGES *** */
    DrawPicture(picture, &aire);
    SetGWorld(savedPort, savedDevice); /********* reset drawing port to what it was before ****/
  } else {
    flashmessage((StringPtr) "Could not locate required map data.", 50, 50, 0, 5000);
    flashmessage((StringPtr) "You need to update your copy of Realmz or this scenario.", 50, 50, 0, 5000);
    scratch(412);
  }

  if (usecustom)
    strcat((StringPtr)filename, " BD");

  if ((fp = MyrFopen(filename, "rb")) == NIL) {
    flashmessage((StringPtr) "Could not locate required map data.", 50, 50, 0, 5000);
    flashmessage((StringPtr) "You need to update your copy of Realmz or this scenario.", 50, 50, 0, 5000);
    scratch(412);
  } else {
    fread(&mapstats, (sizeof mapstats) / 2, 1, fp); /****** load in combat tile build data ***/
    CvtTabMapStatToPc(&mapstats, 402 / 2);
    fread(&basetile[id], sizeof basetile[id], 1, fp);
    CvtShortToPc(&basetile[id]);
    fread(&basescale[id], sizeof basescale[id], 1, fp);
    CvtShortToPc(&basescale[id]);
    fclose(fp);
  }

  lastpix = id;
}
