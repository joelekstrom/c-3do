#include "geometry.h"

transform_3d transform_3d_identity() {
	transform_3d t;
	t.sx = 1, t.ax = 0, t.bx = 0, t.tx = 0;
	t.ay = 0, t.sy = 1, t.by = 0, t.ty = 0;
	t.az = 0, t.bz = 0, t.sz = 1, t.tz = 0;
	t.am = 0, t.bm = 0, t.cm = 0, t.dm = 1;
	return t;
}

transform_3d transform_3d_make_translation(float tx, float ty, float tz) {
	transform_3d t = transform_3d_identity();
	t.tx = tx;
	t.ty = ty;
	t.tz = tz;
	return t;
}

transform_3d transform_3d_make_scale(float sx, float sy, float sz) {
	transform_3d t = transform_3d_identity();
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
	vec3 result;
	result.x = (v.x * t.sx) + (v.y * t.ax) + (v.z * t.bx) + (t.dm * t.tx);
	result.y = (v.x * t.ay) + (v.y * t.sy) + (v.z * t.by) + (t.dm * t.ty);
	result.z = (v.x * t.az) + (v.y * t.bz) + (v.z * t.sz) + (t.dm * t.tz);
	return result;
}