#ifndef __QUEUE_HPP_
#define __QUEUE_HPP_
#include <cstdlib>
#include <iostream>
#include "atomic_ptr.hpp"
using namespace std;

template<typename T, int N>
class queue_chunk {
public:
    queue_chunk() {
        begin_chunk = back_chunk = end_chunk = new chunk_t;
        if (N == 1) {
            begin_chunk->next = new chunk_t;
            end_chunk = begin_chunk->next;
            end_chunk->next = NULL;
        }
        begin_pos = 0;
        back_pos =  0;
        end_pos = (back_pos + 1) % N;
        spare_chunk.set(NULL);
    }

    inline T& back() {
        return back_chunk->values[back_pos];
    }

    inline void push() {
        back_chunk = end_chunk;
        back_pos = end_pos;
        end_pos++;
        if (end_pos < N) return;
        chunk_t *spare = spare_chunk.xchg(NULL);
        if (spare) {
            end_chunk->next = spare;
            spare->next = NULL;
        } else {
            chunk_t * temp = new chunk_t;
            end_chunk->next = temp;
            temp->next = NULL;
        }
        end_chunk = end_chunk->next;
        end_pos = 0;
    }

    inline T& front() {
        return begin_chunk->values[begin_pos];
    }

    inline void pop() {
        begin_pos++;
        if (begin_pos < N) return;
        chunk_t* o = begin_chunk;
        begin_chunk = begin_chunk->next;
        o->next = NULL;
        begin_pos = 0;
        chunk_t *temp = spare_chunk.xchg(o);
        if (temp) delete temp;
    }

    ~queue_chunk() {
        for (chunk_t* c = begin_chunk; c != NULL; ) {
            chunk_t* del = c;
            c = c->next;
            delete del;
        }
        chunk_t* last_chunk = spare_chunk.xchg(NULL);
        if (last_chunk) delete last_chunk;
    }
private:
    struct chunk_t {
        T values[N];
        chunk_t* next;
    };
    chunk_t* begin_chunk;
    chunk_t* back_chunk;
    chunk_t* end_chunk;

    int begin_pos;
    int back_pos;
    int end_pos;
    atomic_ptr_t<chunk_t> spare_chunk;
};
#endif


