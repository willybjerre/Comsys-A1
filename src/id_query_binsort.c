#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include "record.h" 
#include "id_query.h"

struct indexed_record {
int64_t osm_id;
const struct record *record;
};

struct indexed_data{
struct indexed_record *irs;
int n;
};

int comparrison(const void *a, const void *b) {
    const struct indexed_record *ia = a;
    const struct indexed_record *ib = b;

    if (ia->osm_id < ib->osm_id) return -1;
    if (ia->osm_id > ib->osm_id) return 1;
    return 0;
}

struct indexed_data* mk_binary(struct record* rs, int n) {
    struct indexed_data *data = malloc(sizeof *data);
    if (!data) {
        return NULL;
    };
    data->irs = malloc(n * sizeof(struct indexed_record));
    if (!data->irs) {
        free(data);
        return NULL;
    };
    for (int i = 0; i < n; i++) {
        data->irs[i].osm_id = rs[i].osm_id;
        data->irs[i].record = &rs[i];
    }
    data->n = n;
    qsort(data->irs, n, sizeof(struct indexed_record), comparrison);
    return data;

}

void free_binary(struct indexed_data* data) {
    free(data->irs);
    free(data);
}

const struct record* lookup_binary(struct indexed_data *data, int64_t needle) {
while (1) {
    int low = 0;
    int high = data->n - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (data->irs[mid].osm_id == needle) {
            return &data->irs[mid].record[0];
        } else if (data->irs[mid].osm_id < needle) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return NULL;
}
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_binary,
                    (free_index_fn)free_binary,
                    (lookup_fn)lookup_binary);
}
