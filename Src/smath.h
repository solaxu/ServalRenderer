#ifndef SMATH_H_INCLUDED
#define SMATH_H_INCLUDED

/*
*   date: 2017.08.18
*   author: solaxu
*   desc: intersect test
*/

#include <stdbool.h>
#include "SDL2/SDL.h"

#include <mmintrin.h>//mmx
#include <xmmintrin.h>//sse
#include <emmintrin.h>//sse2
#include <pmmintrin.h>//sse3

#define FltTolerance 0.00005f
#define MIN_FLT -999999999.0f
#define MAX_FLT 999999999.0f

#define IsPow2(a) ((((a)-1)&(a)) == 0)

#define IsZeroFlt(a) ((a)>-FltTolerance && (a)< FltTolerance)
#define IsEqualFlt(a,b) IsZeroFlt((a)-(b))

#define Min(a,b)  ((a) > (b)) ? (b) : (a)
#define Max(a,b)  ((a) > (b)) ? (a) : (b)

#define S_PI 3.141592657
#define S_2PI (2*S_PI)
#define S_RADIAN (3.1415926/180.0)
#define S_ANGLE (180.0f/3.1415926)
#define ANGLE_TO_RADIAN(angle) ((angle)*S_RADIAN)
#define RADIAN_TO_ANGLE(radian) ((radian)*S_ANGLE)

#define GET_RED8(c) (((c)&0x00ff0000)>>16)
#define GET_BLUE8(c) ((c)&0x000000ff)
#define GET_GREEN8(c) (((c)&0x0000ff00)>>8)
#define GET_ALPHA8(c) (((c)&0xff000000)>>24)

#define ARGB32_COLOR(a,r,g,b) ((((a)&0xff)<<24) | (((r)&0xff)<<16) | (((g)&255)<<8) | ((b)&0xff))

#define COLOR8_TO_FLT(c) ((float)(c) * 0.003921567f)

#define FLT_TO_COLOR8(c) ((unsigned int)(c * 255.0f))

#define INTERP_FLT(begin,end,ratio) ((begin) + ((end) - (begin)) * (1.0f - (ratio)))

#define IN_NEGATIVE_SAPCE 0
#define IN_POSITIVE_SAPCE 1
#define INTERSECT_PLANE 2

#pragma pack (16)

typedef struct _AABBox2D
{
    float left;
    float right;
    float top;
    float bottom;
}AABBox2D;

typedef struct _Vector2D
{
    union{
		float v[2];
		struct 
		{
			float x;
			float y;
		};
    };
}Vector2D;

typedef struct _Vector3D
{
    union{
		float v[3];
		struct 
		{
			float x;
			float y;
			float z;
		};
    };
}Vector3D;

typedef struct _Vertor4D
{
    union{
		float v[4];
		struct 
		{
			float x, y, z, w;
		};
    };
}Vector4D;

typedef struct _Matrix4x4
{
    union{
		float mat[16];
        struct  
        {
			float m00, m01, m02, m03;
			float m10, m11, m12, m13;
			float m20, m21, m22, m23;
			float m30, m31, m32, m33;
        };
    };
}Matrix4x4;

typedef struct _Plane3D
{
    union{
		float p[4];
		struct  
		{
			float x, y, z, d;
		};
    };
}Plane3D;

// test triangle and rectangle
bool intersect_test_triangle_rect(float x1, float y1,
                             float x2, float y2,
                             float x3, float y3,
                             float left, float right, float top, float bottom);

// test point and rectangle
bool intersect_test_point_rect(float x, float y,
                          float left, float right, float top, float bottom);

// test line and rectangle
bool intersect_test_line_rect(float x1, float y1,
                              float x2, float y2,
                              float left, float right, float top, float bottom, bool is_segment);

// test point and triangle
bool intersect_test_point_triangle(float x1, float y1,
                             float x2, float y2,
                             float x3, float y3,
                             float px, float py);

float calculate_line_equation(float x1, float y1,
                              float x2, float y2,
                              float px, float py);

