#include "common.h"
#include "z80.h"
#define LD(B, A) (B = A); z80.PC += 2;
void setFlag(int flag) { z80.F |= flag; }
void clearFlag(int flag) { z80.F &= ~(flag); }
bool readFlag(int flag) { return z80.F & flag; }

static inline uint16_t getVal16() {
  return readMem(z80.PC + 1) | readMem(z80.PC + 2) << 8;
}


static inline uint8_t getVal8() {
  return readMem(z80.PC + 1);
}

static inline uint16_t getValueAtMemory(){
  int address = readMem(getVal16()) | readMem(getVal16() + 1) << 8;
  z80.PC += 1;
  return address;
}

static inline void checkSet(int flag, bool conditon) {
  if (conditon) {
    setFlag(flag);
  } else {
    clearFlag(flag);
  }
}


void checkAndSetSign(uint32_t number, char size) {
  if (((number & 0x80) != 0) && (size == 'b')) {
    setFlag(Z80_SF);
  } else if (((number & 0x8000) != 0) && (size == 'w')) {
    setFlag(Z80_SF);
  } else {
    clearFlag(Z80_SF);
  }
}


void checkAndSetZero(int number) {
  if ((number == 0)) {
    setFlag(Z80_ZF);
  } else {
    clearFlag(Z80_ZF);
  }
}


void setUndocumentedFlags(int result) {
  if (result & Z80_F3) {
    setFlag(Z80_F3);
  } else {
    clearFlag(Z80_F3);
  }
  if (result & Z80_F5) {
    setFlag(Z80_F5);
  } else {
    clearFlag(Z80_F5);
  }
}


void checkAndSetPV(int *result, int prev, char PV, char width) {
  if (width == 'b' && PV == 'v') {
    if (((prev & 0x80) == 0 && (*result & 0x80) != 0) || ((prev & 0x80) != 0 && (*result & 0x80) == 0)) {
      setFlag(Z80_PF);
    } else {
      clearFlag(Z80_PF);
    }
  } else {
    clearFlag(Z80_PF);
  }
}


static inline void add(int A, int B, void *storeLocation, char width) {
  int result = A + B;
  clearFlag(Z80_NF);
  if (width == 'b') {
    setUndocumentedFlags(result);
    checkSet(Z80_ZF, (uint8_t)result == 0);
    checkSet(Z80_PF, ((A & 0x80) == 0 && (result & 0x80) != 0));
    checkSet(Z80_SF, (result & 0x80) != 0);
    checkSet(Z80_HF, ((result & 0xF) << 4) <= 0);
    checkSet(Z80_CF, result > 255);
    *(uint8_t *)storeLocation = result;
  } else if (width == 'w') {
    setUndocumentedFlags(result >> 8);
    checkSet(Z80_HF, (((A ^ (B) ^ result) >> 8) & Z80_HF));
    checkSet(Z80_CF, result > 65535);
    *(uint16_t *)storeLocation = result;
  }
  z80.PC += 1;
}


static inline void sub(int A, int B, void *storeLocation, char width) {
  int result = A - B;
  setFlag(Z80_NF);
  setUndocumentedFlags(result);
  if (width == 'b') {
    if ((uint8_t)result == 0) {
      setFlag(Z80_ZF);
    } else {
      clearFlag(Z80_ZF);
    }
    if (((A & 0x80) != 0 && (result & 0x80) == 0)) {
      setFlag(Z80_PF);
    } else {
      clearFlag(Z80_PF);
    }
    if ((result & 0x80) != 0) {
      setFlag(Z80_SF);
    } else {
      clearFlag(Z80_SF);
    }
    if ((A & 0x0F) < (B & 0x0F)) {
      setFlag(Z80_HF);
    } else {
      clearFlag(Z80_HF);
    }
    *(uint8_t *)storeLocation = result;
  }
  z80.PC += 1;
}


static inline void rotateC(char direction, void *val, char size) {
  int result;
  clearFlag(Z80_HF);
  clearFlag(Z80_NF);
  if (size == 'b') {
    if (direction == 'l') {
      result = (*(uint8_t *)val << 1) | (*(uint8_t *)val >> 7);
      checkSet(Z80_CF,(result & 1) != 0);
    } else if (direction == 'r') {
      result = (*(uint8_t *)val >> 1) | (*(uint8_t *)val << 7);
      checkSet(Z80_CF,(result & 0x80) != 0);
    }

    setUndocumentedFlags(result);
    *(uint8_t *)val = result;
    z80.PC += 1;
  }
}


