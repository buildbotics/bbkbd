/******************************************************************************\

                     Copyright (C) 2020-2021 Buildbotics LLC.

       This program is free software; you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
                       (at your option) any later version.

\******************************************************************************/

#pragma once

#include "drw.h"
#include "util.h"

#include <stdbool.h>


enum {
  SchemeNorm, SchemeNormABC, SchemePress, SchemeHighlight, SchemeBG, SchemeLast
};

typedef void (*keyboard_show_cb)(bool show);

typedef struct {
  char *label;
  char *label2;
  KeySym keysym;
  unsigned width;
  int x, y, w, h;
  bool pressed;
} Key;

typedef struct {
  Window win;
  Drw *drw;

  int space;
  int w, h;
  int x, y;
  int rows;
  int cols;

  bool meta;
  bool shift;
  bool visible;

  Key *pressed;
  Key *focus;
  Key **keys;

  char *font;
  Clr *scheme[SchemeLast];

  keyboard_show_cb show_cb;
} Keyboard;


void keyboard_destroy(Keyboard *kbd);
Keyboard *keyboard_create(Display *dpy, Key **keys, int space, const char *font,
                          const char *colors[SchemeLast][2]);

void keyboard_event(Keyboard *kbd, XEvent *e);
void keyboard_toggle(Keyboard *kbd);
