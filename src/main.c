#include "graphics_context.h"
#include "geometry.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "obj.h"

rgb_color red = {255, 0, 0};
rgb_color green = {0, 255, 0};
rgb_color blue = {0, 0, 255};
rgb_color white = {255, 255, 255};
rgb_color yellow = {255, 255, 0};
rgb_color black = {0, 0, 0};

typedef enum {
	SHADING_TYPE_WIREFRAME,
	SHADING_TYPE_FLAT,
	SHADING_TYPE_GORAUD
} SHADING_TYPE;

void render(struct model model, 
				 transform_3d transform, 
				 transform_3d view, 
				 float perspective, 
				 struct graphics_context *context, 
				 rgb_color color,
				 SHADING_TYPE shading_type);

int main() {
	struct graphics_context *context = create_context(BMP_CONTEXT_TYPE, 800, 800);

	clear(context, black);

	// load .sob-file
	FILE *fp = fopen("model/human_head.obj", "r");
	if (!fp) {
		fprintf(stderr, "Failed to open model file");
		return 1;
	}

	struct model model = load_model(fp);
	fclose(fp);

	// Center camera on 0.0 and a bit back
	transform_3d view = transform_3d_make_translation(context->width / 2.0, context->height / 2.0, 100.0);
	float perspective = 0.0025;

	transform_3d flip_yz = transform_3d_identity();
	flip_yz.sy = -1.0;
	flip_yz.sz = -1.0;
	transform_3d scale = transform_3d_make_scale(380.0, 380.0, 60.0);
	render(model, transform_3d_concat(flip_yz, scale), view, perspective, context, white, SHADING_TYPE_GORAUD);
	
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

/**
 Calculates the normal vector for a face
 */
vec3 surface_normal(vec3 vertices[3]) {
	vec3 u = vec3_subtract(vertices[2], vertices[0]);
	vec3 v = vec3_subtract(vertices[1], vertices[0]);
	return vec3_unit(cross_product(u, v));
}

void render(struct model model, 
				 transform_3d transform, 
				 transform_3d view, 
				 float perspective, 
				 struct graphics_context *context, 
				 rgb_color color,
				 SHADING_TYPE shading_type) 
{
	for (int i = 0; i < model.num_faces; i++) {
		struct face f = model.faces[i];

		// Position vertices in view and apply any transformations
		transform_3d t = transform_3d_concat(transform, view);
		vec3 v1 = transform_3d_apply(*f.vertices[0], t);
		vec3 v2 = transform_3d_apply(*f.vertices[1], t);
		vec3 v3 = transform_3d_apply(*f.vertices[2], t);

		vec3 view_point = {0, 0, 0};
		view_point = transform_3d_apply(view_point, view);

		vec3 vertices[] = {v1, v2, v3};
		vec3 face_normal = surface_normal(vertices);
		vec3 light_direction = {1.0, 1.0, 1.0};
		float light_intensity = dot_product_3d(face_normal, vec3_unit(light_direction));
		if (light_intensity < 0.0)
			continue; // Back-face culling

		// Apply perspective (map 3D-coordinates to a 2D-space)
		vec2 p1 = apply_perspective(v1, view_point, perspective);
		vec2 p2 = apply_perspective(v2, view_point, perspective);
		vec2 p3 = apply_perspective(v3, view_point, perspective);

		// Wireframe
		if (shading_type == SHADING_TYPE_WIREFRAME) {
			draw_line(p1, p2, context, color);
			draw_line(p2, p3, context, color);
			draw_line(p1, p3, context, color);
		} 

		// Flat shading
		else if (shading_type == SHADING_TYPE_FLAT) {
			// Calculate a color from the view angle. We interpolate
			// from color to black.
			rgb_color shaded_color = interpolate_color(black, color, light_intensity);
			goraud_triangle(p1, p2, p3, shaded_color, shaded_color, shaded_color, context);
		}

		else if (shading_type == SHADING_TYPE_GORAUD) {
			rgb_color p1_color = interpolate_color(black, color, dot_product_3d(*f.normals[0], vec3_unit(light_direction)));
			rgb_color p2_color = interpolate_color(black, color, dot_product_3d(*f.normals[1], vec3_unit(light_direction)));
			rgb_color p3_color = interpolate_color(black, color, dot_product_3d(*f.normals[2], vec3_unit(light_direction)));
			goraud_triangle(p1, p2, p3, p1_color, p2_color, p3_color, context);	
		}
	}
}
