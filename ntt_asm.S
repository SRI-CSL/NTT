/*
 * BD: variant implementations of NTT for intel
 *
 * All variants are specialized to Q=12289.
 * - omega denotes a primitive n-th root of unity (mod Q).
 * - psi denotes a square root of omega (mod Q).
 *
 * These variants use the reduction method introduced by
 * Longa and Naehrig, 2016. The implementation uses intel's
 * AVX2 vector instructions.
 */

// On MacOS we need to prefix all global symbols with an underscore
#if defined(__APPLE__)
#define _G(s) _##s
#else
#define _G(s) s
#endif

	.intel_syntax noprefix

	.data
	.balign 32
// mask = array of 8 integers, all equal to 4095 = 2^12 -1
mask:
	.long  0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff
	
// q_x8 = array of 8 integers, all equal to 12289
q_x8:
	.long  12289, 12289, 12289, 12289, 12289, 12289, 12289, 12289
	
// q_x8 = array of 8 integers, all equal to 12288
q_minus1_x8:
	.long  12288, 12288, 12288, 12288, 12288, 12288, 12288, 12288

// for testing ntt_x86_asm (from Microsoft)
perm0246:
	.long 0, 2, 4, 6, 0, 0, 0, 0

// for ntt_red_ct_rev2std
perm5:
	.long 4, 0, 5, 0, 6, 0, 7, 0
perm4:
	.long 2, 0, 3, 0, 2, 0, 3, 0

	.text
/*
 * Check whether the processor + OS support AVX and AVX2
 *
 * This follows the intel manual.
 *
 * No input parameters.
 * - return with rax = 1 if AVX2 is supported
 * - return with rax = 0 otherwise
 */
	.balign 16
	.global _G(avx2_supported)
_G(avx2_supported):
	push rbx		// rax/rbx/rcx/rdx are modified by CPUID
	mov eax, 1
	cpuid
	and ecx, 0x18000000
	cmp ecx, 0x18000000
	jne not_supported
	mov eax, 7
	xor ecx, ecx
	cpuid
	and ebx, 0x20
	cmp ebx, 0x20
	jne not_supported
	xor ecx, ecx
	xgetbv
	and eax, 0x06
	cmp eax, 0x06
	jne not_supported
	mov eax, 1           	// all good: supported
	pop rbx
	ret
not_supported:
	xor eax, eax
	pop rbx
	ret


/*
 * Reduce all elements of an array of signed 32bit integers
 *
 * Input:
 * - rdi = start of the array
 * - rsi = number of elements (must be positive and a multiple of 16)
 *
 * The array is updated in place
 */
	.balign 16
	.global _G(reduce_array_asm)
_G(reduce_array_asm):
	vmovdqa ymm3, [mask+rip]
	mov rax, rdi
	lea rsi, [rdi+4*rsi]
	.balign 16
loop0:	
	vmovdqu ymm0, [rax]                        // load 8 elements
	vmovdqu ymm1, [rax+32]                     // load 8 elements

	vpsrad  ymm2, ymm0, 12                     // ymm2[i] = ymm0[i] >> 12 (arithmetic shift)	
	vpand   ymm0, ymm0, ymm3	           // ymm3[i] = ymm0[i] & 4095
	vpslld  ymm4, ymm0, 1                      // ymm4[i] = 2*ymm0[i]
	vpaddd  ymm0, ymm0, ymm4                   // ymm0[i] = 3*ymm0[i]
	vpsubd  ymm0, ymm0, ymm2

	vmovdqu [rax], ymm0                        // store 8 elements

	vpsrad  ymm2, ymm1, 12                     // ymm2[i] = ymm1[i] >> 12	
	vpand   ymm1, ymm1, ymm3	           // ymm3[i] = ymm1[i] & 4095
	vpslld  ymm4, ymm1, 1                      // ymm4[i] = 2*ymm1[i]
	vpaddd  ymm1, ymm1, ymm4                   // ymm1[i] = 3*ymm1[i]
	vpsubd  ymm1, ymm1, ymm2

	vmovdqu [rax+32], ymm1                     // store 8 elements

	add rax, 64
	cmp rax, rsi
	jb loop0
	ret

