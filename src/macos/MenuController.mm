#import "../MenuController.h"
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#include <cstddef>
#include <cstdint>
#include <objc/NSObject.h>
#include <resource_file/ResourceFile.hh>

NSMenu* MCCreateMenu(const MenuList& menuList);
NSMenu* MCCreateSubMenu(NSString* title, const Menu& menuRes, const std::list<std::shared_ptr<Menu>> submenus);

@interface MCMenuItemIdentifier : NSObject

@property(readonly) int16_t menuID;
@property(readonly) int16_t itemID;

@end

@implementation MCMenuItemIdentifier

- (id)initWithRawIds:(int16_t)menu_id itemId:(int16_t)item_id {
  if (self = [super init]) {
    _menuID = menu_id;
    _itemID = item_id;
  }
  return self;
}

@end

@interface MCMenuBar : NSObject

@property(readonly) NSMenu* menuObject;
@property(nonatomic) void (*callback)(int16_t, int16_t);

@end

@implementation MCMenuBar

@synthesize callback;

- (id)initWithMenuListCallback:(const MenuList&)menuList callback:(void (*)(int16_t, int16_t))_callback {
  if (self = [super init]) {
    [self MCCreateMenu:menuList];
    callback = _callback;
  }
  return self;
}

- (IBAction)MCHandleMenuClick:(id)sender {
  id identifier = [sender representedObject];
  callback([identifier menuID], [identifier itemID]);
}

- (void)MCCreateMenu:(const MenuList&)menuList {
  _menuObject = [[NSMenu alloc] initWithTitle:@"Realmz"];
  [_menuObject setAutoenablesItems:NO];

  for (auto menu : menuList.menus) {
    NSString* title = [NSString stringWithCString:menu->title.c_str() encoding:NSMacOSRomanStringEncoding];
    NSMenuItem* menuItem = [[NSMenuItem alloc] initWithTitle:title action:NULL keyEquivalent:@""];
    menuItem.enabled = menu->enabled;
    [_menuObject addItem:menuItem];
    if (menu->items.size()) {
      NSMenu* subMenu = [self MCCreateSubMenu:title parentMenu:*menu submenus:menuList.submenus];
      [_menuObject setSubmenu:subMenu forItem:menuItem];
    }
  }
}

- (NSMenu*)MCCreateSubMenu:(NSString*)title parentMenu:(const Menu&)menu submenus:(const std::list<std::shared_ptr<Menu>>)submenus {
  NSMenu* newMenu = [[NSMenu alloc] initWithTitle:title];
  [newMenu setAutoenablesItems:NO];

  int itemId = 0;
  for (auto& subMenuItemRes : menu.items) {
    itemId++;
    if (subMenuItemRes.name == "-") {
      [newMenu addItem:[NSMenuItem separatorItem]];
    } else {
      NSString* name = [NSString stringWithCString:subMenuItemRes.name.c_str() encoding:NSMacOSRomanStringEncoding];
      if (name != nullptr) {
        char key_equiv[2] = {static_cast<char>(tolower(subMenuItemRes.key_equivalent)), '\0'};
        NSString* key = [NSString stringWithCString:key_equiv encoding:NSMacOSRomanStringEncoding];
        NSMenuItem* subMenuItem = [newMenu addItemWithTitle:name action:NULL keyEquivalent:key];
        [subMenuItem setTarget:self];
        [subMenuItem setAction:@selector(MCHandleMenuClick:)];
        id menuIdentifier = [[MCMenuItemIdentifier alloc] initWithRawIds:menu.menu_id itemId:itemId];
        [subMenuItem setRepresentedObject:menuIdentifier];
        subMenuItem.enabled = subMenuItemRes.enabled;
        if (subMenuItemRes.checked) {
          subMenuItem.state = NSControlStateValueOn;
        }

        // Submenu ids are specified by the itemMark field if the itemCmd field has
        // the value 0x1B
        // Macintosh Toolbox Essentials (3-96)
        if (subMenuItemRes.key_equivalent == 0x1B && subMenuItemRes.mark_character) {
          for (auto subMenuRes : submenus) {
            if (subMenuRes->menu_id == static_cast<uint8_t>(subMenuItemRes.mark_character)) {
              NSString* subMenuTitle = [NSString stringWithCString:subMenuRes->title.c_str() encoding:NSMacOSRomanStringEncoding];
              NSMenu* subMenu = [self MCCreateSubMenu:subMenuTitle parentMenu:*subMenuRes submenus:submenus];
              [newMenu setSubmenu:subMenu forItem:subMenuItem];

              break;
            }
          }
        }
      }
    }
  }

  return newMenu;
}

