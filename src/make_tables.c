/*
 * Build tables for ntts.h (using Q=12289)
 *
 * Input: n and psi such that
 * - psi^n = -1 modulo Q
 * - n is a power of two
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

typedef struct parameters_s {
  uint32_t q;        // modulus
  uint32_t n;        // size
  uint32_t inv_n;    // inverse of n
  uint32_t log_n;    // log base 2
  uint32_t psi;      // psi^n = -1
  uint32_t phi;      // psi^2: primitive n-th root of 1
  uint32_t inv_psi;  // inverse of psi
  uint32_t inv_phi;  // inverse of phi
} parameters_t;

/*
 * x^k modulo q
 */
static uint32_t power(uint32_t x, uint32_t k, uint32_t q) {
  uint32_t y;

  assert(q > 0);

  y = 1;
  while (k != 0) {
    if ((k & 1) != 0) {
      y = (y * x) % q;
    }
    k >>= 1;
    x = (x * x) % q;
  }
  return y;
}

/*
 * Check whether x is invertible modulo q and return the inverse in *inv_x
 */
static bool inverse(uint32_t x, uint32_t q, uint32_t *inv_x) {
  int32_t r1, r2, u1, u2, v1, v2, g, aux;

  // invariant: r1 = n * u1 + q * v1
  //            r2 = n * u2 + q * v2
  r1 = x; u1 = 1; v1 = 0;
  r2 = q; u2 = 0, v2 = 1;
  while (r2 > 0) {
    assert(r1 == (int32_t) x * u1 + (int32_t) q * v1);
    assert(r2 == (int32_t) x * u2 + (int32_t) q * v2);
    assert(r1 >= 0);
    g = r1/r2;

    aux = r1; r1 = r2; r2 = aux - g * r2;
    aux = u1; u1 = u2; u2 = aux - g * u2;
    aux = v1; v1 = v2; v2 = aux - g * v2;
  }

  // r1 is gcd(x, q) = x * u1 + q * v1
  if (r1 == 1) {
    u1 = u1 % (int32_t) q;
    if (u1 < 0) u1 += q;
    assert(((((int32_t) x) * u1) % (int32_t) q) == 1);
    *inv_x = u1;
    return true;
  } else {
    return false;
  }
}


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
 * Check that x is a primitive n-th root of unity
 * Brute force check. For debugging.
 */
static bool is_primitive_root(uint32_t x, uint32_t n, uint32_t q) {
  uint32_t i;

  for (i=1; i<n; i++) {
    if (power(x, i, q) == 1) {
      return false;
    }
  }

  return power(x, n, q) == 1;
}  


/*
 * Store a[i] = (x * y^i) mod q for i=0 to n-1
 */
static void build_power_table(uint32_t *a, uint32_t n, uint32_t q, uint32_t x, uint32_t y) {
  uint32_t i;

  for (i=0; i<n; i++) {
    a[i] = x;
    x = (x * y) % q;
  }
}

/*
 * Store a[t + j] = x^(n/2t) * y^(n/2t)^j
 * for t=1, 2, ..., n/2 and j=0, ..., t-1
 *
 * a[0] is unused. It's set to 0.
 */
static void build_table(uint32_t *a, uint32_t n, uint32_t q, uint32_t x, uint32_t y) {
  uint32_t t, j, i;
  uint32_t b, c;

  a[0] = 0;
  i = 1;
  for (t=1; t<n; t <<= 1) {
    b = power(x, n/(2*t), q);
    c = power(y, n/(2*t), q);
    for (j=0; j<t; j++) {
      assert(i == t+j && i < n);
      a[i] = b;
      i ++;
      b = (b * c) % q;
    }
  }
}

/*
 * Store  a[t + j] = x^(n/2t) * y^(n/2t)^ bitrev(j)
 * for t=1, 2, ..., n/2 and j=0, ..., t-1
 *
 * a[0] is unused. It's set to 0.
 */
static void build_rev_table(uint32_t *a, uint32_t n, uint32_t q, uint32_t x, uint32_t y) {
  uint32_t t, j, i, k;
  uint32_t b, c;

  a[0] = 0;
  for (t=1, k=0; t<n; t <<= 1, k++) {
    // t is 2^k
    b = power(x, n/(2*t), q);
    c = power(y, n/(2*t), q);
    for (j=0; j<t; j++) {
      i = t + reverse(j, k);
      assert(t <= i && i < 2*t);
      a[i] = b;
      b = (b * c) % q;
    }
  }
}

/*
 * Print table a:
 * - name = string to use for the array + we add the prefix ntt<n>
 */
