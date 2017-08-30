#include "rasterizer.h"
#include "vertex_processor.h"
#include "renderer_params.h"
#include "renderer.h"
#include "sampler.h"
#include <Mmsystem.h>

static void rasterize_triangle(ServalRenderer* renderer, Rasterize_Tile* tile);
static void rasterize_line(ServalRenderer* renderer, Rasterize_Tile* tile);
static void rasterize_point(ServalRenderer* renderer, Rasterize_Tile* tile);
static void rasterize_none(ServalRenderer* renderer, Rasterize_Tile* tile, Uint32 c);

DWORD WINAPI rasterizer(LPVOID* _renderer)
{
    ServalRenderer* renderer = _renderer;
    DWORD tid = GetCurrentThreadId();
    HANDLE begin_evt = (HANDLE)(linked_hash_find(&renderer->pixel_processor->tid_begin_evt_map, tid)->data);
    HANDLE end_evt = (HANDLE)(linked_hash_find(&renderer->pixel_processor->tid_end_evt_map, tid)->data);
    Rasterize_Tile* tile = NULL;
    PixelProcessor* pp = renderer->pixel_processor;
	printf("Thread %d, begin_evt: %u, end_evt: %u\n", tid, (unsigned int)begin_evt, (unsigned int)end_evt);
    while(true)
    {
        WaitForSingleObject(begin_evt, INFINITE);
        RenderTarget* rt = renderer->cur_render_target;
        EnterCriticalSection(&pp->cs);
//		printf("tile index: %d\n", pp->tile_index);
        if (pp->tile_index >= rt->tile_num)
        {
//			printf("tile %d finished!\n", pp->tile_index);
            SetEvent(end_evt);
            ResetEvent(begin_evt);
            LeaveCriticalSection(&pp->cs);
            continue;
        }
        tile = &rt->render_tiles[pp->tile_index];
		++pp->tile_index;
        LeaveCriticalSection(&pp->cs);

        switch(renderer->vertex_processor->primitive_type)
        {
            case PRIMITIVE_TRIANGLE:rasterize_triangle(renderer, tile);break;
            case PRIMITIVE_LINE:rasterize_line(renderer, tile);break;
            case PRIMITIVE_POINT:rasterize_point(renderer, tile);break;
            case PRIMITIVE_NONE:rasterize_none(renderer, tile, 0);break;
            default:break;
        }
    }
    return 0;
}

static void interp4d(Vector2D* v1, Vector2D* v2, Vector2D* v3, Uint32 data_offset,
	float s1, float s2, float s3, Vector4D* out)
{
	Vector4D* c1 = (Uint8*)v1 + data_offset;
	Vector4D* c2 = (Uint8*)v2 + data_offset;
	Vector4D* c3 = (Uint8*)v3 + data_offset;

	Vector4D interp_c1, interp_c2, interp_c3;
	vector4d_scale(&interp_c1, c1, s1);
	vector4d_scale(&interp_c2, c2, s2);
	vector4d_scale(&interp_c3, c3, s3);

	out->x = interp_c1.x + interp_c2.x + interp_c3.x;
	out->y = interp_c1.y + interp_c2.y + interp_c3.y;
	out->z = interp_c1.z + interp_c2.z + interp_c3.z;
	out->w = interp_c1.w + interp_c2.w + interp_c3.w;
}

