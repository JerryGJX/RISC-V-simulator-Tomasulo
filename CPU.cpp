//
// Created by JerryGuo on 2022/6/28.
//

#include "CPU.hpp"
void RISC_V::CPU::scan() {
  std::string str_ca;
  uint32_t start_pos = 0;
  while (std::getline(std::cin, str_ca)) {
//      std::cout<<str_ca<<std::endl;
//      continue;
    if (str_ca[0] == '@') {
      start_pos = std::stoi(str_ca.substr(1), nullptr, 16);
      continue;
    }
    for (int i = 0; i < str_ca.size(); i += 3) {
      std::string Byte = str_ca.substr(i, 2);
      ram[start_pos] = std::stoi(Byte, nullptr, 16);;
//      std::cout << "startPos = " << start_pos << " ## " << std::hex << ram.Read8(start_pos) << std::endl;
//      printf("pos: %d, mem: %02X\n", start_pos, ram[start_pos]);
      start_pos++;
    }
  }
}

void RISC_V::CPU::run() {

  while (true) {
    /*在这里使用了两阶段的循环部分：
      1. 实现时序电路部分，即在每个周期初同步更新的信息。
      2. 实现逻辑电路部分，即在每个周期中如ex、issue的部分
      已在下面给出代码
    */

//    PrintStall();

    cycle++;
//    std::cout << "[" << cycle << "]" << std::endl;

    run_rob();

    run_slbuffer();
    run_reservation();
    run_regfile();
    run_inst_fetch_queue();
    update();

    run_ex();
    run_issue();
    run_commit();

//    reg_prev.PrintReg();

//    std::cout << ANSI::FG_GREEN << (uint32_t) ram[4204] << ANSI::RESET << std::endl;

    if (!rob_to_commit.stall && rob_to_commit.val.ins.origin_code == 0x0ff00513) {
      printf("%d", reg_prev.Read(10).value & 255u);
      break;
    }
  }
}

void RISC_V::CPU::run_inst_fetch_queue() {
  /*
     在这一部分你需要完成的工作：
     1. 实现一个先进先出的指令队列
     2. 读取指令并存放到指令队列中
     3. 准备好下一条issue的指令
     tips: 考虑边界问题（满/空...）
     */
  fetch_to_issue.stall = true;
  if (if_clear_fq) {
    fq_next.Clear();
    return;
  }
  if (!fq_next.FUll()) {
    fqUnit ins_ca = {ram.Read32(pc), pc};
    fq_next.PushBack(ins_ca);
    pc += 4;
  }
  if (!issue_to_fetch.stall) {
    if (fq_prev.Size() > 1) {
      fetch_to_issue.val = {fq_prev[fq_prev.val.head + 1].origin_ins, fq_prev[fq_prev.val.head + 1].pc};
      fetch_to_issue.stall = false;
    }
    fq_next.PopFront();
  } else {
    if (!fq_prev.Empty()) {
      fetch_to_issue.val = {fq_prev.Front().origin_ins, fq_prev.Front().pc};
      fetch_to_issue.stall = false;
    }
  }

}

