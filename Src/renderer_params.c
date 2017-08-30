#include "renderer_params.h"
#include <limits.h>

void get_vertex_decl_attribute(VertexDeclaration* decl, Uint32 semantic, Uint32* offset, Uint32* stride)
{
	if (semantic == DECL_NONE)
		return;
	while (decl->semantic != DECL_NONE)
	{
		if (decl->semantic == semantic)
		{
			if (offset)
			{
				*offset = decl->offset;
			}
			if (stride)
			{
				*stride = decl->stride;
			}
			return;
		}
		++decl;
	}
	if (offset)
	{
		*offset = INT_MAX;
	}
	if (stride)
	{
		*stride = INT_MAX;
	}
}