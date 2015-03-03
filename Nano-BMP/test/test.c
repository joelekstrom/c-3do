#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "nano_bmp.h"

int main(int argc, char **argv){
	//Make some bmp that's half-red half-blue
	bmp_t *bmp = create_bmp(2, 2, 24);
	for (int i = 0; i < 2; ++i){
		set_pixel(bmp, 0, i, 255, 0, 0);
		set_pixel(bmp, 1, i, 0, 0, 255);
	}
	write_bmp("rb.bmp", bmp);

	//Create bilinear filtered version but sample at pixel centers
	//we'd expect this to look the same as the original (?)
	bmp_t *filtered = create_bmp(2, 2, 24);
	for (int i = 0; i < 4; ++i){
		uint8_t col[3] = { 0 };
		bilinear_filter(bmp, (i % 2) / 2.0 + 0.25, (i / 2) / 2.0 + 0.25, &col[0], &col[1], &col[2]);
		set_pixel(filtered, i % 2, i / 2, col[0], col[1], col[2]);	
	}
	write_bmp("filtered_px_center.bmp", filtered);

	//Now sample the half-red half-blue image at the center of the image
	uint8_t col[3] = { 0 };
	bilinear_filter(bmp, 0.5, 0.5, &col[0], &col[1], &col[2]);
	for (int i = 0; i < 4; ++i){
		set_pixel(filtered, i % 2, i / 2, col[0], col[1], col[2]);	
	}
	write_bmp("filtered_img_center.bmp", filtered);

	destroy_bmp(filtered);
	destroy_bmp(bmp);

	if (argc == 3){
		printf("Copying bmp %s to %s as a different bpp\n", argv[1], argv[2]);
		bmp = load_bmp(argv[1]);
		bmp_t *out = NULL;
		if (bmp->info.bpp == 24){
			printf("Copying to a 32bpp format\n");
			out = convert_32bpp(bmp);
		}
		else {
			printf("Copying to a 24bpp format\n");
			out = convert_24bpp(bmp);
		}
		write_bmp(argv[2], out);
		destroy_bmp(out);
		destroy_bmp(bmp);
	}
	return 0;
}

