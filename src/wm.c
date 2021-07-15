/******************************************************************************\

                     Copyright (C) 2020-2021 Buildbotics LLC.

       This program is free software; you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
                       (at your option) any later version.

\******************************************************************************/

#include "wm.h"
#include "util.h"


#include <stdio.h>

#define MAX_CLIENTS 32
#define WINDOW_FMT "0x%06lx"

static bool wm_detected = false;
static Display *wm_dpy = 0;
static Window wm_clients[MAX_CLIENTS] = {0};
static Window wm_active = 0;
static int wm_keyboard_margin = 0;



const char *request_name(unsigned code) {
  static const char * const names[] = {
    "",
    "CreateWindow",
    "ChangeWindowAttributes",
    "GetWindowAttributes",
    "DestroyWindow",
    "DestroySubwindows",
    "ChangeSaveSet",
    "ReparentWindow",
    "MapWindow",
    "MapSubwindows",
    "UnmapWindow",
    "UnmapSubwindows",
    "ConfigureWindow",
    "CirculateWindow",
    "GetGeometry",
    "QueryTree",
    "InternAtom",
    "GetAtomName",
    "ChangeProperty",
    "DeleteProperty",
    "GetProperty",
    "ListProperties",
    "SetSelectionOwner",
    "GetSelectionOwner",
    "ConvertSelection",
    "SendEvent",
    "GrabPointer",
    "UngrabPointer",
    "GrabButton",
    "UngrabButton",
    "ChangeActivePointerGrab",
    "GrabKeyboard",
    "UngrabKeyboard",
    "GrabKey",
    "UngrabKey",
    "AllowEvents",
    "GrabServer",
    "UngrabServer",
    "QueryPointer",
    "GetMotionEvents",
    "TranslateCoords",
    "WarpPointer",
    "SetInputFocus",
    "GetInputFocus",
    "QueryKeymap",
    "OpenFont",
    "CloseFont",
    "QueryFont",
    "QueryTextExtents",
    "ListFonts",
    "ListFontsWithInfo",
    "SetFontPath",
    "GetFontPath",
    "CreatePixmap",
    "FreePixmap",
    "CreateGC",
    "ChangeGC",
    "CopyGC",
    "SetDashes",
    "SetClipRectangles",
    "FreeGC",
    "ClearArea",
    "CopyArea",
    "CopyPlane",
    "PolyPoint",
    "PolyLine",
    "PolySegment",
    "PolyRectangle",
    "PolyArc",
    "FillPoly",
    "PolyFillRectangle",
    "PolyFillArc",
    "PutImage",
    "GetImage",
    "PolyText8",
    "PolyText16",
    "ImageText8",
    "ImageText16",
    "CreateColormap",
    "FreeColormap",
    "CopyColormapAndFree",
    "InstallColormap",
    "UninstallColormap",
    "ListInstalledColormaps",
    "AllocColor",
    "AllocNamedColor",
    "AllocColorCells",
    "AllocColorPlanes",
    "FreeColors",
    "StoreColors",
    "StoreNamedColor",
    "QueryColors",
    "LookupColor",
    "CreateCursor",
    "CreateGlyphCursor",
    "FreeCursor",
    "RecolorCursor",
    "QueryBestSize",
    "QueryExtension",
    "ListExtensions",
    "ChangeKeyboardMapping",
    "GetKeyboardMapping",
    "ChangeKeyboardControl",
    "GetKeyboardControl",
    "Bell",
    "ChangePointerControl",
    "GetPointerControl",
    "SetScreenSaver",
    "GetScreenSaver",
    "ChangeHosts",
    "ListHosts",
    "SetAccessControl",
    "SetCloseDownMode",
    "KillClient",
    "RotateProperties",
    "ForceScreenSaver",
    "SetPointerMapping",
    "GetPointerMapping",
    "SetModifierMapping",
    "GetModifierMapping",
    "NoOperation",
  };

  return names[code];
}


