#ifndef H3LPR_SRC_MACROS_HPP_
#define H3LPR_SRC_MACROS_HPP_

#include <execinfo.h>
#include <mpi.h>
#include <omp.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

#ifndef NDEBUG
#define M_DEBUG 1
#else
#define M_DEBUG 0
#endif


//==============================================================================
/**
 * @name logs and verbosity 
 * 
 */
namespace H3LPR {
extern short m_log_level_counter;
extern char  m_log_level_prefix[32];
};
using H3LPR::m_log_level_counter;
using H3LPR::m_log_level_prefix;

//==============================================================================
/**
 * @name user-defined parameters 
 * @{
 */

#define M_ALIGNMENT 16  //!< memory alignement (in Byte, 16 = 2 doubles = 4 floats)

/** @} */

//==============================================================================
/** @brief insist more on the alignement */
#define m_inline \
    __attribute__((always_inline)) inline

/** @brief returns true if the memory is aligned */
#define m_isaligned(a)                                  \
    ({                                                  \
        const void* m_isaligned_a_ = (void*)(a);        \
        ((uintptr_t)m_isaligned_a_) % M_ALIGNMENT == 0; \
    })

/** @brief instruct the compiler that the memory is aligned */
#define m_assume_aligned(a)                                                           \
    ({                                                                                \
        decltype(a) m_assume_aligned_a_ = (a);                                        \
        m_assert(m_isaligned(m_assume_aligned_a_), "data has to be aligned");         \
        (decltype(a))(__builtin_assume_aligned(m_assume_aligned_a_, M_ALIGNMENT, 0)); \
    })

/** @brief allocate a given size (in Byte) and set to 0 the array, the return pointer is aligned to M_ALIGMEMENT */
#define m_calloc(size)                                                                    \
    ({                                                                                    \
        size_t m_calloc_size_        = (size_t)(size) + M_ALIGNMENT - 1;                  \
        size_t m_calloc_padded_size_ = (m_calloc_size_) - (m_calloc_size_ % M_ALIGNMENT); \
        void * m_calloc_data_;\
        posix_memalign(&m_calloc_data_,M_ALIGNMENT,m_calloc_padded_size_);\
        std::memset(m_calloc_data_, 0, m_calloc_padded_size_);                            \
        m_calloc_data_;                                                                   \
    })
/** @brief frees the pointer allocated using @ref m_calloc() */
#define m_free(data)                        \
    ({                                      \
        void* m_free_data_ = (void*)(data); \
        std::free(m_free_data_);            \
    })

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



#define m_log_level_plus                                  \
    ({                                                    \
        m_log_level_counter += (m_log_level_counter < 5); \
                                                          \
        m_log_level_prefix[0] = '\0';                     \
        for (short i = 0; i < m_log_level_counter; ++i) { \
            strcat(m_log_level_prefix, "  ");             \
        }                                                 \
    })
#define m_log_level_minus                                 \
    ({                                                    \
        m_log_level_counter -= (m_log_level_counter > 0); \
                                                          \
        m_log_level_prefix[0] = '\0';                     \
        for (short i = 0; i < m_log_level_counter; ++i) { \
            strcat(m_log_level_prefix, "  ");             \
        }                                                 \
    })

/**
 * @brief m_log will be displayed as a log, either by every rank or only by the master (given LOG_ALLRANKS)
 *
 */
