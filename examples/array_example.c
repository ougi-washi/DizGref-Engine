// Syphax-Engine - Ougi Washi

#include "se_array.h"
#include <stdio.h>

#define ARRAY_SIZE 8

SE_DEFINE_ARRAY(i32, ints, ARRAY_SIZE);

i32 main() {
    
    ints my_ints = {0};

    for (sz i = 0; i < ARRAY_SIZE; i++) {
        ints_add(&my_ints, i);
    }
    
    printf("Current size: %zu\n", ints_get_size(&my_ints));
   
    se_foreach(ints, my_ints, i) {
        printf("%d, ", *ints_get(&my_ints, i));
    }
    printf("\n");

    ints_remove(&my_ints, 5);

    printf("Removed 5th element, current size: %zu\n", ints_get_size(&my_ints));
    printf("Current 5th element: %d\n", *ints_get(&my_ints, 5));

    se_foreach(ints, my_ints, i) {
        printf("%d, ", *ints_get(&my_ints, i));
    }
    printf("\n");

    ints_clear(&my_ints);

    printf("Cleared, current size: %zu\n", ints_get_size(&my_ints));
   
    return 0;
}
