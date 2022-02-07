#ifndef C3PO_SRC_PARSER_HPP_
#define C3PO_SRC_PARSER_HPP_

// Most of this code was generously provided by Wim van Rees and is NOT the original work of the
// Murphy development team. It was pulled from the repository
// https://github.com/wimvanrees/growth_SM2018 on November 17th, 2021.

//  Created by Wim van Rees on 8/25/16.
//  Copyright © 2016 Wim van Rees. All rights reserved.

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

#include "macros.hpp"

namespace C3PO {

// ==== helper methods : convert string to type (and specializations for string and bool) ===== //

/**
 * @brief converts the input string to the chosen type
 */
template <typename T>
inline T convertStrToType(const std::string &s) {
    std::stringstream convert(s);

    T value;
    if (!(convert >> value)) {
        m_assert(false, "argument value for type is invalid : <%s>", s.c_str());
    }
    return value;
}

/**
 * @brief converts a string to a boolean based on 'true' and 'false'
 * 
 * @tparam  
 * @param s 
 * @return true 
 * @return false 
 */
template <>
inline bool convertStrToType(const std::string &s) {
    std::stringstream convert(s);

    bool value;
    if (!(convert >> std::boolalpha >> value)) {
        m_assert(false, "argument value for bool type is invalid : <%s>. Use true/false only.", s.c_str());
    }
    return value;
}

template <>
inline std::string convertStrToType(const std::string &s) {
    return s;
}

// ==== helper methods : convert type to string (and specializations for string and bool) ===== //
template <typename T>
inline std::string convertTypeToStr(const T &t) {
    return std::to_string(t);
}

template <>
inline std::string convertTypeToStr(const bool &t) {
    return (t ? "true" : "false");
}

template <>
inline std::string convertTypeToStr(const std::string &t) {
    return t;
}

/**
 * @brief The Parser reads and holds arg/val pairs and provides an interface to access them.
 *
 */
class Parser {
   private:
    std::set<std::string>              flags_set_    = std::set<std::string>::empty();
    std::map<std::string, std::string> arg_map_      = std::map<std::string, std::string>::empty();
    std::map<std::string, std::string> doc_arg_map_  = std::map<std::string, std::string>::empty();
    std::map<std::string, std::string> doc_flag_map_ = std::map<std::string, std::string>::empty();

    /**
     * @brief Reads the arg_string and add the pair <flag , value> to arguments_map
     * 
     * @param arg_string the flag to read
     */
    void AddArg_(const std::string &arg_string) ;

    /**
     * @brief returns the value corresponding to the requested key
     * 
     * @tparam T 
     * @param argkey the key to look for
     * @param doc the documentation that will be registered to this key
     * @param strict if true, the call fails if no key is found
     * @param defval the default value that has to be used if no key is found and strict is false
     * @return T the value corresponding to the argkey
     */
    template <typename T>
    T GetValue(const std::string &argkey, const std::string &doc, const bool strict, const T defval = T()) const {
        //----------------------------------------------------------------------
        auto it = arguments_map.find(argkey);
        // if the key is found, simply register the doc and return the value
        if (arguments_map.find(argkey) != arguments_map.end()) {
            m_verb("Found the value for key %s as %s\n", argkey.data(), it->second.data());

            // register the documentation to overwrite the one by default
            doc_arg_map_[argkey] = doc;

            // return the conversion of the string to the type
            return convertStrToType<T>(it->second);
        } else {
            // no key is found, if the search was strict, fail
            m_assert(!strict, "Mandatory command line option is not given. Argument name: <%s>", argkey.c_str());

            // register the documentation, store the corresponding default value
            doc_map_[argkey]      = doc;
            arguments_map[argkey] = ConvTypeToStr<T>(defval);
            // return the stored default value
            return defval;
        }
        //----------------------------------------------------------------------
    }

   public:

