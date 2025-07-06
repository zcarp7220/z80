#include "z80.h"
#define LD(B, A) \
  (B = A);       \
  z80->PC += 1;

#define HLASINDEX(A)                                      \
  do {                                                    \
    uint16_t _hl_backup = z80->HL;                        \
    uint16_t _addr = isHL_IX_IY                           \
                         ? z80->HL + (int8_t)displacement \
                         : z80->HL;                       \
    if (isHL_IX_IY) z80->PC += 1;                         \
                                                          \
    z80->HL = readMem(z80, _addr);                        \
    A;                                                    \
    writeMem(z80, _addr, z80->HL);                        \
    z80->HL = _hl_backup;                                 \
  } while (0)

int ss1, ss2, tempStorage, displacement = 0;
uint16_t hlStorage = 0;
bool oldParity, oldSign, oldZero, isHL_IX_IY = false;
static inline uint8_t readMem(cpu_t *const z, uint16_t addr) {
  return z->readByte(z, addr);
}
static inline void writeMem(cpu_t *const z, uint16_t addr, uint8_t data) {
  z->writeByte(z, addr, data);
}
static inline uint8_t in(cpu_t *const z, uint16_t addr) {
  z->PC += 1;
  return z->in(z, addr);
}
static inline void out(cpu_t *const z, uint16_t addr, uint8_t data) {
  z->PC += 1;
  z->out(z, addr, data);
}
void cpuStep(cpu_t *z80) {
  handleInterupts(z80);
  if (!z80->halt) {
    runOpcode(z80, readMem(z80, z80->PC));
  } else {
    if (readMem(z80, z80->PC + 1) != 0x76) {
      runOpcode(z80, 0x0);
    } else {
      runOpcode(z80, readMem(z80, z80->PC));
    }
  }
  z80->cycles += 1;
}
void handleInterupts(cpu_t *z80) {
  if (z80->halt && (z80->NMI || z80->MI)) {
    z80->halt = false;
  }
}
void z80Init(cpu_t *z80) {
  z80->readByte = NULL;
  z80->writeByte = NULL;
  z80->in = NULL;
  z80->out = NULL;

  z80->cycles = 0;
  z80->PC = 0;
  z80->IX = 0;
  z80->IY = 0;

  // AF and SP are set to 0xFFFF after reset
  z80->AF = 0xFFFF;
  z80->SP = 0xFFFF;

  z80->BC = 0;
  z80->DE = 0;
  z80->HL = 0;

  z80->AFp = 0;
  z80->BCp = 0;
  z80->DEp = 0;
  z80->HLp = 0;

  z80->I = 0;
  z80->R = 0;

  z80->wasLastInstEI = false;
  z80->IM = 0;
  z80->IFF1 = false;
  z80->IFF2 = false;
  z80->halt = false;
  z80->MI = 0;
  z80->NMI = 0;
}
void setFlag(cpu_t *z80, int flag) { z80->F |= flag; }
void clearFlag(cpu_t *z80, int flag) { z80->F &= ~(flag); }
bool readFlag(cpu_t *z80, int flag) { return z80->F & flag; }
uint8_t pop(cpu_t *z80) {
  z80->SP += 1;
  return readMem(z80, z80->SP - 1);
}
void push(cpu_t *z80, uint8_t A) {
  z80->SP -= 1;
  writeMem(z80, z80->SP, A);
}
void push16(cpu_t *z80, uint16_t A) {
  push(z80, A >> 8);
  push(z80, A & 0xFF);
}
uint16_t pop16(cpu_t *z80) { return pop(z80) | pop(z80) << 8; }
static inline uint16_t readNN(cpu_t *z80) {
  z80->PC += 2;
  return readMem(z80, z80->PC - 1) | readMem(z80, z80->PC) << 8;
}

static inline uint8_t readN(cpu_t *z80) {
  z80->PC += 1;
  return readMem(z80, z80->PC);
}
bool getParity(unsigned int value) {
  value ^= value >> 4;
  value ^= value >> 2;
  value ^= value >> 1;
  return !(value & 1);
}

static inline uint16_t nnAsPointer(cpu_t *z80) {
  int valueToRead = readNN(z80);
  int address = readMem(z80, valueToRead) | readMem(z80, valueToRead + 1) << 8;
  return address;
}

#define checkSet(flag, condition) \
  ((z80)->F = ((z80)->F & ~(flag)) | (-!!(condition) & (flag)))

void setUndocumentedFlags(cpu_t *z80, int result) {
  checkSet(Z80_F3, result & Z80_F3);
  checkSet(Z80_F5, result & Z80_F5);
}

static inline void add(cpu_t *z80, int A, int B, void *storeLocation,
                       char width, bool carry) {
  clearFlag(z80, Z80_NF);
  if (width == 'b') {
    uint8_t result = A + B + carry;
    setUndocumentedFlags(z80, result);
    checkSet(Z80_ZF, result == 0);
    checkSet(Z80_PF, ((A ^ result) & (B ^ result) & 0x80) != 0);
    checkSet(Z80_SF, (result & 0x80) != 0);
    checkSet(Z80_CF, (A + B + carry) > 0xFF);
    checkSet(Z80_HF, ((A & 0xF) + (B & 0xF) + carry) > 0xF);
    *(uint8_t *)storeLocation = result;
  } else if (width == 'w') {
    uint16_t result = A + B + carry;
    setUndocumentedFlags(z80, result >> 8);
    checkSet(Z80_HF, (((A ^ (B) ^ result) >> 8) & Z80_HF));
    checkSet(Z80_CF, (A + B + carry) > 65535);
    *(uint16_t *)storeLocation = result;
  }
  z80->PC += 1;
}

static inline void sub(cpu_t *z80, int A, int B, void *storeLocation,
                       char width, bool carry) {
  if (width == 'b') {
    add(z80, A, ~B, storeLocation, 'b', !carry);
    setFlag(z80, Z80_NF);
    checkSet(Z80_HF, !readFlag(z80, Z80_HF));
    checkSet(Z80_CF, A < B + carry);
  } else if (width == 'w') {
    sub(z80, A & 0xFF, B & 0xFF, &ss1, 'b', carry);
    sub(z80, (A & 0xFF00) >> 8, (B & 0xFF00) >> 8, &ss2, 'b', readFlag(z80, Z80_CF));
    checkSet(Z80_ZF, *(uint16_t *)storeLocation == 0);
    *(uint16_t *)storeLocation = ss1 | ss2 << 8;
    z80->PC -= 1;
  }
}

static inline void and (cpu_t * z80, int A) {
  int result = A & z80->A;
  clearFlag(z80, Z80_CF);
  clearFlag(z80, Z80_NF);
  setFlag(z80, Z80_HF);
  checkSet(Z80_PF, getParity(result));
  checkSet(Z80_ZF, (uint8_t)result == 0);
  checkSet(Z80_SF, (result & 0x80) != 0);
  setUndocumentedFlags(z80, result);
  z80->A = result;
  z80->PC += 1;
}

