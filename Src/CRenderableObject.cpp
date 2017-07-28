#include "CRenderableObject.h"

CRenderableObject::CRenderableObject():index(-1), type(PRIMITIVE_TYPE::None)
{

}

CRenderableObject::~CRenderableObject()
{

}

int CRenderableObject::GetIndex()
{
	return this->index;
}