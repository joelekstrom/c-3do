#include "shaders.h"
#include "textures.h"

vec3 transform_normal(vec3 normal, transform_3d transform) {
	// Remove translations from the matrix so we only apply scale + rotation to
	// the normals. This is not correct for skewed objects. The correct solution
	// for calculating the normal would be to use an inverse transpose matrix
	transform.tx = 1.0;
	transform.ty = 1.0;
	transform.tz = 1.0;

	// Re-normalize and invert the normal since it won't be normalized after scales etc
	return vec3_scale(vec3_unit(transform_3d_apply(normal, transform)), -1.0);;
}

/**
 Applies perspective to simulate vector positions in 3D-space, relative to a view position
*/
vec3 apply_perspective(vec3 position, transform_3d view, double amount) {
	// Convert view matrix to a vector to simplify calculations
	vec3 view_point = {0, 0, 0};
	view_point = transform_3d_apply(view_point, view);
	
    double distance_x = view_point.x - position.x;
    double distance_y = view_point.y - position.y;
    vec3 result = position;
    result.x = position.x + position.z * distance_x * amount;
    result.y = position.y + position.z * distance_y * amount;
    return result;
}

struct vertex goraud_shader(struct vertex_shader_input input) {
	struct vertex v = input.vertex;

	// Apply all transforms. Rotation, scaling, translation etc
	transform_3d transform = transform_3d_multiply(input.model, input.scene.view);
	v.coordinate = transform_3d_apply(v.coordinate, transform);
	v.normal = transform_normal(v.normal, transform);

	// Apply perspective (move x and y further to/away from the middle
	// depending on the Z value, to give the illusion of depth)
	v.coordinate = apply_perspective(v.coordinate, input.scene.view, input.scene.perspective);

	// Start by setting the color to the ambient light
	v.color = input.scene.ambient_light;
		
	// Calculate light intensity by checking the angle of the vertex normal
	// to the angle of the light direction. If vertex is directly facing the
	// light direction, it will be fully illuminated, and if it's >= 90 degrees
	// it will be totally black
	for (int i = 0; i < input.scene.directional_light_count; i++) {
		struct directional_light *light = &input.scene.directional_lights[i];
		double dot_product = dot_product_3d(v.normal, vec3_unit(light->direction));
		if (dot_product > 0.0) {
			rgb_color light_color = scale_color(light->intensity, dot_product);
			v.color = add_color(v.color, light_color);
		}
	}
	return v;
}

/**
 Works the same as the goraud_shader, but uses the face normal for light calculations
 */
struct vertex flat_shader(struct vertex_shader_input input) {
	struct vertex v = input.vertex;
	transform_3d transform = transform_3d_multiply(input.model, input.scene.view);
	v.coordinate = transform_3d_apply(v.coordinate, transform);
	v.normal = transform_normal(v.normal, transform);
	vec3 face_normal = transform_normal(input.face_normal, transform);

	v.coordinate = apply_perspective(v.coordinate, input.scene.view, input.scene.perspective);
	v.color = input.scene.ambient_light;
	for (int i = 0; i < input.scene.directional_light_count; i++) {
		struct directional_light *light = &input.scene.directional_lights[i];
		double dot_product = dot_product_3d(face_normal, vec3_unit(light->direction));
		if (dot_product > 0.0) {
			rgb_color light_color = scale_color(light->intensity, dot_product);
			v.color = add_color(v.color, light_color);
		}
	}
	return v;
}

rgb_color apply_texture_shader(struct fragment_shader_input input) {
	rgb_color texture_color = texture_sample(*input.texture, input.interpolated_v.texture_coordinate);
	rgb_color light_intensity = input.interpolated_v.color;
	return multiply_colors(light_intensity, texture_color);
}
