#include "graphics_context.h"
#include "geometry.h"
#include "textures.h"
#include "shaders.h"
#include "scene.h"
#include "color.h"
#include "obj.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

typedef char * string;

struct object {
	struct model model;
	struct texture texture;
	struct texture normal_map;
	transform_3d transform;
};

struct object object;
struct scene scene;

void prepare_object();
void on_window_event(struct graphics_context *context, SDL_Event event);
void render(struct graphics_context *context);

int main() {
    struct graphics_context *context = create_context(800, 800);
	context->window_event_callback = &on_window_event;
	prepare_object("model/head.obj", "model/head_vcols.bmp", "model/head_normals.bmp");

	// Center camera on 0.0 and a bit back
	scene.view = transform_3d_make_translation(context->width / 2.0, context->height / 2.0, 100.0);
	scene.perspective = 0.0005;
	scene.ambient_light = (rgb_color){0, 0, 0};
	struct directional_light light_1 = {.intensity = {200, 200, 200}, .direction = {0.0, 0.0, 1.0}};
	struct directional_light light_2 = {.intensity = {0, 0, 0}, .direction = {1.0, 0.0, 0.0}};
	struct directional_light light_3 = {.intensity = {100, 140, 100}, .direction = {0.0, 1.0, 0.0}};
	struct directional_light lights[] = {light_1, light_2, light_3};
	scene.directional_lights = lights;
	scene.directional_light_count = sizeof(lights) / sizeof(lights[0]);

	context_activate_window(context);

	unload_model(object.model);
	unload_texture(object.texture);
	unload_texture(object.normal_map);
	destroy_context(context);
	return 0;
}

void prepare_object(string file, string texture, string normal_map) {
	FILE *fp = fopen(file, "r");
    if (!fp) {
		fputs("Failed to open model file", stderr);
		abort();
    }

	object.model = load_model(fp);
	fclose(fp);

	object.texture = load_texture(texture);
	object.normal_map = load_texture(normal_map);

	object.transform = transform_3d_identity;
	object.transform = transform_3d_scale(object.transform, 400.0, -400.0, -400.0); // Flip Y and Z axis to fit coordinate space
	object.transform = transform_3d_multiply(object.transform, transform_3d_make_rotation_y(0.3));
	object.transform = transform_3d_translate(object.transform, 0, 300, 0);
}

void render_object(struct object object,
				   struct scene scene,
				   vertex_shader (*vertex_shader)(struct vertex_shader_input),
				   fragment_shader (*fragment_shader)(struct fragment_shader_input),
				   struct graphics_context *context,
				   rgb_color *wireframe_color)
{
    for (int i = 0; i < object.model.num_faces; i++) {
		struct face f = object.model.faces[i];

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
													   .model = object.transform,
													   .scene = scene};
			vertices[v] = vertex_shader(shader_input);
		}

		// Get the new face normal and drop triangles that are "back facing",
		// aka back-face culling
		v = vec3_subtract(vertices[1].coordinate, vertices[0].coordinate);
		u = vec3_subtract(vertices[2].coordinate, vertices[0].coordinate);
		vec3 new_face_normal = vec3_scale(vec3_unit(cross_product(u, v)), 1.0);
		vec3 camera_direction = {0.0, 0.0, 1.0}; // Camera is always pointing "forward" after all transformations
		double angle = dot_product_3d(new_face_normal, camera_direction);
		if (angle < 0.0) {
			continue; // Back-face culling
        }

		struct fragment_shader_input input;
		input.texture = &object.texture;
		input.normal_map = &object.normal_map;
		input.scene = scene;
		
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

void render(struct graphics_context *context) {
	rgb_color clear_color = {0, 0, 0};
	clear(context, clear_color);
	render_object(object, scene, &goraud_shader, &apply_texture_shader, context, NULL);
	context_refresh_window(context);
}

void on_window_event(struct graphics_context *context, SDL_Event event) {
	static bool mouse1_down = false;
	static bool mouse3_down = false;

	switch (event.type) {
	case SDL_WINDOWEVENT:
		if (event.window.event == SDL_WINDOWEVENT_SHOWN) {
			render(context);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button == 1)
			mouse1_down = true;
		else if (event.button.button == 3)
			mouse3_down = true;
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button == 1)
			mouse1_down = false;
		else if (event.button.button == 3)
			mouse3_down = false;
		break;
	case SDL_MOUSEMOTION:
		if (mouse1_down) {
			object.transform = transform_3d_translate(object.transform, event.motion.xrel, event.motion.yrel, 0.0);
			render(context);
		} else if (mouse3_down) {
			object.transform = transform_3d_rotate_y_around_origin(object.transform, event.motion.xrel * -0.02);
			object.transform = transform_3d_rotate_x_around_origin(object.transform, event.motion.yrel * 0.02);
			render(context);
		}
		break;
	case SDL_MOUSEWHEEL: {
		double delta = 1.0 - (event.wheel.y * 0.01);
		object.transform = transform_3d_scale(object.transform, delta, delta, delta);
		render(context);
		break;
	}
	default:
		break;
	}
}
