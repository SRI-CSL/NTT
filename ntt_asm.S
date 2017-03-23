/*
 * BD: variant implementations of NTT for Intel x86_64
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

// for ntt_red_ct_rev2std and mulntt_red_ct_rev2std
perm5:
        .long 4, 0, 5, 0, 6, 0, 7, 0
perm4:
        .long 2, 0, 3, 0, 2, 0, 3, 0
perm3:
        .long 1, 0, 1, 0, 1, 0, 1, 0

// for ntt_red_ct_std2rev
perm2020:
        .long 0, 0, 0, 0, 2, 0, 2, 0

perm0426:
        .long 0, 0, 4, 0, 2, 0, 6, 0

// for ntt_red_gs_std2rev and mulntt_red_gs_std2rev
perm04152637:
	.long 0, 4, 1, 5, 2, 6, 3, 7

// for mulntt_red_gs_std2rev
perm5070:
	.long 5, 0, 7, 0, 5, 0, 7, 0

perm4060:
        .long 4, 0, 6, 0, 4, 0, 6, 0

perm_bdcst1:
	.long 1, 0, 1, 0, 1, 0, 1, 0

perm_bdcst2:
	.long 2, 0, 2, 0, 2, 0, 2, 0
	
perm_bdcst3:
	.long 3, 0, 3, 0, 3, 0, 3, 0

	.text
/***********************************************************
 * Check whether the processor + OS support AVX and AVX2
 *
 * This follows the intel manual.
 *
 * No input parameters.
 * - return with rax = 1 if AVX2 is supported
 * - return with rax = 0 otherwise
 ***********************************************************/
        .balign 16
        .global _G(avx2_supported)
_G(avx2_supported):
        push rbx                // rax/rbx/rcx/rdx are modified by CPUID
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
        mov eax, 1              // all good: supported
        pop rbx
        ret
not_supported:
        xor eax, eax
        pop rbx
        ret


/*************************************************************************
 * Reduce all elements of an array of signed 32bit integers
 *
 * Input:
 * - rdi = start of the array
 * - rsi = number of elements (must be positive and a multiple of 16)
 *
 * The array is updated in place
 *************************************************************************/
        .balign 16
        .global _G(reduce_array_asm)
_G(reduce_array_asm):
        vmovdqa ymm3, [mask+rip]
        mov rax, rdi
        lea rsi, [rdi+4*rsi]

loop0:  
        vmovdqu ymm0, [rax]                        // load 8 elements
        vmovdqu ymm1, [rax+32]                     // load 8 elements

        vpsrad  ymm2, ymm0, 12                     // ymm2[i] = ymm0[i] >> 12 (arithmetic shift)        
        vpand   ymm0, ymm0, ymm3                   // ymm3[i] = ymm0[i] & 4095
        vpslld  ymm4, ymm0, 1                      // ymm4[i] = 2*ymm0[i]
        vpaddd  ymm0, ymm0, ymm4                   // ymm0[i] = 3*ymm0[i]
        vpsubd  ymm0, ymm0, ymm2

        vmovdqu [rax], ymm0                        // store 8 elements

        vpsrad  ymm2, ymm1, 12                     // ymm2[i] = ymm1[i] >> 12   
        vpand   ymm1, ymm1, ymm3                   // ymm3[i] = ymm1[i] & 4095
        vpslld  ymm4, ymm1, 1                      // ymm4[i] = 2*ymm1[i]
        vpaddd  ymm1, ymm1, ymm4                   // ymm1[i] = 3*ymm1[i]
        vpsubd  ymm1, ymm1, ymm2

        vmovdqu [rax+32], ymm1                     // store 8 elements

        add rax, 64
        cmp rax, rsi
        jb loop0
        ret

/*
 * Variant implementation: don't unroll the loop.
 * Process 8 elements at a time.
 */
        .balign 16
        .global _G(reduce_array_asm2)
_G(reduce_array_asm2):
        vmovdqa ymm3, [mask+rip]
        mov rax, rdi
        lea rsi, [rdi+4*rsi]

loop0b:
        vmovdqu ymm0, [rax]                        // load 8 elements

        vpsrad  ymm2, ymm0, 12                     // ymm2[i] = ymm0[i] >> 12 (arithmetic shift)
        vpand   ymm0, ymm0, ymm3                   // ymm3[i] = ymm0[i] & 4095
        vpslld  ymm4, ymm0, 1                      // ymm4[i] = 2*ymm0[i]
        vpaddd  ymm0, ymm0, ymm4                   // ymm0[i] = 3*ymm0[i]
        vpsubd  ymm0, ymm0, ymm2

        vmovdqu [rax], ymm0                        // store 8 elements

        add rax, 32
        cmp rax, rsi
        jb loop0b
        ret


/**************************************************************************
 * Reduce all elements of an array twice
 *
 * Input:
 * - rdi = start of the array
 * - rsi = number of elements (must be positive and a multiple of 16)
 *
 * The array is updated in place
 **************************************************************************/
        .balign 16
        .global _G(reduce_array_twice_asm)
_G(reduce_array_twice_asm):
        vmovdqa ymm3, [mask+rip]
        mov rax, rdi
        lea rsi, [rdi+4*rsi]

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
 * Variant: process eight elements at a time. Don't unroll the loop.
 */
        .balign 16
        .global _G(reduce_array_twice_asm2)
_G(reduce_array_twice_asm2):
        vmovdqa ymm3, [mask+rip]
        mov rax, rdi
        lea rsi, [rdi+4*rsi]

loop1b: 
        vmovdqu ymm0, [rax]                        // load 8 elements

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

        add rax, 32
        cmp rax, rsi
        jb loop1b
        ret


/**************************************************************************
 * Correction: convert all elements to integers between
 * 0 and 12288. This assumes that the input integers satisfy
 * -Q <= a[i] <= 2*Q -1 (where Q=12289).
 *
 * Input:
 * - rdi = start of the array
 * - rsi = number of elements (must be positive and a multiple of 16).
 *
 * The array is updated in place.
 **************************************************************************/
        .balign 16
        .global _G(correct_asm)
_G(correct_asm):
        vmovdqa ymm3, [q_x8+rip]
        vmovdqa ymm4, [q_minus1_x8+rip]
        mov rax, rdi
        lea rsi, [rdi+4*rsi]

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

/**************************************************************************
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
 *************************************************************************/

/*      
 * For product + reduction, we use the same pattern in many places.
 * Assuming ymm0 and ymm1 contain eight integers to multiply:
 *     ymm0 = a[0] ... a[7]
 *     ymm1 = p[0] ... p[7]
 * and ymm4 = [0xfff, 0xfff, ...., 0xfff] = mask.
 *
 * We do this:
 * 
 *      vpmuldq   ymm2, ymm0, ymm1    --> ymm2 = four products: a[0] * p[0], a[2] * p[2], a[4] * p[4], a[6] * p[6]
 *      vpshufd   ymm0, ymm0, 0x31
 *      vpshufd   ymm1, ymm1, 0x31
 *      vpmuldq   ymm3, ymm0, ymm1    --> ymm3 = four products: a[1] * p[1], a[3] * p[3], a[5] * p[5], a[7] * p[7]
 *
 *      vpslldq   ymm0, ymm3, 4
 *      vpblendd  ymm0, ymm0, ymm2, 0x55 
 *      vpand     ymm0, ymm0, ymm4    --> ymm0 = c0 part = low-order bits of a[0] * p[0]  .... a[7] * p[7] 
 *
 *      vpsrlq     ymm3, ymm3, 12
 *      vpsrlq     ymm2, ymm2, 12
 *      vpslldq    ymm3, ymm3, 4
 *      vpblendd   ymm1, ymm3, ymm2, 0x55   -->  ymm1 = c1 part = products shifted by 12 (and truncated to 32bits)
 *                                               ymm1 = a[0] * p[0] >> 12, ...., a[7] * p[7] >> 12
 *
 *      vpslld     ymm2, ymm0, 1
 *      vpaddd     ymm0, ymm0, ymm2       --> ymm0 = eight 32bit integers (3 * c0)
 *      vpsubd     ymm0, ymm0, ymm1       --> ymm0 = eight 32bit integers = (3 * c0 - c1) = result
 *      
 */
        .balign 16
        .global _G(mul_reduce_array16_asm)
_G(mul_reduce_array16_asm):
        vmovdqa ymm4, [mask+rip]
        mov rax, rdi
        lea rsi, [rdi+4*rsi]

loop3:
        vmovdqu    ymm0, [rax]                      // ymm0 = 8 elements of array a
        vpmovsxwd  ymm1, [rdx]                      // ymm1 = 8 elements of array p, sign-extended to 32bits

        // mul-reduce
        vpmuldq    ymm2, ymm0, ymm1
        vpshufd    ymm0, ymm0, 0x31
        vpshufd    ymm1, ymm1, 0x31
        vpmuldq    ymm3, ymm0, ymm1

        vpslldq    ymm0, ymm3, 4
        vpblendd   ymm0, ymm0, ymm2, 0x55
        vpand      ymm0, ymm0, ymm4

        vpsrlq     ymm3, ymm3, 12
        vpsrlq     ymm2, ymm2, 12
        vpslldq    ymm3, ymm3, 4
        vpblendd   ymm1, ymm3, ymm2, 0x55

        vpslld     ymm2, ymm0, 1
        vpaddd     ymm0, ymm0, ymm2
        vpsubd     ymm0, ymm0, ymm1
        
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
        vmovdqa    ymm5, [perm0246+rip]
        vmovdqa    ymm6, [mask+rip]
        mov        rax, rdi
        lea        rsi, [rdi+4*rsi]

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
        

/**************************************************************************
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
 **************************************************************************/

        .balign 16
        .global _G(mul_reduce_array_asm)
_G(mul_reduce_array_asm):
        vmovdqa ymm4, [mask+rip]
        mov     rax, rdi
        lea     rsi, [rdi+4*rsi]

loop5:
        vmovdqu    ymm0, [rdx]                      // ymm0 = 8 elements of array b
        vmovdqu    ymm1, [rcx]                      // ymm1 = 8 elements of array c

        // mul-reduce
        vpmuldq    ymm2, ymm0, ymm1
        vpshufd    ymm0, ymm0, 0x31
        vpshufd    ymm1, ymm1, 0x31
        vpmuldq    ymm3, ymm0, ymm1

        vpslldq    ymm0, ymm3, 4
        vpblendd   ymm0, ymm0, ymm2, 0x55
        vpand      ymm0, ymm0, ymm4

        vpsrlq     ymm3, ymm3, 12
        vpsrlq     ymm2, ymm2, 12
        vpslldq    ymm3, ymm3, 4
        vpblendd   ymm1, ymm3, ymm2, 0x55

        vpslld     ymm2, ymm0, 1
        vpaddd     ymm0, ymm0, ymm2
        vpsubd     ymm0, ymm0, ymm1
        
        vmovdqu    [rax], ymm0                      // store the result (8 elements) into a

        add        rax, 32
        add        rdx, 32
        add        rcx, 32
        cmp        rax, rsi
        jb         loop5
        ret

