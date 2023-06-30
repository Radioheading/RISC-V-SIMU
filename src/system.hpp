#ifndef SYSTEM_HPP_
#define SYSTEM_HPP_

#include "utils.hpp"

enum CommitType {
  ChangeReg, ReadMem, WriteMem, Branch, End
};

std::string change_string(const CommitType &type) {
  switch (type) {
    case ChangeReg : return "ChangeReg";
    case ReadMem : return "ReadMem";
    case WriteMem : return "WriteMem";
    case Branch : return "Branch";
    case End : return "End";
  }
}

longType read_from_memory(longType addr, int length, const shortType *mem) {\
  shortType a[4] = {0, 0, 0, 0};
  for (longType i = addr; i < addr + length; ++i) {
    a[i - addr] = mem[i];
  }
//  std::cout << "read_address: " << addr << ", value: " << (a[3] << 24 | a[2] << 16 | a[1] << 8 | a[0]) << '\n';
  return a[3] << 24 | a[2] << 16 | a[1] << 8 | a[0];
}

void write_to_memory(longType addr, int length, shortType *mem, longType _value) {
  // std::cout << "write_address: "<< addr << ", value: " << _value << '\n';
  if (length == 1) {
    mem[addr] = getSub(_value, 7, 0);
  } else if (length == 2) {
    mem[addr] = getSub(_value, 7, 0);
    mem[addr + 1] = getSub(_value, 15, 8);
  } else {
    mem[addr] = getSub(_value, 7, 0);
    mem[addr + 1] = getSub(_value, 15, 8);
    mem[addr + 2] = getSub(_value, 23, 16);
    mem[addr + 3] = getSub(_value, 31, 24);
  }
}

struct reorder_buffer_info {
  Operation op;
  CommitType commit_type{};
  bool ready{}, judge{};
  longType value{}, dest{}, pc{};

  friend std::ostream&operator<<(std::ostream &out, const reorder_buffer_info &rob_element) {
    out << "value: " << rob_element.value << ", commit_type: " << change_string(rob_element.commit_type) << '\n';
    out << "ready: " << rob_element.ready << ", judge: " << rob_element.judge << ", dest: " << rob_element.dest << ", pc: " << rob_element.pc;
    return out;
  }
};

class ReOrderBuffer {
  friend class Tomasulo;
 private:
  CircularQueue<reorder_buffer_info> data, data_next;
 public:
  void flush() {
    data = data_next;
  }

  reorder_buffer_info &W(int index) {
    return data_next[index];
  }

  reorder_buffer_info R(int index){
    return data[index];
  }

  bool isFull() {
    return data.isFull();
  }

  void insert(const reorder_buffer_info &to_insert) {
//    std::cout << "Issuing!\n";
//    std::cout << std::string(to_insert.op) << '\n';
//    std::cout << to_insert << '\n';
    data_next.enQueue(to_insert);
  }

  void pop_front() {
    data_next.deQueue();
  }

  void clear() {
    data_next.clear(), data.clear();
  }

  void print() const {
    for (int i = (data.head_id() + 1) % 33; i != (data.back_id() + 1) % 33; i = (i + 1) % 33) {
      // std::cout << data[i] << '\n';
    }
  }
};

struct load_store {
  opType type{};
  bool LR{false}; // true means L, false means R
  longType Vj{}, Vk{}, Dest{}, A{};
  int Qj{-1}, Qk{-1};

  friend std::ostream&operator<<(std::ostream &out, const load_store &ri) {
    out << "opType: " << change_string(ri.type) << '\n';
    out << "Vj: " << ri.Vj << ", Vk: " << ri.Vk << ", Dest: " << ri.Dest << '\n';
    out << "Qj: " << ri.Qj << ", Qk: " << ri.Qk << ", A: " << ri.A;
    return out;
  }
};

class LoadStoreBuffer {
 private:
  bool busy{false}, done{false};
  int countdown{};
  CircularQueue<load_store> data, data_next;
  friend class Tomasulo;
 public:
  void run(shortType *mem) {
    if (data.empty() || busy) return;
    load_store front = data.head(), &to_change = data_next.head();
    if (front.Qj == -1 && front.Qk == -1) {
//      std::cout << "LSB is running!\n";
//      std::cout << front << '\n';
      busy = true, countdown = 3;
      switch (front.type) {
        case LB: to_change.A = extend(read_from_memory(front.Vj + front.A, 1, mem), 7);
          break;
        case LH: to_change.A = extend(read_from_memory(front.Vj + front.A, 2, mem), 15);
          break;
        case LW:
//          std::cout << front.Vj + front.A << '\n';
          to_change.A = extend(read_from_memory(front.Vj + front.A, 4, mem), 31);
          break;
        case LBU: to_change.A = read_from_memory(front.Vj + front.A, 1, mem);
          break;
        case LHU: to_change.A = read_from_memory(front.Vj + front.A, 2, mem);
          break;
        case SB:
        case SH:
        case SW: to_change.A = front.Vj + front.A;
          break;
      }
      // std::cout << "answer: " << to_change.A << ", type: " << (to_change.LR ? "left" : "right") << '\n';
    }
  }

  void flush() {
    if (countdown) {
      --countdown;
      if (!countdown) {
        done = true;
      }
    }
    // std::cout << "remaining: " << countdown << '\n';
    data = data_next;
  }

