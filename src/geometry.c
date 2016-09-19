#include "geometry.h"
#include <stdlib.h>
#include <math.h>

float dot_product_2d(vec2 v1, vec2 v2) {
	return v1.x * v2.x + v1.y * v2.y;
}

float dot_product_3d(vec3 v1, vec3 v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

vec3 cross_product(vec3 v1, vec3 v2) {
	vec3 result = {.x = v1.y * v2.z - v1.z * v2.y,
                   .y = v1.z * v2.x - v1.x * v2.z,
                   .z = v1.x * v2.y - v1.y * v2.x};
	return result;
}

vec3 vec3_unit(vec3 v) {
	float length = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	vec3 result = {v.x / length, v.y / length, v.z / length};
	return result;
}

vec3 vec3_subtract(vec3 v1, vec3 v2) {
	vec3 result = {.x = v1.x - v2.x,
                   .y = v1.y - v2.y,
                   .z = v1.z - v2.z};
	return result;
}

vec3 vec3_add(vec3 v1, vec3 v2) {
	vec3 result = {.x = v1.x + v2.x,
                   .y = v1.y + v2.y,
                   .z = v1.z + v2.z};
	return result;
}

vec3 vec3_scale(vec3 v, float s) {
	vec3 result = {.x = v.x * s,
				   .y = v.y * s,
				   .z = v.z * s};
	return result;
}

/**
 Linearly interpolate, short "lerp", between two float values
 */
float flerp(float a, float b, float value) {
	return a + (b - a) * value;
}

vec2 vec2_lerp(vec2 a, vec2 b, float value) {
	vec2 result = {.x = flerp(a.x, b.x, value), .y = flerp(a.y, b.y, value)};
	return result;
}

vec3 vec3_lerp(vec3 a, vec3 b, float value) {
	vec3 result = {.x = flerp(a.x, b.x, value), .y = flerp(a.y, b.y, value), .z = flerp(a.z, b.z, value)};
	return result;
}

const transform_3d transform_3d_identity = {
    .sx = 1.0, .sy = 1.0, .sz = 1.0, .dm = 1.0
};

transform_3d transform_3d_make_translation(float tx, float ty, float tz) {
	transform_3d t = transform_3d_identity;
	t.tx = tx;
	t.ty = ty;
	t.tz = tz;
	return t;
}

transform_3d transform_3d_make_scale(float sx, float sy, float sz) {
	transform_3d t = transform_3d_identity;
	t.sx = sx;
	t.sy = sy;
	t.sz = sz;
	return t;
}

transform_3d transform_3d_concat(transform_3d t1, transform_3d t2) {
	transform_3d t;
	t.sx = t1.sx * t2.sx, t.ax = 0, t.bx = 0, t.tx = t1.tx + t2.tx;
	t.ay = 0, t.sy = t1.sy * t2.sy, t.by = 0, t.ty = t1.ty + t2.ty;
	t.az = 0, t.bz = 0, t.sz = t1.sz * t2.sz, t.tz = t1.tz + t2.tz;
	t.am = 0, t.bm = 0, t.cm = 0, t.dm = t1.dm * t2.dm;
	return t;
}

vec3 transform_3d_apply(vec3 v, transform_3d t) {
	vec3 result = {
        .x = (v.x * t.sx) + (v.y * t.ax) + (v.z * t.bx) + (t.dm * t.tx),
        .y = (v.x * t.ay) + (v.y * t.sy) + (v.z * t.by) + (t.dm * t.ty),
        .z = (v.x * t.az) + (v.y * t.bz) + (v.z * t.sz) + (t.dm * t.tz)
    };
	return result;
}

int compare_vertex_y(const void *a, const void *b) {
	vec3 *v1 = (vec3 *)a;
	vec3 *v2 = (vec3 *)b;
	if (v1->y < v2->y)
		return -1;
	else if (v1->y > v2->y)
		return 1;
	return 0;
}

void sort_vertices_y(vec3 vertices[], int count) {
	qsort(vertices, count, sizeof(vec3), compare_vertex_y);
}

void get_bounding_box_2d(vec2 vertices[], int count, float *min_x, float *min_y, float *max_x, float *max_y) {
	float x_min = vertices[0].x;
	float y_min = vertices[0].y;
	float x_max = x_min;
	float y_max = y_min;

	for (int i = 1; i < count; i++) {
		vec2 v = vertices[i];
		if (v.x < x_min) x_min = v.x;
		if (v.y < y_min) y_min = v.y;
		if (v.x > x_max) x_max = v.x;
		if (v.y > y_max) y_max = v.y;
	}

	*min_x = x_min;
	*min_y = y_min;
	*max_x = x_max;
	*max_y = y_max;
}
