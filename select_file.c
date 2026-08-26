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

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __psp__
#include <pspdisplay.h>
#include <pspgu.h>
#include <psputility.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"
#endif

#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "checkers.h"
#include "game_save.h"
#include "menu.h"
#include "text.h"
#include "text_input.h"
#include "sprite.h"
#include "select_file.h"

#ifdef __ANDROID__
char save_file_dir[192];
#endif

static void (*select_file_accept)(const char *);
static void (*select_file_cancel)(void);

#define LIST_WINDOW_SIZE	14

int list_scroll_offset = 0;

static char dir_name[128];
struct element *file_elems = NULL;
int num_files = 0;

void	select_file_populate(void);

static void
quit_selecting_file(void)
{
	menu.sg.render[0] = menu_render_items;
	menu.sg.resize = menu_bounds;
	menu.button_event = gui_button_event;
	menu.mouse_up_event = gui_mouse_up_event;
}

int select_file_saving;

static void
select_file_action(int row, int col)
{
	static char path[256];

	if (row == num_files) {
		quit_selecting_file();
		if (col == 0)
			select_file_cancel();
		else
			text_input("Filename", select_file_accept,
				   select_file_cancel);
		return;
	}
	struct stat s;
	stat(file_elems[row].data, &s);
	if (s.st_mode & S_IFDIR) {
		chdir(file_elems[row].data);
		select_file_populate();
	} else {
		quit_selecting_file();
		select_file_accept(file_elems[row].data);
	}
	/* if (row == 3) { */
	/* 	snprintf(path, 256, "%s/autosave.checkers", save_file_dir); */
	/* 	select_file_accept(path); */
	/* 	return; */
	/* } */
	/* if (row == 4) { */
	/* 	snprintf(path, 256, "%s/autosave1.checkers", save_file_dir); */
	/* 	select_file_accept(path); */
	/* 	return; */
	/* } */
	/* snprintf(path, 256, "%s/save%d.checkers", save_file_dir, row + 1); */
}

static struct sprite files_box, scroll_up_but, scroll_down_but, scroll_bar;

static void
select_file_render_items(struct scenegraph *scenegraph)
{
	menu_render_items(scenegraph);
	sprite_draw(scenegraph, &files_box);
	sprite_draw(scenegraph, &scroll_up_but);
	sprite_draw(scenegraph, &scroll_down_but);
	if (num_files > LIST_WINDOW_SIZE)
		sprite_draw(scenegraph, &scroll_bar);
	text_color(0xffffffff);
	text_draw(scenegraph, "^", scroll_up_but.x, scroll_up_but.y, TEXT_CENTRE);
	text_draw(scenegraph, "v", scroll_down_but.x, scroll_down_but.y, TEXT_CENTRE);
	text_draw(scenegraph, "=", scroll_bar.x, scroll_bar.y, TEXT_CENTRE);
	text_color(0xffaaaaaa);
	text_draw(scenegraph, dir_name, 0, 0.9, TEXT_CENTRE);
	text_draw(scenegraph, "Files:", 0, 0.7, TEXT_CENTRE);
	for (int i = list_scroll_offset;
	     i < num_files && i - list_scroll_offset < LIST_WINDOW_SIZE; i++) {
		text_color(button_color(file_elems[i].row, file_elems[i].col, 0));
		text_draw(scenegraph, file_elems[i].data,
			  file_elems[i].x,
			  file_elems[i].y + 0.1 * list_scroll_offset,
			  TEXT_CENTRE);
	}
}

