//
// Created by JerryGuo on 2022/6/28.
//

#ifndef RISC_V_TOMASULO__ALU_HPP_
#define RISC_V_TOMASULO__ALU_HPP_

#include <cstdint>
#include "mydef.hpp"
#include "Decoder.hpp"
#include "exceptional.hpp"
namespace RISC_V {
struct ExecuteResult {
  uint32_t value = 0, pc = 0;
  bool jump = false;

  void PrintExResult() {
    std::cout << "#value= " << value << "\t#jump= " << (jump ? "true" : "false") << "\t#pc= " << pc << std::endl;
  }
};

struct IntALU {
  static ExecuteResult Process(opType op_type_, uint32_t v1_, uint32_t v2_, uint32_t imm, uint32_t pc, uint32_t shamt) {
    ExecuteResult result;
    switch (op_type_) {
      case LUI:result = {imm, 0, false};
        break;
      case AUIPC:result = {pc + imm, pc + imm, true};
        break;
      case JAL:result = {pc + 4, pc + imm, true};
        break;
      case JALR:result = {pc + 4, (v1_ + imm) & ~(uint32_t) 1, true};
        break;
      case BEQ:result = {0, pc + imm, v1_ == v2_};
        break;
      case BNE:result = {0, pc + imm, v1_ != v2_};
        break;
      case BLT:result = {0, pc + imm, (int32_t) v1_ < (int32_t) v2_};
        break;
      case BGE:result = {0, pc + imm, (int32_t) v1_ >= (int32_t) v2_};
        break;
      case BLTU:result = {0, pc + imm, v1_ < v2_};
        break;
      case BGEU:result = {0, pc + imm, v1_ >= v2_};
        break;
      case ADDI:
        result = {v1_ + imm, 0, false};
        break;
      case SLTI:result = {(int32_t) v1_ < (int32_t) imm, 0, false};
        break;
      case SLTIU:result = {v1_ < imm, 0, false};
        break;
      case XORI:result = {v1_ ^ imm, 0, false};
        break;
      case ORI:result = {v1_ | imm, 0, false};
        break;
      case ANDI:result = {v1_ & imm, 0, false};
        break;
      case SLLI:result = {v1_ << imm, 0, false};
        break;
      case SRLI:result = {v1_ >> imm, 0, false};
        break;
      case SRAI:result = {Decoder::Sext(v1_ >> shamt, 31 - shamt), 0, false};
        break;
      case ADD:result = {v1_ + v2_, 0, false};
        break;
      case SUB:result = {v1_ - v2_, 0, false};
        break;
      case SLL:result = {v1_ << Decoder::getPart(v2_, 4, 0), 0, false};
        break;
      case SLT:result = {(int32_t) v1_ < (int32_t) v2_, 0, false};
        break;
      case SLTU:result = {v1_ < v2_, 0, false};
        break;
      case XOR:result = {v1_ ^ v2_, 0, false};
        break;
      case SRL:result = {v1_ >> Decoder::getPart(v2_, 4, 0), 0, false};
        break;
      case SRA: {
        uint32_t move = Decoder::getPart(v2_, 4, 0);
        result = {Decoder::Sext(v1_ >> move, 31 - move), 0, false};
        break;
      }
      case OR:result = {v1_ | v2_, 0, false};
        break;
      case AND:result = {v1_ & v2_, 0, false};
        break;
      default: throw Error("Opcode not right");
    }
    return result;
  }
};

struct AddrALU {
  static ExecuteResult Process(opType op_type_, uint32_t v1_, uint32_t v2_, uint32_t imm, RAM<> &ram_) {
    ExecuteResult result;
    switch (op_type_) {
      case LB: {
        uint32_t ca = ram_.Read8(v1_ + (int32_t) imm);
        ca = RISC_V::Decoder::Sext(ca, 7);
        result = {ca, 0, false};
        break;
      }
      case LH: {
        uint32_t ca = ram_.Read16(v1_ + (int32_t) imm);
        ca = RISC_V::Decoder::Sext(ca, 15);
        result = {ca, 0, false};
        break;
      }
      case LW: {
        uint32_t ca = ram_.Read32(v1_ + (int32_t) imm);
        result = {ca, 0, false};
        break;
      }
      case LBU: {
        uint32_t ca = ram_.Read8(v1_ + (int32_t) imm);
        result = {ca, 0, false};
        break;
      }
      case LHU: {
        uint32_t ca = ram_.Read16(v1_ + (int32_t) imm);
        result = {ca, 0, false};
        break;
      }
      case SB: {
        ram_.Write(v1_ + imm, (uint8_t) v2_);
        break;
      }
      case SH: {
        ram_.Write(v1_ + imm, (uint8_t) v2_);
        ram_.Write(v1_ + imm + 1, (uint8_t) (v2_ >> 8));
        break;
      }
      case SW: {
        ram_.Write(v1_ + imm, (uint8_t) v2_);
        ram_.Write(v1_ + imm + 1, (uint8_t) (v2_ >> 8));
        ram_.Write(v1_ + imm + 2, (uint8_t) (v2_ >> 16));
        ram_.Write(v1_ + imm + 3, (uint8_t) (v2_ >> 24));
        break;
      }
    }
    return result;
  }

};

}

#endif //RISC_V_TOMASULO__ALU_HPP_
