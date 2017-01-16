/*
 * Table for bitverse shuffle.
 *
 * Input: n = table size.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

/*
 * Check that n is a power of two and return k such that n=2^k.
 */
static bool logtwo(uint32_t n, uint32_t *k) {
  uint32_t i;

  i = 0;
  while ((n & 1) == 0) {
    i ++;
    n >>= 1;
  }
  if (n == 1) {
    *k = i;
    return true;
  }
  return false;
}

/*
 * Bitreverse of i, interpreted as a k-bit integer
 */
static uint32_t reverse(uint32_t i, uint32_t k) {
  uint32_t x, b, j;

  x = 0;
  for (j=0; j<k; j++) {
    b = i & 1;
    x = (x<<1) | b;
    i >>= 1;
  }

  return x;
}

/*
 * Count the number of i such that i < bitrev(i, k), where n=2^k.
 */
static uint32_t rev_table_npairs(uint32_t n, uint32_t k) {
  uint32_t i, c;

  c = 0;
  for (i=0; i<n; i++) {
    c += (i < reverse(i, k));
  }
  return c;
}


/*
 * Print the table of pairs (i, reverse(i)) where i < reverse(i)
 * - n = 2^k
 */
static void print_bitrev_table(FILE *f, uint32_t n, uint32_t k) {
  uint32_t i, j, z;

  fprintf(f, "#include \"bitrev%"PRIu32"_table.h\"\n\n", n);
  fprintf(f, "const uint16_t bitrev%"PRIu32"[BITREV%"PRIu32"_NPAIRS][2] = {\n", n, n);
  z = 0;
  for (i=0; i<n; i++) {
    j = reverse(i, k);
    if (i < j) {
      if (z == 0) fprintf(f, "   ");
      fprintf(f, " { %5"PRIu32", %5"PRIu32" },", i, j);
      z ++;
      if (z == 4) {
	fprintf(f, "\n");
	z = 0;
      }
    }
  }
  if (z > 0) fprintf(f, "\n");
  fprintf(f, "};\n\n");
}


/*
 * Declarations in file f
 */
static void print_bitrev_declarations(FILE *f, uint32_t n, uint32_t k) {
  uint32_t m;

  m = rev_table_npairs(n, k);

  fprintf(f, "#ifndef __BITREV%"PRIu32"_TABLE_H\n", n);
  fprintf(f, "#define __BITREV%"PRIu32"_TABLE_H\n\n", n);
  fprintf(f, "#include <stdint.h>\n\n");
  fprintf(f, "#define BITREV%"PRIu32"_NPAIRS %"PRIu32"\n\n", n, m);
  fprintf(f, "extern const uint16_t bitrev%"PRIu32"[BITREV%"PRIu32"_NPAIRS][2];\n\n", n, n);

  fprintf(f, "#endif /* __BITREV%"PRIu32"_TABLE_H */\n", n);
}

/*
 * Open file: name is "bitrev<size>_table.h" or "bitrev<size>_table.c"
 * - return NULL if we can't create the file
 */
#define BUFFER_SIZE 100

static FILE *open_file(uint32_t n, const char *suffix) {
  char filename[BUFFER_SIZE];
  int len;
  FILE *f;

  f = NULL;
  len = snprintf(filename, BUFFER_SIZE, "bitrev%"PRIu32"_table.%s", n, suffix);
  if (len < BUFFER_SIZE) {
    f = fopen(filename, "w");
  }
  return f;
}

int main(int argc, char *argv[]) {
  uint32_t n, log_n;
  long x;
  FILE *f;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <size>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // size
  x = atol(argv[1]);
  if (x <= 1) {
    fprintf(stderr, "Invalid size %ld: must be at least 2\n", x);
    exit(EXIT_FAILURE);
  }
  if (x >= UINT16_MAX) {
    fprintf(stderr, "The size is too large: max = %"PRIu32"\n", (uint32_t)UINT16_MAX);
    exit(EXIT_FAILURE);
  }
  n = (uint32_t) x;
  if (!logtwo(n, &log_n)) {
    fprintf(stderr, "Invalid size: %"PRIu32" is not a power of two\n", n);
    exit(EXIT_FAILURE);
  }

  f = open_file(n, "h");
  if (f == NULL) {
    fprintf(stderr, "failed to open file 'bitrev%"PRIu32"_tables.h'\n", n);
    exit(EXIT_FAILURE);
  }
  print_bitrev_declarations(f, n, log_n);
  fclose(f);

  f = open_file(n, "c");
  if (f == NULL) {
    fprintf(stderr, "failed to open file 'bitrev%"PRIu32"_tables.h'\n", n);
    exit(EXIT_FAILURE);
  }
  print_bitrev_table(f, n, log_n);
  fclose(f);
  
  return 0;
}
