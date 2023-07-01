#ifndef PROGRAM_HPP_
#define PROGRAM_HPP_

#include <algorithm>
#include <iomanip>
#include <random>
#include <string>
#include "load_store_buffer.hpp"
#include "reorder_buffer.hpp"
#include "reservation_station.hpp"
#include "system.hpp"
#include "utils.hpp"
#include "../lib/prediction.hpp"

class Tomasulo {
 private:
  int JALR_dependency{-1}, JALR_imm{-1};
  unsigned reg[32]{}, reg_nxt[32]{};
  int depend[32] = {-1}, depend_nxt[32] = {-1}; // dependency of registers: operation codes in ROB
  uint8_t memory[1 << 20]{};
  ReOrderBuffer ROB;
  CircularQueue<Operation> InsQueue;
  ReservationStation RS;
  LoadStoreBuffer LSB;
  unsigned cur_pc{0}, attempt{0}, success{0};
  unsigned main_clock{0};
  Predictor my_predict;
  bool ins_stall{false};

  void WriteReg(int _dest, int _value) {
    if (_dest) {
      reg_nxt[_dest] = _value;
    }
  }

  unsigned ReadReg(int _dest) {
    return reg[_dest];
  }

  void SetDepend(int _dest, int _value) {
    if (_dest) {
      depend_nxt[_dest] = _value;
    }
  }

  int GetDependency(int target) {
    return depend[target];
  }

  void RSUpdate() {
    for (int i = 0; i < 33; ++i) {
      if (RS.element[i].busy && !RS.element[i].ready) {
        if (RS.element[i].Qj != -1 && ROB.data[(RS.element[i].Qj)].ready) {
          RS.element_next[i].Vj = ROB.data[RS.element[i].Qj].value, RS.element_next[i].Qj = -1;
        }
        if (RS.element[i].Qk != -1 && ROB.data[(RS.element[i].Qk)].ready) {
          RS.element_next[i].Vk = ROB.data[RS.element[i].Qk].value, RS.element_next[i].Qk = -1;
        }
      }
    }
  }

  void LSBUpdate() {
    if (!LSB.data.empty()) {
      for (int i = 0; i < 33; ++i) {
        if (LSB.data[i].Qj != -1 && ROB.data[LSB.data[i].Qj].ready) {
          LSB.data_next[i].Vj = ROB.data[LSB.data[i].Qj].value, LSB.data_next[i].Qj = -1;
        }
        if (LSB.data[i].Qk != -1 && ROB.data[LSB.data[i].Qk].ready) {
          LSB.data_next[i].Vk = ROB.data[LSB.data[i].Qk].value, LSB.data_next[i].Qk = -1;
        }
      }
    }
  }

  void JALRUpdate() {
    if (JALR_dependency != -1 && ROB.data[JALR_dependency].ready) {
      cur_pc = (ROB.data[JALR_dependency].value + JALR_imm) & ~1;
      JALR_dependency = -1;
    }
  }

  void EraseDependency() {
    for (int i = 0; i < 32; ++i) {
      depend_nxt[i] = -1;
      depend[i] = -1;
    }
  }

  void ReadMemory(unsigned _value, unsigned _dest) {
    WriteReg(_dest, _value);
  }

  void WriteMemory(unsigned _value, const Operation &ins) {
    switch (ins.type) {
      case SB : write_to_memory(_value, 1, memory, extend(reg[ins.rs2], 7));
        break;
      case SH : write_to_memory(_value, 2, memory, extend(reg[ins.rs2], 15));
        break;
      case SW : write_to_memory(_value, 4, memory, extend(reg[ins.rs2], 31));
        break;
    }
  }
 public:
  void Fetch() {
    // Fetch from RS
    for (int i = 1; i < 33; ++i) {
      if (RS.element[i].ready && RS.element[i].busy) {
        RS.element_next[i].busy = false;
        ROB.data_next[RS.element[i].Dest].value = RS.element[i].A;
        ROB.data_next[RS.element[i].Dest].ready = true;
      }
    }
    // Fetch from LSB
    if (!LSB.data.empty()) {
      auto LSB_first = LSB.data.head();
      if (LSB.done) {
        if (LSB_first.LR) { // load
          ROB.data_next[LSB_first.Dest].value = LSB_first.A;
          ROB.data_next[LSB_first.Dest].ready = true;
        } else {
          ROB.data_next[LSB_first.Dest].dest = LSB_first.A;
          ROB.data_next[LSB_first.Dest].ready = true;
          ROB.data_next[LSB_first.Dest].value = LSB_first.Vk;
        }
      }
    }
  }

