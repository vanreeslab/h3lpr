#ifndef H3LPR_H3LPR_HPP_
#define H3LPR_H3LPR_HPP_


// declare all the classes here
#ifdef __cplusplus
namespace H3LPR {
struct m_ptr;
}
using H3LPR_m_ptr    = H3LPR::m_ptr;
#else
typedef struct H3LPR_m_ptr    H3LPR_m_ptr;     // ptr.hpp
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void  H3LPR_(Fred*); /* ANSI C prototypes */
extern Fred* cplusplus_callback_function(Fred*);

#ifdef __cplusplus
}
};  // namespace H3LPR
#endif

#endif  // H3LPR_H3LPR_HPP_