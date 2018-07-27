#include "graph.h"
#include "disjoint_sets.h"
#include "heap.h"

int
vertex_init(vertex_t *vertex)
{
    if (NULL != vertex) {
        vertex->edges = list_new();
        if (NULL != vertex->edges) {
            return 0;
        }
    }
    return -1;
}

vertex_t *
vertex_new(void)
{
    vertex_t *vertex = calloc(1, sizeof(vertex_t));

    if (vertex_init(vertex) < 0) {
        free(vertex);
        vertex = NULL;
    }
    
    return vertex;
}

void
vertex_fini(vertex_t *vertex)
{
    if (NULL != vertex) {
        edge_t *edge = NULL;

        if (NULL != vertex->edges) {
            while (NULL != (edge = list_pop_front(vertex->edges))) {
                edge_free(edge);
            }
            list_free(vertex->edges);
            vertex->edges = NULL;
        }
    }
}

void
vertex_free(vertex_t *vertex)
{
    vertex_fini(vertex);
    free(vertex);
}


edge_t *
edge_new(vertex_t *v1, vertex_t *v2, edge_flags_t flags)
{
    edge_t *edge = calloc(1, sizeof(edge_t));
    
    if (NULL != edge) {
        edge->endpoint1 = v1;
        edge->endpoint2 = v2;
        edge->directed = !!(flags & EDGE_F_DIRECTED);
        edge->state = EDGE_CREATED;
    }

    return edge;
}

void
edge_free(edge_t *edge)
{
    if (edge->state == EDGE_SHARED) {
        edge->state = EDGE_REMOVING;
    }
    else {
        free(edge);
    }
}

vertex_t *
edge_pair_get(edge_t *edge, vertex_t *u)
{
    if (edge->endpoint1 == u) {
        return edge->endpoint2;
    }
    if (edge->endpoint2 == u) {
        return edge->endpoint1;
    }
    return NULL;
}

graph_t *
graph_new(size_t size)
{
    graph_t *graph = calloc(1, sizeof(graph_t));

    if (NULL != graph) {

        graph->size = size;
        graph->vertices = malloc(size * sizeof(vertex_t));

        if (NULL != graph->vertices) {
            for (int i = 0; i < size; ++i) {
                if (vertex_init(&graph->vertices[i]) < 0) {
                    goto error;
                }
                graph->vertices[i].index = i;
                graph->vertices[i].heap_index = i;
            }
        }
    }
    return graph;

error :
    graph_free(graph);
    return NULL;
}

void
graph_free(graph_t *graph)
{
    if (NULL != graph) {
        if (NULL != graph->vertices) {
            for (int i = 0; i < graph->size; ++i) { 
                vertex_fini(&graph->vertices[i]);
            }
            free(graph->vertices);
        }
        free(graph);
    }
}

void
vertex_edge_add(vertex_t *vertex, edge_t *edge)
{
    if (edge->state == EDGE_CREATED) {
        edge->state = EDGE_INSERTED;
    }
    else if (edge->state == EDGE_INSERTED) {
        edge->state = EDGE_SHARED;
    }
    list_push_back(vertex->edges, edge); 
}

vertex_t *
graph_vertex_get(graph_t *graph, int i)
{
    return &graph->vertices[i];
}

bool
graph_connected(graph_t *graph, vertex_t *u, vertex_t *v)
{
    node_t *node = NULL;
    u->visited = 1;
    if (u == v) {
        return true;
    }
    list_foreach(u->edges, node) { 
        edge_t *edge = node_data(node);
        vertex_t *z = edge_pair_get(edge, u);
        if (!z->visited) {
            if (graph_connected(graph, z, v)) {
                return true;
            }
        }
    }
    return false; 
}

void
vertex_visit(vertex_t *u)
{
    node_t *node = NULL;
    u->visited = 1;
    list_foreach(u->edges, node) { 
        edge_t *edge = node_data(node);
        vertex_t *v = edge_pair_get(edge, u);
        if (!v->visited) {
            vertex_visit(v);
        }
    }
}

int
graph_connected_count(graph_t *graph)
{
    int count = 0;
    for (int i = 0; i < graph->size; ++i) {
        if (!graph->vertices[i].visited) {
            ++count;
            vertex_visit(&graph->vertices[i]);
        }
    }
    return count;
}

