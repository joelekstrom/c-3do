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
    SHADING_TYPE_FLAT,
    SHADING_TYPE_GORAUD
} SHADING_TYPE;

void render(struct model model, 
			transform_3d transform, 
			transform_3d view, 
			float perspective, 
			struct graphics_context *context, 
			rgb_color color,
			SHADING_TYPE shading_type,
			struct texture *texture,
			rgb_color *wireframe_color);

int main() {
    struct graphics_context *context = create_context(BMP_CONTEXT_TYPE, 800, 800);

    clear(context, black);

	// load .sob-file
    FILE *fp = fopen("model/head.obj", "r");
    if (!fp) {
		fprintf(stderr, "Failed to open model file");
		return 1;
    }

    struct model model = load_model(fp);
	struct texture texture = load_texture("model/head_vcols.bmp");
    fclose(fp);

    // Center camera on 0.0 and a bit back
    transform_3d view = transform_3d_make_translation(context->width / 2.0, context->height / 2.0, 100.0);
    float perspective = 0.0005;

    transform_3d flip_yz = transform_3d_identity;
    flip_yz.sy = -1.0;
    flip_yz.sz = -1.0;
    transform_3d scale = transform_3d_make_scale(400.0, 400.0, 400.0);
    transform_3d translate = transform_3d_make_translation(0.0, 300.0, 1.0);
    transform_3d scale_and_translate = transform_3d_concat(scale, translate);
    render(model, transform_3d_concat(flip_yz, scale_and_translate), view, perspective, context, white, SHADING_TYPE_GORAUD, &texture, NULL);
	
    unload_model(model);
    bmp_context_save(context, "output.bmp");
    destroy_context(context);
    return 0;
}

/**
   Applies perspective to simulate vector positions in 3D-space, relative to a view position
*/
vec2 apply_perspective(vec3 position, vec3 view_point, float amount) {
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
			SHADING_TYPE shading_type,
			struct texture *texture,
			rgb_color *wireframe_color)
{
    for (int i = 0; i < model.num_faces; i++) {
		struct face f = model.faces[i];
		
		// Position vertices in view and apply any transformations
		transform_3d t = transform_3d_concat(transform, view);
		vec3 vertices[3];
		vec2 textures[3];
		float depths[3];
		for (int i = 0; i < 3; ++i) {
			vertices[i] = transform_3d_apply(*f.vertices[i], t);
			depths[i] = vertices[i].z;
			textures[i] = *f.textures[i];
		}

		vec3 view_point = {0, 0, 0};
		view_point = transform_3d_apply(view_point, view);
		vec3 face_normal = surface_normal(vertices);
		vec3 light_direction = {0.0, 0.0, 1.0};
		float light_intensity = dot_product_3d(face_normal, vec3_unit(light_direction));
		if (light_intensity < 0.0) {
			continue; // Back-face culling
		}
		
		// Apply perspective (map 3D-coordinates to a 2D-space)
		vec2 points[3];
		for (int p = 0; p < 3; ++p) {
			points[p] = apply_perspective(vertices[p], view_point, perspective);
		}

		// Flat shading
		if (shading_type == SHADING_TYPE_FLAT) {
			// Calculate a color from the view angle. We interpolate
			// from color to black.
			rgb_color shaded_color = interpolate_color(black, color, light_intensity);
			rgb_color colors[] = {shaded_color, shaded_color, shaded_color}; // Goraud_triangle takes an array of colors
			float intensities[] = {light_intensity, light_intensity, light_intensity};
			triangle(points, colors, intensities, texture, *f.textures, context, depths);
		}

		else if (shading_type == SHADING_TYPE_GORAUD) {
			rgb_color colors[3];
			float intensities[3];
			for (int i = 0; i < 3; ++i) {
				float intensity = dot_product_3d(*f.normals[i], vec3_unit(light_direction));
				colors[i] = interpolate_color(black, color, intensity);
				intensities[i] = intensity;
			}	
			triangle(points, colors, intensities, texture, textures, context, depths);
		}

		// Wireframes
		if (wireframe_color) {
			draw_line(points[0], points[1], context, *wireframe_color);
			draw_line(points[1], points[2], context, *wireframe_color);
			draw_line(points[0], points[2], context, *wireframe_color);
		}
    }
}
