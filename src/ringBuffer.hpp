// ringBuffer.hpp - Basic static ring buffer

#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#    include <Arduino.h>
#else
#    include <WProgram.h>
#endif

// #include <TRACE.h>

//*------------------------------------------------------
//* Simple Ring Buffer class
//*------------------------------------------------------
template<typename T, size_t sz> class ringBuffer {
    static_assert(sz > 0, "RingBuf with size 0 are forbidden");
    static_assert(sz <= __UINT8_MAX__,
                  "RingBuf with size greater than 2^8 (0xff) are forbidden");

// Detect if we can use power of two mod optimization
#if (sz == 0x1 || sz == 0x2 || sz == 0x4 || sz == 0x8 || sz == 0x10 || \
     sz == 0x20 || sz == 0x40 || sz == 0x80)
    static constexpr bool POWoTWO = true;
#else
    static constexpr bool POWoTWO = false;
#endif

private:
//    T _buffer[sz];
uint8_t _buffer[sz][sizeof(T)];
uint8_t _rdPtr;
uint8_t _wrPtr;
uint8_t _maxSize;

public:
ringBuffer();
bool push(const T& data, bool = false);  // Add overload for queueManager compatibility
bool pop(T& data, bool = false);
bool peek(T& data, bool = false);
bool isEmpty() const;  // Add const version
bool isFull() const;  // Add const version for queueManager

uint8_t getCapacity() const { return sz; }
uint8_t getUsed() const { return _maxSize; }
uint8_t getFree() const { return sz - _maxSize; }
size_t size() const { return _maxSize; }  // Add size method
size_t capacity() const { return sz; }  // Add capacity method

void clear() ;
};

//* Start Simple Ring Buffer
template<typename T, size_t sz> ringBuffer<T, sz>::ringBuffer() {
    _rdPtr = 0;
    _wrPtr = 0;
    _maxSize = 0;
}

template<typename T, size_t sz> bool ringBuffer<T, sz>::isFull() const {
    return (_maxSize == sz);
}

template<typename T, size_t sz> bool ringBuffer<T, sz>::isEmpty() const {
    return (_maxSize == 0);
}

template<typename T, size_t sz>
IRAM_ATTR bool ringBuffer<T, sz>::push(const T& data, bool verbose) {
    //    bool verbose = false;
    if (isFull()) {
        if (verbose) Serial.println("Queue is Full");
        return false;
    } else {
        if (verbose) {
            Serial.printf("line= -- %d\n", verbose);
            //        Serial.print("push data = ");
            //        Serial.println(data);
        }
        //      _buffer[_wrPtr]=*data;
        memcpy(_buffer[_wrPtr], &data, sizeof _buffer[_wrPtr]);
        _wrPtr = (_wrPtr + 1) % sz;
        _maxSize++;
        return (true);
    }
};

template<typename T, size_t sz> bool ringBuffer<T, sz>::pop(T& data, bool verbose) {
    if (isEmpty()) {
        if (verbose) Serial.println("Queue is empty");
        return false;
    } else {
        memcpy(&data, _buffer[_rdPtr], sizeof data);
#if (sz & sz - 1 == 0)
        _rdPtr = (_rdPtr + 1) & (sz - 1);
        else _rdPtr = (_rdPtr + 1) % sz;
#endif
        _maxSize--;
        return (true);
    }
};
//* End Simple Ring Buffer

template<typename T, size_t sz> bool ringBuffer<T, sz>::peek(T& data, bool verbose) {
    if (isEmpty()) {
        if (verbose) Serial.println("Queue is empty");
        return false;
    } else {
        memcpy(&data, _buffer[_rdPtr], sizeof data);
        if (verbose) {
            Serial.print("peek data = ");
            Serial.println(data);
        }
        return (true);
    }
};

template<typename T, size_t sz> void ringBuffer<T, sz>::clear() {
    _rdPtr = 0;
    _wrPtr = 0;
    _maxSize = 0;
}