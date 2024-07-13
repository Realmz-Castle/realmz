#ifndef FILES_H
#define FILES_H

#if defined(REALMZ_COCOA)
#include "RealmzCocoa.h"
#endif

char* dialog_open_file(const char** types, const char* prompt);

char* dialog_save_file(const char* prompt, const char* default_name);

int mac_OpenResFile(const Str255 filename);
FILE* mac_fopen(const char* filename, const char* mode);

#endif /* FILES_H */
