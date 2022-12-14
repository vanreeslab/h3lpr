/*
 * Copyright (c) Massachusetts Institute of Technology
 *
 * See LICENSE in top-level directory
 */
#ifndef H3LPR_SRC_MACROS_HPP_
#define H3LPR_SRC_MACROS_HPP_

#include <execinfo.h>
#include <mpi.h>
#include <omp.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>

#ifndef NDEBUG
#define M_DEBUG 1
#else
#define M_DEBUG 0
#endif

#ifdef NO_BTRACE
#define M_BACKTRACE 0
#else
#define M_BACKTRACE 1
#endif

#define M_BACKTRACE_HISTORY 50

//==============================================================================
/** @brief returns true if the memory is aligned */
#define m_isaligned(a, alg)                      \
    ({                                           \
        const void* m_isaligned_a_ = (void*)(a); \
        ((uintptr_t)m_isaligned_a_) % alg == 0;  \
    })
//==============================================================================
/**
 * @name logs and verbosity
 *
 */
namespace H3LPR {
extern short m_log_level_counter;
extern char  m_log_level_prefix[32];

// retun commit id
std::string GetCommit();
// function used for backtrace logging
void PrintBackTrace(const char name[]);
};  // namespace H3LPR

//==============================================================================
// FLOAT/DOUBLE comparison
//==============================================================================
/**
 * @brief returns the max of two expressions
 *
 */
#define m_max(a, b)                                      \
    ({                                                   \
        __typeof__(a) m_max_a_ = (a);                    \
        __typeof__(b) m_max_b_ = (b);                    \
        (m_max_a_ > m_max_b_) ? (m_max_a_) : (m_max_b_); \
    })

//------------------------------------------------------------------------------
/**
 * @brief returns the min of two expressions
 *
 */
#define m_min(a, b)                                      \
    ({                                                   \
        __typeof__(a) m_min_a_ = (a);                    \
        __typeof__(b) m_min_b_ = (b);                    \
        (m_min_a_ < m_min_b_) ? (m_min_a_) : (m_min_b_); \
    })

//------------------------------------------------------------------------------
/**
 * @brief returns the sign of a number: +1 if positive, 0 if 0 and -1 if negative
 *
 */
#define m_sign(a)                                                \
    ({                                                           \
        __typeof__(a) m_sign_a_    = (a);                        \
        __typeof__(a) m_sign_zero_ = 0;                          \
        (m_sign_zero_ < m_sign_a_) - (m_sign_a_ < m_sign_zero_); \
    })

//------------------------------------------------------------------------------
/**
 * @brief returns true if a = b
 */
#define m_fequal(a, b)                                                                           \
    ({                                                                                           \
        real_t m_equal_a_ = (a);                                                                 \
        real_t m_equal_b_ = (b);                                                                 \
        (std::fabs(m_equal_a_ - m_equal_b_) < (100.0 * std::numeric_limits<real_t>::epsilon())); \
    })

//------------------------------------------------------------------------------
/**
 * @brief returns true a >= b
 */
#define m_fgeq(a, b)                                                                   \
    ({                                                                                 \
        real_t m_fgeg_a_ = (a);                                                        \
        real_t m_fgeq_b_ = (b);                                                        \
        ((m_fgeg_a_ - m_fgeq_b_) > (-100.0 * std::numeric_limits<real_t>::epsilon())); \
    })

//------------------------------------------------------------------------------
/**
 *
 * @brief returns true a <= b
 */
#define m_fleq(a, b)                                                                  \
    ({                                                                                \
        real_t m_fgeg_a_ = (a);                                                       \
        real_t m_fgeq_b_ = (b);                                                       \
        ((m_fgeg_a_ - m_fgeq_b_) < (100.0 * std::numeric_limits<real_t>::epsilon())); \
    })

/** @} */

//==============================================================================
// LOGGING
//==============================================================================