/*
 * Reduce all elements of an array twice
 *
 * Input:
 * - rdi = start of the array
 * - rsi = number of elements (must be positive and a multiple of 16)
 *
 * The array is updated in place
 */
	.balign 16
	.global _G(reduce_array_twice_asm)
_G(reduce_array_twice_asm):
	vmovdqa ymm3, [mask+rip]
	mov rax, rdi
	lea rsi, [rdi+4*rsi]
	.balign 16
loop1:	
	vmovdqu ymm0, [rax]                        // load 8 elements
	vmovdqu ymm1, [rax+32]                     // load 8 elements

	// first reduction: all 8 elements of ymm0 in parallel
	vpsrad  ymm2, ymm0, 12 
	vpand   ymm0, ymm0, ymm3
	vpslld  ymm4, ymm0, 1
	vpaddd  ymm0, ymm0, ymm4
	vpsubd  ymm0, ymm0, ymm2
	// second reduction
	vpsrad  ymm2, ymm0, 12
	vpand   ymm0, ymm0, ymm3
	vpslld  ymm4, ymm0, 1
	vpaddd  ymm0, ymm0, ymm4
	vpsubd  ymm0, ymm0, ymm2

	vmovdqu [rax], ymm0                        // store 8 elements

	// same thing for vector ymm1
	vpsrad  ymm2, ymm1, 12
	vpand   ymm1, ymm1, ymm3
	vpslld  ymm4, ymm1, 1
	vpaddd  ymm1, ymm1, ymm4
	vpsubd  ymm1, ymm1, ymm2
	// second reduction
	vpsrad  ymm2, ymm1, 12
	vpand   ymm1, ymm1, ymm3
	vpslld  ymm4, ymm1, 1
	vpaddd  ymm1, ymm1, ymm4
	vpsubd  ymm1, ymm1, ymm2
	
	vmovdqu [rax+32], ymm1                     // store 8 elements

	add rax, 64
	cmp rax, rsi
	jb loop1
	ret


/*
 * Correction: convert all elements to an integer between
 * 0 and 12288. This assumes that the input integers satisfy
 * -Q <= a[i] <= 2*Q -1 (where Q=12289).
 *
 * Input:
 * - rdi = start of the array
 * - rsi = number of elements (must be positive and a multiple of 16).
 *
 * The array is updated in place.
 */
	.balign 16
	.global _G(correct_asm)
_G(correct_asm):
	vmovdqa ymm3, [q_x8+rip]
	vmovdqa ymm4, [q_minus1_x8+rip]
	mov rax, rdi
	lea rsi, [rdi+4*rsi]
	.balign 16
loop2:
	vmovdqu ymm0, [rax]                        // load 8 elements
	vmovdqu ymm1, [rax+32]                     // load 8 elements

	vpsrad  ymm2, ymm0, 31                     // ymm2[i] = -1 if ymm0[i] < 0
	vpcmpgtd ymm5, ymm0, ymm4                  // ymm5[i] = -1 if ymm0[i] >= Q
	vpand   ymm2, ymm2, ymm3
	vpand   ymm5, ymm5, ymm3
	vpaddd  ymm0, ymm0, ymm2
	vpsubd  ymm0, ymm0, ymm5
	
	vmovdqu [rax], ymm0                        // store 8 elements
	
	vpsrad  ymm2, ymm1, 31                     // ymm2[i] = all ones if ymm1[i] < 0
	vpcmpgtd ymm5, ymm1, ymm4                  // ymm5[i] = all ones if ymm1[i] >= Q
	vpand   ymm2, ymm2, ymm3
	vpand   ymm5, ymm5, ymm3
	vpaddd  ymm1, ymm1, ymm2
	vpsubd  ymm1, ymm1, ymm5
	
	vmovdqu [rax+32], ymm1                     // store 8 elements

	add rax, 64
	cmp rax, rsi
	jb loop2
	ret

/*
 * Element-wise multiplication + reduction in place:
 *  a[i] = red(a[i] * p[i])
 *
 * Input:
 * - rdi = start of array a (array of signed 32bit integers)
 * - rsi = number of elements in a and p
 * - rdx = start of array p (array of signed 16bit integers)
 *
 * The number of elements must be positive and a multiple of 16.
 *
 * Output: array a is modified in place.
 */
	.balign 16
	.global _G(mul_reduce_array16_asm)
