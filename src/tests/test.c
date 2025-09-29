#include "test.h"
#include "containers/vector.h"
#include <stddef.h>
void test_run(void) {
    vector_t vec_int = vector_create_and_init(sizeof(int), 5);
    int* ints = vector_get_data(&vec_int);

    for (int i = 0; i < vec_int.size; i++) {
        ints[i] = i + 1;
    }
    int twelve_k = 12000;
    vector_push_back(&vec_int, &twelve_k);
    vector_push_back(&vec_int, &twelve_k);

    for (int i = 0; i < vec_int.size; i++) {
        printf("idx: %d, val: %d\n", i, *(int*)vector_at(&vec_int, i));
    }

    vector_destroy(&vec_int);

    puts("test done");
}
