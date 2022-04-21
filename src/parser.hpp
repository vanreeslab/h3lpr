#ifndef H3LPR_SRC_PARSER_HPP_
#define H3LPR_SRC_PARSER_HPP_



#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <array>
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
    size_t max_flag_length = 0;  //!< max length of the flags (used to properly display the help)
    size_t max_arg_length  = 0;  //!< max length of the flags (used to properly display the help)

    std::string name_;  //!< the name of the program called

    std::map<std::string, std::string> doc_arg_map_;   //!< contains the documentation for the arguments needed in the code
    std::map<std::string, std::string> doc_flag_map_;  //!< contains the documentation for the flags needed in the code

    std::set<std::string>              flag_set_;  //<! contains the list of flags given by the user
    std::map<std::string, std::string> arg_map_;   //<! contains the list of the arguments + values given by the user

   public:
    explicit Parser();
    explicit Parser(const int argc, const char **argv);

    void Finalize();

    //--------------------------------------------------------------------------

    /** @brief return the value of a given argument and register the associated documentation. Fails if the value has not been provided */
    template <typename T>
    T GetValue(const std::string &arg, const std::string &doc) {
        m_verb_h3lpr("looking for %s",arg.c_str());
        return ParseArg_<T>(arg, doc, true);
    }

    /** @brief return the value of a given argument and register the associated documentation. */
    template <typename T>
    T GetValue(const std::string &arg, const std::string &doc, const T defval) {
        //----------------------------------------------------------------------
        m_verb_h3lpr("looking for %s",arg.c_str());
        return ParseArg_<T>(arg, doc, false, defval);
        //----------------------------------------------------------------------
    }

    /** @brief return an array of values of a given argument and register the associated documentation. Fails if the list of values has not been provided */
    template <typename T, int C>
    std::array<T, C> GetValues(const std::string &arg, const std::string &doc) {
        m_verb_h3lpr("looking for %s", arg.c_str());
        return ParseArgs_<T,C>(arg, doc, true);
    }

    /** @brief return a list of values value of a given argument and register the associated documentation. */
    template <typename T, int C>
    std::array<T, C> GetValues(const std::string &arg, const std::string &doc, const std::array<T, C> defval) {
        //----------------------------------------------------------------------
        m_verb_h3lpr("looking for %s", arg.c_str());
        return ParseArgs_<T,C>(arg, doc, false, defval);
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
     * @warning the documentation is overwritten in case the argument has already been requested
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
            doc_arg_map_[argkey] = doc + " (default value: " + convertTypeToStr(value) + " )";
            max_arg_length       = m_max(max_arg_length, argkey.length());
            // return the conversion of the string to the type
            return value;
        } else {
            // no key is found, if the search was strict we need to display the help to help the user
            if (strict) {
                // we add by hand the flag "--help" to force the display of the help
                flag_set_.insert("--help");
                flag_set_.insert("--error");
                // register that the argument is missing
                doc_arg_map_[argkey] = doc + " (MISSING ARGUMENT)";
                max_arg_length       = m_max(max_arg_length, argkey.length());
            } else {
                // it was not strict, so no worries just put the documentation and the defaulted value
                doc_arg_map_[argkey] = doc + " (default value: " + convertTypeToStr(defval) + " )";
                max_arg_length       = m_max(max_arg_length, argkey.length());
            }
            // return the stored default value
            return defval;
        }
        //----------------------------------------------------------------------
    }

    /**
     * @brief Search for an argument and returns the value corresponding to the requested key
     * 
     * Similar to the function @ref ParseArg_ but look for an array of values instead
     * 
     * @tparam T the type of the array
     * @tparam C the length of the array
     */
    template <typename T, int C>
    std::array<T, C> ParseArgs_(const std::string &argkey, const std::string &doc, const bool strict, const std::array<T, C> defval = std::array<T, C>()) {
        //----------------------------------------------------------------------
        // look for the key
        const auto it = arg_map_.find(argkey);

        // from the list obtain the default values as a string
        std::string str_defval;
        for (int i = 0; i < (C - 1); ++i) {
            str_defval += convertTypeToStr(defval[i]) + ",";
        }
        str_defval += convertTypeToStr(defval[C - 1]);

        // if the key is found, simply register the doc and return the value
        if (it != arg_map_.end()) {
            m_verb_h3lpr("Found the value for key %s as %s\n", argkey.data(), it->second.data());

            // everything went fine, register the docstring and the associated value
            doc_arg_map_[argkey] = doc + " (default value: " + str_defval + " )";
            max_arg_length       = m_max(max_arg_length, argkey.length());

            std::array<T, C> value;

            // here we copy the argument string and loop on it to fill the array
            size_t            pos;
            std::string       full_array      = it->second;
            const std::string array_delimiter = ",";
            int               count           = 0;
            while ((pos = full_array.find(",")) != std::string::npos) {
                m_assert_h3lpr(count < (C - 1), "the provided argument <%s> is too long, only %d elements are required", it->second.c_str(), C);
                // read the current value
                std::string cval = full_array.substr(0, pos);
                value[count]     = convertStrToType<T>(cval);
                count += 1;
                // forget about it
                full_array.erase(0, pos + array_delimiter.length());
            }
            // add the last one
            value[count] = convertStrToType<T>(full_array);

            // return the conversion of the string to the type
            return value;
        } else {
            // no key is found, if the search was strict we need to display the help to help the user
            if (strict) {
                // we add by hand the flag "--help" to force the display of the help
                flag_set_.insert("--help");
                flag_set_.insert("--error");
                // register that the argument is missing
                doc_arg_map_[argkey] = doc + " (MISSING ARGUMENT)";
                max_arg_length       = m_max(max_arg_length, argkey.length());
            } else {
                // it was not strict, so no worries just put the documentation and the defaulted value
                doc_arg_map_[argkey] = doc + " (default value: " + str_defval + " )";
                max_arg_length       = m_max(max_arg_length, argkey.length());
            }
            // return the stored default value
            return defval;
        }
        //----------------------------------------------------------------------
    }
};

};  // namespace H3LPR

#endif