_G(mul_reduce_array16_asm):
	vmovdqa ymm4, [mask+rip]
	mov rax, rdi
	lea rsi, [rdi+4*rsi]
	.balign 16
loop3:
	vmovdqu    ymm0, [rax]                      // ymm0 = 8 elements of array a
	vpmovsxwd  ymm1, [rdx]                      // ymm1 = 8 elements of array p, sign-extended to 32bits

	vpmuldq    ymm2, ymm0, ymm1                 // ymm2 = a[0] * p[0], a[2] * p[2], a[4] * p[4], a[6] * p[6]
	vpshufd    ymm0, ymm0, 0x31
	vpshufd    ymm1, ymm1, 0x31
	vpmuldq    ymm3, ymm0, ymm1                 // ymm3 = a[1] * p[1], a[3] * p[3], a[5] * p[5], a[7] * p[7]

	vpslldq    ymm0, ymm3, 4
	vpblendd   ymm0, ymm0, ymm2, 0x55
	vpand      ymm0, ymm0, ymm4                 // c0 = masked low-order bits of a[0] * p[0]  .... a[7] * p[7]

	vpsrlq     ymm3, ymm3, 12
	vpsrlq     ymm2, ymm2, 12
	vpslldq    ymm3, ymm3, 4
	vpblendd   ymm1, ymm3, ymm2, 0x55           // c1 = a[0] * p[0] >> 12, ...., a[7] * p[7] >> 12

	vpslld     ymm2, ymm0, 1
	vpaddd     ymm0, ymm0, ymm2                 // 3 * c0
	vpsubd     ymm0, ymm0, ymm1                 // 3 * c0 - c1
	
	vmovdqu    [rax], ymm0                      // store the result

	add        rax, 32
	add        rdx, 16
	cmp        rax, rsi
	jb         loop3
	ret
	
/*
 * Another implementation (based on Microsoft's ntt_x64_asm.S)
 */
	.balign 16
	.global _G(mul_reduce_array16_asm2)
_G(mul_reduce_array16_asm2):
	vmovdqu    ymm5, [perm0246+rip]
	vmovdqu    ymm6, [mask+rip]
	mov        rax, rdi
	lea        rsi, [rdi+4*rsi]

	.balign 16
loop4:
	vpmovsxdq  ymm0, [rax]                    // ymm0 = 4 elements of a, sign-extended to 64 bits
	vpmovsxwq  ymm1, [rdx]                    // ymm1 = 4 elements of p, sign-extended to 64 bits
	vpmuldq    ymm0, ymm1, ymm0               // product

	vmovdqu    ymm3, ymm0
	vpand      ymm0, ymm6, ymm0               // c0
	vpsrlq     ymm3, ymm3, 12                 // c1
	vpslld     ymm4, ymm0, 1                  // 2*c0
	vpsubd     ymm3, ymm0, ymm3               // c0-c1
	vpaddd     ymm0, ymm3, ymm4               // 3*c0-c1 

	vpermd     ymm0, ymm5, ymm0 
	vmovdqu    [rax], xmm0

	add        rax, 16
	add        rdx, 8
	cmp        rax, rsi
	jb         loop4

	ret
	
/*
 * Element-wise multiplication + reduction:
 *  a[i] = red(b[i] * c[i])
 *
 * Input:
 * - rdi = start of array a (array of signed 32bit integers)
 * - rsi = size of all three arrays
 * - rdx = start of array b (array of signed 32bit integers)
 * - rcx = start of array c (array of signed 32bit integers)
 *
 * The number of elements must be positive and a multiple of 16.
 */
	.balign 16
	.global _G(mul_reduce_array_asm)
_G(mul_reduce_array_asm):
	vmovdqa ymm4, [mask+rip]
	mov     rax, rdi
	lea     rsi, [rdi+4*rsi]

	.balign 16