void RISC_V::CPU::run_issue() {
/*
  在这一部分你需要完成的工作：
  1. 从run_inst_fetch_queue()中得到issue的指令
  2. 对于issue的所有类型的指令向ROB申请一个位置（或者也可以通过ROB预留位置），并修改regfile中相应的值
  2. 对于 非 Load/Store的指令，将指令进行分解后发到Reservation Station
    tip: 1. 这里需要考虑怎么得到rs1、rs2的值，并考虑如当前rs1、rs2未被计算出的情况，参考书上内容进行处理
         2. 在本次作业中，我们认为相应寄存器的值已在ROB中存储但尚未commit的情况是可以直接获得的，即你需要实现这个功能
            而对于rs1、rs2不ready的情况，只需要stall即可，有兴趣的同学可以考虑下怎么样直接从EX完的结果更快的得到计算结果
  3. 对于 Load/Store指令，将指令分解后发到SLBuffer(需注意SLBUFFER也该是个先进先出的队列实现)
  tips: 考虑边界问题（是否还有足够的空间存放下一条指令）
  */
  issue_to_fetch.stall = true;
  issue_to_rob.stall = true;
  issue_to_rs.stall = true;
  issue_to_slb.stall = true;
  issue_to_regfile.stall = true;

  if (!fetch_to_issue.stall) {
    Instruction ins_ca;
    RISC_V::Decoder::decode(fetch_to_issue.val.origin_ins, ins_ca);
    uint32_t opcode = Decoder::getPart(fetch_to_issue.val.origin_ins, 6, 0);
    uint32_t rob_id = 0, slb_id = UINT32_MAX, rs_id = UINT32_MAX;
    if (!rob_next.Full()) {

      rob_id = rob_prev.val.tail;

      if (ins_ca.op_type != WRONG) {
        if (opcode == 0b0000011 || opcode == 0b0100011) {
          if (!slb_prev.FUll()) {
            issue_to_fetch.stall = false;
            issue_to_rob.stall = false;
            issue_to_slb.stall = false;
            slb_id = slb_prev.val.tail;
            issue_to_slb.val = {ins_ca, rob_id, RISC_V::loadStoreTime};
            issue_to_rob.val = {ins_ca, ins_ca.rd, 0, slb_id, false, fetch_to_issue.val.pc, 0};

            if (opcode == 0b0000011 && ins_ca.rd != 0) {
              issue_to_regfile.val = {ins_ca.rd, rob_id};
              issue_to_regfile.stall = false;
            }
          }
        } else {
          rs_id = rs_prev.findFree();
          if (rs_id != UINT32_MAX) {
            issue_to_fetch.stall = false;
            issue_to_rob.stall = false;
            issue_to_rs.stall = false;

            issue_to_rs.val = {ins_ca, RISC_V::calTime, fetch_to_issue.val.pc, rs_id, rob_id};
            issue_to_rob.val = {ins_ca, ins_ca.rd, rs_id, 0, false, fetch_to_issue.val.pc, 0};

            if (opcode != 0b1100011 && ins_ca.rd != 0) {
              issue_to_regfile.val = {ins_ca.rd, rob_id};
              issue_to_regfile.stall = false;
            }
          }
        }
      }
    }
  }
}

void RISC_V::CPU::run_regfile() {

  if (!commit_to_regfile.stall && commit_to_regfile.val.rd) {
    reg_next.WriteValue(commit_to_regfile.val.rd, commit_to_regfile.val.value);
    //reg_next[commit_to_regfile.val.rd].value = commit_to_regfile.val.value;
    if (reg_next.Read(commit_to_regfile.val.rd).rob_id == commit_to_regfile.val.rob_id)
      reg_next.WriteRobId(commit_to_regfile.val.rd, 0);
  }

  if (if_clear_regfile) {
    reg_next.clearRegfile();
    return;
  }

  // commit_to_regfile.stall = true;
  if (!issue_to_regfile.stall) {
    reg_next.WriteRobId(issue_to_regfile.val.rd, issue_to_regfile.val.rob_id);
  }
}

