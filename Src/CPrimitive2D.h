#pragma once

#include "CRenderableObject.h"
#include "CMath2D.h"

class CPrimitive2D_Point : public CRenderableObject
{
protected:
	float x;
	float y;
	Uint32 color;
public:
	CPrimitive2D_Point();
	virtual ~CPrimitive2D_Point();

	void SetX(int px);
	void SetY(int py);
	void SetColor(Uint32 color);
	float GetX();
	float GetY();
	Uint32 GetColor();
};

//////////////////////////////////////////////////////////////////////////
// Line

class CPrimitive2D_Line : public CRenderableObject
{
protected:
	Math2D::POINT start_pt;
	Math2D::POINT end_pt;
	Uint32 start_pt_color = 0xffffffff; // the white color
	Uint32 end_pt_color = 0xffffffff; // the white color
	std::vector<Math2D::POINT> uvs; // how many UVs we used
public:
	CPrimitive2D_Line();
	virtual ~CPrimitive2D_Line();
	const Math2D::POINT& GetStartPoint();
	const Math2D::POINT& GetEndPoint();
	void SetSatrtPoint(float x, float y);
	void SetEndPoint(float x, float y);
	Uint32 GetStartPointColor();
	Uint32 GetEndPointColor();
	void SetStartPointColor(Uint32 color);
	void SetEndPointColor(Uint32 color);
	void AddUV(float start_u, float start_v, float end_u, float end_v);
	const Math2D::POINT& GetStartPointUV(int index);
	const Math2D::POINT& GetEndPointUV(int index);
};

//////////////////////////////////////////////////////////////////////////
// Polygon
// Actually, a polygon is a set of lines 

class CPrimitive2D_Polygon : public CRenderableObject
{
protected:
	std::vector<Math2D::POINT> points;
	std::vector<Math2D::POINT> uvs;
	std::vector<Uint32> color;
	std::vector<Uint32> indices;
	int uv_layer_num;
public:
	CPrimitive2D_Polygon();
	virtual ~CPrimitive2D_Polygon();

	const Math2D::POINT& GetPoint(int index);
	const Math2D::POINT& GetPointUV(int pt_index, int uv_index);
	int GetPointNum();

	void AddPoint(float x, float y);
	void AddColor(Uint32 color);
	void AddUV(float u, float v);
	void AddPointIndex(int index);

	int GetUVLayerNum();
	void SetUVLayerNum(int layer_num);
};