loop5:
	vmovdqu    ymm0, [rdx]                      // ymm0 = 8 elements of array b
	vmovdqu    ymm1, [rcx]                      // ymm1 = 8 elements of array c
	
	vpmuldq    ymm2, ymm0, ymm1                 // ymm2 = b[0] * c[0], b[2] * c[2], b[4] * c[4], b[6] * c[6]
	vpshufd    ymm0, ymm0, 0x31
	vpshufd    ymm1, ymm1, 0x31
	vpmuldq    ymm3, ymm0, ymm1                 // ymm3 = b[1] * c[1], b[3] * c[3], b[5] * c[5], b[7] * c[7]

	vpslldq    ymm0, ymm3, 4
	vpblendd   ymm0, ymm0, ymm2, 0x55
	vpand      ymm0, ymm0, ymm4                 // c0 = masked low-order bits of b[0] * c[0]  .... b[7] * c[7]

	vpsrlq     ymm3, ymm3, 12
	vpsrlq     ymm2, ymm2, 12
	vpslldq    ymm3, ymm3, 4
	vpblendd   ymm1, ymm3, ymm2, 0x55           // c1 = b[0] * c[0] >> 12, ...., b[7] * c[7] >> 12

	vpslld     ymm2, ymm0, 1
	vpaddd     ymm0, ymm0, ymm2                 // 3 * c0
	vpsubd     ymm0, ymm0, ymm1                 // 3 * c0 - c1
	
	vmovdqu    [rax], ymm0                      // store the result (8 elements) into a

	add        rax, 32
	add        rdx, 32
	add        rcx, 32
	cmp        rax, rsi
	jb         loop5
	ret

/*
 * Multiplication by a scalar + reduction:
 *  a[i] = red(a[i] * c)
 *
 * Input:
 * - rdi = start of array a (array of signed 32bit integers)
 * - rsi = array size
 * - rdx = scalar c
 *
 * The number of elements must be positive and a multiple of 16.
 */
	.balign 16
	.global _G(scalar_mul_reduce_array_asm)
_G(scalar_mul_reduce_array_asm):	
	vmovdqa ymm4, [mask+rip]
	mov     rax, rdi
	lea     rsi, [rdi+4*rsi]
	vmovd	xmm0, rdx
	vpbroadcastd ymm1, xmm0                 // ymm1 = 8 copies of scalar c

	.balign 16
loop6:
	vmovdqu    ymm0, [rax]                  // ymm0 = 8 elements of array a

	vpmuldq    ymm2, ymm0, ymm1             // ymm2 = a[0] * c, a[2] * c, a[4] * c, a[6] * c
	vpshufd    ymm0, ymm0, 0x31
	vpmuldq    ymm3, ymm0, ymm1             // ymm3 = a[1] * c, a[3] * c, a[5] * c, a[7] * c
	
	vpslldq    ymm0, ymm3, 4
	vpblendd   ymm0, ymm0, ymm2, 0x55
	vpand      ymm0, ymm0, ymm4             // ymm0 = masked low-order bits of a[0] * c  .... a[7] * c = the c0 part

	vpsrlq     ymm3, ymm3, 12
	vpsrlq     ymm2, ymm2, 12
	vpslldq    ymm3, ymm3, 4
	vpblendd   ymm3, ymm3, ymm2, 0x55       // ymm3 = a[0] * c >> 12, ...., a[7] * c >> 12 = the c1 part

	vpslld     ymm2, ymm0, 1
	vpaddd     ymm0, ymm0, ymm2             // 3 * c0
	vpsubd     ymm0, ymm0, ymm3             // 3 * c0 - c1
	
	vmovdqu    [rax], ymm0                  // store the result (8 elements) into a

	add        rax, 32
	cmp 	   rax, rsi
	jb         loop6
	ret

