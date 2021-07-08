/******************************************************************************\

                     Copyright (C) 2020-2021 Buildbotics LLC.

       This program is free software; you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
                       (at your option) any later version.

\******************************************************************************/

#pragma once

#include <X11/Xlib.h>

#include <stdbool.h>

extern bool verbose;

typedef struct {
  int width;
  int height;
} Dim;

void die(const char *fmt, ...);
void message(const char *fmt, ...);
void simulate_key(Display *dpy, KeySym keysym, bool press);
Dim get_display_dims(Display *dpy, int screen);
