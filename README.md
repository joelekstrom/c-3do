# c-3do
A small software renderer in C

## Models
c-3do loads models from .sob files (Simple Object Format), which look like this:

```
// 2d box example
v 10 10 0
v 90 10 0
v 10 90 0
v 90 90 0

e 0 1
e 0 2
e 2 3
e 1 3
```

Lines starting with `v` defines vertices in 3D-space, lines starting with `e` defines edges between vertices.
The arguments for `e` is the vertex index. All vertices in an edge must be defined before the edge itself.

The above model does not contain any faces. A face is constructed from 3 edges - and is defined by lines starting with `f`.
Normals cannot be defined but will be automatically calculated from a face.
