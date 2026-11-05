	.file	"hellof.c"
	.option nopic
	.attribute arch, "rv64i2p1_m2p0_a2p1_f2p2_d2p2_zicsr2p0_zmmul1p0_zaamo1p0_zalrsc1p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align	3
.LC0:
	.string	"hello"
	.text
	.align	2
	.globl	main
	.type	main, @function
main:
	lui	a5,%hi(.LC0)
	addi	a5,a5,%lo(.LC0)
	li	a4,104
	lui	a3,%hi(.LC0+5)
	addi	a3,a3,%lo(.LC0+5)
.L2:
	sb	a4,512(zero)
	addi	a5,a5,1
	lbu	a4,0(a5)
	bne	a5,a3,.L2
	li	a5,10
	sb	a5,512(zero)
	li	a0,0
	ret
	.size	main, .-main
	.ident	"GCC: (g6afcc4f6d) 16.1.0"
	.section	.note.GNU-stack,"",@progbits
