#include <iostream>
#include "CPU.hpp"

int main() {
  RISC_V::CPU cpu;

//  freopen("../test/testcases/pi.data", "r", stdin);
  //freopen("../test/my_test/tomasulo/sample.ans", "w", stdout);
//  try {
    cpu.scan();
    cpu.run();
//  }
//  catch (Error &get_error) {
//    std::cerr << get_error.what() << std::endl;
//  }


  //std::cout << "Hello, World!" << std::endl;
  return 0;
}