  void GetCommand() {
    if (!InsQueue.isFull() && !ins_stall && JALR_dependency == -1) {
      unsigned cmd = memory[cur_pc] | memory[cur_pc + 1] << 8 | memory[cur_pc + 2] << 16 | memory[cur_pc + 3] << 24;
      InsQueue.enQueue(Parse(cmd));
      ins_stall = true;
    }
  }
  void Issue() {
    if (JALR_dependency != -1 || InsQueue.empty()) { // stalled due to JALR
      return;
    }
    reorder_buffer_info todo;
    load_store new_ls;
    reserve_info new_rs;
    new_rs.pc = cur_pc, new_rs.busy = true;
    if (!ROB.isFull() && !InsQueue.empty()) { // only issue when RS/LSB and ROB are both available
      todo.op = InsQueue.head(), todo.ready = false, todo.pc = cur_pc;
      switch (todo.op.type) {
        case END : {
          todo.ready = true;
          todo.commit_type = End;
          ROB.insert(todo);
          ins_stall = true, cur_pc += 4;
          break;
        }
        case LUI : {
          todo.value = todo.op.imm;
          todo.ready = true;
          todo.dest = todo.op.rd;
          todo.commit_type = ChangeReg;
          ROB.insert(todo);
          SetDepend(todo.dest, ROB.data_next.back_id());
          InsQueue.deQueue();
          ins_stall = false, cur_pc += 4;
          break;
        }
        case AUIPC : {
          todo.value = todo.op.imm + cur_pc;
          todo.ready = true;
          todo.dest = todo.op.rd;
          todo.commit_type = ChangeReg;
          ROB.insert(todo);
          SetDepend(todo.dest, ROB.data_next.back_id());
          InsQueue.deQueue();
          ins_stall = false, cur_pc += 4;
          break;
        }
        case LB :
        case LH :
        case LW :
        case LBU :
        case LHU : {
          if (LSB.data.isFull()) {
            ins_stall = true;
            break;
          }
          todo.dest = todo.op.rd;
          todo.commit_type = ReadMem;
          new_ls.Qj = GetDependency(todo.op.rs1);
          if (new_ls.Qj == -1) {
            new_ls.Vj = reg[todo.op.rs1];
          } else if (ROB.data[new_ls.Qj].ready) {
            new_ls.Vj = ROB.data[new_ls.Qj].value;
            new_ls.Qj = -1;
          }
          ROB.insert(todo);
          SetDepend(todo.dest, ROB.data_next.back_id());
          new_ls.Dest = ROB.data_next.back_id();
          new_ls.type = todo.op.type;
          new_ls.A = todo.op.imm;
          new_ls.LR = true;
          LSB.data_next.enQueue(new_ls);
          LSB.data.enQueue(new_ls);
          InsQueue.deQueue();
          ins_stall = false, cur_pc += 4;
          break;
        }
        case SB :
        case SH :
        case SW : {
          if (LSB.data.isFull()) {
            ins_stall = true;
            break;
          }
          todo.dest = todo.op.rd;
          todo.commit_type = WriteMem;
          new_ls.Qj = GetDependency(todo.op.rs1), new_ls.Qk = GetDependency(todo.op.rs2);
          if (new_ls.Qj == -1) {
            new_ls.Vj = reg[todo.op.rs1];
          } else if (ROB.data[new_ls.Qj].ready) {
            new_ls.Vj = ROB.data[new_ls.Qj].value;
            new_ls.Qj = -1;
          }
          if (new_ls.Qk == -1) {
            new_ls.Vk = reg[todo.op.rs2];
          } else if (ROB.data[new_ls.Qk].ready) {
            new_ls.Vk = ROB.data[new_ls.Qk].value;
            new_ls.Qk = -1;
          }
          ROB.insert(todo);
          new_ls.Dest = ROB.data_next.back_id();
          new_ls.type = todo.op.type;
          new_ls.A = todo.op.imm;
          new_ls.LR = false;
          LSB.data_next.enQueue(new_ls);
          LSB.data.enQueue(new_ls);
          InsQueue.deQueue();
          ins_stall = false, cur_pc += 4;
          break;
        }
        case ADDI :
        case SLTI :
        case SLTIU :
        case XORI :
        case ORI :
        case ANDI : {
          if (RS.isFull()) {
            ins_stall = true;
            break;
          }
          todo.dest = todo.op.rd;
          todo.commit_type = ChangeReg;
          new_rs.Qj = GetDependency(todo.op.rs1);
          if (new_rs.Qj == -1) {
            new_rs.Vj = reg[todo.op.rs1];
          } else if (ROB.data[new_rs.Qj].ready) {
            new_rs.Vj = ROB.data[new_rs.Qj].value;
            new_rs.Qj = -1;
          }
          ROB.insert(todo);
          SetDepend(todo.dest, ROB.data_next.back_id());
          new_rs.Dest = ROB.data_next.back_id();
          new_rs.type = todo.op.type;
          new_rs.A = todo.op.imm;
          RS.insert(new_rs);
          InsQueue.deQueue();
          ins_stall = false, cur_pc += 4;
          break;
        }
        case SLLI :
        case SRLI :
        case SRAI : {
          if (RS.isFull()) {
            ins_stall = true;
            break;
          }
          todo.dest = todo.op.rd;
          todo.commit_type = ChangeReg;
          new_rs.Qj = GetDependency(todo.op.rs1);
          if (new_rs.Qj == -1) {
            new_rs.Vj = reg[todo.op.rs1];
          } else if (ROB.data[new_rs.Qj].ready) {
            new_rs.Vj = ROB.data[new_rs.Qj].value;
            new_rs.Qj = -1;
          }
          new_rs.Vk = todo.op.shamt;
          ROB.insert(todo);
          SetDepend(todo.dest, ROB.data_next.back_id());
          new_rs.Dest = ROB.data_next.back_id();
          new_rs.type = todo.op.type;
          new_rs.A = todo.op.imm;
          RS.insert(new_rs);
          InsQueue.deQueue();
          ins_stall = false, cur_pc += 4;
          break;
        }
        case ADD :
        case SUB :
        case SLL :
        case SLT :
        case SLTU :
        case XOR :
        case SRL :
        case SRA :
        case OR :
        case AND : {
          if (RS.isFull()) {
            ins_stall = true;
            break;
          }
          todo.dest = todo.op.rd;
          todo.commit_type = ChangeReg;
          new_rs.Qj = GetDependency(todo.op.rs1), new_rs.Qk = GetDependency(todo.op.rs2);
          if (new_rs.Qj == -1) {
            new_rs.Vj = reg[todo.op.rs1];
          } else if (ROB.data[new_rs.Qj].ready) {
            new_rs.Vj = ROB.data[new_rs.Qj].value;
            new_rs.Qj = -1;
          }
          if (new_rs.Qk == -1) {
            new_rs.Vk = reg[todo.op.rs2];
          } else if (ROB.data[new_rs.Qk].ready) {
            new_rs.Vk = ROB.data[new_rs.Qk].value;
            new_rs.Qk = -1;
          }
          ROB.insert(todo);
          SetDepend(todo.dest, ROB.data_next.back_id());
          new_rs.Dest = ROB.data_next.back_id();
          new_rs.type = todo.op.type;
          RS.insert(new_rs);
          InsQueue.deQueue();
          ins_stall = false, cur_pc += 4;
          break;
        }
        case BEQ :
        case BNE :
        case BLT :
        case BGE :
        case BLTU :
        case BGEU : {
          if (RS.isFull()) {
            ins_stall = true;
            break;
          }
//          bool my_guess = my_predict.predict(cur_pc);
//          if (my_guess) {
//            cur_pc += todo.op.imm - 4;
//          }
//          todo.judge = my_guess;
          todo.commit_type = Branch;
          todo.dest = cur_pc + todo.op.imm;
          new_rs.Qj = GetDependency(todo.op.rs1), new_rs.Qk = GetDependency(todo.op.rs2);
          if (new_rs.Qj == -1) {
            new_rs.Vj = reg[todo.op.rs1];
          } else if (ROB.data[new_rs.Qj].ready) {
            new_rs.Vj = ROB.data[new_rs.Qj].value;
            new_rs.Qj = -1;
          }
          if (new_rs.Qk == -1) {
            new_rs.Vk = reg[todo.op.rs2];
          } else if (ROB.data[new_rs.Qk].ready) {
            new_rs.Vk = ROB.data[new_rs.Qk].value;
            new_rs.Qk = -1;
          }
          ROB.insert(todo);
          new_rs.Dest = ROB.data_next.back_id();
          new_rs.type = todo.op.type;
          RS.insert(new_rs);
          InsQueue.deQueue();
          ins_stall = true;
          // ins_stall = false, cur_pc += 4;
          break;
        }
        case JAL : {
          todo.commit_type = ChangeReg, todo.dest = todo.op.rd;
          todo.value = cur_pc + 4, todo.ready = true;
          ROB.insert(todo);
          SetDepend(todo.dest, ROB.data_next.back_id());
          InsQueue.deQueue();
          ins_stall = false, cur_pc += todo.op.imm;
          break;
        }
        case JALR : {
          if (RS.isFull()) {
            ins_stall = true;
            break;
          }
          todo.commit_type = ChangeReg, todo.dest = todo.op.rd;
          todo.value = cur_pc + 4, todo.ready = true;
          ROB.insert(todo);
          SetDepend(todo.dest, ROB.data_next.back_id());
          JALR_dependency = GetDependency(todo.op.rs1);
          if (JALR_dependency == -1) {
            cur_pc = (reg[todo.op.rs1] + todo.op.imm) & ~1;
          } else { // InsQueue is stalled
            JALR_imm = todo.op.imm;
          }
          InsQueue.deQueue();
          ins_stall = false;
          break;
        }
      }
    } else {
      ins_stall = true;
    }
  }