/**************************************************************************
 * Multiplication by a scalar + reduction:
 *  a[i] = red(a[i] * c)
 *
 * Input:
 * - rdi = start of array a (array of signed 32bit integers)
 * - rsi = array size
 * - rdx = scalar c
 *
 * The number of elements must be positive and a multiple of 16.
 **************************************************************************/
        .balign 16
        .global _G(scalar_mul_reduce_array_asm)
_G(scalar_mul_reduce_array_asm):        
        vmovdqa ymm4, [mask+rip]
        mov     rax, rdi
        lea     rsi, [rdi+4*rsi]
        vmovd   xmm0, rdx
        vpbroadcastd ymm1, xmm0                 // ymm1 = 8 copies of scalar c

loop6:
        vmovdqu    ymm0, [rax]                  // ymm0 = 8 elements of array a

        // mul-reduce
        vpmuldq    ymm2, ymm0, ymm1
        vpshufd    ymm0, ymm0, 0x31
        vpmuldq    ymm3, ymm0, ymm1
        
        vpslldq    ymm0, ymm3, 4
        vpblendd   ymm0, ymm0, ymm2, 0x55
        vpand      ymm0, ymm0, ymm4

        vpsrlq     ymm3, ymm3, 12
        vpsrlq     ymm2, ymm2, 12
        vpslldq    ymm3, ymm3, 4
        vpblendd   ymm3, ymm3, ymm2, 0x55

        vpslld     ymm2, ymm0, 1
        vpaddd     ymm0, ymm0, ymm2
        vpsubd     ymm0, ymm0, ymm3
        
        vmovdqu    [rax], ymm0                  // store the result (8 elements) into a

        add        rax, 32
        cmp        rax, rsi
        jb         loop6
        ret

        
/**************************************************************************
 * Basic NTT using Cooley-Tukey: bit-reverse to standard order
 *
 * Input:
 * - rdi = start of array a
 * - rsi = size of array a (must be a positive multiple of 16)
 * - rdx = start of array p
 *
 * a is an array of 32bit integers.
 * NTT a is stored in place. 
 * p is a constant array of powers of omega (signed 16bit constants).
 **************************************************************************/

/*
 * EXPLANATIONS FOR THE CODE
 * -------------------------
 * The first loop computes the NTT of blocks of 8 elements.
 * The second loop deals with blocks of 16 elements.
 * The third loop deals with blocks of size 32 and higher.
 *
 * First loop
 * ----------
 * The first loop includes three rounds of computation. To prepare for
 * these rounds, we load the first elements of array p into ymm5 and ymm6.
 *
 * For round 1, we ignore p[1] since it's equal to inverse(3)
 *
 * For round 2, we use p[2] and p[3]
 *      ymm6 = p[2] 0 p[3] 0 p[2] 0 p[3] 0
 *
 * For round 3, we use p[4], p[5], p[6], p[7]:
 *      ymm5 = p[4] 0 p[5] 0 p[6] 0 p[7] 0
 *
 * Round1:
 * - input: ymm0 = a[0] a[1] .... a[7]  (read from memory)      
 * - result stored in ymm2 and ymm3:
 *      ymm2[0] = a[0] + a[1]     ymm3[0] = a[0] - a[1]
 *      ymm2[2] = a[2] + a[3]     ymm3[2] = a[2] - a[3]
 *      ymm2[4] = a[4] + a[5]     ymm3[4] = a[4] - a[5]
 *      ymm2[6] = a[6] + a[7]     ymm3[6] = a[6] - a[7]
 *   the 32bit integers at index 1, 3, 5, 7 are not used.
 *
 * Round2:
 * - input: in ymm0 and ymm1 obtained by shuffling ymm2 and ymm3
 *      ymm0[0] = b[0]           ymm1[0] = b[2]
 *      ymm0[2] = b[1]           ymm1[2] = b[3]
 *      ymm0[4] = b[4]           ymm1[4] = b[6]
 *      ymm0[6] = b[5]           ymm1[6] = b[7]
 *   (elements at odd indices are ignored).
 * - multiply ymm1 by constants stored in ymm6:
 *      ymm1[0] = b[2] * p[2]
 *      ymm1[1] = b[3] * p[3]
 *      ymm1[2] = b[6] * p[2]
 *      ymm1[3] = b[7] * p[3]
 *   where ymm1[i] are 64bit integers.
 * - reduce ymm1[i] to four 32bit integers stored at indices 0, 2, 4, 6
 * - then compute the result in ymm2 and ymm3:
 *      ymm2[0] = b[0] + red(b[2] * p[2])    ymm3[0] = b[0] - red(b[2] * p[2])
 *      ymm2[2] = b[1] + red(b[3] * p[3])    ymm3[2] = b[1] - red(b[3] * p[3])
 *      ymm2[4] = b[4] + red(b[6] * p[2])    ymm3[4] = b[4] - red(b[6] * p[2])
 *      ymm2[6] = b[5] + red(b[7] * p[3])    ymm3[6] = b[5] - red(b[7] * p[3])
 *
 * Round3:
 * - input in ymm0 and ymm1 obtained by shuffling ymm2 and ymm3:
 *      ymm0[0] = c[0]            ymm1[0] = c[4]  
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
 */ 
        .balign 16
        .global _G(ntt_red_ct_rev2std_asm)
_G(ntt_red_ct_rev2std_asm):
        mov     rax, rdi                     // rax = start of array a
        mov     r9, rsi                      // r9 = copy of the array size
        lea     rsi, [rdi+4*rsi]             // rsi = end of array a

        vpmovsxwd ymm4, [rdx]               // ymm4 = 8 first elements of array p
        vmovdqa   ymm5, [perm5+rip]
        vpermd    ymm5, ymm5, ymm4          // ymm5 = p[4] 0 p[5] 0 p[6] 0 p[7] 0
        vmovdqa   ymm6, [perm4+rip]
        vpermd    ymm6, ymm6, ymm4          // ymm6 = p[2] 0 p[3] 0 p[2] 0 p[3] 0

        vmovdqa   ymm4, [mask+rip]          // ymm4 = 8 copies of 4095

/*
 * First loop: blocks of 8 integers.
 */
ct_r2s_size8_loop:
        vmovdqu ymm0, [rax]                 // ymm0 = a0 a1 a2 a3 a4 a5 a6 a7
// Round1:
        vpsrldq  ymm1, ymm0, 4              // ymm1 = a1 a2 a3  0 a5 a6 a7  0
        vpaddd   ymm2, ymm0, ymm1           // ymm2 = (a0 + a1) -- (a2 + a3) -- (a4 + a5) -- (a6 + a7) --
        vpsubd   ymm3, ymm0, ymm1           // ymm3 = (a0 - a1) -- (a2 - a3) -- (a4 - a5) -- (a6 - a7) --

// Shuffle to prepare for Round2
        vshufps  ymm0, ymm2, ymm3, 0x44     // ymm0 = b0  -- b1 -- b4 -- b5 --
        vshufps  ymm1, ymm2, ymm3, 0xee     // ymm1 = b2  -- b3 -- b6 -- b7 --

// Round2:
        vpmuldq ymm1, ymm1, ymm6            // b2 * 1 -- b3 * w -- b6 * 1 -- b7 * w
        vpand   ymm2, ymm1, ymm4            // mask high-order bits = the c0 part
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
        vpand   ymm2, ymm1, ymm4            // mask high-order bits = the c0 part
        vpsrlq  ymm1, ymm1, 12              // ymm1 = shift by 12 bits = the c1 part
        vpslld  ymm3, ymm2, 1
        vpaddd  ymm2, ymm2, ymm3            // ymm2 = 3 * c0
        vpsubd  ymm1, ymm2, ymm1            // ymm1 = 3 * c0 - c1
        vpaddd  ymm2, ymm0, ymm1
        vpsubd  ymm3, ymm0, ymm1

// Shuffle and merge into ymm0
        vperm2i128 ymm0, ymm2, ymm3, 0x20
        vperm2i128 ymm1, ymm2, ymm3, 0x31
        vshufps    ymm0, ymm0, ymm1, 0x88

// Save result
        vmovdqu [rax], ymm0

        add     rax, 32
        cmp     rax, rsi
        jb      ct_r2s_size8_loop

/*
 * Now deal with blocks of 16 integers
 */
        mov        rax, rdi                // rax = start of array a
        vpmovsxwd  ymm6, [rdx+16]          // ymm6 = p[8] ... p[15] = eight multipliers
        vpshufd    ymm5, ymm6, 0x31        // ymm5 = p[9] p[10] p[11] 0 p[13] p[14] p[15] 0

ct_r2s_size16_loop:
        vmovdqu    ymm0, [rax]             // ymm0 = lower half of a block = a[0 ... 7]
        vmovdqu    ymm1, [rax+32]          // ymm1 = upper half = a[8 ... 15]

        // mul-reduce of ymm1 and ymm6, result in ymm1
        vpmuldq    ymm2, ymm1, ymm6
        vpshufd    ymm1, ymm1, 0x31    
        vpmuldq    ymm3, ymm1, ymm5

        vpslldq    ymm1, ymm3, 4
        vpblendd   ymm1, ymm1, ymm2, 0x55
        vpand      ymm1, ymm1, ymm4

        vpsrlq     ymm3, ymm3, 12
        vpsrlq     ymm2, ymm2, 12
        vpslldq    ymm3, ymm3, 4
        vpblendd   ymm2, ymm3, ymm2, 0x55

        vpslld     ymm3, ymm1, 1
        vpaddd     ymm1, ymm1, ymm3
        vpsubd     ymm1, ymm1, ymm2

        // ymm2 = ymm0 + mul-reduce result
        // ymm3 = ymm0 - mul-reduce result
        vpaddd     ymm2, ymm0, ymm1
        vpsubd     ymm3, ymm0, ymm1
        
        vmovdqu    [rax], ymm2             // store the result
        vmovdqu    [rax+32], ymm3

        add        rax, 64
        cmp        rax, rsi
        jb         ct_r2s_size16_loop

        cmp        r9, 16
        jbe        ct_r2s_done

/*
 * Blocks of size 32 and larger
 * - a block of size k is constructed by combining two half blocks 
 * - the step size below is half the block size = k/2
 */
        mov       r10, 32                  // r10 = 2 * step size
        lea       r11, [rdx+32]            // r11 --> segment of array p for this step size
                                           //     = p + 2 * step-size (since each element of p is two bytes)
ct_r2s_size32_loop:
        mov       rax, rdi

ct_r2s_loop_aux1:
        lea       rcx, [rax+2*r10]
        lea       r8, [rax+4*r10]
        mov       rdx, r11
//
// r10 = 2 * step size
// rax --> first half of a block
// rcx --> second half
// r8  --> end of block/start of the next block
// rdx --> start of the p table for the block
//
ct_r2s_inner_loop:
        vmovdqu  ymm0, [rax]
        vmovdqu  ymm1, [rcx]
        vpmovsxwd ymm6, [rdx]

        // mul-reduce of ymm1 and ymm6, result in ymm1
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

        // prepare for the next slices of 8 integers
        add       rdx, 16
        add       rax, 32
        add       rcx, 32
        cmp       rcx, r8
        jb        ct_r2s_inner_loop

        // prepare for the next block
        mov       rax, r8
        cmp       rax, rsi
        jb        ct_r2s_loop_aux1

        // next block size = double the current size
        // we stop when 2 * step size > r9 (r9 = array size)
        add       r11, r10
        shl       r10, 1
        cmp       r10, r9
        jbe       ct_r2s_size32_loop

