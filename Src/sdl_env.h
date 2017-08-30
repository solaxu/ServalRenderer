#ifndef SDL_ENV_H_INCLUDED
#define SDL_ENV_H_INCLUDED

/*
*   date:   2017.08.18
*   author: solaxu
*   description:
*       sdl2.0 environment structure
*/

#include "SDL2/SDL.h"
#include "SDL2/SDL_surface.h"
#include "SDL2/SDL_render.h"
#include "renderer.h"

#ifdef _MSC_VER

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2test.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "zlib.lib")

#endif

typedef struct _SDL_Env SDL_Env;
typedef struct _Serval ServalRenderer;

typedef struct _SDL_Env
{
    Uint32 win_width;
    Uint32 win_height;
    SDL_Window* window;
    SDL_Renderer* sdl_renderer;
    SDL_Surface* win_surface;
    struct _Serval* serval_renderer;
    OnCreate create_call;
    OnDestroy destroy_call;
    OnFrameMove update_call;
    OnFrameRender render_call;
    OnKeyboard keyboard_call;
    OnMouse mouse_call;
}SDL_Env;

// build sdl environment
SDL_Env* sdl_env_init(Uint32 x, Uint32 y, Uint32 w, Uint32 h, const char* win_title);

// loop
void sdl_env_loop(SDL_Env* sdl_env);

// destroy
void sdl_env_destroy(SDL_Env* sdl_env);

#endif // SDL_ENV_H_INCLUDED

