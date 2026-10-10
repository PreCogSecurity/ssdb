/*
 * SSDB Test Harness
 * Copyright (c) 2024 PreCog Security
 * Based on LevelDB's testharness.h
 */

#ifndef SSDB_TEST_HARNESS_H_
#define SSDB_TEST_HARNESS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <functional>

namespace ssdb_test {

struct TestResult {
    const char* name;
    bool passed;
    std::string message;
};

class TestHarness {
public:
    static TestHarness& Instance() {
        static TestHarness instance;
        return instance;
    }

    void RegisterTest(const char* name, std::function<void()> test_func) {
        tests_.push_back({name, test_func});
    }

    int RunAll() {
        int passed = 0;
        int failed = 0;

        printf("Running %zu tests...\n\n", tests_.size());

        for (auto& test : tests_) {
            printf("[RUN] %s\n", test.name);
            bool result = false;
            std::string error_msg;

            try {
                test.func();
                result = true;
            } catch (const std::exception& e) {
                error_msg = e.what();
            } catch (...) {
                error_msg = "Unknown exception";
            }

            if (result) {
                printf("[PASS] %s\n", test.name);
                passed++;
            } else {
                printf("[FAIL] %s: %s\n", test.name, error_msg.c_str());
                failed++;
            }
        }

        printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);
        return failed == 0 ? 0 : 1;
    }

private:
    struct Test {
        const char* name;
        std::function<void()> func;
    };

    std::vector<Test> tests_;
    TestHarness() {}
};

// Assertion macros
#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error("Assertion failed: " #condition); \
        } \
    } while(0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            char buf[256]; \
            snprintf(buf, sizeof(buf), "Expected %s == %s, got %s != %s", #expected, #actual, #expected, #actual); \
            throw std::runtime_error(buf); \
        } \
    } while(0)

#define ASSERT_NE(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            throw std::runtime_error("Expected " #expected " != " #actual); \
        } \
    } while(0)

#define ASSERT_STREQ(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            char buf[512]; \
            snprintf(buf, sizeof(buf), "Expected string \"%s\" == \"%s\"", (expected), (actual)); \
            throw std::runtime_error(buf); \
        } \
    } while(0)

#define ASSERT_STRNE(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) == 0) { \
            throw std::runtime_error("Expected strings to differ"); \
        } \
    } while(0)

#define ASSERT_OK(status) ASSERT_EQ(0, (status))
#define ASSERT_ERROR(status) ASSERT_NE(0, (status))

// Test registration macro
#define TEST(name) \
    static void Test_##name(); \
    static struct TestRegistrar_##name { \
        TestRegistrar_##name() { \
            ssdb_test::TestHarness::Instance().RegisterTest(#name, Test_##name); \
        } \
    } registrar_##name; \
    static void Test_##name()

} // namespace ssdb_test

#endif // SSDB_TEST_HARNESS_H_