static inline void rotate(char direction, void *val, char size) {
  int result;
  clearFlag(Z80_HF);
  clearFlag(Z80_NF);
  if (size == 'b') { 
     uint8_t value = *(uint8_t *)val;
    if (direction == 'l') {
      result = (value << 1) | (value >> 8) | readFlag(Z80_CF);
      checkSet(Z80_CF, result & 0x100);
    } else if (direction == 'r') { 
      result = (value >> 1) | (readFlag(Z80_CF) << 7);
      checkSet(Z80_CF, (value & 0x80) != 0);
    }
    setUndocumentedFlags(result);
    *(uint8_t *)val = result;
    z80.PC += 1;
  }
}


static inline void exchangeRegisters(uint16_t *A, uint16_t *B) {
  uint16_t temp;
  temp = *B;
  *B = *A;
  *A = temp;
  z80.PC += 1;
}


void jr(bool condition){
 if(condition){
  z80.PC += (int8_t)readMem(z80.PC + 1); 
} 
 z80.PC += 2;
}


void dnjz(){
  z80.B--;
  if (z80.B != 0){
    z80.PC += (int8_t)getVal8();
  }
  z80.PC += 2;
}
//Algorithm adapted from:
  //https://worldofspectrum.org/faq/reference/z80reference.htm#DAA
void daa(){
  
int afterA = z80.A;
  int correctFactor = 0;
 if(z80.A > 0x99 || readFlag(Z80_CF)){
  correctFactor += 0x60;
  setFlag(Z80_CF);
 }else{
  clearFlag(Z80_CF);
 }
 if((z80.A & 0xF) > 9 || readFlag(Z80_HF)){
  correctFactor += 0x6;
 }
 if(!readFlag(Z80_NF)){
  afterA += correctFactor;
 }else{
  afterA -= correctFactor;
 }
 setUndocumentedFlags(afterA);
 checkSet(Z80_HF, ((z80.A & 0x0F) + (afterA & 0x0F)) > 0x0F);
 checkSet(Z80_ZF, afterA == 0);
 //checkSet(Z80_PF, parity(afterA));
 checkSet(Z80_SF, (afterA & 0x80) != 0 );
 z80.A = afterA;
 z80.PC += 1;
}



void cpl(){
  setFlag(Z80_NF);
  setFlag(Z80_HF);
  z80.A = ~z80.A;
  setUndocumentedFlags(z80.A);
  z80.PC += 1;
}

static inline void inc8(uint8_t *value) {
  bool oldCarry = readFlag(Z80_CF);
  add(*value, 1, value, 'b');
  checkSet(Z80_CF, oldCarry);
}


static inline void inc16(uint16_t *value){
  *value += 1;
  z80.PC += 1;
  }

static inline void dec8(uint8_t *value) {
  sub(*value, 1, value, 'b');
}

static inline void dec16(uint16_t *value){
  *value -= 1;
  z80.PC += 1;
  }


