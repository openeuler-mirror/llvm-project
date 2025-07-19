#include "pi.h"
#include <iostream>
void func() {
  double d = pi::value<double>;
  std::cout << "func: " << d << std::endl;
}
