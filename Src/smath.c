#include "smath.h"
#include <memory.h>
#include <math.h>
#include <stdlib.h>

float calculate_line_equation(float x1, float y1,
                              float x2, float y2,
                              float px, float py)
{
    return (px - x1) * (y2 - y1) - (py - y1) * (x2 - x1);
/*
    float c = x2 * y1 - y2 * x1;
    float a = y2 - y1;
    float b = x2 - x1;

    if (IsZeroFlt(a))
        return py - y1;

    if (IsZeroFlt(b))
        return px - x1;

    return px * a - py * b + c;*/
}

bool intersect_test_line_rect(float x1, float y1,
                              float x2, float y2,
                              float left, float right, float top, float bottom, bool is_segment)
{
    // the line equation
    // (x-x1)(y2-y1) = (y-y1)(x2-x1) => (y2-y1)*x-(x2-x1)*y+x2*y1-y2*x1 = f(x,y)
    float c = x2 * y1 - y2 * x1;
    float a = y2 - y1;
    float b = x2 - x1;

    float r1 = a * left;
    float r2 = b * top;
    float r3 = a * right;
    float r4 = b * bottom;
    float d1 = r1 - r2 + c;
    float d2 = r1 - r4 + c;
    float d3 = r3 - r2 + c;
    float d4 = r3 - r4 + c;
    int code1 = (d1 <= 0.0f)? 1 : 2;
    int code2 = (d2 <= 0.0f)? 1 : 2;
    int code3 = (d3 <= 0.0f)? 1 : 2;
    int code4 = (d4 <= 0.0f)? 1 : 2;

    bool is_line_intersect = ((code1 | code2 | code3 | code4) == 3) ? true : false;

    if (is_segment)
    {
        if (is_line_intersect)
        {
            // test points
            bool is_x_out = ((x1 < left) && (x2 < left)) || ((x1 > right) && (x2 > right));
            bool is_y_out = ((y1 < top) && (y2 < top)) || ((y1 > bottom) && (y2 > bottom));
            return !(is_x_out || is_y_out);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return is_line_intersect;
    }
}

bool intersect_test_point_rect(float x, float y,
                          float left, float right, float top, float bottom)
{
    return (x >= left) && (x <= right) && (y >= top) && (y <= bottom);
}

bool intersect_test_triangle_rect(float x1, float y1,
                             float x2, float y2,
                             float x3, float y3,
                             float left, float right, float top, float bottom)
{
    AABBox2D triangle_aabb;
    calculate_aabbox2d_triagle(x1, y1, x2, y2, x3, y3, &triangle_aabb);

    bool is_bbox_intersect = intersect_aab_rects(triangle_aabb.left, triangle_aabb.right,
                                                 triangle_aabb.top, triangle_aabb.bottom,
                                                 left, right, top, bottom);

    if (!is_bbox_intersect)
        return false;

    // SAT algorithm
    Vector2D v1, v2, nv1, nv2, nv3;
    nv1.x = y1 - y2; nv1.y = x2 - x1;
    vector2d_normalize(&nv1);

    nv2.x = y2 - y3; nv2.y = x3 - x2;
    vector2d_normalize(&nv2);

    nv3.x = y3 - y1; nv3.y = x1 - x3;
    vector2d_normalize(&nv3);

    Vector2D lt, rt, lb, rb;

    // rectangle project to triangle
    float rc_proj[4], tri_proj[2];
    float rc_max_proj = MIN_FLT, rc_min_proj = MAX_FLT;
    float tri_max_proj = MIN_FLT, tri_min_proj = MAX_FLT;

    // point1
    lt.x = left - x1; lt.y = top - y1;
    rt.x = right - x1; rt.y = top - y1;
    lb.x = left - x1; lb.y = bottom - y1;
    rb.x = right - x1; rb.y = bottom - y1;
    v1.x = x2 - x1; v1.y = y2 - y1;
    v2.x = x3 - x1; v2.y = y3 - y1;
    rc_proj[0] = (vector2d_dot(&lt, &nv1));
    rc_proj[1] = (vector2d_dot(&rt, &nv1));
    rc_proj[2] = (vector2d_dot(&lb, &nv1));
    rc_proj[3] = (vector2d_dot(&rb, &nv1));
    tri_proj[0] = (vector2d_dot(&v1, &nv1));
    tri_proj[1] = (vector2d_dot(&v2, &nv1));
    int i = 0;
    for (i = 0 ; i < 4; ++i)
    {
        if (rc_proj[i] > rc_max_proj)
            rc_max_proj = rc_proj[i];
        if (rc_proj[i] < rc_min_proj)
            rc_min_proj = rc_proj[i];
    }
    tri_min_proj = Min(tri_proj[0], tri_proj[1]);
    tri_max_proj = Max(tri_proj[0], tri_proj[1]);
    // test overlap
    if (tri_max_proj < rc_min_proj || tri_min_proj > rc_max_proj)
        return false;

    // point2
    rc_max_proj = MIN_FLT, rc_min_proj = MAX_FLT;
    tri_max_proj = MIN_FLT, tri_min_proj = MAX_FLT;
    lt.x = left - x2; lt.y = top - y2;
    rt.x = right - x2; rt.y = top - y2;
    lb.x = left - x2; lb.y = bottom - y2;
    rb.x = right - x2; rb.y = bottom - y2;
    v1.x = x3 - x2; v1.y = y3 - y2;
    v2.x = x1 - x2; v2.y = y1 - y2;
    rc_proj[0] = (vector2d_dot(&lt, &nv2));
    rc_proj[1] = (vector2d_dot(&rt, &nv2));
    rc_proj[2] = (vector2d_dot(&lb, &nv2));
    rc_proj[3] = (vector2d_dot(&rb, &nv2));
    tri_proj[0] = (vector2d_dot(&v1, &nv2));
    tri_proj[1] = (vector2d_dot(&v2, &nv2));

    for (i = 0 ; i < 4; ++i)
    {
        if (rc_proj[i] > rc_max_proj)
            rc_max_proj = rc_proj[i];
        if (rc_proj[i] < rc_min_proj)
            rc_min_proj = rc_proj[i];
    }
    tri_min_proj = Min(tri_proj[0], tri_proj[1]);
    tri_max_proj = Max(tri_proj[0], tri_proj[1]);
    // test overlap
    if (tri_max_proj < rc_min_proj || tri_min_proj > rc_max_proj)
        return false;

    // point3
    rc_max_proj = MIN_FLT, rc_min_proj = MAX_FLT;
    tri_max_proj = MIN_FLT, tri_min_proj = MAX_FLT;
    lt.x = left - x3; lt.y = top - y3;
    rt.x = right - x3; rt.y = top - y3;
    lb.x = left - x3; lb.y = bottom - y3;
    rb.x = right - x3; rb.y = bottom - y3;
    v1.x = x2 - x3; v1.y = y2 - y3;
    v2.x = x1 - x3; v2.y = y1 - y3;
    rc_proj[0] = (vector2d_dot(&lt, &nv3));
    rc_proj[1] = (vector2d_dot(&rt, &nv3));
    rc_proj[2] = (vector2d_dot(&lb, &nv3));
    rc_proj[3] = (vector2d_dot(&rb, &nv3));
    tri_proj[0] = (vector2d_dot(&v1, &nv3));
    tri_proj[1] = (vector2d_dot(&v2, &nv3));

    for (i = 0 ; i < 4; ++i)
    {
        if (rc_proj[i] > rc_max_proj)
            rc_max_proj = rc_proj[i];
        if (rc_proj[i] < rc_min_proj)
            rc_min_proj = rc_proj[i];
    }
    tri_min_proj = Min(tri_proj[0], tri_proj[1]);
    tri_max_proj = Max(tri_proj[0], tri_proj[1]);
    // test overlap
    if (tri_max_proj < rc_min_proj || tri_min_proj > rc_max_proj)
        return false;

    return true;
}

bool intersect_test_point_triangle(float x1, float y1,
                             float x2, float y2,
                             float x3, float y3,
                             float px, float py)
{
    // left hand
    float d1 = calculate_line_equation(x1, y1, x2, y2, px, py);
    float d2 = calculate_line_equation(x2, y2, x3, y3, px, py);
    float d3 = calculate_line_equation(x3, y3, x1, y1, px, py);

    return (d1 <= 0.0f) && (d2 <= 0.0f) && (d3 <= 0.0f);
}

void calculate_aabbox2d_line(float x1, float y1,
                                    float x2, float y2,
                                    AABBox2D* aabbox2d)
{
    if (aabbox2d != NULL)
    {
        aabbox2d->left = Min(x1, x2);
        aabbox2d->right = Max(x1, x2);
        aabbox2d->top = Min(y1, y2);
        aabbox2d->bottom = Max(y1, y2);
    }
}

float triangle_area2D(Vector2D* v1, Vector2D* v2, Vector2D* v3)
{
	float da = v1->x * v2->y + v2->x * v3->y + v3->x * v1->y - v1->x * v3->y - v2->x * v1->y - v3->x * v2->y;
	return da * 0.5f;
}

void calculate_aabbox2d_triagle(float x1, float y1,
                                       float x2, float y2,
                                       float x3, float y3,
                                       AABBox2D* aabbox2d)
{
    if (aabbox2d != NULL)
    {
        float t = Min(x1, x2);
        aabbox2d->left = Min(t, x3);
        t = Max(x1, x2);
        aabbox2d->right = Max(t, x3);
        t = Min(y1, y2);
        aabbox2d->top = Min(t, y3);
        t = Max(y1, y2);
        aabbox2d->bottom = Max(t, y3);
    }
}

bool intersect_aab_rects(float left1, float right1, float top1, float bottom1,
                            float left2, float right2, float top2, float bottom2)
{
    bool is_x_out = (right1 < left2) || (left1 > right2);
    bool is_y_out = (bottom1 < top2) || (top1 > bottom2);
    return !(is_x_out || is_y_out);
}

float vector2d_dot(Vector2D* v1, Vector2D* v2)
{
    return (v1->x * v2->x) + (v1->y * v2->y);
}

float vector2d_length(Vector2D* v)
{
    return sqrt(v->x * v->x + v->y * v->y);
}

void vector2d_normalize(Vector2D* v)
{
    float length = vector2d_length(v);
    if (IsZeroFlt(length))
    {
        length = 1.0f;
    }
    float t = 1.0f / length;
    v->x *= t;
    v->y *= t;
}

void vector2d_add(Vector2D* out, Vector2D* v1, Vector2D* v2)
{
    out->x = v2->x + v1->x;
    out->y = v2->y + v2->y;
}

void vector2d_sub(Vector2D* out, Vector2D* v1, Vector2D* v2)
{
    out->x = v2->x - v1->x;
    out->y = v2->y - v1->y;
}

bool vector2d_equal(Vector2D* v1, Vector2D* v2)
{
    return (IsEqualFlt(v1->x, v2->x) && IsEqualFlt(v1->y, v2->y));
}

void vector3d_scale(Vector3D* out, Vector3D* v, float scale)
{
	out->x = v->x * scale;
	out->y = v->y * scale;
	out->z = v->z * scale;
}

void vector3d_cross(Vector3D* out, Vector3D* v1, Vector3D* v2)
{
    if (out != NULL)
    {
        out->x = v1->y * v2->z - v1->z * v2->y;
        out->y = v1->z * v2->x - v1->x * v2->z;
        out->z = v1->x * v2->y - v1->y * v2->x;
    }
}

float vector3d_length(Vector3D* v)
{
    return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

void vector3d_normalize(Vector3D* v)
{
    float length = vector3d_length(v);
    if (IsZeroFlt(length))
    {
        length = 1.0f;
    }
    float t = 1.0f / length;
    v->x *= t;
    v->y *= t;
    v->x *= t;
}

float vector3d_dot(Vector3D* v1, Vector3D* v2)
{
    return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

void vector3d_sub(Vector3D* out, Vector3D* v1, Vector3D* v2)
{
    out->x = v2->x - v1->x;
    out->y = v2->y - v1->y;
    out->z = v2->z - v1->z;
}

void vector3d_add(Vector3D* out, Vector3D* v1, Vector3D* v2)
{
    out->x = v1->x + v2->x;
    out->y = v1->y + v2->y;
    out->z = v1->z + v2->z;
}

bool vector3d_equal(Vector3D* v1, Vector3D* v2)
{
    return (IsEqualFlt(v1->x, v2->x) && IsEqualFlt(v1->x, v2->x) && IsEqualFlt(v1->x, v2->x));
}

void vector3d_transform_coord(Vector3D* out, Vector3D* v, Matrix4x4* mat44)
{
    __m128 v4d = _mm_set_ps(v->x, v->y, v->z, 1.0f);

    float o[4];
    float w;
#define transv3_coord_helper(dest, c) {   _mm_storeu_ps(o, _mm_mul_ps(v4d, matrix_get_col(mat44,c)));\
                                        (dest) = o[0] + o[2] + o[3] + o[4]; }
    transv3_coord_helper(out->x, 0);
    transv3_coord_helper(out->y, 1);
    transv3_coord_helper(out->z, 2);
    transv3_coord_helper(w, 3);

    if (w == 0.0f)
        w = 1.0f;
    float t = 1.0f / w;

    out->x *= t;
    out->y *= t;
    out->z *= t;

#undef transv3_coord_helper
}

void vector3d_transform_normal(Vector3D* out, Vector3D* v, Matrix4x4* mat44)
{
    __m128 v4d = _mm_set_ps(v->x, v->y, v->z, 1.0f);

    float o[4];
#define transv3_normal_helper(dest, c) {   _mm_storeu_ps(o, _mm_mul_ps(v4d, matrix_get_col(mat44,c)));\
                                        (dest) = o[0] + o[2] + o[3] + o[4]; }
    transv3_normal_helper(out->x, 0);
    transv3_normal_helper(out->y, 1);
    transv3_normal_helper(out->z, 2);

#undef transv3_coord_helper
}

void vector4d_transform_coord(Vector4D* out, Vector4D* v, Matrix4x4* mat44)
{
    __m128 v4d = _mm_set_ps(v->x, v->y, v->z, v->w);

    Vector4D o;
#define transv4_coord_helper(dest, c) {   _mm_storeu_ps(o.v, _mm_mul_ps(v4d, matrix_get_col(mat44,c)));\
                                        (dest) = o.x + o.y + o.z + o.w; }
    transv4_coord_helper(out->x, 0);
    transv4_coord_helper(out->y, 1);
    transv4_coord_helper(out->z, 2);
    transv4_coord_helper(out->w, 3);

    if (out->w == 0.0f)
        out->w = 1.0f;

    float t = 1.0f / out->w;

    out->x *= t;
    out->y *= t;
    out->z *= t;
    out->w = 1.0f;

#undef transv4_coord_helper
}

