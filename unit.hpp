//
// Created by JerryGuo on 2022/6/28.
//

#ifndef RISC_V_TOMASULO__UNIT_HPP_
#define RISC_V_TOMASULO__UNIT_HPP_

#include "mydef.hpp"
#include "cstring"
#include "ALU.hpp"
#include "exceptional.hpp"

namespace RISC_V {
//----------loopQueue-----------
template<class T, uint32_t len>
class loopQueue {
 public:
  int tail = 1;
  int head = 1;
  T que[len];
 public:
  T &operator[](uint32_t pos) {
    return que[pos % len];
  }
  const T &operator[](uint32_t pos) const {
    return que[pos % len];
  }
  T &front() {
    return que[head % len];
  }
  uint32_t pushBack(const T &value) {
    que[tail % len] = value;
    ++tail;
    return tail - 1;
  }
  void popFront() {
    ++head;
  }
  uint32_t size() {
    return tail - head;
  }
  void clear() {
    head = tail = 1;
  }
  bool empty() {
    return size() == 0;
  }
  bool full() {
    return size() >= len;
  }

};
//----------mutualCommunicate-----------
enum renameType {
  Reg, Rs, Rob, Slb
};
//----------Register--------------------
struct regUnit {
  uint32_t value = 0, rob_id = 0;
  //bool waiting = false;
  regUnit &operator=(const regUnit &rhs) = default;
};
struct Register {
  regUnit reg[REG_SIZE];
//  regUnit &operator[](uint32_t pos);
//  const regUnit &operator[](uint32_t pos) const;

  regUnit Read(uint32_t pos);
  void Write(uint32_t pos, regUnit regu_ca);
  void WriteValue(uint32_t pos, uint32_t value_);
  void WriteRobId(uint32_t pos, uint32_t rob_id_);

  Register &operator=(const Register &rhs);
  void clearRegfile();
  void Clear();

  void PrintReg();
};
//----------Reservation Station---------
struct rsUnit {
  bool busy = false;
  Instruction instruction{};//opType,rs1,rs2,rd,imm,shamt
  uint32_t time = 0;
  uint32_t pc = 0;
  uint32_t Vi = 0, Vj = 0, Qi = UINT32_MAX, Qj = UINT32_MAX;//rs1_value,rs2_value,rs1_rob,rs2_rob

  uint32_t rob_id = 0;
  rsUnit &operator=(const rsUnit &rhs) = default;
  bool RsReady() const;
};
struct reservationStation {
  rsUnit rs[RS_SIZE];

  rsUnit &operator[](uint32_t pos);
  const rsUnit &operator[](uint32_t pos) const;
  reservationStation &operator=(const reservationStation &rhs);
  uint32_t findFree();//when full return -1
  void Clear();
};
//----------Reorder Buffer--------
struct ROBUnit {
  bool ready = false;
  Instruction ins{};
  uint32_t reg_id = UINT32_MAX, rs_id = UINT32_MAX, slb_id = UINT32_MAX;
//  bool jump = false;//是否为跳转语句
  uint32_t present_pc = 0;
  bool init_jump = false;
  ExecuteResult result;
  ROBUnit &operator=(const ROBUnit &rhs) = default;
};
struct ROB {
  loopQueue<ROBUnit, ROB_SIZE> val;
  ROBUnit &operator[](uint32_t pos);
  const ROBUnit &operator[](uint32_t pos) const;
  ROB &operator=(const ROB &rhs);
  bool Full();
  bool Empty();
  ROBUnit &Front();
  void PopFront();
  uint32_t PushBack(const ROBUnit &value);
  void Clear();

  void PrintRob();

};
//----------Store & Load Buffer-------
struct SLBUnit {
  bool busy = false;
  bool if_commit = false;
  Instruction ins{};
  uint32_t rob_id = 0;//对应的rob编号，用于执行完成后传回答案
  uint32_t Vi = 0, Vj = 0, Qi = UINT32_MAX, Qj = UINT32_MAX;
  uint32_t time = 0;

