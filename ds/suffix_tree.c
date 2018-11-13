#include "suffix_tree.h"
#include "bitmap.h"
#include "array.h"

typedef struct suffix_node suffix_node_t;
typedef struct suffix_end suffix_end_t;

struct suffix_end {
    int refcnt;
    int index;
};

struct suffix_node {
    bitmap_t *bitmap;
    int start;
    suffix_end_t *end;
    suffix_node_t *suffix_link;
    array_t *nodes;
};

struct suffix_tree {
    suffix_node_t *root;
    const char* string;
};

typedef struct suffix_active_point suffix_active_point_t;

struct suffix_active_point {
    suffix_node_t *node; /* active node */
    int edge; /* string index for active edge */
    int len;  /* active path length */
};

suffix_end_t *
suffix_end_new(void)
{
    suffix_end_t *end = calloc(1, sizeof(*end));
    if (NULL != end) {
        end->refcnt = 1;
    }
    return end;
}

void
suffix_end_free(suffix_end_t *end)
{
    if (NULL != end) {
        if (0 == --end->refcnt) {
            free(end);
        }
    }
}

void
suffix_end_incref(suffix_end_t *end)
{
    ++end->refcnt;
}

void
suffix_node_free(suffix_node_t *node)
{
    if (NULL != node) {
        bitmap_free(node->bitmap);
        node->bitmap = NULL;
        suffix_node_t **next = NULL;
        array_foreach(node->nodes, next) {
            suffix_node_free(*next);
        } 
        array_free(node->nodes);
        node->nodes = NULL;
        suffix_end_free(node->end);
        node->end = NULL;
        free(node);
    }
}

suffix_node_t *
suffix_node_new(void)
{
    suffix_node_t *node = calloc(1, sizeof(*node));

    if (NULL == node) {
        goto error;
    }

    node->bitmap = bitmap_new();
    if (NULL == node->bitmap) {
        goto error;
    }
    
    node->nodes = array_new(1, sizeof(suffix_node_t *));
    if (NULL == node->nodes) {
        goto error;
    }

    return node;
    
error:
    suffix_node_free(node);
    return NULL;
}

void
suffix_node_print(suffix_node_t *node)
{
    fprintf(stdout, "<%d, %d>\n", node->start, node->end->index);
}

suffix_node_t *
suffix_node_child(suffix_node_t *parent, char c)
{
    if (bitmap_is_bit_set(parent->bitmap, c)) {
        suffix_node_t **child = NULL;
        int offset = bitmap_offset(parent->bitmap, c);
        child = array_get(parent->nodes, offset);
        return *child;
    }
    return NULL;
}

void
suffix_node_insert(suffix_node_t *parent, suffix_node_t *child, char c)
{
    int offset = bitmap_offset(parent->bitmap, c);
    array_insert(parent->nodes, &child, offset);
    bitmap_bit_set(parent->bitmap, c);
}

void
suffix_node_leaf_insert(suffix_node_t *parent, int start, suffix_end_t *end, const char *string)
{
    suffix_node_t *child;
    child = suffix_node_new();
    child->start = start;
    child->end = end;
    suffix_end_incref(end);
    suffix_node_insert(parent, child, string[start]);
}

void
suffix_node_remove(suffix_node_t *parent, char c)
{
    suffix_node_t *child = NULL;
    int offset = bitmap_offset(parent->bitmap, c);
    array_remove(parent->nodes, &child, offset);
    bitmap_bit_clear(parent->bitmap, c); 
}

bool
suffix_node_is_leaf(suffix_node_t *node)
{
    return 0 == array_size(node->nodes); 
}

size_t
suffix_node_len(suffix_node_t *node)
{
    return node->end->index - node->start + 1;
}

void
suffix_link_update(suffix_node_t *root, suffix_node_t *node, suffix_node_t *last_internal, int phase, const char *msg)
{
    if (NULL != last_internal && root == last_internal->suffix_link) {
        if (!suffix_node_is_leaf(node) && root != node) {
            last_internal->suffix_link = node;
            fprintf(stdout, "%s:%s:%d <%d, %d> -> <%d, %d>\n",
                    __func__, msg, phase,
                    last_internal->start, last_internal->end->index,
                    node->start, node->end->index);
        }
    }
}

void
suffix_tree_walk_down(suffix_active_point_t *active_point, const char *string, suffix_node_t **parent, suffix_node_t **child)
{
    *child = suffix_node_child(*parent, string[active_point->edge]);

    while (active_point->len > suffix_node_len(*child)) {
        active_point->edge += suffix_node_len(*child);
        active_point->len  -= suffix_node_len(*child);
        active_point->node  = *child;
        *parent = *child;
        *child = suffix_node_child(*parent, string[active_point->edge]);
    }
}