#ifndef LOG_MUTE
#ifndef LOG_ALLRANKS
#define m_log(format, ...)                                                       \
    ({                                                                           \
        int m_log_rank_;                                                         \
        MPI_Comm_rank(MPI_COMM_WORLD, &m_log_rank_);                             \
        if (m_log_rank_ == 0) {                                                  \
            char m_log_msg_[1024];                                               \
            sprintf(m_log_msg_, format, ##__VA_ARGS__);                          \
            fprintf(stdout, "[murphy] %s %s\n", m_log_level_prefix, m_log_msg_); \
        }                                                                        \
    })

#define m_log_noheader(format, ...)                           \
    ({                                                        \
        int m_log_noheader_rank_;                             \
        MPI_Comm_rank(MPI_COMM_WORLD, &m_log_noheader_rank_); \
        char m_log_noheader_msg_[1024];                       \
        sprintf(m_log_noheader_msg_, format, ##__VA_ARGS__);  \
        if (m_log_noheader_rank_ == 0) {                      \
            fprintf(stdout, "%s\n", m_log_noheader_msg_);     \
        }                                                     \
    })
#else
#define m_log(format, ...)                                                                   \
    ({                                                                                       \
        int m_log_rank_;                                                                     \
        MPI_Comm_rank(MPI_COMM_WORLD, &m_log_rank_);                                         \
        char m_log_msg_[1024];                                                               \
        sprintf(m_log_msg_, format, ##__VA_ARGS__);                                          \
        fprintf(stdout, "[%d murphy] %s %s\n", m_log_rank_, m_log_level_prefix, m_log_msg_); \
    })
#define m_log_noheader(format, ...)                          \
    ({                                                       \
        char m_log_noheader_msg_[1024];                      \
        sprintf(m_log_noheader_msg_, format, ##__VA_ARGS__); \
        fprintf(stdout, "%s\n", m_log_noheader_msg_);        \
    })
#endif
#else
#define m_log(format, ...) \
    { ((void)0); }
#define m_log_noheader(format, ...) \
    { ((void)0); }
#endif

/**
 * @brief m_verb will be displayed if VERBOSE is enabled
 * 
 */
#ifdef VERBOSE
#define m_verb(format, ...)                                             \
    ({                                                                  \
        int m_verb_rank_;                                               \
        MPI_Comm_rank(MPI_COMM_WORLD, &m_verb_rank_);                   \
        char m_verb_msg_[1024];                                         \
        sprintf(m_verb_msg_, format, ##__VA_ARGS__);                    \
        fprintf(stdout, "[%d murphy] %s\n", m_verb_rank_, m_verb_msg_); \
    })
#else
#define m_verb(format, ...) \
    { ((void)0); }
#endif

/**
 * @brief m_assert defines the assertion call, disable if NDEBUG is asked
 * 
 */
#ifdef NDEBUG
#define m_assert(cond, ...) \
    { ((void)0); }
#else
#define m_assert(cond, ...)                                                                                                               \
    ({                                                                                                                                    \
        bool m_assert_cond_ = (bool)(cond);                                                                                               \
        if (!(m_assert_cond_)) {                                                                                                          \
            char m_assert_msg_[1024];                                                                                                     \
            int  m_assert_rank_;                                                                                                          \
            MPI_Comm_rank(MPI_COMM_WORLD, &m_assert_rank_);                                                                               \
            sprintf(m_assert_msg_, __VA_ARGS__);                                                                                          \
            fprintf(stdout, "[%d murphy-assert] '%s' FAILED: %s (at %s:%d)\n", m_assert_rank_, #cond, m_assert_msg_, __FILE__, __LINE__); \
            fflush(stdout);                                                                                                               \
            MPI_Abort(MPI_COMM_WORLD, MPI_ERR_ASSERT);                                                                                    \
        }                                                                                                                                 \
    })
#endif

/**
 * @brief entry and exit of functions, enabled if VERBOSE is enabled
 * 
 */
#ifdef VERBOSE
#define m_begin                                                                             \
    m_assert(omp_get_num_threads() == 1, "no MPI is allowed in an openmp parallel region"); \
    double m_begin_T0 = MPI_Wtime();                                                        \
    m_verb("----- entering %s", __func__);
#define m_end                                                                               \
    m_assert(omp_get_num_threads() == 1, "no MPI is allowed in an openmp parallel region"); \
    double m_end_T1_ = MPI_Wtime();                                                         \
    m_verb("----- leaving %s after %lf [s]", __func__, (m_end_T1_) - (m_begin_T0));
#else
#define m_begin \
    { ((void)0); }
#define m_end \
    { ((void)0); }
#endif
/** @} */

#endif  // H3LPR_SRC_MACROS_HPP_