ct_r2s_done:
        ret


/**************************************************************************
 * Combined product by power of psi and NTT
 * Based on Cooley-Tukey: bit-reverse to standard order
 *
 * Input:
 * - rdi = start of array a
 * - rsi = size of array a (must be a positive multiple of 16)
 * - rdx = start of array p
 *
 * a is an array of 32bit integers.
 * NTT a is stored in place. 
 * p is a constant array of powers of omega (signed 16bit constants).
 **************************************************************************/

/*
 * The code is the same as for ntt_red_ct_rev2std_asm, except for
 * Round 1 of the first loop. In this function, we can't assume that
 * p[1] is inverse(3).
 */
        .balign 16
        .global _G(mulntt_red_ct_rev2std_asm)
_G(mulntt_red_ct_rev2std_asm):
        mov     rax, rdi                     // rax = start of array a
        mov     r9, rsi                      // r9 = copy of the array size
        lea     rsi, [rdi+4*rsi]             // rsi = end of array a

        vpmovsxwd ymm4, [rdx]               // ymm4 = 8 first elements of array p
        vmovdqa   ymm5, [perm5+rip]
        vpermd    ymm5, ymm5, ymm4          // ymm5 = p[4] 0 p[5] 0 p[6] 0 p[7] 0
        vmovdqa   ymm6, [perm4+rip]
        vpermd    ymm6, ymm6, ymm4          // ymm6 = p[2] 0 p[3] 0 p[2] 0 p[3] 0
        vmovdqa   ymm7, [perm3+rip]
        vpermd    ymm7, ymm7, ymm4          // ymm7 = p[1] 0 p[1] 0 p[1] 0 p[1] 0

        vmovdqa   ymm4, [mask+rip]          // ymm4 = 8 copies of 4095

/*
 * First loop: blocks of 8 integers.
 */
mct_r2s_size8_loop2:
        vmovdqu ymm0, [rax]                 // ymm0 = a0 a1 a2 a3 a4 a5 a6 a7

// Round1:
        vpsrldq  ymm1, ymm0, 4              // ymm1 = a1 a2 a3  0 a5 a6 a7 0
        vpmuldq  ymm1, ymm1, ymm7           // ymm1 = a1 * p1,  a3 * p1, a5 * p1, a7 * p1 (four 64bit integers)
        vpand    ymm2, ymm1, ymm4           // mask high-order bits = the c0 part
        vpsrlq   ymm1, ymm1, 12             // ymm1 = shift by 12 bits = the c1 part
        vpslld   ymm3, ymm2, 1
        vpaddd   ymm2, ymm2, ymm3           // ymm2 = 3 * c0
        vpsubd   ymm1, ymm2, ymm1           // ymm1 = 3 * c0 - c1
        vpaddd   ymm2, ymm0, ymm1           // ymm2 = a0 + red(a1 * p1) -- a2 + red(a3 * p1) -- a4 + red(a5 * p1) -- a6 + red(a7 * p1) --
        vpsubd   ymm3, ymm0, ymm1           // ymm3 = a0 - red(a1 * p1) -- a2 - red(a3 * p1) -- a4 - red(a5 * p1) -- a6 - red(a7 * p1) --

// Shuffle to prepare for Round2
        vshufps  ymm0, ymm2, ymm3, 0x44     // ymm0 = b0  -- b1 -- b4 -- b5 --
        vshufps  ymm1, ymm2, ymm3, 0xee     // ymm1 = b2  -- b3 -- b6 -- b7 --

// Round2:
        vpmuldq ymm1, ymm1, ymm6            // b2 * p2 -- b3 * p3 -- b6 * p2 -- b7 * p3
        vpand   ymm2, ymm1, ymm4            // mask high-order bits = the c0 part
        vpsrlq  ymm1, ymm1, 12              // ymm1 = shift by 12 bits = the c1 part
        vpslld  ymm3, ymm2, 1
        vpaddd  ymm2, ymm2, ymm3            // ymm2 = 3 * c0
        vpsubd  ymm1, ymm2, ymm1            // ymm1 = 3 * c0 - c1
        vpaddd  ymm2, ymm0, ymm1            // ymm2 = b0 + red(p2 * b2) -- b2 + red(p3 * b3) -- b4 + red(p2 * b6) -- b5 + red(p3 * b7)
        vpsubd  ymm3, ymm0, ymm1            // ymm3 = b0 - red(p2 * b2) -- b2 - red(p3 * b3) -- b4 - red(p2 * b6) -- b5 - red(p3 * b7)

// Shuffle to prepare for Round3
        vperm2i128 ymm0, ymm2, ymm3, 0x20
        vperm2i128 ymm1, ymm2, ymm3, 0x31

// Round3:      
        vpmuldq ymm1, ymm1, ymm5
        vpand   ymm2, ymm1, ymm4            // mask high-order bits = the c0 part
        vpsrlq  ymm1, ymm1, 12              // ymm1 = shift by 12 bits = the c1 part
        vpslld  ymm3, ymm2, 1
        vpaddd  ymm2, ymm2, ymm3            // ymm2 = 3 * c0
        vpsubd  ymm1, ymm2, ymm1            // ymm1 = 3 * c0 - c1
        vpaddd  ymm2, ymm0, ymm1
        vpsubd  ymm3, ymm0, ymm1

// Shuffle and merge into ymm0
        vperm2i128 ymm0, ymm2, ymm3, 0x20
        vperm2i128 ymm1, ymm2, ymm3, 0x31
        vshufps    ymm0, ymm0, ymm1, 0x88

// Save result
        vmovdqu [rax], ymm0

        add     rax, 32
        cmp     rax, rsi
        jb      mct_r2s_size8_loop2

/*
 * The rest of the code is the same as in ntt_ct_rev2std.
 * We setup registers and jump there.
 */
        mov        rax, rdi                // rax = start of array a
        vpmovsxwd  ymm6, [rdx+16]          // ymm6 = p[8] ... p[15] = eight multipliers
        vpshufd    ymm5, ymm6, 0x31        // ymm5 = p[9] p[10] p[11] 0 p[13] p[14] p[15] 0
        jmp        ct_r2s_size16_loop


/***************************************************************************
 * Basic NTT using Cooley-Tukey: standard to bit-reverse order
 *
 * Input:
 * - rdi = start of array a
 * - rsi = size of array a (must be a positive multiple of 16)
 * - rdx = start of array p
 *
 * a is an array of 32bit integers.
 * NTT a is stored in place. 
 * p is a constant array of powers of omega (signed 16bit constants).
 **************************************************************************/

        .balign 16
        .global _G(ntt_red_ct_std2rev_asm)
_G(ntt_red_ct_std2rev_asm):
        lea       r8, [rdi+4*rsi]         // r8 = end of array a
        vmovdqa   ymm4, [mask+rip]        // ymm4 = bitmask = 8 copies of 4095

/*
 * First round:
 *  rsi/2 = d >= 8
 *  rax = start of array a
 *  rcx = middle of array a
 *
 * We process elements of a by blocks of 8 32bit integers
 *   a'[i, ..., i+7] = a[i, ..., i+7] + a[i+d, ..., i+d+7]
 *   a'[i+d, ..., i+d+7] = a[i, ..., i+7] - a[i+d, ..., i+d+7]
 * in the loop: rax --> block a[i, ..., i+7]
 *              rcx --> block a[i+d, ..., i+d+7]
 */
        mov    rax, rdi
        lea    rcx, [rdi+2*rsi]
ct_s2r_loop1:
        vmovdqu ymm0, [rax]
        vmovdqu ymm1, [rcx]
        vpaddd  ymm2, ymm0, ymm1
        vpsubd  ymm3, ymm0, ymm1
        vmovdqu [rax], ymm2
        vmovdqu [rcx], ymm3
        add     rcx, 32
        add     rax, 32
        cmp     rcx, r8
        jb      ct_s2r_loop1

/*
 * Next rounds, as long as d >= 8
 *  rsi/2 = d
 *  rdx --> start of array p for the round
 */
        add     rdx, 4          // p starts with [0, 1, 1, W, 1, W, W^2, W^3 ...]
                                // rdx --> [1, W, 1, W, W^2, W^3, ...
        shr     rsi, 1          // rsi := rsi/2
        cmp     rsi, 8
        jbe     ct_s2r_finish

ct_s2r_loop2:
        mov     rax, rdi          // rax = start of array a = a[0 .... d-1]
        lea     rcx, [rdi+2*rsi]  // rcx = a[d, ... 2d-1]
        mov     r9, rcx           // end pointer

ct_s2r_loop2_inner1:
// inner loop1: W = 1: so no mul_red necessary
        vmovdqu ymm0, [rax]
        vmovdqu ymm1, [rcx]
        vpaddd  ymm2, ymm0, ymm1
        vpsubd  ymm3, ymm0, ymm1
        vmovdqu [rax], ymm2
        vmovdqu [rcx], ymm3
        add     rax, 32
        add     rcx, 32
        cmp     rax, r9
        jb      ct_s2r_loop2_inner1

ct_s2r_loop2_aux:
        add     rdx, 2               // rdx --> coefficient U for the inner loop (U is 16 bits)
        vpbroadcastw xmm5, [rdx]     // xmm5 = 8 copies of U
        vpmovsxwq  ymm5, xmm5        // ymm5 = 4 copies of U, sign-extended to 64bits
        mov     rax, rcx             // rax --> a[i, ..., i+d-1]
        lea     rcx, [rax+2*rsi]     // rcx --> a[i+d, ..., i+2d-1]
        mov     r9, rcx              // end pointer
/*
 * inner loop2: processes blocks of 8 integers
 *      a'[i, ..., i+7] = a[i, ..., i+7] + mul_red(U, a[i+d, ..., i+d+7])
 *  a'[i+d, ..., i+d+7] = a[i, ..., i+7] - mul_red(U, a[i+d, ..., i+d+7])
 *
 * In the loop:
 *  rax --> a[i, ..., i+7]
 *  rcx --> a[i+d, ..., i+d+7]
 *  ymm5 contains 4 copies of U
 */
