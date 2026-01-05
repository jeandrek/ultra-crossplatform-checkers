#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(__GNUC__) && defined(macintosh)
/* Retro68 */
#include <Carbon.h>
#include <agl.h>
#include "extra_carbon_defs.h"
#else
#include <Carbon/Carbon.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#endif

#include <AGL/agl.h>

#include "checkers.h"
#include "scenegraph.h"
#include "input.h"

#define WIDTH 800
#define HEIGHT 600

static AGLContext context;

static OSStatus WindowEventHandler(EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon);

int
main(void)
{
  OSStatus err;
  WindowRef window;
  static EventTypeSpec kWindowEvents[] = {
    {kEventClassWindow, kEventWindowDrawContent},
    {kEventClassWindow, kEventWindowBoundsChanged},
    {kEventClassWindow, kEventWindowActivated}
  };
  CGrafPtr port;
  GLint attribs[] = {AGL_RGBA, AGL_DOUBLEBUFFER, AGL_DEPTH_SIZE, 24, AGL_NONE};
  AGLPixelFormat pix;
  GLint swap_interval[] = {1};
  Rect bounds = {100, 100, 100+HEIGHT, 100+WIDTH};

  err = CreateNewWindow(kDocumentWindowClass,
			(kWindowStandardFloatingAttributes |
			 kWindowStandardHandlerAttribute),
			&bounds, &window);

  InstallEventHandler(GetWindowEventTarget(window),
		      NewEventHandlerUPP(WindowEventHandler),
		      sizeof (kWindowEvents) / sizeof (EventTypeSpec),
		      kWindowEvents,
		      window, NULL);

  pix = aglChoosePixelFormat(NULL, 0, attribs);
  context = aglCreateContext(pix, NULL);
  aglDestroyPixelFormat(pix);
  port = GetWindowPort(window);
  aglSetDrawable(context, port);
  aglSetCurrentContext(context);
  aglSetInteger(context, AGL_SWAP_INTERVAL, swap_interval);

  sg_init(WIDTH, HEIGHT);
  checkers_init();

  ShowWindow(window);

  RunApplicationEventLoop();

  return err;
}

static OSStatus
WindowEventHandler(EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon)
{
  OSStatus err = eventNotHandledErr;

  switch (GetEventClass(inEvent)) {
  case kEventClassWindow:
    {
      switch (GetEventKind(inEvent)) {
      case kEventWindowActivated:
      case kEventWindowBoundsChanged:
	aglUpdateContext(context);
	break;
      case kEventWindowDrawContent:
	{
	  WindowRef window;
	  Rect rect = {0, 0, HEIGHT, WIDTH};
	  GetEventParameter(inEvent, '----', 'wind', NULL, sizeof (WindowRef), NULL, &window);
	  input_handle();
	  checkers_update();
	  aglSwapBuffers(context);
	  InvalWindowRect(window, &rect);
	}
	break;
      }
    }
  default:
    break;
  }
  return err;
}