const char *event_name(XEvent *e) {
  switch (e->type) {
  case CreateNotify:     return "CreateNotify";
  case DestroyNotify:    return "DestroyNotify";
  case ReparentNotify:   return "ReparentNotify";
  case MapNotify:        return "MapNotify";
  case UnmapNotify:      return "UnmapNotify";
  case ConfigureNotify:  return "ConfigureNotify";
  case MapRequest:       return "MapRequest";
  case ConfigureRequest: return "ConfigureRequest";
  default: return 0;
  }
}


static int on_wm_detected(Display *dpy, XErrorEvent *e) {
  if (e->error_code == BadAccess) wm_detected = true;
  return 0;
}


static int on_x_error(Display *dpy, XErrorEvent *e) {
  char text[1024];

  XGetErrorText(dpy, e->error_code, text, sizeof(text));
  fprintf(stderr, "X error:\n"
          "  Request: %d %s\n"
          "  Code: %d %s\n"
          "  Resource: %lu\n",
          e->request_code, request_name(e->request_code), e->error_code, text,
          e->resourceid);

  return 0;
}


static void _activate_window(Window win) {
  Dim dim = get_display_dims(wm_dpy, DefaultScreen(wm_dpy));
  int y_offset = 0;
  int width = dim.width;
  int height = dim.height - wm_keyboard_margin + y_offset;

  XRaiseWindow(wm_dpy, win);
  XMoveResizeWindow(wm_dpy, win, 0, -y_offset, width, height);
  XSetInputFocus(wm_dpy, win, RevertToNone, CurrentTime);
  XSync(wm_dpy, false);
  wm_active = win;
  printf("Activated " WINDOW_FMT "\n", win);
}


static void _focus_window() {
  if (wm_active) return _activate_window(wm_active);

  for (int i = 0; i < MAX_CLIENTS; i++)
    if (wm_clients[i]) {
      XWindowAttributes attrs;
      XGetWindowAttributes(wm_dpy, wm_clients[i], &attrs);

      if (attrs.map_state == IsViewable) {
        _activate_window(wm_clients[i]);
        break;
      }
    }
}


void wm_init(Display *dpy) {
  wm_dpy = dpy;
  Window root = DefaultRootWindow(dpy);

  // Check for other WM
  XSetErrorHandler(on_wm_detected);
  XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask);
  XSync(dpy, false);

  if (wm_detected) die("Window manager already exists on display");

  XSetErrorHandler(on_x_error);

  XGrabServer(dpy);

  Window parent;
  Window *windows = 0;
  unsigned num_windows;
  XQueryTree(dpy, root, &root, &parent, &windows, &num_windows);

  for (unsigned i = 0; i < num_windows; i++) {
    Window win = windows[i];

    XWindowAttributes attrs;
    XGetWindowAttributes(dpy, win, &attrs);

    if (attrs.override_redirect || attrs.map_state != IsViewable) continue;

    XAddToSaveSet(dpy, win);
  }

  XFree(windows);
  XUngrabServer(dpy);
}


void wm_event(XEvent *e) {
  switch (e->type) {
  case DestroyNotify: {
    XDestroyWindowEvent *ex = &e->xdestroywindow;

    if (wm_active == ex->window) wm_active = 0;

    for (unsigned i = 0; i < MAX_CLIENTS; i++)
      if (wm_clients[i] && wm_clients[i] == ex->window) {
        printf("Clear WM client " WINDOW_FMT "\n", ex->window);
        wm_clients[i] = 0;
      }

    _focus_window();
    break;
  }

  case UnmapNotify: {
    XUnmapEvent *ex = &e->xunmap;

    if (wm_active == ex->window) {
      wm_active = 0;
      _focus_window();
    }

    break;
  }

  case MapRequest: {
    XMapRequestEvent *ex = &e->xmaprequest;

    for (int i = 0; i < MAX_CLIENTS; i++)
      if (!wm_clients[i]) {
        wm_clients[i] = ex->window;

        XMapWindow(wm_dpy, ex->window);
        printf("Mapped " WINDOW_FMT "\n", ex->window);
        _activate_window(wm_clients[i]);
        return;
      }

    printf("Too many windows\n");
    break;
  }
  }
}


void wm_keyboard(Keyboard *kbd) {
  wm_keyboard_margin = kbd->visible ? kbd->h : 0;
  _focus_window();
}