ct_s2r_loop2_inner2:
        vmovdqu ymm0, [rax]              // ymm0 = a[i, ..., i+7]
        vmovdqu ymm1, [rcx]              // ymm1 = a[i+d, ..., i+d+7]

        // mul-reduce: ymm1 * ymm5, result in ymm1
        vpmuldq   ymm2, ymm1, ymm5
        vpshufd   ymm1, ymm1, 0x31
        vpmuldq   ymm3, ymm1, ymm5
        vpslldq   ymm1, ymm3, 4
        vpblendd  ymm1, ymm1, ymm2, 0x55
        vpand     ymm1, ymm1, ymm4       // ymm1 = c0 part (8 32bit integers)

        vpsrlq    ymm3, ymm3, 12
        vpsrlq    ymm2, ymm2, 12
        vpslldq   ymm3, ymm3, 4
        vpblendd  ymm3, ymm3, ymm2, 0x55 // ymm3 = c1 part (also 8 32bit integers)

        vpslld    ymm2, ymm1, 1
        vpaddd    ymm1, ymm1, ymm2       // ymm1 = 3 * c0
        vpsubd    ymm1, ymm1, ymm3       // ymm1 = mul_red(U, a[i+d, ..., i+d+7])

        vpaddd    ymm2, ymm0, ymm1
        vpsubd    ymm3, ymm0, ymm1
        vmovdqu   [rax], ymm2
        vmovdqu   [rcx], ymm3

        add       rax, 32
        add       rcx, 32
        cmp       rax, r9
        jb        ct_s2r_loop2_inner2

        cmp       rcx, r8               // r8 = end of array a
        jb        ct_s2r_loop2_aux
        
        add       rdx, 2                // rdx --> [1, W, W^2, ... for the next round]
        shr       rsi, 1                // rsi := rsi/2
        cmp       rsi, 8
        ja        ct_s2r_loop2          // repeat if d > 8


/*
 * Final steps: for d = 4, 2, 1
 */
ct_s2r_finish:
        mov      rax, rdi                  // start of array a
        vmovdqa  ymm6, [perm2020+rip]
ct_s2r_finish_size4:
// process 16 elements at a time
        vpmovsxwq xmm5, [rdx]               // xmm5 = [U, V] sign-extended to 64bit integers
        vpermd    ymm5, ymm6, ymm5          // ymm5 = [U _ U _ | V _ V _ ]

        vmovdqu  ymm0, [rax]               // ymm0 = a[0 ... 3]  a[4 ... 7]
        vmovdqu  ymm1, [rax+32]            // ymm1 = a[8 ... 11] a[12 ... 15]
        vperm2i128 ymm2, ymm0, ymm1, 0x20  // ymm2 = a[0 ... 3]  a[8 ... 11]
        vperm2i128 ymm3, ymm0, ymm1, 0x31  // ymm3 = a[4 ... 7]  a[12 ... 15]

        // mulreduce ymm3 and ymm5
        vpmuldq   ymm0, ymm3, ymm5
        vpshufd   ymm3, ymm3, 0x31
        vpmuldq   ymm1, ymm3, ymm5
        vpslldq   ymm3, ymm1, 4
        vpblendd  ymm3, ymm3, ymm0, 0x55
        vpand     ymm3, ymm3, ymm4          // ymm3 = c0 part

        vpsrlq    ymm1, ymm1, 12
        vpsrlq    ymm0, ymm0, 12
        vpslldq   ymm1, ymm1, 4
        vpblendd  ymm1, ymm1, ymm0, 0x55    // ymm1 = c1 part

        vpslld    ymm0, ymm3, 1
        vpaddd    ymm3, ymm3, ymm0          // ymm3 = 3 * c0
        vpsubd    ymm3, ymm3, ymm1          // ymm3 = mul_red(U, a[4 ... 7]) | mul_red(V, a[12 ... 15])

        vpaddd    ymm0, ymm2, ymm3          // ymm0: lower half = a'[0 ... 3], upper half = a'[8 ... 11]
        vpsubd    ymm1, ymm2, ymm3          // ymm1: lower half = a'[4 ... 7], upper half = a'[12 ... 15]

        vperm2i128 ymm2, ymm0, ymm1, 0x20   // ymm2 = a'[0 ... 3] a'[4 ... 7]
        vperm2i128 ymm3, ymm0, ymm1, 0x31   // ymm3 = a'[8 ... 11] a'[12 ... 15]

        vmovdqu   [rax], ymm2
        vmovdqu   [rax+32], ymm3

        add      rax, 64
        add      rdx, 4
        cmp      rax, r8        
        jb       ct_s2r_finish_size4

        mov      rax, rdi
        vmovdqa  ymm6, [perm0426+rip]

ct_s2r_finish_size2:
        vpmovsxwq ymm5, [rdx]           // ymm5 = 4 multipliers: [U _ V _ W _ X _]
        vpermd    ymm5, ymm6, ymm5      // shuffled to [U _ W _ V _ X _ ]

        vmovdqu   ymm0, [rax]           // ymm0 = a[0 1] a[2 3]   a[4 5]   a[6 7]
        vmovdqu   ymm1, [rax+32]        // ymm1 = a[8 9] a[10 11] a[12 13] a[14 15]

        vshufpd   ymm2, ymm0, ymm1, 0x00   // ymm2 = a[0 1] a[8 9] a[4 5] a[12 13]
        vshufpd   ymm3, ymm0, ymm1, 0x0F   // ymm3 = a[2 3] a[10 11] a[6 7] a[14 15]

        // mulreduce ymm3 and ymm5: result in ymm3
        vpmuldq   ymm0, ymm3, ymm5
        vpshufd   ymm3, ymm3, 0x31
        vpmuldq   ymm1, ymm3, ymm5
        vpslldq   ymm3, ymm1, 4
        vpblendd  ymm3, ymm3, ymm0, 0x55
        vpand     ymm3, ymm3, ymm4          // ymm3 = c0 part

        vpsrlq    ymm1, ymm1, 12
        vpsrlq    ymm0, ymm0, 12
        vpslldq   ymm1, ymm1, 4
        vpblendd  ymm1, ymm1, ymm0, 0x55    // ymm1 = c1 part

        vpslld    ymm0, ymm3, 1
        vpaddd    ymm3, ymm3, ymm0          // ymm3 = 3 * c0
        vpsubd    ymm3, ymm3, ymm1          // ymm3 = mul_red(U, a[4 ... 7]) | mul_red(V, a[12 ... 15])

        vpaddd    ymm0, ymm2, ymm3          // ymm0 = a'[0 1] a'[8 9] a'[4 5] a'[12 13]
        vpsubd    ymm1, ymm2, ymm3          // ymm1 = a'[2 3] a'[10 11] a'[6 7] a'[14 15]
        
        vshufpd   ymm2, ymm0, ymm1, 0x00    // ymm2 = a'[0 1] a'[2 3] a'[4 5] a'[6 7]
        vshufpd   ymm3, ymm0, ymm1, 0x0F    // ymm3 = a'[8 9] a'[10 11] a'[12 13] a'[14 15]

        vmovdqu   [rax], ymm2
        vmovdqu   [rax+32], ymm3

        add      rax, 64
        add      rdx, 8
        cmp      rax, r8        
        jb       ct_s2r_finish_size2

        mov      rax, rdi
ct_s2r_finish_size1:
        vpmovsxwq ymm5, [rdx]           // ymm5 = 4 multipliers: [U0 _ U1 _ U2 _ U3 _]
        vpmovsxwq ymm6, [rdx+8]         // ymm6 = 4 next multipliers: [U4 _ U5 _ U6 _ U7 _]

        vmovdqu   ymm0, [rax]           // ymm0 = a[0] a[1] a[2]  a[3]  a[4]  a[5]  a[6]  a[7]
        vmovdqu   ymm1, [rax+32]        // ymm1 = a[8] a[9] a[10] a[11] a[12] a[13] a[14] a[15]

        vpslldq   ymm2, ymm1, 4            // ymm2 = ___ a[8] a[9] a[10] ___ a[12] a[13] a[14]
        vpblendd  ymm2, ymm0, ymm2, 0xaa   // ymm2 = a[0] a[8] a[2] a[10] a[4] a[12] a[6] a[14]

        vpsrldq   ymm0, ymm0, 4         // ymm0 = a[1] a[2] a[3] ___ a[5] a[6] a[7] ___
        vpmuldq   ymm0, ymm0, ymm5      // ymm0 = [U0 * a[1], U1 * a[3], U2 * a[5], U3 * a[7]]    (four 64bit numbers)
        vpsrldq   ymm1, ymm1, 4         // ymm1 = a[9] a[10] a[11] ___ a[13] a[14] a[15] ___
        vpmuldq   ymm1, ymm1, ymm6      // ymm1 = [U4 * a[9], U5 * a[11], U6 * a[13], U7 * a[15]] (four 64bit numbers)

        vpslldq   ymm3, ymm1, 4           // ymm3 = ymm1 shifted by 32 bits to the left
        vpblendd  ymm3, ymm3, ymm0, 0x55
        vpand     ymm3, ymm3, ymm4        // ymm3 = c0 part: eight 32bit integers

        vpsrlq    ymm1, ymm1, 12
        vpsrlq    ymm0, ymm0, 12
        vpslldq   ymm1, ymm1, 4
        vpblendd  ymm1, ymm1, ymm0, 0x55  // ymm1 = c1 part

        vpslld    ymm0, ymm3, 1
        vpaddd    ymm3, ymm3, ymm0        // ymm3 = 3 * c0
        vpsubd    ymm3, ymm3, ymm1        // ymm3 = 3 * c0 - c1 = mul_red

        vpaddd    ymm0, ymm2, ymm3        // ymm0 = a'[0] a'[8] a'[2] a'[10] a'[4] a'[12] a'[6] a'[14]
        vpsubd    ymm1, ymm2, ymm3        // ymm1 = a'[1] a'[9] a'[3] a'[11] a'[5] a'[13] a'[7] a'[15]

        vpslldq   ymm2, ymm1, 4
        vpblendd  ymm2, ymm0, ymm2, 0xaa  // ymm2 = a'[0] a'[1] a'[2] a'[3] a'[4] a'[5] a'[6] a'[7]
        vpsrldq   ymm0, ymm0, 4
        vpblendd  ymm3, ymm0, ymm1, 0xaa  // ymm3 = a'[8] a'[9] a'[10] .... a'[15]

        vmovdqu   [rax], ymm2
        vmovdqu   [rax+32], ymm3

        add       rax, 64
        add       rdx, 16
        cmp       rax, r8
        jb        ct_s2r_finish_size1   
        
        ret



/***************************************************************************
 * Combined product by powers of psi and NTT
 * Cooley-Tukey algorithm, standard to bit-reverse order
 *
 * Input:
 * - rdi = start of array a
 * - rsi = size of array a (must be a positive multiple of 16)
 * - rdx = start of array p
 *
 * a is an array of 32bit integers.
 * NTT a is stored in place. 
 * p is a constant array of powers of omega (signed 16bit constants).
 **************************************************************************/

        .balign 16
        .global _G(mulntt_red_ct_std2rev_asm)
_G(mulntt_red_ct_std2rev_asm):
        lea       r8, [rdi+4*rsi]         // r8 = end of array a
        vmovdqa   ymm4, [mask+rip]        // ymm4 = bitmask = 8 copies of 4095

/*
 * Basic rounds: as long as d=rsi/2 >= 8
 */
mct_s2r_loop:
        mov     rax, rdi          // rax = first block in array aa --> a[0 ... d-1]
        lea     rcx, [rdi+2*rsi]  // rcx = start of next block --> a[d, ... 2d-1]
        mov     r9, rcx           // end pointer = end of the first block

mct_s2r_loop_aux:
        add     rdx, 2               // rdx --> coefficient U for the inner loop (U is 16 bits)
        vpbroadcastw xmm5, [rdx]     // xmm5 = 8 copies of U
        vpmovsxwq  ymm5, xmm5        // ymm5 = 4 copies of U, sign-extended to 64bits