static void interp4d_reciprocal_z(Vector2D* v1, Vector2D* v2, Vector2D* v3, Uint32 data_offset, float rz_1, float rz_2, float rz_3,
	float s1, float s2, float s3, Vector4D* out, float z)
{
	Vector4D* iv_1 = (Uint8*)v1 + data_offset;
	Vector4D* iv_2 = (Uint8*)v2 + data_offset;
	Vector4D* iv_3 = (Uint8*)v3 + data_offset;

	Vector4D t1_recp_z1, t2_recp_z2, t3_recp_z3;
	memset(&t1_recp_z1, 0, 16);
	memset(&t2_recp_z2, 0, 16);
	memset(&t3_recp_z3, 0, 16);
	t1_recp_z1.x = iv_1->x * rz_1;
	t1_recp_z1.y = iv_1->y * rz_1;
	t1_recp_z1.z = iv_1->z * rz_1;
	t1_recp_z1.w = iv_1->w * rz_1;

	t2_recp_z2.x = iv_2->x * rz_2;
	t2_recp_z2.y = iv_2->y * rz_2;
	t2_recp_z2.z = iv_2->z * rz_2;
	t2_recp_z2.w = iv_2->w * rz_2;

	t3_recp_z3.x = iv_3->x * rz_3;
	t3_recp_z3.y = iv_3->y * rz_3;
	t3_recp_z3.z = iv_3->z * rz_3;
	t3_recp_z3.w = iv_3->w * rz_3;

	out->x = (t1_recp_z1.x * s1 + t2_recp_z2.x * s2 + t3_recp_z3.x * s3) * z;
	out->y = (t1_recp_z1.y * s1 + t2_recp_z2.y * s2 + t3_recp_z3.y * s3) * z;
	out->z = (t1_recp_z1.z * s1 + t2_recp_z2.z * s2 + t3_recp_z3.z * s3) * z;
	out->w = (t1_recp_z1.w * s1 + t2_recp_z2.w * s2 + t3_recp_z3.w * s3) * z;

// 	vector4d_scale(&t1_recp_z1, iv_1, rz_1);
// 	vector4d_scale(&t2_recp_z2, iv_2, rz_2);
// 	vector4d_scale(&t3_recp_z3, iv_3, rz_3);
// 
// 	Vector4D interp_t1, interp_t2, interp_t3;
// 	vector4d_scale(&interp_t1, &t1_recp_z1, s1);
// 	vector4d_scale(&interp_t2, &t2_recp_z2, s2);
// 	vector4d_scale(&interp_t3, &t3_recp_z3, s3);
// 
// 	out->x = interp_t1.x + interp_t2.x + interp_t3.x;
// 	out->y = interp_t1.y + interp_t2.y + interp_t3.y;
// 	out->z = interp_t1.z + interp_t2.z + interp_t3.z;
// 	out->w = interp_t1.w + interp_t2.w + interp_t3.w;
// 
// 	out->x = out->x * z;
// 	out->y = out->y * z;
// 	out->z = out->z * z;
// 	out->w = out->w * z;
}