#define m_log_level_plus                                                \
    ({                                                                  \
        H3LPR::m_log_level_counter += (H3LPR::m_log_level_counter < 5); \
                                                                        \
        H3LPR::m_log_level_prefix[0] = '\0';                            \
        for (short i = 0; i < H3LPR::m_log_level_counter; ++i) {        \
            strcat(H3LPR::m_log_level_prefix, "  ");                    \
        }                                                               \
    })
//------------------------------------------------------------------------------
#define m_log_level_minus                                               \
    ({                                                                  \
        H3LPR::m_log_level_counter -= (H3LPR::m_log_level_counter > 0); \
                                                                        \
        H3LPR::m_log_level_prefix[0] = '\0';                            \
        for (short i = 0; i < H3LPR::m_log_level_counter; ++i) {        \
            strcat(H3LPR::m_log_level_prefix, "  ");                    \
        }                                                               \
    })

//------------------------------------------------------------------------------
/**
 * @brief m_log will be displayed as a log, either by every rank or only by the master (given LOG_ALLRANKS)
 *
 */
#ifndef LOG_MUTE
#ifndef LOG_ALLRANKS
#define m_log_def(header_name, format, ...)                                                          \
    ({                                                                                               \
        int m_log_def_rank_;                                                                         \
        MPI_Comm_rank(MPI_COMM_WORLD, &m_log_def_rank_);                                             \
        if (m_log_def_rank_ == 0) {                                                                  \
            char m_log_def_msg_[1024];                                                               \
            sprintf(m_log_def_msg_, format, ##__VA_ARGS__);                                          \
            fprintf(stdout, "[%s] %s %s\n", header_name, H3LPR::m_log_level_prefix, m_log_def_msg_); \
        }                                                                                            \
    })
#define m_log_noheader(format, ...)                              \
    ({                                                           \
        int m_log_noheader_rank_;                                \
        MPI_Comm_rank(MPI_COMM_WORLD, &m_log_noheader_rank_);    \
        if (m_log_noheader_rank_ == 0) {                         \
            char m_log_noheader_msg_[1024];                      \
            sprintf(m_log_noheader_msg_, format, ##__VA_ARGS__); \
            fprintf(stdout, "%s\n", m_log_noheader_msg_);        \
        }                                                        \
    })
#else  // LOG_ALLRANKS
#define m_log_def(header_name, format, ...)                                                                          \
    ({                                                                                                               \
        int m_log_def_rank_;                                                                                         \
        MPI_Comm_rank(MPI_COMM_WORLD, &m_log_def_rank_);                                                             \
        char m_log_def_msg_[1024];                                                                                   \
        sprintf(m_log_def_msg_, format, ##__VA_ARGS__);                                                              \
        fprintf(stdout, "[%d %s] %s %s\n", m_log_def_rank_, header_name, H3LPR::m_log_level_prefix, m_log_def_msg_); \
    })
#define m_log_noheader(format, ...)                          \
    ({                                                       \
        char m_log_noheader_msg_[1024];                      \
        sprintf(m_log_noheader_msg_, format, ##__VA_ARGS__); \
        fprintf(stdout, "%s\n", m_log_noheader_msg_);        \
    })
#endif  // LOG_ALLRANKS
#else   // LOG_MUTE
#define m_log_def(header_name, format, ...) \
    { ((void)0); }
#define m_log_def_noheader(format, ...) \
    { ((void)0); }
#endif  // LOG_MUTE

//------------------------------------------------------------------------------
/**
 * @brief m_verb will be displayed if VERBOSE is enabled
 *
 */
#ifdef VERBOSE
#ifndef LOG_ALLRANKS
#define m_verb_def(header_name, format, ...)                            \
    ({                                                                  \
        int m_verb_def_rank_;                                           \
        MPI_Comm_rank(MPI_COMM_WORLD, &m_verb_def_rank_);               \
        if (m_verb_def_rank_ == 0) {                                    \
            char m_verb_def_msg_[1024];                                 \
            sprintf(m_verb_def_msg_, format, ##__VA_ARGS__);            \
            fprintf(stdout, "[%s] %s\n", header_name, m_verb_def_msg_); \
        }                                                               \
    })
#else  // LOG_ALLRANKS
#define m_verb_def(header_name, format, ...)                                             \
    ({                                                                                   \
        int m_verb_def_rank_;                                                            \
        MPI_Comm_rank(MPI_COMM_WORLD, &m_verb_def_rank_);                                \
        char m_verb_def_msg_[1024];                                                      \
        sprintf(m_verb_def_msg_, format, ##__VA_ARGS__);                                 \
        fprintf(stdout, "[%d %s] %s\n", m_verb_def_rank_, header_name, m_verb_def_msg_); \
    })
#endif  // LOG_ALLRANKS
#else   // VERBOSE
#define m_verb_def(header_name, format, ...) \
    { ((void)0); }
#endif  // VERBOSE

//------------------------------------------------------------------------------
/**
 * @brief m_assert defines the assertion call, disabled if NDEBUG is asked
 *
 */
#ifdef NDEBUG
#define m_assert_def(cond, ...) \
    { ((void)0); }
#else
#define m_assert_def(header_name, cond, ...)                                                                                                               \
    ({                                                                                                                                                     \
        bool m_assert_def_cond_ = (bool)(cond);                                                                                                            \
        if (!(m_assert_def_cond_)) {                                                                                                                       \
            char m_assert_def_msg_[1024];                                                                                                                  \
            int  m_assert_def_rank_;                                                                                                                       \
            MPI_Comm_rank(MPI_COMM_WORLD, &m_assert_def_rank_);                                                                                            \
            sprintf(m_assert_def_msg_, __VA_ARGS__);                                                                                                       \
            fprintf(stdout, "[%d %s-assert] '%s' FAILED: %s (at %s:%d)\n", m_assert_def_rank_, header_name, #cond, m_assert_def_msg_, __FILE__, __LINE__); \
            H3LPR::PrintBackTrace(header_name);                                                                                                            \
            fflush(stdout);                                                                                                                                \
            MPI_Abort(MPI_COMM_WORLD, MPI_ERR_ASSERT);                                                                                                     \
        }                                                                                                                                                  \
    })
#endif

//------------------------------------------------------------------------------
/**
 * @brief entry and exit of functions, enabled if VERBOSE is enabled
 *
 */
#ifdef VERBOSE
#define m_begin_def(header_name)                                                                             \
    m_assert_def(header_name, omp_get_num_threads() == 1, "no MPI is allowed in an openmp parallel region"); \
    double m_begin_def_T0 = MPI_Wtime();                                                                     \
    m_verb_def(header_name, "----- entering %s", __func__);
#define m_end_def(header_name)                                                                               \
    m_assert_def(header_name, omp_get_num_threads() == 1, "no MPI is allowed in an openmp parallel region"); \
    double m_end_def_T1_ = MPI_Wtime();                                                                      \
    m_verb_def(header_name, "----- leaving %s after %lf [s]", __func__, (m_end_def_T1_) - (m_begin_def_T0));
#else
#define m_begin_def(header_name) \
    { ((void)0); }
#define m_end_def(header_name) \
    { ((void)0); }
#endif
/** @} */

//==============================================================================
// H3LPR specific logging
//==============================================================================

#define m_log_h3lpr(format, ...)                   \
    ({                                             \
        m_log_def("h3lpr", format, ##__VA_ARGS__); \
    })

#define m_verb_h3lpr(format, ...)                   \
    ({                                              \
        m_verb_def("h3lpr", format, ##__VA_ARGS__); \
    })

#define m_assert_h3lpr(format, ...)                   \
    ({                                                \
        m_assert_def("h3lpr", format, ##__VA_ARGS__); \
    })

#define m_begin_h3lpr \
    { (m_begin_def("h3lpr")); }
#define m_end_h3lpr \
    { (m_end_def("h3lpr")); }

#endif  // H3LPR_SRC_MACROS_HPP_
