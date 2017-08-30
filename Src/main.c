#define SDL_MAIN_HANDLED

#include "renderer.h"
#include "smath.h"

typedef struct _TestVertex
{
	Vector4D position;
	Vector4D position0;	// for lighting
	Vector4D color;
	Vector4D texcoord;
}TestVertex;

typedef struct _TestTriangle
{
	TestVertex v1, v2, v3;
}TestTriangle;

typedef struct _TestQuad
{
	TestVertex v1, v2, v3, v4, v5, v6;
}TestQuad;

VertexDeclaration g_decl[] = {
	VERTEX_DECL_ELEM(DECL_POSITION, 0, 16),
	VERTEX_DECL_ELEM(DECL_POSITION0, 16, 16),	// for lighting
	VERTEX_DECL_ELEM(DECL_COLOR, 32, 16),
	VERTEX_DECL_ELEM(DECL_TEXCOORD, 48, 16),
	VERTEX_DECL_END()
};

TestTriangle g_triangle;
TestQuad g_quad;
STexture g_texture;
SLight g_point_light;
SLight g_dir_light;
Matrix4x4 g_world_mat;
Matrix4x4 g_view_mat;
Matrix4x4 g_proj_mat;
Matrix4x4 g_view_port_mat;

void set_triangle()
{
	g_triangle.v1.position.x = 0.0f;
	g_triangle.v1.position.y = 5.0f;
	g_triangle.v1.position.z = 5.0f;
	g_triangle.v1.position.w = 1.0f;
	g_triangle.v1.color.x = COLOR8_TO_FLT(255);
	g_triangle.v1.color.y = COLOR8_TO_FLT(255);
	g_triangle.v1.color.z = COLOR8_TO_FLT(0);
	g_triangle.v1.color.w = COLOR8_TO_FLT(0);
	memcpy(&g_triangle.v1.position0, &g_triangle.v1.position, sizeof(Vector4D));
	g_triangle.v1.texcoord.x = 0.0f;
	g_triangle.v1.texcoord.y = 0.0f;
	g_triangle.v1.texcoord.z = 1.0f;
	g_triangle.v1.texcoord.w = 1.0f;

	g_triangle.v2.position.x = -5.0f;
	g_triangle.v2.position.y = 5.0f;
	g_triangle.v2.position.z = 10.0f;
	g_triangle.v2.position.w = 1.0f;
	g_triangle.v2.color.x = COLOR8_TO_FLT(255);
	g_triangle.v2.color.y = COLOR8_TO_FLT(0);
	g_triangle.v2.color.z = COLOR8_TO_FLT(255);
	g_triangle.v2.color.w = COLOR8_TO_FLT(0);
	memcpy(&g_triangle.v2.position0, &g_triangle.v2.position, sizeof(Vector4D));
	g_triangle.v2.texcoord.x = 1.0f;
	g_triangle.v2.texcoord.y = 0.0f;
	g_triangle.v2.texcoord.z = 1.0f;
	g_triangle.v2.texcoord.w = 1.0f;

	g_triangle.v3.position.x = 5.0f;
	g_triangle.v3.position.y = 5.0f;
	g_triangle.v3.position.z = 15.0f;
	g_triangle.v3.position.w = 1.0f;
	g_triangle.v3.color.x = COLOR8_TO_FLT(255);
	g_triangle.v3.color.y = COLOR8_TO_FLT(0);
	g_triangle.v3.color.z = COLOR8_TO_FLT(0);
	g_triangle.v3.color.w = COLOR8_TO_FLT(255);
	memcpy(&g_triangle.v3.position0, &g_triangle.v3.position, sizeof(Vector4D));
	g_triangle.v3.texcoord.x = 0.0f;
	g_triangle.v3.texcoord.y = 1.0f;
	g_triangle.v3.texcoord.z = 1.0f;
	g_triangle.v3.texcoord.w = 1.0f;

}

