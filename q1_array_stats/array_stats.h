#ifndef ARRAY_STATS_H
#define ARRAY_STATS_H
#include <stddef.h>
typedef struct {
    double  average;
    int     max;
    int     min;
    int    *evens;
    size_t  evens_count;
    int     valid;
} ArrayStats;

ArrayStats array_stats_compute(const int *array, size_t n);

void array_stats_free(ArrayStats *stats);

#endif /* ARRAY_STATS_H */
