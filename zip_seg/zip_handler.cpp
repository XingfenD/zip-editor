#include "zip_handler.hpp"
#include "defs.hpp"
#include <iostream>

ZipHandler::ZipHandler(std::ifstream& file, std::string parse_mode) : file(std::move(file)), parse_mode(parse_mode) {}

bool ZipHandler::parse() {
    if (parse_mode == "standard") {
        return parseStandard();
    } else if (parse_mode == "stream") {
        uint16_t success_count = parseStream();

        local_file_header_count = success_count;

        return success_count > 0;
    } else {
        return false;
    }
}

bool ZipHandler::parseStandard() {
    if (!file.is_open() || !file.good()) {
        return false;
    }

    /* find EndOfCentralDirectoryRecord from end of file */
    std::streampos record_pos = EndOfCentralDirectoryRecord::findFromEnd(file);
    if (record_pos == -1) {
        return false;
    }

    /* move file pointer to record position and read */
    file.seekg(record_pos);
    if (!end_of_central_directory_record.readFromFile(file)) {
        return false;
    }

    /* move file pointer to start of central directory */
    file.seekg(end_of_central_directory_record.getCentralDirOffset());

    for (uint16_t i = 0; i < end_of_central_directory_record.getCentralDirRecordCount(); ++i) {
        CentralDirectoryHeader header;
        if (!header.readFromFile(file)) {
            return false;
        }
        central_directory_headers.push_back(std::move(header));
    }

    for (const auto& header : central_directory_headers) {
        file.seekg(header.getLocalFileHeaderOffset());
        LocalFileHeader local_header;
        if (!local_header.readFromFile(file)) {
            return false;
        }
        local_file_headers.push_back(std::move(local_header));
    }

    return true;
}

uint16_t ZipHandler::parseStream() {
    if (!file.is_open() || !file.good()) {
        return 0;
    }
    /* start from the first byte of the file */
    file.seekg(0);

    uint16_t success_count = 0;
    /* parse only local file headers */
    while (true) {
        /* create a local file header object */
        LocalFileHeader local_header;

        /* try to read local file header */
        if (!local_header.readFromFile(file)) {
            /* parse failed, return false */
            return success_count;
        }

        /* read success, increment success count */
        success_count++;

        /* read success, move data to vector */
        local_file_headers.push_back(std::move(local_header));
    }

    return success_count;
}

void ZipHandler::print() const {
    printLocalFileHeaders();
    printCentralDirectoryHeaders();
    printEndOfCentralDirectoryRecord();
}

void ZipHandler::printLocalFileHeaders() const {
    for (const auto& header : local_file_headers) {
        header.print();
    }
}
void ZipHandler::printCentralDirectoryHeaders() const {
    for (const auto& header : central_directory_headers) {
        header.print();
    }
}

void ZipHandler::printEndOfCentralDirectoryRecord() const {
    if (end_of_central_directory_record.getSignature() == END_OF_CENTRAL_DIRECTORY_SIG) {
        end_of_central_directory_record.print();
    }
}

/**
 * Saves the ZIP file to the specified output path
 * @param output_path Path to save the ZIP file
 * @return True if save was successful, false otherwise
 */
bool ZipHandler::save(const std::string& output_path) {
    try {
        /* Open output file in binary mode */
        std::ofstream out_file(output_path, std::ios::binary);
        if (!out_file.is_open()) {
            std::cerr << "Error: Could not open output file: " << output_path << std::endl;
            return false;
        }

        writeToFile();

        std::cout << "ZIP file successfully saved to: " << output_path << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error while saving ZIP file: " << e.what() << std::endl;
        return false;
    }
}

void ZipHandler::writeToFile() {
    for (const auto& header : local_file_headers) {
        header.writeToFile(output_file);
    }
    for (const auto& header : central_directory_headers) {
        header.writeToFile(output_file);
    }
    end_of_central_directory_record.writeToFile(output_file);
}