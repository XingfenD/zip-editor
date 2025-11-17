#include "main_callee.hpp"
#include "cxxopts.hpp"
#include <iostream>
#include <string>
#include "debug_helper.hpp"

int parseCommandLineOptions(int argc, char* argv[], ParsedOptions& options) {
    cxxopts::Options cli_options("zip_analyzer", "A tool to analyze and edit ZIP files");
    cli_options.add_options()
        ("f,file", "ZIP file to analyze", cxxopts::value<std::string>())
        ("m,mode", "Parsing mode (standard or stream) - only valid with -p option", cxxopts::value<std::string>()->default_value("standard"))
        ("p,print", "Print mode - print the parsed results directly")
        ("h,help", "Print help");
    cxxopts::ParseResult result;
    try {
        result = cli_options.parse(argc, argv);
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    /* help option */
    if (result.count("help")) {
        std::cout << cli_options.help() << std::endl;
        return 0; /* display help information and exit */
    }

    /* validate file option */
    if (!result.count("file")) {
        std::cerr << "Error: ZIP file not specified" << std::endl;
        std::cout << cli_options.help() << std::endl;
        return 1;
    }

    /* validate mode option */
    if (!result.count("print") && result.count("mode")) {
        std::cerr << "Error: Option --mode is only valid with --print option" << std::endl;
        std::cout << cli_options.help() << std::endl;
        return 1;
    }

    /* validate mode option */
    options.zip_file = result["file"].as<std::string>();

    /* set print mode flag - default is edit mode */
    options.is_edit_mode = result.count("print") == 0;

    /* validate mode option */
    options.mode = "standard"; /* default mode is standard */
    if (!options.is_edit_mode && result.count("mode")) {
        options.mode = result["mode"].as<std::string>();
    }

    /* validate mode option */
    if (options.mode != "standard" && options.mode != "stream") {
        std::cerr << "Error: Invalid mode specified. Use 'standard' or 'stream'" << std::endl;
        std::cout << cli_options.help() << std::endl;
        return 1;
    }

    return 0; /* options are valid */
}
