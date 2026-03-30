/* ************************************************************************************
 * ringBuffer.hpp - Thread-Safe Static Ring Buffer for Embedded Systems
 ************************************************************************************
 *
 * OVERVIEW (what this is for)
 * ===========================
 * A fixed-capacity FIFO queue backed by a single contiguous array. Memory is
 * reserved at compile time (no heap). Public operations take bounded time O(1).
 * Use it anywhere you need to hand off small records between a producer and a
 * consumer without malloc — UART/ESP-NOW/Wi-Fi callbacks into the main loop,
 * sensor sampling, command queues, etc.
 *
 * CONTRACT (what callers must respect)
 * ====================================
 * - Element type T must be trivially copyable (POD-like). Storage is raw bytes
 *   copied with memcpy; destructors and copy constructors on T are not run.
 *   If you need non-trivial types, store pointers or IDs and manage lifetime
 *   outside the buffer.
 * - Capacity sz must be in 1 .. 255. Indices are uint8_t by design to keep the
 *   object small on 8/32-bit MCUs.
 * - Full behavior: push() on a full buffer returns false and does not overwrite
 *   the oldest element (no automatic drop-old). Callers that need overwrite or
 *   drop-new semantics must implement that on top.
 *
 * THREAD SAFETY (what “safe” means here)
 * ======================================
 * Each public mutator/query takes a critical section so one operation completes
 * before another starts. That prevents torn reads/writes of _rdPtr/_wrPtr/_maxSize.
 *
 *   ESP32:    portMUX spinlock (works across tasks and with ISRs on that core)
 *   ESP8266:  noInterrupts / interrupts (single-core model)
 *   Default:  same as ESP8266 when not on ESP32
 *
 * This is not a full MP-safe queue across different cores without additional
 * barriers if both cores accessed the same instance without the spinlock path.
 * Typical use here is single logical producer/consumer with ISR or callback on
 * one side and loop() on the other — which matches the locking model.
 *
 * Define RINGBUFFER_DISABLE_THREAD_SAFETY to 1 before including this header if
 * the instance is touched from a single thread only (saves a few cycles per op).
 *
 * PERFORMANCE NOTES
 * =================
 * If sz is a power of two (2,4,8,...,128), index wrap uses a bitmask instead of
 * `% sz`. Non-power sizes still work; they use integer modulus.
 *
 * push() is marked IRAM_ATTR on ESP32 so it may be called from an ISR that runs
 * from IRAM. pop/peek are not marked IRAM_ATTR by default to avoid growing IRAM
 * footprint; if you call them from ISR, consider mirroring that attribute.
 *
 * EMBEDDED COMPATIBILITY (firmware-oriented checklist)
 * ====================================================
 * Use this section when reviewing or integrating the buffer on target hardware.
 *
 * Call context (ISR vs task / loop)
 * ---------------------------------
 * - Default design: producer may be ISR/callback, consumer in loop or task.
 * - Never use verbose=true from an ISR: Serial is slow, may block, and is not
 *   IRQ-safe in the usual Arduino sense.
 * - On ESP8266 and generic Arduino paths, operations use noInterrupts(): keep
 *   sizeof(T) small so other IRQs are not stalled long (watchdog-sensitive).
 *
 * Worst-case critical section / latency
 * -------------------------------------
 * - Bounded work per call: compare/fullness check, index update, two memcpy
 *   paths of sizeof(T) total (push: in from caller ref, out to slot; pop: reverse).
 * - Larger T increases interrupt deferral (8266) and spinlock hold (ESP32).
 *
 * IRAM / flash (ESP32)
 * --------------------
 * - Only push() is IRAM_ATTR so flash-cache-disabled ISRs can still enqueue.
 * - pop(), peek(), clear() live in flash by default to conserve IRAM; tag them
 *   IRAM_ATTR only if you must dequeue from an ISR under the same constraints.
 *
 * RAM and stack
 * -------------
 * - Queue storage is static (BSS); no malloc/free in push/pop/peek/clear.
 * - pop(T&) and peek(T&) require a full T on the caller stack; tiny embedded
 *   stacks + large structs need explicit attention.
 *
 * Toolchain / C++ subset
 * ----------------------
 * - Uses std::aligned_storage and std::is_trivially_copyable (C++11 or later).
 * - On a bare-metal port without a full STL, reimplement those pieces or avoid
 *   this header until the environment matches std::memcpy + alignment rules.
 *
 * Determinism
 * -----------
 * - No exceptions from this template; operations are O(1) with fixed upper time
 *   tied to sizeof(T), not queue depth (depth affects full/empty only).
 *
 * USAGE EXAMPLE
 * =============
 *
 *   ringBuffer<float, 8> sensorBuffer;
 *
 *   void onIsr(float v) {
 *     (void)sensorBuffer.push(v);  // false if full — handle policy externally
 *   }
 *
 *   void loop() {
 *     float v;
 *     while (sensorBuffer.pop(v)) { }  // drain queue
 *   }
 *
 * MEMORY LAYOUT (approximate)
 * ===========================
 *   aligned_storage<sz> cells of sizeof(T) each
 *   + read/write indices + occupancy count (uint8_t each)
 *   + ESP32: one portMUX_TYPE
 *
 ************************************************************************************
 * Copyright (c)       2019-2025 Peter K Green            - pkg40@yahoo.com
 ************************************************************************************
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ************************************************************************************/

