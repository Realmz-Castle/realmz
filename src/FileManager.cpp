#include "FileManager.h"

#include <SDL3/SDL_filesystem.h>
#include <phosg/Strings.hh>

#include "StringConvert.hpp"

static phosg::PrefixedLogger fm_log("[FileManager] ");

std::string host_filename_for_mac_filename(const std::string& mac_path, bool implicitly_local) {
  std::string ret = mac_path;

  // If the path begins with ':', it's a relative path. On modern systems,
  // paths starting with / are absolute instead, so remove the : before
  // replacing : with /
  bool explicitly_local = phosg::starts_with(ret, ":");
  if (explicitly_local) {
    ret = ret.substr(1);
  }

  if (explicitly_local || implicitly_local) {
    for (size_t z = 0; z < ret.size(); z++) {
      if (ret[z] == ':') {
        ret[z] = '/';
      }
    }

    auto base_path = SDL_GetBasePath();
    if (!base_path) {
      fm_log.error("Failed to get SDL base path: %s", SDL_GetError());
      return "";
    }

    return base_path + ret;
  } else {
    // Don't allow absolute paths
    throw std::runtime_error("absolute path not supported: " + ret);
  }
}

std::string host_filename_for_FSSpec(const FSSpec* fsp) {
  // We only support relative paths (see above) and only references to the
  // default volume and no directory
  if (fsp->vRefNum != 0) {
    throw std::runtime_error(phosg::string_printf("FSSpec vRefNum is not zero (received %hd)", fsp->vRefNum));
  }
  if ((fsp->parID != 0) && (fsp->parID != -1)) {
    throw std::runtime_error(phosg::string_printf("FSSpec parID is not 0 or -1 (received %" PRId32 ")", fsp->parID));
  }

  return host_filename_for_mac_filename(string_for_pstr<64>(fsp->name), (fsp->parID == -1));
}

OSErr GetVInfo(int16_t drvNum, StringPtr volName, int16_t* vRefNum, int32_t* freeBytes) {
  fm_log.info("Volume info requested for drive %hd", drvNum);

  // Return fake volume info
  strcpy(reinterpret_cast<char*>(volName), "Macintosh HD");
  *vRefNum = 0;
  *freeBytes = 1024 * 1024 * 1024; // 1GB
  return noErr;
}

void GetFInfo(const Str63 fName, int16_t vRefNum, FInfo* fInfo) {
  auto filename = string_for_pstr<64>(fName);
  fm_log.info("Finder info requested for file %s (on volume %hd)", filename.c_str(), vRefNum);

  // Return fake Finder info (Realmz doesn't use it anyway)
  fInfo->fdType = 0x31313131; // '1111'
  fInfo->fdCreator = 0x31313131; // '1111'
  fInfo->fdFlags = 0;
  fInfo->fdLocation.h = 0;
  fInfo->fdLocation.v = 0;
  fInfo->fdFldr = 0;
}

OSErr FSpGetFInfo(const FSSpec* spec, FInfo* fndrInfo) {
  auto filename = string_for_pstr<64>(spec->name);
  fm_log.info("Finder info requested for file %s (on volume %hd) via FSSpec", filename.c_str(), spec->vRefNum);
  GetFInfo(spec->name, spec->vRefNum, fndrInfo);
  return 0;
}

OSErr FSpSetFInfo(const FSSpec* spec, const FInfo* fndrInfo) {
  auto filename = string_for_pstr<64>(spec->name);
  fm_log.info("Skipping Finder info write for file %s (on volume %hd): type=%08" PRIX32 " creator=%08" PRIX32 " flags=%04hX loc.h=%hd loc.v=%hd folder=%hd",
      filename.c_str(),
      spec->vRefNum,
      fndrInfo->fdType,
      fndrInfo->fdCreator,
      fndrInfo->fdFlags,
      fndrInfo->fdLocation.h,
      fndrInfo->fdLocation.v,
      fndrInfo->fdFldr);
  // Ignore writes of Finder info
  return 0;
}

OSErr FSpDelete(const FSSpec* spec) {
  auto filename = string_for_pstr<64>(spec->name);
  fm_log.info("Skipping delete of file %s (on volume %hd)", filename.c_str(), spec->vRefNum);
  // TODO: We probably should have an allow-list of files that can be safely
  // deleted, instead of just ignoring all deletes.
  return 0;
}

OSErr FSMakeFSSpec(int16_t vRefNum, int32_t dirID, ConstStr255Param fileName, FSSpecPtr spec) {
  memcpy(spec->name, fileName, fileName[0] + 1);
  spec->vRefNum = vRefNum;
  spec->parID = dirID;
  return 0;
}

OSErr FindFolder(int16_t vRefNum, OSType folderType, Boolean createFolder, int16_t* foundVRefNum, int32_t* foundDirID) {
  // This is only used by Realmz for getting the Preferences folder (which in
  // Classic Mac OS is within the System Folder). Here, we just use the local
  // working directory instead, and we signal this by setting the directory ID
  // to -1.
  *foundVRefNum = 0;
  *foundDirID = -1;
  return noErr;
}

FILE* mac_fopen(const char* filename, const char* mode) {
  // It seems some codepath in pref.c calls fopen(""). This is never valid (a
  // file cannot have an empty name) so just immediately fail in that case.
  if (filename[0] == '\0') {
    return nullptr;
  }

  std::string host_filename = host_filename_for_mac_filename(filename, false);

  // Sometimes bugs may cause Realmz to try to open a directory. For example,
  // when you click on an empty slot in the party select list, it will call
  // fopen with ":Character Files:". Opening directories this way is an
  // implementation detail; the usual pattern is to call opendir + readdir
  // instead of fopen, but fopen still succeeds on some Unix-like systems. To
  // handle this, we need to check if the file is a directory and not try to
  // open it if so, which emulates the Classic Mac OS behavior.
  if (phosg::isdir(host_filename)) {
    fm_log.info("Cannot open file %s (host: %s) because it is a directory", filename, host_filename.c_str());
    return nullptr;
  }

  fm_log.info("Opening file %s (host: %s) with mode %s", filename, host_filename.c_str(), mode);
  return fopen(host_filename.c_str(), mode);
}