void calculate_aabbox2d_triagle(float x1, float y1,
                                       float x2, float y2,
                                       float x3, float y3,
                                       AABBox2D* aabbox2d);

void calculate_aabbox2d_line(float x1, float y1,
                                    float x2, float y2,
                                    AABBox2D* aabbox2d);

bool intersect_aab_rects(float left1, float right1, float top1, float bottom1,
                            float left2, float right2, float top2, float bottom2);

float triangle_area2D(Vector2D* v1, Vector2D* v2, Vector2D* v3);

float vector2d_dot(Vector2D* v1, Vector2D* v2);
float vector2d_length(Vector2D* v);
void vector2d_normalize(Vector2D* v);
void vector2d_add(Vector2D* out, Vector2D* v1, Vector2D* v2);
void vector2d_sub(Vector2D* out, Vector2D* v1, Vector2D* v2);
bool vector2d_equal(Vector2D* v1, Vector2D* v2);

void vector3d_scale(Vector3D* out, Vector3D* v, float scale);
void vector3d_cross(Vector3D* out, Vector3D* v1, Vector3D* v2);
float vector3d_length(Vector3D* v);
void vector3d_normalize(Vector3D* v);
float vector3d_dot(Vector3D* v1, Vector3D* v2);
void vector3d_sub(Vector3D* out, Vector3D* v1, Vector3D* v2);
void vector3d_add(Vector3D* out, Vector3D* v1, Vector3D* v2);
bool vector3d_equal(Vector3D* v1, Vector3D* v2);
void vector3d_transform_coord(Vector3D* out, Vector3D* v, Matrix4x4* mat44);
void vector3d_transform_normal(Vector3D* out, Vector3D* v, Matrix4x4* mat44);

void vector4d_transform_coord(Vector4D* out, Vector4D* v, Matrix4x4* mat44);
void vector4d_transform_normal(Vector4D* out, Vector4D* v, Matrix4x4* mat44);
void vector4d_scale(Vector4D* out, Vector4D* v, float scale);
void vector4d_mul(Vector4D* out, Vector4D* v1, Vector4D* v2);
void vector4d_add(Vector4D* out, Vector4D* v1, Vector4D* v2);

void u32_color_to_flt_color(Uint32 uc, Vector4D* fc);
void flt_color_to_u32_color(Vector4D* fc, Uint32* uc);

__m128 matrix_get_col(Matrix4x4* mat44, int c);
__m128 matrix_get_row(Matrix4x4* mat44, int r);
void matrix_set_identity(Matrix4x4* mat44);
void matrix_mul(Matrix4x4* dest, Matrix4x4* m1, Matrix4x4* m2);
void matrix_look_at(Matrix4x4* dest, Vector3D* pos, Vector3D* lookat, Vector3D* up);
void matrix_perspective(Matrix4x4* dest, float fov, float aspect, float near, float far);
void matrix_view_port(Matrix4x4* dest, float x, float y, float width, float height);
void matrix_translate(Matrix4x4* dest, float x, float y, float z);
void matrix_scale(Matrix4x4* dest, float x, float y, float z);
void matrix_rotate_x(Matrix4x4* dest, float radian);
void matrix_rotate_y(Matrix4x4* dest, float radian);
void matrix_rotate_z(Matrix4x4* dest, float radian);
void matrix_rotate_axis(Matrix4x4* dest, Vector3D* axis, float radian);
void matrix_get_view_dir_from_view_mat(Vector3D* dir, Matrix4x4* viewMat);
void matrix_get_view_right_from_view_mat(Vector3D* right, Matrix4x4* viewMat);
void matrix_get_view_up_from_view_mat(Vector3D* up, Matrix4x4* viewMat);

void clamp_f32(float* out, float v, float l, float r);
void clamp_i32(unsigned int* out, unsigned int v, unsigned int l, unsigned int r);
void clamp_u32(int* out, int v, int l, int r);

void plane3d_from_points(Plane3D* plane, Vector3D* v1, Vector3D* v2, Vector3D* v3);
#endif // INTERSECT_DECT_H_INCLUDED
