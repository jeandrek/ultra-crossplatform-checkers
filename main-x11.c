#include <X11/Xlib.h>
#include <X11/Xutil.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <X11/XKBlib.h>
#include<string.h>
#include "checkers.h"
#include "scenegraph.h"
#include "input.h"
#include "input_mapping-x11.h"

int
respond_to(Display *dpy, XEvent *evt, XPointer arg)
{
}

int state[NUM_BUTTONS] = {0};

int
main()
{
	Display *dpy = XOpenDisplay(":0");
	int numFbConfs;

	XSetWindowAttributes winAttrib;
	winAttrib.event_mask = SubstructureNotifyMask|KeyPressMask|KeyReleaseMask;

	Window win = XCreateWindow(dpy, DefaultRootWindow(dpy), 30, 30,
				   800, 600, 0,
				   CopyFromParent, InputOutput, CopyFromParent,
				   CWEventMask, &winAttrib);

	char *name = "Checkers";
	XTextProperty prop;
	XStringListToTextProperty(&name, 1, &prop);
	XSetWMName(dpy, win, &prop);
	Atom protocols[1];
	protocols[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", 1);
	XSetWMProtocols(dpy, win, protocols, 1);

	XEvent evt;

	int attribs[] = {
		GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8,
		GLX_DEPTH_SIZE, 16,
		GLX_DOUBLEBUFFER, 1,
		None
	};

	GLXFBConfig *fbconf = glXChooseFBConfig(dpy, 0, attribs, &numFbConfs);
	if (numFbConfs == 0) return 1;
	GLXWindow glxWin = glXCreateWindow(dpy, fbconf[0], win, NULL);
	GLXContext ctx = glXCreateNewContext(dpy, fbconf[0],
					     GLX_RGBA_TYPE, NULL, 1);
	glXMakeCurrent(dpy, glxWin, ctx);
	XFree(fbconf);

	XMapWindow(dpy, win);

	glXSwapIntervalEXT(dpy, glxWin, 1);

	sg_init(800, 600);

	checkers_init();

	XkbSetDetectableAutoRepeat(dpy, True, NULL);

	int8_t keycode_buttons[256];
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
		if (!XCheckIfEvent(dpy, &evt, respond_to, NULL))
			continue;
		switch (evt.type) {
		case KeyPress:
			if (keycode_buttons[evt.xkey.keycode] >= 0)
			state[keycode_buttons[evt.xkey.keycode]] = 1;
			break;
		case KeyRelease:
			if (keycode_buttons[evt.xkey.keycode] >= 0)
			state[keycode_buttons[evt.xkey.keycode]] = 0;
			break;
		case ClientMessage:
			goto quit;
		}
	}
 quit:
	XDestroyWindow(dpy, win);

	XCloseDisplay(dpy);
}
