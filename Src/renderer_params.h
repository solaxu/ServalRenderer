#ifndef RENDERER_PARAMS_H_INCLUDED
#define RENDERER_PARAMS_H_INCLUDED

#include "SDL2/SDL.h"

#define MAX_PRIMITIVE_INDEX_NUM_PER_TILE 4096
#define MAX_VERTEX_CACHE_SIZE 48*9 // 1024*1024*16
#define RASTERIZE_TILE_SIZE 32
#define MAX_RENDER_TILE_NUM 2048
#define MAX_INDEX_NUM 1024*1024
#define MAX_VERTEX_PROCESSOR_CONST_CACHE_SIZE 64*512

#define WORLD_MATRIX 0
#define VIEW_MATRIX 64
#define PROJ_MATRIX 128
#define VIEW_PORT_MATRIX 192
#define MATRIX_BUFFER 256
#define USER_DEF_DATA (256+64*256)

#define MAIN_RENDER_TARGET 0

typedef enum _VertexDecl
{
	DECL_NONE = 0,
	DECL_POSITION,
	DECL_COLOR,
	DECL_NORMAL,
	DECL_TEXCOORD,
	DECL_TANGENT,
	DECL_MATRIX4X4,
	DECL_POSITION0,
	DECL_POSITION1,
	DECL_POSITION2,
	DECL_POSITION3,
	DECL_POSITION4,
	DECL_POSITION5,
	DECL_POSITION6,
	DECL_POSITION7,
	DECL_NORMAL0,
	DECL_NORMAL1,
	DECL_NORMAL2,
	DECL_NORMAL3,
	DECL_NORMAL4,
	DECL_NORMAL5,
	DECL_NORMAL6,
	DECL_NORMAL7,
	DECL_TEXCOORD0,
	DECL_TEXCOORD1,
	DECL_TEXCOORD2,
	DECL_TEXCOORD3,
	DECL_TEXCOORD4,
	DECL_TEXCOORD5,
	DECL_TEXCOORD6,
	DECL_TEXCOORD7,
	DECL_TANGENT0,
	DECL_TANGENT1,
	DECL_TANGENT2,
	DECL_TANGENT3,
	DECL_TANGENT4,
	DECL_TANGENT5,
	DECL_TANGENT6,
	DECL_TANGENT7,
	DECL_COLOR0,
	DECL_COLOR1,
	DECL_COLOR2,
	DECL_COLOR3,
	DECL_COLOR4,
	DECL_COLOR5,
	DECL_COLOR6,
	DECL_COLOR7
};

#define PRIMITIVE_NONE 0
#define PRIMITIVE_POINT 1
#define PRIMITIVE_LINE 2
#define PRIMITIVE_TRIANGLE 3

#define CULL_NONE 0
#define CULL_CW 1
#define CULL_CCW 2

#define TRIANGLE_CLOCKWISE 0
#define TRIANGLE_COUNTER_CLOCKWISE 1
#define TRIANGLE_VERTICAL 2

#define LIGHTS_NUM 8

// if we can use multi-threads to deal with vertex process#

#define MULTI_THREADS_VERTEX_PROCESSOR 0
#define PROGRAMMABLE_PIPELEINE 0

// CoreI5 U7200, 2 Cores, 4 Threads, change to fit other environments
#define RASTERIZER_THREAD_NUM 4
#define VERTEX_THREAD_NUM 4

// vertex declaration
typedef struct _VertexDeclaration
{
	Uint32 semantic;
	Uint32 offset;
	Uint32 stride;
}VertexDeclaration;

#define VERTEX_DECL_ELEM(semantic, offset, stride) {semantic, offset, stride}
#define VERTEX_DECL_END() {DECL_NONE, 0, 0}

typedef struct _VertexListLink
{
	struct _VertexListLink* prev;
	struct _VertexListLink* next;
	float* vertex_ptr;
	Uint32 is_available; // if x == 0 means free, else means used
}VertexListLink;

typedef struct _PixelOutputUint
{
    union{
        float value_f32;
        Uint32 value_u32;
        struct
        {
            Uint8 value[4];
        };
    };
}PixelOutputUint;

void get_vertex_decl_attribute(VertexDeclaration* decl, Uint32 semantic, Uint32* offset, Uint32* stride);

#endif // RENDERER_PARAMS_H_INCLUDED
