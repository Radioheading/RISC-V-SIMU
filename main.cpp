#include "src/program.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
//  freopen("testcases/tak.data", "r", stdin);
//  freopen("lxy.out", "w", stdout);
  Tomasulo CPU;
  CPU.LoadCommand();
  CPU.run();
  return 0;
}