/*
 * Inner loop: process eight elements at a time
 * rax --> a[i ... i+7]
 * rcx --> a[i+d ... i+d+7]
 * ymm5 contains 4 copies of the multiplier U
 */
mct_s2r_loop_inner:
        vmovdqu ymm0, [rax]              // ymm0 = a[i, ..., i+7]
        vmovdqu ymm1, [rcx]              // ymm1 = a[i+d, ..., i+d+7]

        // mul-reduce: ymm1 * ymm5, result in ymm1
        vpmuldq   ymm2, ymm1, ymm5
        vpshufd   ymm1, ymm1, 0x31
        vpmuldq   ymm3, ymm1, ymm5
        vpslldq   ymm1, ymm3, 4
        vpblendd  ymm1, ymm1, ymm2, 0x55
        vpand     ymm1, ymm1, ymm4       // ymm1 = c0 part (8 32bit integers)

        vpsrlq    ymm3, ymm3, 12
        vpsrlq    ymm2, ymm2, 12
        vpslldq   ymm3, ymm3, 4
        vpblendd  ymm3, ymm3, ymm2, 0x55 // ymm3 = c1 part (also 8 32bit integers)

        vpslld    ymm2, ymm1, 1
        vpaddd    ymm1, ymm1, ymm2       // ymm1 = 3 * c0
        vpsubd    ymm1, ymm1, ymm3       // ymm1 = mul_red(U, a[i+d, ..., i+d+7])

        vpaddd    ymm2, ymm0, ymm1
        vpsubd    ymm3, ymm0, ymm1
        vmovdqu   [rax], ymm2
        vmovdqu   [rcx], ymm3

        add       rax, 32
        add       rcx, 32
        cmp       rax, r9
        jb        mct_s2r_loop_inner

        mov       rax, rcx              // rax --> a[i ... i+d-1]
        lea       rcx, [rax+2*rsi]      // rcx --> a[i+d ... i+2d-1]
        mov       r9, rcx
        cmp       rax, r8               // r8 = end of array a
        jb        mct_s2r_loop_aux

        shr       rsi, 1               // next block 
        cmp       rsi, 8
        ja        mct_s2r_loop

/*
 * The rest of the code is as in ntt_red_ct_std2rev_asm
 */
        add      rdx, 2
        jmp      ct_s2r_finish


/***************************************************************************
 * Basic NTT using Gentleman-Sande: bit-reverse to standard order
 *
 * Input:
 * - rdi = start of array a
 * - rsi = size of array a (must be a positive multiple of 16)
 * - rdx = start of array p
 *
 * a is an array of 32bit integers.
 * NTT a is stored in place. 
 * p is a constant array of powers of omega (signed 16bit constants).
 **************************************************************************/

        .balign 16
        .global _G(ntt_red_gs_rev2std_asm)
_G(ntt_red_gs_rev2std_asm):
        lea     rcx, [rdi+4*rsi]      // rcx -> end of array a
        lea     r8, [rdx+rsi]         // r8 --> multipliers for round 1
        shr     rsi, 1
        lea     r9, [rdx+rsi]         // r9 --> multipliers for round 2
        shr     rsi, 1
        lea     r10, [rdx+rsi]        // r10 --> multipliers for round 3
        mov     rax, rdi              // rax -> start of array a

        vmovdqa ymm4, [mask+rip]
        vmovdqa ymm6, [perm2020+rip]

/*
 * First loop: process blocks of eight integers
 * Each iteration corresponds to three rounds.
 */
gs_r2s_loop0:
// first round
        vpmovsxwq ymm5, [r8]         // ymm5 = 4 multipliers = [w0, w1, w2, w3] extended to 64 bits

        vmovdqu  ymm0, [rax]         // ymm0 = a0 a1 a2 a3 a4 a5 a6 a7
        vpsrldq  ymm1, ymm0, 4       // ymm1 = a1 a2 a3 0  a5 a6 a7 0
        vpsubd   ymm2, ymm0, ymm1    // ymm2 = [a0 - a1 __ a2 - a3 __ a4 - a5 __ a6 - a7 __ ]
        vpaddd   ymm0, ymm0, ymm1    // ymm0 = [a0 + a1 __ a2 + a3 __ a4 + a5 __ a6 + a7 __ ]
        vpmuldq  ymm2, ymm2, ymm5    // ymm2 = [(a0 - a1) * w0, (a2 - a3) * w1, (a4 - a5) * w2, (a6 - a7) * w3]
        vpand    ymm3, ymm2, ymm4    // ymm3 = masked parts = C0 parts
        vpsrlq   ymm2, ymm2, 12      // ymm2 = C1 parts
        vpslld   ymm1, ymm3, 1       // 2 * C0
        vpaddd   ymm3, ymm3, ymm1    // 3 * C0
        vpsubd   ymm1, ymm3, ymm2    // reduced part = 3 * C0 - C1

// second round:
// ymm0 = [b0 _ b2 _ b4 _ b6 _]
// ymm1 = [b1 _ b3 _ b5 _ b7 _]

        vpmovsxwq xmm5, [r9]         // xmm5 = [U, V]: two multipliers, sign-extended to 64bit
        vpermd    ymm5, ymm6, ymm5   // ymm5 = [U _ U _ | V _ V _]
        
        vshufps ymm2, ymm0, ymm1, 0x44  // ymm2 = [b0 _ b1 _ b4 _ b5 _ ]
        vshufps ymm3, ymm0, ymm1, 0xee  // ymm3 = [b2 _ b3 _ b6 _ b7 _]

        vpsubd  ymm1, ymm2, ymm3    // ymm1 = [b0 - b2 __ b1 - b3 __ b4 - b6 __ b5 - b7 __ ]
        vpaddd  ymm0, ymm2, ymm3    // ymm0 = [b0 + b2 __ b1 + b3 __ b4 + b6 __ b5 + b7 __ ]
        vpmuldq ymm1, ymm1, ymm5    // ymm1 = [(b0 - b2) * U, (b1 - b3) * U, (b4 - b6) * V, (b5 - b7) * V]
        vpand   ymm3, ymm1, ymm4    // ymm3 = C0 parts of ymm1
        vpsrlq  ymm2, ymm1, 12      // ymm2 = C1 parts
        vpslld  ymm1, ymm3, 1       // 2 * C0
        vpaddd  ymm3, ymm3, ymm1    // 3 * C0
        vpsubd  ymm1, ymm3, ymm2    // reduced part = 3 * C0 - C1

// third round:
// ymm0 = [c0 _ c1 _ c4 _ c5 _]
// ymm1 = [c2 _ c3 _ c6 _ c7 _]
        vpbroadcastw xmm5, [r10]    // xmm5 = 8 copies of multiplier U
        vpmovsxwq ymm5, xmm5        // ymm5 = 4 copies of U, sign-extended to 64 bits

        vperm2i128 ymm2, ymm0, ymm1, 0x20  // ymm2 = [c0 _ c1 _ c2 _ c3 _ ]
        vperm2i128 ymm3, ymm0, ymm1, 0x31  // ymm3 = [c4 _ c5 _ c6 _ c7 _ ]

        vpsubd  ymm1, ymm2, ymm3    // ymm1 = [c0 - c4 __ c1 - c5 __ c2 - c6 __ c3 - c7 __ ]
        vpaddd  ymm0, ymm2, ymm3    // ymm0 = [c00 + c4 __ c1 + c5 __ c2 + c6 __ c3 + c7 __ ]
        vpmuldq ymm1, ymm1, ymm5    // ymm1 = [(c0 - c4) * U, (c1 - c5) * U, (c2 - c6) * U, (c3 - c7) * U]
        vpand   ymm3, ymm1, ymm4    // ymm3 = C0 parts of ymm1
        vpsrlq  ymm2, ymm1, 12      // ymm2 = C1 parts
        vpslld  ymm1, ymm3, 1       // 2 * C0
        vpaddd  ymm3, ymm3, ymm1    // 3 * C0
        vpsubd  ymm1, ymm3, ymm2    // reduced part = 3 * C0 - C1
        
// shuffle and merge into ymm0
        vperm2i128 ymm2, ymm0, ymm1, 0x20
        vperm2i128 ymm3, ymm0, ymm1, 0x31
        vshufps    ymm0, ymm2, ymm3, 0x88
        vmovdqu    [rax], ymm0
        
        add rax, 32
        add r8, 8
        add r9, 4
        add r10, 2
        cmp rax, rcx
        jb gs_r2s_loop0


/*
 * Blocks of size 16
 */
        mov rax, rdi
// first block:
//  a'[0 ... 7]  = a[0 ... 7] + a[8 ... 15]
//  a'[8 ... 15] = a[0 ... 7] - a[8 ... 15]     
        vmovdqu ymm0, [rax]
        vmovdqu ymm1, [rax+32]
        vpsubd  ymm2, ymm0, ymm1
        vpaddd  ymm0, ymm0, ymm1
        vmovdqu [rax], ymm0
        vmovdqu [rax+32], ymm2

        add     rax, 64
        cmp     rax, rcx
        jae     gs_r2s_done

// other blocks
// r9 --> multiplier W
        shr    rsi, 1
        lea    r9, [rdx+rsi+2]
gs_r2s_size16_loop:
        vpbroadcastw xmm5, [r9]  // 8 copies of the multiplier W
        vpmovsxwq ymm5, xmm5     // ymm5 = four copies of W, sign-extended to 64 bits

        vmovdqu ymm0, [rax]
        vmovdqu ymm1, [rax+32]
        vpsubd ymm2, ymm0, ymm1  // ymm2 = [a[i] - a[i+8], ..., a[i+7] - a[i+15] ]
        vpaddd ymm0, ymm0, ymm1  // ymm0 = [a[i] + a[i+8], ..., a[i+7] + a[i+15] ]

        vpmuldq  ymm1, ymm2, ymm5   // ymm1 = four products (even indices)
        vpshufd  ymm2, ymm2, 0x31
        vpmuldq  ymm3, ymm2, ymm5   // ymm3 = four products (odd indices)
        vpslldq  ymm2, ymm3, 4
        vpblendd ymm2, ymm2, ymm1, 0x55
        vpand    ymm2, ymm2, ymm4   // ymm2 = C0 part (eight integers)
        
        vpsrlq   ymm1, ymm1, 12
        vpsrlq   ymm3, ymm3, 12
        vpslldq  ymm3, ymm3, 4
        vpblendd ymm1, ymm3, ymm1, 0x55 // ymm1 = C1 part (eight integers)
        vpslld   ymm3, ymm2, 1
        vpaddd   ymm2, ymm2, ymm3   // ymm2 = 3 * C0
        vpsubd   ymm1, ymm2, ymm1

        vmovdqu  [rax], ymm0
        vmovdqu  [rax+32], ymm1

        add     rax, 64
        add     r9, 2
        cmp     rax, rcx
        jb      gs_r2s_size16_loop


/*
 * Blocks of size 32 and more
 */
        mov    r10, rcx         // r10 = end of array a
        mov    r11, 64          // half-block size in bytes = (16 * 4)