static void rast(ServalRenderer* renderer, float pixel_x, float pixel_y,
	Vector2D* v1, Vector2D* v2, Vector2D* v3, float main_triangle_area,
	Vector4D* final_color)
{
	if (intersect_test_point_triangle(v1->x, v1->y, v2->x, v2->y, v3->x, v3->y, pixel_x, pixel_y))
	{
		Vector2D pixel_v;
		pixel_v.x = pixel_x;
		pixel_v.y = pixel_y;

		float area_a = triangle_area2D(v1, v2, &pixel_v);    // v3
		float area_b = triangle_area2D(v2, v3, &pixel_v);    // v1
		float area_c = triangle_area2D(v3, v1, &pixel_v);    // v2

		if (main_triangle_area == 0.0f)
			main_triangle_area = 1.0f;

		float scale_3 = area_a / main_triangle_area;
		float scale_1 = area_b / main_triangle_area;
		float scale_2 = area_c / main_triangle_area;

		float reciprocal_z_3 = ((Vector3D*)v3)->z == 0.0f ? 1.0f : 1.0f / ((Vector3D*)v3)->z;
		float reciprocal_z_2 = ((Vector3D*)v2)->z == 0.0f ? 1.0f : 1.0f / ((Vector3D*)v2)->z;
		float reciprocal_z_1 = ((Vector3D*)v1)->z == 0.0f ? 1.0f : 1.0f / ((Vector3D*)v1)->z;

		Vector4D interp_color, interp_texcoord, interp_normal, interp_tangent, interp_pos;
		float interp_reciprocal_z = scale_1 * reciprocal_z_1 + scale_2 * reciprocal_z_2 + scale_3 * reciprocal_z_3;

		VertexDeclaration* decl = renderer->vertex_processor->vertex_decl;
		while (decl->semantic != DECL_NONE)
		{
			if (decl->semantic == DECL_COLOR)
			{
				interp4d_reciprocal_z(v1, v2, v3, decl->offset, reciprocal_z_1, reciprocal_z_2, reciprocal_z_3,
					scale_1, scale_2, scale_3, &interp_color, 1.0f / interp_reciprocal_z);
//				interp4d(v1, v2, v3, decl->offset, scale_1, scale_2, scale_3, &interp_color);
			}
			else if (decl->semantic == DECL_TEXCOORD)
			{
				// affine projection
//				interp4d(v1, v2, v3, decl->offset, scale_1, scale_2, scale_3, &interp_texcoord);
				
				// perspective projection
				interp4d_reciprocal_z(v1, v2, v3, decl->offset, reciprocal_z_1, reciprocal_z_2, reciprocal_z_3,
  					scale_1, scale_2, scale_3, &interp_texcoord, 1.0f / interp_reciprocal_z);
//				printf("x: %f, y: %f, u: %f, v: %f\n", pixel_x, pixel_y, interp_texcoord.x, interp_texcoord.y);
			}
			else if (decl->semantic == DECL_NORMAL)
			{
				interp4d_reciprocal_z(v1, v2, v3, decl->offset, reciprocal_z_1, reciprocal_z_2, reciprocal_z_3,
					scale_1, scale_2, scale_3, &interp_normal, 1.0f / interp_reciprocal_z);
			}
			else if (decl->semantic == DECL_TANGENT)
			{
				interp4d_reciprocal_z(v1, v2, v3, decl->offset, reciprocal_z_1, reciprocal_z_2, reciprocal_z_3,
					scale_1, scale_2, scale_3, &interp_tangent, 1.0f / interp_reciprocal_z);
			}
			else if (decl->semantic == DECL_POSITION0)
			{
				interp4d_reciprocal_z(v1, v2, v3, decl->offset, reciprocal_z_1, reciprocal_z_2, reciprocal_z_3,
					scale_1, scale_2, scale_3, &interp_pos, 1.0f / interp_reciprocal_z);
			}
			++decl;
		}

		// do some fragment things such as alpha\depth test, alpha blend, lighting
		bool alpha_test_cert = false;
		bool z_test_cert = false;

		float z_value = *(float*)render_target_get_buffer_value(renderer->cur_render_target, (Uint32)pixel_x, (Uint32)pixel_y, BUFFER_DEPTH);
		DWORD color_value = *(Uint32*)render_target_get_buffer_value(renderer->cur_render_target, (Uint32)pixel_x, (Uint32)pixel_y, BUFFER_COLOR);
		
		memset(final_color, 0, sizeof(Vector4D));

		if (renderer->cur_render_state.enable_z_test)
		{
			switch (renderer->cur_render_state.z_test_func)
			{
			case CMP_GREAT:
				if (z_value > interp_reciprocal_z)
				{
					z_test_cert = true;
				}
				break;
			case CMP_LESS:
				if (z_value < interp_reciprocal_z)
				{
					z_test_cert = true;
				}
				break;
			case CMP_GREAT_EQUAL:
				if (z_value >= interp_reciprocal_z)
				{
					z_test_cert = true;
				}
				break;
			case CMP_LESS_EQUAL:
				if (z_value <= interp_reciprocal_z)
				{
					z_test_cert = true;
				}
			case CMP_ALWAYS:
				z_test_cert = true;
				break;
			}
		}

		if (z_test_cert)
		{
			//
			if (renderer->cur_render_state.enable_lighting)
			{
				int i = 0;
				Vector4D emissive_color, ambient_color, diffuse_color, specular_color;
				memset(&specular_color, 0, sizeof(specular_color));
				for (i = 0; i < LIGHTS_NUM; ++i)
				{
					SLight* light = &renderer->lights[i];
					if (light->enable)
					{
						if (light->type == POINT_LIGHT)
						{
							Vector3D vertex_2_light, vertex_2_eye, eye_pos = { 0.0f, 0.0f, 0.0f }, reflect_ld;
							Vector3D v2l_normalize, nor_normalize;
							vector3d_sub(&vertex_2_light, (Vector3D*)&interp_pos, (Vector3D*)&light->position);
							float vl_dist = vector3d_length(&vertex_2_light);
							if (vl_dist > light->range)
								continue;;
							vector3d_normalize(&v2l_normalize, &vertex_2_light);
							vector3d_normalize(&nor_normalize, (Vector4D*)&interp_normal);
							float diffuse_factor = vector3d_dot(&nor_normalize, &v2l_normalize);
							diffuse_factor = Max(diffuse_factor, 0.0f);
							Vector4D light_diffuse;
							vector4d_scale(&light_diffuse, &light->diffuse, diffuse_factor);
							vector4d_mul(&diffuse_color, &light_diffuse, &renderer->cur_material.diffuse);
							vector4d_mul(&ambient_color, &light->ambient, &renderer->cur_material.ambient);
							if (renderer->cur_render_state.enable_specular)
							{
								Vector3D half_normal, temp_v, neg_pl;
								vector3d_scale(&neg_pl, &v2l_normalize, -1.0f);
								vector3d_scale(&half_normal, &nor_normalize, 0.5f);
								vector3d_add(&temp_v, &neg_pl, &half_normal);
								vector3d_add(&reflect_ld, &nor_normalize, &temp_v);
								vector3d_normalize(&reflect_ld, &reflect_ld);
								vector3d_sub(&vertex_2_eye, (Vector3D*)&interp_pos, (Vector3D*)&eye_pos);
								vector3d_normalize(&vertex_2_eye, &vertex_2_eye);
								float specular_factor = vector3d_dot(&vertex_2_eye, &reflect_ld) * renderer->cur_material.pow;
								specular_factor = Max(specular_factor, 0.0f);
								vector4d_scale(&specular_color, &light->specular, specular_factor);
								vector4d_mul(&specular_color, &specular_color, &renderer->cur_material.specular);
							}
						}
						else if (light->type == DIRECTIONAL_LIGHT)
						{
							Vector3D reflect_ld, vertex_2_eye, eye_pos = { 0.0f, 0.0f, 0.0f };
							Vector3D ld_normalize, nor_normalize;

							vector3d_normalize(&ld_normalize, &light->direction);
							vector3d_normalize(&nor_normalize, (Vector4D*)&interp_normal);
							float diffuse_factor = vector3d_dot(&nor_normalize, &ld_normalize);
							diffuse_factor = Max(diffuse_factor, 0.0f);
							Vector4D light_diffuse;
							vector4d_scale(&light_diffuse, &light->diffuse, diffuse_factor);
							vector4d_mul(&diffuse_color, &light_diffuse, &renderer->cur_material.diffuse);
							vector4d_mul(&ambient_color, &light->ambient, &renderer->cur_material.ambient);
							if (renderer->cur_render_state.enable_specular)
							{
								Vector3D half_normal, temp_v;
								vector3d_scale(&half_normal, &nor_normalize, 0.5f);
								vector3d_add(&temp_v, &light->direction, &half_normal);
								vector3d_add(&reflect_ld, &nor_normalize, &temp_v);
								vector3d_normalize(&reflect_ld, &reflect_ld);
								vector3d_sub(&vertex_2_eye, (Vector3D*)&interp_pos, (Vector3D*)&eye_pos);
								vector3d_normalize(&vertex_2_eye, &vertex_2_eye);
								float specular_factor = vector3d_dot(&vertex_2_eye, &reflect_ld) * renderer->cur_material.pow;
								specular_factor = Max(0.0f, specular_factor);
								vector4d_scale(&specular_color, &light->specular, specular_factor);
								vector4d_mul(&specular_color, &specular_color, &renderer->cur_material.specular);
							}
						}
						final_color->x = renderer->cur_material.emissive.x + ambient_color.x + diffuse_color.x + specular_color.x;
						final_color->y = renderer->cur_material.emissive.y + ambient_color.y + diffuse_color.y + specular_color.y;
						final_color->z = renderer->cur_material.emissive.z + ambient_color.z + diffuse_color.z + specular_color.z;
						final_color->w = renderer->cur_material.emissive.w;
						clamp_f32(&final_color->x, final_color->x, 0.0f, 1.0f);
						clamp_f32(&final_color->y, final_color->y, 0.0f, 1.0f);
						clamp_f32(&final_color->z, final_color->z, 0.0f, 1.0f);
						clamp_f32(&final_color->w, final_color->w, 0.0f, 1.0f);
					}
				}
			}
			else
			{
				memcpy(final_color, &interp_color, sizeof(Vector4D));
			}

			if (renderer->cur_render_state.enable_alpha_test)
			{
				Uint32 alpha_c = GET_ALPHA8(color_value);
				float fa = COLOR8_TO_FLT(alpha_c);
				switch (renderer->cur_render_state.alpha_test_func)
				{
				case CMP_LESS:
					if (fa > final_color->w)
					{
						alpha_test_cert = true;
					}
					break;
				case CMP_GREAT:
					if (fa < final_color->w)
					{
						alpha_test_cert = true;
					}
					break;
				case CMP_LESS_EQUAL:
					if (fa >= final_color->w)
					{
						alpha_test_cert = true;
					}
					break;
				case CMP_GREAT_EQUAL:
					if (fa <= final_color->w)
					{
						alpha_test_cert = true;
					}
				case CMP_ALWAYS:
					alpha_test_cert = true;
					break;
				}
			}

			if (alpha_test_cert)
			{
				Vector4D dest_color;
				dest_color.x = GET_ALPHA8(color_value);
				dest_color.y = GET_RED8(color_value);
				dest_color.z = GET_GREEN8(color_value);
				dest_color.w = GET_BLUE8(color_value);
				if (renderer->cur_render_state.enable_alpha_blend)
				{
					alpha_blend_func(final_color, &dest_color, final_color, renderer->cur_render_state.alpha_func);
				}

				// tex-coord
				if (renderer->cur_render_state.enable_texture)
				{
					Uint32 tex_color = sample_texture2D(&renderer->sampler, interp_texcoord.x, interp_texcoord.y, renderer->sampler.sampler_type);
					Vector4D tex_color_f;
					u32_color_to_flt_color(tex_color, &tex_color_f);
					vector4d_mul(final_color, &interp_color, &tex_color_f);
//					memcpy(final_color, &tex_color_f, 16);

// 					Uint32 px = (Uint32)pixel_x;
// 					Uint32 py = (Uint32)pixel_y;
// 					Uint32* color_ptr = (Uint32*)renderer->cur_render_target->render_buffer->data + py * renderer->cur_render_target->width + px;
// 					*color_ptr = tex_color;
//					printf("x: %u, y: %u --> u: %f, v:%f, a:%u,r:%u,g:%u,b:%u\n", px, py, interp_texcoord.x, interp_texcoord.y, GET_ALPHA8(tex_color), GET_RED8(tex_color), GET_GREEN8(tex_color), GET_BLUE8(tex_color));
				}
			}

			if (alpha_test_cert && z_test_cert)
			{
				// write pixel to render target and 1/z to depth buffer
				Uint32 px = (Uint32)pixel_x;
				Uint32 py = (Uint32)pixel_y;

				Uint32* color_ptr = (Uint32*)renderer->cur_render_target->render_buffer->data + py * renderer->cur_render_target->width + px;
				float* depth_buffer = (float*)renderer->cur_render_target->depth_buffer->data + py * renderer->cur_render_target->width + px;

				flt_color_to_u32_color(final_color, color_ptr);
				*depth_buffer = interp_reciprocal_z;
			}
		}
	}
}