  void Update() {
    RSUpdate();
    LSBUpdate();
    JALRUpdate();
  }

  void Execute() {
    LSB.run(memory);
    RS.run();
  }

  void Commit() {
    if (ROB.data.empty() || !ROB.data.head().ready) {
      return;
    }
    reorder_buffer_info todo = ROB.data.head();
    if (todo.commit_type == ChangeReg) {// change register
      WriteReg(todo.dest, todo.value);
      if (depend_nxt[todo.dest] == ROB.data.head_id()) {
        // really depend!
        // the operation being committed IS the one
        depend_nxt[todo.dest] = -1;
      }
    } else if (todo.commit_type == ReadMem) {
      ReadMemory(todo.value, todo.dest);
      if (depend_nxt[todo.dest] == ROB.data.head_id()) {
        depend_nxt[todo.dest] = -1;
      }
      LSB.data_next.deQueue();
      LSB.data.deQueue();
      LSB.busy = false;
      LSB.done = false;
    } else if (todo.commit_type == WriteMem) {
      WriteMemory(todo.dest, todo.op);
      LSB.data_next.deQueue();
      LSB.data.deQueue();
      LSB.busy = false;
      LSB.done = false;
    } else if (todo.commit_type == Branch) {
      bool guess = todo.judge, real = todo.value;
//      my_predict.flush(todo.pc, real);
//      ++attempt;
//      if (guess != real) {
//        ROB.clear(), RS.clear(), LSB.clear(), InsQueue.clear(), ins_stall = false;
//        EraseDependency();
//        JALR_dependency = -1;
//        if (real) {
//          cur_pc = todo.dest;
//        } else {
//          cur_pc = todo.pc + 4;
//        }
//      } else {
//        ++success;
//      }
      if (real) {
        cur_pc = todo.dest;
      } else {
        cur_pc = todo.pc + 4;
      }
      ins_stall = false;
    } else if (todo.commit_type == End) {
      std::cout << int(reg[10] & 255u) << '\n';
//      std::cout << attempt << ' ' << success << '\n';
//      std::cout << std::fixed << std::setprecision(5) << PredictionAccurancy() << '\n';
//      std::cout << "clocks: " << main_clock << '\n';
      exit(0);
    }
    if (!ROB.data_next.empty()) {
      ROB.data_next.deQueue();
    }
  }

