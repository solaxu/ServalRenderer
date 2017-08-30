#include "renderer.h"
#include <stdlib.h>
#include <assert.h>
#include <memory.h>

ServalRenderer* serval_create(Uint32 x, Uint32 y, Uint32 width, Uint32 height, const char* title,
    OnCreate create_call, OnDestroy destroy_call, OnFrameMove update_call,
    OnFrameRender render_call, OnKeyboard keyboard_call, OnMouse mouse_call)
{
    ServalRenderer* renderer = (ServalRenderer*)malloc(sizeof(ServalRenderer));

    assert(renderer != NULL);
    memset(renderer, 0, sizeof(ServalRenderer));

    renderer->sdl_env = sdl_env_init(x, y, width, height, title);
    assert(renderer->sdl_env);

    renderer->vertex_processor = create_vertex_processor(renderer);
    assert(renderer->vertex_processor);

    renderer->pixel_processor = create_pixel_processor(renderer);
    assert(renderer->pixel_processor);

    linked_hash_init(&renderer->render_targets);

    RenderTarget* main_rt = serval_create_render_target(renderer, MAIN_RENDER_TARGET, width, height, U32);
    renderer->cur_render_target = main_rt;

    renderer->sdl_env->create_call = create_call;
    renderer->sdl_env->destroy_call = destroy_call;
    renderer->sdl_env->keyboard_call = keyboard_call;
    renderer->sdl_env->mouse_call = mouse_call;
    renderer->sdl_env->render_call = render_call;
    renderer->sdl_env->update_call = update_call;

    renderer->sdl_env->serval_renderer = renderer;

    return renderer;
}

void serval_run(ServalRenderer* serval_renderer)
{
    if(serval_renderer)
    {
        ready_pixel_process(serval_renderer->pixel_processor);
        sdl_env_loop(serval_renderer->sdl_env);
    }
}

static void free_rt(void* data)
{
    destroy_render_target((RenderTarget*)data);
}

void serval_destroy(ServalRenderer* serval_renderer)
{
    if (serval_renderer)
    {
		if (serval_renderer->pixel_processor)
		{
			destroy_pixel_processor(serval_renderer->pixel_processor);
			free(serval_renderer->pixel_processor);
		}

		if (serval_renderer->vertex_processor)
        {
            destroy_vertex_processor(serval_renderer->vertex_processor);
            free(serval_renderer->vertex_processor);
        }

		if (serval_renderer->sdl_env)
		{
			sdl_env_destroy(serval_renderer->sdl_env);
			free(serval_renderer->sdl_env);
		}

        linked_hash_destroy(&serval_renderer->render_targets, free_rt);
    }
}

RenderTarget* serval_create_render_target(ServalRenderer* serval_renderer,  Uint32 render_target_id, Uint32 width, Uint32 height, BufferType type)
{
    if (serval_renderer)
    {
        RenderTarget* rt = create_render_target(width, height, type);
        assert(rt);

        linked_hash_insert(&serval_renderer->render_targets, render_target_id, (void*)rt);

        return rt;
    }
    return NULL;
}

void serval_set_world_matrix(ServalRenderer* serval_renderer, float* world_mat)
{
	if (serval_renderer && world_mat)
	{
		if (serval_renderer->vertex_processor && serval_renderer->vertex_processor->constant_memory)
		{
			Uint8* ptr = (Uint8*)serval_renderer->vertex_processor->constant_memory;
			memcpy(ptr + WORLD_MATRIX, world_mat, 64);
		}
	}
}

void serval_set_view_matrix(ServalRenderer* serval_renderer, float* view_mat)
{
    if (serval_renderer && view_mat)
    {
        if (serval_renderer->vertex_processor && serval_renderer->vertex_processor->constant_memory)
        {
            Uint8* ptr = (Uint8*)serval_renderer->vertex_processor->constant_memory;
            memcpy(ptr + VIEW_MATRIX, view_mat, 64);
        }
    }
}

void serval_set_proj_matrix(ServalRenderer* serval_renderer, float* proj_mat)
{
    if (serval_renderer && proj_mat)
    {
        if (serval_renderer->vertex_processor && serval_renderer->vertex_processor->constant_memory)
        {
            Uint8* ptr = (Uint8*)serval_renderer->vertex_processor->constant_memory;
            memcpy(ptr + PROJ_MATRIX, proj_mat, 64);
        }
    }
}

