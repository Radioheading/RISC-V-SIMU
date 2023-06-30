/*
 * //todo: 2/开始array_test2(√)
 *         3/调完parse(√）
 */

#include "src/program.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
//  freopen("testcases/magic.data", "r", stdin);
//  freopen("lxy.out", "w", stdout);
  Tomasulo CPU;
  CPU.LoadCommand();
  CPU.run();
  // fclose(stdin);
  return 0;
}