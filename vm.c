#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "ops.h"

#define IM_SIZE 64

typedef uint16_t word;

word image[IM_SIZE] = {0x12f5, 0x22f5, 0x35b3, 0x4573, 0x2104, 0x4302, 0x4216,
						0x11f5, 0x4101, 0x4314, 0x0000, 0x0004, 0xfffc, 0x2507, 0x0003};

#define LB(x) ((x) & 255)
#define HB(x) ((x) >> 8)

#define FLAGS(x) (((x) >> 5) & 7)
#define OP(x) ((x) & 31)

#define nibble(x) ((x) & 15)
#define blow(x) nibble((x) >> 8)
#define bhigh(x) nibble((x) >> 12)

#define IMM(x) (((x) >> 6) & 63)

#define IP regs[RIP]

#define SBYTE(w, x, h) ((h) ? ((w) & 255) | ((x) << 8) : ((w) & 65280) | (x)) 

#define FBA(a) ((a)&1 ? HB(image[(a)>>1]) : LB(image[(a)>>1]))
#define SBA(a, x) image[(a)>>1] = SBYTE(image[(a)>>1], ((x) & 255), (a)&1);

#define AIREG(x) ((1 << (x)) & regs[-1])

typedef struct {
	uint8_t op;
	uint8_t flags;
	uint8_t op1;
	uint8_t op2;
	int8_t imm;
} instruction;

instruction pl;

word registers[19] = {0};
word *regs = registers + 1;

div_t divres;

void decode(word w) {
	pl.op = OP(w);
	pl.flags = FLAGS(w);
	pl.op1 = blow(w);
	pl.op2 = bhigh(w);
	pl.imm = IMM(w);
}

void printregs(uint8_t x) {
	puts("\nRS:");
	for(uint8_t i = 0; i < x; i++) {
		printf("r%x:%d\n", i, regs[i]);
	}
	puts("");
}

void printmem(uint8_t x, uint8_t i) {
	printf("MEM:");
	for(int j = 0; j < i; j++, x++) {
		printf("%d:%x ", x, image[x]);
	}
	puts("");
}

void execute() {
	if(!pl.op) {
		exit(0);
	}
	printf("ip:%d w:%x\to[%d]\tf[%d]\ta[%d:%d]\ti[%d]\n",
		IP, image[IP], pl.op, pl.flags, pl.op1, pl.op2, pl.imm);
	switch(pl.op) {
	case ADD:
		regs[pl.op2] += regs[pl.op1];
		break;
	case SUB:
		regs[pl.op2] -= regs[pl.op1];
		break;
	case MUL:
		regs[pl.op2] *= regs[pl.op1];
		break;
	case DIV:
		divres = div(regs[pl.op2], regs[pl.op1]);
		regs[pl.op2] = divres.quot;
		regs[RMX] = divres.rem;
		break;
	case LDB:
		if(pl.flags & 1) {
			regs[pl.op2] = FBA((IP << 1) + pl.imm);
		} else {
			regs[pl.op2] = FBA(regs[pl.op1]);
			if(AIREG(pl.op1)) {
				regs[pl.op1] += 1;
			}
		}
		break;
	case STB:
		if(pl.flags & 1) {
			SBA((IP << 1) + pl.imm, regs[pl.op2]); 
		} else {
			SBA(regs[pl.op2], regs[pl.op1]);
			if(AIREG(pl.op2)) {
				regs[pl.op2] += 1;
			}
		}
		break;
	case LD:
		if(pl.flags & 1) {
			regs[pl.op2] = image[IP + pl.imm];
		} else {
			regs[pl.op2] = image[regs[pl.op1] >> 1];
			if(AIREG(pl.op1)) {
				regs[pl.op1] += 2;
			}
		}
		break;
	case ST:
		if(pl.flags & 1) {
			image[IP + pl.imm] = regs[pl.op2]; 
		} else {
			image[regs[pl.op2] >> 1] = regs[pl.op1];
			if(AIREG(pl.op2)) {
				regs[pl.op2] += 2;
			}
		}
		break;
	default:
		printf("E: undefined opcode at %x\n", IP);
		exit(1);
	}
	printregs(5);
	printmem(11, 8);
}

void test() {
	printf("w[%x]  x[%x] sb[%x:%x]\n", 0xfade, 0x77, SBYTE(0xfade, 0x77, 0), SBYTE(0xfade, 0x77, 1));
}

int main(int argc, char ** argv) {
	test();
	printmem(11, 8);
	while(1) {
		decode(image[IP]);
		execute();
		IP++;
	}
}