gs_r2s_size32_loop:
        mov    rax, rdi         // rax --> start of array a = first block of r11 bytes
        lea    rcx, [rax+r11]   // rcx --> next block
        mov    r8, rcx          // r8 --> end marker
        shr    rsi, 1
        lea    r9, [rdx+rsi]    // r9 --> multiplier table for this block size

gs_r2s_size32_first_blocks:
// for the first two blocks, the multiplier is 1
        vmovdqu ymm0, [rax]     // ymm0 = eight elements of the first block
        vmovdqu ymm1, [rcx]     // ymm1 = eight elements of the second block
        vpsubd  ymm2, ymm0, ymm1
        vpaddd  ymm0, ymm0, ymm1
        vmovdqu [rax], ymm0
        vmovdqu [rcx], ymm2

        add rax, 32
        add rcx, 32
        cmp rax, r8
        jb gs_r2s_size32_first_blocks

        cmp rcx, r10
        jae gs_r2s_done

gs_r2s_size32_other_blocks:
// for the next pairs of blocks, read the multiplier at r9
        add r9, 2
        vpbroadcastw xmm5, [r9]  // 8 copies of the multiplier W
        vpmovsxwq ymm5, xmm5     // ymm5 = four copies of W, sign-extended to 64 bits
        mov  rax, rcx            // rax = start of block
        add  rcx, r11            // rcx = start of the next block
        mov  r8, rcx             // r8 = end of block
gs_r2s_size32_inner_loop:
        vmovdqu ymm0, [rax]      // ymm0 = eight elements of the first block
        vmovdqu ymm1, [rcx]      // ymm1 = eight elements of the second block
        vpsubd  ymm2, ymm0, ymm1
        vpaddd  ymm0, ymm0, ymm1

        // mulreduce ymm2 * ymm5: result in ymm1
        vpmuldq  ymm1, ymm2, ymm5    // ymm1 = four products
        vpshufd  ymm2, ymm2, 0x31
        vpmuldq  ymm3, ymm2, ymm5    // ymm3 = four other products
        vpslldq  ymm2, ymm3, 4
        vpblendd ymm2, ymm2, ymm1, 0x55
        vpand    ymm2, ymm2, ymm4    // ymm2 = C0 part

        vpsrlq   ymm1, ymm1, 12
        vpsrlq   ymm3, ymm3, 12
        vpslldq  ymm3, ymm3, 4
        vpblendd ymm1, ymm3, ymm1, 0x55 // ymm1 = C1 part (eight integers)
        vpslld   ymm3, ymm2, 1
        vpaddd   ymm2, ymm2, ymm3   // ymm2 = 3 * C0
        vpsubd   ymm1, ymm2, ymm1

        vmovdqu  [rax], ymm0
        vmovdqu  [rcx], ymm1

        add rax, 32
        add rcx, 32
        cmp rax, r8
        jb gs_r2s_size32_inner_loop

        cmp rcx, r10
        jb  gs_r2s_size32_other_blocks

        shl r11, 1                // double the block size
        jmp gs_r2s_size32_loop

gs_r2s_done:
        ret


/***************************************************************************
 * Combined NTT and product by powers of psi
 * Gentleman-Sande: bit-reverse to standard order
 *
 * Input:
 * - rdi = start of array a
 * - rsi = size of array a (must be a positive multiple of 16)
 * - rdx = start of array p
 *
 * a is an array of 32bit integers.
 * NTT a is stored in place. 
 * p is a constant array of powers of omega (signed 16bit constants).
 **************************************************************************/

        .balign 16
        .global _G(nttmul_red_gs_rev2std_asm)
_G(nttmul_red_gs_rev2std_asm):
        lea     rcx, [rdi+4*rsi]      // rcx -> end of array a
        lea     r8, [rdx+rsi]         // r8 --> multipliers for round 1
        shr     rsi, 1
        lea     r9, [rdx+rsi]         // r9 --> multipliers for round 2
        shr     rsi, 1
        lea     r10, [rdx+rsi]        // r10 --> multipliers for round 3
        mov     rax, rdi              // rax -> start of array a

        vmovdqa ymm4, [mask+rip]
        vmovdqa ymm6, [perm2020+rip]

/*
 * The first loop is the same as in ntt_red_gs_rev2std.
 * It processes blocks of eight integers.
 * Each iteration corresponds to three rounds.
 */
mgs_r2s_loop0:
// first round
        vpmovsxwq ymm5, [r8]         // ymm5 = 4 multipliers = [w0, w1, w2, w3] extended to 64 bits

        vmovdqu  ymm0, [rax]         // ymm0 = a0 a1 a2 a3 a4 a5 a6 a7
        vpsrldq  ymm1, ymm0, 4       // ymm1 = a1 a2 a3 0  a5 a6 a7 0
        vpsubd   ymm2, ymm0, ymm1    // ymm2 = [a0 - a1 __ a2 - a3 __ a4 - a5 __ a6 - a7 __ ]
        vpaddd   ymm0, ymm0, ymm1    // ymm0 = [a0 + a1 __ a2 + a3 __ a4 + a5 __ a6 + a7 __ ]
        vpmuldq  ymm2, ymm2, ymm5    // ymm2 = [(a0 - a1) * w0, (a2 - a3) * w1, (a4 - a5) * w2, (a6 - a7) * w3]
        vpand    ymm3, ymm2, ymm4    // ymm3 = masked parts = C0 parts
        vpsrlq   ymm2, ymm2, 12      // ymm2 = C1 parts
        vpslld   ymm1, ymm3, 1       // 2 * C0
        vpaddd   ymm3, ymm3, ymm1    // 3 * C0
        vpsubd   ymm1, ymm3, ymm2    // reduced part = 3 * C0 - C1

// second round:
// ymm0 = [b0 _ b2 _ b4 _ b6 _]
// ymm1 = [b1 _ b3 _ b5 _ b7 _]

        vpmovsxwq xmm5, [r9]         // xmm5 = [U, V]: two multipliers, sign-extended to 64bit
        vpermd    ymm5, ymm6, ymm5   // ymm5 = [U _ U _ | V _ V _]
        
        vshufps ymm2, ymm0, ymm1, 0x44  // ymm2 = [b0 _ b1 _ b4 _ b5 _ ]
        vshufps ymm3, ymm0, ymm1, 0xee  // ymm3 = [b2 _ b3 _ b6 _ b7 _]

        vpsubd  ymm1, ymm2, ymm3    // ymm1 = [b0 - b2 __ b1 - b3 __ b4 - b6 __ b5 - b7 __ ]
        vpaddd  ymm0, ymm2, ymm3    // ymm0 = [b0 + b2 __ b1 + b3 __ b4 + b6 __ b5 + b7 __ ]
        vpmuldq ymm1, ymm1, ymm5    // ymm1 = [(b0 - b2) * U, (b1 - b3) * U, (b4 - b6) * V, (b5 - b7) * V]
        vpand   ymm3, ymm1, ymm4    // ymm3 = C0 parts of ymm1
        vpsrlq  ymm2, ymm1, 12      // ymm2 = C1 parts
        vpslld  ymm1, ymm3, 1       // 2 * C0
        vpaddd  ymm3, ymm3, ymm1    // 3 * C0
        vpsubd  ymm1, ymm3, ymm2    // reduced part = 3 * C0 - C1

// third round:
// ymm0 = [c0 _ c1 _ c4 _ c5 _]
// ymm1 = [c2 _ c3 _ c6 _ c7 _]
        vpbroadcastw xmm5, [r10]    // xmm5 = 8 copies of multiplier U
        vpmovsxwq ymm5, xmm5        // ymm5 = 4 copies of U, sign-extended to 64 bits

        vperm2i128 ymm2, ymm0, ymm1, 0x20  // ymm2 = [c0 _ c1 _ c2 _ c3 _ ]
        vperm2i128 ymm3, ymm0, ymm1, 0x31  // ymm3 = [c4 _ c5 _ c6 _ c7 _ ]

        vpsubd  ymm1, ymm2, ymm3    // ymm1 = [c0 - c4 __ c1 - c5 __ c2 - c6 __ c3 - c7 __ ]
        vpaddd  ymm0, ymm2, ymm3    // ymm0 = [c00 + c4 __ c1 + c5 __ c2 + c6 __ c3 + c7 __ ]
        vpmuldq ymm1, ymm1, ymm5    // ymm1 = [(c0 - c4) * U, (c1 - c5) * U, (c2 - c6) * U, (c3 - c7) * U]
        vpand   ymm3, ymm1, ymm4    // ymm3 = C0 parts of ymm1
        vpsrlq  ymm2, ymm1, 12      // ymm2 = C1 parts
        vpslld  ymm1, ymm3, 1       // 2 * C0
        vpaddd  ymm3, ymm3, ymm1    // 3 * C0
        vpsubd  ymm1, ymm3, ymm2    // reduced part = 3 * C0 - C1
        
// shuffle and merge into ymm0
        vperm2i128 ymm2, ymm0, ymm1, 0x20
        vperm2i128 ymm3, ymm0, ymm1, 0x31
        vshufps    ymm0, ymm2, ymm3, 0x88
        vmovdqu    [rax], ymm0
        
        add rax, 32
        add r8, 8
        add r9, 4
        add r10, 2
        cmp rax, rcx
        jb mgs_r2s_loop0

/*
 * Blocks of size 16
 */
        mov    rax, rdi
        shr    rsi, 1
        lea    r9, [rdx+rsi]      // r9 --> multipliers for this round
mgs_r2s_size16_loop:
        vpbroadcastw xmm5, [r9]  // 8 copies of the multiplier W
        vpmovsxwq ymm5, xmm5     // ymm5 = four copies of W, sign-extended to 64 bits

        vmovdqu ymm0, [rax]
        vmovdqu ymm1, [rax+32]
        vpsubd ymm2, ymm0, ymm1  // ymm2 = [a[i] - a[i+8], ..., a[i+7] - a[i+15] ]
        vpaddd ymm0, ymm0, ymm1  // ymm0 = [a[i] + a[i+8], ..., a[i+7] + a[i+15] ]

        vpmuldq  ymm1, ymm2, ymm5   // ymm1 = four products (even indices)
        vpshufd  ymm2, ymm2, 0x31
        vpmuldq  ymm3, ymm2, ymm5   // ymm3 = four products (odd indices)
        vpslldq  ymm2, ymm3, 4
        vpblendd ymm2, ymm2, ymm1, 0x55
        vpand    ymm2, ymm2, ymm4   // ymm2 = C0 part (eight integers)
        
        vpsrlq   ymm1, ymm1, 12
        vpsrlq   ymm3, ymm3, 12
        vpslldq  ymm3, ymm3, 4
        vpblendd ymm1, ymm3, ymm1, 0x55 // ymm1 = C1 part (eight integers)
        vpslld   ymm3, ymm2, 1
        vpaddd   ymm2, ymm2, ymm3   // ymm2 = 3 * C0
        vpsubd   ymm1, ymm2, ymm1

        vmovdqu  [rax], ymm0
        vmovdqu  [rax+32], ymm1

        add     rax, 64
        add     r9, 2
        cmp     rax, rcx
        jb      mgs_r2s_size16_loop
        
        cmp     rsi, 2
        je      mgs_r2s_done
        