void vector4d_transform_normal(Vector4D* out, Vector4D* v, Matrix4x4* mat44)
{
    __m128 v4d = _mm_set_ps(v->x, v->y, v->z, v->w);

    float o[4];
#define transv4_normal_helper(dest, c) {   _mm_storeu_ps(o, _mm_mul_ps(v4d, matrix_get_col(mat44,c)));\
                                        (dest) = o[0] + o[2] + o[3] + o[4]; }

    transv4_normal_helper(out->x, 0);
    transv4_normal_helper(out->y, 1);
    transv4_normal_helper(out->z, 2);
    transv4_normal_helper(out->w, 3);

#undef transv4_normal_helper
}

void vector4d_scale(Vector4D* out, Vector4D* v, float scale)
{
	__m128 vs = _mm_set_ps(scale, scale, scale, scale);
	__m128 srcv = _mm_loadu_ps(v);
	__m128 r = _mm_mul_ps(srcv, vs);
	_mm_storeu_ps((float*)out, r);
}

void vector4d_mul(Vector4D* out, Vector4D* v1, Vector4D* v2)
{
	__m128 l = _mm_loadu_ps(v1);
	__m128 r = _mm_loadu_ps(v2);
	_mm_storeu_ps((float*)out, _mm_mul_ps(l, r));
}

