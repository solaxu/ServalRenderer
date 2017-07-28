#include "SLG_header.h"
#include "CRenderTarget.h"

CRenderTarget::CRenderTarget():width(0), height(0), frame_buffer(nullptr), 
	tile_row(0), tile_col(0)
{

}

CRenderTarget::~CRenderTarget()
{
	if (frame_buffer != nullptr)
	{
		delete[] frame_buffer;
		frame_buffer = nullptr;
	}
}

void CRenderTarget::Create(int width, int height, int type)
{
	this->width = width;
	this->height = height;
	this->type = type;
	if (type == CRenderTarget::U32)
		this->frame_buffer = new Uint8[sizeof(Uint32) * width * height];
	else if (type == CRenderTarget::FLOAT32)
		this->frame_buffer = new Uint8[sizeof(float) * width * height];

	this->depth_buffer = new float[width * height];
	this->tile_col = this->width / CRasterizeTile::Tile_Size;
	this->tile_row = this->height / CRasterizeTile::Tile_Size;

	rasterize_tile = new CRasterizeTile[tile_col * tile_row];

	int x = 0;
	int y = 0;

	for (int i = 0; i < this->tile_row; i++)
	{
		for (int j = 0; j < this->tile_col; j++)
		{
			CRasterizeTile* tile = &this->rasterize_tile[j + i * this->tile_col];
			tile->SetLeft(x);
			tile->SetTop(y);
			tile->SetBottom(y + CRasterizeTile::Tile_Size - 1);
			tile->SetRight(x + CRasterizeTile::Tile_Size - 1);
			x += CRasterizeTile::Tile_Size;
		}
		x = 0;
		y += CRasterizeTile::Tile_Size;
	}
}

CRasterizeTile* CRenderTarget::GetTiles()
{
	return this->rasterize_tile;
}

int CRenderTarget::GetTileNum()
{
	return this->tile_col * this->tile_row;
}

void CRenderTarget::Destroy()
{
	if (frame_buffer != nullptr)
	{
		delete[] frame_buffer;
		frame_buffer = nullptr;
	}
}

int CRenderTarget::GetHeight()
{
	return this->height;
}

int CRenderTarget::GetWidth()
{
	return this->width;
}

int CRenderTarget::GetType()
{
	return this->type;
}

Uint8* CRenderTarget::GetData()
{
	return this->frame_buffer;
}

float* CRenderTarget::GetDepthBuffer()
{
	return this->depth_buffer;
}

void CRenderTarget::Clean()
{
	if (this->GetType() == CRenderTarget::FLOAT32)
		memset(this->frame_buffer, 0, sizeof(float) * this->width * this->height);
	else if (this->GetType() == CRenderTarget::U32)
		memset(this->frame_buffer, 0, sizeof(Uint32) * this->width * this->height);
	memset(this->depth_buffer, 0, sizeof(float) * this->width * this->height);
}
