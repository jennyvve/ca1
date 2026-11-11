
test/application/basic.bin:     file format elf64-littleriscv


Disassembly of section .text:

00000000000100b0 <_start>:
   100b0:	02600093          	li	ra,38
   100b4:	00408113          	addi	sp,ra,4
   100b8:	f0b08193          	addi	gp,ra,-245
   100bc:	40110233          	sub	tp,sp,ra
   100c0:	004082b3          	add	t0,ra,tp
   100c4:	00000013          	nop
   100c8:	00000013          	nop
   100cc:	00000013          	nop
   100d0:	00000013          	nop
   100d4:	00000013          	nop
   100d8:	ddffccff          	.word	0xddffccff
   100dc:	00000013          	nop
   100e0:	00000013          	nop
