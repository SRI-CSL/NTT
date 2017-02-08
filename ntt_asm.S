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
	vmovdqu ymm0, YMMWORD PTR [rax]            // load 8 elements
	vmovdqu ymm1, YMMWORD PTR [rax+32]         // load 8 elements

	vpsrad  ymm2, ymm0, 12                     // ymm2[i] = ymm0[i] >> 12 (arithmetic shift)	
	vpand   ymm0, ymm0, ymm3	           // ymm3[i] = ymm0[i] & 4095
	vpslld  ymm4, ymm0, 1                      // ymm4[i] = 2*ymm0[i]
	vpaddd  ymm0, ymm0, ymm4                   // ymm0[i] = 3*ymm0[i]
	vpsubd  ymm0, ymm0, ymm2

	vmovdqu YMMWORD PTR [rax], ymm0            // store 8 elements

	vpsrad  ymm2, ymm1, 12                     // ymm2[i] = ymm1[i] >> 12	
	vpand   ymm1, ymm1, ymm3	           // ymm3[i] = ymm1[i] & 4095
	vpslld  ymm4, ymm1, 1                      // ymm4[i] = 2*ymm1[i]
	vpaddd  ymm1, ymm1, ymm4                   // ymm1[i] = 3*ymm1[i]
	vpsubd  ymm1, ymm1, ymm2

	vmovdqu YMMWORD PTR [rax+32], ymm1         // store 8 elements

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
	vmovdqu ymm0, YMMWORD PTR [rax]            // load 8 elements
	vmovdqu ymm1, YMMWORD PTR [rax+32]         // load 8 elements

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

	vmovdqu YMMWORD PTR [rax], ymm0            // store 8 elements

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
	
	vmovdqu YMMWORD PTR [rax+32], ymm1         // store 8 elements

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
	vmovdqu ymm0, YMMWORD PTR [rax]            // load 8 elements
	vmovdqu ymm1, YMMWORD PTR [rax+32]         // load 8 elements

	vpsrad  ymm2, ymm0, 31                     // ymm2[i] = -1 if ymm0[i] < 0
	vpcmpgtd ymm5, ymm0, ymm4                  // ymm5[i] = -1 if ymm0[i] >= Q
	vpand   ymm2, ymm2, ymm3
	vpand   ymm5, ymm5, ymm3
	vpaddd  ymm0, ymm0, ymm2
	vpsubd  ymm0, ymm0, ymm5
	
	vmovdqu YMMWORD PTR [rax], ymm0            // store 8 elements
	
	vpsrad  ymm2, ymm1, 31                     // ymm2[i] = all ones if ymm1[i] < 0
	vpcmpgtd ymm5, ymm1, ymm4                  // ymm5[i] = all ones if ymm1[i] >= Q
	vpand   ymm2, ymm2, ymm3
	vpand   ymm5, ymm5, ymm3
	vpaddd  ymm1, ymm1, ymm2
	vpsubd  ymm1, ymm1, ymm5
	
	vmovdqu YMMWORD PTR [rax+32], ymm1         // store 8 elements

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
	vmovdqu    ymm0, YMMWORD PTR [rax]          // ymm0 = 8 elements of array a
	vpmovsxwd  ymm1, XMMWORD PTR [rdx]          // ymm1 = 8 elements of array p, sign-extended to 32bits

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
	
	vmovdqu    YMMWORD PTR [rax], ymm0          // store the result

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
	mov rax, rdi
	lea rsi, [rdi+4*rsi]

	.balign 16
loop4:
	vpmovsxdq  ymm0, XMMWORD PTR [rax]        // ymm0 = 4 elements of a, sign-extended to 64 bits
	vpmovsxwq  ymm1, QWORD PTR [rdx]          // ymm1 = 4 elements of p, sign-extended to 64 bits
	vpmuldq    ymm0, ymm1, ymm0               // product

	vmovdqu    ymm3, ymm0
	vpand      ymm0, ymm6, ymm0               // c0
	vpsrlq     ymm3, ymm3, 12                 // c1
	vpslld     ymm4, ymm0, 1                  // 2*c0
	vpsubd     ymm3, ymm0, ymm3               // c0-c1
	vpaddd     ymm0, ymm3, ymm4               // 3*c0-c1 

	vpermd     ymm0, ymm5, ymm0 
	vmovdqu    XMMWORD PTR [rax], xmm0

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
 * - rsi = array sizes
 * - rdx = start of array b (array of signed 32bit integers)
 * - rcx = start of array c (array of signed 32bit integers)
 *
 * The number of elements must be positive and a multiple of 16.
 */
	.balign 16
	.global _G(mul_reduce_array_asm)
_G(mul_reduce_array_asm):
	vmovdqa ymm4, [mask+rip]
	mov rax, rdi
	lea rsi, [rdi+4*rsi]

	.balign 16
loop5:
	vmovdqu    ymm0, YMMWORD PTR [rdx]          // ymm0 = 8 elements of array b
	vmovdqu    ymm1, YMMWORD PTR [rcx]          // ymm1 = 8 elements of array c
	
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
	
	vmovdqu    YMMWORD PTR [rax], ymm0          // store the result into a

	add        rax, 32
	add        rdx, 32
	add        rcx, 32
	cmp        rax, rsi
	jb         loop5
	ret
