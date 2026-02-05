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

#if defined(__psp__)
#include <pspctrl.h>

#include "input_mapping-psp.h"
#elif defined(_WIN32)
#include <windows.h>

#include "input_mapping-win.h"
#endif

#include "checkers.h"
#include "input.h"
#include "config.h"

static const int can_repeat[NUM_BUTTONS] = {
	[INPUT_UP] = 1,
	[INPUT_DOWN] = 1,
	[INPUT_LEFT] = 1,
	[INPUT_RIGHT] = 1
};

static int repeat_delay[NUM_BUTTONS] = {0};

static void
handle_button(int button)
{
	if (repeat_delay[button] == 0) {
		checkers_button_event(button);
		repeat_delay[button] = can_repeat[button] ? REPEAT_DELAY : -1;
	} else if (repeat_delay[button] > 0) {
		repeat_delay[button]--;
	}
}

#ifdef USE_X11
extern int button_state[NUM_BUTTONS];
#endif

void
input_handle(void)
{
#if defined(__psp__)
	SceCtrlData data;

	sceCtrlReadBufferPositive(&data, 1);
#elif defined(_WIN32)
	uint8_t state[256];

	GetKeyboardState(state);
#endif
	for (int i = 0; i < NUM_BUTTONS; i++) {
#if defined(__psp__)
		int val = input_mapping[i];
		if (data.Buttons & val) handle_button(i);
#elif defined(_WIN32)
		int val = input_mapping[i];
		if (state[val] >> 7) handle_button(i);
#elif defined(USE_X11)
		if (button_state[i]) handle_button(i);
#else
		if (0) handle_button(i);
#endif
		else repeat_delay[i] = 0;
	}
}
