#ifndef PIXEL_PROCESSOR_H_INCLUDED
#define PIXEL_PROCESSOR_H_INCLUDED

#include <windows.h>
#include "renderer_params.h"
#include "thread_pool.h"
#include "renderer.h"
#include "dslib/linked_hash.h"

typedef struct _Serval ServalRenderer;

typedef struct _PixelProcessor
{
    ServalRenderer* renderer;
    HANDLE thread_handles[RASTERIZER_THREAD_NUM];
    DWORD thread_handle_ids[RASTERIZER_THREAD_NUM];
    HANDLE begin_evts[RASTERIZER_THREAD_NUM];
    HANDLE end_evts[RASTERIZER_THREAD_NUM];
    CRITICAL_SECTION cs;
    LinkedHash tid_begin_evt_map;
    LinkedHash tid_end_evt_map;
    Uint32 tile_index;
}PixelProcessor;

PixelProcessor* create_pixel_processor(ServalRenderer* renderer);

void destroy_pixel_processor(PixelProcessor* pp);

void ready_pixel_process(PixelProcessor* pp);

void begin_pixel_process(PixelProcessor* pp);

void end_pixel_process(PixelProcessor* pp);

#endif // PIXEL_PROCESSOR_H_INCLUDED
