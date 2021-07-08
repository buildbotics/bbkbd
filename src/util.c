/******************************************************************************\

                     Copyright (C) 2020-2021 Buildbotics LLC.

       This program is free software; you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
                       (at your option) any later version.

\******************************************************************************/

#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif


bool verbose = false;


void die(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);

  if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
    fputc(' ', stderr);
    perror(NULL);

  } else fputc('\n', stderr);

  exit(1);
}


void message(const char *fmt, ...) {
  if (!verbose) return;

  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fflush(stderr);
}



int find_unused_keycode(Display *dpy) {
  // Derived from:
  // https://stackoverflow.com/questions/44313966/
  //   c-xtest-emitting-key-presses-for-every-unicode-character

  int keycode_low, keycode_high;
  XDisplayKeycodes(dpy, &keycode_low, &keycode_high);

  int keysyms_per_keycode = 0;
  KeySym *keysyms =
    XGetKeyboardMapping(dpy, keycode_low, keycode_high - keycode_low,
                        &keysyms_per_keycode);

  for (int i = keycode_low; i <= keycode_high; i++) {
    bool key_is_empty = true;

    for (int j = 0; j < keysyms_per_keycode; j++) {
      int symindex = (i - keycode_low) * keysyms_per_keycode + j;
      if (keysyms[symindex]) key_is_empty = false;
      else break;
    }

    if (key_is_empty) {
      XFree(keysyms);
      return i;
    }
  }

  XFree(keysyms);
  return 1;
}


void simulate_key(Display *dpy, KeySym keysym, bool press) {
  if (!keysym) return;

  KeyCode code = XKeysymToKeycode(dpy, keysym);

  if (!code) {
    static int tmp_keycode = 0;
    if (!tmp_keycode) tmp_keycode = find_unused_keycode(dpy);

    code = tmp_keycode;
    XChangeKeyboardMapping(dpy, tmp_keycode, 1, &keysym, 1);
    XSync(dpy, false);
  }

  XTestFakeKeyEvent(dpy, code, press, 0);
}


Dim get_display_dims(Display *dpy, int screen) {
  Dim dim;

#ifdef XINERAMA
  if (XineramaIsActive(dpy)) {
    int i = 0;
    XineramaScreenInfo *info = XineramaQueryScreens(dpy, &i);
    dim.width  = info[0].width;
    dim.height = info[0].height;
    XFree(info);
    return dim;
  }
#endif

  dim.width  = DisplayWidth(dpy, screen);
  dim.height = DisplayHeight(dpy, screen);

  return dim;
}
