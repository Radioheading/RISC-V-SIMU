#include "src/program.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  Tomasulo CPU;
  CPU.LoadCommand();
  CPU.run();
  return 0;
}