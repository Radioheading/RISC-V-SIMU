#ifndef RISC_V_SIMU_SRC_LOAD_STORE_BUFFER_HPP_
#define RISC_V_SIMU_SRC_LOAD_STORE_BUFFER_HPP_

#include "utils.hpp"
#include "system.hpp"

struct load_store {
  opType type{};
  bool LR{false}; // true means L, false means R
  unsigned Vj{}, Vk{}, Dest{}, A{};
  int Qj{-1}, Qk{-1};

  friend std::ostream &operator<<(std::ostream &out, const load_store &ri) {
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
  void run(uint8_t *mem) {
    if (data.empty() || busy) return;
    load_store front = data.head(), &to_change = data_next.head();
    if (front.Qj == -1 && front.Qk == -1) {
      busy = true, countdown = 3;
      switch (front.type) {
        case LB: to_change.A = extend(read_from_memory(front.Vj + front.A, 1, mem), 7);
          break;
        case LH: to_change.A = extend(read_from_memory(front.Vj + front.A, 2, mem), 15);
          break;
        case LW:to_change.A = extend(read_from_memory(front.Vj + front.A, 4, mem), 31);
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
    }
  }

  void flush() {
    if (countdown) {
      --countdown;
      if (!countdown) {
        done = true;
      }
    }
    data = data_next;
  }
  void clear() {
    busy = false, countdown = 0, done = false, data_next.clear(), data.clear();
  }
};

#endif //RISC_V_SIMU_SRC_LOAD_STORE_BUFFER_HPP_
