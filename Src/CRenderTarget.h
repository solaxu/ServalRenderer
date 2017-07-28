#pragma once

#include "SLG_header.h"
#include "CRasterizeTile.h"

class CRenderTarget
{
public:
	enum
	{
		U32,
		FLOAT32,
	};
protected:
	int width = 0;
	int height = 0;
	int tile_col = 0;
	int tile_row = 0;
	Uint8* frame_buffer = nullptr;
	float* depth_buffer = nullptr;
	int type = CRenderTarget::U32;
	CRasterizeTile* rasterize_tile = nullptr;
public:
	CRenderTarget();
	virtual ~CRenderTarget();

	int GetWidth();
	int GetHeight();
	int GetType();
	CRasterizeTile* GetTiles();
	int GetTileNum();
	Uint8* GetData();
	float* GetDepthBuffer();

	virtual void Create(int width, int height, int type);
	virtual void Destroy();

	virtual void Clean();
};