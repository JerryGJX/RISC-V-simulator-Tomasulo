//
// Created by JerryGuo on 2022/6/28.
//

#ifndef RISC_V_TOMASULO__DECODER_HPP_
#define RISC_V_TOMASULO__DECODER_HPP_

#include <string>
#include "memory.hpp"
#include "mydef.hpp"
//#include <tuple>
#include <map>
#include <bitset>
#include <ostream>
#include "exceptional.hpp"

namespace RISC_V {
class Decoder {
  static opType getOpType(uint32_t opcode, uint32_t func3, uint32_t func7) {
    switch (opcode) {
      case 0b0110111: return LUI;
      case 0b0010111: return AUIPC;
      case 0b1101111: return JAL;
      case 0b1100111: return JALR;
      case 0b1100011: {
        switch (func3) {
          case 0b000: return BEQ;
          case 0b001: return BNE;
          case 0b100: return BLT;
          case 0b101: return BGE;
          case 0b110: return BLTU;
          case 0b111: return BGEU;
          default:return WRONG;
        }
      }
      case 0b0000011: {
        switch (func3) {
          case 0b000: return LB;
          case 0b001: return LH;
          case 0b010: return LW;
          case 0b100: return LBU;
          case 0b101: return LHU;
          default:return WRONG;
        }
      }
      case 0b0100011: {
        switch (func3) {
          case 0b000: return SB;
          case 0b001: return SH;
          case 0b010: return SW;
          default:return WRONG;
        }
      }
      case 0b0010011: {
        switch (func3) {
          case 0b000: return ADDI;
          case 0b010: return SLTI;
          case 0b011: return SLTIU;
          case 0b100: return XORI;
          case 0b110: return ORI;
          case 0b111: return ANDI;
          case 0b001: return SLLI;
          case 0b101: return (func7 != 0) ? SRAI : SRLI;
          default:return WRONG;
        }
      }
      case 0b0110011: {
        switch (func3) {
          case 0b000: return (func7 != 0) ? SUB : ADD;
          case 0b001: return SLL;
          case 0b010: return SLT;
          case 0b011: return SLTU;
          case 0b100: return XOR;
          case 0b101: return (func7 != 0) ? SRA : SRL;
          case 0b110: return OR;
          case 0b111: return AND;
          default:return WRONG;
        }
      }
      default:return WRONG;
    }
  }

  static uint32_t getImm(const instructionType &ins_type, uint32_t ins) {
    uint32_t ans = 0;
    switch (ins_type) {
      case R_type:return ans;
      case I_type:
        ans = (getPart(ins, 31, 31) << 11)
            + (getPart(ins, 30, 25) << 5)
            + (getPart(ins, 24, 21) << 1)
            + getPart(ins, 20, 20);
        ans = Sext(ans, 11);
        return ans;
      case S_type:
        ans = (getPart(ins, 31, 31) << 11)
            + (getPart(ins, 30, 25) << 5)
            + (getPart(ins, 11, 8) << 1)
            + getPart(ins, 7, 7);
        ans = Sext(ans, 11);
        return ans;
      case B_type:
        ans = (getPart(ins, 31, 31) << 12)
            + (getPart(ins, 7, 7) << 11)
            + (getPart(ins, 30, 25) << 5)
            + (getPart(ins, 11, 8) << 1);
        ans = Sext(ans, 12);
        return ans;
      case U_type:
        ans = (getPart(ins, 31, 31) << 31)
            + (getPart(ins, 30, 20) << 20)
            + (getPart(ins, 19, 12) << 12);
        return ans;
      case J_type:
        ans = (getPart(ins, 31, 31) << 20)
            + (getPart(ins, 19, 12) << 12)
            + (getPart(ins, 20, 20) << 11)
            + (getPart(ins, 30, 25) << 5)
            + (getPart(ins, 24, 21) << 1);
        ans = Sext(ans, 20);
        return ans;
      default:throw Error("Imm cal error");
    }
  }

 public:
  inline static uint32_t getPart(uint32_t ins, int high, int low) {
    if (high == 31)return ins >> low;
    return (ins & (1u << (high + 1)) - 1) >> low;
  }

  inline static uint32_t Sext(uint32_t ins, int32_t top) {
    if (ins & (1 << top))return ins | -(1 << top);
    return ins;
  }

  static void decode(uint32_t ins, Instruction &ins_ca) {
    ins_ca.Init();
    ins_ca.opcode = getPart(ins, 6, 0);
    ins_ca.origin_code = ins;
    ins_ca.op_type = getOpType(getPart(ins, 6, 0), getPart(ins, 14, 12), getPart(ins, 31, 25));
    if (ins_ca.op_type != WRONG) {
      instructionType insType_ca = ToInsType[ins_ca.op_type];
      ins_ca.imm = getImm(insType_ca, ins);
      if (insType_ca != U_type && insType_ca != J_type)ins_ca.rs1 = getPart(ins, 19, 15);
      if (insType_ca == R_type || insType_ca == S_type || insType_ca == B_type)ins_ca.rs2 = getPart(ins, 24, 20);
      if (insType_ca != S_type && insType_ca != B_type)ins_ca.rd = getPart(ins, 11, 7);
      if (insType_ca == I_type)ins_ca.shamt = ins >> 20 & 0b111111;
    }
  }
};
}

#endif //RISC_V_TOMASULO__DECODER_HPP_
