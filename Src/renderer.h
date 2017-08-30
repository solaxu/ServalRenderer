#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

/*
*   date: 2017.08.19
*   author: solaxu
*   desc: serval-renderer structure
*/

#include "SDL2/SDL.h"
#include "renderer_params.h"
#include "vertex_processor.h"
#include "pixel_processor.h"
#include "dslib/list.h"
#include "dslib/linked_hash.h"
#include "render_target.h"
#include "render_state.h"
#include "sampler.h"


typedef struct _Serval ServalRenderer;
typedef struct _SDL_Env SDL_Env;
typedef struct _VertexProcessor VertexProcessor;
typedef struct _PixelProcessor PixelProcessor;

typedef void(*OnCreate)(ServalRenderer* renderer);
typedef void(*OnDestroy)(ServalRenderer* renderer);
typedef void(*OnFrameRender)(ServalRenderer* renderer, float, float);
typedef void(*OnFrameMove)(ServalRenderer* renderer, float, float);
typedef void(*OnKeyboard)(ServalRenderer* renderer, SDL_Event* e);
typedef void(*OnMouse)(ServalRenderer* renderer, SDL_Event* e);

#include "sdl_env.h"

typedef enum _LightType
{
	POINT_LIGHT,
	DIRECTIONAL_LIGHT,
}LightType;

typedef struct _SLight
{
	Uint32 type;
	bool enable;
	Vector4D position;
	Vector4D direction;
	Vector4D ambient;
	Vector4D diffuse;
	Vector4D specular;
	float range;
}SLight;

typedef struct _SMaterial
{
	Vector4D ambient;
	Vector4D diffuse;
	Vector4D specular;
	Vector4D emissive;
	float pow;
}SMaterial;

typedef struct _Serval
{
    struct _SDL_Env* sdl_env;
    struct _RenderTarget* cur_render_target;
	struct _RenderState cur_render_state;
    VertexProcessor* vertex_processor;
    PixelProcessor* pixel_processor;
    LinkedHash render_targets;
	SLight lights[LIGHTS_NUM];
	SMaterial cur_material;
	SSampler sampler;
}ServalRenderer;

ServalRenderer* serval_create(Uint32 x, Uint32 y, Uint32 width, Uint32 height, const char* title,
    OnCreate create_call, OnDestroy destroy_call, OnFrameMove update_call,
    OnFrameRender render_call, OnKeyboard key_call, OnMouse mouse_call);

void serval_run(ServalRenderer* serval_renderer);

void serval_destroy(ServalRenderer* serval_renderer);

RenderTarget* serval_create_render_target(ServalRenderer* serval_renderer, Uint32 render_target_id ,Uint32 width, Uint32 height, BufferType type);

void serval_set_world_matrix(ServalRenderer* serval, float* view_mat);

void serval_set_view_matrix(ServalRenderer* serval_renderer, float* world_mat);

void serval_set_proj_matrix(ServalRenderer* serval_renderer, float* proj_mat);

void serval_set_view_port_matrix(ServalRenderer* serval_renderer, float* view_port_mat);

float* serval_get_world_matrix(ServalRenderer* serval_renderer);

float* serval_get_view_matrix(ServalRenderer* serval_renderer);

float* serval_get_proj_matrix(ServalRenderer* serval_renderer);

float* serval_get_view_port_matrix(ServalRenderer* serval_renderer);

float* serval_get_matrix_buffer(ServalRenderer* serval_renderer);

void* serval_get_user_data_buffer(ServalRenderer* serval_renderer);

RenderTarget* serval_get_render_target(ServalRenderer* serval_renderer, Uint32 target_id);

void serval_set_render_target(ServalRenderer* serval_renderer, RenderTarget* rt);

void serval_set_vertex_declaration(ServalRenderer* serval_renderer, VertexDeclaration* decl);

void serval_set_vertex_buffer(ServalRenderer* serval_renderer, void* vb, Uint32 vertex_num, Uint32 stride, Uint32 primitive_type);

void serval_set_indexed_vertex_buffer(ServalRenderer* serval_renderer,
                                      void* vb, Uint32 vertex_num, Uint32 stride,
                                      void* ib, Uint32 index_num,
                                      Uint32 primitive_type);

void serval_set_material(ServalRenderer* serval_renderer, Vector4D* emssive, Vector4D* ambient, Vector4D* diffuse, Vector4D* specular, float pow);

void serval_set_light(ServalRenderer* serval_renderer, SLight* light, Uint32 index);

void serval_enable_lighting(ServalRenderer* serval_renderer, bool b);

void serval_enable_alpha_test(ServalRenderer* serval_renderer, bool b);

void serval_enable_z_test(ServalRenderer* serval_renderer, bool b);

void serval_set_z_test_func(ServalRenderer* serval_renderer, Uint32 func);

void serval_set_alpha_test_func(ServalRenderer* serval_renderer, Uint32 func);

void serval_set_texture(ServalRenderer* serval_renderer, STexture* texture);

void serval_set_texture_sample_type(ServalRenderer* serval_renderer, Uint32 sample_type);

void serval_enable_texture(ServalRenderer* serval_renderer, bool b);

void serval_enable_specular(ServalRenderer* serval_renderer, bool b);

#endif // RENDERER_H_INCLUDED
