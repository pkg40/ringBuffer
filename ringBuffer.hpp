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
#include <Arduino.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <new>
#include <utility>
#include <stdexcept>
#include <cstring>

template <typename T, size_t sz>
class ringBuffer {
    static_assert(sz > 0, "ringBuffer size must be greater than 0");
    static_assert((sz & (sz - 1)) == 0, "ringBuffer size must be a power of two");

    alignas(T) uint8_t _buffer[sz][sizeof(T)];
    uint8_t _rdPtr = 0;
    uint8_t _wrPtr = 0;
    uint8_t _count = 0;

public:
    ringBuffer() = default;
    ~ringBuffer();

    bool push(const T& data, bool verbose = false);
    bool pop(T& out);
    bool peek(T& out) const;
    void clear();

    bool isEmpty() const { return _count == 0; }
    bool isFull() const { return _count == sz; }
    size_t size() const { return _count; }
    size_t capacity() const { return sz; }
};

#include <ringBuffer.ipp>