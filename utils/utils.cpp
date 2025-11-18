#include "utils.hpp"
#include <vector>

template uint16_t readLittleEndian<uint16_t>(std::ifstream& file);
template uint32_t readLittleEndian<uint32_t>(std::ifstream& file);

template void writeLittleEndian(std::ofstream& file, uint32_t value);
template void writeLittleEndian(std::ofstream& file, uint16_t value);

std::vector<std::string> splitString(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

InputType fieldTypeToInputType(FieldType fieldType) {
    switch (fieldType) {
        case FieldType::STRING:
            return InputType::STRING;
        case FieldType::HEX:
            return InputType::HEX;
        default:
            return InputType::STRING;
    }
}