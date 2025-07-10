#include <Arduino.h>
#include <unity.h>
#include "ringBuffer.hpp"

struct TestData {
    int id;
    float value;
};

ringBuffer<TestData, 8> buffer;

void test_push_and_pop() {
    TestData in = {42, 3.14f};
    TEST_ASSERT_TRUE(buffer.push(in));
    TestData out;
    TEST_ASSERT_TRUE(buffer.pop(out));
    TEST_ASSERT_EQUAL(in.id, out.id);
    TEST_ASSERT_FLOAT_WITHIN(0.001, in.value, out.value);
}

void test_overflow() {
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

void setup() {
    delay(1000);
    UNITY_BEGIN();
    RUN_TEST(test_push_and_pop);
    RUN_TEST(test_overflow);
    RUN_TEST(test_underflow);
    RUN_TEST(test_peek);
    UNITY_END();
}

void loop() {}