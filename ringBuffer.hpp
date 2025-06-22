#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
	#include <Arduino.h>
#else
	#include <WProgram.h>
#endif

//#include <TRACE.h>

//*------------------------------------------------------
//* Simple Ring Buffer class 
//*------------------------------------------------------
template <typename T, size_t sz>
class ringBuffer {
    static_assert(sz > 0, "RingBuf with size 0 are forbidden");
    static_assert(sz <= __UINT8_MAX__, "RingBuf with size greater than 2^8 (0xff) are forbidden");
  private:
//    T _buffer[sz];
    uint8_t _buffer[sz][sizeof(T)];
    uint8_t _rdPtr;
    uint8_t _wrPtr;
    uint8_t _maxSize;
  public:
    ringBuffer();
    bool push(T*, int=0);
    bool pop(T &data);
    bool isEmpty();
    bool isFull();
};

//* Start Simple Ring Buffer
template <typename T, size_t sz> ringBuffer<T,sz>::ringBuffer(){
    _rdPtr=0;
    _wrPtr=0;
    _maxSize=0;
  }
  
template <typename T, size_t sz> bool ringBuffer<T,sz>::isFull() { return(_maxSize == sz); }
  
template <typename T, size_t sz> bool ringBuffer<T, sz>::isEmpty() { return(_maxSize == 0); }
  
template <typename T, size_t sz> IRAM_ATTR  bool  ringBuffer<T, sz>::push(T* data, int verbose){
//    bool verbose = false;
    if (isFull()) {if (verbose) Serial.println ("Queue is Full"); return false;}
    else {
      if (verbose) {
        Serial.printf ("line= -- %d\n", verbose);
//        Serial.print("push data = "); 
//        Serial.println(data); 
      }
//      _buffer[_wrPtr]=*data;
      memcpy(_buffer[_wrPtr], data, sizeof _buffer[_wrPtr]);
      _wrPtr = (_wrPtr + 1)%sz;
      _maxSize++;
      return(true);
    }
};
  
template <typename T, size_t sz> bool  ringBuffer<T, sz>::pop(T& data){
    bool verbose = false;
    if (isEmpty()) { 
      if (verbose) Serial.println ("Queue is empty"); 
      return false;
    }
    else {
      memcpy(&data, _buffer[_rdPtr], sizeof data);
//      data = _buffer[_rdPtr];
      _rdPtr = (_rdPtr + 1)%sz;
      _maxSize--;
      if (verbose) {
        Serial.print("pop data = "); 
//        Serial.println(data); 
      }
      return(true);
    }
};
//* End Simple Ring Buffer