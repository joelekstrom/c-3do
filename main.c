#include "graphics_context.h"
#include "geometry.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "sob.h"

void render_mesh(struct model_t model, 
				 transform_3d transform, 
				 transform_3d view, 
				 float perspective, 
				 struct graphics_context *context, 
				 rgb_color color);

int main() {
	struct graphics_context *context = create_context(BMP_CONTEXT_TYPE, 300, 300);
	
	rgb_color red = {255, 0, 0};
	rgb_color green = {0, 255, 0};
	rgb_color blue = {0, 0, 255};
	rgb_color white = {255, 255, 255};
	rgb_color black = {0, 0, 0};

	clear(context, black);

	// load .sob-file
	FILE *fp = fopen("model/cube.sob", "r");
	if (!fp) {
		fprintf(stderr, "Failed to open model file");
		return 1;
		
	}

	struct model_t model = load_model(fp);
	fclose(fp);

	// Make sure origin (0, 0) is drawn in the middle of the image
	transform_3d view = transform_3d_make_translation(context->width / 2, context->height / 2, 0.0);
	float perspective = 0.0025;

	// We want to scale the cube since its coordinate space goes from -1 to 1
	transform_3d scale = transform_3d_make_scale(50.0, 50.0, 50.0);

	// Move the cube up to the left
	transform_3d translate = transform_3d_make_translation(-70, -70, 0); 
	render_mesh(model, transform_3d_concat(scale, translate), view, perspective, context, red);

	// Test drawing of a triangle
	vec2 a = {100,10};
	vec2 b = {5,180};
	vec2 c = {240,180};

	goraud_triangle(a, b, c, red, green, blue, context);

	// Debug, draw actual triangle
	/*draw_line(a, b, context, white);
	draw_line(a, c, context, white);
	draw_line(b, c, context, white);*/

	//fill_triangle(a, b, c, context, white);
	unload_model(model);
	bmp_context_save(context, "bin/output.bmp");
	destroy_context(context);
	return 0;
}

/**
 Applies perspective to simulate vector positions in 3D-space, relative to a view position
 */
static inline vec2 apply_perspective(vec3 position, vec3 view_point, float amount) {
	float distance_x = view_point.x - position.x;
	float distance_y = view_point.y - position.y;
	vec2 result;
	result.x = position.x + position.z * distance_x * amount;
	result.y = position.y + position.z * distance_y * amount;
	return result;
}

void render_mesh(struct model_t model, 
				 transform_3d transform, 
				 transform_3d view, 
				 float perspective, 
				 struct graphics_context *context, 
				 rgb_color color) 
{
	for (int i = 0; i < model.num_edges; i++) {
		edge e = model.edges[i];

		// Position vertices in view and apply any  
		transform_3d t = transform_3d_concat(transform, view);
		vec3 v1 = transform_3d_apply(*e.v1, t);
		vec3 v2 = transform_3d_apply(*e.v2, t);

		// Calculate the view point, will be used to apply perspective
		vec3 view_point = {0, 0, 0};
		view_point = transform_3d_apply(view_point, view);

		// Apply perspective so that Z-position is reflected in a 2D image
		vec2 p1 = apply_perspective(v1, view_point, perspective);
		vec2 p2 = apply_perspective(v2, view_point, perspective);

		draw_line(p1, p2, context, color);
	}
}