void RISC_V::CPU::run_reservation() {
  if (if_clear_rs) {
    rs_to_ex.stall = true;
    rs_next.Clear();
    return;
  }
  if (!issue_to_rs.stall) {
    //auto &val_ca = issue_to_rs.val;
//    auto &rs1 = val_ca.ins.rs1;
//    auto &rs2 = val_ca.ins.rs2;
    auto &target_rs_unit = rs_next[issue_to_rs.val.rs_id];
    target_rs_unit = {true, issue_to_rs.val.ins, RISC_V::calTime, issue_to_rs.val.pc};
    target_rs_unit.rob_id = issue_to_rs.val.rob_id;
    auto reg_rs1 = reg_prev.Read(issue_to_rs.val.ins.rs1);
    auto reg_rs2 = reg_prev.Read(issue_to_rs.val.ins.rs2);
    if (reg_rs1.rob_id == 0) {
      target_rs_unit.Vi = reg_rs1.value;
      target_rs_unit.Qi = 0;
    } else {
      if (!ex_to_rs.stall && ex_to_rs.val.rob_id == reg_rs1.rob_id) {
        target_rs_unit.Vi = ex_to_rs.val.result.value;
        target_rs_unit.Qi = 0;
      } else if (!slb_to_rs_prev.stall && slb_to_rs_prev.val.rob_id == reg_rs1.rob_id) {
        target_rs_unit.Vi = slb_to_rs_prev.val.value;
        target_rs_unit.Qi = 0;
      } else if (rob_prev[reg_rs1.rob_id].ready) {
        target_rs_unit.Vi = rob_prev[reg_rs1.rob_id].result.value;
        target_rs_unit.Qi = 0;
      } else target_rs_unit.Qi = reg_rs1.rob_id;
    }

    if (reg_rs2.rob_id == 0) {
      target_rs_unit.Vj = reg_rs2.value;
      target_rs_unit.Qj = 0;
    } else {
      if (!ex_to_rs.stall && ex_to_rs.val.rob_id == reg_rs2.rob_id) {
        target_rs_unit.Vj = ex_to_rs.val.result.value;
        target_rs_unit.Qj = 0;
      } else if (!slb_to_rs_prev.stall && slb_to_rs_prev.val.rob_id == reg_rs2.rob_id) {
        target_rs_unit.Vj = slb_to_rs_prev.val.value;
        target_rs_unit.Qj = 0;
      } else if (rob_prev[reg_rs2.rob_id].ready) {
        target_rs_unit.Vj = rob_prev[reg_rs2.rob_id].result.value;
        target_rs_unit.Qj = 0;
      } else target_rs_unit.Qj = reg_rs2.rob_id;
    }
  }

  rs_to_ex.stall = true;//表示无指令传到ex

//    if (!ex_to_rs.stall) {
  for (int i = 0; i < RISC_V::RS_SIZE; i++) {
    if (rs_prev[i].busy && rs_prev[i].RsReady()) {
      rs_to_ex.stall = false;
      auto &ins_ca = rs_prev[i].instruction;
      rs_to_ex.val =
          {ins_ca.op_type, rs_prev[i].rob_id, rs_prev[i].Vi,
           rs_prev[i].Vj, ins_ca.imm, rs_prev[i].pc, ins_ca.shamt, RISC_V::calTime};
      rs_next[i].busy = false;
      break;
    }
  }
//    } else rs_to_ex.stall = false;

  if (!ex_to_rs.stall) {
    for (int i = 0; i < RISC_V::RS_SIZE; i++) {
      if (rs_prev[i].busy) {
        if (rs_prev[i].Qi == ex_to_rs.val.rob_id) {
          rs_next[i].Qi = 0;
          rs_next[i].Vi = ex_to_rs.val.result.value;
        }
        if (rs_prev[i].Qj == ex_to_rs.val.rob_id) {
          rs_next[i].Qj = 0;
          rs_next[i].Vj = ex_to_rs.val.result.value;
        }
      }
    }
  }

  if (!slb_to_rs_prev.stall) {
    for (int i = 0; i < RISC_V::RS_SIZE; i++) {
      if (rs_prev[i].busy) {
        if (rs_prev[i].Qi == slb_to_rs_prev.val.rob_id) {
          rs_next[i].Qi = 0;
          rs_next[i].Vi = slb_to_rs_prev.val.value;
        }
        if (rs_prev[i].Qj == slb_to_rs_prev.val.rob_id) {
          rs_next[i].Qj = 0;
          rs_next[i].Vj = slb_to_rs_prev.val.value;
        }
      }
    }
  }

}

void RISC_V::CPU::run_ex() {
  ex_to_rs.stall = true, ex_to_rob.stall = true, ex_to_slb.stall = true;
  if (!rs_to_ex.stall) {
//    if (rs_to_ex.val.time != 0) {
//      rs_to_ex.val.time--;
//    } else {
    ex_to_rs.stall = false, ex_to_rob.stall = false, ex_to_slb.stall = false;
    ExecuteResult result_ca = RISC_V::IntALU::Process(rs_to_ex.val.op_type,
                                                      rs_to_ex.val.rs1_value,
                                                      rs_to_ex.val.rs2_value,
                                                      rs_to_ex.val.imm,
                                                      rs_to_ex.val.pc,
                                                      rs_to_ex.val.shamt);
    ex_to_rs.val = {result_ca, rs_to_ex.val.rob_id};
    ex_to_rob.val = {result_ca, rs_to_ex.val.rob_id};
    ex_to_slb.val = {result_ca, rs_to_ex.val.rob_id};
  }
//  }
}

