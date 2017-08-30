#include "pixel_processor.h"
#include "rasterizer.h"
#include <assert.h>

extern DWORD WINAPI rasterizer(LPVOID* renderer);

PixelProcessor* create_pixel_processor(ServalRenderer* renderer)
{
    PixelProcessor* pp = (PixelProcessor*)malloc(sizeof(PixelProcessor));
    assert(pp);
    memset(pp, 0, sizeof(PixelProcessor));

    pp->renderer = renderer;

    linked_hash_init(&pp->tid_begin_evt_map);
    linked_hash_init(&pp->tid_end_evt_map);

    int i = 0;
    for (i = 0; i < RASTERIZER_THREAD_NUM; ++i)
    {
        pp->thread_handles[i] = CreateThread(NULL, 0, rasterizer, (void*)renderer, CREATE_SUSPENDED, &pp->thread_handle_ids[i]);
        assert(pp->thread_handles[i]);

        pp->begin_evts[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        assert(pp->begin_evts[i]);
        linked_hash_insert(&pp->tid_begin_evt_map, pp->thread_handle_ids[i], pp->begin_evts[i]);

        pp->end_evts[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        assert(pp->end_evts[i]);
        linked_hash_insert(&pp->tid_end_evt_map, pp->thread_handle_ids[i], pp->end_evts[i]);
    }

    InitializeCriticalSection(&pp->cs);

    return pp;
}

void destroy_pixel_processor(PixelProcessor* pp)
{
    if (pp)
    {
        int i = 0;
        for (i = 0; i < RASTERIZER_THREAD_NUM; ++i)
        {
            CloseHandle(pp->begin_evts[i]);
            CloseHandle(pp->end_evts[i]);
            CloseHandle(pp->thread_handles[i]);
        }
        DeleteCriticalSection(&pp->cs);

        linked_hash_destroy(&pp->tid_end_evt_map, NULL);
        linked_hash_destroy(&pp->tid_begin_evt_map, NULL);
    }
}

void ready_pixel_process(PixelProcessor* pp)
{
    int i = 0;
    for (i = 0; i < RASTERIZER_THREAD_NUM; ++i)
    {
        ResumeThread(pp->thread_handles[i]);
    }
}

void begin_pixel_process(PixelProcessor* pp)
{
    EnterCriticalSection(&pp->cs);
    pp->tile_index = 0;
    LeaveCriticalSection(&pp->cs);
    // begin rasterization
    int i = 0;

    for (i = 0; i < RASTERIZER_THREAD_NUM; ++i)
    {
        SetEvent(pp->begin_evts[i]);
    }
}

void end_pixel_process(PixelProcessor* pp)
{
    // wait for all thread end rasterization processes
    WaitForMultipleObjects(RASTERIZER_THREAD_NUM, pp->end_evts, TRUE, INFINITE);
    // turn on end switches
    int i = 0;
    for (i = 0; i < RASTERIZER_THREAD_NUM; ++i)
    {
        ResetEvent(pp->end_evts[i]);
    }
}
