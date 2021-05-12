# The Number Theoretic Transform

The Number Theoretic Transform ([NTT](https://en.wikipedia.org/wiki/Discrete_Fourier_transform_(general)#Number-theoretic_transform)) is an efficient algorithm for
computing the products of polynomials whose coefficients belong to
a finite field.

This repository contains SRI's various implementations of the NTT (developed while 
implementing the [Bliss](https://github.com/SRI-CSL/Bliss)).

It also includes the verification of these algorithms.

The repository is organized into three subdirectories:

* [src](https://github.com/SRI-CSL/NTT/tree/master/src/README.md) contains a plethora of code implementing the algorithms described in paper.

* [verifier](https://github.com/SRI-CSL/NTT/tree/master/verifier/README.md) contains the code of the verifier that proves absence of integer overflows of the programs described in [src](https://github.com/SRI-CSL/NTT/tree/master/src/README.md).

* [paper](https://github.com/SRI-CSL/NTT/blob/master/paper/main_final.pdf) contains the [VSTTE20 conference](https://sri-csl.github.io/VSTTE20/) version of the paper, as well as the [slides](https://github.com/SRI-CSL/NTT/blob/master/paper/slides.pdf) from the conference talk.