static bool 
check_cycle(vertex_t *u)
{
    node_t *node = NULL;
    u->visited = 1;
    u->previsit = ++u->clock;
    list_foreach(u->edges, node) { 
        edge_t *edge = node_data(node);
        vertex_t *v = edge_pair_get(edge, u);
        if (!v->visited) {
            if (check_cycle(v)) {
                return true;
            }
        }
        else {
            if (0 == v->postvisit) {
                return true;
            }
        }
    }
    u->postvisit = ++u->clock;
    return false;
}

bool
graph_contains_cycle(graph_t *graph)
{
    for (int i = 0; i < graph->size; ++i) {
        if (!graph->vertices[i].visited) {
            if (check_cycle(&graph->vertices[i])) {
                return true;
            }
        }
    }
    return false;
}

int
graph_distance(graph_t *graph, vertex_t *v, vertex_t *u)
{
    for (int i = 0; i < graph->size; ++i) { 
        graph->vertices[i].visited = 0;
        graph->vertices[i].distance = 0;
    }

    list_t *list = list_new();

    list_push_back(list, v);

    while ((v = list_pop_front(list))) {

        if (v == u) {
            list_free(list);
            return v->distance;
        }

        node_t *node;
        list_foreach(v->edges, node) { 
            edge_t *edge = node_data(node);
            vertex_t *z = edge_pair_get(edge, v);
            if (!z->visited) {
                z->visited = 1;
                z->distance = v->distance + 1;
                list_push_back(list, z);
            }
        }
    }

    list_free(list);

    return -1;
}

bool
graph_is_bipartite(graph_t *graph)
{
    vertex_t *v = NULL;

    for (int i = 0; i < graph->size; ++i) { 
        graph->vertices[i].visited = 0;
        graph->vertices[i].color = WHITE;
    }

    list_t *list = list_new();

    list_push_back(list, &graph->vertices[0]);

    while ((v = list_pop_front(list))) {

        node_t *node;
        list_foreach(v->edges, node) { 
            edge_t *edge = node_data(node);
            vertex_t *u = edge_pair_get(edge, v);
            if (!u->visited) {
                u->visited = 1;
                u->color = v->color == WHITE ? BLACK : WHITE;
                list_push_back(list, u);
            }
            else {
                if (u->color == v->color) {
                    list_free(list);
                    return false;
                }
            }
        }
    }

    list_free(list);

    return true;
}

static bool
vertex_cmp(const void *o1, const void *o2)
{
    const vertex_t *const *v1 = o1;
    const vertex_t *const *v2 = o2;

    return (*v1)->distance <= (*v2)->distance;
}

static void
vertex_update(void *o, size_t heap_index)
{
    vertex_t *const *v = o;

    (*v)->heap_index = heap_index; 
}

static
bool vertex_relax(vertex_t *u, edge_t *edge)
{
    if (u->distance < LONG_MAX) {
        vertex_t *v = edge_pair_get(edge, u);
        if (v->distance > u->distance + (long)edge->weight) {
            v->distance = u->distance + (long)edge->weight;
            v->parent = u;
            return true;
        }
    }
    return false;
}

int
graph_dijkstra_distance(graph_t *graph, vertex_t *u, vertex_t *v)
{
    heap_t *h = NULL;

    vertex_t **vertices = malloc(graph->size * sizeof(vertex_t *));

    for (int i = 0; i < graph->size; ++i) { 
        graph->vertices[i].visited = 0;
        graph->vertices[i].distance = LONG_MAX;
        vertices[i] = &graph->vertices[i];
    }

    u->distance = 0;

    h = heap_build(vertices, graph->size, sizeof(vertex_t *), vertex_cmp, NULL, vertex_update);

    while (NULL != heap_top(h)) {
        heap_pop_front(h, &u);

        if (u == v) {
            free(vertices);
            if (LONG_MAX == u->distance) {
                return -1;
            }
            return u->distance;
        }

        node_t *node;
        list_foreach(u->edges, node) {
            edge_t *edge = node_data(node);
            if (vertex_relax(u, edge)) {
                vertex_t *z = edge_pair_get(edge, u);
                heap_update(h, z->heap_index);
            }
        }
    }

    heap_free(h);

    return -1;   
}