void set_quad()
{
	g_quad.v1.position.x = -5.0f;
	g_quad.v1.position.y = 5.0f;
	g_quad.v1.position.z = 15.0f;
	g_quad.v1.position.w = 1.0f;
	g_quad.v1.color.x = COLOR8_TO_FLT(255);
	g_quad.v1.color.y = COLOR8_TO_FLT(255);
	g_quad.v1.color.z = COLOR8_TO_FLT(0);
	g_quad.v1.color.w = COLOR8_TO_FLT(0);
	memcpy(&g_quad.v1.position0, &g_quad.v1.position, sizeof(Vector4D));
	g_quad.v1.texcoord.x = 0.0f;
	g_quad.v1.texcoord.y = 0.0f;
	g_quad.v1.texcoord.z = 0.0f;
	g_quad.v1.texcoord.w = 0.0f;

	g_quad.v2.position.x = 5.0f;
	g_quad.v2.position.y = 5.0f;
	g_quad.v2.position.z = 20.0f;
	g_quad.v2.position.w = 1.0f;
	g_quad.v2.color.x = COLOR8_TO_FLT(255);
	g_quad.v2.color.y = COLOR8_TO_FLT(0);
	g_quad.v2.color.z = COLOR8_TO_FLT(255);
	g_quad.v2.color.w = COLOR8_TO_FLT(0);
	memcpy(&g_quad.v2.position0, &g_quad.v2.position, sizeof(Vector4D));
	g_quad.v2.texcoord.x = 1.0f;
	g_quad.v2.texcoord.y = 0.0f;
	g_quad.v2.texcoord.z = 0.0f;
	g_quad.v2.texcoord.w = 0.0f;

	g_quad.v3.position.x = 5.0f;
	g_quad.v3.position.y = -5.0f;
	g_quad.v3.position.z = 20.0f;
	g_quad.v3.position.w = 1.0f;
	g_quad.v3.color.x = COLOR8_TO_FLT(255);
	g_quad.v3.color.y = COLOR8_TO_FLT(0);
	g_quad.v3.color.z = COLOR8_TO_FLT(0);
	g_quad.v3.color.w = COLOR8_TO_FLT(255);
	memcpy(&g_quad.v3.position0, &g_quad.v3.position, sizeof(Vector4D));
	g_quad.v3.texcoord.x = 1.0f;
	g_quad.v3.texcoord.y = 1.0f;
	g_quad.v3.texcoord.z = 0.0f;
	g_quad.v3.texcoord.w = 0.0f;

	g_quad.v4.position.x = 5.0f;
	g_quad.v4.position.y = -5.0f;
	g_quad.v4.position.z = 20.0f;
	g_quad.v4.position.w = 1.0f;
	g_quad.v4.color.x = COLOR8_TO_FLT(255);
	g_quad.v4.color.y = COLOR8_TO_FLT(0);
	g_quad.v4.color.z = COLOR8_TO_FLT(0);
	g_quad.v4.color.w = COLOR8_TO_FLT(255);
	memcpy(&g_quad.v4.position0, &g_quad.v4.position, sizeof(Vector4D));
	g_quad.v4.texcoord.x = 1.0f;
	g_quad.v4.texcoord.y = 1.0f;
	g_quad.v4.texcoord.z = 0.0f;
	g_quad.v4.texcoord.w = 0.0f;

	g_quad.v5.position.x = -5.0f;
	g_quad.v5.position.y = -5.0f;
	g_quad.v5.position.z = 15.0f;
	g_quad.v5.position.w = 1.0f;
	g_quad.v5.color.x = COLOR8_TO_FLT(255);
	g_quad.v5.color.y = COLOR8_TO_FLT(0);
	g_quad.v5.color.z = COLOR8_TO_FLT(255);
	g_quad.v5.color.w = COLOR8_TO_FLT(0);
	memcpy(&g_quad.v5.position0, &g_quad.v5.position, sizeof(Vector4D));
	g_quad.v5.texcoord.x = 0.0f;
	g_quad.v5.texcoord.y = 1.0f;
	g_quad.v5.texcoord.z = 0.0f;
	g_quad.v5.texcoord.w = 0.0f;

	g_quad.v6.position.x = -5.0f;
	g_quad.v6.position.y = 5.0f;
	g_quad.v6.position.z = 15.0f;
	g_quad.v6.position.w = 1.0f;
	g_quad.v6.color.x = COLOR8_TO_FLT(255);
	g_quad.v6.color.y = COLOR8_TO_FLT(255);
	g_quad.v6.color.z = COLOR8_TO_FLT(0);
	g_quad.v6.color.w = COLOR8_TO_FLT(0);
	memcpy(&g_quad.v6.position0, &g_quad.v6.position, sizeof(Vector4D));
	g_quad.v6.texcoord.x = 0.0f;
	g_quad.v6.texcoord.y = 0.0f;
	g_quad.v6.texcoord.z = 0.0f;
	g_quad.v6.texcoord.w = 0.0f;
}

