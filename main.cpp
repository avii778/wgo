#include <atomic>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

template <typename T> class spsc {

private:
  T value;
  atomic<int> flag;
  // flag represents whether the value is full or not

public:
  spsc() { flag.store(0, memory_order_release); }
  void send(T x) {
    while (flag.load(memory_order_acquire) != 0) {
      continue;
    }
    value = x;
    flag.store(1, memory_order_release);
  }
  T receive() {
    while (flag.load(memory_order_acquire) == 0) {
      continue;
    }
    flag.store(0, memory_order_release);
    return value;
  }
};

int main() {
  // testing it
  spsc<string> test;
  thread sender([&] {
    test.send("mario");
    test.send("jump");
    test.send("goomba");
  });
  thread receiver([&] {
    cout << test.receive() << endl;
    cout << test.receive() << endl;
    cout << test.receive() << endl;
  });
  sender.join();
  receiver.join();
}