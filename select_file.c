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

#ifdef __ANDROID__
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#endif

#include "checkers.h"
#include "game_save.h"
#include "menu.h"
#include "text_input.h"
#include "select_file.h"

#ifdef __ANDROID__
char save_file_dir[192];

static void (*select_file_accept)(const char *);
static void (*select_file_cancel)(void);

static void
select_file_action(int row, int col)
{
	static char path[256];

	if (row == 5) {
		select_file_cancel();
		return;
	}
	snprintf(path, 256, "%s/save%d.checkers", save_file_dir, row + 1);
	select_file_accept(path);
}
#endif

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
#elif defined(__ANDROID__)
	static struct element select_file_elems[] = {
		{.x = 0, .y = 0.5, .data = "Empty"},
		{.x = 0, .y = 0.3, .data = "Empty"},
		{.x = 0, .y = 0.1, .data = "Empty"},
		{.x = 0, .y = -0.1, .data = "Empty"},
		{.x = 0, .y = -0.3, .data = "Empty"},
		{.x = 0, .y = -0.5, .data = "Back"}
	};
	static char *save_labels[] = {
		"Save 1", "Save 2", "Save 3", "Save 4", "Save 5"
	};
	DIR *dir = opendir(save_file_dir);
	struct dirent *ent;
	for (int i = 0; i < 5; i++)
		select_file_elems[i].disabled = !saving;
	while ((ent = readdir(dir)) != NULL) {
		if (!strncmp(ent->d_name, "save", 4) &&
		    ent->d_name[4] >= '1' && ent->d_name[4] <= '5' &&
		    !strcmp(ent->d_name + 5, ".checkers")) {
			int i = ent->d_name[4] - '1';
			select_file_elems[i].data = save_labels[i];
			select_file_elems[i].disabled = 0;
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
	text_input("Game file path", accept, cancel);
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