/*
 * Cooley-tukey: bit-reverse to standard order
 *
 * Input:
 * - rdi = start of array a
 * - rsi = size of array a (must be a positive multiple of 16)
 * - rdx = start of array p
 *
 * a is an array of 32bit integers.
 * NTT a is stored in place. 
 * p is a constant array of powers of omega (signed 16bit constants).
 *
 * EXPLANATIONS FOR THE CODE
 * -------------------------
 * The first loop computes the NTT of blocks of 8 elements.
 * It includes three rounds of computation. To prepare for these rounds,
 * we load the first 8 elements of array p into ymm5 and ymm6.
 *
 * For round 1, we ignore p[1] since it's equal to inverse(3)
 * For round 2, we use p[2] and p[3]
 *      ymm6 = p[2] 0 p[3] 0 p[2] 0 p[3] 0
 * For round 2, we use p[4], p[5], p[6], p[7]:
 *      ymm5 = p[4] 0 p[5] 0 p[6] 0 p[7] 0
 *
 * Round1:
 * - input: ymm0 = a[0] a[1] .... a[7]  (read from memory)	
 * - result stored in ymm2 and ymm3:
 *      ymm2[0] = a[0] + a[1]     ymm3[0] = a[0] - a[1]
 *      ymm2[2] = a[2] + a[3]     ymm3[2] = a[2] + a[3]
 *      ymm2[4] = a[4] + a[5]     ymm3[4] = a[4] - a[5]
 *      ymm2[6] = a[6] + a[7]     ymm3[6] = a[6] - a[7]
 *   the 32bit integers at index 1, 3, 5, 7 are not used.
 *
 * Round2:
 * - input: in ymm0 and ymm1 obtained by shuffling ymm2 and ymm3
 *      ymm0[0] = b[0]           ymm1[0] = b[2]
 *      ymm0[2] = b[1]           ymm1[2] = b[3]
 *	ymm0[4] = b[4]           ymm1[4] = b[6]
 *      ymm0[6] = b[5]           ymm1[6] = b[7]
 *   (elements at odd indices are ignored).
 * - multiply ymm1 by constants stored in ymm6:
 *      ymm1[0] = b[2] * p[2]
 *      ymm1[1] = b[3] * p[3]
 *      ymm1[2] = b[6] * p[2]
 *      ymm1[3] = b[7] * p[3]
 *   where ymm1[i] are 64bit integers.
 * - reduce ymm1[i] to four 32bit integers stored at indices 0, 2, 4, 6
 * - the compute result in ymm2 and ymm3:
 *      ymm2[0] = b[0] + red(b[2] * p[2])    ymm3[0] = b[0] - red(b[2] * p[2])
 *      ymm2[2] = b[1] + red(b[3] * p[3])    ymm3[2] = b[1] - red(b[3] * p[3])
 *      ymm2[4] = b[4] + red(b[6] * p[2])    ymm3[4] = b[4] - red(b[6] * p[2])
 *      ymm2[6] = b[5] + red(b[7] * p[3])    ymm3[6] = b[5] - red(b[7] * p[3])
 *
 * Round3:
 * - input in ymm0 and ymm1 obtained by shuffling ymm2 and ymm3:
 *      ymm0[0] = c[0]		  ymm1[0] = c[4]  
 *      ymm0[2] = c[1]            ymm1[2] = c[5]
 *      ymm0[4] = c[2]            ymm1[4] = c[6]
 *      ymm0[6] = c[3]            ymm1[6] = c[7]
 * - multiply ymm1 by constants stored in ymm5:
 *      ymm1[0] = c[4] * p[4]
 *      ymm1[1] = c[5] * p[5]
 *      ymm1[2] = c[6] * p[6]
 *      ymm1[3] = c[7] * p[7]
 *   (ymm1 contains four 64bit integers)
 * - reduce ymm1 to four 32bit integers, stored at indices 0, 2, 4, 6
 * - compute the result in ymm2 and ymm3
 *      ymm2[0] = c[0] + red(c[4] * p[4])     ymm3[0] = c[0] - red(c[4] * p[4])
 *      ymm2[2] = c[1] + red(c[5] * p[5])     ymm3[2] = c[1] - red(c[5] * p[5])
 *      ymm2[4] = c[2] + red(c[6] * p[6])     ymm3[4] = c[2] - red(c[6] * p[6])
 *      ymm2[6] = c[4] + red(c[7] * p[7])     ymm3[6] = c[3] - red(c[7] * p[7])
 *
 * End of the loop: shuffle ymm2 and ymm3 then merge into ymm0, store ymm0 in memory.
 */ 
	.balign 16
	.global _G(ntt_red_ct_rev2std_asm)
