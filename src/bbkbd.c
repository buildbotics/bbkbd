/******************************************************************************\

                     Copyright (C) 2020-2021 Buildbotics LLC.

       This program is free software; you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
                       (at your option) any later version.

\******************************************************************************/

#include "keyboard.h"
#include "button.h"
#include "drw.h"
#include "util.h"
#include "wm.h"
#include "config.h"

#include <signal.h>
#include <locale.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>


#define DEFAULT_FONT "DejaVu Sans:size=18"

static const char *font = DEFAULT_FONT;
static float button_x = 1;
static float button_y = 0;
static bool running = true;
static const char *show_cmd = 0;
static const char *hide_cmd = 0;
static const char *kiosk_cmd = 0;
static int space = 4;
static volatile bool signal_open = false;
static bool button_open = false;


static void signaled(int sig) {
  running = false;
  message("Signal %d received\n", sig);
}


static void kbd_signal(int sig) {
  signal_open = sig == SIGUSR1;
  message("Signal %d received\n", sig);
}


void usage(char *argv0, int ret) {
  const char *usage =
    "usage: %s [-hdb] [-f <font>] [-b <x> <y>]\n"
    "Options:\n"
    "  -h         - Print this help screen and exit\n"
    "  -v         - Verbose output\n"
    "  -f <font>  - Font string, default: " DEFAULT_FONT "\n"
    "  -b <x> <y> - Button screen position. Values between 0 and 1.\n"
    "  -S <cmd>   - Command to run before showing the keyboard.\n"
    "  -H <cmd>   - Command to run after hiding the keyboard.\n"
    "  -k <cmd>   - Run in kiosk mode.  Command is run as child process.\n"
    "  -s <int>   - Space between buttons.\n";

  fprintf(ret ? stderr : stdout, usage, argv0);
  exit(ret);
}


void parse_args(int argc, char *argv[]) {
  for (int i = 1; argv[i]; i++) {
    if (!strcmp(argv[i], "-h")) usage(argv[0], 0);
    else if (!strcmp(argv[i], "-v")) verbose = true;
    else if (!strcmp(argv[i], "-f")) {
      if (argc - 1 <= i) usage(argv[0], 1);
      font = argv[++i];

    } else if (!strcmp(argv[i], "-b")) {
      if (argc - 2 <= i) usage(argv[0], 1);
      button_x = atof(argv[++i]);
      button_y = atof(argv[++i]);

    } else if (!strcmp(argv[i], "-S")) {
      if (argc - 1 <= i) usage(argv[0], 1);
      show_cmd = argv[++i];

    } else if (!strcmp(argv[i], "-H")) {
      if (argc - 1 <= i) usage(argv[0], 1);
      hide_cmd = argv[++i];

    } else if (!strcmp(argv[i], "-k")) {
      if (argc - 1 <= i) usage(argv[0], 1);
      kiosk_cmd = argv[++i];

    } else if (!strcmp(argv[i], "-s")) {
      if (argc - 1 <= i) usage(argv[0], 1);
      space = atoi(argv[++i]);

    } else {
      fprintf(stderr, "Invalid argument: %s\n", argv[i]);
      usage(argv[0], 1);
    }
  }
}


static void toggle(Keyboard *kbd) {
  if (!kbd->visible && show_cmd) system(show_cmd);
  keyboard_toggle(kbd);
  if (!kbd->visible && hide_cmd) system(hide_cmd);
}


static void button_callback(Keyboard *kbd) {
  button_open = !kbd->visible;
  signal_open = false;
  toggle(kbd);
  wm_keyboard(kbd);
}


int main(int argc, char *argv[]) {
  signal(SIGTERM, signaled);
  signal(SIGINT,  signaled);
  signal(SIGUSR1, kbd_signal);
  signal(SIGUSR2, kbd_signal);

  parse_args(argc, argv);

  // Check locale support
  if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
    fprintf(stderr, "warning: no locale support");

  // Init
  Display *dpy = XOpenDisplay(0);
  if (!dpy) die("cannot open display");

  // Create window manager
  if (kiosk_cmd) {
    wm_init(dpy);

    int child = fork();
    if (child == -1) die("Failed to execute child process");
    if (!child) execl("/bin/sh", "sh", "-c", kiosk_cmd, NULL);
  }

  Button *btn = button_create(dpy, button_x, button_y, 55, 35, font);
  Keyboard *kbd = keyboard_create(dpy, keys, space, font, colors);

  button_set_callback(btn, button_callback, kbd);

  // Event loop
  while (running) {
    // Handle signal
    if (!button_open &&
        ((!kbd->visible && signal_open) || (kbd->visible && !signal_open)))
      toggle(kbd);

    // Wait for input
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000; // 100ms

    int xfd = ConnectionNumber(dpy);
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(xfd, &fds);
    int r = select(xfd + 1, &fds, 0, 0, &tv);

    if (r == -1 && errno != EINTR) break;

    while (XPending(dpy)) {
      XEvent ev;
      XNextEvent(dpy, &ev);

      wm_event(&ev);
      if (ev.xany.window == kbd->win) keyboard_event(kbd, &ev);
      if (ev.xany.window == btn->win) button_event(btn, &ev);
    }
  }

  // Cleanup
  button_destroy(btn);
  keyboard_destroy(kbd);
  XCloseDisplay(dpy);

  return 0;
}
