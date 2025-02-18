#include "common.h"
void step(u_int8_t opcode[3]);
bool halt;
struct cpu{
u_int8_t A;
u_int8_t F;
u_int8_t B;
u_int8_t D;
u_int8_t H;
u_int8_t C;
u_int8_t E;
u_int8_t L;
u_int16_t BC;
u_int16_t DE;
u_int16_t HL;
u_int16_t PC;
u_int16_t SP;
u_int16_t IX;
u_int16_t IY;
u_int16_t AF;
u_int8_t R;
};
