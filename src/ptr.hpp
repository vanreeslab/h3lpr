/*
 * Copyright (c) Massachusetts Institute of Technology
 *
 * See LICENSE in top-level directory
 */
#ifndef H3LPR_SRC_PTR_HPP_
#define H3LPR_SRC_PTR_HPP_

#include <cstdint>

#include "macros.hpp"
#include "mpi.h"

//==============================================================================
namespace H3LPR {

typedef enum Allocation_t {
    H3LPR_ALLOC_POSIX,
    H3LPR_ALLOC_MPI
} Allocation_t;

template <Allocation_t L, typename T, int ALG>
class m_ptr;

//==============================================================================
// POSIX = SYSTEM ALLOCATOR
template <typename T, int ALG>
class m_ptr<H3LPR_ALLOC_POSIX, T, ALG> {
    void* ptr_;

   public:
    m_ptr() : ptr_(nullptr){};

    explicit m_ptr(const size_t size_byte) noexcept { calloc(size_byte); };

    void calloc(const size_t size_byte) {
        //----------------------------------------------------------------------
        // first get a multiple of the alignment as a size (in case we allocate back to back, unsure why though...)
        size_t size        = (size_t)(size_byte) + (ALG - 1);
        size_t padded_size = (size) - (size % ALG);
        posix_memalign(&ptr_, ALG, padded_size);
        std::memset(ptr_, 0, padded_size);
        //----------------------------------------------------------------------
    };

    void free() {
        //----------------------------------------------------------------------
        if (ptr_ != nullptr) {
            std::free(ptr_);
        }
        //----------------------------------------------------------------------
    };

    T operator()() const noexcept {
        //----------------------------------------------------------------------
        m_assert_h3lpr(ptr_ != nullptr, "The pointer shouldn't be nullptr here");
        return reinterpret_cast<T>(ptr_);
        //----------------------------------------------------------------------
    };
};

//==============================================================================
// MPI ALLOCATOR
template <typename T, int ALG>
class m_ptr<H3LPR_ALLOC_MPI, T, ALG> {
    void*  ptr_;
    size_t offset_byte_;

   public:
    m_ptr() : ptr_(nullptr), offset_byte_(0){};

    explicit m_ptr(const size_t size_byte) noexcept { calloc(size_byte); };

    /**
     * @brief allocates memory aligned on ALG bytes
     *
     * @param size_byte the memory size in byte
     */
    void calloc(const MPI_Aint size_byte) {
        //----------------------------------------------------------------------
        // first get a multiple of the alignment as a size (in case we allocate back to back, unsure why though...)
        size_t size        = (size_t)(size_byte) + (ALG - 1);
        size_t padded_size = (size) - (size % ALG);
        // add 1 x the alginment in case the allocation is not aligned
        MPI_Aint alloc_size = (MPI_Aint)(padded_size + ALG);
        MPI_Alloc_mem(alloc_size, MPI_INFO_NULL, &ptr_);
        std::memset(ptr_, 0, alloc_size);

        // get the offset in byte, i.e. the address at which the memory is aligned
        const size_t ptr_mod = ((uintptr_t)(ptr_) % ALG);
        offset_byte_         = (ptr_mod == 0) ? 0 : (ALG - ptr_mod);
        m_assert_h3lpr(ptr_mod < ALG, "the modulo = %ld cannot be bigger than %d", ptr_mod, ALG);
        m_assert_h3lpr(offset_byte_ < ALG, "the modulo = %ld cannot be bigger than %d", offset_byte_, ALG);
        //----------------------------------------------------------------------
    };

    void free() {
        //----------------------------------------------------------------------
        if (ptr_) {
            MPI_Free_mem(ptr_);
        }
        //----------------------------------------------------------------------
    };

    T operator()() const noexcept {
        //----------------------------------------------------------------------
        m_assert_h3lpr(ptr_ != nullptr, "The pointer shouldn't be nullptr here");
        // return reinterpret_cast<T>(ptr_);
        m_assert_h3lpr(offset_byte_ < ALG, "the modulo = %ld cannot be bigger than %d", offset_byte_, ALG);
        return reinterpret_cast<T>((uintptr_t)(ptr_) + offset_byte_);
        //----------------------------------------------------------------------
    };
};
};  // namespace H3LPR

#endif  // H3LPR_SRC_PTR_HPP_
