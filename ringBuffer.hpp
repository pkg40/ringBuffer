/*
 * MIT License
 *
 * Copyright (c) 2025 Peter K Green (pkg40)
 * Email: pkg40@yahoo.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
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
    bool push(const T&, int=0);
    bool pop(T& data);
    bool isEmpty();
    bool isFull();
};

//* Start Simple Ring Buffer
template <typename T, size_t sz> 
ringBuffer<T,sz>::ringBuffer(){
    _rdPtr=0;
    _wrPtr=0;
    _maxSize=0;
  }
  
template <typename T, size_t sz>
bool ringBuffer<T,sz>::isFull() { return(_maxSize == sz); }
  
template <typename T, size_t sz> bool ringBuffer<T, sz>::isEmpty() { return(_maxSize == 0); }
  
template <typename T, size_t sz> IRAM_ATTR  bool  ringBuffer<T, sz>::push(const T& data, int verbose){
//    bool verbose = false;
    if (isFull()) {if (verbose) Serial.println ("Queue is Full"); return false;}
    else {
      if (verbose) {
        Serial.printf ("line= -- %d\n", verbose);
      }
      memcpy(&_buffer[_wrPtr], &data, sizeof _buffer[_wrPtr]);
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