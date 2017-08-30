#include "vertex_processor.h"

#include <memory.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

#include "smath.h"
#include "rasterize_tile.h"
#include "render_target.h"

VertexProcessor* create_vertex_processor(ServalRenderer* renderer)
{
    VertexProcessor* vp = (VertexProcessor*)malloc(sizeof(VertexProcessor));
    assert(vp);

    memset(vp, 0, sizeof(VertexProcessor));

    vp->vertex_input_buffer = malloc(MAX_VERTEX_CACHE_SIZE);
    assert(vp->vertex_input_buffer);

    vp->vertex_output_buffer = malloc(MAX_VERTEX_CACHE_SIZE);
    assert(vp->vertex_output_buffer);

    vp->constant_memory = malloc(MAX_VERTEX_PROCESSOR_CONST_CACHE_SIZE);
    assert(vp->constant_memory);

    vp->index_buffer =malloc(MAX_INDEX_NUM * sizeof(Uint32));
    assert(vp->index_buffer);

	vp->renderer = renderer;

    return vp;
}

void destroy_vertex_processor(VertexProcessor* vp)
{
    if (vp)
    {
        if (vp->constant_memory)
            free(vp->constant_memory);
        if (vp->index_buffer)
            free(vp->index_buffer);
        if (vp->vertex_input_buffer)
            free(vp->vertex_input_buffer);
        if (vp->vertex_output_buffer)
            free(vp->vertex_output_buffer);
    }
}

float* vertex_processor_get_view_matrix(VertexProcessor* vp)
{
    return (float*)((unsigned char*)vp->constant_memory + VIEW_MATRIX);
}

float* vertex_processor_get_world_matrix(VertexProcessor* vp)
{
	return (float*)((unsigned char*)vp->constant_memory + WORLD_MATRIX);
}

float* vertex_processor_get_proj_matrix(VertexProcessor* vp)
{
    return (float*)((unsigned char*)vp->constant_memory + PROJ_MATRIX);
}

float* vertex_processor_get_view_port_matrix(VertexProcessor* vp)
{
    return (float*)((unsigned char*)vp->constant_memory + VIEW_PORT_MATRIX);
}

void vertex_processor_set_cull_mode(VertexProcessor* vp, Uint32 cull_mode)
{
    vp->cull_mode = cull_mode;
}

static Uint32 check_cull(Vector3D* v1, Vector3D* v2, Vector3D* v3, Vector3D* view_dir)
{
    Vector3D v12, v13;
    vector3d_sub(&v12, v1, v2);
    vector3d_sub(&v13, v1, v3);
    vector3d_normalize(&v12);
    vector3d_normalize(&v13);
    Vector3D normal;
    vector3d_cross(&normal, &v12, &v13);
    vector3d_normalize(&normal);
    float c = vector3d_dot(view_dir, &normal);
    if (c < 0)
        return TRIANGLE_COUNTER_CLOCKWISE;
    else if(c == 0)
        return TRIANGLE_VERTICAL;
    else
        return TRIANGLE_CLOCKWISE;
}

void vertex_processor_get_near_plane(VertexProcessor* vp, Plane3D* near_plane)
{
    Matrix4x4* projMat = (Matrix4x4*)vertex_processor_get_proj_matrix(vp);
    float near_dist = -projMat->m32 / projMat->m22;
    near_plane->x = 0.0f;
    near_plane->y = 0.0f;
    near_plane->z = 1.0f;
    near_plane->d = near_dist;
}

static void vp_pick_triangles(ServalRenderer* renderer, VertexListLink* vertex)
{
	RenderTarget* rt = renderer->cur_render_target;
	Vector2D* v1 = (Vector2D*)vertex->vertex_ptr;
	Vector2D* v2 = (Vector2D*)(vertex->next->vertex_ptr);
	Vector2D* v3 = (Vector2D*)(vertex->next->next->vertex_ptr);

	int i = 0, tile_triangle_index = 0;
	while (i < rt->tile_num)
	{
		Rasterize_Tile* tile = &rt->render_tiles[i];
		bool is_intersect = intersect_test_triangle_rect(v1->x, v1->y, v2->x, v2->y, v3->x, v3->y, 
			tile->left, tile->right, tile->top, tile->bottom);

		if (is_intersect)
		{
			tile->primitive_indices[tile->primitive_count] = vertex;
			tile->primitive_count++;
		}
		++i;
	}
}

