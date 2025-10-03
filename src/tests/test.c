#include "test.h"
#include "compiler/token.h"
#include "containers/strimap.h"
#include "containers/vector.h"
#include <stddef.h>
#include <stdio.h>
void test_vector(void);
void test_strimap(void);

void test_run(void) { test_strimap(); }

void test_vector(void) {
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

void test_strimap(void) {
    strimap_t str_to_tkn_map = *get_string_to_token_strimap();
    const char* const* tkn_to_str_map = get_token_to_string_map();

    for (strimap_iter_t iter = strimap_iter_begin(&str_to_tkn_map); iter.curr != NULL;
         strimap_iter_next(&iter)) {
        printf("%-8s -> %-8d -> %-8s @ %-8zu\n", iter.curr->key, iter.curr->val,
               tkn_to_str_map[iter.curr->val], iter.bucket_idx);
    }
}
