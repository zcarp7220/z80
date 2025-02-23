#include "common.h"
#define Z80_SF 128 /**< @brief Bitmask of the Z80 S flag.   */
#define Z80_ZF  64 /**< @brief Bitmask of the Z80 Z flag.   */
#define Z80_YF  32 /**< @brief Bitmask of the Z80 Y flag.   */
#define Z80_HF  16 /**< @brief Bitmask of the Z80 H flag.   */
#define Z80_XF   8 /**< @brief Bitmask of the Z80 X flag.   */
#define Z80_PF   4 /**< @brief Bitmask of the Z80 P/V flag. */
#define Z80_NF   2 /**< @brief Bitmask of the Z80 N flag.   */
#define Z80_CF   1 /**< @brief Bitmask of the Z80 C flag.   */
typedef struct cpu{
bool halt;
//Registers
union{struct{uint8_t A; uint8_t F;};uint16_t AF; };
union{struct{uint8_t B; uint8_t C;};uint16_t BC; };
union{struct{uint8_t D; uint8_t E;};uint16_t DE; };
union{struct{uint8_t H; uint8_t L;};uint16_t HL; };
//Shaow Registers
union{struct{uint8_t Ap; uint8_t Fp;};uint16_t AFp; };
union{struct{uint8_t Cp; uint8_t Bp;};uint16_t BCp; };
union{struct{uint8_t Ep; uint8_t Dp;};uint16_t DEp; };
union{struct{uint8_t Lp; uint8_t Hp;};uint16_t HLp; };
//Speacal Pourpse Registers
uint16_t PC; uint16_t SP; uint16_t IX; 
uint16_t IY; uint16_t I; uint16_t R;
bool EI; uint8_t IM; bool IFF1; bool IFF2;
}cpu_t;
void runOpcode(struct cpu * z80);