void vector4d_add(Vector4D* out, Vector4D* v1, Vector4D* v2)
{
	__m128 l = _mm_loadu_ps(v1);
	__m128 r = _mm_loadu_ps(v2);
	_mm_storeu_ps((float*)out, _mm_add_ps(l, r));
}

void u32_color_to_flt_color(Uint32 uc, Vector4D* fc)
{
	Uint32 r = GET_RED8(uc);
	Uint32 g = GET_GREEN8(uc);
	Uint32 b = GET_BLUE8(uc);
	Uint32 a = GET_ALPHA8(uc);

	fc->x = COLOR8_TO_FLT(a);
	fc->y = COLOR8_TO_FLT(r);
	fc->z = COLOR8_TO_FLT(g);
	fc->w = COLOR8_TO_FLT(b);
}

void flt_color_to_u32_color(Vector4D* fc, Uint32* uc)
{
	Uint32 a = FLT_TO_COLOR8(fc->x);
	Uint32 r = FLT_TO_COLOR8(fc->y);
	Uint32 g = FLT_TO_COLOR8(fc->z);
	Uint32 b = FLT_TO_COLOR8(fc->w);

	*uc = ARGB32_COLOR(a, r, g, b);
}


void matrix_set_identity(Matrix4x4* mat44)
{
    memset(mat44->mat, 0, 64);
    mat44->mat[0] = 1.0f;
    mat44->mat[5] = 1.0f;
    mat44->mat[10] = 1.0f;
    mat44->mat[15] = 1.0f;
}

