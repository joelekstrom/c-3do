#ifndef GEOMETRY_H
#define GEOMETRY_H

typedef struct vec2 {
	double x;
	double y;
} vec2;

typedef struct vec3 {
	double x;
	double y;
	double z;
} vec3;

typedef union transform_3d {
	struct {
		double sx, ax, bx, tx;
		double ay, sy, by, ty;
		double az, bz, sz, tz;
		double am, bm, cm, dm;
	};
	double values[4][4];
} transform_3d;

double dot_product_2d(vec2 v1, vec2 v2);
double dot_product_3d(vec3 v1, vec3 v2);
vec3 cross_product(vec3 v1, vec3 v2);

vec3 vec3_unit(vec3 v);
vec3 vec3_subtract(vec3 v1, vec3 v2);
vec3 vec3_add(vec3 v1, vec3 v2);
vec3 vec3_scale(vec3 v, double s);

// Linear interpolation
double flerp(double a, double b, double value);
vec2 vec2_lerp(vec2 a, vec2 b, double value);
vec3 vec3_lerp(vec3 a, vec3 b, double value);

// Transforms
extern const transform_3d transform_3d_identity;
transform_3d transform_3d_make_translation(double tx, double ty, double sz);
transform_3d transform_3d_make_scale(double sx, double sy, double sz);
transform_3d transform_3d_make_rotation_x(double angle);
transform_3d transform_3d_make_rotation_y(double angle);
transform_3d transform_3d_make_rotation_z(double angle);
transform_3d transform_3d_multiply(transform_3d t1, transform_3d t2);
vec3 transform_3d_apply(vec3 v, transform_3d t);

transform_3d transform_3d_translate(transform_3d t, double tx, double ty, double sz);
transform_3d transform_3d_scale(transform_3d t, double sx, double sy, double sz);
transform_3d transform_3d_rotate_y_around_origin(transform_3d t, double angle);
transform_3d transform_3d_rotate_x_around_origin(transform_3d t, double angle);

#endif
