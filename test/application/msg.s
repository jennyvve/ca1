
test/application/msg.bin:     file format elf64-littleriscv


Disassembly of section .text:

00000000000100f0 <_start>:
   100f0:	20000513          	li	a0,512
   100f4:	06c00593          	li	a1,108
   100f8:	00a00813          	li	a6,10
   100fc:	00900893          	li	a7,9
   10100:	00b50023          	sb	a1,0(a0)
   10104:	03300593          	li	a1,51
   10108:	00b50023          	sb	a1,0(a0)
   1010c:	00b50023          	sb	a1,0(a0)
   10110:	07400613          	li	a2,116
   10114:	00c50023          	sb	a2,0(a0)
   10118:	fed58593          	addi	a1,a1,-19
   1011c:	00b50023          	sb	a1,0(a0)
   10120:	04c00593          	li	a1,76
   10124:	00b50023          	sb	a1,0(a0)
   10128:	000116b7          	lui	a3,0x11
   1012c:	18068693          	addi	a3,a3,384 # 11180 <Q>
   10130:	0006b583          	ld	a1,0(a3)
   10134:	00b50023          	sb	a1,0(a0)
   10138:	411585b3          	sub	a1,a1,a7
   1013c:	00b50023          	sb	a1,0(a0)
   10140:	ffc50513          	addi	a0,a0,-4
   10144:	04300593          	li	a1,67
   10148:	00b50223          	sb	a1,4(a0)
   1014c:	01058593          	addi	a1,a1,16
   10150:	00b50223          	sb	a1,4(a0)
   10154:	ffc50513          	addi	a0,a0,-4
   10158:	01050423          	sb	a6,8(a0)
   1015c:	00000013          	nop
   10160:	00000013          	nop
   10164:	00000013          	nop
   10168:	00000013          	nop
   1016c:	00000013          	nop
   10170:	ddffccff          	.word	0xddffccff
