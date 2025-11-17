#include "cxxopts.hpp"
#include "zip_handler.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <csignal>
#include "main_callee.hpp"
#include "debug_helper.hpp"
#include "interactive.hpp"

int main(int argc, char *argv[]) {
    ParsedOptions options;

    /* initialize and validate command line options */
    int ret = parseCommandLineOptions(argc, argv, options);
    if (ret != 0) {
        return ret; /* display help information or error message and exit */
    }

    /* initialize remote debug client */
    RemoteDebugClient::getInstance().initialize();

    std::cout << "Analyzing ZIP file: " << options.zip_file << " in " << options.mode << " mode" << std::endl;
    std::cout << "Edit mode is " << (options.is_edit_mode ? "enabled" : "disabled") << std::endl;

    /* read the file content */
    std::ifstream file(options.zip_file, std::ios::binary);
    if (!file.is_open() || !file.good()) {
        std::cerr << "Error: Failed to open ZIP file for reading" << std::endl;
        return 1;
    }

     /* parse the file content */
    ZipHandler zip_handler(file, options.mode);
    if (!zip_handler.parse()) {
        std::cerr << "Error: Failed to parse ZIP file" << std::endl;
        return 1;
    }

    if (options.is_edit_mode) {
        edit(zip_handler);
    } else {
        zip_handler.print(); /* print the parsed results defaultly */
    }

    return 0;
}
