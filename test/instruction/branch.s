	.text
        .align 4
	.globl	_start
	.type	_start, @function
_start:
            beq     x1,x1,skip_beq
	        addi 	x2,x2,1
skip_beq:   bne     x1,x3,skip_bne
	        addi 	x2,x2,1
skip_bne:   blt     x1,x3,skip_blt
	        addi 	x2,x2,1
skip_blt:   bge     x4,x3,skip_bge
	        addi 	x2,x2,1
skip_bge:   bltu    x1,x3,skip_bltu
	        addi 	x2,x2,1
skip_bltu:  bgeu    x4,x3,skip_bgeu
	        addi 	x2,x2,1
skip_bgeu:  nop
    	    nop
            nop
            nop
            nop
            nop
            .word	0xddffccff
            .size	_start, .-_start

