#include "render_state.h"

void alpha_blend_func(Vector4D* out, Vector4D* src_color, Vector4D* dest_color, Uint32 blend_func)
{
	float sf = src_color->w;
	float df = dest_color->w;
	switch (blend_func)
	{
	case ALPHA_BLEND_DEST_ZERO_SRC_ONE:
		sf = 1.0f;
		df = 0.0f;
		break;
	case ALPHA_BLEND_DEST_ONE_SRC_ZERO:
		sf = 0.0f;
		df = 1.0f;
		break;
	case ALPHA_BLEND_DEST_INV_SRC_INV:
		sf = 1.0f - sf;
		df = 1.0f - df;
		break;
	case ALPHA_BLEND_DEST_SRC_INV:
		sf = 1.0f - sf;
		break;
	case ALPHA_BLEND_DEST_INV_SRC:
		df = 1.0f - df;
		break;
	}
	__m128 src_factor = _mm_set_ps(sf, sf, sf, sf);
	__m128 src_rgb = _mm_mul_ps(_mm_loadu_ps((float*)src_color), src_factor);
	__m128 dest_factor = _mm_set_ps(df, df, df, df);
	__m128 dest_rgb = _mm_mul_ps(_mm_loadu_ps((float*)dest_color), dest_factor);
	__m128 rgb = _mm_add_ps(src_rgb, dest_rgb);
	_mm_storeu_ps((float*)out, rgb);
	clamp_f32(&out->x, src_color->x + dest_color->x, 0.0f, 1.0f);
	clamp_f32(&out->y, src_color->y + dest_color->y, 0.0f, 1.0f);
	clamp_f32(&out->z, src_color->z + dest_color->z, 0.0f, 1.0f);
	clamp_f32(&out->w, src_color->w + dest_color->w, 0.0f, 1.0f);
}