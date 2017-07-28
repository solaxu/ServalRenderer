#include "CVertexArray.h"

CVertexArray::CVertexArray() : vertex_num(0),stride(0),buffer_ptr(nullptr)
{

}

CVertexArray::~CVertexArray()
{
	if (this->buffer_ptr != nullptr)
		delete[] this->buffer_ptr;
}

int CVertexArray::GetVertexNum()
{
	return this->vertex_num;
}

int CVertexArray::GetVertexStride()
{
	return this->stride;
}

void CVertexArray::Create(int vertex_num, int stride)
{
	this->buffer_ptr = new Uint8[vertex_num * stride];
	this->stride = stride;
}

void CVertexArray::FillData(Uint8* src_data, int vertex_num, int offset)
{
	Uint8* dest = this->buffer_ptr + offset;
	Uint32 data_size = vertex_num * this->stride;
	memcpy(dest, src_data, offset);
}

Uint8* CVertexArray::GetData()
{
	return this->buffer_ptr;
}

int CVertexArray::GetStride()
{
	return this->stride;
}

void CVertexArray::Destroy()
{
	if (this->buffer_ptr != nullptr)
	{
		delete[] this->buffer_ptr;
		this->buffer_ptr = nullptr;
	}
}