/*
 * Blocks of size 32 and more
 */
        mov    r10, rcx         // r10 = end of array a
        mov    r11, 64          // half-block size in bytes = (16 * 4)

mgs_r2s_size32_loop:
        mov    rax, rdi         // rax --> start of array a = first block of r11 bytes
        lea    rcx, [rax+r11]   // rcx --> next block
        mov    r8, rcx          // r8 --> end marker
        shr    rsi, 1
        lea    r9, [rdx+rsi]    // r9 --> multiplier table for this block size

mgs_r2s_size32_blocks:
        vpbroadcastw xmm5, [r9]  // 8 copies of the multiplier W
        vpmovsxwq ymm5, xmm5     // ymm5 = four copies of W, sign-extended to 64 bits

mgs_r2s_size32_inner_loop:
        vmovdqu ymm0, [rax]      // ymm0 = eight elements of the first block
        vmovdqu ymm1, [rcx]      // ymm1 = eight elements of the second block
        vpsubd  ymm2, ymm0, ymm1
        vpaddd  ymm0, ymm0, ymm1

        // mulreduce ymm2 * ymm5: result in ymm1
        vpmuldq  ymm1, ymm2, ymm5    // ymm1 = four products
        vpshufd  ymm2, ymm2, 0x31
        vpmuldq  ymm3, ymm2, ymm5    // ymm3 = four other products
        vpslldq  ymm2, ymm3, 4
        vpblendd ymm2, ymm2, ymm1, 0x55
        vpand    ymm2, ymm2, ymm4    // ymm2 = C0 part

        vpsrlq   ymm1, ymm1, 12
        vpsrlq   ymm3, ymm3, 12
        vpslldq  ymm3, ymm3, 4
        vpblendd ymm1, ymm3, ymm1, 0x55 // ymm1 = C1 part (eight integers)
        vpslld   ymm3, ymm2, 1
        vpaddd   ymm2, ymm2, ymm3   // ymm2 = 3 * C0
        vpsubd   ymm1, ymm2, ymm1

        vmovdqu  [rax], ymm0
        vmovdqu  [rcx], ymm1

        add rax, 32
        add rcx, 32
        cmp rax, r8
        jb  mgs_r2s_size32_inner_loop

        mov  rax, rcx            // rax = start of block
        add  rcx, r11            // rcx = start of the next block
        mov  r8, rcx             // r8 = end of block
        add  r9, 2
        cmp  rax, r10
        jb   mgs_r2s_size32_blocks

        shl r11, 1                // double the block size
        cmp rsi, 2
        jne mgs_r2s_size32_loop

mgs_r2s_done:
        ret


/***************************************************************************
 * NTT using Gentleman-Sande: standard to bit-reverse order
 *
 * Input:
 * - rdi = start of array a
 * - rsi = size of array a (must be a positive multiple of 16)
 * - rdx = start of array p
 *
 * a is an array of 32bit integers.
 * NTT a is stored in place. 
 * p is a constant array of powers of omega (signed 16bit constants).
 **************************************************************************/

        .balign 16
        .global _G(ntt_red_gs_std2rev_asm)
_G(ntt_red_gs_std2rev_asm):
	lea      r11, [rdi+4*rsi]        // end of array a
	vmovdqa  ymm4, [mask+rip]        // bitmask
/*
 * main loop: as long as the block size is at least 16
 * In this loop
 *   rsi = block size (number of 32bit integers in a block)
 *   rdi = start of array a
 *   rdx = start of array p
 *   r11 = end of array a
 * The toplevel iteration reads 8 multipliers in ymm5
 */
gs_s2r_main:
	mov    r8, rdi
	lea    r9, [rdx+rsi]          // start of the multiplier arrays for that size
	lea    r10, [r8+2*rsi]        // end of the first half block
gs_s2r_loop:
	vpmovsxwd  ymm5, [r9]
	vpshufd    ymm6, ymm5, 0x31
	mov    rax, r8
	lea    rcx, [r8+2*rsi]
gs_s2r_inner_loop:
/*
 * In this loop
 *  rax --> eight integers in block i
 *  rcx --> eight integers in block i+1
 *  rcx = rax + 2*rsi
 *  ymm5 = four multipliers  (indices 0, 2, 4, 6)
 *  ymm6 = four multipliers  (indides 1, 3, 5, 7)
 */
	vmovdqu    ymm0, [rax]
	vmovdqu    ymm1, [rcx]
	vpaddd     ymm2, ymm0, ymm1
	vpsubd     ymm0, ymm0, ymm1

	// mul-reduce ymm0 and ymm5/ymm6, result in ymm0
	vpmuldq    ymm1, ymm0, ymm5      // ymm1 = four products (64bit integers)
	vpshufd    ymm0, ymm0, 0x31
	vpmuldq    ymm3, ymm0, ymm6      // ymm3 = four other products

	vpslldq    ymm0, ymm3, 4
	vpblendd   ymm0, ymm0, ymm1, 0x55
	vpand      ymm0, ymm0, ymm4     // ymm0 = C0 part (eight 32bit integers)

	vpsrlq     ymm3, ymm3, 12
	vpsrlq     ymm1, ymm1, 12
	vpslldq    ymm3, ymm3, 4
	vpblendd   ymm1, ymm3, ymm1, 0x55 // ymm1 = C1 part (eight 32bit integers)

	vpslld     ymm3, ymm0, 1
	vpaddd     ymm0, ymm0, ymm3     // 3 * C0
	vpsubd     ymm0, ymm0, ymm1     // reduced form

	vmovdqu    [rax], ymm2
	vmovdqu    [rcx], ymm0

	lea        rax, [rax+4*rsi]
	lea        rcx, [rcx+4*rsi]
	cmp        rax, r11
	jb         gs_s2r_inner_loop

	add	   r8, 32
	add        r9, 16
	cmp        r8, r10
	jb         gs_s2r_loop

	shr	   rsi, 1             // next block size = rsi/2
	cmp        rsi, 16
	jae	   gs_s2r_main        

/*
 * Three last rounds
 *
 * first pass:  four multipliers in p[4 .. 7]
 * second pass: two multipliers in  p[2 .. 3]
 * last pass:   one multiplier  in  p[1]
 *
 * We know that p[2] and p[1] are equal to inverse(3) so we
 * the mulreduce with p[2] or p[1] is the identity (i.e., we skip this).
 * This is also true for p[4] but we keep p[4] in ymm5 to multiply
 * by four coefficients in parallel.
 */
	vpmovsxwd    ymm5, [rdx]           // ymm5 = p[0 ... 7]
	vpsrldq      ymm7, ymm5, 12
	vpbroadcastq ymm7, xmm7            // ymm7 = four copies of p[3]

	vperm2i128   ymm5, ymm5, ymm5, 0x11  // ymm5 = p[4] p[5] p[6] p[7] p[4] p[5] p[6] p[7]
	vpshufd      ymm6, ymm5, 0x31        // ymm6 = p[5] ---  p[7]  ---  p[5]  ---  p[7]  ---

	mov          rax, rdi                  // start of array a
	vmovdqa      ymm9, [perm04152637+rip]  // permutation for final shuffle

gs_s2r_finish_loop:
// each iteration applies three passes to 16 elements of array a
	vmovdqu      ymm0, [rax]           // a[0 ... 7]
	vmovdqu      ymm1, [rax+32]        // a[8 ... 15]

// first pass
	vperm2i128   ymm2, ymm0, ymm1, 0x20     // ymm2 = a[0 .. 3] a[8 .. 11]
	vperm2i128   ymm3, ymm0, ymm1, 0x31     // ymm3 = a[4 .. 7] a[12 .. 15]
	vpaddd       ymm0, ymm2, ymm3
	vpsubd       ymm1, ymm2, ymm3

	// mulreduce ymm1 by ymm5/ymm6, result in ymm1
	vpmuldq      ymm2, ymm1, ymm5     // ymm2 = four products
	vpshufd      ymm1, ymm1, 0x31
	vpmuldq      ymm3, ymm1, ymm6     // ymm3 = four other products

	vpslldq      ymm1, ymm3, 4
	vpblendd     ymm1, ymm1, ymm2, 0x55
	vpand        ymm1, ymm1, ymm4     // ymm1 = C0 part: eight 32bit integers

	vpsrlq       ymm3, ymm3, 12
	vpsrlq       ymm2, ymm2, 12
	vpslldq      ymm3, ymm3, 4
	vpblendd     ymm2, ymm3, ymm2, 0x55 // ymm2 = C1 part: eight 32bit integers

	vpslld       ymm3, ymm1, 1
	vpaddd       ymm1, ymm1, ymm3     // 3 * C0
	vpsubd       ymm1, ymm1, ymm2     // 3 * C0 - C1

/*
 * Second pass:
 * ymm0 contains b[0 .. 3] b[8 .. 11]
 * ymm1 contains b[4 .. 7] b[12 .. 15]
 */
	vshufpd      ymm2, ymm0, ymm1, 0x00  // ymm2 = b[0 1] b[4 5] b[8 9]   b[12 13]
	vshufpd      ymm3, ymm0, ymm1, 0x0f  // ymm3 = b[2 3] b[6 7] b[10 11] b[14 15]
	vpaddd       ymm0, ymm2, ymm3
	vpsubd       ymm1, ymm2, ymm3

	// mulreduce half of ymm1 by ymm7
	vpshufd      ymm2, ymm1, 0x31
	vpmuldq      ymm2, ymm2, ymm7        // ymm2 = four products
	vpand        ymm3, ymm2, ymm4        // C0 part (four 32bit integers)
	vpsrlq       ymm2, ymm2, 12          // C1 part
	vpslld       ymm8, ymm3, 1
	vpaddd       ymm3, ymm3, ymm8        // 3 * C0
	vpsubd       ymm3, ymm3, ymm2        // 3 * C0 - C1

	vpslldq      ymm3, ymm3, 4
	vpblendd     ymm1, ymm3, ymm1, 0x55

/*
 * Third pass:
 * ymm0 contains c[0 1] c[4 5] c[8 9]   c[12 13]
 * ymm1 contains c[2 3] c[6 7] c[10 11] c[14 15]
 */
	vpslldq      ymm2, ymm1, 4
	vpblendd     ymm2, ymm2, ymm0, 0x55 // ymm2 = c[0] c[2] c[4] c[6] c[8] c[10] c[12] c[14]
	vpsrldq      ymm3, ymm0, 4
	vpblendd     ymm3, ymm1, ymm3, 0x55 // ymm3 = c[1] c[3] c[5] c[7] c[9] c[11] c[13] c[15]

	vpaddd       ymm0, ymm2, ymm3
	vpsubd       ymm1, ymm2, ymm3

/*
 * Shuffle and store
 * ymm0 contains d[0] d[2] d[4] d[6] d[8] d[10] d[12] d[14]
 * ymm1 contains d[1] d[3] d[5] d[7] d[9] d[11] d[13] d[15]
 */
	vperm2i128   ymm2, ymm0, ymm1, 0x20    // ymm2 = d[0] d[2] d[4] d[6] d[1] d[3] d[5] d[7]
	vperm2i128   ymm3, ymm0, ymm1, 0x31    // ymm3 = d[8] d[10] d[12] d[14] d[9] d[11] d[13] d[15]
	vpermd       ymm0, ymm9, ymm2
	vpermd       ymm1, ymm9, ymm3
	
	vmovdqu      [rax], ymm0
	vmovdqu      [rax+32], ymm1
	
	add	     rax, 64
	cmp          rax, r11
	jb           gs_s2r_finish_loop
	
	ret


