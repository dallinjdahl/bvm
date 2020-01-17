#include <stdint.h>
#include <stdio.h>
#include "ops.h"

char buffer[86] = {0};
char *tib = buffer + 5;

uint16_t constop(uint8_t op) {
	return op;
}

uint16_t singleop(uint8_t op, uint8_t arg) {
	return op+(arg<<12);
}

uint16_t doubleop(uint8_t op, uint8_t arga, uint8_t argb) {
	return op+(argb<<12)+(arga<<8);
}

uint16_t immop(uint8_t op, uint8_t imm, uint8_t arg) {
	return op+(arg<<12)+(imm<<6) + 32;
}

FILE *in;
void init(char *arg) {
	in = fopen(arg, "r");
}

uint8_t line = 1;
void fill() {
	uint8_t i = 0;
	tib[i++] = fgetc(in);
	if(tib[0] == '#') {
		while(fgetc(in) != '\n');
		i = 0;
		line++;
	}
	uint8_t j = -1;
	for(; i < 81; i++) {
		tib[i] = fgetc(in);
		if(tib[i] == ' ' || tib[i] == '\t') {
			tib[i++] = '\0';
			tib[j--] = i;
			char temp = fgetc(in);
			while(temp == ' ' || temp == '\t') {
				temp = fgetc(in);
			}
			tib[i] = temp;
		}
		if(tib[i] == '\n') {
			tib[i] = '\0';
			tib[j] = 0;
			return;
		}
	}
	printf("E: long lines (%d)\n", line);
	exit(1);
}

char * ptable = "halt\0\x80add\0\x81sub\0\x82mul\0\x83"
			"div\0\x84and\0\x85or\0\x86xor\0\x87"
			"not\0\x88rsh\0\x89arsh\0\x8alsh\0\x8b"
			"jmp\0\x8cjmpc\0\x8dcall\0\x8eret\0\x8f"
			"push\0\x90pop\0\x91mov\0\x92ldb\0\x93"
			"stb\0\x94ld\0\x95st\0\x96ainc\0\x97"
			"nop\0\x98ien\0\x99iqu\0\x9aiin\0\x9b"
			".byte\0\x9c.word\0\x9drmx\0\x0ersp\0\x0f";

uint8_t piter(char *s) {
	uint8_t j = 0;
	uint8_t i = 0;
	while(1) { 
		for(; s[i]; i++, j++) {
			if(ptable[j] != s[i]) {

				for(; ptable[j]; j++);
				j++;
				i = -1;

				if(!ptable[j+1]) {
					return -1;
				}
			}
		}

		if(!ptable[j]) {
			return ptable[j];
		}

		for(; ptable[j]; j++);
		j++;
		i = -1;

		if(!ptable[j+1]) {
			return -1;
		}
	}
	return -1;
}
bool err = false;
uint8_t reg(char *s) {
	err = false;
	char c = *s;
	if(c >> 4 == 3) {
		return c & 0x0f;
	}
	c |= 0x20;
	if(c >> 4 == 6) {
		return (c & 0x0f) + 9;
	}
	err = true;
	return -1;
}

uint16_t parse() {
	uint8_t f, o1, o2 = 0;
	char *in;
	f = piter(tib);
	if( f == -1 || !(f & 0x80)) {
		printf("E: op expected (%s)\n", tib);
		exit(1);
	}
	f &= ~0x80;
	switch (f) {
		case 0:
		case 0xf:
		case 0x18:
			return constop(f);
		case 0x08:
		case 0x10:
		case 0x11:
		case 0x19:
		case 0x1a;
		case 0x1b;
			in = tib + tib[-1];
			if(*in != 'r') {
				printf("E: reg expected (%s)\n", in);
				exit(1);
			}
			
			switch(in[1]) {
			case 'm':
				o1 = 0xe;
				break;
			case 's':
				o1 = 0xf;
				break;
			default:
				o1 = reg(in+1);
			}
			if(err) {
				printf("E: reg expected (%s)\n", in);
				exit(1);
			}

			return singleop(f, o1);
}