void runOpcode() {
  switch (readMem(z80.PC)) {
  case 0xCB:
    switch (readMem(z80.PC + 1)) {}
    break;
  case 0xED:
    switch (readMem(z80.PC + 1)) {}
    break;
  case 0xDD:
    switch (readMem(z80.PC + 1)) {}
    break;
  case 0xFD:
    switch (readMem(z80.PC + 1)) {}
    break;
  case 0x0:
    z80.PC += 1;
    break;
  case 0x1:
  LD(z80.BC, getVal16());
  break;
  case 0x2:
  writeMem(readMem(z80.BC), z80.A);
  z80.PC += 1;
    break;
  case 0x3:
inc16(&z80.BC);
    break;
  case 0x4:
    inc8(&z80.B);
    break;
  case 0x5:
    dec8(&z80.B);
    break;
  case 0x6:
    LD(z80.B, z80.PC);
    break;
  case 0x7:
    rotateC('l', &z80.A, 'b');
    break;
  case 0x8:
    exchangeRegisters(&z80.AF, &z80.AFp);
    break;
  case 0x9:
    add(z80.BC, z80.HL, &z80.HL, 'w');
    break;
  case 0xA:
  LD(z80.A,readMem(z80.BC));
    break;
  case 0xB:
  dec16(&z80.BC);
  z80.PC += 1;
    break;
  case 0xC:
  inc8(&z80.C);
    break;
  case 0xD:
    dec8(&z80.C);
    break;
  case 0xE:
  LD(z80.C, getVal8());
  break;
  case 0xF:
  rotateC('r',&z80.A,'b');
    break;
  case 0x10:
  dnjz();
    break;
  case 0x11:
  LD(z80.DE, getVal16());
  break;
  case 0x12:
  writeMem(readMem(z80.DE), z80.A);
  z80.PC += 1;
    break;
  case 0x13:
  inc16(&z80.DE);
    break;
  case 0x14:
  inc8(&z80.D);
    break;
  case 0x15:
  dec8(&z80.D);
    break;
  case 0x16:
  LD(z80.D, getVal8());
  break;
  case 0x17:
  rotate('l',&z80.A,'b');
    break;
  case 0x18:
    jr(true);
    break;
  case 0x19:
    add(z80.DE, z80.HL, &z80.HL, 'w');
    break;
  case 0x1A:
    LD(z80.A, readMem(z80.DE));
    break;
  case 0x1B:
  dec16(&z80.DE);
    break;
  case 0x1C:
  inc8(&z80.E);
    break;
  case 0x1D:
  dec8(&z80.E);
    break;
  case 0x1E:
  LD(z80.E, getVal8());
    break;
  case 0x1F:
  rotate('r',&z80.A,'b');
    break;
  case 0x20:
  jr(!readFlag(Z80_ZF));
    break;
  case 0x21:
  LD(z80.HL, getVal16());
    break;
  case 0x22:
  writeMem(getVal16(), z80.HL);
  z80.PC += 2;
    break;
  case 0x23:
  inc16(&z80.HL);
    break;
  case 0x24:
  inc8(&z80.H);
    break;
  case 0x25:
  dec8(&z80.H);
    break;
  case 0x26:
  LD(z80.H, getVal8());
    break;
  case 0x27:
    daa();
    break;
  case 0x28:
  jr(readFlag(Z80_ZF));
    break;
  case 0x29:
  add(z80.HL,z80.HL,&z80.HL, 'w');
    break;
  case 0x2A:
  LD(z80.HL, getValueAtMemory());
    break;
  case 0x2B:
  dec16(&z80.HL);
    break;
  case 0x2C:
  inc8(&z80.L);
    break;
  case 0x2D:
  dec8(&z80.L);
    break;
  case 0x2E:
  LD(z80.L, getVal8());
    break;
  case 0x2F:
  cpl();
    break;
  case 0x30:
    break;
  case 0x31:
    break;
  case 0x32:
    break;
  case 0x33:
    break;
  case 0x34:
    break;
  case 0x35:
    break;
  case 0x36:
    break;
  case 0x37:
    break;
  case 0x38:
    break;
  case 0x39:
    break;
  case 0x3A:
    break;
  case 0x3B:
    break;
  case 0x3C:
    break;
  case 0x3D:
    break;
  case 0x3E:
    break;
  case 0x3F:
    break;
  case 0x40:
    break;
  case 0x41:
    break;
  case 0x42:
    break;
  case 0x43:
    break;
  case 0x44:
    break;
  case 0x45:
    break;
  case 0x46:
    break;
  case 0x47:
    break;
  case 0x48:
    break;
  case 0x49:
    break;
  case 0x4A:
    break;
  case 0x4B:
    break;
  case 0x4C:
    break;
  case 0x4D:
    break;
  case 0x4E:
    break;
  case 0x4F:
    break;
  case 0x50:
    break;
  case 0x51:
    break;
  case 0x52:
    break;
  case 0x53:
    break;
  case 0x54:
    break;
  case 0x55:
    break;
  case 0x56:
    break;
  case 0x57:
    break;
  case 0x58:
    break;
  case 0x59:
    break;
  case 0x5A:
    break;
  case 0x5B:
    break;
  case 0x5C:
    break;
  case 0x5D:
    break;
  case 0x5E:
    break;
  case 0x5F:
    break;
  case 0x60:
    break;
  case 0x61:
    break;
  case 0x62:
    break;
  case 0x63:
    break;
  case 0x64:
    break;
  case 0x65:
    break;
  case 0x66:
    break;
  case 0x67:
    break;
  case 0x68:
    break;
  case 0x69:
    break;
  case 0x6A:
    break;
  case 0x6B:
    break;
  case 0x6C:
    break;
  case 0x6D:
    break;
  case 0x6E:
    break;
  case 0x6F:
    break;
  case 0x70:
    break;
  case 0x71:
    break;
  case 0x72:
    break;
  case 0x73:
    break;
  case 0x74:
    break;
  case 0x75:
    break;
  case 0x76:
    break;
  case 0x77:
    break;
  case 0x78:
    break;
  case 0x79:
    break;
  case 0x7A:
    break;
  case 0x7B:
    break;
  case 0x7C:
    break;
  case 0x7D:
    break;
  case 0x7E:
    break;
  case 0x7F:
    break;
  case 0x80:
    break;
  case 0x81:
    break;
  case 0x82:
    break;
  case 0x83:
    break;
  case 0x84:
    break;
  case 0x85:
    break;
  case 0x86:
    break;
  case 0x87:
    break;
  case 0x88:
    break;
  case 0x89:
    break;
  case 0x8A:
    break;
  case 0x8B:
    break;
  case 0x8C:
    break;
  case 0x8D:
    break;
  case 0x8E:
    break;
  case 0x8F:
    break;
  case 0x90:
    break;
  case 0x91:
    break;
  case 0x92:
    break;
  case 0x93:
    break;
  case 0x94:
    break;
  case 0x95:
    break;
  case 0x96:
    break;
  case 0x97:
    break;
  case 0x98:
    break;
  case 0x99:
    break;
  case 0x9A:
    break;
  case 0x9B:
    break;
  case 0x9C:
    break;
  case 0x9D:
    break;
  case 0x9E:
    break;
  case 0x9F:
    break;
  case 0xA0:
    break;
  case 0xA1:
    break;
  case 0xA2:
    break;
  case 0xA3:
    break;
  case 0xA4:
    break;
  case 0xA5:
    break;
  case 0xA6:
    break;
  case 0xA7:
    break;
  case 0xA8:
    break;
  case 0xA9:
    break;
  case 0xAA:
    break;
  case 0xAB:
    break;
  case 0xAC:
    break;
  case 0xAD:
    break;
  case 0xAE:
    break;
  case 0xAF:
    break;
  case 0xB0:
    break;
  case 0xB1:
    break;
  case 0xB2:
    break;
  case 0xB3:
    break;
  case 0xB4:
    break;
  case 0xB5:
    break;
  case 0xB6:
    break;
  case 0xB7:
    break;
  case 0xB8:
    break;
  case 0xB9:
    break;
  case 0xBA:
    break;
  case 0xBB:
    break;
  case 0xBC:
    break;
  case 0xBD:
    break;
  case 0xBE:
    break;
  case 0xBF:
    break;
  case 0xC0:
    break;
  case 0xC1:
    break;
  case 0xC2:
    break;
  case 0xC3:
    break;
  case 0xC4:
    break;
  case 0xC5:
    break;
  case 0xC6:
    break;
  case 0xC7:
    break;
  case 0xC8:
    break;
  case 0xCA:
    break;
  case 0xCC:
    break;
  case 0xCD:
    break;
  case 0xCE:
    break;
  case 0xCF:
    break;
  case 0xD0:
    break;
  case 0xD1:
    break;
  case 0xD2:
    break;
  case 0xD3:
    break;
  case 0xD4:
    break;
  case 0xD5:
    break;
  case 0xD6:
    break;
  case 0xD7:
    break;
  case 0xD8:
    break;
  case 0xD9:
    break;
  case 0xDA:
    break;
  case 0xDB:
    break;
  case 0xDC:
    break;

  case 0xDE:
    break;
  case 0xDF:
    break;
  case 0xE0:
    break;
  case 0xE1:
    break;
  case 0xE2:
    break;
  case 0xE3:
    break;
  case 0xE4:
    break;
  case 0xE5:
    break;
  case 0xE6:
    break;
  case 0xE7:
    break;
  case 0xE8:
    break;
  case 0xE9:
    break;
  case 0xEA:
    break;
  case 0xEB:
    break;
  case 0xEC:
    break;

  case 0xEE:
    break;
  case 0xEF:
    break;
  case 0xF0:
    break;
  case 0xF1:
    break;
  case 0xF2:
    break;
  case 0xF3:
    break;
  case 0xF4:
    break;
  case 0xF5:
    break;
  case 0xF6:
    break;
  case 0xF7:
    break;
  case 0xF8:
    break;
  case 0xF9:
    break;
  case 0xFA:
    break;
  case 0xFB:
    break;
  case 0xFC:
    break;
  case 0xFE:
    break;
  case 0xFF:
    break;
  default:
    printf("Unhandeld Opcode, 0x%X", z80.PC);
  }
  if (z80.R + 1 == 0x80) {
    z80.R = 0;
  } else {
    z80.R++;
  }
}
