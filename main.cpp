#include "src/program.hpp"

#include <bits/stdc++.h>

int main() {
  freopen("./testcases/qsort.data", "r", stdin);
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  Tomasulo CPU;
  CPU.LoadCommand();
  CPU.run();
  return 0;
}