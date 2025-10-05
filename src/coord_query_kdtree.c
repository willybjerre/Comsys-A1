#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#include "record.h"
#include "coord_query.h"

// struktur til node i k-d træ
struct kd_node {
    struct kd_node *left;
    struct kd_node *right;
    struct indexed_record *point;
    int axis;
};

// wrapper med record, lon og lat + pointer til første element i record
struct indexed_record {
    struct record *rs;
    double lon;
    double lat;
};

// wrapper til hele k-d træet, så vi kan holde styr på både roden og points-arrayet
struct kd_tree {
    struct kd_node *root;
    struct indexed_record *points;
    int n;
};

// Sammenligningsfunktion ift lon til qsort
int comparrison_lon(const void *a, const void *b) {
    const struct indexed_record *ia = (const struct indexed_record *)a;
    const struct indexed_record *ib = (const struct indexed_record *)b;
    if (ia->lon < ib->lon) return -1;
    if (ia->lon > ib->lon) return 1;
    return 0;
}

// sammenligningsfunktion ift lat til qsort
int comparrison_lat(const void *a, const void *b) {
    const struct indexed_record *ia = (const struct indexed_record *)a;
    const struct indexed_record *ib = (const struct indexed_record *)b;
    if (ia->lat < ib->lat) return -1;
    if (ia->lat > ib->lat) return 1;
    return 0;
}

// rekursiv funktion til at bygge k-d træ
struct kd_node* mk_tree(struct indexed_record *points, int n, int depth) {
    if (n <= 0) return NULL;

    int axis = depth % 2; // 0 for lon, 1 for lat

    if (axis == 0)
        qsort(points, n, sizeof *points, comparrison_lon);
    else
        qsort(points, n, sizeof *points, comparrison_lat);

    int mid = n / 2;

    struct kd_node *node = malloc(sizeof *node);
    if (!node) return NULL;

    node->axis  = axis;
    node->point = &points[mid];
    node->left  = mk_tree(points, mid, depth + 1);
    node->right = mk_tree(points + mid + 1, n - mid - 1, depth + 1);

    return node;
}

// opretter k-d træ og wrapper
struct kd_tree* mk_kdtree(struct record *rs, int n) {
    struct kd_tree *tree = malloc(sizeof *tree);
    if (!tree) return NULL;

    tree->points = malloc(n * sizeof *tree->points);
    if (!tree->points) {
        free(tree);
        return NULL;
    }

    tree->n = n;
    for (int i = 0; i < n; i++) {
        tree->points[i].rs  = &rs[i];
        tree->points[i].lon = rs[i].lon;
        tree->points[i].lat = rs[i].lat;
    }

    tree->root = mk_tree(tree->points, n, 0);
    if (!tree->root && n > 0) {
        free(tree->points);
        free(tree);
        return NULL;
    }

    return tree;
}

// rekursiv free af noder
void free_kd_nodes(struct kd_node *node) {
    if (!node) return;
    free_kd_nodes(node->left);
    free_kd_nodes(node->right);
    free(node);
}

// frigør hele k-d træet og wrapper
void free_kdtree(struct kd_tree *tree) {
    if (!tree) return;
    free_kd_nodes(tree->root);
    free(tree->points);
    free(tree);
}

// rekursiv funktion til at søge i k-d træ
const struct record* lookup_tree(struct kd_node *data,
                                 const struct record *closest_record,
                                 double *best_dist,
                                 double lon, double lat) {
    if (!data) return closest_record;

    double dx = data->point->lon - lon;
    double dy = data->point->lat - lat;
    double d = sqrt(dx*dx + dy*dy);

    if (d < *best_dist) {
        *best_dist = d;
        closest_record = data->point->rs;
    }

    double diff = (data->axis == 0) ? lon - data->point->lon
                                    : lat - data->point->lat;

    struct kd_node *near, *far;
    if (diff < 0) {
        near = data->left;
        far  = data->right;
    } else {
        near = data->right;
        far  = data->left;
    }

    closest_record = lookup_tree(near, closest_record, best_dist, lon, lat);

    if (fabs(diff) < *best_dist)
        closest_record = lookup_tree(far, closest_record, best_dist, lon, lat);

    return closest_record;
}

// funktion til at starte søgning i k-d træ
const struct record* lookup_kdtree(struct kd_tree *tree, double lon, double lat) {
    if (!tree || !tree->root) return NULL;
    double best_dist = INFINITY;
    return lookup_tree(tree->root, NULL, &best_dist, lon, lat);
}

// Starter query-loop
int main(int argc, char** argv) {
    return coord_query_loop(argc, argv,
                            (mk_index_fn)mk_kdtree,
                            (free_index_fn)free_kdtree,
                            (lookup_fn)lookup_kdtree);
}
