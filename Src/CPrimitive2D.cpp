#pragma once

#include "CPrimitive2D.h"

CPrimitive2D_Point::CPrimitive2D_Point() : x(0), y(0), color(0)
{
	this->type = PRIMITIVE_TYPE::Point;
}

CPrimitive2D_Point::~CPrimitive2D_Point()
{

}

void CPrimitive2D_Point::SetX(int px)
{
	this->x = px;
}

void CPrimitive2D_Point::SetY(int py)
{
	this->y = py;
}

void CPrimitive2D_Point::SetColor(Uint32 color)
{
	this->color = color;
}

Uint32 CPrimitive2D_Point::GetColor()
{
	return this->color;
}

float CPrimitive2D_Point::GetX()
{
	return this->x;
}

float CPrimitive2D_Point::GetY()
{
	return this->y;
}

//////////////////////////////////////////////////////////////////////////
// Line

CPrimitive2D_Line::CPrimitive2D_Line():start_pt(), end_pt(), start_pt_color(0xffffffff), end_pt_color(0xffffffff)
{

}

CPrimitive2D_Line::~CPrimitive2D_Line()
{

}

const Math2D::POINT& CPrimitive2D_Line::GetStartPoint()
{
	return this->start_pt;
}

const Math2D::POINT& CPrimitive2D_Line::GetEndPoint()
{
	return this->end_pt;
}

void CPrimitive2D_Line::SetSatrtPoint(float x, float y)
{
	this->start_pt.x = x;
	this->start_pt.y = y;
}

void CPrimitive2D_Line::SetEndPoint(float x, float y)
{
	this->end_pt.x = x;
	this->end_pt.y = y;
}

Uint32 CPrimitive2D_Line::GetStartPointColor()
{
	return this->start_pt_color;
}

Uint32 CPrimitive2D_Line::GetEndPointColor()
{
	return this->end_pt_color;
}

void CPrimitive2D_Line::SetEndPointColor(Uint32 color)
{
	this->end_pt_color = color;
}

void CPrimitive2D_Line::SetStartPointColor(Uint32 color)
{
	this->start_pt_color = color;
}

void CPrimitive2D_Line::AddUV(float start_u, float start_v, float end_u, float end_v)
{
	this->uvs.push_back(Math2D::POINT(start_u, start_v));
	this->uvs.push_back(Math2D::POINT(end_u, end_v));
}

const Math2D::POINT& CPrimitive2D_Line::GetStartPointUV(int index)
{
	index = 2 * index;
	return uvs[index];
}

const Math2D::POINT& CPrimitive2D_Line::GetEndPointUV(int index)
{
	index = 2 * index - 1;
	return uvs[index];
}

//////////////////////////////////////////////////////////////////////////
// Polygon
//

CPrimitive2D_Polygon::CPrimitive2D_Polygon(): uv_layer_num(0)
{
	this->type = PRIMITIVE_TYPE::Polygon2D;
}

CPrimitive2D_Polygon::~CPrimitive2D_Polygon()
{

}

Uint32 CPrimitive2D_Polygon::GetPointNum()
{
	return this->points.size();
}

const Math2D::POINT& CPrimitive2D_Polygon::GetPoint(int index)
{
	return this->points[index];
}

const Math2D::POINT& CPrimitive2D_Polygon::GetPointUV(int pt_index, int uv_index)
{
	int index = pt_index * this->uv_layer_num + uv_index;
	return this->uvs[index];
}

void CPrimitive2D_Polygon::SetUVLayerNum(int layer_num)
{
	this->uv_layer_num = layer_num;
}

void CPrimitive2D_Polygon::AddColor(Uint32 color)
{
	this->color.push_back(color);
}

void CPrimitive2D_Polygon::AddPoint(float x, float y)
{
	this->points.push_back(Math2D::POINT(x, y));
}

void CPrimitive2D_Polygon::AddPointIndex(int index)
{
	this->indices.push_back(index);
}

void CPrimitive2D_Polygon::AddUV(float u, float v)
{
	this->uvs.push_back(Math2D::POINT(u, v));
}

int CPrimitive2D_Polygon::GetUVLayerNum()
{
	return this->uv_layer_num;
}