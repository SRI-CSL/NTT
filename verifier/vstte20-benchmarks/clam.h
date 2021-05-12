#pragma once

#ifdef __cplusplus
extern "C" {
#endif        

extern void __CRAB_assert(int);
extern void __CRAB_assume(int);
extern int int_nd(void);

#ifdef __cplusplus
}
#endif


#define clam_assume __CRAB_assume
#define clam_assert(X) __CRAB_assert(X)

// Enable this for verify2
#define UNROLL_ASSUME_FORALL 


#define STRINGIFY_(A) #A
#define STRINGIFY(A) STRINGIFY_(A)

// forall i :: ARRAY[i] \in [LB_VAL, UB_VAL)
#define ASSUME_FORALL_WITH_LOOP(ARRAY, ARRAY_SIZE, LB_VAL, UB_VAL)  \
  {							  \
int i;							  \
_Pragma("nounroll")                                       \
for(i=0; i<ARRAY_SIZE; i++) {				  \
int x = int_nd();	                                  \
clam_assume(x >= LB_VAL);                                 \
clam_assume(x < UB_VAL);                                  \
ARRAY[i] = x;                                             \
}						          \
}


// forall i :: ARRAY[i] \in [LB_VAL, UB_VAL)
#define ASSUME_FORALL_WITHOUT_LOOP(ARRAY, ARRAY_SIZE, LB_VAL, UB_VAL)  \
{							  \
int i;							  \
_Pragma("unroll(1024)")                                   \
for(i=0; i<ARRAY_SIZE; i++) {				  \
int x = int_nd();	                                  \
clam_assume(x >= LB_VAL);                                 \
clam_assume(x < UB_VAL);                                  \
ARRAY[i] = x;                                             \
}						          \
}


#ifndef UNROLL_ASSUME_FORALL
#define ASSUME_FORALL(ARRAY, ARRAY_SIZE, LB_VAL, UB_VAL)	\
  ASSUME_FORALL_WITH_LOOP(ARRAY, ARRAY_SIZE, LB_VAL, UB_VAL)
#else
#define ASSUME_FORALL(ARRAY, ARRAY_SIZE, LB_VAL, UB_VAL) \
  ASSUME_FORALL_WITHOUT_LOOP(ARRAY, ARRAY_SIZE, LB_VAL, UB_VAL)
#endif 
