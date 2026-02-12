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

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#ifdef __psp__
#include <pspdisplay.h>
#include <pspgu.h>
#include <psputility.h>
#endif

#include "checkers.h"
#include "scenegraph.h"
#include "text.h"
#include "gui.h"
#include "sprite.h"
#include "text_input.h"

static void text_input_update(void);
#ifdef __psp__
static void text_input_psp(char *label, void (*accept)(char *),
			   void (*cancel)(void));
#endif

struct state text_input_screen = {
	.update = text_input_update,
	.button_event = gui_button_event,
	.mouse_up_event = gui_mouse_up_event,
	.mouse_move_event = gui_mouse_move_event
};

static char text_input_label[64];
static char text_buffer[64];
static int text_len = 0;
static void (*text_input_accept)(char *);
static void (*text_input_cancel)(void);
static struct sprite cursor;
static struct state *old_state;

static void
blink_cursor(void)
{
	static int ticks = 0;
	float t = ticks / 60.0 * 2 * M_PI;
	int alpha = round(255*(sin(t)/2.0 + 0.5));
	cursor.base_color = alpha << 24 | 0xffffff;
	if (ticks++ == 60) {
		ticks = 0;
	}
}

static void
text_input_update(void)
{
	gui_update();
	if (gui_focus_row == 0)
		blink_cursor();
	else
		cursor.base_color = 0xffaaaaaa;
}

static void
text_input_render(struct scenegraph *scenegraph)
{
	cursor.x = (float)FONT_WIDTH * (text_len + 1) / scenegraph->height;
	cursor.y = -10.0 / scenegraph->height;
	sprite_draw(scenegraph, &cursor);

	text_scale(1);
	text_color(0xffffffff);
	text_draw(scenegraph, text_buffer, 0, 0, TEXT_CENTRE);
	text_color(0xffaaaaaa);
	text_draw(scenegraph, text_input_label, 0, 0.2, TEXT_CENTRE);
	text_color(button_color(1, 0));
	text_draw(scenegraph, "OK", -0.2, -0.2, TEXT_CENTRE);
	text_color(button_color(1, 1));
	text_draw(scenegraph, "Cancel", 0.2, -0.2, TEXT_CENTRE);
}

static void (*text_input_render_functions[])(struct scenegraph *) = {
	text_input_render
};

static void
text_input_action(int row, int col)
{
	if (row == 1 && col == 0) {
		checkers_switch_state(old_state);
		text_input_accept(text_buffer);
	} else if (row == 1 && col == 1) {
		checkers_switch_state(old_state);
		text_input_cancel();
	}
}

static void
text_field_bounds(struct scenegraph *scenegraph, struct rect *bounds)
{
	int mid_y = scenegraph->height/2;
	bounds->left = 0;
	bounds->top = mid_y - FONT_HEIGHT/2 - 8;
	bounds->right = scenegraph->width;
	bounds->bottom = mid_y + FONT_HEIGHT/2 + 8;
}

void
text_input(char *label, void (*accept)(char *), void (*cancel)(void))
{
#ifdef __psp__
	text_input_psp(label, accept, cancel);
#else
	static struct element text_field;
	static struct element ok_but;
	static struct element cancel_but;

	memset(&text_input_screen.sg, 0, sizeof (text_input_screen.sg));
	text_input_screen.sg.num_render = sizeof (text_input_render_functions)/sizeof (text_input_render_functions[0]);
	text_input_screen.sg.render = text_input_render_functions;
	sg_init_scenegraph(&text_input_screen.sg);

	text_len = 0;
	text_buffer[0] = 0;
	snprintf(text_input_label, 64, "%s:", label);
	text_input_accept = accept;
	text_input_cancel = cancel;

	sprite_init(&cursor, NULL, 0, 0, 8, 2);
	cursor.base_color = 0xffffffff;

	text_field_bounds(&text_input_screen.sg, &text_field.bounds);
	button_bounds(&text_input_screen.sg, strlen("OK"),
		      -0.2, -0.2, &ok_but.bounds);
	button_bounds(&text_input_screen.sg, strlen("Cancel"),
		      0.2, -0.2, &cancel_but.bounds);
	gui_set_rows(2, 1, &text_field, 2, &ok_but, &cancel_but);
	gui_set_action_proc(text_input_action);

	old_state = checkers_get_state();
	checkers_switch_state(&text_input_screen);
#endif
}

void
text_input_add_char(char c)
{
	if (!(checkers_get_state() == &text_input_screen
	      && gui_focus_row == 0))
		return;

	if (c == '\b' && text_len > 0)
		text_len -= 1;
	else if (isprint(c) && text_len < 63)
		text_buffer[text_len++] = c;
	text_buffer[text_len] = 0;
}

#ifdef __psp__
static void
c16s_to_cs(char *dst, uint16_t *src, size_t n)
{
	size_t i;
	for (i = 0; src[i] != 0 && i < n - 1; i++) dst[i] = src[i];
	dst[i] = 0;
}

static void
cs_to_c16s(uint16_t *dst, char *src, size_t n)
{
	size_t i;
	for (i = 0; src[i] != 0 && i < n - 1; i++) dst[i] = src[i];
	dst[i] = 0;
}

static void
text_input_psp(char *label, void (*accept)(char *),
	       void (*cancel)(void))
{
	SceUtilityOskParams params;
	SceUtilityOskData data;
	uint16_t out_text[64] = {0};
	uint16_t in_text[] = {0};
	uint16_t desc[64];

	memset(&data, 0, sizeof (data));
	data.language =  PSP_UTILITY_OSK_LANGUAGE_DEFAULT;
	data.lines = 1;
	cs_to_c16s(desc, label, 64);
	data.desc = desc;
	data.intext = in_text;
	data.outtext = out_text;
	data.outtextlength = 64;
	data.outtextlimit = 64;

	memset(&params, 0, sizeof (params));
	params.base.size = sizeof (params);
	params.base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH; /* XXX */
	params.base.buttonSwap = 1; /* XXX */
	params.base.graphicsThread = 0x12;
	params.base.accessThread = 0x13;
	params.base.fontThread = 0x14;
	params.base.soundThread = 0x15;
	params.datacount = 1;
	params.data = &data;

	sceUtilityOskInitStart(&params);

	for (;;) {
		sceGuStart(GU_DIRECT, display_list);
		sceGuClearColor(GU_COLOR(0.0, 0.0, 0.0, 1.0));
		sceGuClearDepth(0);
		sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
		sceGuFinish();
		sceGuSync(0, 0);

		switch (sceUtilityOskGetStatus()) {
		case PSP_UTILITY_DIALOG_VISIBLE:
			sceUtilityOskUpdate(1);
			break;
		case PSP_UTILITY_DIALOG_QUIT:
			sceUtilityOskShutdownStart();
			break;
		case PSP_UTILITY_DIALOG_FINISHED:
			c16s_to_cs(text_buffer, out_text, 64);
			accept(text_buffer);
			return;
		case PSP_UTILITY_DIALOG_NONE:
			break;
		}
		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();
	}
}
#endif
