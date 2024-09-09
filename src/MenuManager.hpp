#pragma once

#include <list>
#include <resource_file/ResourceFile.hh>

struct MenuList {
  std::list<std::shared_ptr<const ResourceDASM::ResourceFile::DecodedMenu>> menus;
  std::list<std::shared_ptr<const ResourceDASM::ResourceFile::DecodedMenu>> submenus;
};