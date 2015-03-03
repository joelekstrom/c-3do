#ifndef NANO_BMP_H
#define NANO_BMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * This is a very small BMP image library with the bare minimum of
 * features needed to read/write basic BMP images.
 * Supports: Uncompressed 24 or 32bit BMP BI_RGB with the BITMAPINFOHEADER and without
 * color palettes or important colors. The information will be read in but is
 * ignored, and will be set to default (ie. ignored) values when creating new
 * bitmaps and simply left unchanged when copying bmps.
 */
/*
 * The BMP File Header contains information
 * about the layout of information in the file
 */
#pragma pack(2)
typedef struct bmp_file_header_t {
	uint8_t header[2];
	uint32_t file_size;
	//4 reserved bytes that we don't care about
	uint32_t dont_care;
	//The offset in the file to the pixel array
	uint32_t px_array;
} bmp_file_header_t;
/*
 * The BMP Info Header (BITMAPINFOHEADER) contains
 * information about the image stored in the file
 */
#pragma pack(2)
typedef struct bmp_info_header_t {
	uint32_t header_size;
	int32_t w, h;
	//Must be set to 1, ignored otherwise
	uint16_t color_planes;
	//Bits per pixel (only support 24 or 32)
	uint16_t bpp;
	//Only support compression 0, ie. none
	uint32_t compression;
	uint32_t img_size;
	//Horizontal and vertical resolution, just set to 2835 (# from wikipedia)
	int32_t h_res, v_res;
	//Only support default of 0, ie. no palette
	uint32_t color_palette;
	//Set to 0 & ignored
	uint32_t important_colors;
} bmp_info_header_t;
/*
 * A BMP loaded into memory, contains its file
 * header, info header, a pointer to the pixel array
 * and the # of bytes of padding on each row
 * Pixel data is in BGR and "upside-down" with 0,h being
 * the first entry
 */
typedef struct bmp_t {
	//The file and info headers
	bmp_file_header_t file;
	bmp_info_header_t info;
	//Pixels are stored in BGR and are stored upside down
	uint8_t *pixels;
	//Row padding to align rows on 4 byte boundaries
	uint8_t padding;
} bmp_t;
/*
 * Create a bpp-bit BMP with some width and height. Only 24 or 32bpp
 * is supported. The pixel values will be unitialized and thus
 * undefined until they're set via set_pixel.
 * Will return NULL if allocation fails
 */
bmp_t* create_bmp(unsigned w, unsigned h, unsigned bpp);
/*
 * Destroy a BMP
 */
void destroy_bmp(bmp_t *bmp);
/*
 * Load a BMP from a file. Will return NULL if the BMP is an unsupported type
 * or some other loading error occurs
 */
bmp_t* load_bmp(const char *f_name);
/*
 * Write a BMP to a file
 */
void write_bmp(const char *f_name, const bmp_t *bmp);
/*
 * Get the index of the start of some pixels BGR(X) values, returns index
 * of the B element
 */
int pixel_idx(const bmp_t *bmp, int x, int y);
/*
 * Get the RGB color of a pixel
 */
void get_pixel(const bmp_t *bmp, int x, int y, uint8_t *r, uint8_t *g, uint8_t *b);
/*
 * Set the color of a pixel
 */
void set_pixel(bmp_t *bmp, int x, int y, uint8_t r, uint8_t g, uint8_t b);
/*
 * Convert a 24bpp BMP to 32bpp. A new BMP will be returned containing
 * the 32bpp image. Will return NULL if something goes wrong
 */
bmp_t* convert_32bpp(bmp_t *bmp);
/*
 * Convert a 32bpp BMP to 24bpp. A new BMP will be returned containing
 * the 24bpp image. Will return NULL if something goes wrong
 */
bmp_t* convert_24bpp(bmp_t *bmp);
/*
 * Get the bilinearly filtered color value at x,y where x,y are in
 * normalized image coords [0.f, 1.f] range
 */
void bilinear_filter(const bmp_t *bmp, float u, float v,
	uint8_t *r, uint8_t *g, uint8_t *b);

#ifdef __cplusplus
}
#endif

#endif