static void print_table(FILE *f, const char* name, uint32_t *a, uint32_t n) {
  uint32_t i, k;

  k = 0;
  fprintf(f, "const uint16_t ntt%"PRIu32"_%s[%"PRIu32"] = {\n", n, name, n);
  for (i=0; i<n; i++) {
    if (k == 0) fprintf(f, "   ");
    fprintf(f, " %5"PRIu32",", a[i]);
    k ++;
    if (k == 8) {
      fprintf(f, "\n");
      k = 0;
    }
  }
  if (k > 0) fprintf(f, "\n");
  fprintf(f, "};\n\n");
}

/*
 * Header
 */
static void print_header(FILE *f, parameters_t *p) {
  fprintf(f,
	  "/*\n"
	  " * Parameters:\n"
	  " * - q = %"PRIu32"\n"
	  " * - n = %"PRIu32"\n"
	  " * - psi = %"PRIu32"\n"
	  " * - omega = psi^2 = %"PRIu32"\n"
	  " * - inverse of psi = %"PRIu32"\n"
	  " * - inverse of omega = %"PRIu32"\n"
	  " * - inverse of n = %"PRIu32"\n"
	  " */\n\n", 
	  p->q, p->n, p->psi, p->phi,
	  p->inv_psi, p->inv_phi, p->inv_n);
}

/*
 * Print declarastions in file f
 */
static void print_comment(FILE *f, const char *what) {
  fprintf(f, "/*\n * %s\n */\n", what);
}

static void print_param_def(FILE *f, const char *name, uint32_t n, uint32_t val) {
  fprintf(f, "static const int32_t ntt%"PRIu32"_%s = %"PRIu32";\n", n, name, val);
}

static void print_table_decl(FILE *f, const char *name, uint32_t n) {
  fprintf(f, "extern const uint16_t ntt%"PRIu32"_%s[%"PRIu32"];\n", n, name, n);
}

static void print_declarations(FILE *f, parameters_t *p) {
  uint32_t n;

  print_header(f, p);
  n = p->n;

  fprintf(f, "#ifndef __NTT%"PRIu32"_TABLES_H\n", n);
  fprintf(f, "#define __NTT%"PRIu32"_TABLES_H\n\n", n);
  fprintf(f, "#include <stdint.h>\n\n");

  print_comment(f, "PARAMETERS");
  print_param_def(f, "psi", n, p->psi);
  print_param_def(f, "omega", n, p->phi);
  print_param_def(f, "inv_psi", n, p->inv_psi);
  print_param_def(f, "inv_omega", n, p->inv_phi);
  print_param_def(f, "inv_n", n, p->inv_n);
  fprintf(f, "\n");

  print_comment(f, "POWERS OF PSI");
  print_table_decl(f, "psi_powers", n);
  print_table_decl(f, "inv_psi_powers", n);
  print_table_decl(f, "scaled_inv_psi_powers", n);
  fprintf(f, "\n");

  print_comment(f, "TABLES FOR NTT COMPUTATION");
  print_table_decl(f, "omega_powers", n);
  print_table_decl(f, "omega_powers_rev", n);
  print_table_decl(f, "inv_omega_powers", n);
  print_table_decl(f, "inv_omega_powers_rev", n);
  print_table_decl(f, "mixed_powers", n);
  print_table_decl(f, "mixed_powers_rev", n);
  print_table_decl(f, "inv_mixed_powers", n);
  print_table_decl(f, "inv_mixed_powers_rev", n);
  fprintf(f, "\n");

  fprintf(f, "#endif /* __NTT%"PRIu32"_TABLES_H */\n", n);
}

/*
 * Print table definitions in file f
 */
