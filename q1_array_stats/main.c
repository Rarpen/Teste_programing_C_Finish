#include "array_stats.h"
#include <stdio.h>

static void run(const int *a, size_t n, const char *label)
{
    printf("--- %s (n=%zu) ---\n", label, n);
    ArrayStats s = array_stats_compute(a, n);
    if (!s.valid) {
        printf("entrada invalida\n\n");
        return;
    }
    printf("average = %.4f\n", s.average);
    printf("max     = %d\n", s.max);
    printf("min     = %d\n", s.min);
    printf("evens   = [");
    for (size_t i = 0; i < s.evens_count; ++i) {
        printf("%d%s", s.evens[i], (i + 1 < s.evens_count) ? ", " : "");
    }
    printf("] (count=%zu)\n\n", s.evens_count);
    array_stats_free(&s);
}

int main(void)
{
    int a1[] = {5, 2, 9, 4, 7, 8, 1, 6, 3, 10};
    int a2[] = {-4, -3, -2, -1, 0, 1};
    int a3[] = {7, 7, 7};         
    int a4[] = {42};              

    run(a1, sizeof(a1)/sizeof(a1[0]), "misto");
    run(a2, sizeof(a2)/sizeof(a2[0]), "com negativos e zero");
    run(a3, sizeof(a3)/sizeof(a3[0]), "sem pares");
    run(a4, sizeof(a4)/sizeof(a4[0]), "elemento unico");
    run(NULL, 0, "invalido");

    return 0;
}
