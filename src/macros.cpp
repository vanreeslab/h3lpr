#include "macros.hpp"

#include <cxxabi.h>
#include <dlfcn.h>  // for dladdr

// useful global variables for the log
namespace H3LPR {
short m_log_level_counter = 0;
char  m_log_level_prefix[32];

/**
 * @brief returns the commit id of the current h3lpr lib
 *
 * @return std::string
 */
std::string GetCommit() {
    //--------------------------------------------------------------------------
    // register the current git commit for tracking purpose
#ifdef GIT_COMMIT
    return std::string(GIT_COMMIT);
#else
    return std::string("?");
#endif
    //--------------------------------------------------------------------------
}

/**
 * @brief prints the backtrace history
 *
 * based on
 * - https://www.gnu.org/software/libc/manual/html_node/Backtraces.html
 * - https://gist.github.com/fmela/591333/c64f4eb86037bb237862a8283df70cdfc25f01d3
 * - https://linux.die.net/man/3/dladdr
 *
 * The symbols generated might be weird if the demangler doesn't work correctly
 * if you don't see any function name, add the option `-rdynamic` to the linker's flag
 *
 */
void PrintBackTrace(const char name[]) {
    //--------------------------------------------------------------------------
#if (1 == M_BACKTRACE)
    // get the addresses of the function currently in execution
    void *call_stack[M_BACKTRACE_HISTORY];  // array of
    int   size = backtrace(call_stack, M_BACKTRACE_HISTORY);

    // transform the pointers into readable symbols
    char **strings;
    strings = backtrace_symbols(call_stack, size);
    if (strings != NULL) {
        m_log_def(name, "--------------------- CALL STACK ----------------------");
        // we start at 1 to not display this function
        for (int i = 1; i < size; i++) {
            // get the address associated to the callstack
            Dl_info info;
            int     err = dladdr(call_stack[i], &info);
            // try to get it demangled
            int   status;
            char *demgled_name = abi::__cxa_demangle(info.dli_sname, NULL, NULL, &status);

            //  display the demangled name if we succeeded, weird name if not
            m_log_def(name, "%s", (status == 0) ? demgled_name : strings[i]);
            free(demgled_name);
        }
        m_log_def(name, "-------------------------------------------------------");
    }
    // free the different string with the names
    free(strings);
#endif
    //--------------------------------------------------------------------------
}

};  // namespace H3LPR