#include "gtest/gtest.h"
#include "profiler.hpp"

using namespace H3LPR;

class TestProf : public ::testing::Test {
    void SetUp() override {
        const testing::TestInfo* const test_info = testing::UnitTest::GetInstance()->current_test_info();
        m_log_noheader("::group:: Testing %s/%s", test_info->test_suite_name(), test_info->name());
    };
    void TearDown() override {
        m_log_noheader("::endgroup::");
    };
};

#define size 1729 * 1729

TEST_F(TestProf, prof) {
    Profiler prof("loop strategies");
    // alloc a random memory
    for (int itest = 0; itest < 200; ++itest) {
        //----------------------------------------------------------------------
        m_profStart(&prof, "test-c");
        m_profStart(&prof, "alloc");
        double* a = reinterpret_cast<double*>(m_calloc(sizeof(double) * size));
        m_profStop(&prof, "alloc");

        m_assert(m_isaligned(a), "the pointer a must be aligned");

        // normal loop, aligned memory
        m_profStart(&prof, "loop - no assumed aligned");
        for (int i = 0; i < size; ++i) {
            a[i] = i * i;
        }
        m_profStop(&prof, "loop - no assumed aligned");

        // specify the alignement
        double* a_algn = m_assume_aligned(a);
        m_profStart(&prof, "loop - assumed aligned");
        for (int i = 0; i < size; ++i) {
            a_algn[i] = i * i;
        }
        m_profStop(&prof, "loop - assumed aligned");

        // lambda loop
        auto op = [&a](const int i) {
            a[i] = i * i;
        };
        m_profStart(&prof, "loop - lambda");
        for (int i = 0; i < size; ++i) {
            op(i);
        }
        m_profStop(&prof, "loop - lambda");

        m_profStart(&prof, "free");
        m_free(a);
        m_profStop(&prof, "free");
        m_profStop(&prof, "test-c");

        //----------------------------------------------------------------------
        m_profStart(&prof, "test-cpp");
        m_profStart(&prof, "alloc");
        std::vector<double> b(size);
        m_profStop(&prof, "alloc");

        // get the aligned pointer
        m_profStart(&prof, "loop");
        for (int i = 0; i < size; ++i) {
            b[i] = i * i;
        }
        m_profStop(&prof, "loop");

        m_profStop(&prof, "test-cpp");
    }

    m_profDisp(&prof);
}