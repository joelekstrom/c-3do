#include "graphics_context.h"
#include "geometry.h"
#include "textures.h"
#include "color.h"
#include "obj.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

rgb_color red = {255, 0, 0};
rgb_color green = {0, 255, 0};
rgb_color blue = {0, 0, 255};
rgb_color white = {255, 255, 255};
rgb_color yellow = {255, 255, 0};
rgb_color black = {0, 0, 0};

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

struct directional_light {
	vec3 direction;
	rgb_color intensity;
};

struct render_options {
	transform_3d model;
	transform_3d view;
	float perspective;
	rgb_color ambient_light;
	struct directional_light *directional_lights;
	int directional_light_count;
};

struct vertex_shader_input {
	struct vertex vertex;
	vec3 face_normal;
	struct render_options options;
};

void render_model(struct model model,
				  struct render_options options,
				  struct vertex (*vertex_shader)(struct vertex_shader_input),
				  struct graphics_context *context,
				  struct texture *texture,
				  rgb_color *wireframe_color);

struct vertex goraud_shader(struct vertex_shader_input input);
struct vertex flat_shader(struct vertex_shader_input input);

void draw_context(struct graphics_context *context) {
	clear(context, black);

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
				 context,
				 &texture,
				 NULL);
}

struct fragment_shader_input {
	struct texture *texture;
	struct texture *normal_map;
};

vec3 apply_perspective(vec3 position, transform_3d view, float amount);
rgb_color fragment_shader(struct vertex * const interpolated_v, void *input);


void render_model(struct model model,
				  struct render_options options,
				  struct vertex (*vertex_shader)(struct vertex_shader_input),
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
		
		triangle(vertices, &input, &fragment_shader, context);
		
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

/**
 Applies model and view transforms to a vectors coordinate and normal vector
 */
void apply_transforms(struct vertex *v, transform_3d model, transform_3d view) {
	transform_3d transform = transform_3d_concat(model, view);
	v->coordinate = transform_3d_apply(v->coordinate, transform);

	// Remove translations from the matrix so we only apply scale + rotation to
	// the normals. This is not correct for skewed objects. The correct solution
	// for calculating the normal would be to use an inverse transpose matrix
	transform.tx = 1.0;
	transform.ty = 1.0;
	transform.tz = 1.0;	
	v->normal = vec3_scale(vec3_unit(transform_3d_apply(v->normal, transform)), -1.0);
}

/**
 Applies perspective to simulate vector positions in 3D-space, relative to a view position
*/
vec3 apply_perspective(vec3 position, transform_3d view, float amount) {
	// Convert view matrix to a vector to simplify calculations
	vec3 view_point = {0, 0, 0};
	view_point = transform_3d_apply(view_point, view);
	
    float distance_x = view_point.x - position.x;
    float distance_y = view_point.y - position.y;
    vec3 result = position;
    result.x = position.x + position.z * distance_x * amount;
    result.y = position.y + position.z * distance_y * amount;
    return result;
}

struct vertex goraud_shader(struct vertex_shader_input input) {
	struct vertex v = input.vertex;

	// Apply all transforms. Rotation, scaling, translation etc
	apply_transforms(&v, input.options.model, input.options.view);

	// Apply perspective (move x and y further to/away from the middle
	// depending on the Z value, to give the illusion of depth)
	v.coordinate = apply_perspective(v.coordinate, input.options.view, input.options.perspective);

	// Start by setting the color to the ambient light
	v.color = input.options.ambient_light;
		
	// Calculate light intensity by checking the angle of the vertex normal
	// to the angle of the light direction. If vertex is directly facing the
	// light direction, it will be fully illuminated, and if it's >= 90 degrees
	// it will be totally black
	for (int i = 0; i < input.options.directional_light_count; i++) {
		struct directional_light *light = &input.options.directional_lights[i];
		float dot_product = dot_product_3d(v.normal, vec3_unit(light->direction));
		if (dot_product > 0.0) {
			rgb_color light_color = scale_color(light->intensity, dot_product);
			v.color = add_color(v.color, light_color);
		}
	}
	return v;
}

struct vertex flat_shader(struct vertex_shader_input input) {
	struct vertex v = input.vertex;
	apply_transforms(&v, input.options.model, input.options.view);
	v.coordinate = apply_perspective(v.coordinate, input.options.view, input.options.perspective);
	v.color = input.options.ambient_light;
	for (int i = 0; i < input.options.directional_light_count; i++) {
		struct directional_light *light = &input.options.directional_lights[i];
		float dot_product = dot_product_3d(input.face_normal, vec3_unit(light->direction));
		if (dot_product > 0.0) {
			rgb_color light_color = scale_color(light->intensity, dot_product);
			v.color = add_color(v.color, light_color);
		}
	}
	return v;
}

rgb_color fragment_shader(struct vertex * const interpolated_v, void *input) {
	struct fragment_shader_input *args = (struct fragment_shader_input *)input;
	rgb_color texture_color = texture_sample(*args->texture, interpolated_v->texture_coordinate);
	rgb_color light_intensity = interpolated_v->color;
	return multiply_colors(light_intensity, texture_color);

	// Since we currently render from white -> black with light intensity,
	// we can use the red channel as an intensity value for the texture
	float intensity = (float)interpolated_v->color.r / (float)255;
	return interpolate_color(black, texture_color, intensity);
}
