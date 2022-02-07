#include "parser.hpp"

using namespace C3PO;
using std::string;

/**
 * @brief returns the file Handle 
 * 
 * @return FILE* 
 */
FILE *Parser::getFileHandle() const {
    const std::string filepath = "argumentparser.log";
    FILE             *f        = fopen(filepath.c_str(), "a");
    if (f == nullptr) {
        printf("Can not open file %s.\n", filepath.data());
        return nullptr;
    }
    return f;
}




void Parser::AddArg_(const std::string &arg_string) {
        //----------------------------------------------------------------------
        // verify that it is an admissible flag : starts with "--" and has at least one
        if (arg_string.length() <= 2 || arg_string[0] != '-' || arg_string[1] != '-') {
            m_assert(false, "found an unexpected command-line entry : <%s>", arg_string.c_str());
        }

        // check if this arg contains an '=' :
        const size_t equals_pos = arg_string.find('=');

        // if we found an equal sign, this is key-value pair
        if (equals_pos != std::string::npos) {
            std::string key_string = arg_string.substr(0, equals_pos);
            std::string val_string = arg_string.substr(equals_pos + 1);

            // verify that there is not duplicate in the command line argument, fail if it is the case
            // the doc map gatther
            m_assert(arguments_map.find(key_string) != arguments_map.end(), "found a duplicate command line argument : <%s>", key_string.c_str());

            // store the value and associate a documentation by default.
            // the documentation associate to it will be re-written if the testcase uses that value
            arguments_map[key_string] = val_string;
            doc_map_[key_string]      = "unused in the current testcase";
        } else {
            // verify that there is not duplicate in the command line input
            m_assert(flag_set_.find(arg_string) != flag_set_.end(), "found a duplicate command line argument : <%s>", arg_string.c_str());
            // this is a flag, simply insert the flag
            flags_set.insert(arg_string);
            doc_flag_map_[arg_string] = "unused in the current testcase";
        }
        //----------------------------------------------------------------------
    }