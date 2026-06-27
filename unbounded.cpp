#include <atomic>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <vector>

using namespace std;

template <typename T> class spsc_node {

private:
  shared_ptr<spsc_node> pointer;
  T *addr;
  int capacity;

public:
  spsc_node(int size) : capacity(size + 1) {
    addr = malloc(capacity * sizeof(T));
    pointer = nullptr;
  }

  void set_neighbor(shared_ptr<spsc_node> neighbor) { pointer = neighbor; }
  spsc_node *get_neighbor() { return pointer; }
  T get_value(int idx) { return addr[idx]; }
  void set_value(int idx, T val) { addr[idx] = val; }
};

template <typename T> class unbounded_spsc {

private:
  atomic<int> head_idx;
  atomic<int> tail_idx;
  atomic<spsc_node<T>> head_node;
  atomic<spsc_node<T>> tail_node;
  int node_capacity;

public:
  unbounded_spsc(int size) : node_capacity(size) {

    head_idx.store(0, memory_order_release);
    tail_idx.store(0, memory_order_release);

    head_node = spsc_node(node_capacity);
    tail_node = head_node;
  }

  void write(T x) {
    if (tail_idx.load(memory_order_relaxed) == node_capacity) {
      spsc_node<T> new_node = spsc_node(node_capacity);
      tail_node.set_neighbor(new_node);
      tail_node.store(new_node, memory_order_relaxed);
      tail_idx.store(0, memory_order_relaxed);
    }
    tail_node.set_value(tail_idx.load(memory_order_relaxed), x);
    int next = tail_idx.load(memory_order_relaxed);
    tail_idx.store(next + 1, memory_order_release);
  }

  optional<T> read() {
    // if head and tail idx is same on same node

    int tail_i = tail_idx.load(memory_order_acquire);
    int head_i = head_idx.load(memory_order_relaxed);

    auto tail_n = tail_node.load(memory_order_acquire);
    auto head_n = tail_node.load(memory_order_relaxed);

    if (tail_i = head_i && tail_n == head_n) {
      return nullopt;
    }

    T val = tail_n.get_value(tail_i);

    head_idx.store(head_i + 1, memory_order_relaxed);

    if (head_i == node_capacity) {
      while (head_n.get_neighbor() == nullptr) {
        continue;
      }
      head_node.store(head_n.get_neighbor(), memory_order_release);
    }

    return val;
  }
};

int main() {
  // testing it
  unbounded_spsc<string> test(3);
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
