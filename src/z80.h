#include "common.h"
void step(u_int8_t opcode[3]);
struct cpu{

bool halt;
//Registers
struct{u_int8_t A; u_int8_t F;};
union{struct{u_int8_t B; u_int8_t C;};u_int16_t BC; };
union{struct{u_int8_t D; u_int8_t E;};u_int16_t DE; };
union{struct{u_int8_t H; u_int8_t L;};u_int16_t HL; };
//Shaow Registers
struct{u_int8_t Ap; u_int8_t Fp;};
union{struct{u_int8_t Bp; u_int8_t Cp;};u_int16_t BCp; };
union{struct{u_int8_t Dp; u_int8_t Ep;};u_int16_t DEp; };
union{struct{u_int8_t Hp; u_int8_t Lp;};u_int16_t HLp; };
//Speacal Pourpse Registers
u_int16_t PC; u_int16_t SP; u_int16_t IX; 
u_int16_t IY; u_int16_t I; u_int16_t R;

};

