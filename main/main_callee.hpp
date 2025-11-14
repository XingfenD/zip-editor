#ifndef MAIN_CALLEE_HPP
#define MAIN_CALLEE_HPP

#include <string>
#include "zip_handler.hpp"

struct ParsedOptions {
    std::string zip_file;
    std::string mode;
    bool is_edit_mode;
};

int parseCommandLineOptions(int argc, char* argv[], ParsedOptions& options);
void edit(ZipHandler& zip_handler);

#endif /* MAIN_CALLEE_HPP */
