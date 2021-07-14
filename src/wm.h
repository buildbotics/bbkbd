/******************************************************************************\

                     Copyright (C) 2020-2021 Buildbotics LLC.

       This program is free software; you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
                       (at your option) any later version.

\******************************************************************************/

#pragma once

#include "keyboard.h"

#include <X11/Xlib.h>


void wm_init(Display *dpy);
void wm_event(XEvent *e);
void wm_keyboard(Keyboard *kbd);