void serval_set_view_port_matrix(ServalRenderer* serval_renderer, float* view_port_mat)
{
    if (serval_renderer && view_port_mat)
    {
        if (serval_renderer->vertex_processor && serval_renderer->vertex_processor->constant_memory)
        {
            Uint8* ptr = (Uint8*)serval_renderer->vertex_processor->constant_memory;
            memcpy(ptr + VIEW_PORT_MATRIX, view_port_mat, 64);
        }
    }
}

float* serval_get_world_matrix(ServalRenderer* serval_renderer)
{
	if (serval_renderer)
	{
		if (serval_renderer->vertex_processor && serval_renderer->vertex_processor->constant_memory)
		{
			Uint8* ptr = (Uint8*)serval_renderer->vertex_processor->constant_memory;
			return (float*)(ptr + WORLD_MATRIX);
		}
	}
	return NULL;
}

float* serval_get_view_matrix(ServalRenderer* serval_renderer)
{
    if (serval_renderer)
    {
        if (serval_renderer->vertex_processor && serval_renderer->vertex_processor->constant_memory)
        {
            Uint8* ptr = (Uint8*)serval_renderer->vertex_processor->constant_memory;
            return (float*)(ptr + VIEW_MATRIX);
        }
    }
    return NULL;
}

float* serval_get_proj_matrix(ServalRenderer* serval_renderer)
{
    if (serval_renderer)
    {
        if (serval_renderer->vertex_processor && serval_renderer->vertex_processor->constant_memory)
        {
            Uint8* ptr = (Uint8*)serval_renderer->vertex_processor->constant_memory;
            return (float*)(ptr + PROJ_MATRIX);
        }
    }
    return NULL;
}

float* serval_get_view_port_matrix(ServalRenderer* serval_renderer)
{
    if (serval_renderer)
    {
        if (serval_renderer->vertex_processor && serval_renderer->vertex_processor->constant_memory)
        {
            Uint8* ptr = (Uint8*)serval_renderer->vertex_processor->constant_memory;
            return (float*)(ptr + VIEW_PORT_MATRIX);
        }
    }
    return NULL;
}

float* serval_get_matrix_buffer(ServalRenderer* serval_renderer)
{
    if (serval_renderer)
    {
        if (serval_renderer->vertex_processor && serval_renderer->vertex_processor->constant_memory)
        {
            Uint8* ptr = (Uint8*)serval_renderer->vertex_processor->constant_memory;
            return (float*)(ptr + MATRIX_BUFFER);
        }
    }
    return NULL;
}

void* serval_get_user_data_buffer(ServalRenderer* serval_renderer)
{
    if (serval_renderer)
    {
        if (serval_renderer->vertex_processor && serval_renderer->vertex_processor->constant_memory)
        {
            Uint8* ptr = (Uint8*)serval_renderer->vertex_processor->constant_memory;
            return (void*)(ptr + USER_DEF_DATA);
        }
    }
    return NULL;
}


RenderTarget* serval_get_render_target(ServalRenderer* serval_renderer, Uint32 target_id)
{
    if(serval_renderer)
    {
        return (RenderTarget*)linked_hash_find(&serval_renderer->render_targets, target_id)->data;
    }
    return NULL;
}

void serval_set_render_target(ServalRenderer* serval_renderer, RenderTarget* rt)
{
    if (serval_renderer)
    {
        serval_renderer->cur_render_target = rt;
    }
}

void serval_set_vertex_declaration(ServalRenderer* serval_renderer, VertexDeclaration* decl)
{
    if (serval_renderer && serval_renderer->vertex_processor)
        serval_renderer->vertex_processor->vertex_decl = decl;
}