void set_lights()
{
	g_point_light.ambient.x = 0.25f;
	g_point_light.ambient.y = 0.25f;
	g_point_light.ambient.z = 0.25f;
	g_point_light.ambient.w = 1.0f;

	g_point_light.diffuse.x = 0.0f;
	g_point_light.diffuse.y = 0.0f;
	g_point_light.diffuse.z = 0.5f;
	g_point_light.diffuse.w = 1.0f;

	g_point_light.specular.x = 0.5f;
	g_point_light.specular.y = 0.0f;
	g_point_light.specular.z = 0.0f;
	g_point_light.specular.w = 1.0f;

	g_point_light.position.x = 0.0f;
	g_point_light.position.y = 5.0f;
	g_point_light.position.z = 15.0f;
	g_point_light.position.w = 1.0f;
	g_point_light.range = 5.0f;
}

void Create(ServalRenderer* renderer)
{

	serval_set_vertex_declaration(renderer, g_decl);

	set_triangle();

	set_quad();

	set_lights();

	load_texture_from_png(&g_texture, "test.png");

	serval_enable_z_test(renderer, true);
	serval_enable_alpha_test(renderer, true);
	serval_set_alpha_test_func(renderer, CMP_ALWAYS);
	serval_set_z_test_func(renderer, CMP_LESS);
	serval_enable_lighting(renderer, false);
	
	// texture sampler 
	serval_set_texture_sample_type(renderer, DOUBLE_LINEAR);

	// enable texture
	serval_enable_texture(renderer, true);
	serval_set_texture(renderer, &g_texture);

	// enable specular lighting
	serval_enable_specular(renderer, false);

	serval_set_light(renderer, &g_point_light, 0);

	Vector3D eye = {0.0f, 0.0f, 0.0f};
	Vector3D lookat = {0.0f, 0.0f, 1.0f};
	Vector3D up = {0.0f, 1.0f, 0.0f};
	matrix_look_at(&g_view_mat, &eye, &lookat, &up);

	matrix_perspective(&g_proj_mat, ANGLE_TO_RADIAN(90), 640.0f / 480.0f, 10.0f, 500.0f);

	matrix_view_port(&g_view_port_mat, 0.0f, 0.0f, 640.0f, 480.0f);

	matrix_set_identity(&g_world_mat);

	serval_set_world_matrix(renderer, (float*)&g_world_mat);
	serval_set_view_matrix(renderer, (float*)&g_view_mat);
	serval_set_proj_matrix(renderer, (float*)&g_proj_mat);
	serval_set_view_port_matrix(renderer, (float*)&g_view_port_mat);
}

void Destroy(ServalRenderer* renderer)
{

}

void FrameRender(ServalRenderer* renderer, float total_time, float etime)
{

}

void FrameMove(ServalRenderer* renderer, float total_time, float etime)
{
	serval_set_vertex_buffer(renderer, (void*)&g_quad, 6, sizeof(TestVertex), PRIMITIVE_TRIANGLE);
}

void Keyboard(ServalRenderer* renderer, SDL_Event* e)
{

}

void Mouse(ServalRenderer* renderer, SDL_Event* e)
{

}

int main(int argv, char** args)
{
//    if (intersect_test_triangle_rect(2.0f,3.0f,
//                                     1.0f,0.0f,
//                                     2.0f,0.0f,
//                                     0.5f,1.5f,2.0f,3.0f))
//        printf("intersect!\n");

	bool it = intersect_test_point_triangle(240, 160, 380, 180, 380, 300, 300, 200);

    ServalRenderer* renderer = serval_create(100, 100, 640, 480, "serval renderer",
                                             Create, Destroy, FrameMove, FrameRender, Keyboard, Mouse);
    serval_run(renderer);

    serval_destroy(renderer);

    return 0;
}
