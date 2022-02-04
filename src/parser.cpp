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