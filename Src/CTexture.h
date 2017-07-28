#pragma once

#include "SLG_header.h"

enum ETexture
{
	Texture_0,
	Texture_1,
	Texture_2,
	Texture_3,
	Texture_4,
	Texture_5,
	Texture_6,
	Texture_7
};

class CTexture
{
protected:
	int width;
	int height;
	Uint32 type;
};