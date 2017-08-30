#ifndef VERTEX_PROCESSOR_H_INCLUDED
#define VERTEX_PROCESSOR_H_INCLUDED

#include "renderer_params.h"
#include "SDL2/SDL.h"
#include "smath.h"
#include "renderer.h"

typedef struct _Serval ServalRenderer;

typedef struct _VertexProcessor
{
    void* vertex_input_buffer;  // vertex input buffer, 1 stream only
    void* vertex_output_buffer; // vertex output buffer, 1 stream only, clip output_buffer
    void* constant_memory;      // for transform matrices or something else
    void* index_buffer;         //  index buffer
    VertexDeclaration* vertex_decl; // vertex declaration
    VertexListLink* free_slot_head; // first free vertex slot
    VertexListLink* available_slot_head; // first available vertex slot
	ServalRenderer* renderer;
    int vertex_num_before_clip;
    int vertex_num_after_clip;
    Uint32 index_num;
    Uint32 vertex_stride;
    Uint32 vertex_num;
    Uint32 primitive_type;
    Uint32 cull_mode;
}VertexProcessor;

VertexProcessor* create_vertex_processor(ServalRenderer* renderer);

void destroy_vertex_processor(VertexProcessor* vp);

float* vertex_processor_get_world_matrix(VertexProcessor* vp);

float* vertex_processor_get_view_matrix(VertexProcessor* vp);

float* vertex_processor_get_proj_matrix(VertexProcessor* vp);

float* vertex_processor_get_view_port_matrix(VertexProcessor* vp);

void vertex_processor_get_near_plane(VertexProcessor* vp, Plane3D* near_plane);

void vertex_processor_cook_vertices(VertexProcessor* vp);

#endif // VERTEX_PROCESSOR_H_INCLUDED
