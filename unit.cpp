//
// Created by JerryGuo on 2022/6/28.
//

#include "unit.hpp"

//----------register------------
//RISC_V::regUnit &RISC_V::Register::operator[](uint32_t pos) {
//  if (pos == 0)throw Error("Trying to change R[0]");
//  return reg[pos];
//}
//const RISC_V::regUnit &RISC_V::Register::operator[](uint32_t pos) const {
//  if (pos == 0)throw Error("Trying to change R[0]");
//  return reg[pos];
//}

RISC_V::regUnit RISC_V::Register::Read(uint32_t pos) {
  //if (pos > REG_SIZE)throw Error("REG Read exceed");
  return reg[pos];
}
void RISC_V::Register::Write(uint32_t pos, RISC_V::regUnit regu_ca) {
  if (pos != 0)reg[pos] = regu_ca;
}

void RISC_V::Register::WriteValue(uint32_t pos, uint32_t value_) {
  if (pos != 0)reg[pos].value = value_;
}

void RISC_V::Register::WriteRobId(uint32_t pos, uint32_t rob_id_) {
  if (pos != 0)reg[pos].rob_id = rob_id_;
}

RISC_V::Register &RISC_V::Register::operator=(const RISC_V::Register &rhs) {
  if (this == &rhs)return *this;
  for (int i = 0; i < REG_SIZE; i++)reg[i] = rhs.reg[i];
  return *this;
}

void RISC_V::Register::clearRegfile() {
  for (auto &i: reg)i.rob_id = 0;
}

void RISC_V::Register::Clear() {
  memset(reg, 0, sizeof(reg));
}
void RISC_V::Register::PrintReg() {
//  std::cout << "Rank  = ";
//  for (int i = 0; i < 32; i++) std::cout << i << "\t";
//  std::cout << "\nRob_id= ";
//  for (int i = 0; i < 32; i++) std::cout << (int32_t) reg[i].rob_id << "\t";
//  std::cout << "\nValue = ";
//  for (int i = 0; i < 32; i++) std::cout << (int32_t) reg[i].value << "\t";
//  std::cout << std::endl;
  puts("Register:");
  for (int i = 0; i < REG_SIZE; ++i) {
    //printf("%6d", reg[i].value);
    printf("%6d,%d ", reg[i].value, reg[i].rob_id);
    if (i && i % 8 == 7) puts("");
  }
}

//---------reservation station---------
RISC_V::rsUnit &RISC_V::reservationStation::operator[](uint32_t pos) {
  return rs[pos];
}
const RISC_V::rsUnit &RISC_V::reservationStation::operator[](uint32_t pos) const {
  return rs[pos];
}
RISC_V::reservationStation &RISC_V::reservationStation::operator=(const RISC_V::reservationStation &rhs) {
  if (this == &rhs)return *this;
  for (int i = 0; i < RS_SIZE; i++)rs[i] = rhs.rs[i];
  return *this;
}
uint32_t RISC_V::reservationStation::findFree() {
  //when full return UINT32_MAX
  for (int i = 0; i < RS_SIZE; i++) {
    if (!rs[i].busy)return i;
  }
  return UINT32_MAX;
}
void RISC_V::reservationStation::Clear() {
  for (auto &r: rs) r.busy = false;
}