  void Flush() {
    LSB.flush(), RS.flush(), ROB.flush();
    for (int i = 0; i < 32; ++i) {
      reg[i] = reg_nxt[i], depend[i] = depend_nxt[i];
    }
  }

  void LoadCommand() {
    for (int i = 0; i < 32; ++i) {
      reg[i] = reg_nxt[i] = 0, depend[i] = depend_nxt[i] = -1;
    }
    unsigned addr = 0;
    std::string info;
    while (std::cin >> info)
      if (info[0] == '@') {
        addr = std::stoi(info.substr(1), nullptr, 16);
      } else {
        memory[addr] = std::stoi(info, nullptr, 16);
        ++addr;
      }
  }

  void run() {
    int order[6] = {0, 1, 2, 3, 4, 5};
    while (true) {
      std::random_shuffle(order, order + 6);
      for (int i = 0; i < 6; ++i) {
        switch (order[i]) {
          case 0:GetCommand();
            break;
          case 1:Issue();
            break;
          case 2:Fetch();
            break;
          case 3:Update();
            break;
          case 4:Commit();
            break;
          case 5:Execute();
            break;
        }
      }
      Flush();
      ++main_clock;
    }
  }

  double PredictionAccurancy() const {
    return (double)success / attempt;
  }
};

#endif