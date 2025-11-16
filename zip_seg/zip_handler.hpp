#ifndef ZIP_HANDLER_HPP
#define ZIP_HANDLER_HPP

#include <fstream>
#include <string>
#include "zip_seg.hpp"

class ZipHandler {
public:
    ZipHandler(std::ifstream& file, std::string parse_mode);
    ~ZipHandler() = default;
    bool parse();
    uint16_t parseStream();
    bool parseStandard();

    /* ++++ commands ++++ */
    void printLocalFileHeaders() const;
    void printCentralDirectoryHeaders() const;
    void printEndOfCentralDirectoryRecord() const;
    bool save(const std::string& output_path);
    /* ---- commands ---- */

    void print() const;
    void writeToFile();

private:
    std::ifstream file;
    std::ofstream output_file;
    std::string parse_mode;
    std::vector<LocalFileHeader> local_file_headers;
    std::vector<CentralDirectoryHeader> central_directory_headers;
    EndOfCentralDirectoryRecord end_of_central_directory_record;
    uint16_t local_file_header_count;
};

#endif /* ZIP_HANDLER_HPP */
