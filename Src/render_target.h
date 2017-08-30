#ifndef RENDER_TARGET_H_INCLUDED
#define RENDER_TARGET_H_INCLUDED

#include "SDL2/SDL.h"
#include "rasterize_tile.h"
#include "renderer_params.h"
#include "frame_buffer.h"

typedef enum _RenderTargetBufferType
{
    BUFFER_COLOR = 1,
    BUFFER_DEPTH = 2,
    BUFFER_STENCIL = 4,
}RTBufferType;

typedef struct _RenderTarget
{
    Uint32 width;
    Uint32 height;
    FrameBuffer* render_buffer;
    FrameBuffer* depth_buffer;
    FrameBuffer* stencil_buffer;
    Uint32 cur_tile;
    Uint32 tile_num;
    Rasterize_Tile render_tiles[MAX_RENDER_TILE_NUM];
}RenderTarget;


RenderTarget* create_render_target(Uint32 width, Uint32 height, BufferType type);

void destroy_render_target(RenderTarget* rt);

void* render_target_get_render_buffer_ptr(RenderTarget* rt);

void render_target_clean(RenderTarget* rt, Uint32 clean_flag, Uint32 iv, float fv, float depth, float stencil);

void* render_target_get_buffer_value(RenderTarget* rt, Uint32 x, Uint32 y, Uint32 buffer_type);


#endif // RENDER_TARGET_H_INCLUDED
