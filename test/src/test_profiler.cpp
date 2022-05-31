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

TEST_F(TestProf, latency) {
    {
        m_log_h3lpr("clock resolution = %e", MPI_Wtick());
        double tstart = MPI_Wtime();
        for (int i = 0; i < 1729; ++i) {
            MPI_Wtime();
        }
        double tfinal = MPI_Wtime() - tstart;
        m_log_h3lpr("time to call MPI_Wtime() = %e", tfinal / 1729.0);
    }
    {
        Profiler prof("loop latency");

        double x = 0.0;

        double tstart = MPI_Wtime();
        m_profStart(&prof, "latency");
        double tstart_inner = MPI_Wtime();
        for (int i = 0; i < 1729; ++i) {
            x += (i % 2) ? sin(2.0 * i) : cos(2.0 * i);
        }
        double tfinal_inner = MPI_Wtime() - tstart_inner;
        m_profStop(&prof, "latency");
        double tfinal = MPI_Wtime() - tstart;

        double tprof   = prof.GetTime("latency");
        double t_inner = tfinal_inner;
        double t_outer = tfinal;
        m_log_h3lpr("it took %e to measure %e seconds that are actually only %e seconds => measurement accuracy = %e", t_outer, tprof, t_inner, fabs(tprof - t_inner));
    }
    // {
    //     Profiler prof("loop latency");

    //     double x      = 0.0;
    //     double tstart = MPI_Wtime();
    //     for (int i = 0; i < 100; ++i) {
    //         m_profStart(&prof, "latency");
    //         x += (i % 2) ? sin(2.0 * i) : cos(2.0 * i);
    //         m_profStop(&prof, "latency");
    //     }
    //     double tfinal  = MPI_Wtime() - tstart;
    //     double tprof   = prof.GetTime("latency");
    //     double latency = tfinal - tprof;
    //     m_log_h3lpr("latency inside loop = %e = %e - %e", latency, tfinal, tprof);
    // }
    // {
    //     Profiler prof("loop latency");

    //     double x      = 0.0;
    //     double tstart = MPI_Wtime();
    //     m_profStart(&prof, "latency");
    //     for (int i = 0; i < 100; ++i) {
    //         x += (i % 2) ? sin(2.0 * i) : cos(2.0 * i);
    //     }
    //     m_profStop(&prof, "latency");
    //     double tfinal  = MPI_Wtime() - tstart;
    //     double tprof   = prof.GetTime("latency");
    //     double latency = tfinal - tprof;
    //     m_log_h3lpr("latency outside loop = %e = %e - %e", latency, tfinal, tprof);
    // }
    // {
    //     Profiler prof("loop latency");

    //     double x      = 0.0;
    //     m_profInit(&prof, "latency");
        
    //     double tstart = MPI_Wtime();
    //     m_profStartRepeat(&prof,"latency");
    //     for (int i = 0; i < 100; ++i) {
            
    //         x += (i % 2) ? sin(2.0 * i) : cos(2.0 * i);
            
    //     }
    //     m_profStopRepeat(&prof,"latency");
    //     double tfinal  = MPI_Wtime() - tstart;
        
    //     m_profLeave(&prof, "latency");
    //     double tprof   = prof.GetTime("latency");
    //     double latency =  tprof - tfinal;
    //     m_log_h3lpr("latency repeat loop = %e = %e - %e", latency, tfinal, tprof);
    // }
}

TEST_F(TestProf, prof) {
    Profiler prof("loop strategies");
    // alloc a random memory
    for (int itest = 0; itest < 200; ++itest) {
        //----------------------------------------------------------------------
        m_profStart(&prof, "test-c");
        m_profStart(&prof, "alloc");
        double* a = reinterpret_cast<double*>(m_calloc(sizeof(double) * size));
        m_profStop(&prof, "alloc");

        m_assert_h3lpr(m_isaligned(a), "the pointer a must be aligned");

        // normal loop, aligned memory
        m_profStart(&prof, "loop - no assumed aligned");
        for (int i = 0; i < size; ++i) {
            a[i] = 2.3 * i;
        }
        m_profStop(&prof, "loop - no assumed aligned");

        // specify the alignement
        double* a_algn = m_assume_aligned(a);
        m_profStart(&prof, "loop - assumed aligned");
        for (int i = 0; i < size; ++i) {
            a_algn[i] = 2.3 * i;
        }
        m_profStop(&prof, "loop - assumed aligned");

        // lambda loop
        auto op = [&a](const int i) {
            a[i] = 2.3 * i;
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
            b[i] = 2.3 * i;
        }
        m_profStop(&prof, "loop");

        m_profStop(&prof, "test-cpp");
    }

    m_profDisp(&prof);
}