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

// en wrapper med record og id + pointer til første ellement i record
struct indexed_record {
    int64_t osm_id;
    const struct record *record;
};

// wrapper med array af indexed_record og antal
struct indexed_data{
    struct indexed_record *irs;
    int n;
};

// Bygger index
struct indexed_data* mk_indexed(struct record* rs, int n) {
    struct indexed_data *data = malloc(sizeof *data);
    if (!data) return NULL;
    data->irs = malloc(n * sizeof(struct indexed_record));
    if (!data->irs) { free(data); return NULL; } // fejl hvis der ikke er hukommelse
    for (int i = 0; i < n; i++) {
        data->irs[i].osm_id = rs[i].osm_id;
        data->irs[i].record = &rs[i];
    } // gem records og id'er i indexed_record
    data->n = n;
    return data;
}

// Frigør index
void free_indexed(struct indexed_data* data) {
    free(data->irs);
    free(data);
}

// Finder record via id
const struct record* lookup_indexed(struct indexed_data *data, int64_t needle) {
    for (int i = 0; i < data->n; i++) {
        if (data->irs[i].osm_id == needle) {
            return &data->irs[i].record[0];
        }
    }
    return NULL;
}

// Starter loop
int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_indexed,
                    (free_index_fn)free_indexed,
                    (lookup_fn)lookup_indexed);
}
