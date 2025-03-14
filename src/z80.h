#include "common.h"
#define Z80_SF 128 /*  Bitmask of the Z80 S flag.   */
#define Z80_ZF  64 /*  Bitmask of the Z80 Z flag.   */
#define Z80_F5  32 /*  Bitmask of the Z80 F5 flag.  */
#define Z80_HF  16 /*  Bitmask of the Z80 H flag.   */
#define Z80_F3   8 /*  Bitmask of the Z80 F3 flag.   */
#define Z80_PF   4 /*  Bitmask of the Z80 P/V flag. */
#define Z80_NF   2 /*  Bitmask of the Z80 N flag.   */
#define Z80_CF   1 /*  Bitmask of the Z80 C flag.   */

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 
typedef struct cpu{
bool halt;
//Registers
union{struct{uint8_t F; uint8_t A;};uint16_t AF; };
union{struct{uint8_t C; uint8_t B;};uint16_t BC; };
union{struct{uint8_t E; uint8_t D;};uint16_t DE; };
union{struct{uint8_t L; uint8_t H;};uint16_t HL; };
//Shaow Registers
union{struct{uint8_t Fp; uint8_t Ap;};uint16_t AFp; };
union{struct{uint8_t Cp; uint8_t Bp;};uint16_t BCp; };
union{struct{uint8_t Ep; uint8_t Dp;};uint16_t DEp; };
union{struct{uint8_t Lp; uint8_t Hp;};uint16_t HLp; };
//Speacal Pourpse Registers
uint16_t PC; uint16_t SP; uint16_t IX; 
uint16_t IY; uint16_t I; uint16_t R;
/*bool EI;*/ uint8_t IM; bool IFF1; bool IFF2;
}cpu_t;
extern cpu_t z80;
void runOpcode();