static void het_rasterization_for_triangle(ServalRenderer* renderer, float left, float right, float top, float bottom,
	Vector2D* v1, Vector2D* v2, Vector2D* v3, float quad_len)
{
	// do test & reject
	if (!intersect_test_triangle_rect(v1->x, v1->y, v2->x, v2->y, v3->x, v3->y, left + 0.5f, right + 0.5f, top + 0.5f, bottom + 0.5f))
		return;
	//
	if (quad_len == 1)
	{
		Vector4D final_color;
		float pixel_x = left + 0.5f, pixel_y = top + 0.5f;
		float main_triangle_area = triangle_area2D(v1, v2, v3);
		rast(renderer, pixel_x, pixel_y, v1, v2, v3, main_triangle_area, &final_color);
	}
	else
	{
		float half_size = quad_len * 0.5f;
		float cx = left + half_size;
		float cy = top + half_size;
		het_rasterization_for_triangle(renderer, left, cx, top, cy, v1, v2, v3, half_size);
		het_rasterization_for_triangle(renderer, cx, right, top, cy, v1, v2, v3, half_size);
		het_rasterization_for_triangle(renderer, left, cx, cy, bottom, v1, v2, v3, half_size);
		het_rasterization_for_triangle(renderer, cx, right, cy, bottom, v1, v2, v3, half_size);
	}
}

