#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <cstring>
#include <string>
#include <unordered_map>

using shortType = uint8_t;
using longType = uint32_t;

enum opType {
  END, LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT,
  BGE, BLTU, BGEU, LB, LH, LW, LBU, LHU, SB,
  SH, SW, ADDI, SLTI, SLTIU, XORI, ORI, ANDI,
  SLLI, SRLI, SRAI, ADD, SUB, SLL, SLT, SLTU,
  XOR, SRL, SRA, OR, AND
};

std::string change_string(const opType &op) {
  switch (op) {
    case END: return "END";
    case LUI: return "LUI";
    case AUIPC: return "AUIPC";
    case JAL: return "JAL";
    case JALR: return "JALR";
    case BEQ: return "BEQ";
    case BNE: return "BNE";
    case BLT: return "BLT";
    case BGE: return "BGE";
    case BLTU: return "BLTU";
    case BGEU: return "BGEU";
    case LH: return "LH";
    case LW: return "LW";
    case LBU: return "LBU";
    case LHU: return "LHU";
    case SB: return "SB";
    case SH: return "SH";
    case SW: return "SW";
    case ADDI: return "ADDI";
    case SLTI: return "SLTI";
    case SLTIU: return "SLTIU";
    case XORI: return "XORI";
    case ANDI: return "ANDI";
    case SLLI: return "SLLI";
    case SRLI: return "SRLI";
    case SRAI: return "SRAI";
    case ADD: return "ADD";
    case SUB: return "SUB";
    case SLL: return "SLL";
    case SLT: return "SLT";
    case SLTU: return "SLTU";
    case XOR: return "XOR";
    case LB: return "LB";
    case ORI: return "ORI";
    case SRL: return "SRL";
    case SRA: return "SRA";
    case OR: return "OR";
    case AND: return "AND";
  }
}

std::unordered_map<longType, opType> OperationCode{
    {0b00000000000110111, LUI}, {0b00000000000010111, AUIPC},
    {0b00000000001101111, JAL}, {0b00000000001100111, JALR},
    {0b00000000001100011, BEQ}, {0b00000000011100011, BNE},
    {0b00000001001100011, BLT}, {0b00000001011100011, BGE},
    {0b00000001101100011, BLTU}, {0b00000001111100011, BGEU},
    {0b00000000000000011, LB}, {0b00000000010000011, LH},
    {0b00000000100000011, LW}, {0b00000001000000011, LBU},
    {0b00000001010000011, LHU}, {0b00000000000100011, SB},
    {0b00000000010100011, SH}, {0b00000000100100011, SW},
    {0b00000000000010011, ADDI}, {0b00000000100010011, SLTI},
    {0b00000000110010011, SLTIU}, {0b00000001000010011, XORI},
    {0b00000001100010011, ORI}, {0b00000001110010011, ANDI},
    {0b00000000010010011, SLLI}, {0b00000001010010011, SRLI},
    {0b01000001010010011, SRAI}, {0b00000000000110011, ADD},
    {0b01000000000110011, SUB}, {0b00000000010110011, SLL},
    {0b00000000100110011, SLT}, {0b00000000110110011, SLTU},
    {0b00000001000110011, XOR}, {0b00000001010110011, SRL},
    {0b01000001010110011, SRA}, {0b00000001100110011, OR},
    {0b00000001110110011, AND}};

std::unordered_map<longType, char> BigType = {{0b0110111, 'U'}, {0b0010111, 'U'}, {0b1101111, 'J'},
                                              {0b1100111, 'I'}, {0b1100011, 'B'}, {0b0000011, 'I'},
                                              {0b0100011, 'S'}, {0b0010011, 'I'}, {0b0110011, 'R'}};

std::unordered_map<opType, char>
    ToGeneral = {{LUI, 'U'}, {AUIPC, 'U'}, {JAL, 'J'}, {JALR, 'J'}, {BEQ, 'B'}, {BNE, 'B'},
                 {BLT, 'B'}, {BGE, 'B'}, {BLTU, 'B'}, {BGEU, 'B'}, {LB, 'I'}, {LH, 'I'},
                 {LW, 'I'}, {LBU, 'I'}, {LHU, 'I'}, {SB, 'S'}, {SH, 'S'}, {SW, 'S'}, {ADDI, 'I'}, {SLTI, 'I'},
                 {SLTIU, 'I'}, {XORI, 'I'}, {ORI, 'I'}, {ANDI, 'I'}, {SLLI, 'I'}, {SRLI, 'I'}, {SRAI, 'I'}, {ADD, 'R'},
                 {SUB, 'R'}, {SLL, 'R'}, {SLT, 'R'}, {SLTU, 'R'}, {XOR, 'R'}, {SRL, 'R'}, {SRA, 'R'}, {OR, 'R'},
                 {AND, 'R'}};

