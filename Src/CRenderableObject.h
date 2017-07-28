#pragma once

#include "SLG_header.h"
#include "CRasterizeObject.h"

class CRasterizeObject;

class CRenderableObject
{
public:
	CRenderableObject();
	virtual ~CRenderableObject();

	int GetIndex();
	
protected:
	int index = 0;
	int type = PRIMITIVE_TYPE::None;
};