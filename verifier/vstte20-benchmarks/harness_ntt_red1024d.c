#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "ntt.h"
#include "bitrev1024_table.h"
#include "ntt_red1024.h"
#include "sort.h"

#include "clam.h"

#define Q 12289

static int32_t nd_a[1024];
static int16_t nd_p[1024];

int main(void) {
  ASSUME_FORALL(nd_a, 1024, 0, Q)
  ASSUME_FORALL(nd_p, 1024, -6144, 6144)  
  
  // defined in ntt_red.c
  ntt_red_ct_rev2std(nd_a, 1024, nd_p);

  return 0;
}