bool
graph_negative_cycle(graph_t *graph)
{
    for (int i = 0; i < graph->size; ++i) { 
        graph->vertices[i].visited = 0;
        graph->vertices[i].color = WHITE;
        graph->vertices[i].distance = LONG_MAX;
        graph->vertices[i].parent = NULL;
    }

    graph->vertices[0].distance = 0;
   
    for (int j = 0; j < graph->size; ++j) {
        bool relaxed = false;
        for (int i = 0; i < graph->size; ++i) {
            vertex_t *u = &graph->vertices[i];
            node_t *node;
            if (LONG_MAX == u->distance) {
                u->distance = 0;
            }
            list_foreach(u->edges, node) {
                edge_t *edge = node_data(node);
                if (vertex_relax(u, edge)) {
                    relaxed = true;
                }
            }
        }
        if (!relaxed) {
            return false;
        }
        if (j == graph->size - 1) {
            return true;
        }
    }
    return false;
}

void
graph_shortest_paths(graph_t *graph, vertex_t *s)
{
    bool negative_cycle = false;
    list_t *cycle = list_new();


    for (int i = 0; i < graph->size; ++i) { 
        graph->vertices[i].visited = 0;
        graph->vertices[i].color = WHITE;
        graph->vertices[i].distance = LONG_MAX;
        graph->vertices[i].parent = NULL;
    }

    s->distance = 0;
   
    for (int j = 0; j < graph->size; ++j) {
        bool relaxed = false;
        for (int i = 0; i < graph->size; ++i) {
            vertex_t *u = &graph->vertices[i];
            node_t *node;
            if (u->distance != LONG_MAX) {
                list_foreach(u->edges, node) {
                    edge_t *edge = node_data(node);
                    if (vertex_relax(u, edge)) {
                        relaxed = true;
                        if (j == graph->size - 1) {
                            list_push_back(cycle, edge_pair_get(edge, u));
                        }
                    }
                }
            }
        }
        if (!relaxed) {
            break;
        }
        if (j == graph->size - 1) {
            negative_cycle = true;
        }
    }

    if (negative_cycle) {
        vertex_t *v;
        while ((v = list_pop_front(cycle))) {
            if (!v->visited) {
                node_t *node;
                v->visited = 1;
                v->distance = LONG_MIN;
                list_foreach(v->edges, node) {
                    edge_t *edge = node_data(node);
                    list_push_back(cycle, edge_pair_get(edge, v));
                }
            }
        }
    }

    list_free(cycle);
}

static bool
mst_cost_cmp(const void *o1, const void *o2)
{
    const vertex_t *const *v1 = o1;
    const vertex_t *const *v2 = o2;

    return (*v1)->cost <= (*v2)->cost;
}

double
graph_mst_prim_cost(graph_t *graph)
{
    double cost = 0.0;
    heap_t *h = NULL;
    vertex_t **vertices = malloc(graph->size * sizeof(vertex_t *));

    for (int i = 0; i < graph->size; ++i) { 
        graph->vertices[i].visited = 0;
        graph->vertices[i].color = WHITE;
        graph->vertices[i].distance = LONG_MAX;
        graph->vertices[i].parent = NULL;
        graph->vertices[i].cost = DBL_MAX;
        vertices[i] = &graph->vertices[i];
    }

    vertex_t *u = &graph->vertices[0];

    u->cost = 0.0;

    h = heap_build(vertices, graph->size, sizeof(vertex_t *), mst_cost_cmp, NULL, vertex_update);

    while (NULL != heap_top(h)) {
        heap_pop_front(h, &u);

        u->visited = 1;

        node_t *node;
        list_foreach(u->edges, node) {
            edge_t *edge = node_data(node);
            vertex_t *v = edge_pair_get(edge, u);
            if (!v->visited) {
                if (v->cost > edge->weight) {
                    v->cost = edge->weight;
                    v->parent = u;
                    heap_update(h, v->heap_index);
                }
            }
        }
    }

    heap_free(h);

    for (int i = 0; i < graph->size; ++i) {
        cost += graph->vertices[i].cost;
    }

    return cost;
}

static bool
edge_cmp(const void *o1, const void *o2)
{
    const edge_t *const *e1 = o1;
    const edge_t *const *e2 = o2;

    return (*e2)->weight <= (*e1)->weight;
}