#if PROGRAMMABLE_PIPELEINE == 0

static void vp_fixed_cook_triangle(VertexProcessor* vp)
{
    // check cull first
	Matrix4x4* wm = serval_get_world_matrix(vp->renderer);
	Matrix4x4* vm = serval_get_view_matrix(vp->renderer);
	Matrix4x4* pm = serval_get_proj_matrix(vp->renderer);
	Matrix4x4* vpm = serval_get_view_port_matrix(vp->renderer);
    static Uint8 clip_buffer[1024];
    static Uint8 clip_ver_num = 0;
    int i = 0;
    VertexDeclaration* vertex_decl = vp->vertex_decl;
    Vector3D view_dir;
    __m128 s128, e128, r128, f128; // for SIMD interpolation
    matrix_get_view_dir_from_view_mat(&view_dir, (Matrix4x4*)vertex_processor_get_view_matrix(vp));
	VertexListLink* cur_link = vp->available_slot_head;
    VertexListLink* free_link = vp->free_slot_head;
    float near_dist = -((Matrix4x4*)vertex_processor_get_proj_matrix(vp))->m32 / ((Matrix4x4*)vertex_processor_get_proj_matrix(vp))->m22;
	Vector4D temp;
	Vector3D* v1 = NULL, *v2 = NULL, *v3 = NULL;
    while(i < vp->vertex_num)
    {
		// first, translate triangles from local space to view space
		v1 = (Vector3D*)cur_link->vertex_ptr;
		v2 = (Vector3D*)cur_link->next->vertex_ptr;
		v3 = (Vector3D*)cur_link->next->next->vertex_ptr;

		// to world space
		vector4d_transform_coord(&temp, (Vector4D*)v1, (Matrix4x4*)vertex_processor_get_world_matrix(vp));
		memcpy(v1, &temp, 16);
		vector4d_transform_coord(&temp, (Vector4D*)v2, (Matrix4x4*)vertex_processor_get_world_matrix(vp));
		memcpy(v2, &temp, 16);
		vector4d_transform_coord(&temp, (Vector4D*)v3, (Matrix4x4*)vertex_processor_get_world_matrix(vp));
		memcpy(v3, &temp, 16);

		// to view space
		vector4d_transform_coord(&temp, (Vector4D*)v1, (Matrix4x4*)vertex_processor_get_view_matrix(vp));
		memcpy(v1, &temp, 16);
		vector4d_transform_coord(&temp, (Vector4D*)v2, (Matrix4x4*)vertex_processor_get_view_matrix(vp));
		memcpy(v2, &temp, 16);
		vector4d_transform_coord(&temp, (Vector4D*)v3, (Matrix4x4*)vertex_processor_get_view_matrix(vp));
		memcpy(v3, &temp, 16);

        Uint32 cull = check_cull(v1, v2, v3, &view_dir);
        // do cull, drop some triangles
        if ((cull == TRIANGLE_CLOCKWISE && vp->cull_mode == CULL_CW) ||
            (cull == TRIANGLE_COUNTER_CLOCKWISE && vp->cull_mode == CULL_CCW))
        {
			VertexListLink* next_cur = cur_link->next->next->next;
			if (cur_link->prev)
				cur_link->prev->next = next_cur;
			if (next_cur)
				next_cur->prev = cur_link->prev;
			cur_link->is_available = 0;
			cur_link->next->is_available = 0;
			cur_link->next->next->is_available = 0;

			cur_link->next->next->next = vp->free_slot_head;
			if (vp->free_slot_head)
				vp->free_slot_head->prev = cur_link->next->next;
			vp->free_slot_head = cur_link;
			if (vp->free_slot_head)
				vp->free_slot_head->prev = NULL;

			if (cur_link->prev == NULL)
				vp->available_slot_head = next_cur;

			cur_link = next_cur;
        }
        else
        {
            // clip by near
            Plane3D near_plane;
            near_plane.x = 0.0f; near_plane.y = 0.0f; near_plane.z = 1.0f;
            near_plane.d = near_dist;

            float v1_space = vector3d_dot(v1, (Vector3D*)&near_plane) - near_plane.d;
            float v2_space = vector3d_dot(v2, (Vector3D*)&near_plane) - near_plane.d;
            float v3_space = vector3d_dot(v3, (Vector3D*)&near_plane) - near_plane.d;

            // behind or on near plane, drop it
			if ((v1_space < 0.0f && v2_space < 0.0f && v3_space < 0.0f) ||
				(v1_space == 0.0f && v2_space <= 0.0f && v3_space <= 0.0f) ||
				(v1_space <= 0.0f && v2_space == 0.0f && v3_space <= 0.0f) ||
				(v1_space <= 0.0f && v2_space <= 0.0f && v3_space == 0.0f))
			{
				vp->vertex_num_after_clip -= 3;
				VertexListLink* next_cur = cur_link->next->next->next;
				if (cur_link->prev)
					cur_link->prev->next = next_cur;
				if (next_cur)
					next_cur->prev = cur_link->prev;
				cur_link->is_available = 0;
				cur_link->next->is_available = 0;
				cur_link->next->next->is_available = 0;

				cur_link->next->next->next = vp->free_slot_head;
				if (vp->free_slot_head)
					vp->free_slot_head->prev = cur_link->next->next;
				vp->free_slot_head = cur_link;
				if (vp->free_slot_head)
					vp->free_slot_head->prev = NULL;

				if (cur_link->prev == NULL)
					vp->available_slot_head = next_cur;
				cur_link = next_cur;
			}
			else if ((v1_space > 0.0f && v2_space > 0.0f && v3_space > 0.0f) ||
				(v1_space == 0.0f && v2_space >= 0.0f && v3_space >= 0.0f) ||
				(v1_space >= 0.0f && v2_space == 0.0f && v3_space >= 0.0f) ||
				(v1_space >= 0.0f && v2_space >= 0.0f && v3_space == 0.0f))
			{
				// do nothing, jump to next triangle
				vp->vertex_num_after_clip += 3;
				cur_link = cur_link->next->next->next;
			}
			else
			{
#define v4_interp_helper(out, v1, v2, r) {  s128 = _mm_loadu_ps(v1);e128 = _mm_loadu_ps(v2);r128 = _mm_set_ps(r,r,r,r);\
                                            f128 = _mm_add_ps(s128,_mm_mul_ps(r128,_mm_sub_ps(e128,s128)));\
                                            _mm_storeu_ps(out,f128); }

#define vertex_interp_helper(v1, v2, r)   {     while(vertex_decl->semantic != DECL_NONE)\
                                                {\
                                                    if (vertex_decl->semantic == DECL_COLOR)\
                                                    {\
                                                        v4_interp_helper((float*)(new_ver_ptr + vertex_decl->offset),\
                                                                         (float*)((Uint8*)(v1) + vertex_decl->offset),\
                                                                         (float*)((Uint8*)(v2) + vertex_decl->offset), r);\
                                                    }\
                                                    else if (vertex_decl->semantic == DECL_NORMAL)\
                                                    {\
                                                        v4_interp_helper((float*)(new_ver_ptr + vertex_decl->offset),\
                                                                         (float*)((Uint8*)(v1) + vertex_decl->offset),\
                                                                         (float*)((Uint8*)(v2) + vertex_decl->offset), r);\
                                                    }\
                                                    else if (vertex_decl->semantic == DECL_TANGENT)\
                                                    {\
                                                        v4_interp_helper((float*)(new_ver_ptr + vertex_decl->offset),\
                                                                         (float*)((Uint8*)(v1) + vertex_decl->offset),\
                                                                         (float*)((Uint8*)(v2) + vertex_decl->offset), r);\
                                                    }\
                                                    else if (vertex_decl->semantic == DECL_TEXCOORD)\
                                                    {\
                                                        v4_interp_helper((float*)(new_ver_ptr + vertex_decl->offset),\
                                                                         (float*)((Uint8*)(v1) + vertex_decl->offset),\
                                                                         (float*)((Uint8*)(v2) + vertex_decl->offset), r);\
                                                    }\
                                                    else if (vertex_decl->semantic == DECL_POSITION)\
                                                    {\
                                                        v4_interp_helper((float*)(new_ver_ptr + vertex_decl->offset),\
                                                                         (float*)((Uint8*)(v1) + vertex_decl->offset),\
                                                                         (float*)((Uint8*)(v2) + vertex_decl->offset), r);\
                                                    } \
													else if (vertex_decl->semantic == DECL_POSITION0)\
                                                    {\
                                                        v4_interp_helper((float*)(new_ver_ptr + vertex_decl->offset),\
                                                                         (float*)((Uint8*)(v1) + vertex_decl->offset),\
                                                                         (float*)((Uint8*)(v2) + vertex_decl->offset), r);\
                                                    } \
													vertex_decl++;\
                                                } \
                                            }

                memset(clip_buffer, 0, 1024);
                clip_ver_num = 0;
                Uint8* new_ver_ptr = clip_buffer;
                // clip triangle
				if (v1_space < 0.0f)
				{
					if (v2_space == 0.0f)
					{
						memcpy(new_ver_ptr, v2, vp->vertex_stride);
						new_ver_ptr += vp->vertex_stride;

						memcpy(new_ver_ptr, v3, vp->vertex_stride);
						new_ver_ptr += vp->vertex_stride;

						float s31 = (near_dist - v3->z) / (v1->z - v3->z);
						vertex_decl = vp->vertex_decl;
						vertex_interp_helper((float*)v3, (float*)v1, s31);
						clip_ver_num = 3;
					}
					else if (v2_space < 0.0f)
					{
						float s23 = (near_dist - v2->z) / (v3->z - v2->z);
						vertex_decl = vp->vertex_decl;
						vertex_interp_helper((float*)v2, (float*)v3, s23);
						new_ver_ptr += vp->vertex_stride;

						memcpy(new_ver_ptr, v3, vp->vertex_stride);
						new_ver_ptr += vp->vertex_stride;

						float s31 = (near_dist - v3->z) / (v1->z - v3->z);
						vertex_decl = vp->vertex_decl;
						vertex_interp_helper((float*)v3, (float*)v1, s31);
						clip_ver_num = 3;
					}
					else
					{
						if (v3_space == 0.0f)
						{
							float s12 = (near_dist - v1->z) / (v2->z - v1->z);
							vertex_decl = vp->vertex_decl;
							vertex_interp_helper((float*)v1, (float*)v2, s12);
							new_ver_ptr += vp->vertex_stride;

							memcpy(new_ver_ptr, v2, vp->vertex_stride);
							new_ver_ptr += vp->vertex_stride;

							memcpy(new_ver_ptr, v3, vp->vertex_stride);

							clip_ver_num = 3;
						}
						else if (v3_space > 0.0f)
						{
							float s12 = (near_dist - v1->z) / (v2->z - v1->z);
							vertex_decl = vp->vertex_decl;
							vertex_interp_helper((float*)v1, (float*)v2, s12);
							new_ver_ptr += vp->vertex_stride;

							memcpy(new_ver_ptr, v2, vp->vertex_stride);
							new_ver_ptr += vp->vertex_stride;

							memcpy(new_ver_ptr, v3, vp->vertex_stride);
							new_ver_ptr += vp->vertex_stride;

							float s31 = (near_dist - v3->z) / (v1->z - v3->z);
							vertex_decl = vp->vertex_decl;
							vertex_interp_helper((float*)v3, (float*)v1, s31);
							clip_ver_num = 4;
						}
						else
						{
							float s12 = (near_dist - v1->z) / (v2->z - v1->z);
							vertex_decl = vp->vertex_decl;
							vertex_interp_helper((float*)v1, (float*)v2, s12);
							new_ver_ptr += vp->vertex_stride;

							memcpy(new_ver_ptr, v2, vp->vertex_stride);
							new_ver_ptr += vp->vertex_stride;

							float s23 = (near_dist - v2->z) / (v3->z - v2->z);
							vertex_decl = vp->vertex_decl;
							vertex_interp_helper((float*)v2, (float*)v3, s23);
							clip_ver_num = 3;
						}
					}
				}
				else if (v1_space == 0.0f)
				{
					memcpy(new_ver_ptr, v1, vp->vertex_stride);
					new_ver_ptr += vp->vertex_stride;

					memcpy(new_ver_ptr, v2, vp->vertex_stride);
					new_ver_ptr += vp->vertex_stride;

					float s23 = (near_dist - v2->z) / (v3->z - v2->z);
					vertex_decl = vp->vertex_decl;
					vertex_interp_helper((float*)v2, (float*)v3, s23);
					clip_ver_num = 3;
				}
				else
				{
					if (v2_space == 0.0f)
					{
						memcpy(new_ver_ptr, v1, vp->vertex_stride);
						new_ver_ptr += vp->vertex_stride;

						memcpy(new_ver_ptr, v2, vp->vertex_stride);
						new_ver_ptr += vp->vertex_stride;

						float s31 = (near_dist - v3->z) / (v1->z - v3->z);
						vertex_decl = vp->vertex_decl;
						vertex_interp_helper((float*)v3, (float*)v1, s31);
						clip_ver_num = 3;
					}
					else if (v2_space > 0.0f)
					{
						memcpy(new_ver_ptr, v1, vp->vertex_stride);
						new_ver_ptr += vp->vertex_stride;

						memcpy(new_ver_ptr, v2, vp->vertex_stride);
						new_ver_ptr += vp->vertex_stride;

						float s23 = (near_dist - v2->z) / (v3->z - v2->z);
						vertex_decl = vp->vertex_decl;
						vertex_interp_helper((float*)v2, (float*)v3, s23);
						new_ver_ptr += vp->vertex_stride;

						float s31 = (near_dist - v3->z) / (v1->z - v3->z);
						vertex_decl = vp->vertex_decl;
						vertex_interp_helper((float*)v3, (float*)v1, s31);
						clip_ver_num = 4;
					}
					else
					{
						if (v3_space == 0.0f)
						{
							memcpy(new_ver_ptr, v1, vp->vertex_stride);
							new_ver_ptr += vp->vertex_stride;

							float s12 = (near_dist - v1->z) / (v2->z - v1->z);
							vertex_decl = vp->vertex_decl;
							vertex_interp_helper((float*)v1, (float*)v2, s12);
							new_ver_ptr += vp->vertex_stride;

							memcpy(new_ver_ptr, v3, vp->vertex_stride);
							clip_ver_num = 3;
						}
						else if (v3_space > 0.0f)
						{
							memcpy(new_ver_ptr, v1, vp->vertex_stride);
							new_ver_ptr += vp->vertex_stride;

							float s12 = (near_dist - v1->z) / (v2->z - v1->z);
							vertex_decl = vp->vertex_decl;
							vertex_interp_helper((float*)v1, (float*)v2, s12);
							new_ver_ptr += vp->vertex_stride;

							float s23 = (near_dist - v2->z) / (v3->z - v2->z);
							vertex_decl = vp->vertex_decl;
							vertex_interp_helper((float*)v2, (float*)v3, s23);
							new_ver_ptr += vp->vertex_stride;

							memcpy(new_ver_ptr, v3, vp->vertex_stride);
							clip_ver_num = 4;
						}
						else
						{
							memcpy(new_ver_ptr, v1, vp->vertex_stride);
							new_ver_ptr += vp->vertex_stride;

							float s12 = (near_dist - v1->z) / (v2->z - v1->z);
							vertex_decl = vp->vertex_decl;
							vertex_interp_helper((float*)v1, (float*)v2, s12);
							new_ver_ptr += vp->vertex_stride;

							float s31 = (near_dist - v3->z) / (v1->z - v3->z);
							vertex_decl = vp->vertex_decl;
							vertex_interp_helper((float*)v3, (float*)v1, s31);

							clip_ver_num = 3;
						}
					}
				}

                // copy to vertex list buffer
                if (clip_ver_num == 3)
                {
					vp->vertex_num_after_clip += 3;
                    memcpy(v1, clip_buffer, vp->vertex_stride);
                    memcpy(v2, clip_buffer + vp->vertex_stride, vp->vertex_stride);
                    memcpy(v3, clip_buffer + vp->vertex_stride + vp->vertex_stride, vp->vertex_stride);
                }
                else if (clip_ver_num == 4)
                {
                    vp->vertex_num_after_clip += 6;
                    memcpy(v1, clip_buffer, vp->vertex_stride);
                    memcpy(v2, clip_buffer + vp->vertex_stride, vp->vertex_stride);
                    memcpy(v3, clip_buffer + vp->vertex_stride + vp->vertex_stride, vp->vertex_stride);
                    // new slots to fill
                    memcpy((Uint8*)vp->free_slot_head->vertex_ptr, clip_buffer + vp->vertex_stride *  2, vp->vertex_stride);
                    memcpy((Uint8*)vp->free_slot_head->next->vertex_ptr, clip_buffer + vp->vertex_stride * 3, vp->vertex_stride);
                    memcpy((Uint8*)vp->free_slot_head->next->next->vertex_ptr, clip_buffer, vp->vertex_stride);

					VertexListLink* next_free = vp->free_slot_head->next->next->next;
                    if (vp->available_slot_head)
                        vp->available_slot_head->prev = vp->free_slot_head->next->next;
					vp->free_slot_head->next->next->next = vp->available_slot_head;
					vp->free_slot_head->is_available = 1;
					vp->free_slot_head->next->is_available = 1;
					vp->free_slot_head->next->next->is_available = 1;

                    vp->available_slot_head = vp->free_slot_head;

                    vp->free_slot_head = next_free;
					if (vp->free_slot_head)
						vp->free_slot_head->prev = NULL;
                }
				cur_link = cur_link->next->next->next;
            }
        }
        i += 3;
    }
	printf("/////////////////////////////////////////////////\n");
	VertexListLink* temp_head = vp->available_slot_head;
	while (temp_head)
	{
		Vector4D* v4 = (Vector4D*)temp_head->vertex_ptr;
		printf("x: %f, y: %f, z: %f, w: %f\n", v4->x, v4->y, v4->z, v4->w);
		temp_head = temp_head->next;
	}
