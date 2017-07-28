#include "CRenderer.h"

// Rasterization process

void CRenderer::Rec_HET(int left, int top, int size, void* back_buffer)
{

}

DWORD WINAPI CRenderer::RasterizationProc(LPVOID param)
{
	CRenderer* renderer = (CRenderer*)param;

	auto tid = GetCurrentThreadId();
	auto handle = GetCurrentThread();
	auto begin_evt = *renderer->tid_begin_evt_map.GetValuePtr(tid);
	auto end_evt = *renderer->tid_end_evt_map.GetValuePtr(tid);
	while (true)
	{
//		printf("Thread: Wait for begin\n");
		WAIT_FOR_SINGLE_OBJ(begin_evt, INFI_TIME);
//		printf("Thread: Begins\n");
		auto render_target = renderer->GetRenderTarget();
		// lock task queue
		EnterCriticalSection(&renderer->task_mutex);
		int size = renderer->rasterizer_tiles_unprocessed.size();
//		printf("Remain tile: %d.\n", size);
		// if there is no task, the threads are blocked
		if (size == 0)
		{
			// thread block itself and waiting for main thread informs a new frame
			SET_THREAD_EVENT(end_evt);
			RESET_THREAD_EVENT(begin_evt);
//			printf("Thread: Block Self\n");
			LeaveCriticalSection(&renderer->task_mutex);
			//SuspendThread(handle);
			continue;
		}

		// wait to get a tile from unprocessed tasks to fill
		CRasterizeTile* tile = renderer->rasterizer_tiles_unprocessed.front();
		renderer->rasterizer_tiles_unprocessed.pop();
		// unlock task queue
		LeaveCriticalSection(&renderer->task_mutex);

		// do rasterization
		// for each pixel in tile, load pixel shader and execute
		int x = tile->GetLeft();
		int y = tile->GetTop();
		for (int i = 0; i < CRasterizeTile::Tile_Size; ++i)
		{
			for (int j = 0; j < CRasterizeTile::Tile_Size; ++j)
			{
				int px = x + i;
				int py = y + j;
				int index = px + py * render_target->GetWidth();
				// load pixel shader and execute
				if (render_target->GetType() == CRenderTarget::U32)
				{
					Uint32* ptr = (Uint32*)render_target->GetData();
					ptr[index] = size * 100;// ARGB32(255, 255, 0, 0);
				}
				else if (render_target->GetType() == CRenderTarget::FLOAT32)
				{
					float* ptr = (float*)render_target->GetData();
					ptr[index] = 1.0f;
				}
			}
		}

		// lock task queue
		EnterCriticalSection(&renderer->task_mutex);
		renderer->rasterizer_tiles_processed.push(tile);
		// unlock task queue
		LeaveCriticalSection(&renderer->task_mutex);
	}
}

CRenderer::CRenderer():vertex_desc(nullptr),cur_primitive_type(PRIMITIVE_TYPE::None),cur_target(nullptr),
	sdl_env(nullptr), surface(nullptr)
{
	for (int i = 0; i < CRenderer::STREAM_REG_NUM; ++i)
	{
		this->vertex_buffers[i] = nullptr;
	}

	for (int i = 0; i < CRenderer::TEXTURE_REG_NUM; ++i)
	{
		this->textures[i] = nullptr;
	}
}

CRenderer::~CRenderer()
{
	for (int i = 0; i < CRenderer::Rasterize_Threads_Num; ++i)
	{
		CLOSE_THREAD_HANDLE(threads[i]);
		CLOSE_EVENT_HANDLE(end_evts[i]);
		CLOSE_EVENT_HANDLE(begin_evts[i]);
	}

	DeleteCriticalSection(&task_mutex);

	SDL_FreeSurface(this->surface);
}

CRenderTarget* CRenderer::CreateRenderTarget(Uint32 index, int width, int height, int type)
{
	CRenderTarget* target = new CRenderTarget();
	target->Create(width, height, type);
	this->targets.insert(std::pair<Uint32, CRenderTarget*>(index, target));
	return target;
}

