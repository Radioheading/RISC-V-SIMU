#ifndef PREDICTION_HPP_
#define PREDICTION_HPP_

const int mod = 1009;

unsigned my_hash(size_t x) {
  return x % mod;
}

class two_digit_predictor {
  int cur = 0b00;
  friend class Predictor;
  void flush(bool truth) {
    if (truth && cur < 3) ++cur;
    if (!truth && cur > 0) --cur;
  }

  bool predict() const {
    return cur ^ 0b10;
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
};

#endif