  SLBUnit &operator=(const SLBUnit &rhs) = default;
};
struct storeLoadBuffer {
  loopQueue<SLBUnit, SLB_SIZE> val;
  SLBUnit &operator[](uint32_t pos);
  const SLBUnit &operator[](uint32_t pos) const;
  storeLoadBuffer &operator=(const storeLoadBuffer &rhs);
  bool FUll();
  bool Empty();
  SLBUnit &Front();
  void PopFront();
  uint32_t PushBack(const SLBUnit &value);
  void Clear();
};
//----------Instruction Queue---------
struct fqUnit {
  Instruction ins{};
  uint32_t pc = 0;//该指令对应的pc
  //---predict
  bool init_jump = false;
};
struct fetchQueue {
  loopQueue<fqUnit, FQ_SIZE> val;
  fqUnit &operator[](uint32_t pos);
  const fqUnit &operator[](uint32_t pos) const;
  fetchQueue &operator=(const fetchQueue &rhs);
  bool FUll();
  bool Empty();
  fqUnit &Front();
  void PopFront();
  uint32_t Size();
  uint32_t PushBack(const fqUnit &value);
  void Clear();
};
//----------CDB------------
template<class T>
struct Wire {
  bool stall = true;
  T val;
};

//--------fetch--------------
struct FetchToIssue {
  Instruction ins{};
  uint32_t pc = 0;
  //---predict
  bool init_jump = false;
};

//---------issue-------------
struct Null {};

struct IssueToRegfile {
  uint32_t rd = 0, rob_id = 0;
};

struct IssueToRob {
  Instruction ins{};
  uint32_t reg_id = UINT32_MAX, rs_id = UINT32_MAX, slb_id = UINT32_MAX;
  //bool jump = false;//是否为跳转语句
  uint32_t present_pc = 0;
//---predict
  bool init_jump = false;
  IssueToRob &operator=(const IssueToRob &rhs) = default;
};

struct IssueToRs {
  Instruction ins{};//opType,rs1,rs2,rd,imm,shamt
  uint32_t time = 0;
  uint32_t pc = 0;
  uint32_t rs_id = UINT32_MAX, rob_id = 0;
  IssueToRs &operator=(const IssueToRs &rhs) = default;
};

struct IssueToSlb {
  Instruction ins{};
  uint32_t rob_id = 0;//对应的rob编号，用于执行完成后传回答案
  uint32_t time = 0;

  IssueToSlb &operator=(const IssueToSlb &rhs) = default;
};

//----------rs-----------------
struct RsToEx {
  opType op_type = LUI;
  uint32_t rob_id = 0;
  uint32_t rs1_value = 0, rs2_value = 0;
  uint32_t imm = 0;
  uint32_t pc = 0;
  uint32_t shamt = 0;
  uint32_t time = 0;
};

//----------commit-------------
struct CommitToRegfile {
  uint32_t rd = 0, value = 0, rob_id = 0;
};

struct CommitToSlb {
  uint32_t slb_id = UINT32_MAX;
};

//------------slb---------------
struct SlbToRs {
  uint32_t value = 0;
  uint32_t rob_id = 0;
};

struct SlbToSlb {
  uint32_t value = 0;
  uint32_t rob_id = 0;
};

struct SlbToRob {
  ExecuteResult result;
  uint32_t rob_id = 0;
};

//--------------ex----------------
struct ExToRob {
  ExecuteResult result;
  uint32_t rob_id = 0;
};

struct ExToSlb {
  ExecuteResult result;
  uint32_t rob_id = 0;
};

struct ExToRs {
  ExecuteResult result;
  uint32_t rob_id = 0;
};

//---------------rob----------------
struct RobToCommit {
  Instruction ins{};
  uint32_t reg_id = 0, rob_id = 0, slb_id = 0;
  ExecuteResult result{};

  //---predict
  bool init_jump = false;
  uint32_t now_pc = 0;
};

}

#endif //RISC_V_TOMASULO__UNIT_HPP_
