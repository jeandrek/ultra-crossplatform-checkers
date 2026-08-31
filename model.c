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

#include <stdint.h>
#include <stdlib.h>

#if defined(__psp__)
#include <pspkernel.h>
#endif

#include "scenegraph.h"
#include "asset.h"
#include "model.h"

#if defined(__BIG_ENDIAN__) || \
    defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define SWAP_DATA_ENDIANNESS
#endif

#ifdef SWAP_DATA_ENDIANNESS
static void
swap_array_endianness(void *data, size_t size)
{
	uint8_t *bytes = data;
	for (size_t i = 0; i < size; i += 4) {
		char byte0 = bytes[i], byte1 = bytes[i + 1];
		bytes[i] = bytes[i + 3];
		bytes[i + 1] = bytes[i + 2];
		bytes[i + 2] = byte1;
		bytes[i + 3] = byte0;
	}
}
#endif

struct model *
model_from_file(const char *name)
{
	size_t vert_size, verts_size;
	struct model *model;
	uint32_t format;
	char *data;

	model = malloc(sizeof (*model));
	model->data = data = asset_read(name, MODEL, NULL);
	format = data[0] | data[1] << 8 | data[2] << 16 | data[3] << 24;
	model->flags = SG_OBJ_INDEXED;
	if (format == 1) {
		model->flags |= SG_OBJ_TEXTURED;
		vert_size = 8 * sizeof (float);
	} else {
		vert_size = 6 * sizeof (float);
	}
	model->num_vertices =
		data[4] | data[5] << 8 | data[6] << 16 | data[7] << 24;
	model->num_indices =
		data[8] | data[9] << 8 | data[10] << 16 | data[11] << 24;
	model->vertices = (float *)(data + 16);
	verts_size = model->num_vertices * vert_size;
#ifdef SWAP_DATA_ENDIANNESS
	swap_array_endianness(model->vertices, verts_size);
#endif
	model->indices = (uint8_t *)(data + 16 + verts_size);
#ifdef __psp__
	sceKernelDcacheWritebackInvalidateRange(model->vertices,
						verts_size + model->num_indices);
#endif
	return model;
}

void
free_model(struct model *model)
{
	asset_free(model->data);
	free(model);
}
