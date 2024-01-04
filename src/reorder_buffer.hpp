#ifndef RISC_V_SIMU_SRC_REORDER_BUFFER_HPP_
#define RISC_V_SIMU_SRC_REORDER_BUFFER_HPP_

#include "utils.hpp"

enum CommitType {
  ChangeReg, ReadMem, WriteMem, Branch
};

std::string change_string(const CommitType &type) {
  switch (type) {
    case ChangeReg : return "ChangeReg";
    case ReadMem : return "ReadMem";
    case WriteMem : return "WriteMem";
    case Branch : return "Branch";
  }
}

struct reorder_buffer_info {
  Operation op;
  CommitType commit_type{};
  bool ready{}, judge{};
  unsigned value{}, dest{}, pc{};

  friend std::ostream &operator<<(std::ostream &out, const reorder_buffer_info &rob_element) {
    out << "value: " << rob_element.value << ", commit_type: " << change_string(rob_element.commit_type) << '\n';
    out << "ready: " << rob_element.ready << ", judge: " << rob_element.judge << ", dest: " << rob_element.dest
        << ", pc: " << rob_element.pc;
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

  bool isFull() {
    return data.isFull();
  }

  void insert(const reorder_buffer_info &to_insert) {
    data_next.enQueue(to_insert);
  }

  void pop_front() {
    data_next.deQueue();
  }

  void clear() {
    data_next.clear(), data.clear();
  }
};

#endif //RISC_V_SIMU_SRC_REORDER_BUFFER_HPP_
