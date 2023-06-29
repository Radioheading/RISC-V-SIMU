/*
 * //todo: 1/封装修改dependency和寄存器读写（规避0）
 *         2/开始调第二个点
 *         3/调完parse(√）
 */

#include "src/program.hpp"

int main() {
  // freopen("testcases/array_test1.data", "r", stdin);
  // freopen("lxy.out", "w", stdout);
  Tomasulo CPU;
  CPU.LoadCommand();
  CPU.run();
  // fclose(stdin);
  return 0;
}