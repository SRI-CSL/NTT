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

// BD: attempt to get the code working on both MacOSX and Linux
// On MacOS we need to prefix all global symbols with an underscore
#if defined(__APPLE__)
#define _G(s) _##s
#else
#define _G(s) s
#endif

	.intel_syntax noprefix

	.text
	.balign 16
/*
 * Check whether the processor + OS support AVX and AVX2
 *
 * This follows the intel manual.
 *
 * No input parameters.
 * - return with rax = 1 if AVX2 is supported
 * - return with rax = 0 otherwise
 */
	.global _G(avx2_supported)
_G(avx2_supported):
	push rbx		// rax/rbx/rcx/rdx are modified by CPUID
	mov eax, 1
	cpuid
	and ecx, 018000000H
	cmp ecx, 018000000H
	jne not_supported
	mov eax, 7
	mov ecx, 0
	cpuid
	and ebx, 20H
	cmp ebx, 20H
	jne not_supported
	mov ecx, 0
	xgetbv
	and eax, 06H
	cmp eax, 06H
	jne not_supported
	mov eax, 1           	// all good: supported
	jmp done
not_supported:
	mov eax, 0
done:
	pop rbx
	ret
	
	
