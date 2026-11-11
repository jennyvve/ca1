	.file	"roman.c"
	.option nopic
	.attribute arch, "rv64i2p1_m2p0_a2p1_f2p2_d2p2_zicsr2p0_zmmul1p0_zaamo1p0_zalrsc1p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	2
	.globl	print_roman
	.type	print_roman, @function
print_roman:
	lui	a4,%hi(.LANCHOR0)
	addi	a4,a4,%lo(.LANCHOR0)
.L2:
	lw	a5,0(a4)
	bgt	a5,a0,.L5
.L3:
	lbu	a5,4(a4)
	sb	a5,512(zero)
	lw	a5,0(a4)
	subw	a0,a0,a5
	ble	a5,a0,.L3
.L5:
	lw	a5,8(a4)
	addi	a4,a4,8
	bne	a5,zero,.L2
	li	a5,10
	sb	a5,512(zero)
	ret
	.size	print_roman, .-print_roman
	.section	.rodata
	.align	3
	.set	.LANCHOR0,. + 0
	.type	roman_table, @object
	.size	roman_table, 64
roman_table:
	.word	1000
	.byte	77
	.zero	3
	.word	500
	.byte	68
	.zero	3
	.word	100
	.byte	67
	.zero	3
	.word	50
	.byte	76
	.zero	3
	.word	10
	.byte	88
	.zero	3
	.word	5
	.byte	86
	.zero	3
	.word	1
	.byte	73
	.zero	3
	.word	0
	.byte	0
	.zero	3
	.ident	"GCC: (g6afcc4f6d) 16.1.0"
	.section	.note.GNU-stack,"",@progbits
