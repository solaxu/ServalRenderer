#pragma once

#include "SLG_header.h"

class CSDLEnv
{
protected:
	int win_width = 0;
	int win_height = 0;
	int win_pixel_bits_width = 0;
	SDL_Window* win = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Surface* win_surface = nullptr;
public:
	CSDLEnv();
	virtual ~CSDLEnv();

	int GetWidth();
	int GetHeight();
	int GetPixelBitsWidth();
	SDL_Window* GetWindow();
	SDL_Renderer* GetSDLRenderer();
	SDL_Surface* GetWindowSurface();

	virtual bool Create(int width, int height, int pixel_bits_width);

	virtual void Destroy();
};