_G(ntt_red_ct_rev2std_asm):
	mov     rax, rdi                     // rax = start of array a
	mov     rcx, rsi                     // rcx = copy of the array size
	lea     rsi, [rdi+4*rsi]             // rsi = end of array a

	vpmovsxwd ymm4, [rdx]               // ymm4 = 8 first elements of array p
	vmovdqa   ymm5, [perm5+rip]
	vpermd    ymm5, ymm5, ymm4          // ymm5 = p[4] 0 p[5] 0 p[6] 0 p[7] 0
	vmovdqa   ymm6, [perm4+rip]
	vpermd    ymm6, ymm6, ymm4          // ymm6 = p[2] 0 p[3] 0 p[2] 0 p[3] 0

	vmovdqa   ymm4, [mask+rip]          // ymm4 = 8 copies of 4095

	.balign 16
/*
 * First loop: blocks of 8 integers.
 */
size8_loop:
	vmovdqu ymm0, [rax]                 // ymm0 = a0 a1 a2 a3 a4 a5 a6 a7
// Round1:
	vpsrldq  ymm1, ymm0, 4              // ymm1 = a1 a2 a3  0 a5 a6 a7 0
        vpaddd   ymm2, ymm0, ymm1           // ymm2 = (a0 + a1) -- (a2 + a3) -- (a4 + a5) -- (a6 + a7) --
	vpsubd   ymm3, ymm0, ymm1           // ymm3 = (a0 - a1) -- (a2 - a3) -- (a4 - a5) -- (a6 - a7) --

// Shuffle to prepare for Round2
	vshufps  ymm0, ymm2, ymm3, 0x44     // ymm0 = b0  -- b1 -- b4 -- b5 --
	vshufps  ymm1, ymm2, ymm3, 0xee     // ymm1 = b2  -- b3 -- b6 -- b7 --

// Round2:
	vpmuldq ymm1, ymm1, ymm6            // b2 * 1 -- b3 * w -- b6 * 1 -- b7 * w
	vpand   ymm2, ymm1, ymm4            // mask low-order bits = the c0 part
	vpsrlq  ymm1, ymm1, 12              // ymm1 = shift by 12 bits = the c1 part
	vpslld  ymm3, ymm2, 1
	vpaddd  ymm2, ymm2, ymm3            // ymm2 = 3 * c0
	vpsubd  ymm1, ymm2, ymm1            // ymm1 = 3 * c0 - c1
	vpaddd  ymm2, ymm0, ymm1            // ymm2 = b0 + red(1 * b2) -- b2 + red(w b3) -- b4 + red(1 * b6) -- b5 + red(w * b7)
	vpsubd  ymm3, ymm0, ymm1            // ymm3 = b0 - red(1 * b2) -- b2 - red(w b3) -- b4 - red(1 * b6) -- b5 - red(w * b7)

// Shuffle to prepare for Round3
	vperm2i128 ymm0, ymm2, ymm3, 0x20
	vperm2i128 ymm1, ymm2, ymm3, 0x31

// Round3:	
	vpmuldq ymm1, ymm1, ymm5
	vpand   ymm2, ymm1, ymm4            // mask low-order bits = the c0 part
	vpsrlq  ymm1, ymm1, 12              // ymm1 = shift by 12 bits = the c1 part
	vpslld  ymm3, ymm2, 1
	vpaddd  ymm2, ymm2, ymm3            // ymm2 = 3 * c0
	vpsubd  ymm1, ymm2, ymm1            // ymm1 = 3 * c0 - c1
	vpaddd  ymm2, ymm0, ymm1            // ymm2 = b0 + red(1 * b2) -- b2 + red(w b3) -- b4 + red(1 * b6) -- b5 + red(w * b7)
	vpsubd  ymm3, ymm0, ymm1            // ymm3 = b0 - red(1 * b2) -- b2 - red(w b3) -- b4 - red(1 * b6) -- b5 - red(w * b7)

// Shuffle and merge into ymm0
	vperm2i128 ymm0, ymm2, ymm3, 0x20
	vperm2i128 ymm1, ymm2, ymm3, 0x31
	vshufps    ymm0, ymm0, ymm1, 0x88

// Save result
	vmovdqu [rax], ymm0

	add     rax, 32
	cmp     rax, rsi
	jb      size8_loop

