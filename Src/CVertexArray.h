#pragma once

#include "CVertexDescription.h"

enum EVertexStream
{
	STREAM_0,
	STREAM_1,
	STREAM_2,
	STREAM_3,
	STREAM_4,
	STREAM_5,
	STREAM_6,
	STREAM_7
};

class CVertexArray
{
protected:
	int vertex_num = 0;
	int stride = 0;
	Uint8* buffer_ptr = nullptr;
public:

	CVertexArray();
	virtual ~CVertexArray();

	int GetVertexStride();
	int GetVertexNum();
	Uint8* GetData(); 
	int GetStride();
	void Create(int vertex_num, int stride);
	void FillData(Uint8* src_data, int vertex_num, int offset);
	void Destroy();
};