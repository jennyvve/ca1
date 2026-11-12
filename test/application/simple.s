
test/application/simple.bin:     file format elf64-littleriscv


Disassembly of section .text:

00000000000100b0 <_start>:
   100b0:	000260b7          	lui	ra,0x26
   100b4:	c0108193          	addi	gp,ra,-1023 # 25c01 <__global_pointer$+0x1431d>
   100b8:	7ffff2b7          	lui	t0,0x7ffff
   100bc:	005282b3          	add	t0,t0,t0
   100c0:	005282b3          	add	t0,t0,t0
   100c4:	0162831b          	addiw	t1,t0,22 # 7ffff016 <__global_pointer$+0x7ffed732>
   100c8:	403283bb          	subw	t2,t0,gp
   100cc:	00000013          	nop
   100d0:	00000013          	nop
   100d4:	00000013          	nop
   100d8:	00000013          	nop
   100dc:	00000013          	nop
   100e0:	ddffccff          	.word	0xddffccff
