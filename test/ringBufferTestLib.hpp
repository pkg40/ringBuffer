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

class ringBufferTestLib {
public:
    static int passCount;
    static int failCount;
    static int testCount;

    static void startTests() {
        passCount = 0;
        failCount = 0;
        testCount = 0;
        Serial.println("\n=== ringBuffer Test Suite ===");
        Serial.println("Starting comprehensive testing...\n");
    }

    static void finishTests() {
        Serial.println("\n=== Test Results Summary ===");
        Serial.printf("Total Tests: %d\n", testCount);
        Serial.printf("Passed: %d\n", passCount);
        Serial.printf("Failed: %d\n", failCount);
        Serial.printf("Success Rate: %.1f%%\n", (testCount > 0) ? (100.0 * passCount / testCount) : 0.0);
        if (failCount == 0) {
            Serial.println("All tests passed!\n");
        } else {
            Serial.println("Some tests failed.\n");
        }
    }

    static void assertTrue(bool cond, const char* msg) {
        ++testCount;
        if (cond) {
            ++passCount;
            Serial.printf("[PASS] %s\n", msg);
        } else {
            ++failCount;
            Serial.printf("[FAIL] %s\n", msg);
        }
    }
};

int ringBufferTestLib::passCount = 0;
int ringBufferTestLib::failCount = 0;
int ringBufferTestLib::testCount = 0;
