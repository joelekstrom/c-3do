#include <stdlib.h>
#include <string.h>
#include "obj.h"

vec3 parse_vertex(FILE *fp) {
	vec3 vertex;
	fscanf(fp, "%f %f %f", &vertex.x, &vertex.y, & vertex.z);
	return vertex;
}

struct face parse_face(FILE *fp, vec3 *vertices, vec3 *normals, vec3 *textures) {
	struct face f;

	// A face consists of 3 tokens (one for each corner), so we need to consume all of them
	for (int i = 0; i < 3; ++i) {
		f.vertices[i] = NULL; f.normals[i] = NULL; f.textures[i] = NULL;
		int vertex_index = -1;
		int texture_index = -1;
		int normal_index = -1;

		char token[256];
		fscanf(fp, "%s", token);

		// .obj doesn't require normals or textures in faces, so we need to check all possible cases
		if (sscanf(token, "%i/%i/%i", &vertex_index, &texture_index, &normal_index) == 3) {
			f.vertices[i] = &vertices[vertex_index - 1];
			f.normals[i] = &normals[normal_index - 1];
			f.textures[i] = &textures[texture_index - 1];
		} else if (sscanf(token, "%i//%i", &vertex_index, &normal_index) == 2) {
			f.vertices[i] = &vertices[vertex_index - 1];
			f.normals[i] = &normals[normal_index - 1];
		} else if (sscanf(token, "%i/%i", &vertex_index, &texture_index) == 2) {
			f.vertices[i] = &vertices[vertex_index - 1];
			f.textures[i] = &textures[texture_index - 1];
		} else if (sscanf(token, "%i", &vertex_index) == 1) {
			f.vertices[i] = &vertices[vertex_index - 1];
		}
	}

	return f;
}

/**
 Increases the size of an array if needed, using realloc(). If the current count is equal to the
 max count, it will double the size.

 Returns the new size, or the old size if nothing was done.
 */
int increase_size_if_needed(void **ptr, size_t object_size, int current_count, int max_count) {
	if (current_count == max_count) {
		int new_size = max_count * 2;
		*ptr = realloc(*ptr, new_size * object_size);
		return new_size;
	}
	return max_count;
}

struct model load_model(FILE *fp) {
	struct model model;
	model.num_vertices = 0;
	model.num_normals = 0;
	model.num_faces = 0;
	model.num_textures = 0;

	// Start by allocating space for 4 of each object, we will realloc later if needed
	int vertex_array_size = 4; 
	int normal_array_size = 4; 
	int face_array_size = 4;
	int texture_array_size = 4;
	model.vertices = malloc(sizeof(vec3) * vertex_array_size);
	model.normals = malloc(sizeof(vec3) * normal_array_size);
	model.faces = malloc(sizeof(struct face) * face_array_size);
	model.textures = malloc(sizeof(vec3) * texture_array_size);

	rewind(fp);
	char token[256];
	while (fscanf(fp, "%s", token) != EOF) {

		// Vertex
		if (strcmp(token, "v") == 0) {
			vertex_array_size = increase_size_if_needed((void **)&model.vertices, sizeof(vec3), model.num_vertices, vertex_array_size);
			model.vertices[model.num_vertices++] = parse_vertex(fp);

		} 

		// Vertex normal
		else if (strcmp(token, "vn") == 0) {
			normal_array_size = increase_size_if_needed((void **)&model.normals, sizeof(vec3), model.num_normals, normal_array_size);
			model.normals[model.num_normals++] = parse_vertex(fp);
		} 

		// Vertex texture
		else if (strcmp(token, "vt") == 0) {
			// Not yet implemented
		}

		// Face
		else if (strcmp(token, "f") == 0) {
			face_array_size = increase_size_if_needed((void **)&model.faces, sizeof(struct face), model.num_faces, face_array_size);
			model.faces[model.num_faces++] = parse_face(fp, model.vertices, model.normals, model.textures);
		}
	}

	printf("Loaded %i vertices, %i normals and %i faces.\n", model.num_vertices, model.num_normals, model.num_faces);

	return model;
}

void unload_model(struct model model) {
	free(model.vertices);
	free(model.normals);
	free(model.faces);
}