    /**
     * @brief Parse a typical command line
     */
    Parser(const int argc, char **argv) {
        // start to parse the commande line, argc = 0 is the name of the executable so skip it
        for (int i = 1; i < argc; i++) {
            std::string arg_string(argv[i]);
            AddArg_(arg_string);
        }

        m_verb("found %ld arguments and %ld flags out of %d\n", arguments_map_.size(), flags_set_.size(), argc);
    }

    bool HasValue(const std::string& arg) const {
        return arguments_map.find(arg) != arguments_map.end();
    }

    template <typename T>
    T GetValue(const std::string& arg, const std::string& doc) const {
        return parse_<T>(arg,doc, true);
    }

    template <typename T>
    T GetValue(const std::string& arg, const std::string& doc, const T defval) const {
        return parse_<T>(arg,doc, false, defval);
    }

    bool GetFlag(const std::string arg, const std::string& doc) {
        return flags_set.count(arg) != 0;
    }

    void print_options() const {
        for (const auto &it : arguments_map)
            printf("%s %s ", it.first.data(), it.second.data());
        printf("\n");
    }

    void save_options() {
        FILE *f = getFileHandle();
        if (f == nullptr) return;
        for (const auto &it : arguments_map)
            fprintf(f, "%s %s ", it.first.data(), it.second.data());
        fclose(f);
    }

    void finalize() const {
        FILE *f = getFileHandle();
        if (f == nullptr) return;
        fprintf(f, "\n");
        fclose(f);
    }
};

// class ArgumentParser : public Parser {
//    private:
//     const int    parse_argc = 0;
//     const char **parse_argv = nullptr;

//    public:
//     ArgumentParser(const int argc, char **argv) : parse_argc(argc), parse_argv(argv) {
//         // start to parse the commande line, argc=0 is the name of the 
//         for (int i = 1; i < argc; i++) {
//             std::string arg_string(argv[i]);
//             AddArg_(arg_string);
//         }

//         if (verbose) m_log("found %ld arguments and %ld flags out of %d\n", arguments_map.size(), flags_set.size(), argc);
//     }

//     int getargc() const {
//         return parse_argc;
//     }

//     char **getargv() const {
//         return parse_argv;
//     }
// };

/**
 * @brief Parses a string containing arg/val pairs.
 *
 * Example usage:
 *
 *     std::string testString = "--arg1=val1 --arg2=val2 --arg3=val3";
 *     StringParser parser(testString);
 *     std::cout << "arg1 = " << parser("--arg1").asString() << std::endl;
 */
class StringParser : public Parser {
   protected:
    // trim methods from stackoverflow

    // trim from start
    inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
    }

    // trim from end
    inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
    }

    // trim from both ends
    inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
    }

   public:
    StringParser(std::string &input) {
        std::string        arg_string;
        std::istringstream iss(input);
        while (iss >> arg_string) {
            add_arg_(arg_string);
        }
    }
};

/**
 * @brief Parses a config file. See doc/parser.md for the format.
 *
 */
class ConfigParser : public Parser {
   protected:
    // trim methods from stackoverflow

    // trim from start
    inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
    }

    // trim from end
    inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
    }

    // trim from both ends
    inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
    }

   public:
    ConfigParser(const std::string &filename) {
        // open file
        std::ifstream input_fs(filename.c_str());
        m_assert(input_fs.is_open(), "Could not open configuration file <%s>", filename.c_str());

        // remove extraneous whitespace and comments
        std::stringstream clean_ss;
        std::string       file_line_str;

        while (std::getline(input_fs, file_line_str)) {
            std::string::size_type eol = file_line_str.find('#');
            if (eol != std::string::npos) {
                file_line_str.erase(eol);
            }
            clean_ss << file_line_str << ' ';
        }

        // parse clean string
        std::string arg_string;
        while (clean_ss >> arg_string) {
            add_arg_(arg_string);
        }
    }
};

};  // namespace C3PO

#endif