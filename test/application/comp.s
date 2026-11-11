	.file	"comp.c"
	.option nopic
	.attribute arch, "rv64i2p1_m2p0_a2p1_f2p2_d2p2_zicsr2p0_zmmul1p0_zaamo1p0_zalrsc1p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.section	.text.startup,"ax",@progbits
	.align	2
	.globl	main
	.type	main, @function
main:
	lui	a1,%hi(.LANCHOR0)
	addi	a1,a1,%lo(.LANCHOR0)
	addi	sp,sp,-16
	sd	ra,8(sp)
	mv	a4,a1
	mv	a3,a1
	li	a5,0
	li	a2,20
.L2:
	sw	a5,0(a3)
	addiw	a5,a5,1
	addi	a3,a3,4
	bne	a5,a2,.L2
	addi	a3,a1,84
	addi	a2,a1,76
.L3:
	lw	a5,0(a4)
	addi	a4,a4,4
	addi	a3,a3,4
	addiw	a5,a5,42
	sw	a5,-4(a3)
	bne	a4,a2,.L3
	lw	a0,156(a1)
	call	print_roman
	ld	ra,8(sp)
	li	a0,0
	addi	sp,sp,16
	jr	ra
	.size	main, .-main
	.globl	B
	.globl	A
	.bss
	.align	3
	.set	.LANCHOR0,. + 0
	.type	B, @object
	.size	B, 80
B:
	.zero	80
	.type	A, @object
	.size	A, 80
A:
	.zero	80
	.ident	"GCC: (g6afcc4f6d) 16.1.0"
	.section	.note.GNU-stack,"",@progbits
