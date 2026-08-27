#include "array_stats.h"
#include <stdlib.h>

ArrayStats array_stats_compute(const int *array, size_t n)
{
    ArrayStats s = {0};
    if (array == NULL || n == 0) {
        s.valid = 0;
        return s;
    }

    long long sum = 0;  
    s.max = array[0];
    s.min = array[0];
    size_t even_count = 0;

    for (size_t i = 0; i < n; ++i) {
        int v = array[i];
        sum += v;
        if (v > s.max) s.max = v;
        if (v < s.min) s.min = v;
        if (v % 2 == 0) even_count++;
    }

    s.average = (double)sum / (double)n;

    if (even_count > 0) {
        s.evens = (int *)malloc(even_count * sizeof(int));
        if (s.evens == NULL) {
            s.valid = 0;
            return s;
        }
        size_t j = 0;
        for (size_t i = 0; i < n; ++i) {
            if (array[i] % 2 == 0) {
                s.evens[j++] = array[i];
            }
        }
    }
    s.evens_count = even_count;
    s.valid = 1;
    return s;
}

void array_stats_free(ArrayStats *stats)
{
    if (stats == NULL) return;
    free(stats->evens);
    stats->evens = NULL;
    stats->evens_count = 0;
}
