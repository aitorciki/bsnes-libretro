#if defined(Hiro_Window)

@implementation CocoaWindow : NSWindow

-(id) initWith:(hiro::mWindow&)windowReference {
  window = &windowReference;

  NSUInteger style = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
  if(window->state.resizable) style |= NSResizableWindowMask;

  if(self = [super initWithContentRect:NSMakeRect(0, 0, 640, 480) styleMask:style backing:NSBackingStoreBuffered defer:YES]) {
    [self setDelegate:self];
    [self setReleasedWhenClosed:NO];
    [self setAcceptsMouseMovedEvents:YES];
    [self setTitle:@""];

    NSBundle* bundle = [NSBundle mainBundle];
    NSDictionary* dictionary = [bundle infoDictionary];
    NSString* applicationName = [dictionary objectForKey:@"CFBundleDisplayName"];
    if(applicationName == nil) applicationName = [NSString stringWithUTF8String:hiro::Application::state.name];

    menuBar = [[NSMenu alloc] init];

    NSMenuItem* item;
    string text;

    rootMenu = [[NSMenu alloc] init];
    item = [[[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""] autorelease];
    [item setSubmenu:rootMenu];
    [menuBar addItem:item];

    item = [[[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:@"About %@ ...", applicationName] action:@selector(menuAbout) keyEquivalent:@""] autorelease];
    [item setTarget:self];
    [rootMenu addItem:item];
    [rootMenu addItem:[NSMenuItem separatorItem]];

    item = [[[NSMenuItem alloc] initWithTitle:@"Preferences" action:@selector(menuPreferences) keyEquivalent:@""] autorelease];
    [item setTarget:self];
    [rootMenu addItem:item];
    [rootMenu addItem:[NSMenuItem separatorItem]];

    NSMenu* servicesMenu = [[[NSMenu alloc] initWithTitle:@"Services"] autorelease];
    item = [[[NSMenuItem alloc] initWithTitle:@"Services" action:nil keyEquivalent:@""] autorelease];
    [item setTarget:self];
    [item setSubmenu:servicesMenu];
    [rootMenu addItem:item];
    [rootMenu addItem:[NSMenuItem separatorItem]];
    [NSApp setServicesMenu:servicesMenu];

    item = [[[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:@"Hide %@", applicationName] action:@selector(hide:) keyEquivalent:@""] autorelease];
    [item setTarget:NSApp];
    [rootMenu addItem:item];

    item = [[[NSMenuItem alloc] initWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@""] autorelease];
    [item setTarget:NSApp];
    [rootMenu addItem:item];

    item = [[[NSMenuItem alloc] initWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""] autorelease];
    [item setTarget:NSApp];
    [rootMenu addItem:item];
    [rootMenu addItem:[NSMenuItem separatorItem]];

    item = [[[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:@"Quit %@", applicationName] action:@selector(menuQuit) keyEquivalent:@""] autorelease];
    [item setTarget:self];
    [rootMenu addItem:item];

    statusBar = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
    [statusBar setAlignment:NSLeftTextAlignment];
    [statusBar setBordered:YES];
    [statusBar setBezeled:YES];
    [statusBar setBezelStyle:NSTextFieldSquareBezel];
    [statusBar setEditable:NO];
    [statusBar setHidden:YES];

    [[self contentView] addSubview:statusBar positioned:NSWindowBelow relativeTo:nil];
  }

  return self;
}

-(BOOL) canBecomeKeyWindow {
  return YES;
}

-(BOOL) canBecomeMainWindow {
  return YES;
}

-(void) windowDidBecomeMain:(NSNotification*)notification {
  if(window->state.menuBar) {
    [NSApp setMainMenu:menuBar];
  }
}

-(void) windowDidMove:(NSNotification*)notification {
  if(auto p = window->self()) p->moveEvent();
}

-(void) windowDidResize:(NSNotification*)notification {
  if(auto p = window->self()) p->sizeEvent();
}

-(BOOL) windowShouldClose:(id)sender {
  if(window->state.onClose) window->doClose();
  else window->setVisible(false);
  if(window->state.modal && !window->visible()) window->setModal(false);
  return NO;
}

-(NSDragOperation) draggingEntered:(id<NSDraggingInfo>)sender {
  return DropPathsOperation(sender);
}

-(BOOL) performDragOperation:(id<NSDraggingInfo>)sender {
  lstring paths = DropPaths(sender);
  if(paths.empty()) return NO;
  window->doDrop(paths);
  return YES;
}

-(NSMenu*) menuBar {
  return menuBar;
}

-(void) menuAbout {
  hiro::Application::Cocoa::doAbout();
}

-(void) menuPreferences {
  hiro::Application::Cocoa::doPreferences();
}

-(void) menuQuit {
  hiro::Application::Cocoa::doQuit();
}

-(NSTextField*) statusBar {
  return statusBar;
}

@end

namespace hiro {

auto pWindow::none() -> Window& {
  static Window* window = nullptr;
  if(window == nullptr) window = new Window;
  return *window;
}

auto pWindow::construct() -> void {
  @autoreleasepool {
    cocoaWindow = [[CocoaWindow alloc] initWith:self()];
  }
}

auto pWindow::destruct() -> void {
  @autoreleasepool {
    [cocoaWindow release];
  }
}

auto pWindow::append(sLayout layout) -> void {
  layout->setGeometry(self().geometry().setPosition(0, 0));
  statusBarReposition();
}

auto pWindow::append(sMenuBar menuBar) -> void {
  @autoreleasepool {
//    [[cocoaWindow menuBar] addItem:menu.p.cocoaAction];
  }
}

auto pWindow::append(sStatusBar statusBar) -> void {
}

auto pWindow::focused() const -> bool {
  @autoreleasepool {
    return [cocoaWindow isMainWindow] == YES;
  }
}

auto pWindow::frameMargin() const -> Geometry {
  @autoreleasepool {
    NSRect frame = [cocoaWindow frameRectForContentRect:NSMakeRect(0, 0, 640, 480)];
    return {abs(frame.origin.x), (int)(frame.size.height - 480), (int)(frame.size.width - 640), abs(frame.origin.y)};
  }
}

/*
auto pWindow::geometry() -> Geometry {
  @autoreleasepool {
    NSRect area = [cocoaWindow contentRectForFrameRect:[cocoaWindow frame]];
    area.size.height -= statusBarHeight();
    return {area.origin.x, Desktop::size().height - area.origin.y - area.size.height, area.size.width, area.size.height};
  }
}
*/

auto pWindow::remove(sLayout layout) -> void {
  @autoreleasepool {
    [[cocoaWindow contentView] setNeedsDisplay:YES];
  }
}

auto pWindow::remove(sMenuBar menuBar) -> void {
  @autoreleasepool {
//    [[cocoaWindow menuBar] removeItem:menu.p.cocoaAction];
  }
}

auto pWindow::remove(sStatusBar statusBar) -> void {
}

/*
auto pWindow::remove(Widget& widget) -> void {
  @autoreleasepool {
    [widget.p.cocoaView removeFromSuperview];
    [[cocoaWindow contentView] setNeedsDisplay:YES];
  }
}
*/

auto pWindow::setBackgroundColor(Color color) -> void {
  @autoreleasepool {
    [cocoaWindow
      setBackgroundColor:[NSColor
        colorWithCalibratedRed:color.red() / 255.0
        green:color.green() / 255.0
        blue:color.blue() / 255.0
        alpha:color.alpha() / 255.0
      ]
    ];
  }
}

auto pWindow::setDroppable(bool droppable) -> void {
  @autoreleasepool {
    if(droppable) {
      [cocoaWindow registerForDraggedTypes:[NSArray arrayWithObject:NSFilenamesPboardType]];
    } else {
      [cocoaWindow unregisterDraggedTypes];
    }
  }
}

auto pWindow::setFocused() -> void {
  @autoreleasepool {
    [cocoaWindow makeKeyAndOrderFront:nil];
  }
}

auto pWindow::setFullScreen(bool fullScreen) -> void {
  @autoreleasepool {
    if(fullScreen == true) {
      [NSApp setPresentationOptions:NSApplicationPresentationFullScreen];
      [cocoaWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
      [cocoaWindow toggleFullScreen:nil];
    } else {
      [cocoaWindow toggleFullScreen:nil];
      [cocoaWindow setCollectionBehavior:NSWindowCollectionBehaviorDefault];
      [NSApp setPresentationOptions:NSApplicationPresentationDefault];
    }
  }
}

auto pWindow::setGeometry(Geometry geometry) -> void {
  lock();

  @autoreleasepool {
    [cocoaWindow
      setFrame:[cocoaWindow
        frameRectForContentRect:NSMakeRect(
          geometry.x(), Desktop::size().height() - geometry.y() - geometry.height(),
          geometry.width(), geometry.height() + statusBarHeight()
        )
      ]
      display:YES
    ];

    if(auto& layout = state().layout) {
      layout->setGeometry(self().geometry().setPosition(0, 0));
    }

    statusBarReposition();
  }

  unlock();
}

auto pWindow::setModal(bool modal) -> void {
  @autoreleasepool {
    if(modal == true) {
      [NSApp runModalForWindow:cocoaWindow];
    } else {
      [NSApp stopModal];
      NSEvent* event = [NSEvent otherEventWithType:NSApplicationDefined location:NSMakePoint(0, 0) modifierFlags:0 timestamp:0.0 windowNumber:0 context:nil subtype:0 data1:0 data2:0];
      [NSApp postEvent:event atStart:true];
    }
  }
}

auto pWindow::setResizable(bool resizable) -> void {
  @autoreleasepool {
    NSUInteger style = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
    if(resizable) style |= NSResizableWindowMask;
    [cocoaWindow setStyleMask:style];
  }
}

/*
auto pWindow::setStatusFont(string font) -> void {
  @autoreleasepool {
    [[cocoaWindow statusBar] setFont:pFont::cocoaFont(font)];
  }
  statusBarReposition();
}

auto pWindow::setStatusText(string text) -> void {
  @autoreleasepool {
    [[cocoaWindow statusBar] setStringValue:[NSString stringWithUTF8String:text]];
  }
}

auto pWindow::setStatusVisible(bool visible) -> void {
  @autoreleasepool {
    [[cocoaWindow statusBar] setHidden:!visible];
    setGeometry(geometry());
  }
}
*/

auto pWindow::setTitle(const string& text) -> void {
  @autoreleasepool {
    [cocoaWindow setTitle:[NSString stringWithUTF8String:text]];
  }
}

auto pWindow::setVisible(bool visible) -> void {
  @autoreleasepool {
    if(visible) [cocoaWindow makeKeyAndOrderFront:nil];
    else [cocoaWindow orderOut:nil];
  }
}

auto pWindow::moveEvent() -> void {
  if(!locked() && !self().fullScreen() && self().visible()) {
    @autoreleasepool {
      NSRect area = [cocoaWindow contentRectForFrameRect:[cocoaWindow frame]];
      area.size.height -= statusBarHeight();
      state().geometry.setX(area.origin.x);
      state().geometry.setY(Desktop::size().height() - area.origin.y - area.size.height);
    }
  }

  if(!locked()) self().doMove();
}

auto pWindow::sizeEvent() -> void {
  if(!locked() && !self().fullScreen() && self().visible()) {
    @autoreleasepool {
      NSRect area = [cocoaWindow contentRectForFrameRect:[cocoaWindow frame]];
      area.size.height -= statusBarHeight();
      state().geometry.setWidth(area.size.width);
      state().geometry.setHeight(area.size.height);
    }
  }

  if(auto& layout = state().layout) {
    layout->setGeometry(self().geometry().setPosition(0, 0));
  }

  statusBarReposition();

  if(!locked()) self().doSize();
}

auto pWindow::statusBarHeight() -> uint {
  if(!state().statusBar) return 0;
//return Font::size(window.state.statusFont, " ").height + 6;
  return 0;
}

auto pWindow::statusBarReposition() -> void {
  @autoreleasepool {
    NSRect area = [cocoaWindow contentRectForFrameRect:[cocoaWindow frame]];
    [[cocoaWindow statusBar] setFrame:NSMakeRect(0, 0, area.size.width, statusBarHeight())];
    [[cocoaWindow contentView] setNeedsDisplay:YES];
  }
}

auto pWindow::_append(mWidget& widget) -> void {
  @autoreleasepool {
    if(auto pWidget = widget.self()) {
      [pWidget->cocoaView removeFromSuperview];
      [[cocoaWindow contentView] addSubview:pWidget->cocoaView positioned:NSWindowAbove relativeTo:nil];
      pWidget->setGeometry(widget.geometry());
      [[cocoaWindow contentView] setNeedsDisplay:YES];
    }
  }
}

}

#endif