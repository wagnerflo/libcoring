#include <covent.hh>
#include <iostream>
#include <chrono>
#include <string>

using namespace std::chrono_literals;

namespace covent {
  class task_group {
    public:
      
  };
}

covent::task<> sleeper(int n) {
  std::cout << ">>> sleeper n=" << n << " starting" << std::endl;
  co_await (3s - std::chrono::seconds { n });
  std::cout << ">>> sleeper n=" << n << " ending" << std::endl;
}

int main() {
  auto i = covent::run([]() -> covent::task<int> {
    std::cout << ">>> main starting" << std::endl;

    auto t1 = sleeper(1);
    auto t2 = sleeper(2);

    // covent::task_group { t1, t2 };

    std::cout << ">>> main ending" << std::endl;
    co_return 42;
  });
  std::cout << "i=" << 42 << std::endl;
  return 0;
}
