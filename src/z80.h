#include "common.h"
#define Z80_SF 128 /**< @brief Bitmask of the Z80 S flag.   */
#define Z80_ZF  64 /**< @brief Bitmask of the Z80 Z flag.   */
#define Z80_YF  32 /**< @brief Bitmask of the Z80 Y flag.   */
#define Z80_HF  16 /**< @brief Bitmask of the Z80 H flag.   */
#define Z80_XF   8 /**< @brief Bitmask of the Z80 X flag.   */
#define Z80_PF   4 /**< @brief Bitmask of the Z80 P/V flag. */
#define Z80_NF   2 /**< @brief Bitmask of the Z80 N flag.   */
#define Z80_CF   1 /**< @brief Bitmask of the Z80 C flag.   */
void step();
typedef struct cpu{

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

}cpu_t;
void runOpcode(struct cpu * z80);