/*
 * Now deal with blocks of 16 integers
 */
	mov        rax, rdi                // rax = start of array a
	vpmovsxwd  ymm6, [rdx+16]          // ymm6 = p[8] ... p[15] = eight multipliers
	vpshufd    ymm5, ymm6, 0x31        // ymm5 = p[9] p[10] p[11] 0 p[13] p[14] p[15] 0

	.balign 16
size16_loop:
	vmovdqu    ymm0, [rax]             // ymm0 = lower half of a block = a[0 ... 7]
	vmovdqu    ymm1, [rax+32]          // ymm1 = upper half = a[8 ... 15]

	vpmuldq    ymm2, ymm1, ymm6        // ymm2 = a[8] * p[8], a[10] * p[10], a[12] * p[12], a[14] * p[14] 
	vpshufd    ymm1, ymm1, 0x31    
	vpmuldq    ymm3, ymm1, ymm5        // ymm3 = a[9] * p[9], a[11] * p[11], a[13] * p[13], a[15] * p[15]

	vpslldq    ymm1, ymm3, 4
	vpblendd   ymm1, ymm1, ymm2, 0x55
	vpand      ymm1, ymm1, ymm4        // c0 = masked low-order bits of a[8] * p[8]  .... a[15] * p[15]

	vpsrlq     ymm3, ymm3, 12
	vpsrlq     ymm2, ymm2, 12
	vpslldq    ymm3, ymm3, 4
	vpblendd   ymm2, ymm3, ymm2, 0x55  // c1 = a[8] * p[8] >> 12, ...., a[15] * p[15] >> 12

       	vpslld     ymm3, ymm1, 1
	vpaddd     ymm1, ymm1, ymm3        // 3 * c0
	vpsubd     ymm1, ymm1, ymm2        // 3 * c0 - c1

	vpaddd     ymm2, ymm0, ymm1
	vpsubd     ymm3, ymm0, ymm1
	
	vmovdqu    [rax], ymm2             // store the result
	vmovdqu    [rax+32], ymm3

	add        rax, 64
	cmp        rax, rsi
	jb         size16_loop

	cmp        rcx, 16
	jbe        done

/*
 * Blocks of size 32 and more
 */
	mov       r9, rcx
	shl	  r9, 1                    // r9 = 2 * n
	mov       r10, 32                  // r10 = 2 * step size
	add       rdx, r10                 // rdx --> p[16] ....
size_loop:
	mov       rax, rdi
loop_aux1:
	lea       rcx, [rax+2*r10]
	lea       r8, [rax+4*r10]
//
// r10 = step size * 2
// rax --> first half of a block
// rcx --> second half
// r8  --> end of block/start of the next block
// rdx --> start of multiplier table for this size
//
inner_loop:
	vmovdqu  ymm0, [rax]
	vmovdqu  ymm1, [rcx]
	vpmovsxwd ymm6, [rdx]

	// mulred
	vpmuldq   ymm2, ymm1, ymm6
	vpshufd   ymm1, ymm1, 0x31
	vpshufd   ymm6, ymm6, 0x31
	vpmuldq   ymm3, ymm1, ymm6
	vpslldq   ymm1, ymm3, 4
	vpblendd  ymm1, ymm1, ymm2, 0x55
	vpand     ymm1, ymm1, ymm4
	vpsrlq    ymm3, ymm3, 12
	vpsrlq    ymm2, ymm2, 12
	vpslldq   ymm3, ymm3, 4
	vpblendd  ymm2, ymm3, ymm2, 0x55
	vpslld    ymm3, ymm1, 1
	vpaddd    ymm1, ymm1, ymm3
	vpsubd    ymm1, ymm1, ymm2

	vpaddd    ymm2, ymm0, ymm1
	vpsubd    ymm3, ymm0, ymm1
	vmovdqu   [rax], ymm2
	vmovdqu   [rcx], ymm3

	add       rdx, 16
	add       rax, 32
	add       rcx, 32
	cmp       rcx, r8
	jb        inner_loop

	mov       rax, r8
	sub       rdx, r10
	cmp       rax, rsi
	jb        loop_aux1

	add       rdx, r10
	shl       r10, 1
	cmp       r10, r9
	jb        size_loop

done:
	ret
