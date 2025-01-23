#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "checkers.h"
#include "scenegraph.h"

#include <pspsdk.h>
#include <pspkernel.h>

PSP_MODULE_INFO("checkers", 0, 1, 1);

int
exit_callback(int arg1, int arg2, void *argp)
{
	sceKernelExitGame();
}

int
callback_thread(SceSize args, void *arg)
{
	int cbid;

	cbid = sceKernelCreateCallback("exit_callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
}

int
main(void)
{
	struct scenegraph scenegraph;
	int thread = sceKernelCreateThread("update_thread", callback_thread,
					   0x11, 0xfa0, 0, NULL);
	if (thread >= 0)
		sceKernelStartThread(thread, 0, NULL);

	checkers_init(&scenegraph, 480, 272);

	for (;;) {
		sg_render(&scenegraph);
	}
}
