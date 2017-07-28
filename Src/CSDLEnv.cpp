#include "CSDLEnv.h"

CSDLEnv::CSDLEnv():win_width(0), win_height(0), win_pixel_bits_width(32), 
	win(nullptr), renderer(nullptr), win_surface(nullptr)
{

}

CSDLEnv::~CSDLEnv()
{

}

int CSDLEnv::GetWidth()
{
	return win_width;
}

int CSDLEnv::GetHeight()
{
	return win_height;
}

int CSDLEnv::GetPixelBitsWidth()
{
	return win_pixel_bits_width;
}

SDL_Renderer* CSDLEnv::GetSDLRenderer()
{
	return renderer;
}

SDL_Window* CSDLEnv::GetWindow()
{
	return win;
}

SDL_Surface* CSDLEnv::GetWindowSurface()
{
	return win_surface;
}

bool CSDLEnv::Create(int width, int height, int pixel_bits_width)
{
	assert(pixel_bits_width == win_pixel_bits_width);

	auto code = SDL_Init(SDL_INIT_VIDEO);

	if (code != 0)
	{
		printf("SDL_Init Error!\n%s\n", SDL_GetError());
		return false;
	}

	this->win = SDL_CreateWindow("Serval SLG", 100, 100, width, height, SDL_WINDOW_SHOWN);

	if (this->win == nullptr)
	{
		printf("SDL_CreateWindow Error!\n%s\n", SDL_GetError());
		SDL_Quit();
		return false;
	}

	this->renderer = SDL_CreateRenderer(this->win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (this->renderer == nullptr)
	{
		printf("SDL_CreateRenderer Error!\n%s\n", SDL_GetError());
		SDL_DestroyWindow(this->win);
		SDL_Quit();
		return false;
	}

	this->win_width = width;
	this->win_height = height;
	this->win_pixel_bits_width = pixel_bits_width;

	this->win_surface = SDL_GetWindowSurface(this->win);

	return true;
}

void CSDLEnv::Destroy()
{
	if (this->renderer)
		SDL_DestroyRenderer(this->renderer);
	if (this->win)
		SDL_DestroyWindow(this->win);
	SDL_Quit();
}