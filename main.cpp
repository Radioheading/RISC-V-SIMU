/*
 * //todo: 2/开始array_test2(√)
 *         3/调完parse(√）
 */

#include "src/program.hpp"

int main() {
  freopen("testcases/gcd.data", "r", stdin);
  freopen("lxy.out", "w", stdout);
  Tomasulo CPU;
  CPU.LoadCommand();
  CPU.run();
  // fclose(stdin);
  return 0;
}