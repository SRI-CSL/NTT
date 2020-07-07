#include <stdio.h>
#include <inttypes.h>

static void print_shift(int32_t x) {
  printf("shift_31(%"PRId32") = %"PRId32"\n", x, (x >> 31));
  printf("shift_11(%"PRId32") = %"PRId32"\n", x, (x >> 11));
}

static void print_shift_and(int32_t x, int32_t q) {
  printf("shift_31(%"PRId32" & %"PRId32") = %"PRId32"\n", x, q, (x >> 31) & q);
  printf("shift_11(%"PRId32" & %"PRId32") = %"PRId32"\n", x, q, (x >> 11) & q);
}

int main(void) {
  int32_t i;

  for (i=0; i<1003; i++) {
    print_shift(i);
    print_shift(-i);
    print_shift_and(i, 12289);
    print_shift_and(-i, 12289);
  }

  return 0;
}
