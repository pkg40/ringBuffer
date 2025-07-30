/**
 * @file ringBuffer.ipp
 * @brief Implementation of ringBuffer methods.
 *
 * NOTE: T must be trivially copyable and destructible, or the user must ensure correct construction and destruction.
 * This ringBuffer uses placement new and manual destructor calls. Using non-trivial types may cause undefined behavior if not handled carefully.
 */

template <typename T, size_t sz>
/**
 * @brief Destructor. Clears the buffer and destroys all elements.
 */
ringBuffer<T, sz>::~ringBuffer() {
    clear();
}

template <typename T, size_t sz>
/**
 * @brief Push an element to the buffer.
 * @param data Element to push.
 * @param verbose If true, prints error if full.
 * @return true if successful, false if full.
 */
bool ringBuffer<T, sz>::push(const T& data, bool verbose) {
    if (isFull()) {
        if (verbose) {
            Serial.println("[ringBuffer] Error: ringBuffer is full");
        }
        return false;
    }

    void* slot = static_cast<void*>(&_buffer[_wrPtr]);
    new (slot) T(data);  // Placement new
    _wrPtr = (_wrPtr + 1) & (sz - 1);  // Wrap around
    ++_count;
    return true;
}

template <typename T, size_t sz>
/**
 * @brief Pop an element from the buffer.
 * @param out Output reference for popped element.
 * @return true if successful, false if empty.
 */
bool ringBuffer<T, sz>::pop(T& out) {
    if (isEmpty()) {
        Serial.println("[ringBuffer] Warning: pop called on empty buffer");
        return false;
    }

    T* obj = reinterpret_cast<T*>(&_buffer[_rdPtr]);
    out = std::move(*obj);
    obj->~T();  // Explicitly call destructor
    _rdPtr = (_rdPtr + 1) & (sz - 1);  // Wrap around
    --_count;
    return true;
}

template <typename T, size_t sz>
/**
 * @brief Peek at the next element to be popped.
 * @param out Output reference for peeked element.
 * @return true if successful, false if empty.
 */
bool ringBuffer<T, sz>::peek(T& out) const {
    if (isEmpty()) {
        Serial.println("[ringBuffer] Warning: peek called on empty buffer");
        return false;
    }

    const T* obj = reinterpret_cast<const T*>(&_buffer[_rdPtr]);
    out = *obj;
    return true;
}

template <typename T, size_t sz>
/**
 * @brief Clear the buffer, destroying all elements.
 */
void ringBuffer<T, sz>::clear() {
    while (!isEmpty()) {
        T* obj = reinterpret_cast<T*>(&_buffer[_rdPtr]);
        obj->~T();  // Explicitly call destructor
        _rdPtr = (_rdPtr + 1) & (sz - 1);  // Wrap around
        --_count;
    }
}
//void queueManager<packetType, bufferSize>::setProcessor(F&& f) {
//    _processor = std::forward<F>(f);
//}