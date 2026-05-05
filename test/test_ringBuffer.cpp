#include <Arduino.h>
#include <unity.h>
#include "ringBuffer.hpp"

struct TestData {
    int id;
    float value;
};

ringBuffer<TestData, 8> buffer;

void test_push_and_pop() {
    buffer.clear();
    TestData in = {42, 3.14f};
    TEST_ASSERT_TRUE(buffer.push(in));
    TestData out;
    TEST_ASSERT_TRUE(buffer.pop(out));
    TEST_ASSERT_EQUAL(in.id, out.id);
    TEST_ASSERT_FLOAT_WITHIN(0.001, in.value, out.value);
}

void test_overflow() {
    buffer.clear();
    for (int i = 0; i < 8; ++i)
        TEST_ASSERT_TRUE(buffer.push({i, i * 1.0f}));
    TEST_ASSERT_FALSE(buffer.push({999, 9.99f}));
}

void test_underflow() {
    buffer.clear();
    TestData dummy;
    TEST_ASSERT_FALSE(buffer.pop(dummy));
}

void test_peek() {
    buffer.clear();
    TestData in = {7, 7.77f};
    buffer.push(in);
    TestData peeked;
    TEST_ASSERT_TRUE(buffer.peek(peeked));
    TEST_ASSERT_EQUAL(in.id, peeked.id);
}

void test_clear_and_reuse() {
    buffer.clear();
    for (int i = 0; i < 8; ++i) buffer.push({i, i * 2.0f});
    buffer.clear();
    TEST_ASSERT_TRUE(buffer.isEmpty());
    TEST_ASSERT_TRUE(buffer.push({1, 1.1f}));
}

void test_full_and_empty_flags() {
    buffer.clear();
    TEST_ASSERT_TRUE(buffer.isEmpty());
    for (int i = 0; i < 8; ++i) buffer.push({i, 0});
    TEST_ASSERT_TRUE(buffer.isFull());
    TestData out;
    buffer.pop(out);
    TEST_ASSERT_FALSE(buffer.isFull());
}

/* ---- v1.6.0 random-access slot API tests ---- */
/* peekAt/pokeAt do not modify FIFO state, but tests use a separate instance to
 * keep table-mode behavior cleanly isolated from FIFO-mode tests above. */

ringBuffer<TestData, 8> slotBuffer;

void test_pokeAt_and_peekAt() {
    slotBuffer.clear();
    TestData a = {11, 1.1f};
    TestData b = {22, 2.2f};
    TEST_ASSERT_TRUE(slotBuffer.pokeAt(0, a));
    TEST_ASSERT_TRUE(slotBuffer.pokeAt(7, b));
    TestData out;
    TEST_ASSERT_TRUE(slotBuffer.peekAt(0, out));
    TEST_ASSERT_EQUAL(11, out.id);
    TEST_ASSERT_TRUE(slotBuffer.peekAt(7, out));
    TEST_ASSERT_EQUAL(22, out.id);
}

void test_pokeAt_out_of_range() {
    slotBuffer.clear();
    TestData v = {1, 1.0f};
    TEST_ASSERT_FALSE(slotBuffer.pokeAt(8, v));
    TEST_ASSERT_FALSE(slotBuffer.pokeAt(255, v));
    TestData out;
    TEST_ASSERT_FALSE(slotBuffer.peekAt(8, out));
}

void test_pokeAt_does_not_change_size() {
    slotBuffer.clear();
    TEST_ASSERT_TRUE(slotBuffer.isEmpty());
    TestData v = {99, 9.9f};
    slotBuffer.pokeAt(3, v);
    /* pokeAt must NOT advance _maxSize / push pointers */
    TEST_ASSERT_TRUE(slotBuffer.isEmpty());
    TEST_ASSERT_EQUAL(0u, slotBuffer.size());
}

void test_cursor_walk_and_reset() {
    slotBuffer.clear();
    for (uint8_t i = 0; i < 8; ++i) {
        TestData v = {(int)i, (float)i};
        slotBuffer.pokeAt(i, v);
    }
    slotBuffer.resetCursor();
    TEST_ASSERT_EQUAL(0, slotBuffer.getCursor());
    TestData out;
    for (uint8_t i = 0; i < 8; ++i) {
        TEST_ASSERT_TRUE(slotBuffer.readCursor(out));
        TEST_ASSERT_EQUAL((int)i, out.id);
    }
    /* Wrapped back to 0 */
    TEST_ASSERT_EQUAL(0, slotBuffer.getCursor());
    slotBuffer.readCursor(out);
    TEST_ASSERT_EQUAL(1, slotBuffer.getCursor());
    slotBuffer.resetCursor();
    TEST_ASSERT_EQUAL(0, slotBuffer.getCursor());
}

void setUp(void) {
    // Optional: runs before each test
}

void tearDown(void) {
    // Optional: runs after each test
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    
    // Give Serial time to initialize properly
    delay(2000);
    
    // Ensure serial is ready before sending data
    Serial.flush();
    
    Serial.println();
    Serial.println("Starting Unity Tests...");
    Serial.println("Ring Buffer Library Test Suite");
    Serial.println("==============================");
    Serial.flush(); // Make sure initial messages are sent
}

static bool tests_run = false;

void blink_led(int times, int delay_ms = 200) {
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(delay_ms);
        digitalWrite(LED_BUILTIN, LOW);
        delay(delay_ms);
    }
}

void loop() {
    if (!tests_run) {
        Serial.println("Running ring buffer tests...");
        
        // Blink 3 times before starting tests
        blink_led(3);
        
        UNITY_BEGIN();
        RUN_TEST(test_push_and_pop);
        RUN_TEST(test_overflow);
        RUN_TEST(test_underflow);
        RUN_TEST(test_peek);
        RUN_TEST(test_clear_and_reuse);
        RUN_TEST(test_full_and_empty_flags);
        RUN_TEST(test_pokeAt_and_peekAt);
        RUN_TEST(test_pokeAt_out_of_range);
        RUN_TEST(test_pokeAt_does_not_change_size);
        RUN_TEST(test_cursor_walk_and_reset);
        UNITY_END();
        
        // Blink 5 times after completing tests
        blink_led(5);
        
        Serial.println("Tests completed successfully!");
        Serial.println("All ring buffer functionality verified.");
        tests_run = true;
    }
    
    // Slow blink to indicate tests are done
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}