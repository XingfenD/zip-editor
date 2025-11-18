#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <memory>
#include <fstream>
#include "field_descriptor.hpp"
#include "input_field.hpp"

/* read specified size of data and convert it to little endian */
template<typename T>
inline T readLittleEndian(std::ifstream& file) {
    T value;
    file.read(reinterpret_cast<char*>(&value), sizeof(T));
    return value;
}

template<typename T>
inline void writeLittleEndian(std::ofstream& file, T value) {
    file.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

std::vector<std::string> splitString(const std::string& str, const std::string& delimiter);

InputType fieldTypeToInputType(FieldType fieldType);

int hexStrToInt(const std::string& hexStr);

#endif /* UTILS_HPP */
