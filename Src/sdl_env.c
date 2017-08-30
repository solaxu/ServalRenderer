#include "sdl_env.h"
#include "render_target.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <stdbool.h>

SDL_Env* sdl_env_init(Uint32 x, Uint32 y, Uint32 w, Uint32 h, const char* win_title)
{
    // new sdl_env
    SDL_Env* sdl_env = (SDL_Env*)malloc(sizeof(SDL_Env));
    if (sdl_env == NULL)
    {
        printf("Out of Memory!\n");
        return NULL;
    }
    memset(sdl_env, 0, sizeof(SDL_Env));

    // init sdl
    if (-1 == SDL_Init(SDL_INIT_VIDEO))
    {
        printf("SDL_Init Error!\n%s\n", SDL_GetError());
        free(sdl_env);
        return NULL;
    }

    // create sdl_window
    sdl_env->window = SDL_CreateWindow(win_title, x, y, w, h, SDL_WINDOW_SHOWN);
    if (sdl_env->window == NULL)
    {
        printf("SDL_CreateWindow Error!\n%s\n", SDL_GetError());
        SDL_Quit();
        free(sdl_env);
        return NULL;
    }

    // create sdl_renderer
    Uint32 renderer_flag = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    sdl_env->sdl_renderer = SDL_CreateRenderer(sdl_env->window, -1, renderer_flag);
    if (sdl_env->sdl_renderer == NULL)
    {
        printf("SDL_CreateRenderer Error!\n%s\n", SDL_GetError());
		SDL_DestroyWindow(sdl_env->window);
		SDL_Quit();
		free(sdl_env);
        return NULL;
    }

    // get window surface
    sdl_env->win_surface = SDL_GetWindowSurface(sdl_env->window);
    if (sdl_env->win_surface == NULL)
    {
        printf("SDL_GetWindowSurface Error!\n%s\n", SDL_GetError());
		SDL_DestroyWindow(sdl_env->window);
		SDL_Quit();
		free(sdl_env);
        return NULL;
    }

    sdl_env->win_width = w;
    sdl_env->win_height = h;

    return sdl_env;
}


void sdl_env_loop(SDL_Env* sdl_env)
{
    if (sdl_env == NULL)
    {
        printf("sdl_env == NULL\n");
        return;
    }

    if (sdl_env->create_call != NULL)
        sdl_env->create_call(sdl_env->serval_renderer);

// 	SDL_Surface* png_surface = SDL_CreateRGBSurface(0, sdl_env->serval_renderer->sampler.texture->width, sdl_env->serval_renderer->sampler.texture->height, 32, 0, 0, 0, 0);
// 	SDL_LockSurface(png_surface);
// 	memcpy(png_surface->pixels, sdl_env->serval_renderer->sampler.texture->data, sdl_env->serval_renderer->sampler.texture->data_size);
// 	SDL_UnlockSurface(png_surface);
    bool quit = false;
    SDL_Event e;
    while(!quit)
    {
        //Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			// handle events here
			else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
			{
				if (sdl_env->keyboard_call != NULL)
                    sdl_env->keyboard_call(sdl_env->serval_renderer, &e);
			}
			else if (e.type >= SDL_MOUSEMOTION && e.type <= SDL_MOUSEBUTTONUP)
			{
				if (sdl_env->mouse_call != NULL)
                    sdl_env->mouse_call(sdl_env->serval_renderer, &e);
			}
		}

		float elapsed_time = 0.0f;
		float time = 0.0f;

		// do render here
		if (sdl_env->update_call != NULL)
            sdl_env->update_call(sdl_env->serval_renderer, time, elapsed_time);

        if (sdl_env->render_call != NULL)
            sdl_env->render_call(sdl_env->serval_renderer, time, elapsed_time);

        // vertex processor
		vertex_processor_cook_vertices(sdl_env->serval_renderer->vertex_processor);
		
		// clean
		render_target_clean(sdl_env->serval_renderer->cur_render_target, BUFFER_COLOR | BUFFER_DEPTH, 0, 0, 0.0f, 1.0f);
        // pixel processor
        begin_pixel_process(sdl_env->serval_renderer->pixel_processor);
        end_pixel_process(sdl_env->serval_renderer->pixel_processor);

        SDL_LockSurface(sdl_env->win_surface);
        if (sdl_env->serval_renderer)
        {
            void* render_buffer_ptr = render_target_get_render_buffer_ptr(sdl_env->serval_renderer->cur_render_target);
            memcpy(sdl_env->win_surface->pixels, render_buffer_ptr, sizeof(Uint32) * sdl_env->win_width * sdl_env->win_height);
        }
        SDL_UnlockSurface(sdl_env->win_surface);

		SDL_UpdateWindowSurface(sdl_env->window);
    }

    if (sdl_env->destroy_call != NULL)
        sdl_env->destroy_call(sdl_env->serval_renderer);
    return;
}

void sdl_env_destroy(SDL_Env* sdl_env)
{
    if (sdl_env)
    {
        SDL_DestroyWindow(sdl_env->window);
		SDL_Quit();
    }
}
