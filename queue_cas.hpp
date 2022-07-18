#ifndef __QUEUE_CAS_HPP_
#define __QUEUE_CAS_HPP_
#include <cstdio>
#include "queue.hpp"

template<typename T, int N>
class queue_cas {
public:
    queue_cas() {
        f = w = r = &queue.front();
        c.set(NULL);
    }
    void write(T& value, bool complate) {
        queue.back() = value;
        queue.push();

        if (complate) {
            f = &queue.back();
            //printf("write f:%p \n", f);
        }
    }

    bool flush() {
        if (w == f) return true;
        if (c.cas(w, f) != w) { //说明队列空了，写入新的值之后返回false便于后续给读线程发送通知
            c.set(f);
            w = f;
            return false;  //当队列为空的时候返回false 便于后续通知挂起的读线程唤醒
        }
        w = f;
        return true;
    }

    bool check_read() {
        if (&queue.front() != r && r) return true;

        r = c.cas(&queue.front(), NULL);
        if (r == &queue.front() || r == NULL) {  //r依然等于w 说明队列为NULL 返回false
            return false;
        }
        return true;
    }

    bool read(T* value) {
        if (!check_read()) {
            return false;
        }
        *value = queue.front();
        queue.pop();
        return true;
    }
private:
    T* f;    //用于flush
    T* w;    //代表写入的上界
    T* r;   //代表能读到哪里

    queue_chunk<T, N> queue;
    atomic_ptr_t<T> c;

    queue_cas(const queue_cas&);
    const queue_cas & operator=(const queue_cas&);
};

#endif