static inline void xor
        (cpu_t * z80, int A) {
          int result = A ^ z80->A;
          clearFlag(z80, Z80_CF);
          clearFlag(z80, Z80_NF);
          clearFlag(z80, Z80_HF);
          checkSet(Z80_PF, getParity(result));
          checkSet(Z80_ZF, (uint8_t)result == 0);
          checkSet(Z80_SF, (result & 0x80) != 0);
          setUndocumentedFlags(z80, result);
          z80->A = result;
          z80->PC += 1;
        }

        static inline void or
    (cpu_t * z80, int A) {
  int result = A | z80->A;
  clearFlag(z80, Z80_CF);
  clearFlag(z80, Z80_NF);
  clearFlag(z80, Z80_HF);
  checkSet(Z80_PF, getParity(result));
  checkSet(Z80_ZF, (uint8_t)result == 0);
  checkSet(Z80_SF, (result & 0x80) != 0);
  setUndocumentedFlags(z80, result);
  z80->A = result;
  z80->PC += 1;
}
static inline void cp(cpu_t *z80, int A) {
  uint8_t trash;
  sub(z80, z80->A, A, &trash, 'b', false);
  setUndocumentedFlags(z80, A);
}
static inline void rotateC(cpu_t *z80, char direction, void *val) {
  int result = 0;
  clearFlag(z80, Z80_HF);
  clearFlag(z80, Z80_NF);
  if (direction == 'l') {
    result = (*(uint8_t *)val << 1) | (*(uint8_t *)val >> 7);
    checkSet(Z80_CF, (result & 1) != 0);
  } else if (direction == 'r') {
    result = (*(uint8_t *)val >> 1) | (*(uint8_t *)val << 7);
    checkSet(Z80_CF, (result & 0x80) != 0);
  }
  checkSet(Z80_PF, getParity(result));
  checkSet(Z80_ZF, (uint8_t)result == 0);
  checkSet(Z80_SF, (result & 0x80) != 0);
  setUndocumentedFlags(z80, result);
  *(uint8_t *)val = result;
  z80->PC += 1;
}
static inline void rc_a(cpu_t *z80, char direction) {
  uint8_t *val = &z80->A;
  int result = 0;
  clearFlag(z80, Z80_HF);
  clearFlag(z80, Z80_NF);
  if (direction == 'l') {
    result = (*(uint8_t *)val << 1) | (*(uint8_t *)val >> 7);
    checkSet(Z80_CF, (result & 1) != 0);
  } else if (direction == 'r') {
    result = (*(uint8_t *)val >> 1) | (*(uint8_t *)val << 7);
    checkSet(Z80_CF, (result & 0x80) != 0);
  }
  setUndocumentedFlags(z80, result);
  *(uint8_t *)val = result;
  z80->PC += 1;
}
static inline void rotate(cpu_t *z80, char direction, void *val) {
  int result = 0;
  clearFlag(z80, Z80_HF);
  clearFlag(z80, Z80_NF);
  uint8_t value = *(uint8_t *)val;
  if (direction == 'l') {
    result = (value << 1) | (value >> 8) | readFlag(z80, Z80_CF);
    checkSet(Z80_CF, result & 0x100);
  } else if (direction == 'r') {
    result = (value >> 1) | (readFlag(z80, Z80_CF) << 7);
    checkSet(Z80_CF, (value & 0x1) != 0);
  }
  setUndocumentedFlags(z80, result);
  checkSet(Z80_PF, getParity(result));
  checkSet(Z80_ZF, (uint8_t)result == 0);
  checkSet(Z80_SF, (result & 0x80) != 0);
  *(uint8_t *)val = result;
  z80->PC += 1;
}
static inline void r_a(cpu_t *z80, char direction) {
  uint8_t *val = &z80->A;
  int result = 0;
  clearFlag(z80, Z80_HF);
  clearFlag(z80, Z80_NF);
  uint8_t value = *(uint8_t *)val;
  if (direction == 'l') {
    result = (value << 1) | (value >> 8) | readFlag(z80, Z80_CF);
    checkSet(Z80_CF, result & 0x100);
  } else if (direction == 'r') {
    result = (value >> 1) | (readFlag(z80, Z80_CF) << 7);
    checkSet(Z80_CF, (value & 0x1) != 0);
  }
  setUndocumentedFlags(z80, result);
  *(uint8_t *)val = result;
  z80->PC += 1;
}
static inline void shift(cpu_t *z80, char direction, void *val) {
  int result = 0;
  clearFlag(z80, Z80_HF);
  clearFlag(z80, Z80_NF);
  uint8_t value = *(uint8_t *)val;
  if (direction == 'l') {
    result = value << 1;
    checkSet(Z80_CF, result & 0x100);
  } else if (direction == 'r') {
    result = value >> 1;
    result |= value & 0x80;
    checkSet(Z80_CF, (value & 0x1) != 0);
  }
  setUndocumentedFlags(z80, result);
  checkSet(Z80_PF, getParity(result));
  checkSet(Z80_ZF, (uint8_t)result == 0);
  checkSet(Z80_SF, (result & 0x80) != 0);
  *(uint8_t *)val = result;
  z80->PC += 1;
}

static inline void logicalShift(cpu_t *z80, char direction, void *val) {
  int result = 0;
  clearFlag(z80, Z80_HF);
  clearFlag(z80, Z80_NF);
  uint8_t value = *(uint8_t *)val;
  if (direction == 'l') {
    result = value << 1;
    result |= 0x1;
    checkSet(Z80_CF, result & 0x100);
  } else if (direction == 'r') {
    result = value >> 1;
    result &= ~0x80;
    checkSet(Z80_CF, (value & 0x1) != 0);
  }
  setUndocumentedFlags(z80, result);
  checkSet(Z80_PF, getParity(result));
  checkSet(Z80_ZF, (uint8_t)result == 0);
  checkSet(Z80_SF, (result & 0x80) != 0);
  *(uint8_t *)val = result;
  z80->PC += 1;
}
static inline void bit(cpu_t *z80, int pos, uint8_t *ss2) {
  uint8_t value = *ss2;
  int result = value & (1 << (pos));
  clearFlag(z80, Z80_NF);
  setFlag(z80, Z80_HF);
  checkSet(Z80_ZF, !result);
  checkSet(Z80_PF, !result);
  checkSet(Z80_SF, result & Z80_SF);
  checkSet(Z80_F3, value & Z80_F3);
  checkSet(Z80_F5, value & Z80_F5);
  z80->PC += 1;
}

static inline void set(cpu_t *z80, int pos, uint8_t *ss2) {
  *ss2 |= (1 << pos);
  z80->PC += 1;
}

static inline void reset(cpu_t *z80, int pos, uint8_t *ss2) {
  *ss2 &= ~(1 << pos);
  z80->PC += 1;
}
static inline void exchangeRegisters(cpu_t *z80, uint16_t *A, uint16_t *B) {
  uint16_t ss1;
  ss1 = *B;
  *B = *A;
  *A = ss1;
  z80->PC += 1;
}

void jr(cpu_t *z80, bool condition) {
  if (condition) {
    z80->PC += (int8_t)readMem(z80, z80->PC + 1);
  }
  z80->PC += 2;
}

void dnjz(cpu_t *z80) {
  z80->B--;
  if (z80->B != 0) {
    z80->PC += (int8_t)readN(z80);
  } else {
    z80->PC += 1;
  }
  z80->PC += 1;
}
// Algorithm adapted from:
// https://worldofspectrum.org/faq/reference/z80reference.htm#DATA
void DAA_inst(cpu_t *z80) {
  int afterA = z80->A;
  int correctFactor = 0;
  if (z80->A > 0x99 || readFlag(z80, Z80_CF)) {
    correctFactor += 0x60;
    setFlag(z80, Z80_CF);
  } else {
    clearFlag(z80, Z80_CF);
  }
  if ((z80->A & 0xF) > 9 || readFlag(z80, Z80_HF)) {
    correctFactor += 0x6;
  }
  if (!readFlag(z80, Z80_NF)) {
    afterA += correctFactor;
  } else {
    afterA -= correctFactor;
  }
  setUndocumentedFlags(z80, afterA);
  checkSet(Z80_HF, (z80->A ^ afterA) & Z80_HF);
  checkSet(Z80_ZF, (uint8_t)afterA == 0);
  checkSet(Z80_PF, getParity(afterA));
  checkSet(Z80_SF, (afterA & 0x80) != 0);
  z80->A = afterA;
  z80->PC += 1;
}
void cpl(cpu_t *z80) {
  setFlag(z80, Z80_NF);
  setFlag(z80, Z80_HF);
  z80->A = ~z80->A;
  setUndocumentedFlags(z80, z80->A);
  z80->PC += 1;
}
void ccf(cpu_t *z80) {
  clearFlag(z80, Z80_NF);
  setUndocumentedFlags(z80, z80->A);
  checkSet(Z80_HF, readFlag(z80, Z80_CF));
  checkSet(Z80_CF, !readFlag(z80, Z80_CF));
  z80->PC += 1;
}
void scf(cpu_t *z80) {
  clearFlag(z80, Z80_NF);
  clearFlag(z80, Z80_HF);
  setUndocumentedFlags(z80, z80->A);
  setFlag(z80, Z80_CF);
  z80->PC += 1;
}
void write16(cpu_t *z80, uint16_t addr, uint16_t data) {
  z80->PC += 1;
  writeMem(z80, addr, (data & 0xFF));
  writeMem(z80, addr + 1, (data & 0xFF00) >> 8);
}
static inline void inc8(cpu_t *z80, void *value) {
  bool oldCarry = readFlag(z80, Z80_CF);
  add(z80, *(uint8_t *)value, 0, value, 'b', true);
  checkSet(Z80_CF, oldCarry);
}

