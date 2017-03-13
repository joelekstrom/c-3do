#include "graphics_context.h"
#include "geometry.h"
#include "textures.h"
#include "shaders.h"
#include "color.h"
#include "obj.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void draw_context(struct graphics_context *context);

struct model model;
struct texture texture;

int main() {
    struct graphics_context *context = create_context(WINDOW_CONTEXT_TYPE, 800, 800, &draw_context);

	// load .sob-file
    FILE *fp = fopen("model/head.obj", "r");
    if (!fp) {
		fprintf(stderr, "Failed to open model file");
		return 1;
    }

    model = load_model(fp);
	fclose(fp);
	texture = load_texture("model/head_vcols.bmp");

	context_activate(context);

    unload_model(model);
    destroy_context(context);
    return 0;
}

void render_model(struct model model,
				  struct render_options options,
				  struct vertex (*vertex_shader)(struct vertex_shader_input),
				  rgb_color (*fragment_shader)(struct fragment_shader_input),
				  struct graphics_context *context,
				  struct texture *texture,
				  rgb_color *wireframe_color);

void draw_context(struct graphics_context *context) {
	rgb_color clear_color = {0, 0, 0};
	clear(context, clear_color);

	// Center camera on 0.0 and a bit back
    transform_3d view = transform_3d_make_translation(context->width / 2.0, context->height / 2.0, 100.0);
    float perspective = 0.0005;

    transform_3d flip_yz = transform_3d_identity;
    flip_yz.sy = -1.0;
    flip_yz.sz = -1.0;
    transform_3d scale = transform_3d_make_scale(400.0, 400.0, 400.0);
	static float counter = 0.0;
	counter += 0.1;
    transform_3d translate = transform_3d_make_translation(sinf(counter) * 100, 300.0, 1.0);
    transform_3d scale_and_translate = transform_3d_concat(scale, translate);

	rgb_color ambient_light = {0, 0, 0};
	struct directional_light light_1 = {.intensity = {200, 200, 200}, .direction = {0.0, 0.0, 1.0}};
	struct directional_light light_2 = {.intensity = {0, 0, 30}, .direction = {1.0, 0.0, 0.0}};
	struct directional_light light_3 = {.intensity = {0, 50, 0}, .direction = {0.0, 1.0, 0.0}};
	struct directional_light lights[] = {light_1, light_2, light_3};

	struct render_options options = {
		.model = transform_3d_concat(flip_yz, scale_and_translate),
		.view = view,
		.perspective = perspective,
		.ambient_light = ambient_light,
		.directional_lights = (struct directional_light *)&lights,
		.directional_light_count = 3
	};

    render_model(model,
				 options,
				 &goraud_shader,
				 &apply_texture_shader,
				 context,
				 &texture,
				 NULL);
}

void render_model(struct model model,
				  struct render_options options,
				  struct vertex (*vertex_shader)(struct vertex_shader_input),
				  rgb_color (*fragment_shader)(struct fragment_shader_input),
				  struct graphics_context *context,
				  struct texture *texture,
				  rgb_color *wireframe_color)
{
    for (int i = 0; i < model.num_faces; i++) {
		struct face f = model.faces[i];

		// Calculate the face normal which is used for back-face culling and flat shading
		vec3 v = vec3_subtract(*f.vertices[1], *f.vertices[0]);
		vec3 u = vec3_subtract(*f.vertices[2], *f.vertices[0]);
		vec3 face_normal = vec3_unit(cross_product(v, u));
		
		// Create vertex objects that are used by shaders/drawing code
		struct vertex vertices[3];
		for (int v = 0; v < 3; v++) {
			struct vertex vertex = {.coordinate = *f.vertices[v],
									.texture_coordinate = *f.textures[v],
									.normal = *f.normals[v]};

			struct vertex_shader_input shader_input = {.vertex = vertex,
													   .face_normal = face_normal,
													   .options = options};
			vertices[v] = vertex_shader(shader_input);
		}

		// Get the new face normal and drop triangles that are "back facing",
		// aka back-face culling
		v = vec3_subtract(vertices[1].coordinate, vertices[0].coordinate);
		u = vec3_subtract(vertices[2].coordinate, vertices[0].coordinate);
		vec3 new_face_normal = vec3_scale(vec3_unit(cross_product(u, v)), 1.0);
		vec3 camera_direction = {0.0, 0.0, 1.0}; // Camera is always pointing "forward" after all transformations
		float angle = dot_product_3d(new_face_normal, camera_direction);
		if (angle < 0.0) {
			continue; // Back-face culling
        }

		struct fragment_shader_input input;
		input.texture = texture;
		
		triangle(vertices, input, fragment_shader, context);

		// Wireframes
		if (wireframe_color) {
			vec2 p1 = {.x = vertices[0].coordinate.x, .y = vertices[0].coordinate.y};
			vec2 p2 = {.x = vertices[1].coordinate.x, .y = vertices[1].coordinate.y};
			vec2 p3 = {.x = vertices[2].coordinate.x, .y = vertices[2].coordinate.y};
			draw_line(p1, p2, context, *wireframe_color);
			draw_line(p2, p3, context, *wireframe_color);
			draw_line(p1, p3, context, *wireframe_color);
		}
    }
}