static void
select_file_resize(void)
{
	menu_bounds();
	files_box.width = menu.sg.width/2;
	files_box.height = 8*menu.sg.height/10;
	for (int i = 0; i < num_files; i++)
		button_bounds(&menu.sg, strlen(file_elems[i].data),
			      file_elems[i].x,
			      file_elems[i].y + 0.1 * list_scroll_offset, &file_elems[i].bounds);
	scroll_up_but.x = 0.5 * menu.sg.width/(float)menu.sg.height;
	scroll_up_but.y = 0.8;
	scroll_down_but.x = 0.5 * menu.sg.width/(float)menu.sg.height;
	scroll_down_but.y = -0.8;
	scroll_bar.x = 0.5 * menu.sg.width/(float)menu.sg.height;
	if (num_files > LIST_WINDOW_SIZE)
		scroll_bar.height =
			LIST_WINDOW_SIZE * (8 * menu.sg.height / 10 - 25) / num_files;
}

static void
scroll(void)
{
	float top_x = 0.8 - (25.0 + scroll_bar.height)/menu.sg.height;
	float factor = list_scroll_offset/((float)num_files - LIST_WINDOW_SIZE);
	scroll_bar.y = top_x * (1 - 2 * factor);
	for (int i = 0; i < num_files; i++)
		button_bounds(&menu.sg, strlen(file_elems[i].data),
			      file_elems[i].x,
			      file_elems[i].y + 0.1 * list_scroll_offset, &file_elems[i].bounds);
}

static void
select_file_button_event(int button)
{
	gui_button_event(button);
	if (gui_focus_row < num_files &&
	    gui_focus_row - list_scroll_offset >= LIST_WINDOW_SIZE) {
		list_scroll_offset = gui_focus_row - LIST_WINDOW_SIZE + 1;
		scroll();
	} else if (gui_focus_row < list_scroll_offset) {
		list_scroll_offset = gui_focus_row;
		scroll();
	}
}

static void
select_file_mouse_up_event(int x, int y)
{
	struct rect scroll_up_but_bounds = {
		.left = 3*menu.sg.width/4 - 10,
		.right = 3*menu.sg.width/4 + 10,
		.top = 1*menu.sg.height/10 - 10,
		.bottom = 1*menu.sg.height/10 + 10
	};
	struct rect scroll_down_but_bounds = {
		.left = 3*menu.sg.width/4 - 10,
		.right = 3*menu.sg.width/4 + 10,
		.top = 9*menu.sg.height/10 - 10,
		.bottom = 9*menu.sg.height/10 + 10
	};
	gui_mouse_up_event(x, y);
	if (x >= scroll_up_but_bounds.left && x <= scroll_up_but_bounds.right
	    && y >= scroll_up_but_bounds.top && y <= scroll_up_but_bounds.bottom
	    && list_scroll_offset > 0) {
		list_scroll_offset--; scroll();
	}
	if (x >= scroll_down_but_bounds.left && x <= scroll_down_but_bounds.right
	    && y >= scroll_down_but_bounds.top && y <= scroll_down_but_bounds.bottom
	    && list_scroll_offset < num_files - LIST_WINDOW_SIZE) {
		list_scroll_offset++; scroll();
	}
}

static int
compare_elems(const void *a, const void *b)
{
	const struct element *elem1 = a;
	const struct element *elem2 = b;
	return strcmp(elem1->data, elem2->data);
}

void
select_file_populate(void)
{
	getcwd(dir_name, 128);
	if (file_elems) {
		for (int i = 0; i < num_files; i++)
			free(file_elems[i].data);
		free(file_elems);
	}
	num_files = 0;
	list_scroll_offset = 0;
	gui_set_rows(0);
	int max_files = 16;
	file_elems = malloc(max_files * sizeof (file_elems[0]));
	DIR *dir = opendir(".");
	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL) {
		struct stat s;
		stat(ent->d_name, &s);
		char *ss = strstr(ent->d_name, ".checkers");
		if ((!ss || ss[9]) && !(s.st_mode & S_IFDIR))
			continue;
		if (num_files == max_files) {
			max_files += 16;
			file_elems = realloc(file_elems, max_files * sizeof (file_elems[0]));
		}
		struct element *elem = &file_elems[num_files];
		elem->data = strdup(ent->d_name);
		elem->x = 0;
		elem->disabled = 0;
		num_files++;
	}
	closedir(dir);
	qsort(file_elems, num_files, sizeof (file_elems[0]), compare_elems);
	float y = 0.6;
	for (int i = 0; i < num_files; i++) {
		file_elems[i].y = y;
		y -= 0.1;
		button_bounds(&menu.sg, strlen(file_elems[i].data),
			      file_elems[i].x, file_elems[i].y, &file_elems[i].bounds);
		gui_add_row(1, &file_elems[i]);
	}
	if (select_file_saving)
		gui_add_row(2, &elems[0], &elems[1]);
	else
		gui_add_row(1, &elems[0]);
	if (num_files > LIST_WINDOW_SIZE)
		scroll_bar.height =
			LIST_WINDOW_SIZE * (8 * menu.sg.height / 10 - 25) / num_files;
	scroll();
}