static void full_tile_rasterization(ServalRenderer* renderer, float left, float right, float top, float bottom,
	Vector2D* v1, Vector2D* v2, Vector2D* v3)
{
	int i = 0, j = 0;
	float pixel_x = left + 0.5f, pixel_y = top + 0.5f;
	float main_triangle_area = triangle_area2D(v1, v2, v3);
	Vector4D final_color;
	for (i = top; i <= bottom; ++i)
	{
		pixel_y = (float)i + 0.5f;
		for (j = left; j <= right; ++j)
		{
			pixel_x = (float)j + 0.5f;

			rast(renderer, pixel_x, pixel_y, v1, v2, v3, main_triangle_area, &final_color);
		}
	}
}

static void rasterize_triangle(ServalRenderer* renderer, Rasterize_Tile* tile)
{
	int i = 0;
	for (i = 0; i < tile->primitive_count; ++i)
	{
		Vector2D* v1 = (Vector2D*)tile->primitive_indices[i]->vertex_ptr;
		Vector2D* v2 = (Vector2D*)tile->primitive_indices[i]->next->vertex_ptr;
		Vector2D* v3 = (Vector2D*)tile->primitive_indices[i]->next->next->vertex_ptr;

		// is tile in triangle?
		if (intersect_test_point_triangle(v1->x, v1->y, v2->x, v2->y, v3->x, v3->y, tile->left + 0.5f, tile->top + 0.5f) &&
			intersect_test_point_triangle(v1->x, v1->y, v2->x, v2->y, v3->x, v3->y, tile->right + 0.5f, tile->top + 0.5f) &&
			intersect_test_point_triangle(v1->x, v1->y, v2->x, v2->y, v3->x, v3->y, tile->left + 0.5f, tile->bottom + 0.5f) &&
			intersect_test_point_triangle(v1->x, v1->y, v2->x, v2->y, v3->x, v3->y, tile->right + 0.5f, tile->bottom + 0.5f))
		{
			//rasterize_none(renderer, tile, 128 << 16);
			full_tile_rasterization(renderer, tile->left, tile->right, tile->top, tile->bottom, v1, v2, v3);
		}
		//
		else
		{
			//rasterize_none(renderer, tile, 0);
			het_rasterization_for_triangle(renderer, tile->left, tile->right, tile->top, tile->bottom, v1, v2, v3, RASTERIZE_TILE_SIZE);
		}
	}
}

static void rasterize_line(ServalRenderer* renderer, Rasterize_Tile* tile)
{

}

static void rasterize_point(ServalRenderer* renderer, Rasterize_Tile* tile)
{

}

static void rasterize_none(ServalRenderer* renderer, Rasterize_Tile* tile, Uint32 c)
{
    RenderTarget* rt = renderer->cur_render_target;
    srand(ctime());
    fill_rect_u32(rt->render_buffer, rand()%255 + c, tile->left, tile->right, tile->top, tile->bottom);
}
