# Requirements #

- Install [cmake](https://cmake.org/) >= 3.13 
- Install [llvm 11](https://releases.llvm.org/download.html)
- Install [gllvm](https://github.com/SRI-CSL/gllvm)
- Install [gmp](https://gmplib.org/)
 

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


