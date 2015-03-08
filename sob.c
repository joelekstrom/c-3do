#include <stdlib.h>
#include "sob.h"

void count_objects(FILE *fp, int *vertices, int *edges, int *faces) {
	char buf[255];

	int num_vertices = 0, num_edges = 0, num_faces = 0;
	while (fgets (buf, sizeof(buf), fp)) {
		switch (buf[0]) {
			case 'v':
				num_vertices++;
				break;

			case 'e':
				num_edges++;
				break;

			case 'f':
				num_faces++;
				break;
		}
	}
	*vertices = num_vertices;
	*edges = num_edges;
	// Add faces when supported
	// *faces = num_faces;
}

vec3 parse_vertex(const char *vertex_str) {
	vec3 vertex;
	sscanf(vertex_str, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
	printf("Load vertex (%f, %f, %f)\n", vertex.x, vertex.y, vertex.z);
	return vertex;
}

edge parse_edge(const char *edge_str, vec3 vertices[]) {
	int v1_i;
	int v2_i;
	sscanf(edge_str, "e %d %d", &v1_i, &v2_i);

	edge e;
	e.v1 = &vertices[v1_i];
	e.v2 = &vertices[v2_i];
	printf("Load edge (%i, %i)\n", v1_i, v2_i);
	return e;	
}

struct model_t load_model(FILE *fp) {
	struct model_t model;
	int num_faces = 0; // Added for future compatibility
	count_objects(fp, &model.num_vertices, &model.num_edges, &num_faces);
	printf("Found %i vertices and %i edges\n", model.num_vertices, model.num_edges);
	model.vertices = malloc(sizeof(vec3) * model.num_vertices);
	model.edges = malloc(sizeof(edge) * model.num_edges);

	rewind(fp);
	char buf[256];
	int vertex_i = 0;
	int edge_i = 0;
	while (fgets (buf, sizeof(buf), fp)) {
		if (buf[0] == 'v') {
			model.vertices[vertex_i++] = parse_vertex(buf);
		} else if (buf[0] == 'e') {
			model.edges[edge_i++] = parse_edge(buf, model.vertices);
		}
	}
	return model;
}

void unload_model(struct model_t model) {
	free(model.vertices);
	free(model.edges);
}