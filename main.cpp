#include <cstdio>
#include <cstdlib>
#include <thread>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include "queue_cas.hpp"
#include "queue.hpp"
using namespace std;
#define LOOP 100000
queue_cas<int, 1> queue;
mutex m;
condition_variable cond;

void thread_write() {
    for (int i = 0; i < LOOP; i++) {
        queue.write(i, true);
        if (!queue.flush()) {
            lock_guard<mutex> lock(m);
            cond.notify_one();
        }
    }
}

void thread_read() {
    int i = 0;
    while (true) {
        int value;
        while (!queue.read(&value)) {
            unique_lock<mutex> lock(m);
            cond.wait(lock);
        }
        if (i != value) {
            printf("read is error i:%d value:%d \n", i, value);
            fflush(stdout);
            break;
        }
        if (i == value) {
            printf("i=%d is ok\n", i);
            fflush(stdout);
        }
        i++;
        if (i == LOOP) break;
    }
}

int main() {
    thread t1(thread_read);
    thread t2(thread_write);

    t1.join();
    t2.join();
    return 0;
}
