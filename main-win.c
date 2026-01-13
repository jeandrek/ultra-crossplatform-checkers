#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/wgl.h>
#include <stdio.h>
#include <string.h>

#include "checkers.h"
#include "scenegraph.h"
#include "input.h"

PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

HGLRC hglrc;

static void
init_window(HWND hWnd)
{
	PIXELFORMATDESCRIPTOR ppfd;
	int format;
	RECT rect;
	HDC hdc;

	memset(&ppfd, 0, sizeof (ppfd));
	ppfd.nSize = sizeof (ppfd);
	ppfd.nVersion = 1;
	ppfd.dwFlags = (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL
			| PFD_DOUBLEBUFFER);
	ppfd.iPixelType = PFD_TYPE_RGBA;
	ppfd.cColorBits = 24;
	ppfd.cDepthBits = 32;
	ppfd.iLayerType = PFD_MAIN_PLANE;

	hdc = GetDC(hWnd);
	format = ChoosePixelFormat(hdc, &ppfd);
	SetPixelFormat(hdc, format, &ppfd);

	hglrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hglrc);
	wglSwapIntervalEXT = (void *)wglGetProcAddress("wglSwapIntervalEXT");
	wglSwapIntervalEXT(1);
	checkers_init();
	sg_init(800, 600);
	ReleaseDC(hWnd, hdc);
}

LRESULT WINAPI
WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CREATE:
		init_window(hWnd);
		return 0;
	case WM_DESTROY:
		wglDeleteContext(hglrc);
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int state[NUM_BUTTONS] = {0};

int WINAPI
WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
	LPSTR lpCmdLine, int nCmdShow)
{
	RECT rect = {0, 0, 800, 600};
	WNDCLASS cls;
	HWND hWnd;
	MSG msg;

	memset(&cls, 0, sizeof (cls));
	cls.lpszClassName = "CheckersWindow";
	cls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	cls.hInstance     = hInst;
	cls.style         = CS_VREDRAW | CS_HREDRAW;
	cls.lpfnWndProc   = (WNDPROC)WndProc;
	RegisterClass(&cls);

	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);

	hWnd = CreateWindowEx(0, "CheckersWindow", "Checkers",
			      WS_OVERLAPPEDWINDOW,
			      CW_USEDEFAULT, CW_USEDEFAULT,
			      rect.right - rect.left,
			      rect.bottom - rect.top,
			      NULL, NULL, hInst, NULL);
	if (hWnd == NULL)
		return 1;
	ShowWindow(hWnd, nCmdShow);

	for (;;) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (LOWORD(msg.message) == WM_QUIT)
				break;
			DispatchMessage(&msg);
		}
		HDC hdc = GetDC(hWnd);
		input_handle();
		checkers_update();
		SwapBuffers(hdc);
		ReleaseDC(hWnd, hdc);
	}
	return 0;
}
