#ifndef FileManager_hpp
#define FileManager_hpp

/**
 * This file contains extra function definitions that are only usable in C++.
 * This is primarily for use by the other managers; e.g. the Resource Manager
 * needs to be able to convert Classic Mac OS paths to host paths.
 *
 * Similarly to how Types.h and Types.hpp work, C++ code that needs to use the
 * File Manager should include this file, and C code should include
 * FileManager.h instead.
 */

#include "FileManager.h"

#include <string>

std::string host_filename_for_mac_filename(const std::string& mac_path, bool implicitly_local);
std::string host_filename_for_FSSpec(const FSSpec* fsp);

#endif // FileManager_hpp
