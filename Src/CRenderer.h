#pragma once

#include "CVertexDescription.h"
#include "CVertexArray.h"
#include "CTexture.h"
#include "CRenderTarget.h"
#include "CSDLEnv.h"
#include "LinkedHash.h"
#include "DoubleLinkedList.h"
#include <unordered_map>


class CRenderer
{
public:
	static const int TEXTURE_REG_NUM = 8;
	static const int STREAM_REG_NUM = 8;
	static const int MAIN_TARGET = 0;
protected:
	class CRenderState
	{
	public:
		Uint32 depth_test_func;
		Uint32 alpha_test_func;
		Uint32 stencil_test_func;
		Uint32 fill_mode;
		bool depth_write_enable;
		bool depth_cmp_enable;
		bool alpha_enable;
		bool stencil_enable;
	};

	class CTextureSamplerState
	{
	public:
		Uint32 max_filter;
		Uint32 min_filter;
		Uint32 mip_filter;
		bool enable_mip;
	};

	CRenderState prev_state;
	CRenderState  cur_state;

	CTextureSamplerState tex_sampler_state;

	CVertexArray* vertex_buffers[STREAM_REG_NUM];
	CTexture* textures[CRenderer::TEXTURE_REG_NUM];

	CVertexUsageElement* vertex_desc;

	int cur_primitive_type;

	CRenderTarget* cur_target;

	std::unordered_map<Uint32, CRenderTarget*> targets;

	SDL_Surface* surface;

	CSDLEnv* sdl_env;

public:
	CRenderer();
	virtual ~CRenderer();

	void SetTexture(int texture_reg, CTexture* texture);
	void SetVertexBuffer(int stream_reg, CVertexArray* vb);
	void SetPrimitiveType(int primitive_type);
	void SetVertexDecl(CVertexUsageElement* decl, int decl_size);
	void SetRenderTarget(CRenderTarget* render_target);
	
	CRenderTarget* GetRenderTarget();

	void Create(CSDLEnv* sdl_env);

	CRenderTarget* CreateRenderTarget(Uint32 index, int width, int height, int type);

	void ThreadReady();

	void Clean();
	virtual void Begin();
	virtual void End();
	virtual void Present();
public:
	static const int Rasterize_Threads_Num = 4;

	// Fill tile, rasterization process
	static void Rec_HET(int left, int top, int size, void* back_buffer);

	static DWORD WINAPI RasterizationProc(LPVOID param);

	std::array<THREAD_HANDLE, CRenderer::Rasterize_Threads_Num> threads;
	std::array<THREAD_ID, CRenderer::Rasterize_Threads_Num> tids;
	std::array<THREAD_EVENT_HANDLE, CRenderer::Rasterize_Threads_Num> begin_evts;
	std::array<THREAD_EVENT_HANDLE, CRenderer::Rasterize_Threads_Num> end_evts;
	LinkedHash<THREAD_EVENT_HANDLE> tid_begin_evt_map;
	LinkedHash<THREAD_EVENT_HANDLE> tid_end_evt_map;
	// task queue
	std::queue<CRasterizeTile*> rasterizer_tiles_unprocessed;
	std::queue<CRasterizeTile*> rasterizer_tiles_processed;
	// mutex for task queue
	CRITICAL_SECTION task_mutex;
};