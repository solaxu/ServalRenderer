#pragma once

namespace Math2D
{
	class POINT
	{
	public:
		float x;
		float y;
		POINT() :x(0.0f), y(0.0f)
		{

		}
		POINT(float _x, float _y) : x(_x), y(_y)
		{

		}
	};
}