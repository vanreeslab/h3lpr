#include "gtest/gtest.h"
#include "macros.hpp"

class TestMacros : public ::testing::Test {
    void SetUp() override {
        const testing::TestInfo* const test_info = testing::UnitTest::GetInstance()->current_test_info();
        m_log_noheader("::group:: Testing %s/%s", test_info->test_suite_name(), test_info->name());
    };
    void TearDown() override {
        m_log_noheader("::endgroup::");
    };
};

TEST_F(TestMacros, alloc) {
    // alloc a random memory
    double* a = reinterpret_cast<double*>(m_calloc(sizeof(double) * 17));

    m_assert(m_isaligned(a), "the pointer a must be aligned");
    // get the aligned pointer
    double* a_algn = m_assume_aligned(a);
    for (int i = 0; i < 17; ++i) {
        a_algn[i] = i * i;
    }

    m_free(a);
}

TEST_F(TestMacros, log) {
    m_log_h3lpr("coucou-1.0");
    m_log_level_minus;
    m_log_h3lpr("coucou-2.0");
    m_log_level_plus;
    m_log_h3lpr("coucou-2.1");
    m_log_level_plus;
    m_log_level_plus;
    m_log_h3lpr("coucou-2.3");
    m_log_level_minus;
    m_log_level_minus;
    m_log_level_minus;
    m_log_level_minus;
    m_log_h3lpr("coucou-3.0");
}

TEST_F(TestMacros, verb) {
    m_verb_h3lpr("this message should be seen if compiled in VERBOSE");
}