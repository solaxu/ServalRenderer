#include "render_target.h"
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdbool.h>

RenderTarget* create_render_target(Uint32 width, Uint32 height, BufferType type)
{
    RenderTarget* target = (RenderTarget*)malloc(sizeof(RenderTarget));
    assert(target != NULL);

    memset(target, 0, sizeof(RenderTarget));

    target->render_buffer = create_frame_buffer(width, height, type);
    assert(target->render_buffer);

    target->depth_buffer = create_frame_buffer(width, height, F32);
    assert(target->depth_buffer);

    target->stencil_buffer = create_frame_buffer(width, height, F32);
    assert(target->stencil_buffer);

	target->width = width;
	target->height = height;

    // fill render tiles
    Uint32 n = width / RASTERIZE_TILE_SIZE;
    Uint32 m = width - n * RASTERIZE_TILE_SIZE;
    Uint32 tile_col_num = (m == 0) ? n : (n + 1);

    n = height / RASTERIZE_TILE_SIZE;
    m = height - n * RASTERIZE_TILE_SIZE;
    Uint32 tile_row_num = (m == 0) ? n : (n + 1);

    target->tile_num = tile_row_num * tile_col_num;

    Uint32 left = 0, top = 0;

    for (n = 0; n < tile_row_num; ++n)
    {
        for (m = 0; m < tile_col_num; ++m)
        {
            Uint32 tile_index = tile_col_num * n + m;
            target->render_tiles[tile_index].left = left;
            target->render_tiles[tile_index].right = left + RASTERIZE_TILE_SIZE - 1;
            target->render_tiles[tile_index].top = top;
            target->render_tiles[tile_index].bottom = top + RASTERIZE_TILE_SIZE - 1;
			left += RASTERIZE_TILE_SIZE;
        }
		top += RASTERIZE_TILE_SIZE;
		left = 0;
    }

    return target;
}

void destroy_render_target(RenderTarget* rt)
{
    if (rt)
    {
        if (rt->render_buffer)
        {
            destroy_frame_buffer(rt->render_buffer);
            free(rt->render_buffer);
        }

        if (rt->depth_buffer)
        {
            destroy_frame_buffer(rt->depth_buffer);
            free(rt->depth_buffer);
        }

        if (rt->stencil_buffer)
        {
            destroy_frame_buffer(rt->stencil_buffer);
            free(rt->stencil_buffer);
        }
    }
}


void* render_target_get_render_buffer_ptr(RenderTarget* rt)
{
    if (rt && rt->render_buffer)
    {
        return rt->render_buffer->data;
    }
    return NULL;
}

void render_target_clean(RenderTarget* rt, Uint32 clean_flag, Uint32 iv, float fv, float depth, float stencil)
{
    if (rt == NULL)
        return;

    bool clean_color = (clean_flag & BUFFER_COLOR) != 0;
    bool clean_depth = (clean_flag & BUFFER_DEPTH) != 0;
    bool clean_stencil = (clean_flag & BUFFER_STENCIL) != 0;

    if (clean_color)
    {
        if (rt->render_buffer->buffer_type == U32)
            fill_rect_u32(rt->render_buffer, iv, 0, rt->width - 1, 0, rt->height - 1);
        else if (rt->render_buffer->buffer_type == F32)
            fill_rect_f32(rt->render_buffer, fv, 0, rt->width - 1, 0, rt->height - 1);
    }

    if (clean_depth)
    {
        fill_rect_f32(rt->depth_buffer, depth, 0, rt->width - 1, 0, rt->height - 1);
    }

    if (clean_stencil)
    {
        fill_rect_f32(rt->stencil_buffer, stencil, 0, rt->width - 1, 0, rt->height - 1);
    }
}

void* render_target_get_buffer_value(RenderTarget* rt, Uint32 x, Uint32 y, Uint32 buffer_type)
{
	Uint32* buffer = NULL;
	switch (buffer_type)
	{
	case BUFFER_COLOR:
		buffer = rt->render_buffer->data;
		break;
	case BUFFER_DEPTH:
		buffer = rt->depth_buffer->data;
		break;
	case BUFFER_STENCIL:
		buffer = rt->stencil_buffer->data;
		break;
	default:
		break;
	}
	return &buffer[y * rt->width + x];
}