static void print_tables(FILE *f, parameters_t *p) {
  uint32_t *table;
  uint32_t n, q;

  n = p->n;
  q = p->q;

  // allocate the table
  table = (uint32_t *) malloc(n * sizeof(uint32_t));
  if (table == NULL) {
    fprintf(stderr, "failed to allocate table of size %"PRIu32"\n", n);
    exit(EXIT_FAILURE);
  }

  print_header(f, p);

  fprintf(f, "#include \"ntt%"PRIu32"_tables.h\"\n\n", n);

  // powers of psi
  build_power_table(table, n, q, 1, p->psi);
  print_table(f, "psi_powers", table, n);
  build_power_table(table, n, q, 1, p->inv_psi);
  print_table(f, "inv_psi_powers", table, n);
  build_rev_table(table, n, q, 1, p->inv_psi);
  print_table(f, "inv_psi_powers_rev", table, n);
  build_power_table(table, n, q, p->inv_n, p->inv_psi);
  print_table(f, "scaled_inv_psi_powers", table, n);

  // NTT tables
  build_table(table, n, q, 1, p->phi);
  print_table(f, "omega_powers", table, n);
  build_rev_table(table, n, q, 1, p->phi);
  print_table(f, "omega_powers_rev", table, n);
  build_table(table, n, q, 1, p->inv_phi);
  print_table(f, "inv_omega_powers", table, n);
  build_rev_table(table, n, q, 1, p->inv_phi);
  print_table(f, "inv_omega_powers_rev", table, n);

  build_table(table, n, q, p->psi, p->phi);
  print_table(f, "mixed_powers", table, n);
  build_rev_table(table, n, q, p->psi, p->phi);
  print_table(f, "mixed_powers_rev", table, n);
  build_table(table, n, q, p->inv_psi, p->inv_phi);
  print_table(f, "inv_mixed_powers", table, n);
  build_rev_table(table, n, q, p->inv_psi, p->inv_phi);
  print_table(f, "inv_mixed_powers_rev", table, n);

  free(table);
}

/*
 * Open file: name is "ntt<size>_tables.h" or "ntt<size>_tables.c"
 * - return NULL if we can't create the file
 */
#define BUFFER_SIZE 100

static FILE *open_file(uint32_t n, const char *suffix) {
  char filename[BUFFER_SIZE];
  int len;
  FILE *f;

  f = NULL;
  len = snprintf(filename, BUFFER_SIZE, "ntt%"PRIu32"_tables.%s", n, suffix);
  if (len < BUFFER_SIZE) {
    f = fopen(filename, "w");
  }
  return f;
}

int main(int argc, char *argv[]) {
  uint32_t q, psi, phi, n, log_n, i, inv_n, inv_psi, inv_phi;
  long x;
  parameters_t params;
  FILE *f;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <size> <psi>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  q = 12289;

  // size
  x = atol(argv[1]);
  if (x <= 1) {
    fprintf(stderr, "Invalid size %ld: must be at least 2\n", x);
    exit(EXIT_FAILURE);
  }
  if (x >= 100000) {
    fprintf(stderr, "The size is too large: max = %"PRIu32"\n", (uint32_t)100000);
    exit(EXIT_FAILURE);
  }
  n = (uint32_t) x;
  if (!logtwo(n, &log_n)) {
    fprintf(stderr, "Invalid size: %"PRIu32" is not a power of two\n", n);
    exit(EXIT_FAILURE);
  }

  // psi
  x = atol(argv[2]);
  if (x <= 1 || x >= q) {
    fprintf(stderr, "psi must be between 2 and %"PRIu32"\n", q-1);
    exit(EXIT_FAILURE);
  }
  psi = (uint32_t) x;

  i = power(psi, n, q);
  if (power(psi, n, q) != q-1) {
    fprintf(stderr, "invalid psi: %"PRIu32" is not an n-th root of -1  (%"PRIu32"^n = %"PRIu32")\n", psi, psi, i);
    exit(EXIT_FAILURE);
  }

  phi = (psi * psi) % q;
  assert(is_primitive_root(phi, n, q));
  if (!inverse(psi, q, &inv_psi)) {
    fprintf(stderr, "BUG: failed to compute the inverse of %"PRIu32" modulo %"PRIu32"\n", psi, q);
    exit(EXIT_FAILURE);
  }
  if (!inverse(phi, q, &inv_phi)) {
    fprintf(stderr, "BUG: failed to compute the inverse of %"PRIu32" modulo %"PRIu32"\n", phi, q);
    exit(EXIT_FAILURE);
  }
  if (!inverse(n, q, &inv_n)) {
    fprintf(stderr, "BUG: failed to compute the inverse of %"PRIu32" modulo %"PRIu32"\n", n, q);
    exit(EXIT_FAILURE);
  }

  params.q = q;
  params.n = n;
  params.inv_n = inv_n;
  params.log_n = log_n;
  params.psi = psi;
  params.phi = phi;
  params.inv_psi = inv_psi;
  params.inv_phi = inv_phi;

  f = open_file(n, "h");
  if (f == NULL) {
    fprintf(stderr, "failed to open file 'ntt%"PRIu32"_tables.h'\n", n);
    exit(EXIT_FAILURE);
  }
  print_declarations(f, &params);
  fclose(f);

  f = open_file(n, "c");
  if (f == NULL) {
    fprintf(stderr, "failed to open file 'ntt%"PRIu32"_tables.h'\n", n);
    exit(EXIT_FAILURE);
  }
  print_tables(f, &params);
  fclose(f);
  
  return 0;
}
