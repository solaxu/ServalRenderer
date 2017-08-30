#ifndef SAMPLER_H_INCLUDE
#define SAMPLER_H_INCLUDE

#include "renderer_params.h"
#include <stdbool.h>

typedef struct _STexture
{
	Uint32 width;
	Uint32 height;
	Uint32 data_size;
	Uint32* data;
}STexture;

void load_texture_from_png(STexture* texture, const char* pngfile);

typedef enum _SampleType
{
	NEAREST_POINT,
	DOUBLE_LINEAR,
}ESampleType;

typedef struct _Sampler
{
	STexture* texture;
	Uint32 sampler_type;
	bool enable;
}SSampler;

Uint32 sample_texture2D(SSampler* sampler, float u, float v, Uint32 sample_type);

#endif // SAMPLER_H_INCLUDE

