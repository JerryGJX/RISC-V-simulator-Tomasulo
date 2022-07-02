//
// Created by JerryGuo on 2022/6/28.
//

#ifndef RISC_V_TOMASULO__MYDEF_HPP_
#define RISC_V_TOMASULO__MYDEF_HPP_

#include <cstdint>
#include <string>
#include <iostream>
#include <bitset>

namespace RISC_V {
static constexpr uint32_t RAM_LIMIT = 500000;
static constexpr int loadStoreTime = 3, calTime = 0;
static constexpr uint32_t REG_SIZE = 32, MEM_SIZE = 5e5, RS_SIZE = 32,
    ROB_SIZE = 32, SLB_SIZE = 32, FQ_SIZE = 32;//load_buffer,store_buffer


enum opType {
  LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU, LB, LH, LW,
  LBU, LHU, SB, SH, SW, ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI,
  SRAI, ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND, WRONG
};

const std::string opName[37] = {
    "LUI", "AUIPC", "JAL", "JALR",
    "BEQ", "BNE", "BLT", "BGE", "BLTU", "BGEU",
    "LB", "LH", "LW", "LBU", "LHU", "SB", "SH", "SW",
    "ADDI", "SLTI", "SLTIU", "XORI", "ORI", "ANDI", "SLLI", "SRLI", "SRAI",
    "ADD", "SUB", "SLL", "SLT", "SLTU", "XOR", "SRL", "SRA", "OR", "AND"
};

enum instructionType {
  R_type, I_type, S_type, B_type, U_type, J_type
};

const std::string insTypeName[6] = {
    "[R]", "[I]", "[S]", "[B]", "[U]", "[J]"
};

const instructionType ToInsType[37] = {
    U_type, U_type, J_type, I_type,
    B_type, B_type, B_type, B_type, B_type, B_type,
    I_type, I_type, I_type, I_type, I_type,
    S_type, S_type, S_type,
    I_type, I_type, I_type, I_type, I_type, I_type, I_type, I_type, I_type,
    R_type, R_type, R_type, R_type, R_type, R_type, R_type, R_type, R_type, R_type

};

class Instruction {
 public:
  opType op_type = WRONG;
  uint32_t opcode = 0;
  uint32_t origin_code = 0;
  uint32_t rs1 = 0, rs2 = 0, rd = 0, shamt = 0;
  uint32_t imm = 0;

  Instruction() = default;
  void Init() {
    op_type = WRONG, rs1 = 0, rs2 = 0, opcode = 0, origin_code = 0,
    rd = 0, imm = 0, shamt = 0;
  }

  Instruction &operator=(const Instruction &rhs) = default;

  friend std::ostream &operator<<(std::ostream &os, const Instruction &instruction) {
    os << "###Ins =" << std::hex << instruction.origin_code << std::dec << "\n"
       << "inst type: " << insTypeName[ToInsType[instruction.op_type]]
       << opName[instruction.op_type] << ", "
       << " \trs1: " << instruction.rs1 << ", "
       << " \trs2: " << instruction.rs2 << ", "

       << " \timm: " << (int32_t) instruction.imm << ", "
       << " \trd: " << instruction.rd;
//        << ", "
//       << " funct3: " << instruction.funct3 << ", "
//       << " funct7: " << instruction.funct7 << ", "
//
//       << " shamt: " << instruction.shamt;

    return os;
  }

};

}

#endif //RISC_V_TOMASULO__MYDEF_HPP_
