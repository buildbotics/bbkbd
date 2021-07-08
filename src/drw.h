/******************************************************************************\

                     Copyright (C) 2020-2021 Buildbotics LLC.

       This program is free software; you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
                       (at your option) any later version.

\******************************************************************************/

#pragma once

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>


#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))


typedef struct Fnt {
  Display *dpy;
  unsigned h;
  XftFont *xfont;
  FcPattern *pattern;
  struct Fnt *next;
} Fnt;

enum {ColFg, ColBg}; // Clr scheme index
typedef XftColor Clr;

typedef struct {
  unsigned w, h;
  Display *dpy;
  int screen;
  Window root;
  Drawable drawable;
  GC gc;
  Clr *scheme;
  Fnt *fonts;
} Drw;


// Drawable abstraction
Drw *drw_create(Display *dpy, int screen, Window win, unsigned w, unsigned h);
void drw_resize(Drw *drw, unsigned w, unsigned h);
void drw_free(Drw *drw);

// Fnt abstraction
Fnt *drw_fontset_create(Drw *drw, const char *fonts[], size_t fontcount);
void drw_fontset_free(Fnt *set);
unsigned drw_fontset_getwidth(Drw *drw, const char *text);
void drw_font_getexts(Fnt *font, const char *text, unsigned len, unsigned *w,
                      unsigned *h);

// Colorscheme abstraction
void drw_clr_create(Drw *drw, Clr *dest, const char *clrname);
Clr *drw_scm_create(Drw *drw, const char *clrnames[], size_t clrcount);

// Drawing context manipulation
void drw_setfontset(Drw *drw, Fnt *set);
void drw_setscheme(Drw *drw, Clr *scm);

// Drawing functions
void drw_rect(Drw *drw, int x, int y, unsigned w, unsigned h, int filled,
              int invert);
int drw_text(Drw *drw, int x, int y, unsigned w, unsigned h, unsigned lpad,
             const char *text, int invert);

// Map functions
void drw_map(Drw *drw, Window win, int x, int y, unsigned w, unsigned h);
void drw_sync(Drw *drw);
