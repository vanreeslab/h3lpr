/*
 * Copyright (c) Massachusetts Institute of Technology
 *
 * See LICENSE in top-level directory
 */
#include "parser.hpp"

using string = std::string;

namespace H3LPR {

/**
 * @brief creates an empty parser with the default options: --help and --logfile
 *
 */
Parser::Parser() : flag_set_(), arg_map_(), doc_arg_map_(), doc_flag_map_() {
    //--------------------------------------------------------------------------
    string help_key   = "--help";
    string config_key = "--config";
    // register a stupid
    doc_flag_map_[help_key]  = "prints this help message";
    doc_arg_map_[config_key] = h3lpr_docargstr("reads the configuration from filename, ex: --config=filename", "");
    // update the lengths
    max_flag_length = m_max(max_flag_length, help_key.length());
    max_arg_length  = m_max(max_arg_length, config_key.length());
    //--------------------------------------------------------------------------
}

/**
 * @brief creates a parser and reads the command line input
 */
Parser::Parser(const int argc, const char **argv) : Parser() {
    //--------------------------------------------------------------------------
    // get the program name
    string name_string(argv[0]);
    size_t slash_pos = name_string.find_last_of("/");
    name_ = name_string.substr(slash_pos+1);
    // after the init of a standard Parser, process the command line
    // start to parse the commande line, argc = 0 is the name of the executable so skip it
    for (int i = 1; i < argc; i++) {
        string arg_string(argv[i]);
        ReadArgString_(arg_string);
    }
    m_verb_h3lpr("found %ld arguments and %ld flags out of %d\n", arg_map_.size(), flag_set_.size(), argc);

    // after having read the input, we must read the file if any
    ParseLogFile_();
    //--------------------------------------------------------------------------
}

/**
 * @brief displays the help if requested
 *
 */
void Parser::Finalize() {
    //--------------------------------------------------------------------------
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // get the action to do
    const bool do_help = flag_set_.find("--help") != flag_set_.end();

    if (do_help && rank == 0) {
        std::ostringstream buff;
        buff << "\nPossible parameters and flags for <" << name_ << "> \n";

        // possible flags
        buff << "\nflags:\n";
        for (auto it : doc_flag_map_) {
            string key = it.first;
            m_assert_h3lpr(max_flag_length >= key.length(), "the max length = %ld must be bigger than the key length = %ld", max_flag_length, key.length());
            key.append(max_flag_length - key.length() + 3, 0x20);
            buff << "\t" << key << it.second << "\n";
        }

        // possible arguments
        buff << "\narguments:\n";
        for (auto it : doc_arg_map_) {
            string key    = it.first;
            string doc    = std::get<0>(it.second);
            string defval = std::get<1>(it.second);

            string msg_key = key;
            if (defval.length() > 0) {
                msg_key += "[=" + defval + "]";
            }
            m_assert_h3lpr(max_arg_length >= msg_key.length(), "the max length = %ld must be bigger than the key length = %ld", max_arg_length, key.length());
            msg_key.append(max_arg_length - msg_key.length() + 3, 0x20);
            buff << "\t" << msg_key << doc << "\n";
        }

        // list the provided arguments
        buff << "\nprovided:\n";
        for (auto it : flag_set_) {
            buff << "\t" << it << "\n";
        }
        for (auto it : arg_map_) {
            buff << "\t" << it.first << "=" << it.second << "\n";
        }

        // display all that
        std::cout << buff.str();
    }
    // check if we need to fail (no arguement provided)
    const bool do_fail = flag_set_.find("--error") != flag_set_.end();
    m_assert_h3lpr(!do_fail,"you have failed to provide the required argument, please read the help");
    //--------------------------------------------------------------------------
}

/**
 * @brief returns true if the flag has been found in the command line
 *
 * if the flag is found, register the doc and return true
 * if the flag is not found, register the doc anyway and return false
 * 
 * @warning the documentation is overwritten in case the flag has already been requested
 *
 */
bool Parser::ParseFlag_(const std::string &flagkey, const std::string &doc) {
    //--------------------------------------------------------------------------
    // register the doc if the documentation is not empty
    // if already present it's overwritten
    if (doc != "") {
        doc_flag_map_[flagkey] = doc;
        max_flag_length = m_max(max_flag_length, flagkey.length());
    }

    // try to find the flag and return it
    return (flag_set_.count(flagkey) > 0);
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
        m_assert_h3lpr(false, "found an unexpected command-line entry : <%s>", arg_string.c_str());
    }

    // check if this arg contains an '=' :
    const size_t equals_pos = arg_string.find('=');

    if (equals_pos != string::npos) {
        // if we found an equal sign, this is key-value pair
        string key_string = arg_string.substr(0, equals_pos);
        string val_string = arg_string.substr(equals_pos + 1);

        // verify that there is not duplicate in the command line argument, fail if it is the case
        // the doc map gatther
        m_assert_h3lpr(arg_map_.find(key_string) == arg_map_.end(), "found a duplicate command line argument : <%s>", key_string.c_str());

        // store the value and associate a documentation by default.
        // the documentation associate to it will be re-written if the testcase uses that value
        arg_map_[key_string] = val_string;
    } else {
        // remove the first two '--' for the flag, so from 2 to the enda
        // string flag = arg_string.substr(2);
        // verify that there is not duplicate in the command line input
        m_assert_h3lpr(flag_set_.find(arg_string) == flag_set_.end(), "found a duplicate command line argument : <%s>", arg_string.c_str());
        // this is a flag, simply insert the flag
        flag_set_.insert(arg_string);
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
    const auto it      = arg_map_.find("--config");
    const bool do_read = it != arg_map_.end();

    // all the cpus will read the file together
    if (do_read) {
        const string filename = it->second;
        // open file
        std::ifstream input_fs(filename.c_str());
        m_assert_h3lpr(input_fs.is_open(), "Could not open configuration file <%s>", filename.c_str());

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
            ReadArgString_(arg_string);
        }
    }
    //--------------------------------------------------------------------------
}

}  // namespace H3LPR
