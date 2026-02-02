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
#include <stdarg.h>
#include <stdlib.h>

#include "scenegraph.h"
#include "text.h"
#include "input.h"
#include "gui.h"

int gui_focus_row = 0;
int gui_focus_col = 0;

static int num_rows;

static struct row {
	int	len;
	struct	element **elems;
} *rows = NULL;

static void (*action)(int, int);

uint32_t
button_color(int i, int j)
{
	if (i == gui_focus_row && j == gui_focus_col)
		return 0xffffffff;
	else
		return 0xffaaaaaa;
}

void
button_bounds(struct scenegraph *scenegraph, int len,
	      float x, float y, struct rect *bounds)
{
	text_screen_bounds(scenegraph, len, x, y, TEXT_CENTRE, bounds);
	bounds->left -= 8;
	bounds->top -= 8;
	bounds->right += 8;
	bounds->bottom += 8;
}

static int
point_in_rect(int x, int y, struct rect *bounds)
{
	return (x >= bounds->left && x <= bounds->right
		&& y >= bounds->top && y <= bounds->bottom);
}

void
gui_set_rows(int num, ...)
{
	va_list ap;

	gui_free_rows();
	num_rows = num;
	rows = malloc(num * sizeof (struct row));

	va_start(ap, num);
	for (int i = 0; i < num; i++) {
		rows[i].len = va_arg(ap, int);
		rows[i].elems = malloc(rows[i].len * sizeof (struct element *));
		for (int j = 0; j < rows[i].len; j++)
			gui_set_element(i, j, va_arg(ap, struct element *));
	}
	va_end(ap);
}

void
gui_set_row_lengths(int num, int *lengths)
{
	gui_free_rows();
	num_rows = num;
	rows = malloc(num * sizeof (struct row));
	for (int i = 0; i < num; i++) {
		rows[i].len = lengths[i];
		rows[i].elems = malloc(lengths[i] * sizeof (struct element *));
	}
}

void
gui_free_rows(void)
{
	if (rows != NULL) {
		for (int i = 0; i < num_rows; i++)
			free(rows[i].elems);
		free(rows);
	}
}

void
gui_set_element(int i, int j, struct element *elem)
{
	elem->row = i;
	elem->col = j;
	rows[i].elems[j] = elem;
}

void
gui_set_action_proc(void (*proc)(int, int))
{
	action = proc;
}

void
gui_update(void)
{
	for (int i = 0; i < num_rows; i++) {
		for (int j = 0; j < rows[i].len; j++) {
			struct rect *bounds = &rows[i].elems[j]->bounds;
			if (point_in_rect(mouse_x, mouse_y, bounds)) {
				gui_focus_row = i;
				gui_focus_col = j;
			}
		}
	}
}

void
gui_mouse_up_event(float x, float y)
{
	for (int i = 0; i < num_rows; i++) {
		for (int j = 0; j < rows[i].len; j++) {
			struct rect *bounds = &rows[i].elems[j]->bounds;
			if (point_in_rect(mouse_x, mouse_y, bounds))
				action(i, j);
		}
	}
}

void
gui_button_event(int button)
{
	switch (button) {
	case INPUT_LEFT:
		gui_focus_col =
			(gui_focus_col == 0
			 ? rows[gui_focus_row].len - 1
			 : gui_focus_col - 1);
		break;
	case INPUT_RIGHT:
		gui_focus_col = (gui_focus_col + 1) % rows[gui_focus_row].len;
		break;
	case INPUT_UP:
		gui_focus_row =
			(gui_focus_row == 0 ? num_rows - 1 : gui_focus_row - 1);
		gui_focus_col = 0;
		break;
	case INPUT_DOWN:
		gui_focus_row = (gui_focus_row + 1) % num_rows;
		gui_focus_col = 0;
		break;
	case INPUT_ACCEPT:
		action(gui_focus_row, gui_focus_col);
		break;
	}
}