static inline void inc16(cpu_t *z80, uint16_t *value) {
  *value += 1;
  z80->PC += 1;
}
static inline void incR(cpu_t *z80) {
  if (z80->R + 1 == 0x80) {
    z80->R = 0;
  } else {
    z80->R++;
  }
}
static inline void dec8(cpu_t *z80, void *value) {
  bool oldCarry = readFlag(z80, Z80_CF);
  sub(z80, *(uint8_t *)value, 0, value, 'b', true);
  checkSet(Z80_CF, oldCarry);
}

static inline void dec16(cpu_t *z80, uint16_t *value) {
  *value -= 1;
  z80->PC += 1;
}
void call(cpu_t *z80, bool input) {
  if (input) {
    push16(z80, z80->PC + 3);
    z80->PC = readNN(z80);
  } else {
    z80->PC += 3;
  }
}
void rst(cpu_t *z80, int resetVector) {
  push16(z80, z80->PC + 1);
  z80->PC = resetVector;
}
void ret(cpu_t *z80, bool input) {
  if (input) {
    z80->PC = pop(z80) | pop(z80) << 8;
  } else {
    z80->PC += 1;
  }
}
void jp(cpu_t *z80, bool input) {
  if (input) {
    z80->PC = readNN(z80);
  } else {
    z80->PC += 3;
  }
}
void undocCbPrefix(cpu_t *z80, uint8_t *input, uint16_t index_Displacment) {
  ss2 = z80->PC + 2;
  *input = readMem(z80, index_Displacment);
  bitInstructions(z80, readMem(z80, ss2));
  writeMem(z80, index_Displacment, *input);
}
void ini(cpu_t *z80) {
  ss2 = in(z80, z80->BC);
  writeMem(z80, z80->HL, ss2);
  dec8(z80, &z80->B);
  z80->PC -= 1;
  z80->HL++;
  checkSet(Z80_NF, ss2 >> 7);
  checkSet(Z80_HF | Z80_CF, (ss2 + ((z80->C + 1) & 255) > 255));
  checkSet(Z80_PF, getParity(((ss2 + ((z80->C + 1) & 255)) & 7) ^ z80->B));
}
static inline void postINI_OUTI_R(cpu_t *z80, uint8_t data) {
  // I guess this is right, source
  // https://github.com/hoglet67/Z80Decoder/wiki/Undocumented-Flags
  checkSet(Z80_F5, ((z80->PC - 2) >> 11) & 1);
  checkSet(Z80_F3, ((z80->PC - 2) >> 13) & 1);
  if (readFlag(z80, Z80_CF)) {
    if (data & 0x80) {
      checkSet(Z80_PF, readFlag(z80, Z80_PF) ^ getParity((z80->B - 1) & 7) ^ 1);
      checkSet(Z80_HF, (z80->B & 0x0F) == 0);
    } else {
      checkSet(Z80_PF, readFlag(z80, Z80_PF) ^ getParity((z80->B + 1) & 7) ^ 1);
      checkSet(Z80_HF, (z80->B & 0x0F) == 0x0F);
    }
  } else {
    checkSet(Z80_PF, readFlag(z80, Z80_PF) ^ getParity(z80->B & 7) ^ 1);
  }
}

