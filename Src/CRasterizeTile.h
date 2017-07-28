#pragma once

#include "SLG_header.h"

// cuz the renerer use indexed-vertex-buffer to draw primitives, so a rasterize tile includes the indices of primitives 

class CRasterizeTile
{
public:
	static const int Tile_Size = 32;

public:
	// draw list
	std::vector<Uint32> indices;
	int primitive_num = 0;
	std::vector<bool> overlapped_tile; // is tile are overlapped by primitives
	// tile rect
	int left;
	int top;
	int right;
	int bottom;

public:

	int GetLeft();
	int GetRight();
	int GetTop();
	int GetBottom();

	void SetLeft(int left);
	void SetRight(int right);
	void SetTop(int top);
	void SetBottom(int bottom);

	std::vector<Uint32>& GetDisplayIndices();
	std::vector<bool>& GetOverlapped();
	int GetPrimitive();

	void Render();
};