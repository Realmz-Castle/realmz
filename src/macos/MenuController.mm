#import "../MenuController.h"
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#include <cstddef>
#include <cstdint>
#include <objc/NSObject.h>
#include <phosg/Image.hh>
#include <SDL3/SDL_surface.h>

NSMenu* MCCreateMenu(const MenuList& menuList);
NSMenu* MCCreateSubMenu(NSString* title, const Menu& menuRes, const std::list<std::shared_ptr<Menu>> submenus);

// We have to forward declare because of PixMap collision w/QuickDraw
phosg::ImageRGBA8888N DecodeCIconImage(int16_t iconID);

// Wraps a decoded cicn as an NSImage, NN scale 2x, byteswap BGRA->RGBA.
static constexpr NSInteger kMenuIconUpscale = 2;

static NSImage* MCImageForCicn(int16_t cicnID) {
  if (cicnID == 0) {
    return nil;
  }
  try {
    auto decoded = DecodeCIconImage(cicnID);
    NSInteger srcW = decoded.get_width();
    NSInteger srcH = decoded.get_height();
    if (srcW <= 0 || srcH <= 0) {
      return nil;
    }
    NSInteger dstW = srcW * kMenuIconUpscale;
    NSInteger dstH = srcH * kMenuIconUpscale;
    NSBitmapImageRep* rep = [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes:NULL
                      pixelsWide:dstW
                      pixelsHigh:dstH
                   bitsPerSample:8
                 samplesPerPixel:4
                        hasAlpha:YES
                        isPlanar:NO
                  colorSpaceName:NSDeviceRGBColorSpace
                    bitmapFormat:0
                     bytesPerRow:dstW * 4
                    bitsPerPixel:32];
    if (rep == nil) {
      return nil;
    }
    const uint32_t* src = decoded.get_data();
    uint32_t* dst = reinterpret_cast<uint32_t*>([rep bitmapData]);
    for (NSInteger y = 0; y < dstH; y++) {
      const uint32_t* srcRow = src + (y / kMenuIconUpscale) * srcW;
      uint32_t* dstRow = dst + y * dstW;
      for (NSInteger x = 0; x < dstW; x++) {
        uint32_t pixel = srcRow[x / kMenuIconUpscale];
        if constexpr (!phosg::IS_BIG_ENDIAN) {
          pixel = __builtin_bswap32(pixel);
        }
        dstRow[x] = pixel;
      }
    }
    [rep setSize:NSMakeSize(srcW, srcH)];
    NSImage* image = [[NSImage alloc] initWithSize:NSMakeSize(srcW, srcH)];
    [image addRepresentation:rep];
    return image;
  } catch (const std::exception&) {
    return nil;
  }
}

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

@interface MCMenuBar : NSObject <NSMenuDelegate>

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

- (IBAction)MCHandleFilter:(id)sender {
  WM_SetScaleMode((int)[sender tag]);
}

- (IBAction)MCHandleScale:(id)sender {
  NSInteger tag = [sender tag];
  WM_SetWindowSize((int)(tag >> 16), (int)(tag & 0xFFFF));
}

- (IBAction)MCHandleAspectLock:(id)sender {
  WM_SetAspectLocked(!WM_GetAspectLocked());
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

  [self addPortMenu];
}

