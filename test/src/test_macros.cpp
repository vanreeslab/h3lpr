#include "gtest/gtest.h"
#include "macros.hpp"
#include "ptr.hpp"

using namespace H3LPR;

#define ALIGNMENT 16

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
    {
        auto    a_ptr = m_ptr<H3LPR_ALLOC_POSIX, double*, ALIGNMENT>(17 * sizeof(double));
        double* a     = a_ptr();

        m_assert_h3lpr(m_isaligned(a_ptr(), ALIGNMENT), "the pointer a must be aligned");
        // get the aligned pointer
        double* a_algn = a;
        for (int i = 0; i < 17; ++i) {
            a_algn[i] = i * i;
        }
        a_ptr.free();
    }
    {
        auto    a_ptr = m_ptr<H3LPR_ALLOC_MPI, double*, ALIGNMENT>(17 * sizeof(double));
        double* a     = a_ptr();

        m_assert_h3lpr(m_isaligned(a_ptr(), ALIGNMENT), "the pointer a must be aligned");
        // get the aligned pointer
        double* a_algn = a;
        for (int i = 0; i < 17; ++i) {
            a_algn[i] = i * i;
        }
        a_ptr.free();
        
    }
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