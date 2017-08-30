#ifndef RASTERIZE_TILE_H_INCLUDED
#define RASTERIZE_TILE_H_INCLUDED

#include "SDL2/SDL.h"
#include "renderer_params.h"

typedef struct _Rasterize_Tile
{
    float left;
    float right;
    float top;
    float bottom;
    Uint32 primitive_count; // how many primitives are there intersect with the tile
    VertexListLink* primitive_indices[MAX_PRIMITIVE_INDEX_NUM_PER_TILE]; // as default
}Rasterize_Tile;

#endif // RASTERIZE_TILE_H_INCLUDED