#undef v4_interp_helper
#undef vertex_interp_helper
    // find first available vertex
    VertexListLink* list_head = vp->available_slot_head;

    // do other transforms and copy vertexes to output buffer
	Uint32 position0_offset, normal_offset;
	get_vertex_decl_attribute(vp->vertex_decl, DECL_POSITION0, &position0_offset, NULL);
	get_vertex_decl_attribute(vp->vertex_decl, DECL_NORMAL, &normal_offset, NULL);

	for (i = 0; i < vp->renderer->cur_render_target->tile_num; ++i)
		vp->renderer->cur_render_target->render_tiles[i].primitive_count = 0;
	while (list_head)
	{
		v1 = (Vector3D*)list_head->vertex_ptr;
		v2 = (Vector3D*)list_head->next->vertex_ptr;
		v3 = (Vector3D*)list_head->next->next->vertex_ptr;

		// POSITION0 to view space for phong-lighting
		if(position0_offset != INT_MAX)
		{
			vector4d_transform_coord(&temp, (Vector4D*)((Uint8*)v1 + position0_offset), (Matrix4x4*)vertex_processor_get_view_matrix(vp));
			memcpy((Uint8*)v1 + position0_offset, &temp, 16);
			vector4d_transform_coord(&temp, (Vector4D*)((Uint8*)v2 + position0_offset), (Matrix4x4*)vertex_processor_get_view_matrix(vp));
			memcpy((Uint8*)v2 + position0_offset, &temp, 16);
			vector4d_transform_coord(&temp, (Vector4D*)((Uint8*)v3 + position0_offset), (Matrix4x4*)vertex_processor_get_view_matrix(vp));
			memcpy((Uint8*)v3 + position0_offset, &temp, 16);
		}

		if (normal_offset != INT_MAX)
		{
			// NORMAL to view space for phong-lighting
			vector4d_transform_coord(&temp, (Vector4D*)((Uint8*)v1 + normal_offset), (Matrix4x4*)vertex_processor_get_view_matrix(vp));
			memcpy((Uint8*)v1 + normal_offset, &temp, 16);
			vector4d_transform_coord(&temp, (Vector4D*)((Uint8*)v2 + normal_offset), (Matrix4x4*)vertex_processor_get_view_matrix(vp));
			memcpy((Uint8*)v2 + normal_offset, &temp, 16);
			vector4d_transform_coord(&temp, (Vector4D*)((Uint8*)v3 + normal_offset), (Matrix4x4*)vertex_processor_get_view_matrix(vp));
			memcpy((Uint8*)v3 + normal_offset, &temp, 16);
		}
		
		// to projection space
		vector4d_transform_coord(&temp, (Vector4D*)v1, (Matrix4x4*)vertex_processor_get_proj_matrix(vp));
		memcpy(v1, &temp, 16);
		vector4d_transform_coord(&temp, (Vector4D*)v2, (Matrix4x4*)vertex_processor_get_proj_matrix(vp));
		memcpy(v2, &temp, 16);
		vector4d_transform_coord(&temp, (Vector4D*)v3, (Matrix4x4*)vertex_processor_get_proj_matrix(vp));
		memcpy(v3, &temp, 16);

		// to viewport
		vector4d_transform_coord(&temp, (Vector4D*)v1, (Matrix4x4*)vertex_processor_get_view_port_matrix(vp));
		memcpy(v1, &temp, 16);
		vector4d_transform_coord(&temp, (Vector4D*)v2, (Matrix4x4*)vertex_processor_get_view_port_matrix(vp));
		memcpy(v2, &temp, 16);
		vector4d_transform_coord(&temp, (Vector4D*)v3, (Matrix4x4*)vertex_processor_get_view_port_matrix(vp));
		memcpy(v3, &temp, 16);

		// pick triangles to tiles
		vp_pick_triangles(vp->renderer, (VertexListLink*)list_head);
		
		list_head = list_head->next->next->next;
	}
}

