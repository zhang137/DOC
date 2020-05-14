#define POINTER_ALIGN(x,align) ((typeof(x))((((ptrdiff_t)(x))+((align)-1)) & (~((ptrdiff_t)(align)-1))))
#define ALIGN(x,align)   (((x)+((align)-1)) & (~((align)-1)))

#include <iostream>
#include <memory>
#include <cstring>
#include <utility>
#include <atomic>
#include <thread>
#include <climits>

using std::atomic;
using std::nullptr_t;


class SafeQueue
{
public:
    SafeQueue(int size = 0,int tlen = 0) {
        front_ = rear_ = len_ = 0;
        size_ = size * tlen;
        data = new nullptr_t[size_];
    }

    ~SafeQueue() {  delete [] data; }

    int put(nullptr_t * src, unsigned len);
    int get(nullptr_t * dest, unsigned len);

    inline bool isEmpty();
    inline bool isFull();
    inline unsigned int LeftSpace();
    inline unsigned int SpaceInUsed();

private:
    atomic<unsigned int> front_;
    atomic<unsigned int> rear_;
    atomic<unsigned int> len_;
    atomic<unsigned int> size_;
    nullptr_t *data;
};


bool SafeQueue::isFull()
{
    return (len_ >= size_);
}

bool SafeQueue::isEmpty()
{
    return (len_ == 0);
}

unsigned int SafeQueue::LeftSpace()
{
    return (size_ - SpaceInUsed());
}

unsigned int SafeQueue::SpaceInUsed()
{    
    //atomic_thread_fence(std::memory_order::memory_order_acquire);
    return rear_.load(std::memory_order_acquire) - front_.load(std::memory_order_acquire);
}


int SafeQueue::put(nullptr_t *src, unsigned len)
{
    if(len > LeftSpace()) return 0;
    
    unsigned char *src_ = reinterpret_cast<unsigned char*>(src);
    unsigned int rear = rear_.load(std::memory_order_relaxed) % size_;

    unsigned int slice = size_ - rear;
    unsigned int offset = (slice) > len ? len: slice;

    memcpy(data + rear, src_, offset);
    memcpy(data, src_ + offset, len - offset);
    
    rear_.fetch_add(len,std::memory_order_release);
    

    return 1;
}


int SafeQueue::get(nullptr_t * dest, unsigned len)
{
    if(len > SpaceInUsed()) return 0;
    
    unsigned char *dest_ = reinterpret_cast<unsigned char*>(dest);
    unsigned int front = front_.load(std::memory_order_relaxed) % size_;

    unsigned int slice = size_ - front;
    unsigned int offset = (slice) > len ? len: slice;

    memcpy(dest_, data + front, offset);
    memcpy(dest_ + offset, data, len - offset);

    front_.fetch_add(len, std::memory_order_release);

    return 1;
}

void write_queue(SafeQueue &q)
{
    int j = 0; 

    while(j < 10000) {
        int i = 0;
        while(i < 1000) {
            unsigned char buf[4];
            memcpy(buf, &i, 4);
            if(!q.put(reinterpret_cast<nullptr_t*>(buf), 4)) {
            //    std::cerr<< "write err:   "<<i<<std::endl;
                continue;
            }
            i++;
        }
        j++;
    }
}

void read_queue(SafeQueue &q)
{
    int j = 0; 
    while(j < 10000) {
        int i = 0;
        while(i < 1000) {
            unsigned char buf[4];
            if(!q.get(reinterpret_cast<nullptr_t*>(buf), 4)) {
                continue;
            }   
            int *pi = reinterpret_cast<int*>(buf);
           // std::cout<< "get one:   "<< *pi <<std::endl;
            i++;
        }
        j++;
    }
    
}


int main(int argc, char **argv)
{
    SafeQueue q(1000, sizeof(int));

    std::thread wq(write_queue, std::ref(q));
    std::thread rq(read_queue, std::ref(q));
    wq.join();
    rq.join();


    return 0;
}
