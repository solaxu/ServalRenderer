#pragma once

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <array>
#include <memory>
#include <queue>
#include <vector>

#include <SDL.h>
#include <SDL_render.h>
#include <SDL_surface.h>
#include <SDL_thread.h>

#include "ThreadMacro.h"

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2test.lib")

enum PRIMITIVE_TYPE
{
	None = 0,
	Point2D = 1,
	Line2D = 2,
	Polygon2D = 3,
	Point3D = 4,
	Line3D = 5,
	Triangle3D = 6,
	Advanced = 7
};


#define ARGB32(a,r,g,b) ( ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | ((b & 0xff) ))