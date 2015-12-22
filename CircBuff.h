#include <stdexcept>

/*
   T must implement operator=, copy ctor
*/

template<typename T> class CircBuf
{
    // don't use default ctor
    CircBuf();

    const int size;
    T *data;
    int front;
    int count;
public:
    CircBuf(int);
    ~CircBuf();

    bool empty()
    {
        return count == 0;
    }
    bool full()
    {
        return count == size;
    }
    bool add(const T&);
    bool remove(T*);
};

template<typename T> CircBuf<T>::CircBuf(int sz): size(sz)
{
    if (sz==0) throw std::invalid_argument("size cannot be zero");
    data = new T[sz];
    front = 0;
    count = 0;
}
template<typename T> CircBuf<T>::~CircBuf()
{
    delete data;
}

// returns true if add was successful, false if the buffer is already full
template<typename T> bool CircBuf<T>::add(const T &t)
{
    if ( full() )
    {
        return false;
    }
    else
    {
        // find index where insert will occur
        int end = (front + count) % size;
        data[end] = t;
        count++;
        return true;
    }
}

// returns true if there is something to remove, false otherwise
template<typename T> bool CircBuf<T>::remove(T *t)
{
    if ( empty() )
    {
        return false;
    }
    else
    {
        *t = data[front];

        front = front == size ? 0 : front + 1;
        count--;
        return true;
    }
}
