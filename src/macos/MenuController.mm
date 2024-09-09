#import "MenuController.h"
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#include <cstddef>
#include <objc/NSObject.h>
#include <resource_file/ResourceFile.hh>

NSMenu* MCCreateMenu(const MenuList& menuList);
NSMenu* MCCreateSubMenu(NSString* title, const ResourceDASM::ResourceFile::DecodedMenu& menuRes);

void MCSync(std::shared_ptr<MenuList> menuList) {
  NSApplication* application = [NSApplication sharedApplication];

  id newMenu = MCCreateMenu(*menuList);

  application.mainMenu = newMenu;
}

NSMenu* MCCreateMenu(const MenuList& menuList) {
  NSMenu* newMenu = [[NSMenu alloc] initWithTitle:@"Realmz"];
  [newMenu setAutoenablesItems:NO];

  for (auto menuRes : menuList.menus) {
    NSString* title = [NSString stringWithUTF8String:menuRes->title.c_str()];
    NSMenuItem* menuItem = [[NSMenuItem alloc] initWithTitle:title action:NULL keyEquivalent:@""];
    menuItem.enabled = menuRes->enabled;
    [newMenu addItem:menuItem];
    if (menuRes->items.size()) {
      NSMenu* subMenu = MCCreateSubMenu(title, *menuRes);
      [newMenu setSubmenu:subMenu forItem:menuItem];
    }
  }

  return newMenu;
}

NSMenu* MCCreateSubMenu(NSString* title, const ResourceDASM::ResourceFile::DecodedMenu& menuRes) {
  NSMenu* subMenu = [[NSMenu alloc] initWithTitle:title];
  [subMenu setAutoenablesItems:NO];

  for (auto& subMenuItemRes : menuRes.items) {
    if (subMenuItemRes.name == "-") {
      [subMenu addItem:[NSMenuItem separatorItem]];
    } else {
      NSString* name = [NSString stringWithCString:subMenuItemRes.name.c_str() encoding:NSMacOSRomanStringEncoding];
      if (name != nullptr) {
        auto s = std::string(&subMenuItemRes.key_equivalent, 1);
        NSString* key = [NSString stringWithUTF8String:s.c_str()];
        NSMenuItem* subMenuItem = [subMenu addItemWithTitle:name action:NULL keyEquivalent:@""];
        subMenuItem.enabled = subMenuItemRes.enabled;
        [subMenuItem setEnabled:subMenuItemRes.enabled];
      }
    }
  }

  return subMenu;
}