@end

@interface MCPopupMenu : NSObject

@property(readonly) NSMenu* contextualMenu;
@property(nonatomic) void (*callback)(int16_t, int16_t);

@end

@implementation MCPopupMenu

@synthesize callback;

- (id)initWithWindow:(void *)nsWindow
    menu:(std::shared_ptr<Menu>)menu
    loc:(std::pair<int16_t, int16_t>)loc
    callback:(void (*)(int16_t, int16_t))_callback {
  if (self = [super init]) {
    callback = _callback;
    [self CreateMCPopupMenu:nsWindow menu:menu loc:loc];
  }
  return self;
}

- (IBAction)MCHandlePopupMenuClick:(id)sender {
  id identifier = [sender representedObject];
  callback([identifier menuID], [identifier itemID]);
}

- (void)CreateMCPopupMenu:(void *)nsWindow
    menu:(std::shared_ptr<Menu>)menu
    loc:(std::pair<int16_t, int16_t>)p {
  _contextualMenu = [[NSMenu alloc] initWithTitle:@"Contextual Menu"];
  [_contextualMenu setAutoenablesItems:NO];

  int itemId = 0;
  for(const auto& item: menu->items) {
    itemId++;
    NSString* name = [NSString stringWithCString:item.name.c_str() encoding:NSMacOSRomanStringEncoding];
    NSMenuItem *menuItem = [[NSMenuItem alloc] initWithTitle:name action:NULL keyEquivalent:@""];
    [menuItem setTarget:self];
    [menuItem setAction:@selector(MCHandlePopupMenuClick:)];
    id menuIdentifier = [[MCMenuItemIdentifier alloc] initWithRawIds:menu->menu_id itemId:itemId];
    [menuItem setRepresentedObject:menuIdentifier];
    menuItem.enabled = true;
    [_contextualMenu addItem:menuItem];
  }

  // In the Cocoa framework, the origin is the bottom left. The point p is passed to us as (top, left).
  NSWindow* window = (NSWindow*)nsWindow;
  NSView* view = window.contentView;
  NSSize size = view.frame.size;
  NSPoint loc = NSMakePoint(p.second, size.height - p.first);

  [[NSNotificationCenter defaultCenter] addObserver:self
    selector:@selector(HandlePopupMenuClosed:)
    name:NSMenuDidEndTrackingNotification
    object:_contextualMenu];

  BOOL result = [_contextualMenu popUpMenuPositioningItem:nil atLocation:loc inView:view];
}

- (void)HandlePopupMenuClosed:(NSNotification*)notification {
  [[NSNotificationCenter defaultCenter] removeObserver:self
    name:NSMenuDidEndTrackingNotification
    object:_contextualMenu];
  callback(0, 0);
}

@end

void MCSync(std::shared_ptr<MenuList> menuList, void (*callback)(int16_t, int16_t)) {
  NSApplication* application = [NSApplication sharedApplication];

  id newMenu = [[MCMenuBar alloc] initWithMenuListCallback:*menuList callback:callback];

  application.mainMenu = [newMenu menuObject];
}

void MCCreatePopupMenu(void *nsWindow, std::shared_ptr<Menu> menu, std::pair<int16_t, int16_t> loc, void (*callback)(int16_t, int16_t)) {
  [[MCPopupMenu alloc] initWithWindow:nsWindow menu:menu loc:loc callback:callback];
}
