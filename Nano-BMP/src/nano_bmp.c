#include <stdlib.h>
#include <stdio.h>
#include "../include/nano_bmp.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
//Clamp x between low and high
#define CLAMP(X, L, H) (MIN(MAX((X), (L)), (H)))

bmp_t* create_bmp(unsigned w, unsigned h, unsigned bpp){
	if (bpp != 24 && bpp != 32){
		fprintf(stderr, "create_bmp error: Unsupported bits-per-pixel\n");
		return NULL;
	}
	bmp_t *bmp = malloc(sizeof(bmp_t));
	if (!bmp){
		fprintf(stderr, "create_bmp error: BMP allocation failed\n");
		return NULL;
	}
	//Compute the bytes we'll need to store the image, accounting for padding
	bmp->info.bpp = bpp;
	bmp->padding = (w * bmp->info.bpp / 8) % 4;
	bmp->info.img_size = h * (bmp->info.bpp / 8 * w + bmp->padding);
	bmp->pixels = malloc(bmp->info.img_size);
	if (!bmp->pixels){
		fprintf(stderr, "create_bmp error: Failed to allocate pixel array\n");
		destroy_bmp(bmp);
		return NULL;
	}

	//Setup the BMP File Header
	bmp->file.header[0] = 'B';
	bmp->file.header[1] = 'M';
	bmp->file.file_size = sizeof(bmp_file_header_t) + sizeof(bmp_info_header_t)
		+ bmp->info.img_size;
	bmp->file.px_array = sizeof(bmp_file_header_t) + sizeof(bmp_info_header_t);

	//Setup the BMP info header
	bmp->info.header_size = sizeof(bmp_info_header_t);
	bmp->info.w = w;
	bmp->info.h = h;
	bmp->info.color_planes = 1;
	bmp->info.compression = 0;
	bmp->info.h_res = 2835;
	bmp->info.v_res = 2835;
	bmp->info.color_palette = 0;
	bmp->info.important_colors = 0;
	return bmp;
}
void destroy_bmp(bmp_t *bmp){
	if (bmp->pixels){
		free(bmp->pixels);
	}
	free(bmp);
}
bmp_t *load_bmp(const char *f_name){
	FILE *f = fopen(f_name, "rb");
	if (!f){
		fprintf(stderr, "load_bmp error: Failed to open file %s\n", f_name);
		return NULL;
	}
	bmp_t *bmp = malloc(sizeof(bmp_t));
	if (!bmp){
		fprintf(stderr, "load_bmp error: Failed to allocate a bmp_t\n");
		fclose(f);
		return NULL;
	}
	bmp->pixels = NULL;

	if (fread(&bmp->file, sizeof(bmp_file_header_t), 1, f) != 1){
		fprintf(stderr, "load_bmp error: Failed to read BMP File Header\n");
		fclose(f);
		destroy_bmp(bmp);
		return NULL;
	}
	if (fread(&bmp->info, sizeof(bmp_info_header_t), 1, f) != 1){
		fprintf(stderr, "load_bmp error: Failed to read BMP Info Header\n");
		fclose(f);
		destroy_bmp(bmp);
		return NULL;
	}
	if (bmp->info.header_size != sizeof(bmp_info_header_t)){
		fprintf(stderr, "load_bmp error: Unsupported BMP Info Header, ");
	   	fprintf(stderr, "only the BITMAPINFOHEADER is supported\n");
		fclose(f);
		destroy_bmp(bmp);
		return NULL;
	}
	//w/h can be negative so make sure they aren't
	bmp->info.w = abs(bmp->info.w);
	bmp->info.h = abs(bmp->info.h);
	if (bmp->info.bpp != 24 && bmp->info.bpp != 32){
		fprintf(stderr, "load_bmp error: Only 24 or 32bit BMPs are supported\n");
		fclose(f);
		destroy_bmp(bmp);
		return NULL;
	}
	if (bmp->info.compression != 0){
		fprintf(stderr, "load_bmp error: Compressed BMPs are not supported\n");
		fclose(f);
		destroy_bmp(bmp);
		return NULL;
	}

	//It's possible for img_size to be 0 so if it is determine the image size
	//by examining the file
	if (bmp->info.img_size == 0){
		fseek(f, 0, SEEK_END);
		long end = ftell(f);
		bmp->info.img_size = end - bmp->file.px_array;
		fseek(f, bmp->file.px_array, SEEK_SET);
	}

	bmp->pixels = malloc(bmp->info.img_size);
	if (fread(bmp->pixels, 1, bmp->info.img_size, f) != bmp->info.img_size){
		fprintf(stderr, "load_bmp error: Failed to read pixels\n");
		fclose(f);
		destroy_bmp(bmp);
		return NULL;
	}
	bmp->padding = (bmp->info.w * bmp->info.bpp / 8) % 4;
	return bmp;
}
void write_bmp(const char *f_name, const bmp_t *bmp){
	FILE *f = fopen(f_name, "wb");
	if (!f){
		fprintf(stderr, "write_bmp error: Failed to open file %s\n", f_name);
		return;
	}
	if (fwrite(&bmp->file, sizeof(bmp_file_header_t), 1, f) != 1){
		fprintf(stderr, "write_bmp error: Failed to write BMP File Header\n");
		fclose(f);
		return;
	}
	if (fwrite(&bmp->info, sizeof(bmp_info_header_t), 1, f) != 1){
		fprintf(stderr, "write_bmp error: Failed to write BMP Info Header\n");
		fclose(f);
		return;
	}
	if (fwrite(bmp->pixels, 1, bmp->info.img_size, f) != bmp->info.img_size){
		fprintf(stderr, "write_bmp error: Failed to write pixels\n");
	}
	fclose(f);
}
int pixel_idx(const bmp_t *bmp, int x, int y){
	//Clamp pixels in range
	x = CLAMP(x, 0, bmp->info.w - 1);
	y = CLAMP(y, 0, bmp->info.h - 1);
	//Determine # of bytes per row
	size_t bpr = bmp->info.bpp / 8 * bmp->info.w + bmp->padding;
	//Invert y when returning index b/c bmp stored "upside-down"
	return x * bmp->info.bpp / 8 + (bmp->info.h - y - 1) * bpr;
}
void get_pixel(const bmp_t *bmp, int x, int y, uint8_t *r, uint8_t *g, uint8_t *b){
	int i = pixel_idx(bmp, x, y);
	*r = bmp->pixels[i + 2];
	*g = bmp->pixels[i + 1];
	*b = bmp->pixels[i];
}
void set_pixel(bmp_t *bmp, int x, int y, uint8_t r, uint8_t g, uint8_t b){
	int i = pixel_idx(bmp, x, y);
	bmp->pixels[i + 2] = r;
	bmp->pixels[i + 1] = g;
	bmp->pixels[i] = b;
}
bmp_t* convert_32bpp(bmp_t *bmp){
	bmp_t *converted = create_bmp(bmp->info.w, bmp->info.h, 32);
	if (!converted){
		fprintf(stderr, "convert_32bpp error: Failed to allocated room for conversion\n");
		return NULL;
	}
	int w = bmp->info.w, h = bmp->info.h;
	for (int i = 0; i < h; ++i){
		for (int j = 0; j < w; ++j){
			uint8_t color[3];
			get_pixel(bmp, j, i, &color[0], &color[1], &color[2]);
			set_pixel(converted, j, i, color[0], color[1], color[2]);
		}
	}
	return converted;
}
bmp_t* convert_24bpp(bmp_t *bmp){
	bmp_t *converted = create_bmp(bmp->info.w, bmp->info.h, 24);
	if (!converted){
		fprintf(stderr, "convert_24bpp error: Failed to allocate room for conversion\n");
		return NULL;
	}
	int w = bmp->info.w, h = bmp->info.h;
	for (int i = 0; i < h; ++i){
		for (int j = 0; j < w; ++j){
			uint8_t color[3];
			get_pixel(bmp, j, i, &color[0], &color[1], &color[2]);
			set_pixel(converted, j, i, color[0], color[1], color[2]);
		}
	}
	return converted;
}
void bilinear_filter(const bmp_t *bmp, float u, float v,
	uint8_t *r, uint8_t *g, uint8_t *b)
{
	u = u * bmp->info.w - 0.5f;
	v = v * bmp->info.h - 0.5f;
	int x = u;
	int y = v;
	float u_ratio = u - x;
	float v_ratio = v - y;
	float u_opposite = 1 - u_ratio;
	float v_opposite = 1 - v_ratio;

	int idx[4];
	for (int i = 0; i < 4; ++i){
		//If we're at the right or bottom of the texture we want
		//to use pixels to our left or above so subtract instead
		int p_x = x + i % 2;
		int p_y = y + i / 2;
		if (p_x > bmp->info.w - 1){
			p_x = x - i % 2;
		}
		if (p_y > bmp->info.h - 1){
			p_y = y - i / 2;
		}
		idx[i] = pixel_idx(bmp, p_x, p_y);
	}

	//Blend the RGB values
	*r = (bmp->pixels[idx[0] + 2] * u_opposite + bmp->pixels[idx[1] + 2] * u_ratio) * v_opposite
		+ (bmp->pixels[idx[2] + 2] * u_opposite	+ bmp->pixels[idx[3] + 2] * u_ratio) * v_ratio;

	*g = (bmp->pixels[idx[0] + 1] * u_opposite + bmp->pixels[idx[1] + 1] * u_ratio) * v_opposite
		+ (bmp->pixels[idx[2] + 1] * u_opposite	+ bmp->pixels[idx[3] + 1] * u_ratio) * v_ratio;
	
	*b = (bmp->pixels[idx[0]] * u_opposite + bmp->pixels[idx[1]] * u_ratio) * v_opposite
		+ (bmp->pixels[idx[2]] * u_opposite	+ bmp->pixels[idx[3]] * u_ratio) * v_ratio;
}

