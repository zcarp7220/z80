#include "common.h"

void step();
struct cpu{

bool halt;
//Registers
struct{uint8_t A; uint8_t F;};
union{struct{uint8_t B; uint8_t C;};uint16_t BC; };
union{struct{uint8_t D; uint8_t E;};uint16_t DE; };
union{struct{uint8_t H; uint8_t L;};uint16_t HL; };
//Shaow Registers
struct{uint8_t Ap; uint8_t Fp;};
union{struct{uint8_t Bp; uint8_t Cp;};uint16_t BCp; };
union{struct{uint8_t Dp; uint8_t Ep;};uint16_t DEp; };
union{struct{uint8_t Hp; uint8_t Lp;};uint16_t HLp; };
//Speacal Pourpse Registers
uint16_t PC; uint16_t SP; uint16_t IX; 
uint16_t IY; uint16_t I; uint16_t R;

}z80;
void runOpcode(struct cpu * z80);