static void vp_fixed_cook_line(VertexProcessor* vp)
{

}

static void vp_fixed_cook_point(VertexProcessor* vp)
{

}

#elif PROGRAMMABLE_PIPELEINE == 1

static void vp_programmable_cook_triangle(VertexProcessor* vp)
{

}

static void vp_programmable_cook_line(VertexProcessor* vp)
{

}

static void vp_programmable_cook_point(VertexProcessor* vp)
{

}

#endif

void vertex_processor_cook_vertices(VertexProcessor* vp)
{
    VertexDeclaration* vertex_decl = vp->vertex_decl;
    Uint32 vertex_stride = 0;
    while(vertex_decl->semantic != DECL_NONE)
    {
        vertex_stride += vertex_decl->stride;
        vertex_decl++;
    }
    if (vertex_stride != vp->vertex_stride)
    {
        printf("Error: Vertex Declaration Do Not Matched, Check 'vertex stride'\n");
        return;
    }
    vp->vertex_num_before_clip = 0;
#if MULTI_THREADS_VERTEX_PROCESSOR == 0
// single thread to cook vertexes
#if PROGRAMMABLE_PIPELEINE == 0
    // fixed pipeline, only colored triangle is supported
    if (vp->primitive_type == PRIMITIVE_TRIANGLE)
        vp_fixed_cook_triangle(vp);
    else if (vp->primitive_type == PRIMITIVE_LINE)
        vp_fixed_cook_line(vp);
    else if (vp->primitive_type == PRIMITIVE_POINT)
        vp_fixed_cook_point(vp);
#elif PROGRAMMABLE_PIPELEINE == 1
    if (vp->primitive_type == PRIMITIVE_TRIANGLE)
        vp_programmable_cook_triangle(vp);
    else if (vp->primitive_type == PRIMITIVE_LINE)
        vp_programmable_cook_triangle(vp);
    else if (vp->primitive_type == PRIMITIVE_POINT)
        vp_programmable_cook_point(vp);
#endif
//
#elif MULTI_THREADS_VERTEX_PROCESSOR == 1
	// multiple threads to cook vertexes
#if PROGRAMMABLE_PIPELEINE == 0
    // fixed pipeline
#elif PROGRAMMABLE_PIPELEINE == 1

#endif
#endif
}
