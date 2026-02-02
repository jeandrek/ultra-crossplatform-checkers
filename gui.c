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

#include <string.h>

#include "checkers.h"
#include "game.h"
#include "main_menu.h"
#include "scenegraph.h"
#include "texture.h"
#include "input.h"
#include "text.h"

static int
point_in_rect(int x, int y, struct rect *bounds)
{
	return (x >= bounds->left && x <= bounds->right
		&& y >= bounds->top && y <= bounds->bottom);
}

int gui_focus_row = 0;
int gui_focus_col = 0;
static int num_rows;
static int *row_lengths;
static void (*action)(int, int);

void
gui_set_row_lengths(int new_num_rows, int *new_row_lengths)
{
	num_rows = new_num_rows;
	row_lengths = new_row_lengths;
}

void
gui_set_action_proc(void (*proc)(int, int))
{
	action = proc;
}

void
gui_update(void)
{
	/* for (int i = 0; i < NUM_MENU_ITEMS; i++) { */
	/* 	struct rect *bounds = &buttons[i]; */
	/* 	if (point_in_rect(mouse_x, mouse_y, bounds)) */
	/* 		selected_item = i; */
	/* } */
}

void
gui_mouse_up_event(float x, float y)
{
	/* for (int i = 0; i < NUM_MENU_ITEMS; i++) { */
	/* 	struct rect *bounds = &buttons[i]; */
	/* 	if (point_in_rect(x, y, bounds)) */
	/* 		action(gui_focus_row, gui_focus_col); */
	/* } */
}

void
gui_button_event(int button)
{
	switch (button) {
	case INPUT_LEFT:
		gui_focus_col =
			(gui_focus_col == 0
			 ? row_lengths[gui_focus_row] - 1
			 : gui_focus_col - 1);
		break;
	case INPUT_RIGHT:
		gui_focus_col = (gui_focus_col + 1) % row_lengths[gui_focus_row];
		break;
	case INPUT_UP:
		gui_focus_row = (gui_focus_row == 0 ? num_rows - 1 : gui_focus_row - 1);
		break;
	case INPUT_DOWN:
		gui_focus_row = (gui_focus_row + 1) % num_rows;
		break;
	case INPUT_ACCEPT:
		action(gui_focus_row, gui_focus_col);
		break;
	}
}
