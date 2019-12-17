#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "ops.h"

#define IM_SIZE 64

typedef uint16_t word;

word image[IM_SIZE] = {0x10b5, 0x20b5, 0x2101, 0x0000, 0x0011, 0x0020, 0};

#define a(x) ((x) & 255)
#define b(x) a((x) >> 8)

#define FLAGS(x) (((x) >> 5) & 7)
#define OP(x) ((x) & 31)

#define nibble(x) ((x) & 15)
#define blow(x) nibble((x) >> 8)
#define bhigh(x) nibble((x) >> 12)

#define IMM(x) (((x) >> 6) & 63)

#define IP regs[RIP]

typedef struct {
	uint8_t op;
	uint8_t flags;
	uint8_t op1;
	uint8_t op2;
	uint8_t imm;
} instruction;

instruction pl;

word registers[19] = {0};
word *regs = registers + 1;


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

void execute() {
	if(!pl.op) {
		exit(0);
	}
	printf("o[%d]\tf[%d]\ta[%d:%d]\ti[%d]\n",
		pl.op, pl.flags, pl.op1, pl.op2, pl.imm);
	switch(pl.op) {
	case ADD:
		regs[pl.op2] += regs[pl.op1];
		break;
	case LD:
		if(pl.flags & 1) {
			regs[pl.op2] = image[IP + (pl.imm << 1)];
		} else {
			regs[pl.op2] = image[regs[pl.op1 << 1]];
		}
		break;
	default:
		printf("E: undefined opcode at %x\n", IP);
		exit(1);
	}
	printregs(3);
}

int main(int argc, char ** argv) {
	while(1) {
		decode(image[IP]);
		execute();
		IP++;
	}
}

