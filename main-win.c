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

#include <windows.h>
#include <GL/gl.h>
#include <GL/wgl.h>
#include <stdio.h>
#include <string.h>

#include "checkers.h"
#include "scenegraph.h"
#include "input.h"

static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

static HGLRC hglrc;
static int vsync = 0;

static void
init_window(HWND hWnd)
{
	PIXELFORMATDESCRIPTOR ppfd;
	const char *extensions;
	int format;
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

	extensions = (char *)glGetString(GL_EXTENSIONS);
	if (strstr(extensions, "WGL_EXT_swap_control") != NULL) {
		vsync = 1;
		wglSwapIntervalEXT =
			(void *)wglGetProcAddress("wglSwapIntervalEXT");
		wglSwapIntervalEXT(1);
	}
	sg_init(800, 600);
	checkers_init();
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

int WINAPI
WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
	LPSTR lpCmdLine, int nCmdShow)
{
	RECT rect = {0, 0, 800, 600};
	WNDCLASS cls;
	DWORD ticks;
	HWND hWnd;
	MSG msg;

	memset(&cls, 0, sizeof (cls));
	cls.lpszClassName = "CheckersWindow";
	cls.hCursor       = LoadCursor(NULL, IDC_ARROW);
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

	if (!vsync) ticks = timeGetTime();

	for (;;) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (LOWORD(msg.message) == WM_QUIT)
				break;
			DispatchMessage(&msg);
		}
		if (vsync || timeGetTime() - ticks >= 15) {
			HDC hdc = GetDC(hWnd);
			input_handle();
			checkers_update();
			SwapBuffers(hdc);
			ReleaseDC(hWnd, hdc);
			ticks = timeGetTime();
		} else {
			Sleep(2);
		}
	}
	return 0;
}