longType getSub(longType todo, int r, int l) {
  return (todo >> l) & ((1u << (r - l + 1)) - 1u);
}

unsigned extend(unsigned todo, int num) {
  return todo >> num ? -1 ^ (1u << num) - 1 | todo : todo;
}

struct Operation {
  opType type{};
  int imm{};
  shortType rs2{}, rs1{}, rd{}, shamt{};

  explicit operator std::string() const {
    return "type: " + change_string(type) + ", imm: " + std::to_string((int)imm) + '\n'
        + "rs2: " + std::to_string(rs2) + ", rs1: " + std::to_string(rs1) + ", rd: " + std::to_string(rd)
        + ", shamt: " + std::to_string(shamt);
  }
};

Operation Parse(longType command) {
  Operation ret;
  if (command == 0x0ff00513) {
    ret.type = END;
    return ret;
  }
  longType general_type = getSub(command, 6, 0), p1{}, p2{};
  char big_type = BigType[general_type];
  // p1 is the 14 to 12 digit, p2 is the 31 to 25 digit
  if (big_type == 'U' || big_type == 'J') {
    ret.type = OperationCode[general_type];
  } else {
    p1 = getSub(command, 14, 12);
    if (big_type == 'R' || big_type == 'I' && p1 == 0b101) {
      p2 = getSub(command, 31, 25);
    }
    ret.type = OperationCode[p2 << 10 | p1 << 7 | general_type];
  }
  if (big_type == 'U') {
    ret.imm = extend(getSub(command, 31, 12) << 12, 31);
    ret.rd = getSub(command, 11, 7);
  } else if (big_type == 'J') {
    ret.imm = extend(getSub(command, 31, 31) << 20 | getSub(command, 30, 21) << 1 | getSub(command, 20, 20) << 11
                         | getSub(command, 19, 12) << 12, 20);
    ret.rd = getSub(command, 11, 7);
  } else if (big_type == 'B') {
    ret.imm = extend(getSub(command, 31, 31) << 12 | getSub(command, 30, 25) << 5 | getSub(command, 11, 8) << 1
                         | getSub(command, 7, 7) << 11, 12);
    ret.rs1 = getSub(command, 19, 15);
    ret.rs2 = getSub(command, 24, 20);
  } else if (big_type == 'I') {
    ret.imm = extend(getSub(command, 31, 20), 11);
    ret.rd = getSub(command, 11, 7);
    ret.rs1 = getSub(command, 19, 15);
    ret.shamt = getSub(command, 24, 20);
  } else if (big_type == 'S') {
    ret.imm = extend(getSub(command, 31, 25) << 5 | getSub(command, 11, 7), 11);
    ret.rs1 = getSub(command, 19, 15);
    ret.rs2 = getSub(command, 24, 20);
  } else if (big_type == 'R') {
    ret.rd = getSub(command, 11, 7);
    ret.rs1 = getSub(command, 19, 15);
    ret.rs2 = getSub(command, 24, 20);
  }
  return ret;
}

/*
 * class: CircularQueue
 * for storing the information in the ROB
 */
template<class T, int capacity = 32>
class CircularQueue {
  T data[capacity + 1]{};
  int front = 0, rear = 0;

 public:
  CircularQueue() = default;

  bool empty() const {
    return front == rear;
  }

  void enQueue(const T &x) {
    rear = (rear + 1) % capacity;
    data[rear] = x;
  }

  T deQueue() {
    front = (front + 1) % capacity;
    return data[front];
  }

  bool isFull() const {
    return front == (rear + 1) % capacity;
  }

  T &head() {
    return data[(front + 1) % capacity];
  }

  size_t head_id() const {
    return (front + 1) % capacity;
  }

  size_t back_id() const {
    return rear % capacity;
  };

  T &operator[](int index) {
    return data[index];
  };

  T operator[](int index) const {
    return data[index];
  }

  void clear() {
    while (!empty()) {
      deQueue();
    }
  }

  class iterator {
   private:
    int id;
    CircularQueue<T, capacity> *origin;
   public:
    iterator(int _id, CircularQueue<T, capacity> *_origin) : id(_id), origin(_origin) {}
    iterator operator++(int x) {
      iterator ret = *this;
      id = (id + 1) % capacity;
      return *this;
    }

    iterator operator++() {
      return iterator((id + 1) % capacity, origin);
    }

    friend bool operator==(const iterator &cmp_1, const iterator cmp_2) {
      return cmp_1.id == cmp_2.id && cmp_1.origin == cmp_2.origin;
    }
    friend bool operator!=(const iterator &cmp_1, const iterator cmp_2) {
      return cmp_1.id != cmp_2.id || cmp_1.origin != cmp_2.origin;
    }
  };

  iterator end() const {
    return iterator((rear + 1) % capacity, this);
  }

  iterator begin() const {
    return iterator((front + 1) % capacity, this);
  }
};

#endif