#ifndef __psp__
void
select_file(int saving,
	    void (*accept)(const char *),
	    void (*cancel)(void))
{
#if defined(_WIN32)
	OPENFILENAME ofn = {0};
	char path[256];

	ofn.lStructSize = sizeof (ofn);
	ofn.hwndOwner = checkers_hwnd;
	ofn.lpstrFilter =
		"Checkers saves (*.checkers)\0*.checkers\0"
		"All files (*.*)\0*.*\0\0";
	ofn.lpstrFile = path;
	ofn.nMaxFile = sizeof (path);
	ofn.lpstrDefExt = "checkers";
	if (saving)
		ofn.Flags = OFN_OVERWRITEPROMPT;
	path[0] = 0;

	if ((saving ? GetSaveFileName : GetOpenFileName)(&ofn))
		accept(path);
	else
		cancel();
#elif 0//defined(__ANDROID__)
	static struct element select_file_elems[] = {
		{.x = 0, .y = 0.5, .data = "Empty"},
		{.x = 0, .y = 0.3, .data = "Empty"},
		{.x = 0, .y = 0.1, .data = "Empty"},
		{.x = 0, .y = -0.1, .data = "Empty <Autosave>"},
		{.x = 0, .y = -0.3, .data = "Empty <Autosave-1>"},
		{.x = 0, .y = -0.5, .data = "Back"},
	};
	static char *save_labels[] = {
		"Save 1", "Save 2", "Save 3", "Save 4"
	};
	DIR *dir = opendir(save_file_dir);
	struct dirent *ent;
	for (int i = 0; i < 3; i++)
		select_file_elems[i].disabled = !saving;
	select_file_elems[3].disabled = 1;
	select_file_elems[4].disabled = 1;
	while ((ent = readdir(dir)) != NULL) {
		if (!strncmp(ent->d_name, "save", 4) &&
		    ent->d_name[4] >= '1' && ent->d_name[4] <= '3' &&
		    !strcmp(ent->d_name + 5, ".checkers")) {
			int i = ent->d_name[4] - '1';
			select_file_elems[i].data = save_labels[i];
			select_file_elems[i].disabled = 0;
		}
		if (!strcmp(ent->d_name, "autosave.checkers")) {
			select_file_elems[3].data = "Autosave";
			select_file_elems[3].disabled = 0;
		}
		if (!strcmp(ent->d_name, "autosave1.checkers")) {
			select_file_elems[4].data = "Autosave-1";
			select_file_elems[4].disabled = 0;
		}
	}
	closedir(dir);
	select_file_accept = accept;
	select_file_cancel = cancel;
	menu_set_elements(6, select_file_elems);
	gui_set_rows(6, 1, &elems[0], 1, &elems[1], 1, &elems[2], 1, &elems[3],
		     1, &elems[4], 1, &elems[5]);
	gui_set_action_proc(select_file_action);
#else
	//text_input("Game file path", accept, cancel);
	static struct element elems[] = {
		{.x = -0.2, .y = -0.9, .data = "Back"},
		{.x = 0.2, .y = -0.9, .data = "New~"}
	};
	select_file_accept = accept;
	select_file_cancel = cancel;
	select_file_saving = saving;
	sprite_init(&files_box, NULL, 0, 0,
		    menu.sg.width/2, 8*menu.sg.height/10);
	sprite_init(&scroll_up_but, NULL, 0, 0, 20, 20);
	sprite_init(&scroll_down_but, NULL, 0, 0, 20, 20);
	sprite_init(&scroll_bar, NULL, 0, 0, 20, 40);
	scroll_up_but.base_color = 0xff707070;
	scroll_down_but.base_color = 0xff707070;
	scroll_up_but.x = 0.5 * menu.sg.width/(float)menu.sg.height;
	scroll_up_but.y = 0.8;
	scroll_down_but.x = 0.5 * menu.sg.width/(float)menu.sg.height;
	scroll_down_but.y = -0.8;
	scroll_bar.base_color = 0xff707070;
	scroll_bar.x = 0.5 * menu.sg.width/(float)menu.sg.height;
	files_box.base_color = 0xff101010;
	menu.sg.render[0] = select_file_render_items;
	menu.sg.resize = select_file_resize;
	menu.button_event = select_file_button_event;
	menu.mouse_up_event = select_file_mouse_up_event;
	elems[0].x = saving ? -0.2 : 0;
	menu_set_elements(saving ? 2 : 1, elems);
	gui_set_action_proc(select_file_action);
	select_file_populate();
#endif
}
#endif

