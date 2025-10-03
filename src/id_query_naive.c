#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include "record.h"
#include "id_query.h"

// struktur til records og antal
struct naive_data {
  struct record *rs;
  int n;
};

// Opretter wrapper
struct naive_data* mk_naive(struct record* rs, int n) {
  struct naive_data* data = malloc(sizeof *data);
  if (!data) {
    return NULL; // fejl hvis der ikke er hukommelse
  };
  data->rs = rs; // gem records
  data->n = n;   // gem antal
  return data;
}

// Frigør plads i hukommelsen
void free_naive(struct naive_data* data) {
  free(data);
}

// Finder record med osm_id
const struct record* lookup_naive(struct naive_data *data, int64_t needle) {
  for (int i = 0; i < data->n; i++) {
    if (data->rs[i].osm_id == needle) {
      return &data->rs[i];
    }
  }
  return NULL;
}

// Starter query-loop
int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_naive,
                    (free_index_fn)free_naive,
                    (lookup_fn)lookup_naive);
}
