/*
 * Copyright (c) 2026 Jeandre Kruger
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <string.h>

#include "checkers.h"
#include "scenegraph.h"
#include "input.h"
#include "input_mapping-x11.h"
#include "text_input.h"

int
respond_to_event(Display *dpy, XEvent *evt, XPointer arg)
{
	return 1;
}

int button_state[NUM_BUTTONS] = {0};
static int8_t keycode_buttons[256];

static void
key_press_event(XKeyEvent *evt)
{
	char text[5];

	XLookupString(evt, text, sizeof (text), NULL, NULL);
	for (int i = 0; text[i] != 0; i++)
		text_input_add_char(text[i]);

	if (keycode_buttons[evt->keycode] >= 0)
		button_state[keycode_buttons[evt->keycode]] = 1;
}

static void
key_release_event(XKeyEvent *evt)
{
	if (keycode_buttons[evt->keycode] >= 0)
		button_state[keycode_buttons[evt->keycode]] = 0;
}

int
main()
{
	int glx_attribs[] = {
		GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8,
		GLX_DEPTH_SIZE, 16,
		GLX_DOUBLEBUFFER, 1,
		None
	};
	XSetWindowAttributes win_attribs;
	char *name = "Checkers";
	XTextProperty prop;
	Atom protocols[1];
	GLXFBConfig *fb_confs;
	int num_fb_confs;
	GLXWindow glxWin;
	GLXContext ctx;
	Display *dpy;
	Window win;
	XEvent evt;

	dpy = XOpenDisplay(NULL);

	win_attribs.event_mask = (SubstructureNotifyMask
				  | KeyPressMask
				  | KeyReleaseMask
				  | PointerMotionMask
				  | ButtonPressMask
				  | ButtonReleaseMask);

	win = XCreateWindow(dpy, DefaultRootWindow(dpy), 30, 30,
			    800, 600, 0,
			    CopyFromParent, InputOutput, CopyFromParent,
			    CWEventMask, &win_attribs);

	XStringListToTextProperty(&name, 1, &prop);
	XSetWMName(dpy, win, &prop);
	protocols[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", 1);
	XSetWMProtocols(dpy, win, protocols, 1);

	fb_confs = glXChooseFBConfig(dpy, 0, glx_attribs, &num_fb_confs);
	if (num_fb_confs == 0)
		return 1;

	glxWin = glXCreateWindow(dpy, fb_confs[0], win, NULL);
	ctx = glXCreateNewContext(dpy, fb_confs[0],
				  GLX_RGBA_TYPE, NULL, 1);
	glXMakeCurrent(dpy, glxWin, ctx);
	XFree(fb_confs);

	XMapWindow(dpy, win);

	glXSwapIntervalEXT(dpy, glxWin, 1);

	sg_init(800, 600);

	checkers_init();

	XkbSetDetectableAutoRepeat(dpy, True, NULL);

	memset(keycode_buttons, -1, 256);
	for (int i = 0; i < 256; i++) {
		KeySym ks = XkbKeycodeToKeysym(dpy, i, 0, 0);
		for (int j = 0; j < NUM_BUTTONS; j++) {
			if (ks == input_mapping[j]) {
				keycode_buttons[i] = j;
				break;
			}
		}
	}

	for (;;) {
		input_handle();
		checkers_update();
		glXSwapBuffers(dpy, glxWin);
		if (!XCheckIfEvent(dpy, &evt, respond_to_event, NULL))
			continue;
		switch (evt.type) {
		case KeyPress:
			key_press_event(&evt.xkey);
			break;
		case KeyRelease:
			key_release_event(&evt.xkey);
			break;
		case MotionNotify:
			checkers_mouse_move(evt.xbutton.x, evt.xbutton.y);
			break;
		case ButtonRelease:
			checkers_mouse_up(evt.xbutton.x, evt.xbutton.y);
			break;
		case ClientMessage:
			goto quit;
		}
	}
quit:
	glXDestroyContext(dpy, ctx);
	glXDestroyWindow(dpy, glxWin);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
	return 0;
}