void RISC_V::CPU::run_slbuffer() {
  if (!issue_to_slb.stall) {
    Instruction ins_ca = issue_to_slb.val.ins;
    SLBUnit slbu_ca = {true, false, issue_to_slb.val.ins, issue_to_slb.val.rob_id, 0, 0, UINT32_MAX, UINT32_MAX,
                       issue_to_slb.val.time};
    if (reg_prev.Read(ins_ca.rs1).rob_id == 0) {
      slbu_ca.Qi = 0, slbu_ca.Vi = reg_prev.Read(ins_ca.rs1).value;
    } else {
      if (!ex_to_slb.stall && ex_to_slb.val.rob_id == reg_prev.Read(ins_ca.rs1).rob_id) {
        slbu_ca.Qi = 0, slbu_ca.Vi = ex_to_slb.val.result.value;
      } else if (!slb_to_slb_prev.stall && slb_to_slb_prev.val.rob_id == reg_prev.Read(ins_ca.rs1).rob_id) {
        slbu_ca.Qi = 0, slbu_ca.Vi = slb_to_slb_prev.val.value;
      } else if (rob_prev[reg_prev.Read(ins_ca.rs1).rob_id].ready) {
        slbu_ca.Qi = 0, slbu_ca.Vi = rob_prev[reg_prev.Read(ins_ca.rs1).rob_id].result.value;
      } else {
        slbu_ca.Qi = reg_prev.Read(ins_ca.rs1).rob_id;
      }
    }
    if (reg_prev.Read(ins_ca.rs2).rob_id == 0) {
      slbu_ca.Qj = 0, slbu_ca.Vj = reg_prev.Read(ins_ca.rs2).value;
    } else {
      if (!ex_to_slb.stall && ex_to_slb.val.rob_id == reg_prev.Read(ins_ca.rs2).rob_id) {
        slbu_ca.Qj = 0, slbu_ca.Vj = ex_to_slb.val.result.value;
      } else if (!slb_to_slb_prev.stall && slb_to_slb_prev.val.rob_id == reg_prev.Read(ins_ca.rs2).rob_id) {
        slbu_ca.Qj = 0, slbu_ca.Vj = slb_to_slb_prev.val.value;
      } else if (rob_prev[reg_prev.Read(ins_ca.rs2).rob_id].ready) {
        slbu_ca.Qj = 0, slbu_ca.Vj = rob_prev[reg_prev.Read(ins_ca.rs2).rob_id].result.value;
      } else {
        slbu_ca.Qj = reg_prev.Read(ins_ca.rs2).rob_id;
      }
    }
    slb_next.PushBack(slbu_ca);
  }

  if (!ex_to_slb.stall) {
    for (int i = slb_prev.val.head; i < slb_prev.val.tail; i++) {
      if (ex_to_slb.val.rob_id == slb_prev[i].Qi) {
        slb_next[i].Qi = 0, slb_next[i].Vi = ex_to_slb.val.result.value;
      }
      if (ex_to_slb.val.rob_id == slb_prev[i].Qj) {
        slb_next[i].Qj = 0, slb_next[i].Vj = ex_to_slb.val.result.value;
      }
    }
  }

  if (!slb_to_slb_prev.stall) {
    for (int i = slb_prev.val.head; i < slb_prev.val.tail; i++) {
      if (slb_to_slb_prev.val.rob_id == slb_prev[i].Qi) {
        slb_next[i].Qi = 0, slb_next[i].Vi = slb_to_slb_prev.val.value;
      }
      if (slb_to_slb_prev.val.rob_id == slb_prev[i].Qj) {
        slb_next[i].Qj = 0, slb_next[i].Vj = slb_to_slb_prev.val.value;
      }
    }
  }

  if (!commit_to_slb.stall) {
    slb_next[commit_to_slb.val.slb_id].if_commit = true;
    slb_next[commit_to_slb.val.slb_id].time--;
  }

  slb_to_rob_next.stall = true, slb_to_rs_next.stall = true, slb_to_slb_next.stall = true;
  if (!slb_prev.Empty()) {
    const SLBUnit &front_ca = slb_prev.Front();
    if (front_ca.ins.opcode == 0b0000011) {//load
      if (front_ca.Qi == 0) {
        if (front_ca.time == 1) {
          ExecuteResult result_ca;
          result_ca = RISC_V::AddrALU::Process(front_ca.ins.op_type, front_ca.Vi, front_ca.Vj, front_ca.ins.imm, ram);
          slb_to_slb_next.val = {result_ca.value, front_ca.rob_id};
          slb_to_slb_next.stall = false;
          slb_to_rs_next.val = {result_ca.value, front_ca.rob_id};
          slb_to_rs_next.stall = false;
          slb_to_rob_next.val = {result_ca, front_ca.rob_id};
          slb_to_rob_next.stall = false;
          slb_next.PopFront();
        } else slb_next.Front().time--;
      }
    } else {//save
      if (commit_to_slb.stall || commit_to_slb.val.slb_id != slb_prev.val.head) {
        if (front_ca.Qi == 0 && front_ca.Qj == 0) {
          if (!front_ca.if_commit) {
            slb_to_rob_next.stall = false;
            slb_to_rob_next.val.rob_id = front_ca.rob_id;
          } else {
            if (front_ca.time == 1) {
              RISC_V::AddrALU::Process(front_ca.ins.op_type, front_ca.Vi, front_ca.Vj, front_ca.ins.imm, ram);
              slb_next.PopFront();
            } else slb_next.Front().time--;
          }
        }
      }
    }
  }

  if (if_clear_slb) {
    if (!slb_prev.Empty() && slb_prev.Front().if_commit && slb_prev.Front().time > 1)
      slb_next.val.tail = slb_next.val.head + 1;
    else slb_next.Clear();
    slb_to_rob_next.stall = true, slb_to_rs_next.stall = true, slb_to_slb_next.stall = true;
  }
}

