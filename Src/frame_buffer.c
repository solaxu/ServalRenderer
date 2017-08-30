#include "frame_buffer.h"
#include "smath.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include "renderer_params.h"

FrameBuffer* create_frame_buffer(Uint32 width, Uint32 height, BufferType type)
{
    if (width%RASTERIZE_TILE_SIZE != 0 || height%RASTERIZE_TILE_SIZE != 0)
    {
        printf("The width and height of the surface should be multiple of %d.\n", RASTERIZE_TILE_SIZE);
        return NULL;
    }

    FrameBuffer* buffer = (FrameBuffer*)malloc(sizeof(FrameBuffer));
    assert(buffer);
    memset(buffer, 0, sizeof(FrameBuffer));

    if (type == U32)
    {
        buffer->data = malloc(width * height * sizeof(Uint32));
        buffer->buffer_type = U32;
    }
    else if (type == F32)
    {
        buffer->data = malloc(width * height * sizeof(float));
        buffer->buffer_type = F32;
    }

    buffer->width = width;
    buffer->height = height;

    return buffer;
}

void set_u32_data(FrameBuffer* frame_buffer, Uint32 x, Uint32 y, Uint32 v)
{
    Uint32* ptr = (Uint32*)frame_buffer->data;
    ptr[y * frame_buffer->width + x] = v;
}

void set_f32_data(FrameBuffer* frame_buffer, Uint32 x, Uint32 y, float v)
{
    float* ptr = (float*)frame_buffer->data;
    ptr[y * frame_buffer->width + x] = v;
}

void destroy_frame_buffer(FrameBuffer* frame_buffer)
{
    if (frame_buffer)
    {
        free(frame_buffer->data);
    }
}

void fill_rect_u32(FrameBuffer* frame_buffer, Uint32 v, Uint32 left, Uint32 right, Uint32 top, Uint32 bottom)
{
    int i = 0, j = 0;
    Uint32* ptr = (Uint32*)frame_buffer->data;
    Uint32* dest = ptr + top * frame_buffer->width + left;
    for (i = top; i <= bottom; ++i)
    {
        for (j = 0; j <= right - left; ++j)
        {
            *(dest + j) = v;
        }
        dest += frame_buffer->width;
    }
}

void fill_rect_f32(FrameBuffer* frame_buffer, float v, Uint32 left, Uint32 right, Uint32 top, Uint32 bottom)
{
    int i = 0, j = 0;
    // can do optimization here
    float* ptr = (float*)frame_buffer->data;
    float* dest = ptr + top * frame_buffer->width + left;
	for (i = top; i <= bottom; ++i)
	{
		for (j = 0; j <= right - left; ++j)
		{
			*(dest + j) = v;
		}
		dest += frame_buffer->width;
	}
}
