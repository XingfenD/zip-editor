#include "main_callee.hpp"
#include "cxxopts.hpp"
#include <iostream>
#include <string>
#include "debug_helper.hpp"

int parseCommandLineOptions(int argc, char* argv[], ParsedOptions& options) {
    cxxopts::Options cli_options("zip_analyzer", "A tool to analyze ZIP files");
    cli_options.add_options()
        ("f,file", "ZIP file to analyze", cxxopts::value<std::string>())
        ("m,mode", "Parsing mode (standard or stream)", cxxopts::value<std::string>()->default_value("standard"))
        ("e,edit", "Edit mode (conflicts with mode option)")
        ("h,help", "Print help");

    auto result = cli_options.parse(argc, argv);

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
    if (result.count("edit") && result.count("mode")) {
        std::cerr << "Error: Options --edit and --mode are conflicting" << std::endl;
        std::cout << cli_options.help() << std::endl;
        return 1;
    }

    /* validate mode option */
    options.zip_file = result["file"].as<std::string>();

    /* validate edit option */
    options.is_edit_mode = result.count("edit") > 0;

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