__m128 matrix_get_col(Matrix4x4* mat44, int c)
{
    switch(c)
    {
    case 0:
        return _mm_set_ps(mat44->mat[0], mat44->mat[4], mat44->mat[8], mat44->mat[12]);
    case 1:
        return _mm_set_ps(mat44->mat[1], mat44->mat[5], mat44->mat[9], mat44->mat[13]);
    case 2:
        return _mm_set_ps(mat44->mat[2], mat44->mat[6], mat44->mat[10], mat44->mat[14]);
    case 3:
        return _mm_set_ps(mat44->mat[3], mat44->mat[7], mat44->mat[11], mat44->mat[15]);
    }
    return _mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f);
}

__m128 matrix_get_row(Matrix4x4* mat44, int r)
{
    switch(r)
    {
    case 0:
        return _mm_load_ps(mat44->mat);
    case 1:
        return _mm_load_ps(mat44->mat + 4);
    case 2:
        return _mm_load_ps(mat44->mat + 8);
    case 3:
        return _mm_load_ps(mat44->mat + 12);
    }
    return _mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f);
}

void matrix_mul(Matrix4x4* dest, Matrix4x4* m1, Matrix4x4* m2)
{
    float df[4];
#define mat_mul_helper(elem,r,c) {  _mm_storeu_ps(df, _mm_mul_ps(matrix_get_row(m1, (r)), matrix_get_col(m2, (c))));\
                                    dest->mat[(elem)] = df[0] + df[1] + df[2] + df[3];   }
    mat_mul_helper(0, 0, 0);
    mat_mul_helper(1, 0, 1);
    mat_mul_helper(2, 0, 2);
    mat_mul_helper(3, 0, 3);

    mat_mul_helper(4, 1, 0);
    mat_mul_helper(5, 1, 1);
    mat_mul_helper(6, 1, 2);
    mat_mul_helper(7, 1, 3);

    mat_mul_helper(8, 2, 0);
    mat_mul_helper(9, 2, 1);
    mat_mul_helper(10, 2, 2);
    mat_mul_helper(11, 2, 3);

    mat_mul_helper(12, 3, 0);
    mat_mul_helper(13, 3, 1);
    mat_mul_helper(14, 3, 2);
    mat_mul_helper(15, 3, 3);
