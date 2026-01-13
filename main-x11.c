#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <string.h>

#include "checkers.h"
#include "scenegraph.h"
#include "input.h"
#include "input_mapping-x11.h"

int
respond_to_event(Display *dpy, XEvent *evt, XPointer arg)
{
	return 1;
}

int button_state[NUM_BUTTONS] = {0};

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
	int8_t keycode_buttons[256];
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
				  | KeyReleaseMask);

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
			if (keycode_buttons[evt.xkey.keycode] >= 0)
				button_state[keycode_buttons[evt.xkey.keycode]] = 1;
			break;
		case KeyRelease:
			if (keycode_buttons[evt.xkey.keycode] >= 0)
				button_state[keycode_buttons[evt.xkey.keycode]] = 0;
			break;
		case ClientMessage:
			goto quit;
		}
	}
quit:
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
	return 0;
}
