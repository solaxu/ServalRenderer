#pragma once

#include "SLG_header.h"

class CVertexUsageElement
{
public:
	Uint32 stride;	// vertex size
	Uint32 offset;	// usage
	Uint32 usage;	// where to begin
	Uint32 type;	// type
	Uint32 stream_index; // stream index
};

enum EVertexUsageEnum
{
	Position = 0,
	Normal = 1,
	TextureCoord = 2,
	Tangent = 3,
	Color = 4
};

#define VERTEX_ELEMENT_BEGIN {
#define VERTEX_USAGE_ELEMENT(stream_index,stride, offset, usage, type) {stream_index,stride, offset, usage, type}
#define VERTEX_ELEMENT_END }