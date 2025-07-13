// NOTE: T must be trivially copyable and destructible, or the user must ensure correct construction and destruction.
// This ringBuffer uses placement new and manual destructor calls. Using non-trivial types may cause undefined behavior if not handled carefully.
template <typename T, size_t sz>
ringBuffer<T, sz>::~ringBuffer() {
    clear();
}

template <typename T, size_t sz>
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
bool ringBuffer<T, sz>::pop(T& out) {
    if (isEmpty()) {
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
bool ringBuffer<T, sz>::peek(T& out) const {
    if (isEmpty()) {
        return false;
    }

    const T* obj = reinterpret_cast<const T*>(&_buffer[_rdPtr]);
    out = *obj;
    return true;
}

template <typename T, size_t sz>
void ringBuffer<T, sz>::clear() {
    while (!isEmpty()) {
        T* obj = reinterpret_cast<T*>(&_buffer[_rdPtr]);
        obj->~T();  // Explicitly call destructor
        _rdPtr = (_rdPtr + 1) & (sz - 1);  // Wrap around
        --_count;
    }
}

//template<typename packetType, size_t bufferSize> 
//template <typename F>
//void queueManager<packetType, bufferSize>::setProcessor(F&& f) {
//    _processor = std::forward<F>(f);
//}