#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
    #include <Arduino.h>
#else
    #include <WProgram.h>
#endif

#include <type_traits>

#ifndef RINGBUFFER_DISABLE_THREAD_SAFETY
    #define RINGBUFFER_DISABLE_THREAD_SAFETY 0
#endif

#if RINGBUFFER_DISABLE_THREAD_SAFETY
    #define RINGBUF_ENTER_CRITICAL() ((void)0)
    #define RINGBUF_EXIT_CRITICAL() ((void)0)
    #define RINGBUF_DECLARE_LOCK()
#elif defined(ESP32)
    /* Spinlock: serializes with other tasks/ISRs using the same lock object. */
    #define RINGBUF_DECLARE_LOCK() mutable portMUX_TYPE _mux = portMUX_INITIALIZER_UNLOCKED
    #define RINGBUF_ENTER_CRITICAL() portENTER_CRITICAL(&_mux)
    #define RINGBUF_EXIT_CRITICAL() portEXIT_CRITICAL(&_mux)
#elif defined(ESP8266)
    #define RINGBUF_DECLARE_LOCK()
    #define RINGBUF_ENTER_CRITICAL() noInterrupts()
    #define RINGBUF_EXIT_CRITICAL() interrupts()
#else
    #define RINGBUF_DECLARE_LOCK()
    #define RINGBUF_ENTER_CRITICAL() noInterrupts()
    #define RINGBUF_EXIT_CRITICAL() interrupts()
#endif

/**
 * RAII wrapper so every exit path releases the lock. Embedded note: scope should
 * stay short — defer Serial, allocation, or slow work to outside the guard.
 * ESP32: per-instance spinlock. Other platforms: global interrupt masking.
 */
class ringBufCriticalGuard {
#if !RINGBUFFER_DISABLE_THREAD_SAFETY && defined(ESP32)
        portMUX_TYPE* _mux;

    public:
        explicit ringBufCriticalGuard(portMUX_TYPE* mux) : _mux(mux) {
            portENTER_CRITICAL(_mux);
        }
        ~ringBufCriticalGuard() {
            portEXIT_CRITICAL(_mux);
        }
#elif !RINGBUFFER_DISABLE_THREAD_SAFETY
    public:
        explicit ringBufCriticalGuard(void* = nullptr) {
            noInterrupts();
        }
        ~ringBufCriticalGuard() {
            interrupts();
        }
#else
    public:
        explicit ringBufCriticalGuard(void* = nullptr) {
        }
        ~ringBufCriticalGuard() {
        }
#endif
        ringBufCriticalGuard(const ringBufCriticalGuard&) = delete;
        ringBufCriticalGuard& operator=(const ringBufCriticalGuard&) = delete;
};

/**
 * Fixed FIFO ring buffer: one slot per element, memcpy in/out, critical sections
 * around state. See file banner "EMBEDDED COMPATIBILITY" for ISR, IRAM, stack,
 * and latency expectations on resource-constrained targets.
 */
template<typename T, size_t sz>
class ringBuffer {
        static_assert(sz > 0, "ringBuffer size must be non-zero");
        static_assert(sz <= __UINT8_MAX__, "ringBuffer size must fit uint8_t indices (max 255)");
        static_assert(std::is_trivially_copyable<T>::value,
                      "ringBuffer<T>: T must be trivially copyable (safe for memcpy)");

    private:
#if (sz == 0x1 || sz == 0x2 || sz == 0x4 || sz == 0x8 || sz == 0x10 || \
     sz == 0x20 || sz == 0x40 || sz == 0x80)
        static constexpr bool _powerOfTwoCapacity = true;
#else
        static constexpr bool _powerOfTwoCapacity = false;
