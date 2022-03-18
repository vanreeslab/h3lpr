#ifndef H3LPR_SRC_PARSER_HPP_
#define H3LPR_SRC_PARSER_HPP_



#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

#include "macros.hpp"

namespace H3LPR {

//==============================================================================
/**
 * @brief converts the input string to the chosen type
 */
template <typename T>
inline T convertStrToType(const std::string &s) {
    //--------------------------------------------------------------------------
    T                  value;
    std::istringstream convert(s);
    convert >> value;
    return value;
    //--------------------------------------------------------------------------
}

/**
 * @brief specialize convertStrToType() in the case of a boolean input
 */
template <>
inline bool convertStrToType(const std::string &s) {
    m_assert_h3lpr(s == "true" || s == "false", "The string <%s> cannot be transformed into a boolean value", s.c_str());
    //--------------------------------------------------------------------------
    bool               value;
    std::istringstream convert(s);
    convert >> std::boolalpha >> value;
    return value;
    //--------------------------------------------------------------------------
}

/**
 * @brief specialize convertStrToType() in the case of an input string
 */
template <>
inline std::string convertStrToType(const std::string &s) {
    return s;
}

//==============================================================================
/**
 * @brief converts a value of type T to a string
 */
template <typename T>
inline std::string convertTypeToStr(const T &t) {
    std::ostringstream convert;
    convert << t;
    return convert.str();
}

/**
 * @brief specializes convertTypeToStr() for a boolean input
 */
template <>
inline std::string convertTypeToStr(const bool &t) {
    return (t ? "true" : "false");
}

/**
 * @brief specializes convertTypeToStr() for a string input
 */
template <>
inline std::string convertTypeToStr(const std::string &t) {
    return t;
}

//==============================================================================
/**
 * @brief The Parser reads and holds arg/val pairs and provides an interface to access them.
 *
 * :warning: this class is largely based on the code provided by Wim van Rees:
 * (https://github.com/wimvanrees/growth_SM2018 as of November 17th, 2021).
 *
 */
class Parser {
   private:
    std::string                        name_;          //!< the name of the program called
    std::set<std::string>              flag_set_;      //<! contains the list of flags given by the user
    std::map<std::string, std::string> arg_map_;       //<! containes the list of the arguments + values given by the user
    std::map<std::string, std::string> doc_arg_map_;   //!< contains the documentation for the arguments needed in the code
    std::map<std::string, std::string> doc_flag_map_;  //!< contains the documentation for the flags needed in the code

   public:
    explicit Parser();
    explicit Parser(const int argc, const char **argv);

    void Finalize();

    //--------------------------------------------------------------------------

    /** @brief return the value of a given argument and register the associated documentation. Fails if the value has not been provided */
    template <typename T>
    T GetValue(const std::string &arg, const std::string &doc) {
        return ParseArg_<T>(arg, doc, true);
    }

    /** @brief return the value of a given argument and register the associated documentation. */
    template <typename T>
    T GetValue(const std::string &arg, const std::string &doc, const T defval) {
        //----------------------------------------------------------------------
        return ParseArg_<T>(arg, doc, false, defval);
        //----------------------------------------------------------------------
    }

    /** @brief Test if a flag has been registered and register the associated documentation */
    bool GetFlag(const std::string arg, const std::string &doc) {
        //----------------------------------------------------------------------
        m_assert_h3lpr(doc != "", "the documentation cannot be empty");
        return ParseFlag_(arg, doc);
        //----------------------------------------------------------------------
    }

    /** @brief Test if a flag has been registered, no documentation is registered */
    bool TestFlag(const std::string arg) {
        //----------------------------------------------------------------------
        return ParseFlag_(arg, "");
        //----------------------------------------------------------------------
    }

    /** @brief force the call of the help at the coming Finalize call */
    void ForceHelp() {
        //----------------------------------------------------------------------
        flag_set_.insert("--help");
        //----------------------------------------------------------------------
    }

   protected:
    void ReadArgString_(const std::string &arg_string);
    bool ParseFlag_(const std::string &flagkey, const std::string &doc);
    void ParseLogFile_();

    /**
     * @brief Search for an argument and returns the value corresponding to the requested key
     *
     * if the key is found, register the doc and return the value associate to the key
     * if the key is not found, register the doc anyway and return the default value
     * However if strict is true, then fail the command if the value has not been found
     *
     * @tparam T
     * @param argkey the key to look for
     * @param doc the documentation that will be registered to this key
     * @param strict if true, the call fails if no key is found
     * @param defval the default value that has to be used if no key is found and strict is false
     * @return T the value corresponding to the argkey
     */
    template <typename T>
    T ParseArg_(const std::string &argkey, const std::string &doc, const bool strict, const T defval = T()) {
        //----------------------------------------------------------------------
        // look for the key
        const auto it = arg_map_.find(argkey);

        // if the key is found, simply register the doc and return the value
        if (it != arg_map_.end()) {
            m_verb_h3lpr("Found the value for key %s as %s\n", argkey.data(), it->second.data());
            const T value = convertStrToType<T>(it->second);
            // everything went fine, register the docstring and the associated value
            doc_arg_map_[argkey] = doc + " (default value: " + convertTypeToStr(value) + ")";
            // return the conversion of the string to the type
            return value;
        } else {
            // no key is found, if the search was strict we need to display the help to help the user
            if (strict) {
                // we add by hand the flag "--help" to force the display of the help
                flag_set_.insert("--help");
                flag_set_.insert("--error");
                m_log_h3lpr("inserting help and error");
                // register that the argument is missing
                doc_arg_map_[argkey] = doc + " (MISSING ARGUMENT)";
            } else {
                // it was not strict, so no worries just put the documentation and the defaulted value
                doc_arg_map_[argkey] = doc + " (default value: " + convertTypeToStr(defval) + ")";
            }
            // return the stored default value
            return defval;
        }
        //----------------------------------------------------------------------
    }
};

};  // namespace H3LPR

#endif