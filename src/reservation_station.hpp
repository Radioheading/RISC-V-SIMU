#ifndef RISC_V_SIMU_SRC_RESERVATION_STATION_HPP_
#define RISC_V_SIMU_SRC_RESERVATION_STATION_HPP_

struct reserve_info {
  opType type{};
  bool busy{false}, ready{false};
  unsigned Vj{}, Vk{}, Dest{}, pc{};
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
        break;
      case SLTI: element_next[i].A = ((int) element[i].Vj < (int) element[i].A);
        break;
      case SLTIU: element_next[i].A = (element[i].Vj < element[i].A);
        break;
      case XORI: element_next[i].A = (element[i].Vj ^ (unsigned) element[i].A);
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
};

#endif //RISC_V_SIMU_SRC_RESERVATION_STATION_HPP_