void cpd(cpu_t *z80) {
  oldZero = readFlag(z80, Z80_CF);
  ss1 = readMem(z80, z80->HL--);
  cp(z80, ss1);
  z80->BC--;
  checkSet(Z80_PF, z80->BC);
  setFlag(z80, Z80_NF);
  checkSet(Z80_F3, ss1 & Z80_F3);
  checkSet(Z80_F5, ss1 & 0x2);
  checkSet(Z80_CF, oldZero);
}
void cpi(cpu_t *z80) {
  cpd(z80);
  z80->HL += 2;
}
void adcHL(cpu_t *z80, uint16_t input) {
  add(z80, z80->L, (input & 0xFF), &ss1, 'b', readFlag(z80, Z80_CF));
  add(z80, z80->H, (input & 0xFF00) >> 8, &ss2, 'b', readFlag(z80, Z80_CF));
  z80->HL = ss1 | ss2 << 8;
  checkSet(Z80_SF, z80->HL >> 15);
  checkSet(Z80_ZF, z80->HL == 0);
  z80->PC -= 1;
}
void sbcHL(cpu_t *z80, uint16_t input) {
  sub(z80, z80->HL, input, &z80->HL, 'w', readFlag(z80, Z80_CF));
  checkSet(Z80_SF, z80->HL >> 15);
  checkSet(Z80_ZF, z80->HL == 0);
}
void prefixInst(cpu_t *z80) {
  z80->PC += 1;
  displacement = readMem(z80, z80->PC + 1);
  if (readMem(z80, z80->PC) != 0xCB) {
    runOpcode(z80, readMem(z80, z80->PC));
  } else if (readMem(z80, z80->PC) == 0xCB) {
    ss1 = z80->HL + (int8_t)displacement;
    if (readMem(z80, z80->PC + 2) >= 0x40 &&
        readMem(z80, z80->PC + 2) <= 0x7F) {
      uint8_t temp = readMem(z80, ss1);
      incR(z80);
      bit(z80, (readMem(z80, z80->PC + 2) - 0x40) / 8, &temp);
      z80->PC += 2;
    } else {
      switch (readMem(z80, z80->PC + 2) & 7) {
      case 0:
        undocCbPrefix(z80, &z80->B, ss1);
        break;
      case 1:
        undocCbPrefix(z80, &z80->C, ss1);
        break;
      case 2:
        undocCbPrefix(z80, &z80->D, ss1);
        break;
      case 3:
        undocCbPrefix(z80, &z80->E, ss1);
        break;
      case 4:
        tempStorage = z80->H;
        undocCbPrefix(z80, &z80->H, ss1);
        hlStorage = (z80->H << 8) | (hlStorage & 0xFF);
        z80->H = tempStorage;
        break;
      case 5:
        tempStorage = z80->L;
        undocCbPrefix(z80, &z80->L, ss1);
        hlStorage = (z80->L) | (hlStorage & 0xFF00);
        z80->L = tempStorage;
        break;
      case 6:
        bitInstructions(z80, readMem(z80, z80->PC + 2));
        break;
      case 7:
        undocCbPrefix(z80, &z80->A, ss1);
        break;
      }
      z80->PC += 2;
    }
  }
}
void runOpcode(cpu_t *z80, uint8_t opcode) {
  incR(z80);
  switch (opcode) {
  case 0x00:
    z80->PC += 1;
    break;
  case 0x01:
    LD(z80->BC, readNN(z80));
    break;
  case 0x02:
    writeMem(z80, z80->BC, z80->A);
    z80->PC += 1;
    break;
  case 0x03:
    inc16(z80, &z80->BC);
    break;
  case 0x04:
    inc8(z80, &z80->B);
    break;
  case 0x05:
    dec8(z80, &z80->B);
    break;
  case 0x06:
    LD(z80->B, readN(z80));
    break;
  case 0x07:
    rc_a(z80, 'l');
    break;
  case 0x08:
    exchangeRegisters(z80, &z80->AF, &z80->AFp);
    break;
  case 0x09:
    add(z80, z80->BC, z80->HL, &z80->HL, 'w', false);
    break;
  case 0x0A:
    LD(z80->A, readMem(z80, z80->BC));
    break;
  case 0x0B:
    dec16(z80, &z80->BC);
    break;
  case 0x0C:
    inc8(z80, &z80->C);
    break;
  case 0x0D:
    dec8(z80, &z80->C);
    break;
  case 0x0E:
    LD(z80->C, readN(z80));
    break;
  case 0x0F:
    rc_a(z80, 'r');
    break;
  case 0x10:
    dnjz(z80);
    break;
  case 0x11:
    LD(z80->DE, readNN(z80));
    break;
  case 0x12:
    writeMem(z80, z80->DE, z80->A);
    z80->PC += 1;
    break;
  case 0x13:
    inc16(z80, &z80->DE);
    break;
  case 0x14:
    inc8(z80, &z80->D);
    break;
  case 0x15:
    dec8(z80, &z80->D);
    break;
  case 0x16:
    LD(z80->D, readN(z80));
    break;
  case 0x17:
    r_a(z80, 'l');
    break;
  case 0x18:
    jr(z80, true);
    break;
  case 0x19:
    add(z80, z80->DE, z80->HL, &z80->HL, 'w', false);
    break;
  case 0x1A:
    LD(z80->A, readMem(z80, z80->DE));
    break;
  case 0x1B:
    dec16(z80, &z80->DE);
    break;
  case 0x1C:
    inc8(z80, &z80->E);
    break;
  case 0x1D:
    dec8(z80, &z80->E);
    break;
  case 0x1E:
    LD(z80->E, readN(z80));
    break;
  case 0x1F:
    r_a(z80, 'r');
    break;
  case 0x20:
    jr(z80, !readFlag(z80, Z80_ZF));
    break;
  case 0x21:
    LD(z80->HL, readNN(z80));
    break;
  case 0x22:
    write16(z80, readNN(z80), z80->HL);
    break;
  case 0x23:
    inc16(z80, &z80->HL);
    break;
  case 0x24:
    inc8(z80, &z80->H);
    break;
  case 0x25:
    dec8(z80, &z80->H);
    break;
  case 0x26:
    LD(z80->H, readN(z80));
    break;
  case 0x27:
    DAA_inst(z80);
    break;
  case 0x28:
    jr(z80, readFlag(z80, Z80_ZF));
    break;
  case 0x29:
    add(z80, z80->HL, z80->HL, &z80->HL, 'w', false);
    break;
  case 0x2A:
    LD(z80->HL, nnAsPointer(z80));
    break;
  case 0x2B:
    dec16(z80, &z80->HL);
    break;
  case 0x2C:
    inc8(z80, &z80->L);
    break;
  case 0x2D:
    dec8(z80, &z80->L);
    break;
  case 0x2E:
    LD(z80->L, readN(z80));
    break;
  case 0x2F:
    cpl(z80);
    break;
  case 0x30:
    jr(z80, !readFlag(z80, Z80_CF));
    break;
  case 0x31:
    LD(z80->SP, readNN(z80));
    break;
  case 0x32:
    writeMem(z80, readNN(z80), z80->A);
    z80->PC += 1;
    break;
  case 0x33:
    inc16(z80, &z80->SP);
    break;
  case 0x34:
    HLASINDEX(inc8(z80, &z80->HL));
    break;
  case 0x35:
    HLASINDEX(dec8(z80, &z80->HL));
    break;
  case 0x36:
    HLASINDEX(LD(z80->HL, readN(z80)));
    break;
  case 0x37:
    scf(z80);
    break;
  case 0x38:
    jr(z80, readFlag(z80, Z80_CF));
    break;
  case 0x39:
    add(z80, z80->SP, z80->HL, &z80->HL, 'w', false);
    break;
  case 0x3A:
    LD(z80->A, nnAsPointer(z80));
    break;
  case 0x3B:
    dec16(z80, &z80->SP);
    break;
  case 0x3C:
    inc8(z80, &z80->A);
    break;
  case 0x3D:
    dec8(z80, &z80->A);
    break;
  case 0x3E:
    LD(z80->A, readN(z80));
    break;
  case 0x3F:
    ccf(z80);
    break;
  case 0x40:
    LD(z80->B, z80->B);
    break;
  case 0x41:
    LD(z80->B, z80->C);
    break;
  case 0x42:
    LD(z80->B, z80->D);
    break;
  case 0x43:
    LD(z80->B, z80->E);
    break;
  case 0x44:
    LD(z80->B, z80->H);
    break;
  case 0x45:
    LD(z80->B, z80->L);
    break;
  case 0x46:
    HLASINDEX(LD(z80->B, z80->HL));
    break;
  case 0x47:
    LD(z80->B, z80->A);
    break;
  case 0x48:
    LD(z80->C, z80->B);
    break;
  case 0x49:
    LD(z80->C, z80->C);
    break;
  case 0x4A:
    LD(z80->C, z80->D);
    break;
  case 0x4B:
    LD(z80->C, z80->E);
    break;
  case 0x4C:
    LD(z80->C, z80->H);
    break;
  case 0x4D:
    LD(z80->C, z80->L);
    break;
  case 0x4E:
    HLASINDEX(LD(z80->C, z80->HL));
    break;
  case 0x4F:
    LD(z80->C, z80->A);
    break;
  case 0x50:
    LD(z80->D, z80->B);
    break;
  case 0x51:
    LD(z80->D, z80->C);
    break;
  case 0x52:
    LD(z80->D, z80->D);
    break;
  case 0x53:
    LD(z80->D, z80->E);
    break;
  case 0x54:
    LD(z80->D, z80->H);
    break;
  case 0x55:
    LD(z80->D, z80->L);
    break;
  case 0x56:
    HLASINDEX(LD(z80->D, z80->HL));
    break;
  case 0x57:
    LD(z80->D, z80->A);
    break;
  case 0x58:
    LD(z80->E, z80->B);
    break;
  case 0x59:
    LD(z80->E, z80->C);
    break;
  case 0x5A:
    LD(z80->E, z80->D);
    break;
  case 0x5B:
    LD(z80->E, z80->E);
    break;
  case 0x5C:
    LD(z80->E, z80->H);
    break;
  case 0x5D:
    LD(z80->E, z80->L);
    break;
  case 0x5E:
    HLASINDEX(LD(z80->E, z80->HL));
    break;
  case 0x5F:
    LD(z80->E, z80->A);
    break;
  case 0x60:
    LD(z80->H, z80->B);
    break;
  case 0x61:
    LD(z80->H, z80->C);
    break;
  case 0x62:
    LD(z80->H, z80->D);
    break;
  case 0x63:
    LD(z80->H, z80->E);
    break;
  case 0x64:
    LD(z80->H, z80->H);
    break;
  case 0x65:
    LD(z80->H, z80->L);
    break;
  case 0x66:
    HLASINDEX(LD(tempStorage, z80->HL));
    if (!isHL_IX_IY) {
      z80->H = tempStorage;
    } else {
      hlStorage = (hlStorage & 0xFF) | (tempStorage << 8);
    }
    break;
  case 0x67:
    LD(z80->H, z80->A);
    break;
  case 0x68:
    LD(z80->L, z80->B);
    break;
  case 0x69:
    LD(z80->L, z80->C);
    break;
  case 0x6A:
    LD(z80->L, z80->D);
    break;
  case 0x6B:
    LD(z80->L, z80->E);
    break;
  case 0x6C:
    LD(z80->L, z80->H);
    break;
  case 0x6D:
    LD(z80->L, z80->L);
    break;
  case 0x6E:
    HLASINDEX(LD(tempStorage, z80->HL));
    if (!isHL_IX_IY) {
      z80->L = tempStorage;
    } else {
      hlStorage = (hlStorage & 0xFF00) | tempStorage;
    }
    break;
  case 0x6F:
    LD(z80->L, z80->A);
    break;
  case 0x70:
    HLASINDEX(LD(z80->HL, z80->B));
    break;
  case 0x71:
    HLASINDEX(LD(z80->HL, z80->C));
    break;
  case 0x72:
    HLASINDEX(LD(z80->HL, z80->D));
    break;
  case 0x73:
    HLASINDEX(LD(z80->HL, z80->E));
    break;
  case 0x74:
    if (!isHL_IX_IY) {
      tempStorage = z80->H;
    } else {
      tempStorage = (hlStorage & 0xFF00) >> 8;
    }
    HLASINDEX(LD(z80->HL, tempStorage));
    break;
  case 0x75:
    if (!isHL_IX_IY) {
      tempStorage = z80->L;
    } else {
      tempStorage = hlStorage & 0xFF;
    }
    HLASINDEX(LD(z80->HL, tempStorage));
    break;
  case 0x76:
    z80->halt = true;
    z80->PC += 1;
    break;
  case 0x77:
    HLASINDEX(LD(z80->HL, z80->A));
    break;
  case 0x78:
    LD(z80->A, z80->B);
    break;
  case 0x79:
    LD(z80->A, z80->C);
    break;
  case 0x7A:
    LD(z80->A, z80->D);
    break;
  case 0x7B:
    LD(z80->A, z80->E);
    break;
  case 0x7C:
    LD(z80->A, z80->H);
    break;
  case 0x7D:
    LD(z80->A, z80->L);
    break;
  case 0x7E:
    HLASINDEX(LD(z80->A, z80->HL));
    break;
  case 0x7F:
    LD(z80->A, z80->A);
    break;
  case 0x80:
    add(z80, z80->A, z80->B, &z80->A, 'b', false);
    break;
  case 0x81:
    add(z80, z80->A, z80->C, &z80->A, 'b', false);
    break;
  case 0x82:
    add(z80, z80->A, z80->D, &z80->A, 'b', false);
    break;
  case 0x83:
    add(z80, z80->A, z80->E, &z80->A, 'b', false);
    break;
  case 0x84:
    add(z80, z80->A, z80->H, &z80->A, 'b', false);
    break;
  case 0x85:
    add(z80, z80->A, z80->L, &z80->A, 'b', false);
    break;
  case 0x86:
    HLASINDEX(add(z80, z80->A, z80->HL, &z80->A, 'b', false));
    break;
  case 0x87:
    add(z80, z80->A, z80->A, &z80->A, 'b', false);
    break;
  case 0x88:
    add(z80, z80->A, z80->B, &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0x89:
    add(z80, z80->A, z80->C, &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0x8A:
    add(z80, z80->A, z80->D, &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0x8B:
    add(z80, z80->A, z80->E, &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0x8C:
    add(z80, z80->A, z80->H, &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0x8D:
    add(z80, z80->A, z80->L, &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0x8E:
    HLASINDEX(add(z80, z80->A, z80->HL, &z80->A, 'b', readFlag(z80, Z80_CF)));
    break;
  case 0x8F:
    add(z80, z80->A, z80->A, &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0x90:
    sub(z80, z80->A, z80->B, &z80->A, 'b', false);
    break;
  case 0x91:
    sub(z80, z80->A, z80->C, &z80->A, 'b', false);
    break;
  case 0x92:
    sub(z80, z80->A, z80->D, &z80->A, 'b', false);
    break;
  case 0x93:
    sub(z80, z80->A, z80->E, &z80->A, 'b', false);
    break;
  case 0x94:
    sub(z80, z80->A, z80->H, &z80->A, 'b', false);
    break;
  case 0x95:
    sub(z80, z80->A, z80->L, &z80->A, 'b', false);
    break;
  case 0x96:
    HLASINDEX(sub(z80, z80->A, z80->HL, &z80->A, 'b', false));
    break;
  case 0x97:
    sub(z80, z80->A, z80->A, &z80->A, 'b', false);
    break;
  case 0x98:
    sub(z80, z80->A, z80->B, &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0x99:
    sub(z80, z80->A, z80->C, &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0x9A:
    sub(z80, z80->A, z80->D, &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0x9B:
    sub(z80, z80->A, z80->E, &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0x9C:
    sub(z80, z80->A, z80->H, &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0x9D:
    sub(z80, z80->A, z80->L, &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0x9E:
    HLASINDEX(sub(z80, z80->A, z80->HL, &z80->A, 'b', readFlag(z80, Z80_CF)));
    break;
  case 0x9F:
    sub(z80, z80->A, z80->A, &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0xA0:
    and(z80, z80->B);
    break;
  case 0xA1:
    and(z80, z80->C);
    break;
  case 0xA2:
    and(z80, z80->D);
    break;
  case 0xA3:
    and(z80, z80->E);
    break;
  case 0xA4:
    and(z80, z80->H);
    break;
  case 0xA5:
    and(z80, z80->L);
    break;
  case 0xA6:
    HLASINDEX(and(z80, z80->HL));
    break;
  case 0xA7:
    and(z80, z80->A);
    break;
  case 0xA8:
    xor(z80, z80->B);
    break;
  case 0xA9:
    xor(z80, z80->C);
    break;
  case 0xAA:
    xor(z80, z80->D);
    break;
  case 0xAB:
    xor(z80, z80->E);
    break;
  case 0xAC:
    xor(z80, z80->H);
    break;
  case 0xAD:
    xor(z80, z80->L);
    break;
  case 0xAE:
    HLASINDEX(xor(z80, z80->HL));
    break;
  case 0xAF:
    xor(z80, z80->A);
    break;
  case 0xB0:
    or (z80, z80->B);
    break;
  case 0xB1:
    or (z80, z80->C);
    break;
  case 0xB2:
    or (z80, z80->D);
    break;
  case 0xB3:
    or (z80, z80->E);
    break;
  case 0xB4:
    or (z80, z80->H);
    break;
  case 0xB5:
    or (z80, z80->L);
    break;
  case 0xB6:
    HLASINDEX(or (z80, z80->HL));
    break;
  case 0xB7:
    or (z80, z80->A);
    break;
  case 0xB8:
    cp(z80, z80->B);
    break;
  case 0xB9:
    cp(z80, z80->C);
    break;
  case 0xBA:
    cp(z80, z80->D);
    break;
  case 0xBB:
    cp(z80, z80->E);
    break;
  case 0xBC:
    cp(z80, z80->H);
    break;
  case 0xBD:
    cp(z80, z80->L);
    break;
  case 0xBE:
    HLASINDEX(cp(z80, z80->HL));
    break;
  case 0xBF:
    cp(z80, z80->A);
    break;
  case 0xC0:
    ret(z80, !readFlag(z80, Z80_ZF));
    break;
  case 0xC1:
    z80->BC = pop16(z80);
    z80->PC += 1;
    break;
  case 0xC2:
    jp(z80, !readFlag(z80, Z80_ZF));
    break;
  case 0xC3:
    jp(z80, true);
    break;
  case 0xC4:
    call(z80, !readFlag(z80, Z80_ZF));
    break;
  case 0xC5:
    push16(z80, z80->BC);
    z80->PC += 1;
    break;
  case 0xC6:
    add(z80, z80->A, readN(z80), &z80->A, 'b', false);
    break;
  case 0xC7:
    rst(z80, 0x0);
    break;
  case 0xC8:
    ret(z80, readFlag(z80, Z80_ZF));
    break;
  case 0xC9:
    ret(z80, true);
    break;
  case 0xCA:
    jp(z80, readFlag(z80, Z80_ZF));
    break;
  case 0xCB:
    z80->PC += 1;
    bitInstructions(z80, readMem(z80, z80->PC));
    break;
  case 0xCC:
    call(z80, readFlag(z80, Z80_ZF));
    break;
  case 0xCD:
    call(z80, true);
    break;
  case 0xCE:
    add(z80, z80->A, readN(z80), &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0xCF:
    rst(z80, 0x08);
    break;
  case 0xD0:
    ret(z80, !readFlag(z80, Z80_CF));
    break;
  case 0xD1:
    z80->DE = pop16(z80);
    z80->PC += 1;
    break;
  case 0xD2:
    jp(z80, !readFlag(z80, Z80_CF));
    break;
  case 0xD3:
    out(z80, z80->A << 8 | readN(z80), z80->A);
    break;
  case 0xD4:
    call(z80, !readFlag(z80, Z80_CF));
    break;
  case 0xD5:
    push16(z80, z80->DE);
    z80->PC += 1;
    break;
  case 0xD6:
    sub(z80, z80->A, readN(z80), &z80->A, 'b', false);
    break;
  case 0xD7:
    rst(z80, 0x10);
    break;
  case 0xD8:
    ret(z80, readFlag(z80, Z80_CF));
    break;
  case 0xD9:
    exchangeRegisters(z80, &z80->BC, &z80->BCp);
    exchangeRegisters(z80, &z80->DE, &z80->DEp);
    if (!isHL_IX_IY) {
      exchangeRegisters(z80, &z80->HL, &z80->HLp);
    } else {
      exchangeRegisters(z80, &hlStorage, &z80->HLp);
    }
    z80->PC -= 2;
    break;
  case 0xDA:
    jp(z80, readFlag(z80, Z80_CF));
    break;
  case 0xDB:
    z80->A = in(z80, (z80->A << 8 | readN(z80)));
    break;
  case 0xDC:
    call(z80, readFlag(z80, Z80_CF));
    break;
  case 0xDD:
    isHL_IX_IY = true;
    hlStorage = z80->HL;
    z80->HL = z80->IX;
    prefixInst(z80);
    z80->IX = z80->HL;
    z80->HL = hlStorage;
    isHL_IX_IY = false;
    break;
  case 0xDE:
    sub(z80, z80->A, readN(z80), &z80->A, 'b', readFlag(z80, Z80_CF));
    break;
  case 0xDF:
    rst(z80, 0x18);
    break;
  case 0xE0:
    ret(z80, !readFlag(z80, Z80_PF));
    break;
  case 0xE1:
    z80->HL = pop16(z80);
    z80->PC += 1;
    break;
  case 0xE2:
    jp(z80, !readFlag(z80, Z80_PF));
    break;
  case 0xE3:
    ss2 = z80->HL;
    z80->HL = pop16(z80);
    push16(z80, ss2);
    z80->PC += 1;
    break;
  case 0xE4:
    call(z80, !readFlag(z80, Z80_PF));
    break;
  case 0xE5:
    push16(z80, z80->HL);
    z80->PC += 1;
    break;
  case 0xE6:
    and(z80, readN(z80));
    break;
  case 0xE7:
    rst(z80, 0x20);
    break;
  case 0xE8:
    ret(z80, readFlag(z80, Z80_PF));
    break;
  case 0xE9:
    z80->PC = z80->HL;
    break;
  case 0xEA:
    jp(z80, readFlag(z80, Z80_PF));
    break;
  case 0xEB:
    if (!isHL_IX_IY) {
      exchangeRegisters(z80, &z80->HL, &z80->DE);
    } else {
      exchangeRegisters(z80, &hlStorage, &z80->DE);
    }
    break;
  case 0xEC:
    call(z80, readFlag(z80, Z80_PF));
    break;
  case 0xED:
    z80->PC += 1;
    miscInstructions(z80, readMem(z80, z80->PC));
    break;
  case 0xEE:
    xor(z80, readN(z80));
    break;
  case 0xEF:
    rst(z80, 0x28);
    break;
  case 0xF0:
    ret(z80, !readFlag(z80, Z80_SF));
    break;
  case 0xF1:
    z80->AF = pop16(z80);
    z80->PC += 1;
    break;
  case 0xF2:
    jp(z80, !readFlag(z80, Z80_SF));
    break;
  case 0xF3:
    z80->IFF1 = false;
    z80->IFF2 = false;
    z80->PC += 1;
    break;
  case 0xF4:
    call(z80, !readFlag(z80, Z80_SF));
    break;
  case 0xF5:
    push16(z80, z80->AF);
    z80->PC += 1;
    break;
  case 0xF6:
    or (z80, readN(z80));
    break;
  case 0xF7:
    rst(z80, 0x30);
    break;
  case 0xF8:
    ret(z80, readFlag(z80, Z80_SF));
    break;
  case 0xF9:
    LD(z80->SP, z80->HL);
    break;
  case 0xFA:
    jp(z80, readFlag(z80, Z80_SF));
    break;
  case 0xFB:
    z80->IFF1 = true;
    z80->IFF2 = true;
    z80->PC += 1;
    break;
  case 0xFC:
    call(z80, readFlag(z80, Z80_SF));
    break;
  case 0xFD:
    isHL_IX_IY = true;
    hlStorage = z80->HL;
    z80->HL = z80->IY;
    prefixInst(z80);
    z80->IY = z80->HL;
    z80->HL = hlStorage;
    isHL_IX_IY = false;
    break;
  case 0xFE:
    cp(z80, readN(z80));
    break;
  case 0xFF:
    rst(z80, 0x38);
    break;
  }
}
void bitInstructions(cpu_t *z80, uint8_t opcode) {
  uint8_t memAtHL;
  uint16_t var = 0;
  memAtHL = readMem(z80, z80->HL);
  var = z80->HL;
  if (isHL_IX_IY) {
    var = z80->HL + (int8_t)displacement;
    memAtHL = readMem(z80, var);
  }
  incR(z80);
  uint8_t *registers[8] = {&z80->B, &z80->C, &z80->D, &z80->E, &z80->H, &z80->L, &memAtHL, &z80->A};
  void (*shiftFunctions[4])(cpu_t *z80, char direction, void *val) = {rotateC, rotate, shift, logicalShift};
  void (*bitFunctions[3])(cpu_t *z80, int pos, uint8_t *ss2) = {bit, reset, set};
  if (opcode < 0x40) {
    if ((opcode & 0xF) > 0x7) {
      shiftFunctions[opcode / 16](z80, 'r', registers[opcode % 8]);
    } else {
      shiftFunctions[opcode / 16](z80, 'l', registers[opcode % 8]);
    }
  } else {
    bitFunctions[(opcode - 0x40) / 64](z80, ((opcode - 0x40) % 64) / 8, registers[opcode % 8]);
  }
  writeMem(z80, var, memAtHL);
}
void miscInstructions(cpu_t *z80, uint8_t opcode) {
  incR(z80);
  switch (opcode) {
  case 0x40:
    z80->B = in(z80, z80->BC);
    clearFlag(z80, Z80_NF);
    clearFlag(z80, Z80_HF);
    checkSet(Z80_PF, getParity(z80->B));
    checkSet(Z80_SF, (z80->B & 0x80) != 0);
    checkSet(Z80_ZF, z80->B == 0);
    setUndocumentedFlags(z80, z80->B);
    break;
  case 0x41:
    out(z80, z80->BC, z80->B);
    break;
  case 0x42:
    sbcHL(z80, z80->BC);
    break;
  case 0x43:
    ss1 = readNN(z80);
    writeMem(z80, ss1, z80->C);
    writeMem(z80, ss1 + 1, z80->B);
    z80->PC += 1;
    break;
  case 0x44:
    sub(z80, 0, z80->A, &z80->A, 'b', false);
    break;
  case 0x45:
    z80->IFF1 = z80->IFF2;
    z80->PC = pop16(z80);
    break;
  case 0x46:
    z80->IM = 0;
    z80->PC += 1;
    break;
  case 0x47:
    LD(z80->I, z80->A);
    break;
  case 0x48:
    z80->C = in(z80, z80->BC);
    clearFlag(z80, Z80_NF);
    clearFlag(z80, Z80_HF);
    checkSet(Z80_PF, getParity(z80->C));
    checkSet(Z80_SF, (z80->C & 0x80) != 0);
    checkSet(Z80_ZF, z80->C == 0);
    setUndocumentedFlags(z80, z80->C);
    break;
  case 0x49:
    out(z80, z80->BC, z80->C);
    break;
  case 0x4A:
    adcHL(z80, z80->BC);
    break;
  case 0x4B:
    LD(z80->BC, nnAsPointer(z80));
    break;
  case 0x4C:
    sub(z80, 0, z80->A, &z80->A, 'b', false);
    break;
  case 0x4D:
    z80->IFF1 = z80->IFF2;
    ret(z80, true);
    break;
  case 0x4E:
    z80->IM = 0;
    z80->PC += 1;
    break;
  case 0x4F:
    LD(z80->R, z80->A);
    break;
  case 0x50:
    z80->D = in(z80, z80->BC);
    clearFlag(z80, Z80_NF);
    clearFlag(z80, Z80_HF);
    checkSet(Z80_PF, getParity(z80->D));
    checkSet(Z80_SF, (z80->D & 0x80) != 0);
    checkSet(Z80_ZF, z80->D == 0);
    setUndocumentedFlags(z80, z80->D);
    break;
  case 0x51:
    out(z80, z80->BC, z80->D);
    break;
  case 0x52:
    sbcHL(z80, z80->DE);
    break;
  case 0x53:
    ss1 = readNN(z80);
    writeMem(z80, ss1, z80->E);
    writeMem(z80, ss1 + 1, z80->D);
    z80->PC += 1;
    break;
  case 0x54:
    sub(z80, 0, z80->A, &z80->A, 'b', false);
    break;
  case 0x55:
    z80->IFF1 = z80->IFF2;
    z80->PC = pop16(z80);
    break;
  case 0x56:
    z80->IM = 1;
    z80->PC += 1;
    break;
  case 0x57:
    LD(z80->A, z80->I);
    clearFlag(z80, Z80_HF);
    clearFlag(z80, Z80_NF);
    checkSet(Z80_ZF, z80->A == 0);
    checkSet(Z80_SF, (z80->A & 0x80) != 0);
    checkSet(Z80_PF, z80->IFF2);
    setUndocumentedFlags(z80, z80->A);
    break;
  case 0x58:
    z80->E = in(z80, z80->BC);
    clearFlag(z80, Z80_NF);
    clearFlag(z80, Z80_HF);
    checkSet(Z80_PF, getParity(z80->E));
    checkSet(Z80_SF, (z80->E & 0x80) != 0);
    checkSet(Z80_ZF, z80->E == 0);
    setUndocumentedFlags(z80, z80->E);
    break;
  case 0x59:
    out(z80, z80->BC, z80->E);
    break;
  case 0x5A:
    adcHL(z80, z80->DE);
    break;
  case 0x5B:
    LD(z80->DE, nnAsPointer(z80));
    break;
  case 0x5C:
    sub(z80, 0, z80->A, &z80->A, 'b', false);
    break;
  case 0x5D:
    z80->IFF1 = z80->IFF2;
    ret(z80, true);
    break;
  case 0x5E:
    z80->IM = 2;
    z80->PC += 1;
    break;
  case 0x5F:
    LD(z80->A, z80->R);
    clearFlag(z80, Z80_HF);
    clearFlag(z80, Z80_NF);
    checkSet(Z80_ZF, z80->A == 0);
    checkSet(Z80_SF, (z80->A & 0x80) != 0);
    checkSet(Z80_PF, z80->IFF2);
    setUndocumentedFlags(z80, z80->A);
    break;
  case 0x60:
    z80->H = in(z80, z80->BC);
    clearFlag(z80, Z80_NF);
    clearFlag(z80, Z80_HF);
    checkSet(Z80_PF, getParity(z80->H));
    checkSet(Z80_SF, (z80->H & 0x80) != 0);
    checkSet(Z80_ZF, z80->H == 0);
    setUndocumentedFlags(z80, z80->H);
    break;
  case 0x61:
    out(z80, z80->BC, z80->H);
    break;
  case 0x62:
    sbcHL(z80, z80->HL);
    break;
  case 0x63:
    ss1 = readNN(z80);
    writeMem(z80, ss1, z80->L);
    writeMem(z80, ss1 + 1, z80->H);
    z80->PC += 1;
    break;
  case 0x64:
    sub(z80, 0, z80->A, &z80->A, 'b', false);
    break;
  case 0x65:
    z80->IFF1 = z80->IFF2;
    z80->PC = pop16(z80);
    break;
  case 0x66:
    z80->IM = 0;
    z80->PC += 1;
    break;
  case 0x67:
    ss1 = z80->A;
    ss2 = readMem(z80, z80->HL);
    z80->A = ((ss1 & 0xF0) | (ss2 & 0xF));
    writeMem(z80, z80->HL, (ss2 >> 4) | (ss1 << 4));
    clearFlag(z80, Z80_NF | Z80_HF);
    checkSet(Z80_PF, getParity(z80->A));
    checkSet(Z80_ZF, z80->A == 0);
    checkSet(Z80_SF, (z80->A & 0x80) != 0);
    setUndocumentedFlags(z80, z80->A);
    z80->PC += 1;
    break;
  case 0x68:
    z80->L = in(z80, z80->BC);
    clearFlag(z80, Z80_NF | Z80_HF);
    checkSet(Z80_PF, getParity(z80->L));
    checkSet(Z80_SF, (z80->L & 0x80) != 0);
    checkSet(Z80_ZF, z80->L == 0);
    setUndocumentedFlags(z80, z80->L);
    break;
  case 0x69:
    out(z80, z80->BC, z80->L);
    break;
  case 0x6A:
    adcHL(z80, z80->HL);
    break;
  case 0x6B:
    LD(z80->HL, nnAsPointer(z80));
    break;
  case 0x6C:
    sub(z80, 0, z80->A, &z80->A, 'b', false);
    break;
  case 0x6D:
    z80->IFF1 = z80->IFF2;
    ret(z80, true);
    break;
  case 0x6E:
    z80->IM = 0;
    z80->PC += 1;
    break;
  case 0x6F:
    ss1 = z80->A;
    ss2 = readMem(z80, z80->HL);
    z80->A = ((ss1 & 0xF0) | (ss2 >> 4));
    writeMem(z80, z80->HL, ((ss2 << 4) | (ss1 & 0xF)));
    clearFlag(z80, Z80_NF | Z80_HF);
    checkSet(Z80_PF, getParity(z80->A));
    checkSet(Z80_ZF, z80->A == 0);
    checkSet(Z80_SF, (z80->A & 0x80) != 0);
    setUndocumentedFlags(z80, z80->A);
    z80->PC += 1;
    break;
  case 0x70:
    ss1 = in(z80, z80->BC);
    clearFlag(z80, Z80_NF);
    clearFlag(z80, Z80_HF);
    checkSet(Z80_PF, getParity(ss1));
    checkSet(Z80_SF, (ss1 >> 7));
    checkSet(Z80_ZF, ss1 == 0);
    setUndocumentedFlags(z80, ss1);
    break;
  case 0x71:
    out(z80, z80->BC, 0);
    break;
  case 0x72:
    sbcHL(z80, z80->SP);
    break;
  case 0x73:
    write16(z80, readNN(z80), z80->SP);
    break;
  case 0x74:
    sub(z80, 0, z80->A, &z80->A, 'b', false);
    break;
  case 0x75:
    z80->IFF1 = z80->IFF2;
    z80->PC = pop16(z80);
    break;
  case 0x76:
    z80->IM = 1;
    z80->PC += 1;
    break;
  case 0x77:
    z80->PC += 1;
    break;
  case 0x78:
    z80->A = in(z80, z80->BC);
    clearFlag(z80, Z80_NF);
    clearFlag(z80, Z80_HF);
    checkSet(Z80_PF, getParity(z80->A));
    checkSet(Z80_SF, (z80->A & 0x80) != 0);
    checkSet(Z80_ZF, z80->A == 0);
    setUndocumentedFlags(z80, z80->A);
    break;
  case 0x79:
    out(z80, z80->BC, z80->A);
    break;
  case 0x7A:
    adcHL(z80, z80->SP);
    break;
  case 0x7B:
    LD(z80->SP, nnAsPointer(z80));
    break;
  case 0x7C:
    sub(z80, 0, z80->A, &z80->A, 'b', false);
    break;
  case 0x7D:
    z80->IFF1 = z80->IFF2;
    z80->PC = pop16(z80);
    break;
  case 0x7E:
    z80->IM = 2;
    z80->PC += 1;
    break;
  case 0x7F:
    z80->PC += 1;
    break;
  case 0xA0:
    ss1 = readMem(z80, z80->HL++);
    clearFlag(z80, Z80_NF | Z80_HF);
    writeMem(z80, z80->DE++, ss1);
    ss1 += z80->A;
    checkSet(Z80_PF, --z80->BC);
    checkSet(Z80_F3, ss1 & Z80_F3);
    checkSet(Z80_F5, ss1 & 0x2);
    z80->PC += 1;
    break;
  case 0xB0:
    ss1 = readMem(z80, z80->HL++);
    clearFlag(z80, Z80_NF | Z80_HF);
    writeMem(z80, z80->DE++, ss1);
    ss1 += z80->A;
    checkSet(Z80_PF, --z80->BC);
    checkSet(Z80_F3, ss1 & Z80_F3);
    checkSet(Z80_F5, ss1 & 0x2);
    if (z80->BC != 0) {
      z80->PC -= 2;
      setUndocumentedFlags(z80, (z80->PC) >> 8);
    }
    z80->PC += 1;
    break;
  case 0xA1:
    cpi(z80);
    break;
  case 0xB1:
    cpi(z80);
    if (z80->BC != 0 && !readFlag(z80, Z80_ZF)) {
      z80->PC -= 2;
    }
    break;
  case 0xA8:
    ss1 = readMem(z80, z80->HL--);
    clearFlag(z80, Z80_NF | Z80_HF);
    writeMem(z80, z80->DE--, ss1);
    ss1 += z80->A;
    checkSet(Z80_PF, --z80->BC);
    checkSet(Z80_F3, ss1 & Z80_F3);
    checkSet(Z80_F5, ss1 & 0x2);
    z80->PC += 1;
    break;
  case 0xB8:
    ss1 = readMem(z80, z80->HL--);
    clearFlag(z80, Z80_NF | Z80_HF);
    writeMem(z80, z80->DE--, ss1);
    ss1 += z80->A;
    checkSet(Z80_PF, --z80->BC);
    checkSet(Z80_F3, ss1 & Z80_F3);
    checkSet(Z80_F5, ss1 & 0x2);
    if (z80->BC != 0) {
      z80->PC -= 2;
      setUndocumentedFlags(z80, (z80->PC) >> 8);
    }
    z80->PC += 1;
    break;
  case 0xA9:
    cpd(z80);
    break;
  case 0xB9:
    cpd(z80);
    if (z80->BC != 0 && !readFlag(z80, Z80_ZF)) {
      z80->PC -= 2;
    }
    break;
  case 0xA2:
    // INI
    ini(z80);
    break;
  case 0xB2:
    // INIR
    ini(z80);
    if (z80->B != 0) {
      z80->PC -= 2;
      postINI_OUTI_R(z80, ss2);
    }
    break;
  case 0xAA:
    // IND
    ss2 = in(z80, z80->BC);
    writeMem(z80, z80->HL, ss2);
    dec8(z80, &z80->B);
    z80->PC -= 1;
    z80->HL--;
    checkSet(Z80_NF, ss2 >> 7);
    checkSet(Z80_HF | Z80_CF, (ss2 + ((z80->C - 1) & 255) > 255));
    checkSet(Z80_PF, getParity(((ss2 + ((z80->C - 1) & 255)) & 7) ^ z80->B));
    break;
  case 0xBA:
    // INDR
    ss2 = in(z80, z80->BC);
    writeMem(z80, z80->HL, ss2);
    dec8(z80, &z80->B);
    z80->PC -= 1;
    z80->HL--;
    checkSet(Z80_NF, ss2 >> 7);
    checkSet(Z80_HF | Z80_CF, (ss2 + ((z80->C - 1) & 255) > 255));
    checkSet(Z80_PF, getParity(((ss2 + ((z80->C - 1) & 255)) & 7) ^ z80->B));
    if (z80->B != 0) {
      z80->PC -= 2;
      postINI_OUTI_R(z80, ss2);
    }
    break;
  case 0xA3:
    // OUTI
    ss2 = readMem(z80, z80->HL);
    dec8(z80, &z80->B);
    out(z80, z80->BC, ss2);
    z80->PC -= 1;
    z80->HL++;
    checkSet(Z80_NF, ss2 >> 7);
    checkSet(Z80_HF | Z80_CF, (ss2 + z80->L > 255));
    checkSet(Z80_PF, getParity((((ss2 + z80->L) & 7) ^ z80->B)));
    break;
  case 0xB3:
    // OUTIR
    ss2 = readMem(z80, z80->HL);
    dec8(z80, &z80->B);
    out(z80, z80->BC, ss2);
    z80->PC -= 1;
    z80->HL++;
    checkSet(Z80_NF, ss2 >> 7);
    checkSet(Z80_HF | Z80_CF, (ss2 + z80->L > 255));
    checkSet(Z80_PF, getParity((((ss2 + z80->L) & 7) ^ z80->B)));
    if (z80->B != 0) {
      z80->PC -= 2;
      postINI_OUTI_R(z80, ss2);
    }
    break;
  case 0xAB:
    // OUTD
    ss2 = readMem(z80, z80->HL);
    dec8(z80, &z80->B);
    out(z80, z80->BC, ss2);
    z80->PC -= 1;
    z80->HL--;
    checkSet(Z80_NF, ss2 >> 7);
    checkSet(Z80_HF | Z80_CF, (ss2 + z80->L > 255));
    checkSet(Z80_PF, getParity((((ss2 + z80->L) & 7) ^ z80->B)));
    break;
  case 0xBB:
    // OUTDR
    ss2 = readMem(z80, z80->HL);
    dec8(z80, &z80->B);
    out(z80, z80->BC, ss2);
    z80->PC -= 1;
    z80->HL--;
    checkSet(Z80_NF, ss2 >> 7);
    checkSet(Z80_HF | Z80_CF, (ss2 + z80->L > 255));
    checkSet(Z80_PF, getParity((((ss2 + z80->L) & 7) ^ z80->B)));
    if (z80->B != 0) {
      z80->PC -= 2;
    }
    postINI_OUTI_R(z80, ss2);
    break;
  }
}
