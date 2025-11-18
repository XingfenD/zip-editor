#ifndef FIELD_DESCRIPTOR_HPP
#define FIELD_DESCRIPTOR_HPP

#include <string>
#include <cctype>
#include <vector>
#include <map>

enum class FieldType {
    STRING = 0,
    HEX = 1
};

class FieldDescriptor {
private:
    std::string name;
    int bytes;
    FieldType type;
public:
    FieldDescriptor(const std::string& name, int bytes, FieldType type) : name(name), bytes(bytes), type(type) {}
    std::string getName() const { return name; }
    int getBytes() const { return bytes; }
    FieldType getType() const { return type; }

    /* parse input string to get the number of bytes it represents */
    int getBytesFromString(std::string inputStr) const {
        switch (type) {
            case FieldType::STRING:
                return inputStr.length();
            case FieldType::HEX:
                return inputStr.length() / 2;
        }
    }

    /* returns the field name with first letter capitalized and underscores replaced by spaces */
    std::string getTitle() const {
        if (name.empty()) {
            return name;
        }
        std::string title = name;
        bool capitalizeNext = true;  /* flag to indicate next character should be capitalized */

        for (size_t i = 0; i < title.length(); ++i) {
            if (title[i] == '_') {
                title[i] = ' ';  /* replace underscore with space */
                capitalizeNext = true;  /* next character should be capitalized */
            } else if (capitalizeNext) {
                title[i] = std::toupper(title[i]);  /* capitalize current character */
                capitalizeNext = false;  /* reset flag */
            } else {
                title[i] = std::tolower(title[i]);  /* make other characters lowercase */
            }
        }

        return title;
    }
};

class InputDescriptor {
public:
    InputDescriptor(const FieldDescriptor& field, const std::string& defaultValue) : field(field), defaultValue(defaultValue) {}

    const FieldDescriptor& getField() const { return field; }
    const std::string& getDefaultValue() const { return defaultValue; }



private:
    FieldDescriptor field;
    std::string defaultValue;
};

// 首先定义RelatedFieldPair结构体
struct RelatedFieldPair {
    const FieldDescriptor& key;
    const FieldDescriptor& value;

    // 可选：添加构造函数方便初始化
    RelatedFieldPair(const FieldDescriptor& k, const FieldDescriptor& v)
        : key(k), value(v) {}
};


static const FieldDescriptor SIGNATURE("signature", 4, FieldType::HEX);
static const FieldDescriptor VERSION_NEEDED("version_needed", 2, FieldType::HEX);
static const FieldDescriptor GENERAL_BIT_FLAG("general_bit_flag", 2, FieldType::HEX);
static const FieldDescriptor COMPRESSION_METHOD("compression_method", 2, FieldType::HEX);
static const FieldDescriptor LAST_MOD_TIME("last_mod_time", 2, FieldType::HEX);
static const FieldDescriptor LAST_MOD_DATE("last_mod_date", 2, FieldType::HEX);
static const FieldDescriptor CRC32("crc32", 4, FieldType::HEX);
static const FieldDescriptor COMPRESSED_SIZE("compressed_size", 4, FieldType::HEX);
static const FieldDescriptor UNCOMPRESSED_SIZE("uncompressed_size", 4, FieldType::HEX);
static const FieldDescriptor FILE_NAME_LENGTH("file_name_length", 2, FieldType::HEX);
static const FieldDescriptor EXTRA_FIELD_LENGTH("extra_field_length", 2, FieldType::HEX);
static const FieldDescriptor FILE_NAME("file_name", -1, FieldType::STRING);
static const FieldDescriptor EXTRA_FIELD("extra_field", -1, FieldType::HEX);
static const FieldDescriptor FILE_DATA("file_data", -1, FieldType::HEX);

static const std::vector<InputDescriptor> LOCAL_FILE_HEADER_INPUT_DESCRIPTORS = {
    InputDescriptor(SIGNATURE, "04034B50"),
    InputDescriptor(VERSION_NEEDED, "000A"),
    InputDescriptor(GENERAL_BIT_FLAG, "0000"),
    InputDescriptor(COMPRESSION_METHOD, "0000"),
    InputDescriptor(LAST_MOD_TIME, "5AF2"),
    InputDescriptor(LAST_MOD_DATE, "5AF2"),
    InputDescriptor(CRC32, "00000000"),
    InputDescriptor(COMPRESSED_SIZE, "00000004"),
    InputDescriptor(UNCOMPRESSED_SIZE, "00000004"),
    InputDescriptor(FILE_NAME_LENGTH, "0007"),
    InputDescriptor(EXTRA_FIELD_LENGTH, "0002"),
    InputDescriptor(FILE_NAME, "newfile"),
    InputDescriptor(EXTRA_FIELD, "1234"),
    InputDescriptor(FILE_DATA, "12345678")
};

static const std::vector<RelatedFieldPair> LOCAL_FILE_HEADER_RELATED_FIELDS = {
    RelatedFieldPair(FILE_NAME_LENGTH, FILE_NAME),
    RelatedFieldPair(EXTRA_FIELD_LENGTH, EXTRA_FIELD)
};

static const std::string LFH_LENGTH_UNMATCH_KEY("lfh_length_unmatch");

#endif /* FIELD_DESCRIPTOR_HPP */
