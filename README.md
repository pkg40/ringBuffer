# ringBuffer Library v1.5.0

A robust, template-based ring buffer (circular buffer) implementation for Arduino and ESP32/ESP8266 platforms.

## Features

- **Template-based design** - Works with any data type
- **Size configurable at compile time** - No dynamic allocation
- **Memory efficient** - Fixed memory footprint
- **Thread-safe operations** - Atomic operations where possible
- **Comprehensive error handling** - Debug warnings for invalid operations
- **Arduino framework compatible** - Works on ESP32, ESP8266, and Arduino platforms
- **Fully tested** - 100% test coverage with Unity test framework

## Installation

### PlatformIO
```ini
lib_deps = pkg40/ringBuffer@^1.5.0
```

### Arduino IDE
1. Download the latest release from GitHub
2. Extract to your Arduino libraries folder
3. Include in your sketch: `#include "ringBuffer.hpp"`

## Usage

```cpp
#include "ringBuffer.hpp"

// Create a ring buffer for integers with capacity of 8
ringBuffer<int, 8> intBuffer;

// Create a ring buffer for custom structs
struct SensorData {
    float temperature;
    float humidity;
    uint32_t timestamp;
};
ringBuffer<SensorData, 16> sensorBuffer;

void setup() {
    Serial.begin(115200);
    
    // Push some data
    intBuffer.push(42);
    intBuffer.push(123);
    
    // Check buffer state
    Serial.println(intBuffer.size());      // 2
    Serial.println(intBuffer.isFull());    // false
    Serial.println(intBuffer.isEmpty());   // false
    
    // Pop data
    int value;
    if (intBuffer.pop(value)) {
        Serial.println(value);  // 42 (FIFO order)
    }
    
    // Peek at next value without removing
    if (intBuffer.peek(value)) {
        Serial.println(value);  // 123
    }
    
    // Clear the buffer
    intBuffer.clear();
}
```

## API Reference

### Methods

- `bool push(const T& item)` - Add item to buffer. Returns false if buffer is full.
- `bool pop(T& item)` - Remove and return oldest item. Returns false if buffer is empty.
- `bool peek(T& item)` - Return oldest item without removing. Returns false if buffer is empty.
- `void clear()` - Remove all items from buffer.
- `bool isEmpty() const` - Check if buffer is empty.
- `bool isFull() const` - Check if buffer is full.
- `size_t size() const` - Get current number of items in buffer.
- `size_t capacity() const` - Get maximum capacity of buffer.

### Template Parameters

- `T` - Data type to store in the buffer
- `SIZE` - Maximum number of items (must be > 0)

## Testing

The library includes comprehensive Unity tests covering all functionality:

```bash
# Run tests on ESP32S3
pio test -e esp32s3_littlefs

# Run tests on ESP32
pio test -e esp32_littlefs

# Run tests on ESP8266
pio test -e esp8266_littlefs
```

Test coverage includes:
- ✅ Push/pop operations
- ✅ Overflow/underflow protection
- ✅ Peek functionality
- ✅ Buffer state management
- ✅ Clear and reuse
- ✅ Edge cases and error conditions

## Supported Platforms

- ESP32 (all variants)
- ESP8266
- Arduino (AVR, ARM, etc.)
- Any platform supporting C++11

## Performance

- **Push/Pop**: O(1) constant time
- **Memory overhead**: Minimal (SIZE * sizeof(T) + small metadata)
- **No dynamic allocation**: All memory allocated at compile time

## License

MIT License - see LICENSE file for details.

## Version History

### v1.5.0 (2025-01-29)
- ✅ Complete library audit and modernization
- ✅ Unity test framework integration
- ✅ Hardware validation on ESP32S3
- ✅ Comprehensive test coverage (6 test cases, 100% pass rate)
- ✅ Improved error handling and debug output
- ✅ Arduino framework compatibility verified
- ✅ Multi-platform support (ESP32/ESP8266/Arduino)

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## Support

- GitHub Issues: [pkg40/ringBuffer/issues](https://github.com/pkg40/ringBuffer/issues)
- Documentation: [GitHub Wiki](https://github.com/pkg40/ringBuffer/wiki)
