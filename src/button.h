/******************************************************************************\

                     Copyright (C) 2020-2021 Buildbotics LLC.

       This program is free software; you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
                       (at your option) any later version.

\******************************************************************************/

#pragma once

#include "drw.h"

#include <stdbool.h>

typedef void (*button_cb)();

typedef struct {
  Window win;
  Drw *drw;
  Clr *scheme;

  button_cb cb;
  void *cb_data;

  int w;
  int h;
  bool mouse_in;
} Button;


Button *button_create(Display *dpy, float x, float y, int w, int h,
                      const char *font);
void button_destroy(Button *btn);
void button_set_callback(Button *btn, button_cb cb, void *data);
void button_event(Button *btn, XEvent *e);