void RISC_V::CPU::run_rob() {
  if (if_clear_rob) {
    rob_next.Clear();
    rob_to_commit.stall = true;
    return;
  }

//  rob_prev.PrintRob();

  if (!issue_to_rob.stall) {
    ROBUnit new_issue = {false, issue_to_rob.val.ins, issue_to_rob.val.reg_id,
                         issue_to_rob.val.rs_id, issue_to_rob.val.slb_id,
                         issue_to_rob.val.present_pc};
    rob_next.PushBack(new_issue);
  }

  if (!ex_to_rob.stall) {
    rob_next[ex_to_rob.val.rob_id].ready = true;
    rob_next[ex_to_rob.val.rob_id].result = ex_to_rob.val.result;
  }

  if (!slb_to_rob_prev.stall) {
    rob_next[slb_to_rob_prev.val.rob_id].ready = true;
    rob_next[slb_to_rob_prev.val.rob_id].result = slb_to_rob_prev.val.result;
  }

  rob_to_commit.stall = true;
  if (!rob_prev.Empty() && rob_prev.Front().ready) {
    const ROBUnit &front = rob_prev.Front();
    rob_to_commit.val = {front.ins, front.reg_id, static_cast<uint32_t>(rob_prev.val.head), front.slb_id, front.result};
    rob_to_commit.stall = false;
    rob_next.PopFront();
  }
}

void RISC_V::CPU::run_commit() {
  commit_to_regfile.stall = true;
  commit_to_slb.stall = true;
  if_clear_fq = if_clear_rob = if_clear_rs = if_clear_slb = if_clear_regfile = false;
  if (!rob_to_commit.stall) {
    instructionType insType = ToInsType[rob_to_commit.val.ins.op_type];
    if (insType != B_type && insType != S_type) {
      commit_to_regfile.stall = false;
      commit_to_regfile.val = {rob_to_commit.val.ins.rd, rob_to_commit.val.result.value, rob_to_commit.val.rob_id};
    }
    if ((insType == B_type || rob_to_commit.val.ins.op_type == JAL || rob_to_commit.val.ins.op_type == JALR)
        && rob_to_commit.val.result.jump) {
      if_clear_fq = if_clear_rob = if_clear_rs = if_clear_slb = if_clear_regfile = true;
      pc = rob_to_commit.val.result.pc;
    }
    if (insType == S_type) {
      commit_to_slb.stall = false;
      commit_to_slb.val = {rob_to_commit.val.slb_id};
    }

//    reg_prev.PrintReg();

  }
}

void RISC_V::CPU::update() {
  fq_prev = fq_next;
  reg_prev = reg_next;
  rs_prev = rs_next;
  rob_prev = rob_next;
  slb_prev = slb_next;

  slb_to_slb_prev = slb_to_slb_next;
  slb_to_rs_prev = slb_to_rs_next;
  slb_to_rob_prev = slb_to_rob_next;
}
