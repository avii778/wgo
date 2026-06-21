#include <atomic>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <vector>

using namespace std;

template <typename T> class spsc {

private:
  vector<T> buffer;
  atomic<int> write_idx;
  atomic<int> read_idx;
  int capacity;

public:
  spsc(int size) : capacity(size + 1), buffer(size + 1) {
    // adding +1 to the cap and buffer, bc we need an
    // empty slot to make sure that write idx doesnt
    // rewrite over stuff that hasnt been read yet
    // dont really undersstand how the memory_order_x flags work, just been
    // using acquire when i want to load it safely and release to tell everyone
    // im done
    write_idx.store(0, memory_order_release);
    read_idx.store(0, memory_order_release);
  }
  bool write(T x) {
    if ((write_idx.load(memory_order_acquire) + 1) % capacity ==
        read_idx.load(memory_order_acquire)) {
      // full
      return false;
      // rn assuming we just fail fast rather than spin
    }
    buffer[write_idx.load(memory_order_acquire)] = x;
    write_idx.store((write_idx.load(memory_order_acquire) + 1) % capacity,
                    memory_order_release);
    return true;
  }
  optional<T> read() {
    if (write_idx.load(memory_order_acquire) ==
        read_idx.load(memory_order_acquire)) {
      // empty
      return nullopt;

      // rn assuming we just fail fast rather than spin
    }
    T value = buffer[read_idx.load(memory_order_acquire)];
    read_idx.store((read_idx.load(memory_order_acquire) + 1) % capacity,
                   memory_order_release);
    return value;
  }
};

int main() {
  // testing it
  spsc<string> test(3);
  thread sender([&] {
    test.write("mario");
    test.write("jump");
    test.write("goomba");
    test.write("bowser");
  });
  sender.join();

  thread receiver([&] {
    auto msg1 = test.read();

    if (msg1) {
      cout << *msg1 << endl;
    } else {
      cout << "empty" << endl;
    }
    auto msg2 = test.read();

    if (msg2) {
      cout << *msg2 << endl;
    } else {
      cout << "empty" << endl;
    }
    auto msg3 = test.read();

    if (msg3) {
      cout << *msg3 << endl;
    } else {
      cout << "empty" << endl;
    }
  });

  receiver.join();
  thread reciever2([&] {
    auto msg = test.read();

    if (msg) {
      cout << *msg << endl;
    } else {
      cout << "empty" << endl;
    }
  });
  reciever2.join();
}