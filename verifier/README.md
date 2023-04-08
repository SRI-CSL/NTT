# Requirements #

- Install [cmake](https://cmake.org/) >= 3.13 
- Install [llvm 11](https://releases.llvm.org/download.html)
- Install [gllvm](https://github.com/SRI-CSL/gllvm) >= 1.3.0
- Install [gmp](https://gmplib.org/)
- Install [boost](https://www.boost.org/) >= 1.65

# Compilation and Installation #

``` bash
./install.sh
```

# Run VSTTE 2020 programs #

``` bash
cd vstte20-benchmarks
./verify_all

```

## Expected results without inlining ## 

The verification times are on MacOS with 2.6 GHz 6-Core Intel Core i7
and 32 GB 2667 MHz DDR4. This is a similar machine to the one used in the paper but the verification times are much
faster than the ones reported in the paper due to improvements in the inter-procedural analysis.

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
These are the verification times on Apple M1 8 cores 16 GB:

```
=== Verifying harness_intt_red1024.all_linked.bc ===
0m19.970s
=== Verifying harness_intt_red1024b.all_linked.bc ===
0m19.892s
=== Verifying harness_ntt_red1024.all_linked.bc ===
0m20.419s
=== Verifying harness_ntt_red1024b.all_linked.bc ===
0m20.000s
=== Verifying harness_ntt_red1024c.all_linked.bc ===
0m35.007s
=== Verifying harness_ntt_red1024d.all_linked.bc ===
0m35.255s
=== Verifying harness_ntt_red1024e.all_linked.bc ===
0m37.501s
=== Verifying harness_ntt_red1024f.all_linked.bc ===
0m37.459s
```

## Expected results with inlining ## 

The verification times are on MacOS with 2.6 GHz 6-Core Intel Core i7
and 32 GB 2667 MHz DDR4.

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
These are the verification times on Apple M1 8 cores 16 GB:

```
=== Verifying harness_intt_red1024.all_linked.bc with inlining ===
0m6.293s
=== Verifying harness_intt_red1024b.all_linked.bc with inlining ===
0m6.255s
=== Verifying harness_ntt_red1024.all_linked.bc with inlining ===
0m6.307s
=== Verifying harness_ntt_red1024b.all_linked.bc with inlining ===
0m6.399s
=== Verifying harness_ntt_red1024c.all_linked.bc with inlining ===
0m10.891s
=== Verifying harness_ntt_red1024d.all_linked.bc with inlining ===
0m11.386s
=== Verifying harness_ntt_red1024e.all_linked.bc with inlining ===
0m7.787s
=== Verifying harness_ntt_red1024f.all_linked.bc with inlining ===
0m7.512s
```