- (void)addPortMenu {
  NSMenuItem* portItem = [[NSMenuItem alloc] initWithTitle:@"Port" action:NULL keyEquivalent:@""];
  [_menuObject addItem:portItem];
  NSMenu* portMenu = [[NSMenu alloc] initWithTitle:@"Port"];
  [portMenu setAutoenablesItems:NO];
  portMenu.delegate = self;

  const struct {
    const char* title;
    SDL_ScaleMode mode;
  } filters[] = {
      {"Filter: Pixel Art", SDL_SCALEMODE_PIXELART},
      {"Filter: Linear", SDL_SCALEMODE_LINEAR},
      {"Filter: Nearest", SDL_SCALEMODE_NEAREST},
  };
  for (const auto& f : filters) {
    NSMenuItem* item = [portMenu addItemWithTitle:[NSString stringWithUTF8String:f.title]
                                           action:@selector(MCHandleFilter:)
                                    keyEquivalent:@""];
    [item setTarget:self];
    [item setTag:(NSInteger)f.mode];
  }

  [portMenu addItem:[NSMenuItem separatorItem]];
  NSMenuItem* scaleItem = [[NSMenuItem alloc] initWithTitle:@"Scale" action:NULL keyEquivalent:@""];
  [portMenu addItem:scaleItem];
  NSMenu* scaleMenu = [[NSMenu alloc] initWithTitle:@"Scale"];
  [scaleMenu setAutoenablesItems:NO];
  scaleMenu.delegate = self;
  const struct {
    const char* title;
    int w;
    int h;
  } scales[] = {
      {"1x", 800, 600},
      {"1.5x", 1200, 900},
      {"2x", 1600, 1200},
      {"2.5x", 2000, 1500},
      {"3x", 2400, 1800},
  };
  for (const auto& s : scales) {
    NSMenuItem* item = [scaleMenu addItemWithTitle:[NSString stringWithUTF8String:s.title]
                                            action:@selector(MCHandleScale:)
                                     keyEquivalent:@""];
    [item setTarget:self];
    [item setTag:(NSInteger)((s.w << 16) | s.h)];
  }
  [portMenu setSubmenu:scaleMenu forItem:scaleItem];

  [portMenu addItem:[NSMenuItem separatorItem]];
  NSMenuItem* aspectItem = [portMenu addItemWithTitle:@"Lock Aspect Ratio"
                                               action:@selector(MCHandleAspectLock:)
                                        keyEquivalent:@""];
  [aspectItem setTarget:self];

  [portMenu addItem:[NSMenuItem separatorItem]];
  // TODO: Potential color correction.
  NSMenuItem* gammaItem = [portMenu addItemWithTitle:@"Color Correction"
                                              action:nil
                                       keyEquivalent:@""];
  gammaItem.enabled = NO;

  [_menuObject setSubmenu:portMenu forItem:portItem];
}

- (void)menuNeedsUpdate:(NSMenu*)menu {
  int currentMode = WM_GetScaleMode();
  BOOL fullscreen = WM_IsFullscreen() ? YES : NO;
  int curW = 0, curH = 0;
  WM_GetWindowSize(&curW, &curH);
  for (NSMenuItem* item in menu.itemArray) {
    if (item.action == @selector(MCHandleFilter:)) {
      item.state = ((int)item.tag == currentMode) ? NSControlStateValueOn : NSControlStateValueOff;
    } else if (item.action == @selector(MCHandleScale:)) {
      int w = (int)(item.tag >> 16);
      int h = (int)(item.tag & 0xFFFF);
      item.enabled = (!fullscreen && WM_SizeFits(w, h)) ? YES : NO;
      item.state = (!fullscreen && curW == w && curH == h) ? NSControlStateValueOn : NSControlStateValueOff;
    } else if (item.action == @selector(MCHandleAspectLock:)) {
      item.enabled = !fullscreen;
      item.state = WM_GetAspectLocked() ? NSControlStateValueOn : NSControlStateValueOff;
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
        bool mark_is_submenu_link = (subMenuItemRes.key_equivalent == 0x1B);
        char mark = mark_is_submenu_link ? 0 : subMenuItemRes.mark_character;
        if (subMenuItemRes.checked || mark == 19) {
          subMenuItem.state = NSControlStateValueOn;
        } else if (mark != 0) {
          subMenuItem.state = NSControlStateValueMixed;
        }
        NSImage* icon = MCImageForCicn(subMenuItemRes.icon_id);
        if (icon != nil) {
          [subMenuItem setImage:icon];
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
    menuItem.enabled = item.enabled;
    NSImage* icon = MCImageForCicn(item.icon_id);
    if (icon != nil) {
      [menuItem setImage:icon];
    }
    [_contextualMenu addItem:menuItem];
  }

  // In the Cocoa framework, the origin is the bottom left. The point p is passed to us as (top, left).
  NSWindow* window = (__bridge NSWindow*)nsWindow;
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

  static MCMenuBar* currentMenuBar = nil;
  currentMenuBar = [[MCMenuBar alloc] initWithMenuListCallback:*menuList callback:callback];

  application.mainMenu = [currentMenuBar menuObject];
}

void MCCreatePopupMenu(void *nsWindow, std::shared_ptr<Menu> menu, std::pair<int16_t, int16_t> loc, void (*callback)(int16_t, int16_t)) {
  [[MCPopupMenu alloc] initWithWindow:nsWindow menu:menu loc:loc callback:callback];
}
