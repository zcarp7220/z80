#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#define Z80_SF 128 /*  Bitmask of the Z80 S flag.   */
#define Z80_ZF 64  /*  Bitmask of the Z80 Z flag.   */
#define Z80_F5 32  /*  Bitmask of the Z80 F5 flag.  */
#define Z80_HF 16  /*  Bitmask of the Z80 H flag.   */
#define Z80_F3 8   /*  Bitmask of the Z80 F3 flag.   */
#define Z80_PF 4   /*  Bitmask of the Z80 P/V flag. */
#define Z80_NF 2   /*  Bitmask of the Z80 N flag.   */
#define Z80_CF 1   /*  Bitmask of the Z80 C flag.   */

typedef struct cpu {
  // Registers
  union {
    struct {
      uint8_t F;
      uint8_t A;
    };
    uint16_t AF;
  };
  union {
    struct {
      uint8_t C;
      uint8_t B;
    };
    uint16_t BC;
  };
  union {
    struct {
      uint8_t E;
      uint8_t D;
    };
    uint16_t DE;
  };
  union {
    struct {
      uint8_t L;
      uint8_t H;
    };
    uint16_t HL;
  };

  // Shadow Registers
  union {
    struct {
      uint8_t Fp;
      uint8_t Ap;
    };
    uint16_t AFp;
  };
  union {
    struct {
      uint8_t Cp;
      uint8_t Bp;
    };
    uint16_t BCp;
  };
  union {
    struct {
      uint8_t Ep;
      uint8_t Dp;
    };
    uint16_t DEp;
  };
  union {
    struct {
      uint8_t Lp;
      uint8_t Hp;
    };
    uint16_t HLp;
  };
  // Speacal Purpose Registers
  uint16_t PC;
  uint16_t SP;
  uint16_t IX;
  uint16_t IY;
  uint16_t I;
  uint16_t R;
  uint8_t IM;
  bool IFF1;
  bool IFF2;
  bool eiDelay;
  bool INT;
  bool NMI;
  bool halt;
  long cycles;
  long numInst;
  void *userdata;
  uint16_t intData;
  // Functions
  uint8_t (*readByte)(struct cpu *, uint16_t);
  void (*writeByte)(struct cpu *, uint16_t, uint8_t);
  uint8_t (*in)(struct cpu *, uint16_t);
  void (*out)(struct cpu *, uint8_t, uint8_t);
} cpu_t;
void runOpcode(cpu_t *z80, uint8_t opcode);
void handleInterupts(cpu_t *z80);
void bitInstructions(cpu_t *z80, uint8_t opcode);
void miscInstructions(cpu_t *z80, uint8_t opcode);
void prefixInst(cpu_t *z80, uint8_t opcode);
// User Called Functions
void z80Init(cpu_t *z80);
void cpuStep(cpu_t *z80);
void request_NMI(cpu_t *z80);
void request_INT(cpu_t *z80, uint16_t intData);
