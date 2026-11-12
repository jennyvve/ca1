
test/application/triangle.bin:     file format elf64-littleriscv


Disassembly of section .text:

00000000000100b0 <_start>:
   100b0:	20000513          	li	a0,512
   100b4:	07800593          	li	a1,120
   100b8:	00100613          	li	a2,1

00000000000100bc <print_line>:
   100bc:	00000693          	li	a3,0

00000000000100c0 <print_char>:
   100c0:	00b50023          	sb	a1,0(a0)
   100c4:	0016869b          	addiw	a3,a3,1
   100c8:	fec6cce3          	blt	a3,a2,100c0 <print_char>
   100cc:	00a00813          	li	a6,10
   100d0:	01050023          	sb	a6,0(a0)
   100d4:	00800813          	li	a6,8
   100d8:	0016061b          	addiw	a2,a2,1
   100dc:	fec850e3          	bge	a6,a2,100bc <print_line>
   100e0:	00000013          	nop
   100e4:	00000013          	nop
   100e8:	00000013          	nop
   100ec:	00000013          	nop
   100f0:	00000013          	nop
   100f4:	ddffccff          	.word	0xddffccff
   100f8:	00000013          	nop
   100fc:	00000013          	nop
   10100:	00000013          	nop