//----------ROB--------------
RISC_V::ROBUnit &RISC_V::ROB::operator[](uint32_t pos) {
  return val[pos];
}
const RISC_V::ROBUnit &RISC_V::ROB::operator[](uint32_t pos) const {
  return val[pos];
}
RISC_V::ROB &RISC_V::ROB::operator=(const RISC_V::ROB &rhs) {
  if (this == &rhs)return *this;
  for (int i = 0; i < ROB_SIZE; i++)val[i] = rhs.val[i];
  val.head = rhs.val.head;
  val.tail = rhs.val.tail;
  return *this;
}
bool RISC_V::ROB::Full() {
  return val.full();
}
bool RISC_V::ROB::Empty() {
  return val.empty();
}
RISC_V::ROBUnit &RISC_V::ROB::Front() {
  //if (val.empty())throw Error("ROB is empty");
  return val.front();
}
void RISC_V::ROB::PopFront() {
  //if (val.empty())throw Error("ROB is empty");
  val.popFront();
}
uint32_t RISC_V::ROB::PushBack(const RISC_V::ROBUnit &value) {
  //if (val.full())throw Error("ROB is full");
  return val.pushBack(value);
}
void RISC_V::ROB::Clear() {
  val.clear();
}
void RISC_V::ROB::PrintRob() {
  std::cout << "rob_size= " << val.size() << std::endl;

  for (int i = val.head; i < val.tail; i++) {
    std::cout << "rob_id= " << i << "\t" << std::endl;
    std::cout << "ready: " << ((val[i].ready) ? "true" : "false") << std::endl;
    std::cout << val[i].ins << std::endl;
    std::cout << "#reg_id= " << val[i].reg_id << "\t#rs_id= " << val[i].rs_id << "\t#slb_id= " << val[i].slb_id
              << std::endl;
    std::cout << "#present_pc= " << val[i].present_pc << std::endl;

    std::cout << "init_jump: " << ((val[i].init_jump) ? "true" : "false") << std::endl;

    val[i].result.PrintExResult();
    std::cout << std::endl;
  }
}

//--------SLB---------------
RISC_V::SLBUnit &RISC_V::storeLoadBuffer::operator[](uint32_t pos) {
  return val[pos];
}
const RISC_V::SLBUnit &RISC_V::storeLoadBuffer::operator[](uint32_t pos) const {
  return val[pos];
}
RISC_V::storeLoadBuffer &RISC_V::storeLoadBuffer::operator=(const RISC_V::storeLoadBuffer &rhs) {
  if (this == &rhs)return *this;
  for (int i = rhs.val.head; i < rhs.val.tail; i++)val[i] = rhs.val[i];
  val.head = rhs.val.head;
  val.tail = rhs.val.tail;
  return *this;
}
bool RISC_V::storeLoadBuffer::FUll() {
  return val.full();
}
bool RISC_V::storeLoadBuffer::Empty() {
  return val.empty();
}
RISC_V::SLBUnit &RISC_V::storeLoadBuffer::Front() {
  //if (val.empty())throw Error("SLB is empty visit");
  return val.front();
}
void RISC_V::storeLoadBuffer::PopFront() {
  //if (val.empty())throw Error("SLB is empty pop");
  val.popFront();
}
uint32_t RISC_V::storeLoadBuffer::PushBack(const RISC_V::SLBUnit &value) {
  //if (val.full())throw Error("SLB is full");
  return val.pushBack(value);
}
void RISC_V::storeLoadBuffer::Clear() {
  val.clear();
}

//------------fetch queue-------------
RISC_V::fqUnit &RISC_V::fetchQueue::operator[](uint32_t pos) {
  return val[pos];
}
const RISC_V::fqUnit &RISC_V::fetchQueue::operator[](uint32_t pos) const {
  return val[pos];
}
RISC_V::fetchQueue &RISC_V::fetchQueue::operator=(const RISC_V::fetchQueue &rhs) {
  if (this == &rhs)return *this;
  for (int i = 0; i < SLB_SIZE; i++)val[i] = rhs.val[i];
  val.head = rhs.val.head;
  val.tail = rhs.val.tail;
  return *this;
}
bool RISC_V::fetchQueue::FUll() {
  return val.full();
}
bool RISC_V::fetchQueue::Empty() {
  return val.empty();
}
RISC_V::fqUnit &RISC_V::fetchQueue::Front() {
  //if (val.empty())throw Error("FQ is empty");
  return val.front();
}
void RISC_V::fetchQueue::PopFront() {
  //if (val.empty())throw Error("SLB is empty");
  val.popFront();
}
uint32_t RISC_V::fetchQueue::PushBack(const RISC_V::fqUnit &value) {
  //if (val.full())throw Error("FQ is full");
  return val.pushBack(value);
}
void RISC_V::fetchQueue::Clear() {
  val.clear();
}
uint32_t RISC_V::fetchQueue::Size() {
  return val.size();
}
bool RISC_V::rsUnit::RsReady() const {
  instructionType insType = ToInsType[instruction.op_type];
  switch (insType) {
    case U_type:
    case J_type: return true;

    case R_type:
    case B_type: {
      if (!Qi && !Qj)return true;
      return false;
    }
    case I_type:if (!Qi)return true;
      return false;

  }

  return false;
}
