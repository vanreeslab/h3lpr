#include "parser.hpp"

using string = std::string;

namespace C3PO {

/**
 * @brief creates an empty parser with the default options: --help and --logfile
 *
 */
Parser::Parser() : flags_set_(), arg_map_(), doc_arg_map_(), doc_flag_map_() {
    //--------------------------------------------------------------------------
    // register a stupid
    doc_flag_map["--help"]   = "prints this help message";
    doc_arg_map_["--config"] = "reads the configuration from filename (ex: --config=filename";
    //--------------------------------------------------------------------------
}

/**
 * @brief creates a parser and reads the command line input
 * 
 * @param argc 
 * @param argv 
 */
Parser::Parser(const int argc, char **argv) : Parser() {
    //--------------------------------------------------------------------------
    // after the init of a standard Parser, process the command line
    // start to parse the commande line, argc = 0 is the name of the executable so skip it
    for (int i = 1; i < argc; i++) {
        string arg_string(argv[i]);
        ReadArgString_(arg_string);
    }
    m_verb("found %ld arguments and %ld flags out of %d\n", arguments_map_.size(), flags_set_.size(), argc);
    //--------------------------------------------------------------------------
}

/**
 * @brief returns true if the flag has been found in the command line
 *
 * if the flag is found, register the doc and return true
 * if the flag is not found, register the doc anyway and return false
 *
 */
bool ParseFlag_(const std::string &flagkey, const std::string &doc, const bool strict) const {
    //--------------------------------------------------------------------------
    // register the doc anyway
    doc_map_[flagkey] = doc;

    // try to find the flag and return it
    const auto it = flag_set_.find(flagkey);
    return (it != arg_map_.end());
    //--------------------------------------------------------------------------
}

/**
 * @brief Reads the arg_string and add the pair <flag , value> to arguments_map
 *
 * @param arg_string the input to process, can be of the form `--key=value` or `--flag`
 * @param doc the documentation associated to the arg/flag passed (optional)
 */
void Parser::ReadArgString_(const std::string &arg_string) {
    //--------------------------------------------------------------------------
    // verify that it is an admissible flag : starts with "--" and has at least one
    if (arg_string.length() <= 2 || arg_string[0] != '-' || arg_string[1] != '-') {
        m_assert(false, "found an unexpected command-line entry : <%s>", arg_string.c_str());
    }

    // check if this arg contains an '=' :
    const size_t equals_pos = arg_string.find('=');

    if (equals_pos != string::npos) {
        // if we found an equal sign, this is key-value pair
        string key_string = arg_string.substr(0, equals_pos);
        string val_string = arg_string.substr(equals_pos + 1);

        // verify that there is not duplicate in the command line argument, fail if it is the case
        // the doc map gatther
        m_assert(arg_map_.find(key_string) != arg_map_.end(), "found a duplicate command line argument : <%s>", key_string.c_str());

        // store the value and associate a documentation by default.
        // the documentation associate to it will be re-written if the testcase uses that value
        arg_map_[key_string] = val_string;
    } else {
        // verify that there is not duplicate in the command line input
        m_assert(flag_set_.find(arg_string) != flag_set_.end(), "found a duplicate command line argument : <%s>", arg_string.c_str());
        // this is a flag, simply insert the flag
        flags_set.insert(arg_string);
    }
    //--------------------------------------------------------------------------
}

/**
 * @brief try to parse the log file if needed
 *
 */
void Parser::ParseLogFile_() {
    //--------------------------------------------------------------------------
    // get the filename from the arguments given in command line
    const auto   it       = arg_map_.find("--config");
    const string filename = it->second;
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
    //--------------------------------------------------------------------------
}
}  // namespace C3PO