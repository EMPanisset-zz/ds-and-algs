#ifndef __GRAPH__H__
#define __GRAPH__H__

#include "includes.h"

typedef enum color color_t;

enum color {
    WHITE,
    BLACK
};

typedef struct vertex vertex_t;

struct vertex {
    vertex_t *parent;
    void *data;
    list_t *edges;
    int clock;
    int previsit;
    int postvisit;
    long distance;
    size_t index;
    size_t heap_index;
    color_t color;
    double cost;
    unsigned visited:1;
};

typedef enum edge_flags edge_flags_t;

enum edge_flags {
    EDGE_F_NONE = 0x00,
    EDGE_F_DIRECTED = 0x01
};

typedef enum edge_state edge_state_t;

enum edge_state {
    EDGE_CREATED   = 0,
    EDGE_INSERTED  = 1,
    EDGE_SHARED    = 2,
    EDGE_REMOVING  = 3
};

typedef struct edge edge_t;

struct edge {
    vertex_t *endpoint1;
    vertex_t *endpoint2;
    long weight;
    unsigned char directed:1;
    unsigned char state:2;
};

typedef struct graph graph_t;

struct graph {
    vertex_t *vertices;
    int size;
};

vertex_t *
vertex_new(void);

void
vertex_free(vertex_t *vertex);

edge_t *
edge_new(vertex_t *v1, vertex_t *v2, edge_flags_t flags);

void
edge_free(edge_t *edge);

graph_t *
graph_new(size_t size);

void
graph_free(graph_t *graph);

void
vertex_edge_add(vertex_t *vertex, edge_t *edge);

vertex_t *
graph_vertex_get(graph_t *graph, int i);

bool
graph_connected(graph_t *graph, vertex_t *u, vertex_t *v);

int 
graph_connected_count(graph_t *graph);

bool
graph_contains_cycle(graph_t *graph);

int
graph_distance(graph_t *graph, vertex_t *v, vertex_t *u);

int
graph_dijkstra_distance(graph_t *graph, vertex_t *v, vertex_t *u);

bool
graph_is_bipartite(graph_t *graph);

bool
graph_negative_cycle(graph_t *graph);

void
graph_shortest_paths(graph_t *graph, vertex_t *s);

double
graph_mst_prim_cost(graph_t *graph);

double
graph_max_distance_k_cluster(vertex_t *vertices, size_t nvertices, edge_t **edges, size_t nedges, int k);

long
graph_bidirectional_dijkstra_distance(graph_t *graph, graph_t *graph_r, vertex_t *s, vertex_t *t);

#endif /* __GRAPH__H__ */
