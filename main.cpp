#include <atomic>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

template <typename T> class spsc {

private:
  vector<T> buffer;
  atomic<int> write_idx;
  atomic<int> read_idx;

public:
  spsc(int size) : buffer(size) {
    // dont really undersstand how the memory_order_x flags work, just been
    // using acquire when i want to load it safely and release to tell everyone
    // im done
    write_idx.store(0, memory_order_release);
    read_idx.store(0, memory_order_release);
  }
  bool write(T x) {
    if ((write_idx.load(memory_order_acquire) + 1) % size ==
        read_idx.load(memory_order_acquire)) {
      // full
      return false;
      // rn assuming we just fail fast rather than spin
    }
    buffer[write_idx.load(memory_order_acquire)] = x;
    write_idx.store((write_idx.load(memory_order_acquire) + 1) % n,
                    memory_order_release);
    return true;
  }
  T read() {
    if (write_idx.load(memory_order_acquire) ==
        read_idx.load(memory_order_acquire)) {
      // empty
      return false;

      // rn assuming we just fail fast rather than spin
    }

    read_idx.store(read_idx.load((memory_order_acquire) + 1) % n,
                   memory_order_release);
    return value;
  }
};

int main() {
  // testing it
  //   spsc<string> test;
  //   thread sender([&] {
  //     test.send("mario");
  //     test.send("jump");
  //     test.send("goomba");
  //   });
  //   thread receiver([&] {
  //     cout << test.receive() << endl;
  //     cout << test.receive() << endl;
  //     cout << test.receive() << endl;
  //   });
  //   sender.join();
  //   receiver.join();
}