#include "CRenderer.h"
#include <ctime>

int main(int argc, char* args[])
{
	CSDLEnv* sdl_env = new CSDLEnv();
	sdl_env->Create(640, 480, 32);
	CRenderer* renderer = new CRenderer();
	renderer->Create(sdl_env);
	SDL_Surface* gHelloWorld = NULL;
	gHelloWorld = SDL_LoadBMP("test.bmp");
	renderer->ThreadReady();
	bool quit = false;
	SDL_Event e;
	while (!quit)
	{
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
		}
		clock_t s__ = clock();
		renderer->Begin();
		renderer->End();
		SDL_BlitSurface(gHelloWorld, NULL, sdl_env->GetWindowSurface(), NULL);
		renderer->Present();
		clock_t e__ = clock();

		printf("Frame time %ld\n", e__ - s__);
	}
	return 0;
}