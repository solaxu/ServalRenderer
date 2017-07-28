#pragma once

#include "SLG_header.h"
#include "CRenderableObject.h"

class CRenderableObject;

class CRasterizeObject
{
protected:
	// where from
	CRenderableObject* render_obj = nullptr;
	int type = PRIMITIVE_TYPE::None;
public:
	CRasterizeObject() : render_obj(nullptr), type(PRIMITIVE_TYPE::None)
	{

	}

	virtual ~CRasterizeObject()
	{

	}

	CRenderableObject* GetObjFrom()
	{
		return this->render_obj;
	}
};