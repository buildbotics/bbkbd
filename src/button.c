/******************************************************************************\

                     Copyright (C) 2020-2021 Buildbotics LLC.

       This program is free software; you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
                       (at your option) any later version.

\******************************************************************************/

#include "button.h"
#include "util.h"

#include <X11/Xatom.h>
#include <X11/Xcursor/Xcursor.h>


void button_draw(Button *btn) {
  drw_rect(btn->drw, 0, 0, 100, 100, 1, 1);

  const char *label = "⌨";
  int h = btn->drw->fonts[0].xfont->height * 2;
  int y = (btn->h - h) / 2;
  int w = drw_fontset_getwidth(btn->drw, label);
  int x = (btn->w - w) / 2;
  drw_text(btn->drw, x, y, w, h, 0, label, 0);

  drw_map(btn->drw, btn->win, 0, 0, 100, 100);
}


void button_event(Button *btn, XEvent *e) {
  switch (e->type) {
  case VisibilityNotify: {
    if (e->xvisibility.state == VisibilityFullyObscured)
      XRaiseWindow(btn->drw->dpy, btn->win);
    break;
  }

  case MotionNotify: {
    int x = e->xmotion.x;
    int y = e->xmotion.y;
    btn->mouse_in = 0 <= x && x < btn->w && 0 <= y && y < btn->h;
    break;
  }

  case ButtonPress: break;

  case ButtonRelease:
    if (e->xbutton.button == 1 && btn->mouse_in && btn->cb)
      btn->cb(btn->cb_data);
    break;

  case Expose: if (!e->xexpose.count) button_draw(btn); break;
  }
}


void button_set_callback(Button *btn, button_cb cb, void *data) {
  btn->cb = cb;
  btn->cb_data = data;
}


Button *button_create(Display *dpy, float x, float y, int w, int h,
                      const char *font) {
  Button *btn = (Button *)calloc(1, sizeof(Button));

  int screen = DefaultScreen(dpy);
  Window root = RootWindow(dpy, screen);

  // Dimensions
  Dim dim = get_display_dims(dpy, screen);
  x *= dim.width - w;
  y *= dim.height - h;
  btn->w = w;
  btn->h = h;

  // Create drawable
  Drw *drw = btn->drw = drw_create(dpy, screen, root, w, h);

  // Setup font
  if (!drw_fontset_create(drw, &font, 1)) die("no fonts could be loaded");

  // Init color scheme
  const char *colors[] = {"#bbbbbb", "#132a33"};
  btn->scheme = drw_scm_create(drw, colors, 2);
  drw_setscheme(drw, btn->scheme);

  XSetWindowAttributes wa;
  wa.override_redirect = true;

  btn->win = XCreateWindow
    (dpy, root, x, y, w, h, 0, CopyFromParent, CopyFromParent,
     CopyFromParent, CWOverrideRedirect | CWBorderPixel | CWBackingPixel, &wa);

  // Enable window events
  XSelectInput(dpy, btn->win, ButtonReleaseMask | ButtonPressMask |
               ExposureMask | PointerMotionMask | VisibilityChangeMask);

  // Set window properties
  XWMHints *wmHints = XAllocWMHints();
  wmHints->input = false;
  wmHints->flags = InputHint;

  const char *name = "bbkbd-button";
  XTextProperty str;
  XStringListToTextProperty((char **)&name, 1, &str);

  XClassHint *classHints = XAllocClassHint();
  classHints->res_class = (char *)name;
  classHints->res_name = (char *)name;

  XSetWMProperties(dpy, btn->win, &str, &str, 0, 0, 0, wmHints, classHints);

  XFree(classHints);
  XFree(wmHints);
  XFree(str.value);

  // Set window type
  Atom atom = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", false);
  Atom type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_UTILITY", false);
  XChangeProperty(dpy, btn->win, atom, XA_ATOM, 32, PropModeReplace,
                  (unsigned char *)&type, 1);

  // Set cursor
  Cursor c = XcursorLibraryLoadCursor(dpy, "hand2");
  XDefineCursor(dpy, btn->win, c);

  // Raise window to top of stack
  XMapRaised(dpy, btn->win);

  return btn;
}


void button_destroy(Button *btn) {
  drw_sync(btn->drw);
  drw_free(btn->drw);

  free(btn->scheme);
  free(btn);
}