void CRenderer::Create(CSDLEnv* sdl_env)
{
	assert(sdl_env != nullptr);
	this->sdl_env = sdl_env;
	auto main_target = CreateRenderTarget(MAIN_TARGET, sdl_env->GetWidth(), sdl_env->GetHeight(), CRenderTarget::U32);
	this->SetRenderTarget(main_target);

	for (int i = 0; i < CRenderer::Rasterize_Threads_Num; ++i)
	{
		this->threads[i] = CREATE_THREAD(NULL, 0, CRenderer::RasterizationProc, this, CREATE_SUSPENDED, &this->tids[i]);
		assert(this->threads[i] != nullptr);
		this->begin_evts[i] = CREATE_EVENT(NULL, TRUE, FALSE, NULL);
		assert(this->begin_evts[i] != nullptr);
		this->end_evts[i] = CREATE_EVENT(NULL, TRUE, FALSE, NULL);
		assert(this->end_evts[i] != nullptr);
		this->tid_begin_evt_map.Insert(this->tids[i], this->begin_evts[i]);
		this->tid_end_evt_map.Insert(this->tids[i], this->end_evts[i]);
	}

	InitializeCriticalSection(&this->task_mutex);
// 	this->task_mutex = CRETAE_MUTEX(NULL, FALSE, NULL);
//	assert(this->task_mutex != nullptr);
	this->surface = SDL_CreateRGBSurface(0, sdl_env->GetWidth(), sdl_env->GetHeight(), 32, 0, 0, 0, 0);
	assert(this->surface != nullptr);
}

CRenderTarget* CRenderer::GetRenderTarget()
{
	return this->cur_target;
}

void CRenderer::SetRenderTarget(CRenderTarget* render_target)
{
	assert(render_target != nullptr);
	while (this->rasterizer_tiles_processed.size() > 0)
		this->rasterizer_tiles_processed.pop();
	while (this->rasterizer_tiles_unprocessed.size() > 0)
		this->rasterizer_tiles_unprocessed.pop();
	this->cur_target = render_target;
	auto tiles = render_target->GetTiles();
	for (int i = 0; i < render_target->GetTileNum(); ++i)
		this->rasterizer_tiles_processed.push(&tiles[i]);
}

void CRenderer::SetTexture(int texture_reg, CTexture* texture)
{
	assert(texture != nullptr);
	this->textures[texture_reg] = texture;
}

void CRenderer::SetVertexBuffer(int stream_reg, CVertexArray* vb)
{
	assert(vb != nullptr);
	this->vertex_buffers[stream_reg] = vb;
}

void CRenderer::SetPrimitiveType(int primitive_type)
{
	this->cur_primitive_type = primitive_type;
}

void CRenderer::SetVertexDecl(CVertexUsageElement* decl, int decl_size)
{
	memcpy(this->vertex_desc, decl, decl_size);
}

void CRenderer::Clean()
{
	assert(this->cur_target != nullptr);
	this->cur_target->Clean();
}

void CRenderer::ThreadReady()
{
	for (int i = 0; i < CRenderer::Rasterize_Threads_Num; ++i)
		WAKE_UP_THREAD(this->threads[i]);
}

void CRenderer::Begin()
{
	EnterCriticalSection(&this->task_mutex);
	while (this->rasterizer_tiles_processed.size() > 0)
	{
		auto tile = this->rasterizer_tiles_processed.front();
		this->rasterizer_tiles_processed.pop();
		this->rasterizer_tiles_unprocessed.push(tile);
	}
	LeaveCriticalSection(&this->task_mutex);
//	printf("-------------------Begin Threads\n");
	for (int i = 0; i < CRenderer::Rasterize_Threads_Num; ++i)
		SetEvent(this->begin_evts[i]);
//	ThreadReady();
}

void CRenderer::End()
{
//	printf("-------------------Wait for End Events\n");
	WAIT_FOR_MULTI_OBJ(CRenderer::Rasterize_Threads_Num, this->end_evts.data(), TRUE, INFI_TIME);
//	printf("-------------------Four Threads End\n");
	for (int i = 0; i < CRenderer::Rasterize_Threads_Num; ++i)
		ResetEvent(this->end_evts[i]);
}

void CRenderer::Present()
{
	static int frame_num = 0;
	SDL_LockSurface(this->surface);
 	memcpy(this->surface->pixels, this->cur_target->GetData(), 
 		sizeof(Uint32) * this->cur_target->GetWidth()  * this->cur_target->GetHeight());
	SDL_UnlockSurface(this->surface);
	SDL_BlitSurface(this->surface, NULL, this->sdl_env->GetWindowSurface(), NULL);
	SDL_UpdateWindowSurface(this->sdl_env->GetWindow()); //SDL_RenderPresent(sdl_env->GetSDLRenderer());

	printf("Frame Count: %d\n", ++frame_num);
}