#ifdef __psp__
int
select_save(int saving, struct game_save *buf)
{
	PspUtilitySavedataListSaveNewData newData = {0};
	SceUtilitySavedataParam params = {0};

	params.base.size = sizeof (params);
	params.base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH; /* XXX */
	params.base.buttonSwap = 1;
	params.base.graphicsThread = 0x12;
	params.base.accessThread = 0x13;
	params.base.fontThread = 0x14;
	params.base.soundThread = 0x15;

	params.mode = saving ?
		PSP_UTILITY_SAVEDATA_LISTSAVE : PSP_UTILITY_SAVEDATA_LISTLOAD;
	strcpy(params.gameName, "CHECKERS");

	params.saveNameList = malloc(20 * 5);
	strcpy(params.saveNameList[0], "DATA0");
	strcpy(params.saveNameList[1], "DATA1");
	strcpy(params.saveNameList[2], "DATA2");
	strcpy(params.saveNameList[3], "DATA3");
	strcpy(params.saveNameList[4], "");

	strcpy(params.fileName, "GAME");
	params.overwrite = 1;
	params.focus = PSP_UTILITY_SAVEDATA_FOCUS_FIRSTDATA;
	params.dataBuf = buf;
	params.dataBufSize = sizeof (*buf);
	params.dataSize = sizeof (*buf);

	if (saving) {
		strcpy(params.sfoParam.title, "Checkers");
		strcpy(params.sfoParam.savedataTitle, "Save");
		game_save_description(buf, params.sfoParam.detail,
				      sizeof (params.sfoParam.detail));
		params.sfoParam.parentalLevel = 1;
	}

	params.newData = &newData;
	newData.title = "New save";

	strncpy(params.key, "AAAAAAAAAAAAAAAA", 16);

	sceUtilitySavedataInitStart(&params);

	for (;;) {
		sceGuStart(GU_DIRECT, display_list);
		sceGuClearColor(GU_COLOR(0.0, 0.0, 0.0, 1.0));
		sceGuClearDepth(0);
		sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
		sceGuFinish();
		sceGuSync(0, 0);

		switch (sceUtilitySavedataGetStatus()) {
		case PSP_UTILITY_DIALOG_VISIBLE:
			sceUtilitySavedataUpdate(1);
			break;
		case PSP_UTILITY_DIALOG_QUIT:
			sceUtilitySavedataShutdownStart();
			break;
		case PSP_UTILITY_DIALOG_FINISHED:
			break;
		case PSP_UTILITY_DIALOG_NONE:
			goto done;
		}
		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();
	}
done:
	free(params.saveNameList);

	return !params.base.result;
}
#endif
