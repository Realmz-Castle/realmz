#include "files.h"

static OSErr mac_GetRelativeFSSpec(const Str255 filename, FSSpec* spec);

#if !defined(CARBON)

char* dialog_open_file(const char** types, const char* prompt) {
  SFReply sf_reply;
  SFTypeList sf_types;
  Point where = {0, 0};
  char p_prompt[256];
  char* filename = NULL;

  unsigned int type_count = 0;
  unsigned int i = 0;

  for (i = 0; i < (sizeof(sf_types) / sizeof(sf_types[0])) && types[i]; i++) {
    sf_types[i] = *((OSType*)types[i]);
  }

  strncpy(p_prompt, prompt, sizeof(p_prompt));
  C2PStr(p_prompt);

  type_count = i;

  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(jpetrie): In the original implementation, p_prompt was cast to "const unsigned char*" - this isn't neccessary
   * any longer since SFGetFile() takes a signed char string. The length byte of fName has also been explicitly cast to
   * an unsigned value to ensure it is handled properly.
   */
  SFGetFile(where, p_prompt, NULL, -1, sf_types, NULL, &sf_reply);
  if (sf_reply.good) {

    filename = (char*)malloc((uint8_t)sf_reply.fName[0] + 1);
    // GetFInfo(sf_reply.fName, sf_reply.
    P2CStr(sf_reply.fName);
    strcpy(filename, (const char*)sf_reply.fName);
  }

  return filename;
}

char* dialog_save_file(const char* prompt, const char* default_name) {
  SFReply sf_reply;
  Point where = {0, 0};
  char p_prompt[256], p_default[256];
  char* filename = NULL;

  strncpy(p_prompt, prompt, sizeof(p_prompt));
  C2PStr(p_prompt);
  strncpy(p_default, default_name, sizeof(p_default));
  C2PStr(p_default);

  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(jpetrie): In the original implementation, p_prompt and p_default were cast to unsigned types - this isn't
   * neccessary any longer since SFPutFile() takes signed char strings. The length byte of fName has also been
   * explicitly cast to an unsigned value to ensure it is handled properly.
   */
  SFPutFile(where, p_prompt, p_default, NULL, &sf_reply);
  if (sf_reply.good) {
    filename = (char*)malloc((uint8_t)sf_reply.fName[0] + 1);
    // GetFInfo(sf_reply.fName, sf_reply.
    P2CStr(sf_reply.fName);
    strcpy(filename, (char*)sf_reply.fName);
  }

  return filename;
}

#else /* !CARBON */

char* dialog_open_file(const char** types, const char* prompt) {
  // notify("Open file dialog not implemented yet for Carbon");
  return NULL;
}

char* dialog_save_file(const char* prompt, const char* default_name) {
  // notify("Save file dialog not implemented yet for Carbon");
  return NULL;
}

FILE* mac_fopen(const char* filename, const char* mode) {
  char file_path[PATH_MAX];
  FILE* retval = NULL;
  OSErr err;
  FSSpec spec;
  FSRef ref;
  CFURLRef url;

  // Get it through the FSSpec (since none of the CFURL conversion routines
  // understand HFS paths correctly when using double colons) and convert to
  // an FSRef, then convert to a string... there must be a better way.
  c2pstrcpy((char*)file_path, filename);
  err = mac_GetRelativeFSSpec((const unsigned char*)file_path, &spec);

  if (err && err != fnfErr)
    return NULL;

  /*
  FSpMakeFSRef(&spec, &ref);

url = CFURLCreateFromFSRef(NULL, &ref);
  */
  CFStringRef s = CFStringCreateWithBytes(NULL,
      (uint8_t*)filename,
      strlen(filename),
      kCFStringEncodingMacRoman,
      FALSE);
  url = CFURLCreateWithFileSystemPath(NULL, s, kCFURLHFSPathStyle, FALSE);

  if (url) {
    if (CFURLGetFileSystemRepresentation(url, TRUE, file_path, sizeof(file_path))) {
      retval = fopen(file_path, mode);
    }

    CFRelease(url);
  }

  CFRelease(s);

  return retval;
}

#endif /* CARBON */

static OSErr mac_GetRelativeFSSpec(const Str255 filename, FSSpec* spec) {
  static FSSpec app_spec = {0};
  OSErr err;

  if (app_spec.name[0] == 0) {
    ProcessSerialNumber psn;

    psn.highLongOfPSN = 0;
    psn.lowLongOfPSN = kCurrentProcess;

    // If it's Carbon, we need to use another method to find
    // the bundle directory.
#ifdef CARBON
    {
      FSRef ref, parent;
      GetProcessBundleLocation(&psn, &ref);
      err = FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, &app_spec, &parent);
      // err = FSGetCatalogInfo(&parent, kFSCatInfoNone, NULL, NULL, &app_spec, NULL);
    }
#else
    {
      ProcessInfoRec proc_info;
      proc_info.processInfoLength = sizeof(proc_info);
      proc_info.processName = NULL;
      proc_info.processAppSpec = &app_spec;

      err = GetProcessInformation(&psn, &proc_info);
    }
#endif

    if (err)
      return err;
  }

  err = FSMakeFSSpec(app_spec.vRefNum, app_spec.parID, filename, spec);
  return err;
}

int mac_OpenResFile(const Str255 filename) {
  OSStatus err;
  FSSpec spec;

  err = mac_GetRelativeFSSpec(filename, &spec);

  if (err)
    return -1;

  err = FSpOpenResFile(&spec, fsRdWrPerm);
  return err;
}