#endif

        /**
         * Each slot is aligned for T so memcpy to/from the slot never hits
         * misaligned hardware access (unlike uint8_t[sizeof(T)] alone).
         */
        typename std::aligned_storage<sizeof(T), alignof(T)>::type _cells[sz];

        /** Index of the slot that pop/peek read next. */
        volatile uint8_t _rdPtr;
        /** Index of the slot push writes next. */
        volatile uint8_t _wrPtr;
        /** Number of elements currently stored (0 .. sz). When 0, empty; when sz, full. */
        volatile uint8_t _maxSize;

        RINGBUF_DECLARE_LOCK();

        uint8_t _nextIndex(uint8_t index) const {
            if (_powerOfTwoCapacity) {
                return static_cast<uint8_t>((index + 1) & (sz - 1));
            }
            return static_cast<uint8_t>((index + 1) % sz);
        }

        void* _cellPtr(uint8_t slot) {
            return &_cells[slot];
        }

        const void* _cellPtr(uint8_t slot) const {
            return &_cells[slot];
        }

    public:
        ringBuffer();

        /**
         * Append one element at the tail if space remains.
         * ESP32: IRAM_ATTR — suitable for IRQ when flash cache may be off.
         * @param data value to copy into the buffer (memcpy)
         * @param verbose if true, logs via Serial when full — not for ISR/task with timing constraints
         * @return false if full; true if stored
         */
        bool push(const T& data, bool verbose = false);

        /**
         * Remove one element from the head into data.
         * Embedded: places sizeof(T) on caller stack; defaults to flash code on ESP32 (not IRAM_ATTR).
         * @param verbose if true, logs when empty — not for ISR
         */
        bool pop(T& data, bool verbose = false);

        /**
         * Copy head element into data without removing it.
         * Embedded: same stack and IRAM considerations as pop().
         * verbose logs a generic line only (not the value of T).
         */
        bool peek(T& data, bool verbose = false);

        bool isEmpty() const;
        bool isFull() const;

        /** Fixed capacity (template parameter sz). */
        uint8_t getCapacity() const {
            return static_cast<uint8_t>(sz);
        }

        /**
         * Current occupancy under lock. Prefer this or size() over open-coded
         * checks when the count matters across threads.
         */
        uint8_t getUsed() const {
#if !RINGBUFFER_DISABLE_THREAD_SAFETY && defined(ESP32)
            ringBufCriticalGuard guard(const_cast<portMUX_TYPE*>(&_mux));
#else
            ringBufCriticalGuard guard;
#endif
            return _maxSize;
        }

        uint8_t getFree() const {
            return static_cast<uint8_t>(sz - getUsed());
        }

        /** Same as getUsed() — name matches common STL-style queues. */
        size_t size() const {
            return getUsed();
        }

        /** Same as getCapacity() — STL-style alias. */
        size_t capacity() const {
            return sz;
        }

        /**
         * Drop all elements; indices reset. Not IRAM_ATTR on ESP32 by default.
         * Serialize with the same rules as other methods if producer is in ISR.
         */
        void clear();
};

template<typename T, size_t sz>
ringBuffer<T, sz>::ringBuffer() {
    _rdPtr = 0;
    _wrPtr = 0;
    _maxSize = 0;
}

template<typename T, size_t sz>
bool ringBuffer<T, sz>::isFull() const {
#if !RINGBUFFER_DISABLE_THREAD_SAFETY && defined(ESP32)
    ringBufCriticalGuard guard(const_cast<portMUX_TYPE*>(&_mux));
#else
    ringBufCriticalGuard guard;
#endif
    return _maxSize == sz;
}

template<typename T, size_t sz>
bool ringBuffer<T, sz>::isEmpty() const {
#if !RINGBUFFER_DISABLE_THREAD_SAFETY && defined(ESP32)
    ringBufCriticalGuard guard(const_cast<portMUX_TYPE*>(&_mux));
#else
    ringBufCriticalGuard guard;
#endif
    return _maxSize == 0;
}

template<typename T, size_t sz>
IRAM_ATTR bool ringBuffer<T, sz>::push(const T& data, bool verbose) {
#if !RINGBUFFER_DISABLE_THREAD_SAFETY && defined(ESP32)
    ringBufCriticalGuard guard(&_mux);
#else
    ringBufCriticalGuard guard;
#endif

    if (_maxSize == sz) {
        if (verbose) {
            Serial.println(F("ringBuffer: push rejected (full)"));
        }
        return false;
    }

    memcpy(_cellPtr(_wrPtr), &data, sizeof(T));
    _wrPtr = _nextIndex(_wrPtr);
    _maxSize++;
    return true;
}

template<typename T, size_t sz>
bool ringBuffer<T, sz>::pop(T& data, bool verbose) {
#if !RINGBUFFER_DISABLE_THREAD_SAFETY && defined(ESP32)
    ringBufCriticalGuard guard(&_mux);
#else
    ringBufCriticalGuard guard;
#endif

    if (_maxSize == 0) {
        if (verbose) {
            Serial.println(F("ringBuffer: pop rejected (empty)"));
        }
        return false;
    }

    memcpy(&data, _cellPtr(_rdPtr), sizeof(T));
    _rdPtr = _nextIndex(_rdPtr);
    _maxSize--;
    return true;
}

template<typename T, size_t sz>
bool ringBuffer<T, sz>::peek(T& data, bool verbose) {
#if !RINGBUFFER_DISABLE_THREAD_SAFETY && defined(ESP32)
    ringBufCriticalGuard guard(&_mux);
#else
    ringBufCriticalGuard guard;
#endif

    if (_maxSize == 0) {
        if (verbose) {
            Serial.println(F("ringBuffer: peek rejected (empty)"));
        }
        return false;
    }

    memcpy(&data, _cellPtr(_rdPtr), sizeof(T));
    if (verbose) {
        Serial.println(F("ringBuffer: peek ok"));
    }
    return true;
}

template<typename T, size_t sz>
void ringBuffer<T, sz>::clear() {
#if !RINGBUFFER_DISABLE_THREAD_SAFETY && defined(ESP32)
    ringBufCriticalGuard guard(&_mux);
#else
    ringBufCriticalGuard guard;
#endif

    _rdPtr = 0;
    _wrPtr = 0;
    _maxSize = 0;
}