  load_store &W(int index) {
    return data_next[index];
  }

  load_store R(int index) {
    return data[index];
  }

  void clear() {
    busy = false, countdown = 0, done = false, data_next.clear(), data.clear();
  }

  void print() const {
    for (int i = (data.head_id() + 1) % 33; i != (data.back_id() + 1) % 33; i = (i + 1) % 33) {
      // std::cout << data[i] << '\n';
    }
  }
};

struct reserve_info {
  opType type{};
  bool busy{false}, ready{false};
  longType Vj{}, Vk{}, Dest{}, pc{};
  int Qj{-1}, Qk{-1};
  int A{-1};
  friend std::ostream&operator<<(std::ostream &out, const reserve_info &ri) {
    out << "opType: " << change_string(ri.type) << ", busy: " << ri.busy << ", ready: " << ri.ready << '\n';
    out << "Vj: " << ri.Vj << ", Vk: " << ri.Vk << ", Dest: " << ri.Dest << ", pc: " << ri.pc << '\n';
    out << "Qj: " << ri.Qj << ", Qk: " << ri.Qk << ", A: " << ri.A;
    return out;
  }
};
/*
 * class: ReservationStation
 * buffer of the issued commands, busy when computed and idle when operands remain
 */
class ReservationStation {
  friend class Tomasulo;
 private:
  reserve_info element[33], element_next[33];
 public:
  void insert(const reserve_info &todo) {
    for (int i = 1; i < 33; ++i) {
      if (!element[i].busy) {
        element_next[i] = todo;
        element_next[i].busy = true;
        return;
      }
    }
  }

  bool isFull() const {
    for (int i = 1; i < 33; ++i) {
      if (!element[i].busy) {
        return false;
      }
    }
    return true;
  }

  void clear() {
    for (int i = 1; i < 33; ++i) {
      element_next[i].busy = false;
      element[i].busy = false;
    }
  }

  reserve_info &W(int index) {
    return element_next[index];
  }

  reserve_info R(int index) {
    return element[index];
  }

  void run() {
    int i;
    for (i = 1; i < 33; ++i) {
      if (element[i].busy && !element[i].ready && element[i].Qj == -1 && element[i].Qk == -1) {
        break; // can only execute one instruction at a cycle
      }
    }
    if (i == 33) return;
    // todo: check the problem of int/unsigned
//    std::cout << "RS is running!\n";
//    std::cout << element[i] << '\n';
    switch (element[i].type) {
      case BEQ: element_next[i].A = ((int) element[i].Vj == (int) element[i].Vk);
        break;
      case BNE: element_next[i].A = ((int) element[i].Vj != (int) element[i].Vk);
        break;
      case BLT: element_next[i].A = ((int) element[i].Vj < (int) element[i].Vk);
        break;
      case BGE: element_next[i].A = ((int) element[i].Vj >= (int) element[i].Vk);
        break;
      case BLTU: element_next[i].A = (element[i].Vj < element[i].Vk);
        break;
      case BGEU: element_next[i].A = (element[i].Vj >= element[i].Vk);
        break;
      case ADDI: element_next[i].A = element[i].Vj + element[i].A;
//        std::cout << "trying to compute!\n";
//        std::cout << element_next[i].A << ' ' << element[i].Vj <<'\n';
        break;
      case SLTI: element_next[i].A = ((int) element[i].Vj < (int) element[i].A);
        break;
      case SLTIU: element_next[i].A = (element[i].Vj < element[i].A);
        break;
      case XORI: element_next[i].A = (element[i].Vj ^ (longType) element[i].A);
        break;
      case ORI: element_next[i].A = element[i].Vj | element[i].A;
        break;
      case ANDI: element_next[i].A = element[i].Vj & element[i].A;
        break;
      case SLLI: element_next[i].A = element[i].Vj << element[i].Vk;
        break;
      case SRLI: element_next[i].A = element[i].Vj >> element[i].Vk;
        break;
      case SRAI: element_next[i].A = (int) element[i].Vj >> element[i].Vk;
        break;
      case ADD: element_next[i].A = element[i].Vj + element[i].Vk;
        break;
      case SUB: element_next[i].A = element[i].Vj - element[i].Vk;
        break;
      case SLL: element_next[i].A = element[i].Vj << element[i].Vk;
        break;
      case SLT: element_next[i].A = (int) element[i].Vj < (int) element[i].Vk;
        break;
      case SLTU: element_next[i].A = element[i].Vj < element[i].Vk;
        break;
      case XOR: element_next[i].A = element[i].Vj ^ element[i].Vk;
        break;
      case SRL: element_next[i].A = element[i].Vj >> element[i].Vk;
        break;
      case SRA: element_next[i].A = (int) element[i].Vj >> element[i].Vk;
        break;
      case OR: element_next[i].A = element[i].Vj | element[i].Vk;
        break;
      case AND: element_next[i].A = element[i].Vj & element[i].Vk;
        break;
    }
    element_next[i].ready = true;
  }

  void flush() {
    for (int i = 1; i < 33; ++i) {
      element[i] = element_next[i];
    }
  }

  void print() const {
    for (int i = 1; i < 33; ++i) {
      std::cout << element[i] << '\n';
    }
  }
};
#endif