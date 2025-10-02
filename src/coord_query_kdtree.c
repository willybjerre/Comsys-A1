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


struct kd_node {
  struct kd_node *left;
  struct kd_node *right;
  struct indexed_record *point;
  int axis; 
};

struct indexed_record {
  struct record *rs;
    double lon;
    double lat;
};

int comparrison_lon(const void *a, const void *b) {
    const struct indexed_record *ia = a;
    const struct indexed_record *ib = b;
    if (ia->lon < ib->lon) return -1;
    if (ia->lon > ib->lon) return 1;
    return 0;
}

int comparrison_lat(const void *a, const void *b) {
    const struct indexed_record *ia = a;
    const struct indexed_record *ib = b;
    if (ia->lat < ib->lat) return -1;
    if (ia->lat > ib->lat) return 1;
    return 0;
}
struct kd_node* mk_tree(struct indexed_record *points, int n, int depth) {
    if (n <= 0) {
        return NULL;
    }

    int axis = depth % 2;

    if (axis == 0)
        qsort(points, n, sizeof *points, comparrison_lon);
    else
        qsort(points, n, sizeof *points, comparrison_lat);

    int mid = n / 2;

    struct kd_node* node = malloc(sizeof *node);
    if (!node) return NULL;

    node->axis  = axis;
    node->point = &points[mid]; 
    node->left  = mk_tree(points, mid, depth + 1);
    node->right = mk_tree(points + mid + 1, n - mid - 1, depth + 1);

    return node;
}

struct kd_node* mk_kdtree(struct record* rs, int n) {
    struct indexed_record *points = malloc(n * sizeof *points);
    if (!points) return NULL;

    for (int i = 0; i < n; i++) {
        points[i].rs  = &rs[i];
        points[i].lon = rs[i].lon;
        points[i].lat = rs[i].lat;
    }

    return mk_tree(points, n, 0);
}


void free_kdtree(struct kd_node *node) {
    if (!node) return;
    free_kdtree(node->left);
    free_kdtree(node->right);
    free(node);
}

const struct record* lookup_tree(struct kd_node *data, const struct record *closest_record, double *best_dist, double lon, double lat){
    if (!data) {
        return closest_record;
    }
    double dx = data->point->lon - lon;
    double dy = data->point->lat - lat;
    double d = sqrt(dx*dx + dy*dy);

    if (d < *best_dist) {
        *best_dist = d;
        closest_record = data->point->rs;
    }
    double diff;
    if (data->axis == 0) {
        diff = lon - data->point->lon;  
    } else {
        diff = lat - data->point->lat; 
    }

    struct kd_node *near, *far;
    if (diff < 0) {
        near = data->left;
        far  = data->right;
    } else {
        near = data->right;
        far  = data->left;
    }
    closest_record = lookup_tree(near, closest_record, best_dist, lon, lat);

    if (fabs(diff) < *best_dist) {
        closest_record = lookup_tree(far, closest_record, best_dist, lon, lat);
    }

    return closest_record;

}

const struct record* lookup_kdtree(struct kd_node *data, double lon, double lat) {
    if (!data) {
        return NULL;
    }
    double best_dist = INFINITY;
    return lookup_tree(data, NULL, &best_dist, lon, lat);
    
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kdtree,
                          (free_index_fn)free_kdtree,
                          (lookup_fn)lookup_kdtree);
}
