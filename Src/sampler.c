#include "sampler.h"
#include "libpng/png.h"
#include "smath.h"
#include <stdbool.h>
void load_texture_from_png(STexture* texture, const char* pngfile)
{
	FILE* pf = NULL;
	fopen_s(&pf, pngfile, "rb");
	png_byte header[8];
	fread(header, 1, 8, pf);
	bool is_png = !png_sig_cmp(header, 0, 8);
	if (!is_png)
	{
		fclose(pf);
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png_ptr == NULL)
		fclose(pf);

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (info_ptr == NULL)
		fclose(pf);

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(pf);
	}

	png_init_io(png_ptr, pf);

	png_set_sig_bytes(png_ptr, 8);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

	texture->width = png_get_image_width(png_ptr, info_ptr);
	texture->height = png_get_image_height(png_ptr, info_ptr);

	int color_type = png_get_color_type(png_ptr, info_ptr);
	texture->data = (Uint32*)malloc(sizeof(Uint32) * texture->width * texture->height);

	png_bytep *row_point = NULL;
	row_point = png_get_rows(png_ptr, info_ptr);

	int block_size = (color_type == 6 ? 4 : 3);

	Uint32 pos = 0, x = 0, y = 0;
	for (x = 0; x < texture->height; ++x)
	{
		for (y = 0; y < texture->width * block_size; y += block_size)
		{
			((Uint8*)texture->data)[pos + 1] = row_point[x][y + 2];// r
			((Uint8*)texture->data)[pos + 2] = row_point[x][y + 1];// g
			((Uint8*)texture->data)[pos + 3] = row_point[x][y + 0];// b
			if (color_type == 6) // alpha channel
				((unsigned char*)texture->data)[pos + 0] = row_point[x][y + 3];// a
			else
				((unsigned char*)texture->data)[pos + 0] = 0xff;// a
			pos += 4;
		}
	}
	fclose(pf);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	texture->data_size = sizeof(Uint32) * texture->width * texture->height;
}

Uint32 sample_texture2D(SSampler* sampler, float u, float v, Uint32 sample_type)
{
	float pixel_x = (float)sampler->texture->width * u;
	float pixel_y = (float)sampler->texture->height * v;
	Uint32 px = 0, py = 0;
	if (sample_type == NEAREST_POINT)
	{
		Uint32 floor_x = (Uint32)pixel_x;
		if (pixel_x - (float)floor_x > 0.5f)
			px = floor_x + 1;
		else
			px = floor_x;
		Uint32 floor_y = (Uint32)pixel_y;
		if (pixel_y - (float)floor_y > 0.5f)
			py = floor_y;
		else
			py = floor_y + 1;
// 		Uint32 t = sampler->texture->data[sampler->texture->width * 27 + 18];
// 		printf("texture[%u, %u]: %u\n",px, py, ((t & 0xffffff00) >> 8) | ((t & 0xff) << 24));
// 		Vector4D tex_color_f;
// 		u32_color_to_flt_color(((t & 0xffffff00) >> 8) | ((t & 0xff) << 24), &tex_color_f);
		if (px >= sampler->texture->width || py >= sampler->texture->height || px < 0 || py < 0)
			return 0;
		// RGBA
		Uint32 rgba = sampler->texture->data[sampler->texture->width * py + px];
//		printf("texture[%u, %u]: %u\n", px, py, ((rgba & 0xffffff00) >> 8) | ((rgba & 0xff) << 24));
		// convert to ARGB
		return ((rgba & 0xffffff00) >> 8) | ((rgba & 0xff) << 24);
	}
	else if (sample_type == DOUBLE_LINEAR)
	{
		Uint32 floor_x = (Uint32)pixel_x;
		Uint32 floor_y = (Uint32)pixel_y;
		Uint32 ceil_x = floor_x + 1;
		Uint32 ceil_y = floor_y + 1;
		if (ceil_x >= sampler->texture->width)
			ceil_x -= 1;
		if (ceil_y >= sampler->texture->height)
			ceil_y -= 1;
		float scale_x = pixel_x - (float)floor_x;
		float scale_y = pixel_y - (float)floor_y;

		Vector4D lt, rt, lb, rb;

		if (px >= sampler->texture->width || py >= sampler->texture->width || px < 0 || py < 0)
			return 0;
		Uint32 rgba = sampler->texture->data[sampler->texture->width * floor_y + floor_x];
		Uint32 argb = ((rgba & 0xffffff00) >> 8) | ((rgba & 0xff) << 24);
		u32_color_to_flt_color(argb, &lt);

		sampler->texture->data[sampler->texture->width * floor_y + floor_x];
		argb = ((rgba & 0xffffff00) >> 8) | ((rgba & 0xff) << 24);
		u32_color_to_flt_color(argb, &rt);

		sampler->texture->data[sampler->texture->width * floor_y + floor_x];
		argb = ((rgba & 0xffffff00) >> 8) | ((rgba & 0xff) << 24);
		u32_color_to_flt_color(argb, &lb);

		sampler->texture->data[sampler->texture->width * floor_y + floor_x];
		argb = ((rgba & 0xffffff00) >> 8) | ((rgba & 0xff) << 24);
		u32_color_to_flt_color(argb, &rb);

		Vector4D interp_top, interp_bottom;
		interp_top.x = INTERP_FLT(lt.x, rt.x, scale_x);
		interp_top.y = INTERP_FLT(lt.y, rt.y, scale_x);
		interp_top.z = INTERP_FLT(lt.z, rt.z, scale_x);
		interp_top.w = INTERP_FLT(lt.w, rt.w, scale_x);

		interp_bottom.x = INTERP_FLT(lb.x, rb.x, scale_x);
		interp_bottom.y = INTERP_FLT(lb.y, rb.y, scale_x);
		interp_bottom.z = INTERP_FLT(lb.z, rb.z, scale_x);
		interp_bottom.w = INTERP_FLT(lb.w, rb.w, scale_x);

		Vector4D final_color;
		final_color.x = INTERP_FLT(interp_top.x, interp_bottom.x, scale_x);
		final_color.y = INTERP_FLT(interp_top.y, interp_bottom.y, scale_x);
		final_color.z = INTERP_FLT(interp_top.z, interp_bottom.z, scale_x);
		final_color.w = INTERP_FLT(interp_top.w, interp_bottom.w, scale_x);

		Uint32 fc = 0;
		flt_color_to_u32_color(&final_color, &fc);

		return fc;
	}
	return 0;
}