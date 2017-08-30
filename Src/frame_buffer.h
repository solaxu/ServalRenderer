#ifndef SURFACE_H_INCLUDED
#define SURFACE_H_INCLUDED

#include "SDL2/SDL.h"

typedef enum _BufferType
{
    U32,
    F32
}BufferType;

typedef struct _FrameBuffer
{
    Uint32 width;
    Uint32 height;
    void* data;
    Uint32 buffer_type;
}FrameBuffer;

FrameBuffer* create_frame_buffer(Uint32 width, Uint32 height, BufferType type);

void set_u32_data(FrameBuffer* frame_buffer, Uint32 x, Uint32 y, Uint32 v);

void set_f32_data(FrameBuffer* frame_buffer, Uint32 x, Uint32 y, float v);

void destroy_frame_buffer(FrameBuffer* frame_buffer);

void fill_rect_u32(FrameBuffer* frame_buffer, Uint32 v, Uint32 left, Uint32 right, Uint32 top, Uint32 bottom);

void fill_rect_f32(FrameBuffer* frame_buffer, float v, Uint32 left, Uint32 right, Uint32 top, Uint32 bottom);

#endif // SURFACE_H_INCLUDED
