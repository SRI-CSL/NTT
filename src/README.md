# The Sources


## The Implementations

The various algorithms are parametric in Q and n. Q is the prime order of the finite field, and is always assumed to be 12289. 
n is the degree of the polynomials under consideration, and varies over various powers of 2.
If a file has a number in it, that number is the fixed value that n is assumed to take. Thus
```
naive_ntt[16, 256, 512, 1024].[ch]
```
contain implementations of polynomial multiplication using naive versions of the NTT for fixed n = 16, 256, 512, and 1024. More details are contained in the 
respective header files.
```
ntt[16, 256, 512, 1024].[ch]
```
These fixed n versions depend on precomputed tables. Explicitly

* `naive_ntt[16, 256, 512, 1024].c` depends on `ntt[16, 256, 512, 1024]_tables.h`

* `ntt[16, 256, 512, 1024].c` depends on `ntt[16, 256, 512, 1024]_tables.h`


For parametric n 
```
naive_ntt.[ch]
```
contains various implementations of the naive algorithm, including the Cooley-Tukey, and Gentleman-Sande variants.
As does:
```
ntt.[ch]
```

```
ntt_asm.[h,S]
```

```
ntt_red.[ch]
```

```
ntt_red[16, 256, 512, 11024].[ch]
```

* `ntt_red[16, 256, 512, 1024].c` depends on `ntt_red[16, 256, 512, 1024]_tables.h`, `ntt.[ch]`, `ntt_red.[ch]`, 
and the `bitrev[16, 256, 512, 1024]_table.h`





```
ntt_red_asm[16, 256, 512, 1024].[ch]
```

* `ntt_red_asm[16, 256, 512, 1024].c` depends on `ntt_red[16, 256, 512, 1024]_tables.h`, `ntt.[ch]`, `ntt_asm.[hS]`, `ntt_red.[ch]`, 
and the `bitrev[16, 256, 512, 1024]_table.h`



```
speed_mul1024[, _naive, _red, _red_asm].c
```





## The Tables


## The Tests



Known Answer Tests (kat):

```
data_poly1024.[ch]
kat_mul1024[, _red, red_asm].c
```

