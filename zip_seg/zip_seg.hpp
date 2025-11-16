#ifndef ZIP_SEG_HPP
#define ZIP_SEG_HPP

#include <cstdint>
#include <memory>
#include <fstream>
#include <vector>

/* virtual base class for zip segment */
class ZipSeg {
public:
    virtual void print() const = 0;
    virtual bool readFromFile(std::ifstream& file) = 0;
    virtual ~ZipSeg() = default;
};

class LocalFileHeader: public ZipSeg {
public:
    /* define default constructor */
    LocalFileHeader() :
        signature(0), version_needed(0), general_bit_flag(0),
        compression_method(0), last_mod_time(0), last_mod_date(0),
        crc32(0), compressed_size(0), uncompressed_size(0),
        filename_length(0), extra_field_length(0) {}

    /* ++++ get methods ++++ */
    uint32_t getSignature() const { return signature; }
    uint16_t getVersionNeeded() const { return version_needed; }
    uint16_t getGeneralBitFlag() const { return general_bit_flag; }
    uint16_t getCompressionMethod() const { return compression_method; }
    uint16_t getLastModTime() const { return last_mod_time; }
    uint16_t getLastModDate() const { return last_mod_date; }
    uint32_t getCrc32() const { return crc32; }
    uint32_t getCompressedSize() const { return compressed_size; }
    uint32_t getUncompressedSize() const { return uncompressed_size; }
    uint16_t getFilenameLength() const { return filename_length; }
    uint16_t getExtraFieldLength() const { return extra_field_length; }
    std::string getFilename() const { return filename; }

    /* ---- get methods ---- */

    void print() const override;
    bool readFromFile(std::ifstream& file) override;
    bool writeToFile(std::ofstream& file) const;

    ~LocalFileHeader() = default;

    /* define move constructor and assignment operator */
    LocalFileHeader(LocalFileHeader&& other) noexcept = default;
    LocalFileHeader& operator=(LocalFileHeader&& other) noexcept = default;

    /* forbid copy constructor and assignment operator */
    LocalFileHeader(const LocalFileHeader&) = delete;
    LocalFileHeader& operator=(const LocalFileHeader&) = delete;

private:
    uint32_t signature;
    uint16_t version_needed;
    uint16_t general_bit_flag;
    uint16_t compression_method;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length;
    uint16_t extra_field_length;
    std::string filename;
    std::unique_ptr<uint8_t[]> extra_field;

    /* the file data is not belong to local file header, but defined in LocalFileHeader for convenience */
    std::unique_ptr<uint8_t[]> file_data;
};

class CentralDirectoryHeader: public ZipSeg {
public:
    /* define default constructor */
    CentralDirectoryHeader() :
        signature(0), version_made_by(0), version_needed(0),
        general_bit_flag(0), compression_method(0), last_mod_time(0),
        last_mod_date(0), crc32(0), compressed_size(0), uncompressed_size(0),
        filename_length(0), extra_field_length(0), file_comment_length(0),
        disk_number_start(0), internal_attr(0), external_attr(0),
        local_header_offset(0) {}


    /* ++++ get methods ++++ */
    uint32_t getSignature() const { return signature; }
    uint16_t getVersionMadeBy() const { return version_made_by; }
    uint16_t getVersionNeeded() const { return version_needed; }
    uint16_t getGeneralBitFlag() const { return general_bit_flag; }
    uint16_t getCompressionMethod() const { return compression_method; }
    uint16_t getLastModTime() const { return last_mod_time; }
    uint16_t getLastModDate() const { return last_mod_date; }
    uint32_t getCrc32() const { return crc32; }
    uint32_t getCompressedSize() const { return compressed_size; }
    uint32_t getUncompressedSize() const { return uncompressed_size; }
    uint16_t getFilenameLength() const { return filename_length; }
    uint16_t getExtraFieldLength() const { return extra_field_length; }
    uint16_t getFileCommentLength() const { return file_comment_length; }
    std::string getFilename() const { return filename; }
    std::string getFileComment() const { return file_comment; }
    /* ---- get methods ---- */

    void print() const override;
    bool readFromFile(std::ifstream& file) override;
    std::streampos getLocalFileHeaderOffset() const { return local_header_offset; }
    bool writeToFile(std::ofstream& file) const;


    ~CentralDirectoryHeader() = default;

    /* define move constructor and assignment operator */
    CentralDirectoryHeader(CentralDirectoryHeader&& other) noexcept = default;
    CentralDirectoryHeader& operator=(CentralDirectoryHeader&& other) noexcept = default;

    /* forbid copy constructor and assignment operator */
    CentralDirectoryHeader(const CentralDirectoryHeader&) = delete;
    CentralDirectoryHeader& operator=(const CentralDirectoryHeader&) = delete;

private:
    uint32_t signature;
    uint16_t version_made_by;
    uint16_t version_needed;
    uint16_t general_bit_flag;
    uint16_t compression_method;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length;
    uint16_t extra_field_length;
    uint16_t file_comment_length;
    uint16_t disk_number_start;
    uint16_t internal_attr;
    uint32_t external_attr;
    uint32_t local_header_offset;
    std::string filename;
    std::unique_ptr<uint8_t[]> extra_field;
    std::string file_comment;
};

class EndOfCentralDirectoryRecord: public ZipSeg {
public:
    void print() const override;
    bool readFromFile(std::ifstream& file) override;
    /* return the position of EndOfCentralDirectoryRecord signature found from end of file, or -1 if not found */
    static std::streampos findFromEnd(std::ifstream& file);

    /* get methods */
    uint32_t getSignature() const { return signature; }
    std::streampos getCentralDirOffset() const { return central_dir_offset; }
    uint16_t getCentralDirRecordCount() const { return central_dir_record_count; }

    bool writeToFile(std::ofstream& file) const;

    ~EndOfCentralDirectoryRecord() = default;

private:
    uint32_t signature;
    uint16_t disk_number;
    uint16_t disk_with_central_dir_start;
    uint16_t central_dir_record_count;
    uint16_t total_central_dir_record_count;
    uint32_t central_dir_size;
    uint32_t central_dir_offset;
    uint16_t zip_file_comment_length;
    std::string zip_file_comment;
};

#endif /* ZIP_SEG_HPP */