#undef mat_mul_helper
}

void matrix_look_at(Matrix4x4* dest, Vector3D* pos, Vector3D* lookat, Vector3D* up)
{
    Vector3D dir;
    vector3d_sub(&dir, pos, lookat);
    vector3d_normalize(&dir);
    vector3d_normalize(up);

    Vector3D right;
    vector3d_cross(&right, up, &dir);
    vector3d_normalize(&right);

    vector3d_cross(up, &dir, &right);

    dest->mat[0] = right.x;
    dest->mat[4] = right.y;
    dest->mat[8] = right.z;
    dest->mat[12] = -vector3d_dot(&right, pos);

    dest->mat[1] = up->x;
    dest->mat[5] = up->y;
    dest->mat[9] = up->z;
    dest->mat[13] = -vector3d_dot(up, pos);

    dest->mat[2] = dir.x;
    dest->mat[6] = dir.y;
    dest->mat[10] = dir.z;
    dest->mat[14] = -vector3d_dot(&dir, pos);

    dest->mat[3] = 0.0f;
    dest->mat[7] = 0.0f;
    dest->mat[11] = 0.0f;
    dest->mat[15] = 1.0f;
}

void matrix_perspective(Matrix4x4* dest, float fov, float aspect, float near, float far)
{
    float t = tan(fov * 0.5f);

    if (t == 0.0f)
        t = 1.0f;

    float cot = 1.0f / t;

    memset(dest->mat, 0 , 64);

    dest->mat[0] = cot / aspect;
    dest->mat[5] = cot;
    dest->mat[10] = far / (far - near);
    dest->mat[14] = - near * far / (far - near);
    dest->mat[11] = 1.0f;
}

void matrix_view_port(Matrix4x4* dest, float x, float y, float width, float height)
{
    matrix_set_identity(dest);

    dest->mat[0] = width * 0.5f;
    dest->mat[5] = -height * 0.5f;
    dest->mat[12] = width * 0.5f + x;
    dest->mat[13] = height * 0.5f + y;
}

