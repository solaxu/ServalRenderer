#include "CRasterizeTile.h"

int CRasterizeTile::GetTop()
{
	return this->top;
}

int CRasterizeTile::GetRight()
{
	return this->right;
}

int CRasterizeTile::GetLeft()
{
	return this->left;
}

int CRasterizeTile::GetBottom()
{
	return this->bottom;
}

std::vector<Uint32>& CRasterizeTile::GetDisplayIndices()
{
	return this->indices;
}

std::vector<bool>& CRasterizeTile::GetOverlapped()
{
	return this->overlapped_tile;
}

int CRasterizeTile::GetPrimitive()
{
	return primitive_num;
}

void CRasterizeTile::SetBottom(int bottom)
{
	this->bottom = bottom;
}

void CRasterizeTile::SetLeft(int left)
{
	this->left = left;
}

void CRasterizeTile::SetRight(int right)
{
	this->right = right;
}

void CRasterizeTile::SetTop(int top)
{
	this->top = top;
}