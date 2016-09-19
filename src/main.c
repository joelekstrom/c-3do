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

typedef enum {
    SHADING_TYPE_FLAT,
    SHADING_TYPE_GORAUD
} SHADING_TYPE;

void render(struct model model, 
			transform_3d transform, 
			transform_3d view, 
			float perspective,
			vec3 light_direction,
			rgb_color color,
			struct graphics_context *context, 
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

	// Add a light source pointing straight forward from the camera
	vec3 light_direction = {.x = 0.0, .y = 0.0, .z = 1.0};

    transform_3d flip_yz = transform_3d_identity;
    flip_yz.sy = -1.0;
    flip_yz.sz = -1.0;
    transform_3d scale = transform_3d_make_scale(400.0, 400.0, 400.0);
    transform_3d translate = transform_3d_make_translation(0.0, 300.0, 1.0);
    transform_3d scale_and_translate = transform_3d_concat(scale, translate);
    render(model,
		   transform_3d_concat(flip_yz, scale_and_translate),
		   view,
		   perspective,
		   light_direction,
		   white,
		   context,
		   SHADING_TYPE_GORAUD,
		   &texture,
		   NULL);
	
    unload_model(model);
    bmp_context_save(context, "output.bmp");
    destroy_context(context);
    return 0;
}

vec3 apply_perspective(vec3 position, transform_3d view, float amount);
void goraud_shader(struct vertex *v, transform_3d model, transform_3d view, float perspective, vec3 light_direction);
void flat_shader(struct vertex *v, transform_3d model, transform_3d view, float perspective, vec3 light_direction, vec3 face_normal);
rgb_color fragment_shader(struct vertex * const interpolated_v, void *input);

struct fragment_shader_input {
	struct texture *texture;
	struct texture *normal_map;
};

void render(struct model model, 
			transform_3d transform, 
			transform_3d view, 
			float perspective,
			vec3 light_direction,
			rgb_color color,
			struct graphics_context *context, 
			SHADING_TYPE shading_type,
			struct texture *texture,
			rgb_color *wireframe_color)
{
    for (int i = 0; i < model.num_faces; i++) {
		struct face f = model.faces[i];

		// Calculate the face normal which is used for back-face culling and flat shading
		vec3 v = vec3_subtract(*f.vertices[1], *f.vertices[0]);
		vec3 u = vec3_subtract(*f.vertices[2], *f.vertices[0]);
		vec3 face_normal = vec3_unit(cross_product(u, v));
		
		// Create vertex objects that are used by shaders/drawing code
		struct vertex vertices[3];
		for (int v = 0; v < 3; v++) {
			struct vertex vertex;
			vertex.coordinate = *f.vertices[v];
			vertex.texture_coordinate = *f.textures[v];
			vertex.normal = *f.normals[v];
			vertex.color = color;
		
			// Apply the vertex shader
			switch (shading_type) {
			case SHADING_TYPE_GORAUD:
				goraud_shader(&vertex, transform, view, perspective, light_direction);
				break;
			case SHADING_TYPE_FLAT:
				flat_shader(&vertex, transform, view, perspective, light_direction, face_normal);
				break;
			}
			
			vertices[v] = vertex;
		}

		// Get the new face normal and drop triangles that are "back facing",
		// aka back-face culling
		v = vec3_subtract(vertices[1].coordinate, vertices[0].coordinate);
		u = vec3_subtract(vertices[2].coordinate, vertices[0].coordinate);
		vec3 new_face_normal = vec3_scale(vec3_unit(cross_product(u, v)), 1.0);
		float angle = dot_product_3d(new_face_normal, vec3_unit(light_direction));
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

void goraud_shader(struct vertex *v, transform_3d model, transform_3d view, float perspective, vec3 light_direction) {
	// Apply all transforms. Rotation, scaling, translation etc
	apply_transforms(v, model, view);

	// Apply perspective (move x and y further to/away from the middle
	// depending on the Z value, to give the illusion of depth)
	v->coordinate = apply_perspective(v->coordinate, view, perspective);
		
	// Calculate light intensity by checking the angle of the vertex normal
	// to the angle of the light direction. If vertex is directly facing the
	// light direction, it will be fully illuminated, and if it's >= 90 degrees
	// it will be totally black
	float light_intensity = dot_product_3d(v->normal, vec3_unit(light_direction));
	v->color = interpolate_color(black, v->color, light_intensity);
}

void flat_shader(struct vertex *v, transform_3d model, transform_3d view, float perspective, vec3 light_direction, vec3 face_normal) {
	apply_transforms(v, model, view);
	v->coordinate = apply_perspective(v->coordinate, view, perspective);
	float light_intensity = dot_product_3d(face_normal, vec3_unit(light_direction));
	v->color = interpolate_color(black, v->color, light_intensity);
}

rgb_color fragment_shader(struct vertex * const interpolated_v, void *input) {
	struct fragment_shader_input *args = (struct fragment_shader_input *)input;
	rgb_color texture_color = texture_sample(*args->texture, interpolated_v->texture_coordinate);

	// Since we currently render from white -> black with light intensity,
	// we can use the red channel as an intensity value for the texture
	float intensity = (float)interpolated_v->color.r / (float)255;
	return interpolate_color(black, texture_color, intensity);
}