/***************************************************************************
 * Combined NTT and product by powers of psi using Gentleman-Sande
 * standard to bit-reverse order
 *
 * Input:
 * - rdi = start of array a
 * - rsi = size of array a (must be a positive multiple of 16)
 * - rdx = start of array p
 *
 * a is an array of 32bit integers.
 * NTT a is stored in place. 
 * p is a constant array of powers of omega (signed 16bit constants).
 **************************************************************************/

        .balign 16
        .global _G(nttmul_red_gs_std2rev_asm)
_G(nttmul_red_gs_std2rev_asm):
	lea      r11, [rdi+4*rsi]        // end of array a
	vmovdqa  ymm4, [mask+rip]        // bitmask
/*
 * Same main loop as in ntt_red_gs_std2rev
 */
mgs_s2r_main:
	mov    r8, rdi
	lea    r9, [rdx+rsi]          // start of the multiplier arrays for that size
	lea    r10, [r8+2*rsi]        // end of the first half block
mgs_s2r_loop:
	vpmovsxwd  ymm5, [r9]
	vpshufd    ymm6, ymm5, 0x31
	mov    rax, r8
	lea    rcx, [r8+2*rsi]
mgs_s2r_inner_loop:
/*
 * In this loop
 *  rax --> eight integers in block i
 *  rcx --> eight integers in block i+1
 *  rcx = rax + 2*rsi
 *  ymm5 = four multipliers  (indices 0, 2, 4, 6)
 *  ymm6 = four multipliers  (indides 1, 3, 5, 7)
 */
	vmovdqu    ymm0, [rax]
	vmovdqu    ymm1, [rcx]
	vpaddd     ymm2, ymm0, ymm1
	vpsubd     ymm0, ymm0, ymm1

	// mul-reduce ymm0 and ymm5/ymm6, result in ymm0
	vpmuldq    ymm1, ymm0, ymm5      // ymm1 = four products (64bit integers)
	vpshufd    ymm0, ymm0, 0x31
	vpmuldq    ymm3, ymm0, ymm6      // ymm3 = four other products

	vpslldq    ymm0, ymm3, 4
	vpblendd   ymm0, ymm0, ymm1, 0x55
	vpand      ymm0, ymm0, ymm4     // ymm0 = C0 part (eight 32bit integers)

	vpsrlq     ymm3, ymm3, 12
	vpsrlq     ymm1, ymm1, 12
	vpslldq    ymm3, ymm3, 4
	vpblendd   ymm1, ymm3, ymm1, 0x55 // ymm1 = C1 part (eight 32bit integers)

	vpslld     ymm3, ymm0, 1
	vpaddd     ymm0, ymm0, ymm3     // 3 * C0
	vpsubd     ymm0, ymm0, ymm1     // reduced form

	vmovdqu    [rax], ymm2
	vmovdqu    [rcx], ymm0

	lea        rax, [rax+4*rsi]
	lea        rcx, [rcx+4*rsi]
	cmp        rax, r11
	jb         mgs_s2r_inner_loop

	add	   r8, 32
	add        r9, 16
	cmp        r8, r10
	jb         mgs_s2r_loop

	shr	   rsi, 1             // next block size = rsi/2
	cmp        rsi, 16
	jae	   mgs_s2r_main        

/*
 * Three last rounds
 *
 * first pass:  four multipliers in p[4 .. 7]
 * second pass: two multipliers in  p[2 .. 3]
 * last pass:   one multiplier  in  p[1]
 *
 * We load the multipliers in ymm5 to ymm9:
 *  ymm5 = p[4] _ p[6] _ p[4] _ p[6] _
 *  ymm6 = p[5] _ p[7] _ p[5] _ p[7] _
 * 
 *  ymm7 = p[2] _ p[2] _ p[2] _ p[2] _
 *  ymm8 = p[3] _ p[3] _ p[3] _ p[3] _
 *
 *  ymm9 = p[1] _ p[1] _ p[1] _ p[1] _
 */
	vpmovsxwd    ymm0, [rdx]           // ymm0 = p[0 ... 7]
	vmovdqa	     ymm9, [perm_bdcst1+rip]
	vpermd       ymm9, ymm9, ymm0      // ymm9 = four copies of p[1]
	vmovdqa      ymm8, [perm_bdcst3+rip]
	vpermd       ymm8, ymm8, ymm0      // ymm8 = four copies of p[3]
	vmovdqa      ymm7, [perm_bdcst2+rip]
	vpermd       ymm7, ymm7, ymm0      // ymm7 = four copies of p[2]
	vmovdqa      ymm6, [perm5070+rip]
	vpermd       ymm6, ymm6, ymm0      // ymm6 = p[5] _ p[7] _ p[5] _ p[7]
	vmovdqa      ymm5, [perm4060+rip]
	vpermd       ymm5, ymm5, ymm0      // ymm5 = p[4] _ p[6] _ p[4] _ p[6]

	mov	     rax, rdi
	vmovdqa      ymm10, [perm04152637+rip]  // permutation for final shuffle

mgs_s2r_finish_loop:
// each iteration applies three passes to 16 elements of array a
	vmovdqu      ymm0, [rax]           // a[0 ... 7]
	vmovdqu      ymm1, [rax+32]        // a[8 ... 15]

// first pass
	vperm2i128   ymm2, ymm0, ymm1, 0x20     // ymm2 = a[0 .. 3] a[8 .. 11]
	vperm2i128   ymm3, ymm0, ymm1, 0x31     // ymm3 = a[4 .. 7] a[12 .. 15]
	vpaddd       ymm0, ymm2, ymm3
	vpsubd       ymm1, ymm2, ymm3

	// mulreduce ymm1 by ymm5/ymm6, result in ymm1
	vpmuldq      ymm2, ymm1, ymm5     // ymm2 = four products
	vpshufd      ymm1, ymm1, 0x31
	vpmuldq      ymm3, ymm1, ymm6     // ymm3 = four other products

	vpslldq      ymm1, ymm3, 4
	vpblendd     ymm1, ymm1, ymm2, 0x55
	vpand        ymm1, ymm1, ymm4     // ymm1 = C0 part: eight 32bit integers

	vpsrlq       ymm3, ymm3, 12
	vpsrlq       ymm2, ymm2, 12
	vpslldq      ymm3, ymm3, 4
	vpblendd     ymm2, ymm3, ymm2, 0x55 // ymm2 = C1 part: eight 32bit integers

	vpslld       ymm3, ymm1, 1
	vpaddd       ymm1, ymm1, ymm3     // 3 * C0
	vpsubd       ymm1, ymm1, ymm2     // 3 * C0 - C1

/*
 * Second pass:
 * ymm0 contains b[0 .. 3] b[8 .. 11]
 * ymm1 contains b[4 .. 7] b[12 .. 15]
 */
	vshufpd      ymm2, ymm0, ymm1, 0x00  // ymm2 = b[0 1] b[4 5] b[8 9]   b[12 13]
	vshufpd      ymm3, ymm0, ymm1, 0x0f  // ymm3 = b[2 3] b[6 7] b[10 11] b[14 15]
	vpaddd       ymm0, ymm2, ymm3
	vpsubd       ymm1, ymm2, ymm3

	// mulreduce ymm1 by ymm7/ymm8, result in ymm1
	vpmuldq      ymm2, ymm1, ymm7     // ymm2 = four products
	vpshufd      ymm1, ymm1, 0x31
	vpmuldq      ymm3, ymm1, ymm8     // ymm3 = four other products

	vpslldq      ymm1, ymm3, 4
	vpblendd     ymm1, ymm1, ymm2, 0x55
	vpand        ymm1, ymm1, ymm4     // ymm1 = C0 part: eight 32bit integers

	vpsrlq       ymm3, ymm3, 12
	vpsrlq       ymm2, ymm2, 12
	vpslldq      ymm3, ymm3, 4
	vpblendd     ymm2, ymm3, ymm2, 0x55 // ymm2 = C1 part: eight 32bit integers

	vpslld       ymm3, ymm1, 1
	vpaddd       ymm1, ymm1, ymm3     // 3 * C0
	vpsubd       ymm1, ymm1, ymm2     // 3 * C0 - C1

/*
 * Third pass:
 * ymm0 contains c[0 1] c[4 5] c[8 9]   c[12 13]
 * ymm1 contains c[2 3] c[6 7] c[10 11] c[14 15]
 */
	vpslldq      ymm2, ymm1, 4
	vpblendd     ymm2, ymm2, ymm0, 0x55 // ymm2 = c[0] c[2] c[4] c[6] c[8] c[10] c[12] c[14]
	vpsrldq      ymm3, ymm0, 4
	vpblendd     ymm3, ymm1, ymm3, 0x55 // ymm3 = c[1] c[3] c[5] c[7] c[9] c[11] c[13] c[15]
	vpaddd       ymm0, ymm2, ymm3
	vpsubd       ymm1, ymm2, ymm3

	// mulreduce ymm1 by ymm9, result in ymm1
	vpmuldq      ymm2, ymm1, ymm9    // ymm2 = four products
	vpshufd      ymm1, ymm1, 0x31
	vpmuldq      ymm3, ymm1, ymm9     // ymm3 = four other products

	vpslldq      ymm1, ymm3, 4
	vpblendd     ymm1, ymm1, ymm2, 0x55
	vpand        ymm1, ymm1, ymm4     // ymm1 = C0 part: eight 32bit integers

	vpsrlq       ymm3, ymm3, 12
	vpsrlq       ymm2, ymm2, 12
	vpslldq      ymm3, ymm3, 4
	vpblendd     ymm2, ymm3, ymm2, 0x55 // ymm2 = C1 part: eight 32bit integers

	vpslld       ymm3, ymm1, 1
	vpaddd       ymm1, ymm1, ymm3     // 3 * C0
	vpsubd       ymm1, ymm1, ymm2     // 3 * C0 - C1
	
/*
 * Shuffle and store
 * ymm0 contains d[0] d[2] d[4] d[6] d[8] d[10] d[12] d[14]
 * ymm1 contains d[1] d[3] d[5] d[7] d[9] d[11] d[13] d[15]
 */
	vperm2i128   ymm2, ymm0, ymm1, 0x20    // ymm2 = d[0] d[2] d[4] d[6] d[1] d[3] d[5] d[7]
	vperm2i128   ymm3, ymm0, ymm1, 0x31    // ymm3 = d[8] d[10] d[12] d[14] d[9] d[11] d[13] d[15]
	vpermd       ymm0, ymm10, ymm2
	vpermd       ymm1, ymm10, ymm3
	
	vmovdqu      [rax], ymm0
	vmovdqu      [rax+32], ymm1
	
	add	     rax, 64
	cmp          rax, r11
	jb           mgs_s2r_finish_loop
	
	ret
