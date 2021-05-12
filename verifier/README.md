# Requirements #

- Install [cmake](https://cmake.org/) >= 3.13 
- Install [llvm 11](https://releases.llvm.org/download.html)
- Install [gllvm](https://github.com/SRI-CSL/gllvm)
- Install [gmp](https://gmplib.org/)
- Install [boost](???)

# Compilation and Installation #

```
mkdir build && cd build
cmake ..
cmake --build . --target clam-seadsa && cmake ..
cmake --build . --target clam-seallvm && cmake ..
cmake --build . --target ntt-clam && cmake ..
cmake --build . --target crab && cmake ..
cmake --build . --target install
```

# Run VSTTE 2020 programs #

```
cd vstte20-benchmarks
./verify_all

```

## Expected results without inlining ## 

The verification times are on MacOS with 2.6 GHz 6-Core Intel Core i7
and 32 GB 2667 MHz DDR4. Note that the verification times are much
faster than the ones reported in the paper.

```
=== Verifying harness_intt_red1024.all_linked.bc ===
===Assertion checks ===
2026  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

0m41.768s

=== Verifying harness_intt_red1024b.all_linked.bc ===
===Assertion checks ===
2026  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

0m41.229s

=== Verifying harness_ntt_red1024.all_linked.bc ===
===Assertion checks ===
2026  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

0m48.723s

=== Verifying harness_ntt_red1024b.all_linked.bc ===
===Assertion checks ===
2026  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

0m42.847s

=== Verifying harness_ntt_red1024c.all_linked.bc ===
===Assertion checks ===
1974  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

1m20.550s

=== Verifying harness_ntt_red1024d.all_linked.bc ===
===Assertion checks ===
1974  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

1m13.803s

=== Verifying harness_ntt_red1024e.all_linked.bc ===
===Assertion checks ===
8194  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

1m20.922s

=== Verifying harness_ntt_red1024f.all_linked.bc ===
===Assertion checks ===
8194  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

1m23.990s
```

## Expected results with inlining ## 

```
=== Verifying harness_intt_red1024.all_linked.bc with inlining ===
===Assertion checks ===
8188  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

0m12.112s

=== Verifying harness_intt_red1024b.all_linked.bc with inlining ===
===Assertion checks ===
8188  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

0m12.002s

=== Verifying harness_ntt_red1024.all_linked.bc with inlining ===
===Assertion checks ===
8188  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

0m12.330s

=== Verifying harness_ntt_red1024b.all_linked.bc with inlining ===
===Assertion checks ===
8188  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

0m12.101s

=== Verifying harness_ntt_red1024c.all_linked.bc with inlining ===
===Assertion checks ===
8194  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

0m23.402s

=== Verifying harness_ntt_red1024d.all_linked.bc with inlining ===
===Assertion checks ===
8194  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

0m21.240s

=== Verifying harness_ntt_red1024e.all_linked.bc with inlining ===
===Assertion checks ===
8188  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

0m13.578s

=== Verifying harness_ntt_red1024f.all_linked.bc with inlining ===
===Assertion checks ===
8188  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

0m13.059s

```

Apple M1 8 cores 16 GB
```
=== Verifying harness_intt_red1024.all_linked.bc ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
2026  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m19.970s
user	0m19.627s
sys	0m0.078s
=== Verifying harness_intt_red1024b.all_linked.bc ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
2026  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m19.892s
user	0m19.801s
sys	0m0.087s
=== Verifying harness_ntt_red1024.all_linked.bc ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
2026  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m20.419s
user	0m20.337s
sys	0m0.078s
=== Verifying harness_ntt_red1024b.all_linked.bc ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
2026  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m20.000s
user	0m19.930s
sys	0m0.067s
=== Verifying harness_ntt_red1024c.all_linked.bc ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
1974  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m35.007s
user	0m34.788s
sys	0m0.213s
=== Verifying harness_ntt_red1024d.all_linked.bc ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
1974  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m35.255s
user	0m35.078s
sys	0m0.172s
=== Verifying harness_ntt_red1024e.all_linked.bc ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
8194  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m37.501s
user	0m37.391s
sys	0m0.106s
=== Verifying harness_ntt_red1024f.all_linked.bc ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
8194  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m37.459s
user	0m37.301s
sys	0m0.154s
=== Verifying harness_intt_red1024.all_linked.bc with inlining ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
8188  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m6.293s
user	0m6.254s
sys	0m0.035s
=== Verifying harness_intt_red1024b.all_linked.bc with inlining ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
8188  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m6.255s
user	0m6.219s
sys	0m0.034s
=== Verifying harness_ntt_red1024.all_linked.bc with inlining ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
8188  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m6.307s
user	0m6.268s
sys	0m0.037s
=== Verifying harness_ntt_red1024b.all_linked.bc with inlining ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
8188  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m6.399s
user	0m6.361s
sys	0m0.035s
=== Verifying harness_ntt_red1024c.all_linked.bc with inlining ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
8194  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m10.891s
user	0m10.817s
sys	0m0.066s
=== Verifying harness_ntt_red1024d.all_linked.bc with inlining ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
8194  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m11.386s
user	0m11.300s
sys	0m0.059s
=== Verifying harness_ntt_red1024e.all_linked.bc with inlining ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
8188  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m7.787s
user	0m7.627s
sys	0m0.056s
=== Verifying harness_ntt_red1024f.all_linked.bc with inlining ===
Sea-Dsa type aware!
=== Devirtualization stats===
BRUNCH_STAT INDIRECT CALLS 0
BRUNCH_STAT RESOLVED CALLS 0
BRUNCH_STAT UNRESOLVED CALLS 0
===Assertion checks ===
8188  Number of total safe checks
   0  Number of total error checks
   0  Number of total warning checks
   0  Number of total unreachable checks

real	0m7.512s
user	0m7.362s
sys	0m0.058s

```
