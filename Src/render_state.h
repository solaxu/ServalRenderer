#ifndef RENDER_STATE_H_INCLUDED
#define RENDER_STATE_H_INCLUDED

#include "render_target.h"
#include <stdbool.h>
#include "SDL2/SDL.h"

#include "smath.h"

typedef enum _CompareFunc
{
	CMP_LESS = 0,
	CMP_GREAT,
	CMP_LESS_EQUAL,
	CMP_GREAT_EQUAL,
	CMP_ALWAYS
}COMPARE_FUNC;

typedef enum _AlpahBlendFunc
{
	ALPHA_BLEND_DEST_INV_SRC,
	ALPHA_BLEND_DEST_SRC_INV,
	ALPHA_BLEND_DEST_INV_SRC_INV,
	ALPHA_BLEND_DEST_SRC,
	ALPHA_BLEND_DEST_ONE_SRC_ZERO,
	ALPHA_BLEND_DEST_ZERO_SRC_ONE,
};

typedef struct _RenderState
{
	bool enable_alpha_test;
	bool enable_z_test;
	bool enable_alpha_blend;
	bool enable_lighting;
	bool enable_specular;
	bool enable_texture;
	Uint32 alpha_test_func;
	Uint32 z_test_func;
	Uint32 alpha_func;
}RenderState;

void alpha_blend_func(Vector4D* out, Vector4D* c1, Vector4D* c2, Uint32 blend_func);

#endif // RENDER_STATE_H_INCLUDED