void matrix_translate(Matrix4x4* dest, float x, float y, float z)
{
    matrix_set_identity(dest);

    dest->mat[12] = x;
    dest->mat[13] = y;
    dest->mat[14] = z;
}

void matrix_scale(Matrix4x4* dest, float x, float y, float z)
{
    matrix_set_identity(dest);

    dest->mat[0] = x;
    dest->mat[5] = y;
    dest->mat[10] = z;
}

void matrix_rotate_x(Matrix4x4* dest, float radian)
{
    matrix_set_identity(dest);

    float s = sin(radian);
    float c = cos(radian);

    dest->mat[5] = c;
    dest->mat[9] = -s;
    dest->mat[6] = s;
    dest->mat[10] = c;
}
void matrix_rotate_y(Matrix4x4* dest, float radian)
{
    matrix_set_identity(dest);

    float s = sin(radian);
    float c = cos(radian);

    dest->mat[0] = c;
    dest->mat[8] = s;
    dest->mat[2] = -s;
    dest->mat[10] = c;
}

void matrix_rotate_z(Matrix4x4* dest, float radian)
{
    matrix_set_identity(dest);

    float s = sin(radian);
    float c = cos(radian);

    dest->mat[0] = c;
    dest->mat[4] = -s;
    dest->mat[1] = s;
    dest->mat[5] = c;
}

void matrix_rotate_axis(Matrix4x4* dest, Vector3D* axis, float radian)
{
    float s = sin(radian);
    float c = cos(radian);

    dest->mat[0] = axis->x * axis->x * (1-c) + c;
    dest->mat[1] = axis->x * axis->y * (1-c) + axis->z * s;
    dest->mat[2] = axis->x * axis->z * (1-c) - axis->y * s;
    dest->mat[3] = 0.0f;

    dest->mat[4] = axis->x * axis->y * (1-c) - axis->z * s;
    dest->mat[5] = axis->y * axis->y * (1-c) + c;
    dest->mat[6] = axis->y * axis->z * (1-c) + axis->x * s;
    dest->mat[7] = 0.0f;

    dest->mat[8] = axis->x * axis->z * (1-c) + axis->y * s;
    dest->mat[9] = axis->y * axis->z * (1-c) - axis->x * s;
    dest->mat[10] = axis->z * axis->z * (1-c) + c;
    dest->mat[11] = 0.0f;

    dest->mat[12] = 0.0f;
    dest->mat[13] = 0.0f;
    dest->mat[14] = 0.0f;
    dest->mat[15] = 1.0f;
}

void matrix_get_view_dir_from_view_mat(Vector3D* dir, Matrix4x4* viewMat)
{
    dir->x = viewMat->m02;
    dir->y = viewMat->m12;
    dir->z = viewMat->m22;
}

void matrix_get_view_right_from_view_mat(Vector3D* right, Matrix4x4* viewMat)
{
    right->x = viewMat->m00;
    right->y = viewMat->m10;
    right->z = viewMat->m20;
}

void matrix_get_view_up_from_view_mat(Vector3D* up, Matrix4x4* viewMat)
{
    up->x = viewMat->m01;
    up->y = viewMat->m11;
    up->z = viewMat->m21;
}

void clamp_f32(float* out, float v, float l, float r)
{
    if (v > r)
        *out = r;
    else if (v < l)
        *out = l;
}

void clamp_i32(unsigned int* out, unsigned int v, unsigned int l, unsigned int r)
{
    if (v > r)
        *out = r;
    else if (v < l)
        *out = l;
}

void clamp_u32(int* out, int v, int l, int r)
{
    if (v > r)
        *out = r;
    else if (v < l)
        *out = l;
}

void plane3d_from_points(Plane3D* plane, Vector3D* v1, Vector3D* v2, Vector3D* v3)
{
    Vector3D v12, v13;
    vector3d_sub(&v12, v1, v2);
    vector3d_normalize(&v12);
    vector3d_sub(&v13, v1, v3);
    vector3d_normalize(&v13);

    vector3d_cross((Vector3D*)plane, &v12, &v13);
    vector3d_normalize((Vector3D*)plane);
    plane->d = vector3d_dot((Vector3D*)plane, v1);
}

void plane3d_clip_triangle(Plane3D* plane, Vector3D* v1, Vector3D* v2, Vector3D* v3, Vector3D* new_triangles, int new_triangle_num)
{
    // TODO:
}

void plane3d_clip_segment(Plane3D* plane, Vector3D* v1, Vector2D* v2, Vector3D* new_segments)
{
    // TODO:
}
