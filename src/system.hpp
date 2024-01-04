#ifndef SYSTEM_HPP_
#define SYSTEM_HPP_

unsigned read_from_memory(unsigned addr, int length, const uint8_t *mem) {
//  std::cout << "read from memory: " << addr << ' ' << length << '\n';
  uint8_t a[4] = {0, 0, 0, 0};
  for (unsigned i = addr; i < addr + length; ++i) {
    a[i - addr] = mem[i];
  }
//  std::cerr << "load address:\t" << addr << ", value:\t" << (a[3] << 24 | a[2] << 16 | a[1] << 8 | a[0]) << '\n';
  return a[3] << 24 | a[2] << 16 | a[1] << 8 | a[0];
}

void write_to_memory(unsigned addr, int length, uint8_t *mem, unsigned _value) {
//  std::cerr << "store address:\t" << addr << ", size:\t" << length << ", value:\t" << _value << '\n';
  if (addr == 0x30000) {
    std::cerr << char(_value);
  } else if (addr == 0x30004) {
    exit(0);
  }
  if (length == 1) {
    mem[addr] = getSub(_value, 7, 0);
  } else if (length == 2) {
    mem[addr] = getSub(_value, 7, 0);
    mem[addr + 1] = getSub(_value, 15, 8);
  } else {
    mem[addr] = getSub(_value, 7, 0);
    mem[addr + 1] = getSub(_value, 15, 8);
    mem[addr + 2] = getSub(_value, 23, 16);
    mem[addr + 3] = getSub(_value, 31, 24);
  }
}

#endif