void
suffix_tree_build(suffix_tree_t *tree)
{
    suffix_node_t *root = tree->root;
    const char *string = tree->string;
    int phase_cnt = strlen(string);
    suffix_end_t *end = suffix_end_new();
    suffix_active_point_t active_point;
    int remaining = 0;

    active_point.node = root;
    active_point.edge = -1;
    active_point.len  =  0;

    int i;
    for (i = 0; i < phase_cnt; ++i) {
        suffix_node_t *last_internal = NULL;
        end->index = i;
        ++remaining;

        while (remaining) {
            suffix_node_t *parent = NULL;
            suffix_node_t *child = NULL;

            parent = active_point.node;

            if (0 == active_point.len) {

                child = suffix_node_child(parent, string[i]);

                if (NULL != child) {
                    active_point.edge = child->start;
                    active_point.len = 1;
                    break;
                }
                suffix_node_leaf_insert(parent, i, end, string);
                --remaining;
            }
            else {

                suffix_tree_walk_down(&active_point, string, &parent, &child);

                if (active_point.len < suffix_node_len(child)) {

                    int next = child->start + active_point.len;
                    if (string[next] == string[i]) {
                        ++active_point.len;
                        break;
                    }

                    suffix_node_remove(parent, string[active_point.edge]);
                    
                    suffix_end_t *new_end = suffix_end_new();
                    new_end->index = next - 1;

                    suffix_node_t *internal = suffix_node_new();
                    internal->start = child->start;
                    internal->end = new_end;
                    if (NULL != last_internal) {
                        last_internal->suffix_link = internal;
                    }
                    internal->suffix_link = root;
                    last_internal = internal;

                    child->start = next;
                    suffix_node_insert(internal, child, string[next]);

                    suffix_node_leaf_insert(internal, i, end, string);
                    --remaining;

                    suffix_node_insert(parent, internal, string[internal->start]);

                }
                else {
                    parent = child;

                    child = suffix_node_child(parent, string[i]);

                    if (NULL != child) {
                        active_point.node = parent;
                        active_point.edge = child->start;
                        active_point.len = 1;
                        suffix_link_update(root, parent, last_internal, i, "from internal");
                        break;
                    }
    
                    suffix_link_update(root, parent, last_internal, i, "from leaf");
                    suffix_node_leaf_insert(parent, i, end, string);
                    --remaining;
                }

                if (active_point.node == root) {
                    --active_point.len;
                    if (0 == active_point.len) {
                        active_point.edge = -1;
                    }
                    else {
                        ++active_point.edge;
                    }
                }
                else {

                    //fprintf(stdout, "Getting a ride on suffix link <%d, %d, %d> -> <%d, %d, %d>\n",
                    //    active_point.node->start, active_point.node->end ? active_point.node->end->index : 0, active_point.node == root,
                    //    active_point.node->suffix_link->start, active_point.node->suffix_link->end ? active_point.node->suffix_link->end->index : 0, active_point.node->suffix_link == root);

                    active_point.node = active_point.node->suffix_link;
                }
            }
        }
    }

    suffix_end_free(end);
}

void
suffix_tree_node_print(suffix_tree_t *tree, const char *string, suffix_node_t *node)
{
    int i;
    suffix_node_t **next = NULL;
    if (NULL != node->end) {
        for (i = node->start; i <= node->end->index; ++i) {
            fprintf(stdout, "%c", string[i]);
        }
        fprintf(stdout, "\n");
        //fprintf(stdout, "< %d, %d, %d, %d>\n", node->start, node->end ? node->end->index : 0, node->suffix_link != NULL, node->suffix_link == tree->root);
    }
    array_foreach(node->nodes, next) {
        suffix_tree_node_print(tree, string, *next);
    }
}

void
suffix_tree_print(suffix_tree_t *tree)
{
    const char *string = tree->string;

    suffix_tree_node_print(tree, string, tree->root);
}

void
suffix_tree_free(suffix_tree_t *tree)
{
    if (NULL != tree) {
        suffix_node_free(tree->root);
        free(tree);
    }
}

suffix_tree_t *
suffix_tree_new(const char* string)
{
    suffix_tree_t *tree = calloc(1, sizeof(*tree));
    if (NULL != tree) {
        tree->root = suffix_node_new();
        if (NULL != tree->root) {
            tree->string = string;
            return tree;
        }
        suffix_tree_free(tree);
    }
    return NULL;
}

int
main(int argc, char **args)
{
    char *text = NULL;
    //suffix_tree_t *tree = suffix_tree_new("mississi$");
    //suffix_tree_t *tree = suffix_tree_new("abcabxabcd");
    //suffix_tree_t *tree = suffix_tree_new("xyzxyaxyz$");
    scanf("%ms", &text);
    suffix_tree_t *tree = suffix_tree_new(text);
    suffix_tree_build(tree);
    suffix_tree_print(tree);
    suffix_tree_free(tree);
    free(text);
    return 0;
}
