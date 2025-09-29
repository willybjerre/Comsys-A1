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

struct indexed_record {
  struct record *rs;
  double lon;
  double lat;
};
struct indexed_data {
  struct indexed_record *irs;
  int n;
};

struct indexed_data* mk_naive(struct record* rs, int n) {

  struct indexed_data* data = malloc(sizeof *data);
  if (!data) {
    return NULL;
  };
  data->irs = malloc(n * sizeof(struct indexed_record));
  if (!data->irs) {
    free(data);
    return NULL;
  };
  for (int i = 0; i < n; i++) {
    data->irs[i].rs = &rs[i];
    data->irs[i].lon = rs[i].lon;
    data->irs[i].lat = rs[i].lat;
  }
  data->n = n;
  return data;
}

void free_naive(struct indexed_data* data) {
  free(data->irs);
  free(data);
}

const struct record* lookup_naive(struct indexed_data *data, double lon, double lat) {
  double comparrison_element = ((data->irs[0].lon - lon)*(data->irs[0].lon - lon)) + ((data->irs[0].lat - lat)*(data->irs[0].lat - lat));
  double temp;
  struct record *result = data->irs[0].rs;
  for (int i = 1; i < data->n; i++) {
    temp = sqrt(((data->irs[i].lon - lon)*(data->irs[i].lon - lon)) + ((data->irs[i].lat - lat)*(data->irs[i].lat - lat)));
    
    
    if (temp < comparrison_element) {
      comparrison_element = temp;
      result = data->irs[i].rs;
    }
  }
  return result;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}
