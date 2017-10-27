#include "geometry.h"
#include <stdlib.h>
#include <math.h>

double dot_product_2d(vec2 v1, vec2 v2) {
	return v1.x * v2.x + v1.y * v2.y;
}

double dot_product_3d(vec3 v1, vec3 v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

vec3 cross_product(vec3 v1, vec3 v2) {
	vec3 result = {.x = v1.y * v2.z - v1.z * v2.y,
                   .y = v1.z * v2.x - v1.x * v2.z,
                   .z = v1.x * v2.y - v1.y * v2.x};
	return result;
}

vec3 vec3_unit(vec3 v) {
	double length = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
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

vec3 vec3_scale(vec3 v, double s) {
	vec3 result = {.x = v.x * s,
				   .y = v.y * s,
				   .z = v.z * s};
	return result;
}

/**
 Linearly interpolate, short "lerp", between two double values
 */
double flerp(double a, double b, double value) {
	return a + (b - a) * value;
}

vec2 vec2_lerp(vec2 a, vec2 b, double value) {
	vec2 result = {.x = flerp(a.x, b.x, value), .y = flerp(a.y, b.y, value)};
	return result;
}

vec3 vec3_lerp(vec3 a, vec3 b, double value) {
	vec3 result = {.x = flerp(a.x, b.x, value), .y = flerp(a.y, b.y, value), .z = flerp(a.z, b.z, value)};
	return result;
}

const transform_3d transform_3d_identity = {
    .sx = 1.0, .sy = 1.0, .sz = 1.0, .dm = 1.0
};

transform_3d transform_3d_make_translation(double tx, double ty, double tz) {
	transform_3d t = transform_3d_identity;
	t.tx = tx;
	t.ty = ty;
	t.tz = tz;
	return t;
}

transform_3d transform_3d_make_scale(double sx, double sy, double sz) {
	transform_3d t = transform_3d_identity;
	t.sx = sx;
	t.sy = sy;
	t.sz = sz;
	return t;
}

transform_3d transform_3d_make_rotation_x(double angle) {
	transform_3d t = transform_3d_identity;
	t.sy = cosf(angle); t.by = -sinf(angle);
	t.bz = sinf(angle); t.sz = cosf(angle);
	return t;
}

transform_3d transform_3d_make_rotation_y(double angle) {
	transform_3d t = transform_3d_identity;
	t.sx = cosf(angle);  t.bx = sinf(angle);
	t.az = -sinf(angle); t.sz = cosf(angle);
	return t;
}

transform_3d transform_3d_make_rotation_z(double angle) {
	transform_3d t = transform_3d_identity;
	t.sx = cosf(angle);  t.ax = sinf(angle);
	t.ay = -sinf(angle); t.sy = cosf(angle);
	return t;
}

transform_3d transform_3d_multiply(transform_3d t1, transform_3d t2) {
	transform_3d t;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			double value = 0.0;
			for (int i = 0; i < 4; i++) {
				value += t1.values[i][y] * t2.values[x][i];
			}
			t.values[x][y] = value;
		}
	}
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

transform_3d transform_3d_translate(transform_3d t, double tx, double ty, double tz) {
	t.tx += tx;
	t.ty += ty;
	t.tz += tz;
	return t;
}

transform_3d transform_3d_scale(transform_3d t, double sx, double sy, double sz) {
	t.sx *= sx;
	t.sy *= sy;
	t.sz *= sz;
	return t;
}

transform_3d transform_3d_rotate_y_around_origin(transform_3d t, double angle) {
	// Remove translation so we can rotate around origin, then we add it back
	double tx = t.tx;
	double ty = t.ty;
	double tz = t.tz;
	t = transform_3d_translate(t, -tx, -ty, -tz);
	t = transform_3d_multiply(t, transform_3d_make_rotation_y(angle));
	t = transform_3d_translate(t, tx, ty, tz);
	return t;
}

transform_3d transform_3d_rotate_x_around_origin(transform_3d t, double angle) {
	// Remove translation so we can rotate around origin, then we add it back
	double tx = t.tx;
	double ty = t.ty;
	double tz = t.tz;
	t = transform_3d_translate(t, -tx, -ty, -tz);
	t = transform_3d_multiply(t, transform_3d_make_rotation_x(angle));
	t = transform_3d_translate(t, tx, ty, tz);
	return t;
}
