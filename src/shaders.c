#include "shaders.h"
#include "textures.h"

/**
 Applies model and view transforms to a vectors coordinate and normal vector
 */
void apply_transforms(struct vertex *v, transform_3d model, transform_3d view) {
	transform_3d transform = transform_3d_multiply(model, view);
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

vertex_shader goraud_shader(struct vertex_shader_input input) {
	struct vertex v = input.vertex;

	// Apply all transforms. Rotation, scaling, translation etc
	apply_transforms(&v, input.model, input.scene.view);

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
		float dot_product = dot_product_3d(v.normal, vec3_unit(light->direction));
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
vertex_shader flat_shader(struct vertex_shader_input input) {
	struct vertex v = input.vertex;
	apply_transforms(&v, input.model, input.scene.view);
	v.coordinate = apply_perspective(v.coordinate, input.scene.view, input.scene.perspective);
	v.color = input.scene.ambient_light;
	for (int i = 0; i < input.scene.directional_light_count; i++) {
		struct directional_light *light = &input.scene.directional_lights[i];
		float dot_product = dot_product_3d(input.face_normal, vec3_unit(light->direction));
		if (dot_product > 0.0) {
			rgb_color light_color = scale_color(light->intensity, dot_product);
			v.color = add_color(v.color, light_color);
		}
	}
	return v;
}

fragment_shader apply_texture_shader(struct fragment_shader_input input) {
	rgb_color texture_color = texture_sample(*input.texture, input.interpolated_v.texture_coordinate);
	rgb_color light_intensity = input.interpolated_v.color;
	return multiply_colors(light_intensity, texture_color);
}