double
graph_max_distance_k_cluster(vertex_t *vertices, size_t nvertices, edge_t **edges, size_t nedges, int k)
{
    int i;

    for (i = 0; i < nvertices; ++i) {
        vertices[i].heap_index = i;
    }

    disjoint_sets_t *ds = disjoint_sets_new(nvertices);

    disjoint_sets_make_set(ds);

    heap_sort(edges, nedges, sizeof(edge_t *), edge_cmp);

    for (i = 0; i < nedges; ++i) {
        vertex_t *u = edges[i]->endpoint1;
        vertex_t *v = edges[i]->endpoint2;

        if (disjoint_sets_find(ds, u->heap_index) != disjoint_sets_find(ds, v->heap_index)) {
            if (nvertices > k) {
                disjoint_sets_union(ds, u->heap_index, v->heap_index);
                --nvertices;
            }
            else {
                break;
            }
        }
    }

    disjoint_sets_free(ds);

    return edges[i]->weight;
}

graph_t *
graph_reverse(graph_t *graph)
{
    graph_t *graph_r = NULL;

    graph_r = graph_new(graph->size);

    for (int i = 0; i < graph->size; ++i) {
        node_t *node = NULL;
        vertex_t *u = &graph->vertices[i];
        list_foreach(u->edges, node) {
            edge_t *edge = node_data(node);
            vertex_t *v = edge_pair_get(edge, u);
            edge_t *edge_r = edge_new(&graph_r->vertices[v->index], &graph_r->vertices[u->index], EDGE_F_DIRECTED);
            edge_r->weight = edge->weight;
            vertex_edge_add(&graph_r->vertices[v->index], edge_r);
        }
    }
    return graph_r;
}

static bool 
bidirectional_dijkstra_distance(graph_t *graph, heap_t *h, list_t *proc, graph_t *graph_r, list_t *proc_r, long *distance)
{
    node_t *node = NULL;
    vertex_t *v, *v_r;

    *distance = LONG_MAX;

    heap_pop_front(h, &v);

    list_foreach(v->edges, node) {
        edge_t *edge = node_data(node);
        if (vertex_relax(v, edge)) {
            vertex_t *z = edge_pair_get(edge, v);
            heap_update(h, z->heap_index);
        }
    }

    //fprintf(stdout, "from heap (%zu %ld)\n", u->index, u->distance);

    v->visited = 1;
    list_push_back(proc, v);

    v_r = &graph_r->vertices[v->index]; 

    if (v_r->visited) {
        node_t *node;
        list_foreach(proc, node) {
            vertex_t *u   = node_data(node);
            vertex_t *u_r = &graph_r->vertices[u->index];
            //fprintf(stdout, "(%zu %ld) (%zu %ld %d)\n", v->index, v->distance, v_r->index, v_r->distance, v_r->visited);
            if ((u->distance < LONG_MAX && u_r->distance < LONG_MAX) && (u->distance + u_r->distance < *distance)) {
                *distance = u->distance + u_r->distance;
            }
        }
        return true; /**< forward/backward searches met in the middle */
    }
    return false;
}

long
graph_bidirectional_dijkstra_distance(graph_t *graph, graph_t *graph_r, vertex_t *s, vertex_t *t)
{
    long distance = LONG_MAX;
    heap_t *h_r = NULL;
    heap_t *h = NULL;
    list_t *proc = NULL;
    list_t *proc_r = NULL;

    vertex_t **vertices   = malloc(graph->size   * sizeof(vertex_t *));
    vertex_t **vertices_r = malloc(graph_r->size * sizeof(vertex_t *));

    for (int i = 0; i < graph->size; ++i) { 

        graph->vertices[i].visited = 0;
        graph->vertices[i].distance = LONG_MAX;
        vertices[i] = &graph->vertices[i];
        vertices[i]->parent = NULL;

        graph_r->vertices[i].visited = 0;
        graph_r->vertices[i].distance = LONG_MAX;
        vertices_r[i] = &graph_r->vertices[i];
        vertices_r[i]->parent = NULL;
    }

    s->distance = 0;
    graph_r->vertices[t->index].distance = 0;

    h   = heap_build(vertices,   graph->size,   sizeof(vertex_t *), vertex_cmp, NULL, vertex_update);
    h_r = heap_build(vertices_r, graph_r->size, sizeof(vertex_t *), vertex_cmp, NULL, vertex_update);

    proc   = list_new();
    proc_r = list_new();
    
    do {

        if (bidirectional_dijkstra_distance(graph,   h,   proc,   graph_r, proc_r, &distance)) {
            break;
        }

        if (bidirectional_dijkstra_distance(graph_r, h_r, proc_r, graph,   proc,   &distance)) {
            break;
        }

    } while (true);

    list_free(proc);
    list_free(proc_r);

    heap_free(h);
    heap_free(h_r);

    return distance;   
}