void serval_set_vertex_buffer(ServalRenderer* serval_renderer, void* vb, Uint32 vertex_num, Uint32 stride, Uint32 primitive_type)
{
    if (serval_renderer)
    {
        serval_renderer->vertex_processor->vertex_num = vertex_num;
        serval_renderer->vertex_processor->vertex_stride = stride;
        serval_renderer->vertex_processor->primitive_type = primitive_type;
        serval_renderer->vertex_processor->index_num = 0;
        int i = 0;
        Uint8* vb_ptr = (Uint8*)vb;
        Uint8* ptr = serval_renderer->vertex_processor->vertex_input_buffer;
        Uint32 total_stride = stride + sizeof(VertexListLink);
        for (i = 0; i < vertex_num; ++i)
        {
            VertexListLink* link = (VertexListLink*)ptr;
            memset(link, 0, sizeof(VertexListLink));
            Uint8* vertex_ptr = ptr + sizeof(VertexListLink);
            if (i == 0)
                link->prev = NULL;
            else
                link->prev = (VertexListLink*)(ptr - total_stride);
            if (link->prev)
                link->prev->next = link;
            link->vertex_ptr = (float*)vertex_ptr;
            link->is_available = 1;
            memcpy(vertex_ptr, vb_ptr, stride);
            vb_ptr += stride;
            ptr += total_stride;
            if (i == vertex_num - 1)
                link->next = NULL;
        }
        serval_renderer->vertex_processor->available_slot_head = (VertexListLink*)serval_renderer->vertex_processor->vertex_input_buffer;

        // build free list
        Uint32 d = (Uint8*)ptr - (Uint8*)serval_renderer->vertex_processor->vertex_input_buffer;
		if (d >= MAX_VERTEX_CACHE_SIZE)
		{
			serval_renderer->vertex_processor->free_slot_head = NULL;
			return;
		}
		serval_renderer->vertex_processor->free_slot_head = (VertexListLink*)ptr;
        VertexListLink* free = (VertexListLink*)ptr;
		free->prev = NULL;
        while(true)
        {
            d = (Uint8*)ptr - (Uint8*)serval_renderer->vertex_processor->vertex_input_buffer;
            free->vertex_ptr = (float*)(ptr + sizeof(VertexListLink));
			free->is_available = 0;
            if (d + total_stride >= MAX_VERTEX_CACHE_SIZE)
            {
                free->next = NULL;
                break;
            }
            free->next = (VertexListLink*)(ptr + total_stride);
            free->next->prev = free;
			free = free->next;
            ptr += total_stride;
        }
    }
}

void serval_set_indexed_vertex_buffer(ServalRenderer* serval_renderer,
                                      void* vb, Uint32 vertex_num, Uint32 stride,
                                      void* ib, Uint32 index_num,
                                      Uint32 primitive_type)
{
    if (serval_renderer)
    {
        memcpy(serval_renderer->vertex_processor->vertex_input_buffer, vb, vertex_num * stride);
        memcpy(serval_renderer->vertex_processor->index_buffer, ib, vertex_num * sizeof(Uint32));
        serval_renderer->vertex_processor->index_num = index_num;
        serval_renderer->vertex_processor->vertex_num = vertex_num;
        serval_renderer->vertex_processor->vertex_stride = stride;
        serval_renderer->vertex_processor->primitive_type = primitive_type;
    }
}


void serval_set_material(ServalRenderer* serval_renderer, Vector4D* emssive, Vector4D* ambient, Vector4D* diffuse, Vector4D* specular, float pow)
{
	memcpy(&serval_renderer->cur_material.emissive, emssive, sizeof(Vector4D));
	memcpy(&serval_renderer->cur_material.ambient, ambient, sizeof(Vector4D));
	memcpy(&serval_renderer->cur_material.diffuse, diffuse, sizeof(Vector4D));
	memcpy(&serval_renderer->cur_material.specular, specular, sizeof(Vector4D));
}

void serval_set_light(ServalRenderer* serval_renderer, SLight* light, Uint32 index)
{
	memcpy(&serval_renderer->lights[index], light, sizeof(SLight));
}

void serval_enable_lighting(ServalRenderer* serval_renderer, bool b)
{
	serval_renderer->cur_render_state.enable_lighting = b;
}

void serval_enable_alpha_test(ServalRenderer* serval_renderer, bool b)
{
	serval_renderer->cur_render_state.enable_alpha_test = b;
}

void serval_enable_z_test(ServalRenderer* serval_renderer, bool b)
{
	serval_renderer->cur_render_state.enable_z_test = b;
}

void serval_set_z_test_func(ServalRenderer* serval_renderer, Uint32 func)
{
	serval_renderer->cur_render_state.z_test_func = func;
}

void serval_set_alpha_test_func(ServalRenderer* serval_renderer, Uint32 func)
{
	serval_renderer->cur_render_state.alpha_test_func = func;
}

void serval_set_texture(ServalRenderer* serval_renderer, STexture* texture)
{
	serval_renderer->sampler.texture = texture;
}

void serval_set_texture_sample_type(ServalRenderer* serval_renderer, Uint32 sample_type)
{
	serval_renderer->sampler.sampler_type = sample_type;
}

void serval_enable_texture(ServalRenderer* serval_renderer, bool b)
{
	serval_renderer->sampler.enable = b;
	serval_renderer->cur_render_state.enable_texture = b;
}

void serval_enable_specular(ServalRenderer* serval_renderer, bool b)
{
	serval_renderer->cur_render_state.enable_specular = b;
}