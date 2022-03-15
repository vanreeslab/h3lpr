#include "parser.hpp"

using string = std::string;

namespace H3LPR {

/**
 * @brief creates an empty parser with the default options: --help and --logfile
 *
 */
Parser::Parser() : flag_set_(), arg_map_(), doc_arg_map_(), doc_flag_map_() {
    //--------------------------------------------------------------------------
    // register a stupid
    doc_flag_map_["--help"]  = "prints this help message";
    doc_arg_map_["--config"] = "reads the configuration from filename (ex: --config=filename)";
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
    m_verb("found %ld arguments and %ld flags out of %d\n", arguments_map_.size(), flag_set_.size(), argc);

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
            buff << "\t" << it.first << "\t" << it.second << "\n";
        }

        // possible arguments
        buff << "\narguments:\n";
        for (auto it : doc_arg_map_) {
            buff << "\t" << it.first << "\t" << it.second << "\n";
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
    m_assert(!do_fail,"you have failed to provide the required argument, please read the help");
    //--------------------------------------------------------------------------
}

/**
 * @brief returns true if the flag has been found in the command line
 *
 * if the flag is found, register the doc and return true
 * if the flag is not found, register the doc anyway and return false
 *
 */
bool Parser::ParseFlag_(const std::string &flagkey, const std::string &doc) {
    //--------------------------------------------------------------------------
    // register the doc anyway
    doc_flag_map_[flagkey] = doc;

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
        m_assert(arg_map_.find(key_string) == arg_map_.end(), "found a duplicate command line argument : <%s>", key_string.c_str());

        // store the value and associate a documentation by default.
        // the documentation associate to it will be re-written if the testcase uses that value
        arg_map_[key_string] = val_string;
    } else {
        // remove the first two '--' for the flag, so from 2 to the enda
        // string flag = arg_string.substr(2);
        // verify that there is not duplicate in the command line input
        m_assert(flag_set_.find(arg_string) == flag_set_.end(), "found a duplicate command line argument : <%s>", arg_string.c_str());
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
            ReadArgString_(arg_string);
        }
    }
    //--------------------------------------------------------------------------
}

}  // namespace H3LPR