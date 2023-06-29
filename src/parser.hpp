#include "utils.hpp"

enum broadType {
  R = 1, I = 2, S = 3, B = 4, U = 5, J = 6
};

enum specType {

};

bool parse(longType cmd, Program program) {
  if (cmd == 0x0ff00513) {
    std::cout << (program.reg_now[10] & 11111111) << '\n';
    return false;
  }

  return true;
}
