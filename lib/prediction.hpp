#ifndef PREDICTION_HPP_
#define PREDICTION_HPP_

const int mod = 127;

unsigned my_hash(size_t x) {
  return x % mod;
}

class two_digit_predictor {
  uint8_t cur = 0b01;
  friend class Predictor;
  void flush(bool truth) {
    if (truth && cur != 0b11) ++cur;
    if (!truth && cur != 0b00) --cur;
  }

  bool predict() const {
    return cur & 0b10;
  }

  void reset() {
    cur = 0b01;
  }
};

class Predictor {
  two_digit_predictor my_predictor[mod];
 public:
  void flush(size_t cur_pc, bool truth) {
    my_predictor[my_hash(cur_pc)].flush(truth);
  }

  bool predict(size_t cur_pc) const {
    return my_predictor[my_hash(cur_pc)].predict();
  }

  void reset() {
    for (int i = 0; i < mod; ++i) {
      my_predictor[i].reset();
    }
  }
};

#endif